#include "game.h"

#include <ui/digitankswindow.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <renderer/dissolver.h>
#include <sound/sound.h>
#include <network/network.h>

#include "camera.h"

CGameServer* CGameServer::s_pGameServer = NULL;

CGameServer::CGameServer()
{
	assert(!s_pGameServer);
	s_pGameServer = this;

	m_bLoading = true;

	m_flRealTime = 0;
	m_flGameTime = 0;
	m_flSimulationTime = 0;
	m_flFrameTime = 0;

	for (size_t i = 0; i < CBaseEntity::s_aEntityRegistration.size(); i++)
		CBaseEntity::s_aEntityRegistration[i].m_pfnRegisterCallback();

	m_iClient = -1;

#ifdef _DEBUG
	CParticleSystemLibrary::Get()->LoadParticleSystem(0);
#endif
}

CGameServer::~CGameServer()
{
	delete m_pRenderer;
	delete m_pCamera;
	assert(s_pGameServer == this);
	s_pGameServer = NULL;
}

void CGameServer::Initialize()
{
	if (CNetwork::IsHost())
		m_hGame = CreateGame();

	m_pRenderer = CreateRenderer();
	m_pCamera = CreateCamera();

	RegisterNetworkFunctions();
}

void CGameServer::RegisterNetworkFunctions()
{
	CNetwork::RegisterFunction("UV", this, UpdateValueCallback, 2, NET_HANDLE, NET_HANDLE);

	CNetwork::RegisterFunction("ClientInfo", this, ClientInfoCallback, 2, NET_INT, NET_FLOAT);
	CNetwork::RegisterFunction("CreateEntity", this, CreateEntityCallback, 3, NET_INT, NET_HANDLE, NET_INT);
	CNetwork::RegisterFunction("DestroyEntity", this, DestroyEntityCallback, 1, NET_INT);
	CNetwork::RegisterFunction("LoadingDone", this, LoadingDoneCallback, 0);
}

void CGameServer::ClientConnect(CNetworkParameters* p)
{
	CNetwork::CallFunction(p->i2, "ClientInfo", p->i2, GetGameTime());

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		CNetwork::CallFunction(p->i2, "CreateEntity", CBaseEntity::FindRegisteredEntity(pEntity->GetClassName()), pEntity->GetHandle(), pEntity->GetSpawnSeed());
	}

	CNetwork::UpdateNetworkVariables(p->i2, true);

	GetGame()->OnClientConnect(p);

	// Update entities after all creations have been run, so we don't refer to entities that haven't been created yet.
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		pEntity->ClientUpdate(p->i2);
	}

	CNetwork::CallFunction(p->i2, "LoadingDone");

	CNetwork::CallFunction(p->i2, "EnterGame");
}

void CGameServer::LoadingDone(CNetworkParameters* p)
{
	m_bLoading = false;
}

void CGameServer::ClientDisconnect(CNetworkParameters* p)
{
	GetGame()->OnClientDisconnect(p);
}

void CGameServer::Think(float flRealTime)
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

void CGameServer::Simulate()
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

void CGameServer::Render()
{
	m_pCamera->Think();

	m_pRenderer->SetCameraPosition(m_pCamera->GetCameraPosition());
	m_pRenderer->SetCameraTarget(m_pCamera->GetCameraTarget());

	m_pRenderer->SetupFrame();
	m_pRenderer->DrawBackground();
	m_pRenderer->StartRendering();

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (pEntity->ShouldRender())
			pEntity->Render();
	}

	CParticleSystemLibrary::Render();
	CModelDissolver::Render();

	m_pRenderer->FinishRendering();
}

CEntityHandle<CBaseEntity> CGameServer::Create(const char* pszEntityName)
{
	assert(CNetwork::IsHost());

	if (!CNetwork::ShouldRunClientFunction())
		return CEntityHandle<CBaseEntity>();

	size_t iRegisteredEntity = CBaseEntity::FindRegisteredEntity(pszEntityName);

	if (iRegisteredEntity == ~0)
		return CEntityHandle<CBaseEntity>();

	CEntityHandle<CBaseEntity> hEntity(CreateEntity(iRegisteredEntity));

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "CreateEntity", iRegisteredEntity, hEntity->GetHandle(), hEntity->GetSpawnSeed());

	return hEntity;
}

size_t CGameServer::CreateEntity(size_t iRegisteredEntity, size_t iHandle, size_t iSpawnSeed)
{
	CBaseEntity::s_iOverrideEntityListIndex = iHandle;
	iHandle = CBaseEntity::s_aEntityRegistration[iRegisteredEntity].m_pfnCreateCallback();
	CBaseEntity::s_iOverrideEntityListIndex = ~0;

	CEntityHandle<CBaseEntity> hEntity(iHandle);

	if (iSpawnSeed)
		hEntity->SetSpawnSeed(iSpawnSeed);
	else
		hEntity->SetSpawnSeed(rand()%99999);	// Don't pick a number so large that it can't fit in (int)

	hEntity->RegisterNetworkVariables();

	hEntity->Spawn();

	if (dynamic_cast<CGame*>(hEntity.GetPointer()))
		m_hGame = CEntityHandle<CGame>(hEntity->GetHandle());

	return iHandle;
}

void CGameServer::Delete(CBaseEntity* pEntity)
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "DestroyEntity", pEntity->GetHandle());

	CNetworkParameters p;
	p.i1 = (int)pEntity->GetHandle();
	DestroyEntity(&p);
}

void CGameServer::CreateEntity(CNetworkParameters* p)
{
	if (CBaseEntity::s_aEntityRegistration.size() <= (size_t)p->i1)
		return;

	CreateEntity(p->i1, p->ui2, p->i3);
}

void CGameServer::DestroyEntity(CNetworkParameters* p)
{
	CBaseEntity* pEntity = CBaseEntity::GetEntity(p->i1);

	CSoundLibrary::EntityDeleted(pEntity);

	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		if (m_ahDeletedEntities[i] == pEntity)
			return;

	pEntity->OnDeleted();

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pNotify = CBaseEntity::GetEntityNumber(i);
		pNotify->OnDeleted(pEntity);
	}

	pEntity->SetDeleted();
	m_ahDeletedEntities.push_back(pEntity);

	pEntity->DeregisterNetworkVariables();
}

void CGameServer::UpdateValue(CNetworkParameters* p)
{
	CEntityHandle<CBaseEntity> hEntity(p->ui1);

	if (!hEntity)
		return;

	CNetworkedVariableBase* pVariable = hEntity->GetNetworkVariable((char*)p->m_pExtraData);

	if (!pVariable)
		return;

	pVariable->Unserialize((unsigned char*)p->m_pExtraData + strlen((char*)p->m_pExtraData)+1);
}

void CGameServer::ClientInfo(CNetworkParameters* p)
{
	m_iClient = p->i1;
	m_flGameTime = m_flSimulationTime = p->fl2;
}
