#include "tphysics.h"

#include <tinker/profiler.h>
#include <tinker/cvar.h>
#include <game/gameserver.h>
#include <renderer/game_renderer.h>
#include <renderer/renderingcontext.h>

#include "heightmap.h"

CTPhysics::CTPhysics()
{
	// Allocate all memory up front to avoid reallocations
	m_aEntityList.set_capacity(GameServer()->GetMaxEntities());

	m_flSimulationTime = m_flGameTime = m_flServerTime = 0;
}

CTPhysics::~CTPhysics()
{
}

void CTPhysics::AddEntity(IPhysicsEntity* pEntity, collision_type_t eCollisionType)
{
	TAssert(eCollisionType != CT_NONE);
	if (eCollisionType == CT_NONE)
		return;

	TAssert(pEntity);
	if (!pEntity)
		return;

	size_t iHandle = pEntity->GetHandle();
	if (m_aEntityList.size() <= iHandle)
		m_aEntityList.resize(iHandle + 1);

	CPhysicsEntity* pPhysicsEntity = new (&m_aEntityList[iHandle]) CPhysicsEntity(pEntity, eCollisionType);
}

void CTPhysics::RemoveAllEntities()
{
	for (size_t i = 0; i < m_aEntityList.size(); i++)
	{
		auto pPhysicsEntity = &m_aEntityList[i];

		RemoveEntity(pPhysicsEntity);
	}
}

void CTPhysics::RemoveEntity(IPhysicsEntity* pEntity)
{
	CPhysicsEntity* pPhysicsEntity = GetPhysicsEntity(pEntity);
	if (!pPhysicsEntity)
		return;

	RemoveEntity(pPhysicsEntity);
}

void CTPhysics::RemoveEntity(CPhysicsEntity* pPhysicsEntity)
{
	if (!pPhysicsEntity)
		return;

	pPhysicsEntity->m_pGameEntity = nullptr;
	pPhysicsEntity->m_bActive = false;
}

size_t CTPhysics::AddExtra(size_t iExtraMesh, const Vector& vecOrigin)
{
	size_t iIndex = ~0;
	for (size_t i = 0; i < m_apExtraEntityList.size(); i++)
	{
		if (!m_apExtraEntityList[i])
		{
			iIndex = i;
			m_apExtraEntityList[i] = new CPhysicsEntity();
			break;
		}
	}

	if (iIndex == ~0)
	{
		iIndex = m_apExtraEntityList.size();
		m_apExtraEntityList.push_back(new CPhysicsEntity());
	}

	CPhysicsEntity* pPhysicsEntity = m_apExtraEntityList[iIndex];
	pPhysicsEntity->m_bActive = true;
	pPhysicsEntity->m_eCollisionType = CT_STATIC_MESH;

	TAssert(m_apExtraCollisionMeshes[iExtraMesh]);
	if (!m_apExtraCollisionMeshes[iExtraMesh])
		return ~0;

	CPhysicsMesh* pMesh = m_apExtraCollisionMeshes[iExtraMesh]->m_pMesh;

	TAssert(pMesh);

	pPhysicsEntity->m_pMesh = pMesh;

	return iIndex;
}

void CTPhysics::RemoveExtra(size_t iExtra)
{
	CPhysicsEntity* pPhysicsEntity = m_apExtraEntityList[iExtra];

	TAssert(pPhysicsEntity);
	if (!pPhysicsEntity)
		return;

	delete m_apExtraEntityList[iExtra];
	m_apExtraEntityList[iExtra] = nullptr;
}

size_t CTPhysics::LoadExtraCollisionHeightmapMesh(size_t iWidth, size_t iHeight, const AABB& aabbBounds, float* aflVerts)
{
	size_t iIndex = ~0;
	for (size_t i = 0; i < m_apExtraCollisionMeshes.size(); i++)
	{
		if (!m_apExtraCollisionMeshes[i])
		{
			iIndex = i;
			m_apExtraCollisionMeshes[i] = new CCollisionMesh();
			break;
		}
	}

	if (iIndex == ~0)
	{
		iIndex = m_apExtraCollisionMeshes.size();
		m_apExtraCollisionMeshes.push_back(new CCollisionMesh());
	}

	m_apExtraCollisionMeshes[iIndex]->m_pMesh = new CHeightmapMesh(iWidth, iHeight, aabbBounds, aflVerts);

	return iIndex;
}

