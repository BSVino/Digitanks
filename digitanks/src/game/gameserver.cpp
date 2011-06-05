#include "game.h"

#include <iostream>
#include <fstream>

#include <mtrand.h>
#include <platform.h>
#include <configfile.h>
#include <strutils.h>

#include <tinker/application.h>
#include <tinker/profiler.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <renderer/dissolver.h>
#include <sound/sound.h>
#include <network/network.h>
#include <network/commands.h>
#include <datamanager/data.h>
#include <datamanager/dataserializer.h>
#include <models/models.h>
#include <models/texturelibrary.h>
#include <tinker/portals/portal.h>
#include <tinker/lobby/lobby_server.h>
#include <tinker/cvar.h>

#include "camera.h"
#include "level.h"

CGameServer* CGameServer::s_pGameServer = NULL;

ConfigFile g_cfgEngine(L"scripts/engine.cfg");

CGameServer::CGameServer()
{
	TAssert(!s_pGameServer);
	s_pGameServer = this;

	m_iMaxEnts = g_cfgEngine.read(L"MaxEnts", 1024);

	CBaseEntity::s_apEntityList.resize(m_iMaxEnts);

	m_pRenderer = NULL;
	m_pCamera = NULL;

	m_iSaveCRC = 0;

	m_bLoading = true;

	m_flHostTime = 0;
	m_flGameTime = 0;
	m_flSimulationTime = 0;
	m_flFrameTime = 0;
	m_flNextClientInfoUpdate = 0;

	size_t iPostSeed = mtrand();

	for (size_t i = 0; i < CBaseEntity::GetEntityRegistration().size(); i++)
	{
		CEntityRegistration* pRegistration = &CBaseEntity::GetEntityRegistration()[i];

		if (pRegistration->m_pszParentClass)
		{
			bool bFound = false;
			for (size_t j = 0; j < CBaseEntity::GetEntityRegistration().size(); j++)
			{
				if (strcmp(CBaseEntity::GetEntityRegistration()[j].m_pszEntityName, pRegistration->m_pszParentClass) == 0)
				{
					bFound = true;
					pRegistration->m_iParentRegistration = j;
					break;
				}
			}

			TAssert(bFound);	// I have no idea how you could trip this.
		}
		else
			pRegistration->m_iParentRegistration = ~0;
	}

	TMsg(L"Precaching entities... ");
	for (size_t i = 0; i < CBaseEntity::GetEntityRegistration().size(); i++)
	{
		CEntityRegistration* pRegistration = &CBaseEntity::GetEntityRegistration()[i];
		pRegistration->m_pfnRegisterCallback();
	}
	TMsg(L"Done.\n");
	TMsg(sprintf(L"%d models, %d textures, %d sounds and %d particle systems precached.\n", CModelLibrary::GetNumModels(), CTextureLibrary::GetNumTextures(), CSoundLibrary::GetNumSounds(), CParticleSystemLibrary::GetNumParticleSystems()));

	mtsrand(iPostSeed);

	CBaseEntity::s_iNextEntityListIndex = 0;

	m_iPort = 0;
	m_iClient = -1;

	m_bHalting = false;
}

CGameServer::~CGameServer()
{
	for (size_t i = 0; i < m_apLevels.size(); i++)
		delete m_apLevels[i];

	if (m_pRenderer)
		delete m_pRenderer;

	if (m_pCamera)
		delete m_pCamera;

	TAssert(s_pGameServer == this);
	s_pGameServer = NULL;
}

CLIENT_COMMAND(SendNickname)
{
	TAssert(GameServer());
	if (!GameServer())
		return;

	GameServer()->SetClientNickname(iClient, sParameters);
}

void CGameServer::SetPlayerNickname(const eastl::string16& sNickname)
{
	m_sNickname = sNickname;

	if (CNetwork::IsHost() || m_bGotClientInfo)
		SendNickname.RunCommand(m_sNickname);
}

