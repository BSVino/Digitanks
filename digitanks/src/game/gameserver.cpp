#include "game.h"

#include <iostream>
#include <fstream>

#include <mtrand.h>

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

	m_pRenderer = NULL;
	m_pCamera = NULL;

	m_iSaveCRC = 0;

	m_bLoading = true;

	m_flRealTime = 0;
	m_flGameTime = 0;
	m_flSimulationTime = 0;
	m_flFrameTime = 0;

	size_t iPostSeed = mtrand();

	for (size_t i = 0; i < CBaseEntity::s_aEntityRegistration.size(); i++)
		CBaseEntity::s_aEntityRegistration[i].m_pfnRegisterCallback();

	mtsrand(iPostSeed);

	CBaseEntity::s_iNextEntityListIndex = 0;

	m_iClient = -1;

	m_bHalting = false;

#ifdef _DEBUG
	CParticleSystemLibrary::Get()->LoadParticleSystem(0);
#endif
}

CGameServer::~CGameServer()
{
	if (m_pRenderer)
		delete m_pRenderer;

	if (m_pCamera)
		delete m_pCamera;

	assert(s_pGameServer == this);
	s_pGameServer = NULL;
}

void CGameServer::Initialize()
{
	m_bLoading = true;

	CNetwork::ClearRegisteredFunctions();

	DestroyAllEntities(eastl::vector<eastl::string>(), true);

	CParticleSystemLibrary::ClearInstances();

	if (!m_pRenderer)
	{
		m_pRenderer = CreateRenderer();
		m_pRenderer->Initialize();
	}

	if (!m_pCamera)
		m_pCamera = CreateCamera();

	RegisterNetworkFunctions();

	int iPort = m_iPort;

	if (m_eServerType == SERVER_HOST)
		CNetwork::CreateHost(iPort);

	if (m_eServerType == SERVER_CLIENT)
	{
		CNetwork::ConnectToHost(m_sConnectHost.c_str(), iPort);
		if (!CNetwork::IsConnected())
			return;
	}
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

		if (m_bHalting)
			break;
	}

	Think();

	CParticleSystemLibrary::Simulate();
	CModelDissolver::Simulate();
}

