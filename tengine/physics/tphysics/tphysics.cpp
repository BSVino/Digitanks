#include "tphysics.h"

#include <tinker/profiler.h>
#include <tinker/cvar.h>
#include <game/gameserver.h>

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

	// Move all entities
	for (auto& pEntity : m_apSimulateList)
	{
		// Break simulations up into very small steps in order to preserve accuracy.
		// I think floating point precision causes this problem but I'm not sure. Anyway this works better for my projectiles.
		for (double flCurrentSimulationTime = m_flSimulationTime; flCurrentSimulationTime < m_flGameTime; flCurrentSimulationTime += flSimulationFrameTime)
		{
			Vector vecVelocity = pEntity->m_vecVelocity;
			Matrix4x4 mPhysics = pEntity->m_pGameEntity->GetPhysicsTransform();

			Vector vecOrigin = mPhysics.GetTranslation() + vecVelocity * flSimulationFrameTime;
			mPhysics.SetTranslation(vecOrigin);
			pEntity->m_pGameEntity->SetPhysicsTransform(mPhysics);

			pEntity->m_vecVelocity = vecVelocity + pEntity->m_vecGravity * flSimulationFrameTime;
		}
	}

	while (m_flSimulationTime < m_flGameTime)
		m_flSimulationTime += flSimulationFrameTime;

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

CPhysicsModel* CreatePhysicsModel()
{
	return new CTPhysics();
}