void CGameServer::Initialize()
{
	m_bGotClientInfo = false;
	m_bLoading = true;

	TMsg(L"Initializing game server\n");

	ReadLevels();

	CNetwork::ClearRegisteredFunctions();

	RegisterNetworkFunctions();

	DestroyAllEntities(eastl::vector<eastl::string>(), true);

	CParticleSystemLibrary::ClearInstances();

	if (!m_pRenderer)
	{
		m_pRenderer = CreateRenderer();
		m_pRenderer->Initialize();
	}

	if (!m_pCamera)
		m_pCamera = CreateCamera();

	CNetwork::ResumePumping();
}

void CGameServer::ReadLevels()
{
	for (size_t i = 0; i < m_apLevels.size(); i++)
		delete m_apLevels[i];

	m_apLevels.clear();

	ReadLevels(L"levels");

	TMsg(sprintf(L"Read %d levels from disk.\n", m_apLevels.size()));
}

void CGameServer::ReadLevels(eastl::string16 sDirectory)
{
	eastl::vector<eastl::string16> asFiles = ListDirectory(sDirectory);

	for (size_t i = 0; i < asFiles.size(); i++)
	{
		eastl::string16 sFile = sDirectory + L"\\" + asFiles[i];

		if (IsFile(sFile) && sFile.substr(sFile.length()-4).compare(L".txt") == 0)
			ReadLevel(sFile);

		if (IsDirectory(sFile))
			ReadLevels(sFile);
	}
}

void CGameServer::ReadLevel(eastl::string16 sFile)
{
	std::ifstream f(sFile.c_str());

	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	CLevel* pLevel = CreateLevel();
	pLevel->SetFile(str_replace(sFile, L"\\", L"/"));
	pLevel->ReadFromData(pData);
	m_apLevels.push_back(pLevel);

	delete pData;
}

CLevel* CGameServer::GetLevel(eastl::string16 sFile)
{
	sFile = str_replace(sFile, L"\\", L"/");
	for (size_t i = 0; i < m_apLevels.size(); i++)
	{
		CLevel* pLevel = m_apLevels[i];
		eastl::string16 sLevelFile = pLevel->GetFile();
		if (sLevelFile == sFile)
			return pLevel;
		if (sLevelFile == sFile + L".txt")
			return pLevel;
		if (sLevelFile == eastl::string16(L"levels/") + sFile)
			return pLevel;
		if (sLevelFile == eastl::string16(L"levels/") + sFile + L".txt")
			return pLevel;
	}

	return NULL;
}

void CGameServer::Halt()
{
	m_bHalting = true;

	CNetwork::SuspendPumping();
}

void CGameServer::RegisterNetworkFunctions()
{
	CNetwork::RegisterFunction("UV", this, UpdateValueCallback, 2, NET_HANDLE, NET_HANDLE);
	CNetwork::RegisterFunction("NC", this, NetworkCommandCallback, 0);

	CNetwork::RegisterFunction("ClientInfo", this, ClientInfoCallback, 2, NET_INT, NET_FLOAT);
	CNetwork::RegisterFunction("CreateEntity", this, CreateEntityCallback, 3, NET_INT, NET_HANDLE, NET_INT);
	CNetwork::RegisterFunction("DestroyEntity", this, DestroyEntityCallback, 1, NET_INT);
	CNetwork::RegisterFunction("LoadingDone", this, LoadingDoneCallback, 0);
}

void CGameServer::ClientConnect(CNetworkParameters* p)
{
	ClientConnect(p->i1);
}

void CGameServer::LoadingDone(CNetworkParameters* p)
{
	m_bLoading = false;
}

void CGameServer::ClientDisconnect(CNetworkParameters* p)
{
	ClientDisconnect(p->i1);
}