void CTPhysics::UnloadExtraCollisionMesh(size_t iIndex)
{
	TAssert(m_apExtraCollisionMeshes[iIndex]);
	if (!m_apExtraCollisionMeshes[iIndex])
		return;

	// Make sure there are no objects using this collision shape.
	for (size_t i = 0; i < m_apExtraEntityList.size(); i++)
	{
		if (!m_apExtraEntityList[i])
			continue;

		if (!m_apExtraCollisionMeshes[i])
			continue;

		if (m_apExtraEntityList[i]->m_pMesh == m_apExtraCollisionMeshes[iIndex]->m_pMesh)
		{
			RemoveExtra(i);
			TError("Entity found with collision mesh which is being unloaded\n");
		}
	}

	delete m_apExtraCollisionMeshes[iIndex]->m_pMesh;

	delete m_apExtraCollisionMeshes[iIndex];
	m_apExtraCollisionMeshes[iIndex] = nullptr;
}

#ifdef _DEBUG
#define PHYS_TIMESTEP "0.03333333333"
#else
#define PHYS_TIMESTEP "0.01666666666"
#endif

CVar phys_timestep("phys_timestep", PHYS_TIMESTEP);

void CTPhysics::Simulate()
{
	TPROF("CTPhysics::Simulate");

	if (!m_flServerTime)
	{
		m_flServerTime = GameServer()->GetGameTime();
		return;
	}

	m_flGameTime += GameServer()->GetGameTime() - m_flServerTime;
	m_flServerTime = GameServer()->GetGameTime();

	m_apSimulateList.reserve(m_aEntityList.size());
	m_apSimulateList.clear();

	for (auto& oEntity : m_aEntityList)
	{
		if (!oEntity.m_pGameEntity)
			continue;

		if (!oEntity.m_bActive)
			continue;

		if (oEntity.m_eCollisionType != CT_DYNAMIC)
			continue;

		m_apSimulateList.push_back(&oEntity);
	}

	float flSimulationFrameTime = phys_timestep.GetFloat();

	// Break simulations up into very small steps in order to preserve accuracy.
	// I think floating point precision causes this problem but I'm not sure. Anyway this works better for my projectiles.
	for (double flCurrentSimulationTime = m_flSimulationTime; flCurrentSimulationTime < m_flGameTime; flCurrentSimulationTime += flSimulationFrameTime)
	{
		// Move all entities
		for (auto& pEntity : m_apSimulateList)
		{
			if (!pEntity->m_pGameEntity)
				continue;

			Vector vecVelocity = pEntity->m_vecVelocity;
			Matrix4x4 mNewPhysics, mOldPhysics;
			mNewPhysics = mOldPhysics = pEntity->m_pGameEntity->GetPhysicsTransform();

			Vector vecOrigin = mOldPhysics.GetTranslation() + vecVelocity * flSimulationFrameTime;
			mNewPhysics.SetTranslation(vecOrigin);

			CTraceResult tr;
			DetectCollisions(pEntity, mOldPhysics, mNewPhysics, tr);

			for (size_t i = 0; i < tr.m_aHits.size(); i++)
			{
				auto& oOther = tr.m_aHits[i];

				bool bCollide = true;

				if (pEntity->m_pGameEntity)
				{
					if (oOther.m_iHit != ~0)
						bCollide = pEntity->m_pGameEntity->ShouldCollideWith(oOther.m_iHit, oOther.m_vecHit);
					else if (oOther.m_iHitExtra != ~0)
						bCollide = pEntity->m_pGameEntity->ShouldCollideWithExtra(oOther.m_iHitExtra, oOther.m_vecHit);
				}

				if (bCollide)
				{
					if (pEntity->m_pGameEntity)
					{
						if (oOther.m_iHit != ~0)
							pEntity->m_pGameEntity->Touching(oOther.m_iHit);
						else if (oOther.m_iHitExtra != ~0)
							pEntity->m_pGameEntity->TouchingExtra(oOther.m_iHitExtra);
					}

					mNewPhysics.SetTranslation(LerpValue<Vector>(mOldPhysics.GetTranslation(), mNewPhysics.GetTranslation(), oOther.m_flFraction - 0.001f));

					break;
				}
			}

			if (pEntity->m_pGameEntity)
				pEntity->m_pGameEntity->SetPhysicsTransform(mNewPhysics);

			pEntity->m_vecVelocity = vecVelocity + pEntity->m_vecGravity * flSimulationFrameTime;
		}

		while (m_flSimulationTime < m_flGameTime)
			m_flSimulationTime += flSimulationFrameTime;
	}

	TStubbed("Trigger collision");
#if 0
	for (auto& pEntity : m_apSimulateList)
	{
		if (!pEntity->m_pGameEntity)
			continue;

		for (auto& pOther : m_apSimulateList)
		{
			if (!pOther->m_pGameEntity)
				continue;

			if (!pEntity->m_pGameEntity->ShouldCollideWith(pOther->m_pGameEntity->GetHandle(), pOther->m_pGameEntity->GetPhysicsTransform().GetTranslation()))
				continue;

			Vector vecPoint;
			if (pEntity->m_pGameEntity->IsTouching(pEntity2, vecPoint))
			{
				pEntity->SetOrigin(vecPoint);
				pEntity->Touching(pEntity2);
			}
		}
	}
#endif
}

