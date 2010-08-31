#include "game.h"

#include <ui/digitankswindow.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <renderer/dissolver.h>
#include <sound/sound.h>
#include <network/network.h>

#include "camera.h"

CGame* CGame::s_pGame = NULL;
CEntityHandle<CTeam> CGame::s_hLocalTeam;

CGame::CGame()
{
	assert(!s_pGame);
	s_pGame = this;

	m_bLoading = true;

	m_flRealTime = 0;
	m_flGameTime = 0;
	m_flSimulationTime = 0;
	m_flFrameTime = 0;

	for (size_t i = 0; i < CBaseEntity::s_aEntityRegistration.size(); i++)
		CBaseEntity::s_aEntityRegistration[i].m_pfnRegisterCallback();

	m_pCamera = new CCamera();
	m_pCamera->SnapDistance(120);

	m_iClient = -1;

#ifdef _DEBUG
	CParticleSystemLibrary::Get()->LoadParticleSystem(0);
#endif
}

CGame::~CGame()
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
		m_ahTeams[i]->Delete();

	delete m_pRenderer;
	delete m_pCamera;
	assert(s_pGame == this);
	s_pGame = NULL;
}

void CGame::RegisterNetworkFunctions()
{
	CNetwork::RegisterFunction("ClientInfo", this, ClientInfoCallback, 2, NET_INT, NET_FLOAT);
	CNetwork::RegisterFunction("CreateEntity", this, CreateEntityCallback, 3, NET_INT, NET_HANDLE, NET_INT);
	CNetwork::RegisterFunction("DestroyEntity", this, DestroyEntityCallback, 1, NET_INT);
	CNetwork::RegisterFunction("LoadingDone", this, LoadingDoneCallback, 0);
	CNetwork::RegisterFunction("SetOrigin", this, SetOriginCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("SetAngles", this, SetAnglesCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("AddTeam", this, AddTeamCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("SetTeamColor", this, SetTeamColorCallback, 4, NET_HANDLE, NET_INT, NET_INT, NET_INT);
	CNetwork::RegisterFunction("SetTeamClient", this, SetTeamClientCallback, 2, NET_HANDLE, NET_INT);
	CNetwork::RegisterFunction("AddEntityToTeam", this, AddEntityToTeamCallback, 2, NET_HANDLE, NET_HANDLE);
}

void CGame::ClientConnect(CNetworkParameters* p)
{
	CNetwork::CallFunction(p->i2, "ClientInfo", p->i2, GetGameTime());

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		CNetwork::CallFunction(p->i2, "CreateEntity", CBaseEntity::FindRegisteredEntity(pEntity->GetClassName()), pEntity->GetHandle(), pEntity->GetSpawnSeed());
	}

	OnClientConnect(p);

	// Update entities after all creations have been run, so we don't refer to entities that haven't been created yet.
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		pEntity->ClientUpdate(p->i2);
	}

	OnClientUpdate(p);

	CNetwork::CallFunction(p->i2, "LoadingDone");
}

void CGame::LoadingDone(CNetworkParameters* p)
{
	m_bLoading = false;
}

void CGame::ClientDisconnect(CNetworkParameters* p)
{
	OnClientDisconnect(p);
}

void CGame::Think(float flRealTime)
{
	m_flFrameTime = flRealTime - m_flRealTime;

	if (m_flFrameTime > 0.15f)
		m_flFrameTime = 0.15f;

	m_flGameTime += m_flFrameTime;
	m_flRealTime = flRealTime;

	// Erase anything deleted last frame.
	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		delete m_ahDeletedEntities[i];

	m_ahDeletedEntities.clear();

	CNetwork::Think();

	Simulate();

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		pEntity->Think();
	}

	Think();

	CParticleSystemLibrary::Simulate();
	CModelDissolver::Simulate();
}

void CGame::Simulate()
{
	float flSimulationFrameTime = 0.01f;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i));

		pEntity->SetLastOrigin(pEntity->GetOrigin());
	}

	// Move all entities
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i));

		if (!pEntity->ShouldSimulate())
			continue;

		// Break simulations up into very small steps in order to preserve accuracy.
		// I think floating point precision causes this problem but I'm not sure. Anyway this works better for my projectiles.
		for (float flCurrentSimulationTime = m_flSimulationTime; flCurrentSimulationTime < m_flGameTime; flCurrentSimulationTime += flSimulationFrameTime)
		{
			Vector vecVelocity = pEntity->GetVelocity();
			pEntity->SetOrigin(pEntity->GetOrigin() + vecVelocity * flSimulationFrameTime);
			pEntity->SetVelocity(vecVelocity + pEntity->GetGravity() * flSimulationFrameTime);
		}
	}

	while (m_flSimulationTime < m_flGameTime)
		m_flSimulationTime += flSimulationFrameTime;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);

		if (pEntity->IsDeleted())
			continue;

		for (size_t j = 0; j < CBaseEntity::GetNumEntities(); j++)
		{
			CBaseEntity* pEntity2 = CBaseEntity::GetEntityNumber(j);

			if (pEntity2->IsDeleted())
				continue;

			if (!pEntity->ShouldTouch(pEntity2))
				continue;

			Vector vecPoint;
			if (pEntity->IsTouching(pEntity2, vecPoint))
			{
				pEntity->SetOrigin(vecPoint);
				pEntity->Touching(pEntity2);
			}
		}
	}
}