void CGameServer::ClientConnect(int iClient)
{
	CNetwork::CallFunction(iClient, "ClientInfo", iClient, GetGameTime());

	GetGame()->OnClientConnect(iClient);

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		CNetwork::CallFunction(iClient, "CreateEntity", CBaseEntity::FindRegisteredEntity(pEntity->GetClassName()), pEntity->GetHandle(), pEntity->GetSpawnSeed());
	}

	CGameServerNetwork::UpdateNetworkVariables(iClient, true);

	// Update entities after all creations have been run, so we don't refer to entities that haven't been created yet.
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		pEntity->ClientUpdate(iClient);
	}

	CNetwork::CallFunction(iClient, "LoadingDone");

	CNetwork::CallFunction(iClient, "EnterGame");
}

void CGameServer::ClientDisconnect(int iClient)
{
	CApplication::Get()->OnClientDisconnect(iClient);

	if (GetGame())
		GetGame()->OnClientDisconnect(iClient);
}

void CGameServer::SetClientNickname(int iClient, const eastl::string16& sNickname)
{
	if (iClient == GetClientIndex() && Game()->GetNumLocalTeams())
	{
		Game()->GetLocalTeam(0)->SetTeamName(sNickname);
		return;
	}

	for (size_t i = 0; i < Game()->GetNumTeams(); i++)
	{
		if (Game()->GetTeam(i)->GetClient() == iClient)
		{
			Game()->GetTeam(i)->SetTeamName(sNickname);
			return;
		}
	}

	TMsg(sprintf(L"Can't find client %d to give nickname %s.\n", iClient, sNickname));
}

void CGameServer::Think(float flHostTime)
{
	TPROF("CGameServer::Think");

	m_flFrameTime = flHostTime - m_flHostTime;

	// If the framerate drops, don't let too much happen without the player seeing
	if (CNetwork::IsConnected())
	{
		// But not as much in multiplayer games where we need to keep the game time synchronized
		if (m_flFrameTime > 1.0f)
			m_flFrameTime = 1.0f;
	}
	else
	{
		if (m_flFrameTime > 0.15f)
			m_flFrameTime = 0.15f;
	}

	m_flGameTime += m_flFrameTime;

	m_flHostTime = flHostTime;

	if (CNetwork::IsConnected() && CNetwork::IsHost() && !CGameLobbyServer::GetActiveLobbies() && m_flHostTime > m_flNextClientInfoUpdate)
	{
		m_flNextClientInfoUpdate = m_flHostTime + 5.0f;

		size_t iClientsConnected = CNetwork::GetClientsConnected();
		for (size_t i = 0; i < iClientsConnected; i++)
		{
			size_t iClient = CNetwork::GetClientConnectionId(i);
			if (iClient == ~0)
				continue;

			CNetwork::CallFunction(iClient, "ClientInfo", iClient, GetGameTime());
		}
	}

	// Erase anything deleted last frame.
	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		delete m_ahDeletedEntities[i];

	m_ahDeletedEntities.clear();

	CNetwork::Think();

	Simulate();

	size_t iMaxEntities = GameServer()->GetMaxEntities();
	for (size_t i = 0; i < iMaxEntities; i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		pEntity->Think();

		if (m_bHalting)
			break;
	}

	Think();

	if (CNetwork::IsHost())
		CGameServerNetwork::UpdateNetworkVariables(NETWORK_TOCLIENTS);

	if (CNetwork::IsHost())
	{
		for (size_t i = 0; i < iMaxEntities; i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
			if (!pEntity)
				continue;

			if (!pEntity->HasIssuedClientSpawn())
				pEntity->IssueClientSpawn();

			if (m_bHalting)
				break;
		}
	}

	CParticleSystemLibrary::Simulate();
	CModelDissolver::Simulate();

	TPortal_Think();
}