void CTPhysics::DetectCollisions(CPhysicsEntity* pEntity, const Matrix4x4& mOld, const Matrix4x4& mNew, CTraceResult& tr)
{
	if (pEntity->m_pMesh)
		TUnimplemented();

	TraceLine(tr, mOld.GetTranslation(), mNew.GetTranslation());
}

void CTPhysics::SetEntityCollisionDisabled(IPhysicsEntity* pEnt, bool bDisabled)
{
	CPhysicsEntity* pEntity = GetPhysicsEntity(pEnt);

	if (pEntity)
		pEntity->m_bActive = !bDisabled;
}

void CTPhysics::SetEntityVelocity(IPhysicsEntity* pEnt, const Vector& vecVelocity)
{
	CPhysicsEntity* pEntity = GetPhysicsEntity(pEnt);

	if (pEntity)
		pEntity->m_vecVelocity = vecVelocity;
}

void CTPhysics::SetEntityGravity(IPhysicsEntity* pEnt, const Vector& vecGravity)
{
	CPhysicsEntity* pEntity = GetPhysicsEntity(pEnt);

	if (pEntity)
		pEntity->m_vecGravity = vecGravity;
}

void CTPhysics::SetEntityTransform(IPhysicsEntity* pEnt, const Matrix4x4& mTransform)
{
	// No op, we always use pEnt->GetPhysicsTransform() as the current transform.
}

Vector CTPhysics::GetEntityVelocity(IPhysicsEntity* pEnt)
{
	CPhysicsEntity* pEntity = GetPhysicsEntity(pEnt);

	if (pEntity)
		return pEntity->m_vecVelocity;

	TAssert(false);
	return Vector();
}

CPhysicsEntity* CTPhysics::GetPhysicsEntity(IPhysicsEntity* pEnt)
{
	TAssert(pEnt);
	if (!pEnt)
		return NULL;

	size_t iHandle = pEnt->GetHandle();
	TAssert(m_aEntityList.size() > iHandle);
	if (m_aEntityList.size() <= iHandle)
		return NULL;

	CPhysicsEntity* pPhysicsEntity = &m_aEntityList[iHandle];
	TAssert(pPhysicsEntity);

	return pPhysicsEntity;
}

void CTPhysics::TraceLine(CTraceResult& tr, const Vector& v1, const Vector& v2, collision_group_t eCollisions, IPhysicsEntity* pIgnore)
{
	for (auto& oEnt : m_aEntityList)
	{
		if (!oEnt.m_pGameEntity)
			continue;

		if (!oEnt.m_bActive)
			continue;

		// Really this should be removed and it done with eCollisions.
		if (oEnt.m_eCollisionType == CT_DYNAMIC || oEnt.m_eCollisionType == CT_NONE)
			continue;

		TUnimplemented();

		if (oEnt.m_pGameEntity == pIgnore)
			continue;

		// Not correct but you get the idea.
		//if (!(oEnt.m_eCollisionType&eCollisions))
		//	continue;
	}

	for (size_t i = 0; i < m_apExtraEntityList.size(); i++)
	{
		auto& pExtraEnt = m_apExtraEntityList[i];
		if (!pExtraEnt)
			continue;

		if (!pExtraEnt->m_bActive)
			continue;

		if (!pExtraEnt->m_pMesh)
			continue;

		if (pExtraEnt->m_eCollisionType == CT_DYNAMIC || pExtraEnt->m_eCollisionType == CT_NONE)
			continue;

		pExtraEnt->m_pMesh->TraceLine(i, tr, v1, v2);
	}
}

void CTPhysics::DebugDraw(physics_debug_t iLevel)
{
	if (iLevel == PHYS_DEBUG_NONE)
		return;

	CRenderingContext c(GameServer()->GetRenderer(), true);
	c.UseProgram("debug");
	c.SetUniform("vecColor", Color(1.0f, 1.0f, 1.0f, 1.0f));

	for (auto& oEnt : m_aEntityList)
	{
		if (!oEnt.m_pGameEntity)
			continue;

		if (!oEnt.m_bActive)
			continue;

		oEnt.DebugDraw(iLevel);
	}

	for (auto& pEnt : m_apExtraEntityList)
	{
		if (!pEnt)
			continue;

		if (!pEnt->m_bActive)
			continue;

		pEnt->DebugDraw(iLevel);
	}
}

void CPhysicsEntity::DebugDraw(physics_debug_t iLevel)
{
	if (m_pMesh)
		m_pMesh->DebugDraw(m_pGameEntity?m_pGameEntity->GetPhysicsTransform():Matrix4x4(), iLevel);
	else
		TUnimplemented();
}

CPhysicsModel* CreatePhysicsModel()
{
	return new CTPhysics();
}