void CGame::Render()
{
	m_pCamera->Think();

	m_pRenderer->SetCameraPosition(m_pCamera->GetCameraPosition());
	m_pRenderer->SetCameraTarget(m_pCamera->GetCameraTarget());

	m_pRenderer->SetupFrame();
	m_pRenderer->DrawBackground();
	m_pRenderer->StartRendering();

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		CBaseEntity::GetEntityNumber(i)->Render();

	CParticleSystemLibrary::Render();
	CModelDissolver::Render();

	m_pRenderer->FinishRendering();
}

void CGame::OnDeleted(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pNotify = CBaseEntity::GetEntityNumber(i);
		pNotify->OnDeleted(pEntity);
	}
}

CEntityHandle<CBaseEntity> CGame::Create(const char* pszEntityName)
{
	if (!CNetwork::ShouldRunClientFunction())
		return CEntityHandle<CBaseEntity>();

	size_t iRegisteredEntity = CBaseEntity::FindRegisteredEntity(pszEntityName);

	if (iRegisteredEntity == ~0)
		return CEntityHandle<CBaseEntity>();

	CEntityHandle<CBaseEntity> hEntity(CreateEntity(iRegisteredEntity));

	CNetwork::CallFunction(-1, "CreateEntity", iRegisteredEntity, hEntity->GetHandle(), hEntity->GetSpawnSeed());

	return hEntity;
}

size_t CGame::CreateEntity(size_t iRegisteredEntity, size_t iHandle, size_t iSpawnSeed)
{
	CBaseEntity::s_iOverrideEntityListIndex = iHandle;
	iHandle = CBaseEntity::s_aEntityRegistration[iRegisteredEntity].m_pfnCreateCallback();
	CBaseEntity::s_iOverrideEntityListIndex = ~0;

	CEntityHandle<CBaseEntity> hEntity(iHandle);

	if (iSpawnSeed)
		hEntity->SetSpawnSeed(iSpawnSeed);
	else
		hEntity->SetSpawnSeed(rand()%99999);	// Don't pick a number so large that it can't fit in (int)

	hEntity->Spawn();
	return iHandle;
}

void CGame::Delete(CBaseEntity* pEntity)
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	CNetwork::CallFunction(-1, "DestroyEntity", pEntity->GetHandle());

	CNetworkParameters p;
	p.i1 = (int)pEntity->GetHandle();
	DestroyEntity(&p);
}

void CGame::CreateEntity(CNetworkParameters* p)
{
	if (CBaseEntity::s_aEntityRegistration.size() <= (size_t)p->i1)
		return;

	CreateEntity(p->i1, p->ui2, p->i3);
}

void CGame::DestroyEntity(CNetworkParameters* p)
{
	CBaseEntity* pEntity = CBaseEntity::GetEntity(p->i1);

	CSoundLibrary::EntityDeleted(pEntity);

	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		if (m_ahDeletedEntities[i] == pEntity)
			return;

	pEntity->OnDeleted();
	OnDeleted(pEntity);
	pEntity->SetDeleted();
	m_ahDeletedEntities.push_back(pEntity);
}

void CGame::ClientInfo(CNetworkParameters* p)
{
	m_iClient = p->i1;
	m_flGameTime = m_flSimulationTime = p->fl2;
	s_hLocalTeam = NULL;
}

void CGame::SetOrigin(CNetworkParameters* p)
{
	CEntityHandle<CBaseEntity> hEntity(p->ui1);

	if (hEntity != NULL)
		hEntity->SetOrigin(Vector(p->fl2, p->fl3, p->fl4));
}

void CGame::SetAngles(CNetworkParameters* p)
{
	CEntityHandle<CBaseEntity> hEntity(p->ui1);

	if (hEntity != NULL)
		hEntity->SetAngles(EAngle(p->fl2, p->fl3, p->fl4));
}

void CGame::AddTeam(CNetworkParameters* p)
{
	m_ahTeams.push_back(CEntityHandle<CTeam>(p->ui1));
}

void CGame::CreateRenderer()
{
	m_pRenderer = new CRenderer(CDigitanksWindow::Get()->GetWindowWidth(), CDigitanksWindow::Get()->GetWindowHeight());
}

bool CGame::TraceLine(const Vector& s1, const Vector& s2, Vector& vecHit, CBaseEntity** pHit)
{
	Vector vecClosest = s2;
	bool bHit = false;
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		Vector vecPoint;
		if (pEntity->Collide(s1, s2, vecPoint))
		{
			if (!bHit || (vecPoint - s1).LengthSqr() < (vecClosest - s1).LengthSqr())
			{
				vecClosest = vecPoint;
				bHit = true;
				if (pHit)
					*pHit = pEntity;
			}
		}
	}

	if (bHit)
		vecHit = vecClosest;

	return bHit;
}

bool CGame::IsTeamControlledByMe(CTeam* pTeam)
{
	if (!pTeam)
		return false;

	if (pTeam->IsPlayerControlled() && pTeam->GetClient() == m_iClient)
		return true;

	return false;
}

CTeam* CGame::GetLocalTeam()
{
	if (s_hLocalTeam == NULL || s_hLocalTeam->IsDeleted())
	{
		CGame* pGame = Game();
		for (size_t i = 0; i < pGame->m_ahTeams.size(); i++)
		{
			if (!pGame->m_ahTeams[i]->IsPlayerControlled())
				continue;

			if (pGame->m_ahTeams[i]->GetClient() == pGame->m_iClient)
			{
				s_hLocalTeam = pGame->m_ahTeams[i];
				break;
			}
		}
	}

	return s_hLocalTeam;
}