void CGameServer::Simulate()
{
	float flSimulationFrameTime = 0.01f;

	m_apSimulateList.reserve(CBaseEntity::GetNumEntities());
	m_apSimulateList.clear();

	size_t iMaxEntities = GetMaxEntities();
	for (size_t i = 0; i < iMaxEntities; i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		pEntity->SetLastOrigin(pEntity->GetOrigin());

		if (!pEntity->ShouldSimulate())
			continue;

		m_apSimulateList.push_back(pEntity);
	}

	// Move all entities
	for (size_t i = 0; i < m_apSimulateList.size(); i++)
	{
		CBaseEntity* pEntity = m_apSimulateList[i];
		if (!pEntity)
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

	for (size_t i = 0; i < m_apSimulateList.size(); i++)
	{
		CBaseEntity* pEntity = m_apSimulateList[i];
		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		for (size_t j = 0; j < iMaxEntities; j++)
		{
			CBaseEntity* pEntity2 = CBaseEntity::GetEntity(j);

			if (!pEntity2)
				continue;

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

CVar r_cullfrustum("r_frustumculling", "on");

void CGameServer::Render()
{
	TPROF("CGameServer::Render");

	if (!m_pCamera)
		return;

	m_pCamera->Think();

	m_pRenderer->SetCameraPosition(m_pCamera->GetCameraPosition());
	m_pRenderer->SetCameraTarget(m_pCamera->GetCameraTarget());
	m_pRenderer->SetCameraFOV(m_pCamera->GetCameraFOV());
	m_pRenderer->SetCameraNear(m_pCamera->GetCameraNear());
	m_pRenderer->SetCameraFar(m_pCamera->GetCameraFar());

	m_pRenderer->SetupFrame();
	m_pRenderer->DrawBackground();
	m_pRenderer->StartRendering();

	m_apRenderList.reserve(CBaseEntity::GetNumEntities());
	m_apRenderList.clear();

	bool bFrustumCulling = r_cullfrustum.GetBool();

	// None of these had better get deleted while we're doing this since they're not handles.
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		if (!pEntity->ShouldRender())
			continue;

		if (bFrustumCulling && !m_pRenderer->IsSphereInFrustum(pEntity->GetRenderOrigin(), pEntity->GetRenderRadius()))
			continue;

		m_apRenderList.push_back(pEntity);
	}

	m_pRenderer->BeginBatching();

	// First render all opaque objects
	size_t iEntites = m_apRenderList.size();
	for (size_t i = 0; i < iEntites; i++)
		m_apRenderList[i]->Render(false);

	m_pRenderer->RenderBatches();

	// Now render all transparent objects. Should really sort this back to front but meh for now.
	for (size_t i = 0; i < iEntites; i++)
		m_apRenderList[i]->Render(true);

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
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

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

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		if (!CBaseEntity::GetEntity(i))
			continue;

		CBaseEntity::GetEntity(i)->ClientEnterGame();
	}

	GameServer()->SetLoading(false);

	Game()->EnterGame();

	return true;
}

CEntityHandle<CBaseEntity> CGameServer::Create(const char* pszEntityName)
{
	TAssert(CNetwork::IsHost());

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
	iHandle = CBaseEntity::GetEntityRegistration()[iRegisteredEntity].m_pfnCreateCallback();
	CBaseEntity::s_iOverrideEntityListIndex = ~0;

	CEntityHandle<CBaseEntity> hEntity(iHandle);
	hEntity->m_iRegistration = iRegisteredEntity;

	size_t iPostSeed = mtrand();

	if (iSpawnSeed)
		hEntity->SetSpawnSeed(iSpawnSeed);
	else
		hEntity->SetSpawnSeed(mtrand()%99999);	// Don't pick a number so large that it can't fit in (int)

	hEntity->SetSpawnTime(GameServer()->GetGameTime());

	hEntity->Spawn();

	mtsrand(iPostSeed);

	if (dynamic_cast<CGame*>(hEntity.GetPointer()))
		m_hGame = CEntityHandle<CGame>(hEntity->GetHandle());

	return iHandle;
}

void CGameServer::Delete(CBaseEntity* pEntity)
{
	TAssert(CNetwork::IsHost() || IsLoading());

	if (CNetwork::IsHost())
		CNetwork::CallFunction(NETWORK_TOCLIENTS, "DestroyEntity", pEntity->GetHandle());

	CNetworkParameters p;
	p.i1 = (int)pEntity->GetHandle();
	DestroyEntity(&p);
}

void CGameServer::CreateEntity(CNetworkParameters* p)
{
	if (CBaseEntity::GetEntityRegistration().size() <= (size_t)p->i1)
		return;

	CreateEntity(p->i1, p->ui2, p->i3);
}

void CGameServer::DestroyEntity(CNetworkParameters* p)
{
	CBaseEntity* pEntity = CBaseEntity::GetEntity(p->i1);

	if (!pEntity)
		return;

	CSoundLibrary::EntityDeleted(pEntity);

	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		if (m_ahDeletedEntities[i] == pEntity)
			return;

	pEntity->OnDeleted();

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pNotify = CBaseEntity::GetEntity(i);

		if (!pNotify)
			continue;

		pNotify->OnDeleted(pEntity);
	}

	pEntity->SetDeleted();
	m_ahDeletedEntities.push_back(pEntity);
}

void CGameServer::DestroyAllEntities(const eastl::vector<eastl::string>& asSpare, bool bRemakeGame)
{
	if (!CNetwork::IsHost() && !IsLoading())
		return;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

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

	if (bRemakeGame && CNetwork::IsHost())
		m_hGame = CreateGame();
}

void CGameServer::UpdateValue(CNetworkParameters* p)
{
	CEntityHandle<CBaseEntity> hEntity(p->ui1);

	if (!hEntity)
		return;

	CNetworkedVariableData* pVarData = hEntity->GetNetworkVariable((char*)p->m_pExtraData);
	CNetworkedVariableBase* pVariable = pVarData->GetNetworkedVariableBase(hEntity);

	if (!pVariable)
		return;

	void* pDataStart = (unsigned char*)p->m_pExtraData + strlen((char*)p->m_pExtraData)+1;
	pVariable->Unserialize(p->m_iExtraDataSize - (size_t)strlen((char*)p->m_pExtraData) - 1, pDataStart);

	if (pVarData->m_pfnChanged)
		pVarData->m_pfnChanged(pVariable);
}

void CGameServer::NetworkCommand(CNetworkParameters* p)
{
	TAssert(sizeof(eastl::string16::value_type) == sizeof(char16_t));
	char16_t* pszData = (char16_t*)p->m_pExtraData;

	eastl::string16 sCommand(pszData);

	size_t iSpace = sCommand.find(L' ');

	eastl::string16 sName;
	eastl::string16 sParameters;
	if (eastl::string16::npos == iSpace)
	{
		sName = sCommand;
		sParameters = L"";
	}
	else
	{
		sName = sCommand.substr(0, iSpace);
		sParameters = sCommand.substr(iSpace+1);
	}

	CNetworkCommand* pCommand = CNetworkCommand::GetCommand(sName);

	if (!pCommand)
	{
		TMsg(sprintf(L"Network command '%s' unknown.\n", sName));
		return;
	}

	pCommand->RunCallback(p->ui1, sParameters);
}

void CGameServer::ClientInfo(CNetworkParameters* p)
{
	m_iClient = p->i1;
	float flNewGameTime = p->fl2;
	if (flNewGameTime - m_flGameTime > 0.1f)
		TMsg(sprintf(L"New game time from server %.1f different!\n", flNewGameTime - m_flGameTime));

	m_flGameTime = m_flSimulationTime = flNewGameTime;

	// Can't send any client commands until we've gotten the client info because we need m_iClient filled out properly.
	if (!m_bGotClientInfo)
	{
		CNetwork::SetRunningClientFunctions(false);
		SendNickname.RunCommand(m_sNickname);
	}

	m_bGotClientInfo = true;
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