void CGameServer::Simulate()
{
	float flSimulationFrameTime = 0.01f;

	eastl::vector<CEntityHandle<CBaseEntity> > ahEntities;
	ahEntities.reserve(CBaseEntity::GetNumEntities());
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		ahEntities.push_back(CBaseEntity::GetEntityNumber(i));

	for (size_t i = 0; i < ahEntities.size(); i++)
	{
		CBaseEntity* pEntity = ahEntities[i];
		if (!pEntity)
			continue;

		pEntity->SetLastOrigin(pEntity->GetOrigin());
	}

	// Move all entities
	for (size_t i = 0; i < ahEntities.size(); i++)
	{
		CBaseEntity* pEntity = ahEntities[i];
		if (!pEntity)
			continue;

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

	for (size_t i = 0; i < ahEntities.size(); i++)
	{
		CBaseEntity* pEntity = ahEntities[i];
		if (!pEntity)
			continue;

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
	m_pRenderer->SetCameraFOV(m_pCamera->GetCameraFOV());
	m_pRenderer->SetCameraNear(m_pCamera->GetCameraNear());
	m_pRenderer->SetCameraFar(m_pCamera->GetCameraFar());

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

void CGameServer::GenerateSaveCRC(size_t iInput)
{
	mtsrand(m_iSaveCRC^iInput);
	m_iSaveCRC = mtrand();
}

void CGameServer::SaveToFile(const wchar_t* pFileName)
{
	if (!GameServer())
		return;

	std::ofstream o;
	o.open(pFileName, std::ios_base::binary|std::ios_base::out);

	o.write("GameSave", 8);

	CGameServer* pGameServer = GameServer();

	o.write((char*)&pGameServer->m_iSaveCRC, sizeof(pGameServer->m_iSaveCRC));

	o.write((char*)&pGameServer->m_flGameTime, sizeof(pGameServer->m_flGameTime));
	o.write((char*)&pGameServer->m_flSimulationTime, sizeof(pGameServer->m_flSimulationTime));

	eastl::vector<CBaseEntity*> apSaveEntities;
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (pEntity->IsDeleted())
			continue;

		apSaveEntities.push_back(pEntity);
	}

	size_t iEntities = apSaveEntities.size();
	o.write((char*)&iEntities, sizeof(iEntities));

	for (size_t i = 0; i < apSaveEntities.size(); i++)
	{
		CBaseEntity::SerializeEntity(o, apSaveEntities[i]);
	}
}

bool CGameServer::LoadFromFile(const wchar_t* pFileName)
{
	if (!GameServer())
		return false;

	DigitanksWindow()->RenderLoading();

	GameServer()->Initialize();

	// Erase all existing entites. We're going to load in new ones!
	GameServer()->DestroyAllEntities();

	std::ifstream i;
	i.open(pFileName, std::ios_base::binary|std::ios_base::in);

	char szTag[8];
	i.read(szTag, 8);
	if (strncmp(szTag, "GameSave", 8) != 0)
		return false;

	CGameServer* pGameServer = GameServer();

	size_t iLoadCRC;
	i.read((char*)&iLoadCRC, sizeof(iLoadCRC));

	if (iLoadCRC != pGameServer->m_iSaveCRC)
		return false;

	i.read((char*)&pGameServer->m_flGameTime, sizeof(pGameServer->m_flGameTime));
	i.read((char*)&pGameServer->m_flSimulationTime, sizeof(pGameServer->m_flSimulationTime));

	size_t iEntities;
	i.read((char*)&iEntities, sizeof(iEntities));

	for (size_t j = 0; j < iEntities; j++)
	{
		if (!CBaseEntity::UnserializeEntity(i))
			return false;
	}

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		CBaseEntity::GetEntityNumber(i)->ClientEnterGame();

	GameServer()->SetLoading(false);

	return true;
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

	hEntity->RegisterNetworkVariables();

	size_t iPostSeed = mtrand();

	if (iSpawnSeed)
		hEntity->SetSpawnSeed(iSpawnSeed);
	else
		hEntity->SetSpawnSeed(mtrand()%99999);	// Don't pick a number so large that it can't fit in (int)

	hEntity->Spawn();

	mtsrand(iPostSeed);

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

void CGameServer::DestroyAllEntities(const eastl::vector<eastl::string>& asSpare, bool bRemakeGame)
{
	if (!CNetwork::IsHost() && !m_bLoading)
		return;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);

		bool bSpare = false;
		for (size_t j = 0; j < asSpare.size(); j++)
		{
			if (asSpare[j] == pEntity->GetClassName())
			{
				bSpare = true;
				break;
			}
		}

		if (bSpare)
			continue;

		pEntity->Delete();
	}

	for (size_t i = 0; i < GameServer()->m_ahDeletedEntities.size(); i++)
		delete GameServer()->m_ahDeletedEntities[i];

	GameServer()->m_ahDeletedEntities.clear();

	if (CBaseEntity::GetNumEntities() == 0)
		CBaseEntity::s_iNextEntityListIndex = 0;

	if (bRemakeGame)
		m_hGame = CreateGame();
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

	if (pVariable->m_pfnChanged)
		pVariable->m_pfnChanged(pVariable);
}

void CGameServer::ClientInfo(CNetworkParameters* p)
{
	m_iClient = p->i1;
	m_flGameTime = m_flSimulationTime = p->fl2;
}

CRenderer* CGameServer::GetRenderer()
{
	if (m_pRenderer)
		return m_pRenderer;

	m_pRenderer = CreateRenderer();

	return m_pRenderer;
}

CGame* CGameServer::GetGame()
{
	return m_hGame;
}