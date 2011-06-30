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
#include <tinker/gamewindow.h>

#include "camera.h"
#include "level.h"

CGameServer* CGameServer::s_pGameServer = NULL;

ConfigFile g_cfgEngine(_T("scripts/engine.cfg"));

CGameServer::CGameServer(IWorkListener* pWorkListener)
{
	TAssert(!s_pGameServer);
	s_pGameServer = this;

	GameNetwork()->SetCallbacks(this, CGameServer::ClientConnectCallback, CGameServer::ClientEnterGameCallback, CGameServer::ClientDisconnectCallback);

	m_pWorkListener = pWorkListener;

	m_iMaxEnts = g_cfgEngine.read(_T("MaxEnts"), 1024);

	CBaseEntity::s_apEntityList.resize(m_iMaxEnts);

	m_pCamera = NULL;

	m_iSaveCRC = 0;

	m_bLoading = true;

	m_flHostTime = 0;
	m_flGameTime = 0;
	m_flSimulationTime = 0;
	m_flFrameTime = 0;
	m_flNextClientInfoUpdate = 0;

	size_t iPostSeed = mtrand();

	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	TMsg(_T("Precaching entities... "));

	if (m_pWorkListener)
		m_pWorkListener->SetAction(_T("Loading polygons"), CBaseEntity::GetEntityRegistration().size());

	size_t i = 0;
	for (eastl::map<tstring, CEntityRegistration>::iterator it = CBaseEntity::GetEntityRegistration().begin(); it != CBaseEntity::GetEntityRegistration().end(); it++)
	{
		CEntityRegistration* pRegistration = &it->second;
		pRegistration->m_pfnRegisterCallback();

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(++i);
	}
	TMsg(_T("Done.\n"));
	TMsg(sprintf(tstring("%d models, %d textures, %d sounds and %d particle systems precached.\n"), CModelLibrary::GetNumModels(), CTextureLibrary::GetNumTextures(), CSoundLibrary::GetNumSounds(), CParticleSystemLibrary::GetNumParticleSystems()));

	mtsrand(iPostSeed);

	CBaseEntity::s_iNextEntityListIndex = 0;

	m_iPort = 0;
	m_iClient = NETWORK_LOCAL;

	m_bHalting = false;

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

CGameServer::~CGameServer()
{
	GameNetwork()->SetCallbacks(NULL, NULL, NULL, NULL);

	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	if (m_pWorkListener)
		m_pWorkListener->SetAction(_T("Scrubbing database"), CBaseEntity::GetEntityRegistration().size());

	for (size_t i = 0; i < m_apLevels.size(); i++)
		delete m_apLevels[i];

	if (m_pCamera)
		delete m_pCamera;

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();

	TAssert(s_pGameServer == this);
	s_pGameServer = NULL;
}

CLIENT_GAME_COMMAND(SendNickname)
{
	TAssert(GameServer());
	if (!GameServer())
		return;

	GameServer()->SetClientNickname(iClient, sParameters);
}

void CGameServer::SetPlayerNickname(const tstring& sNickname)
{
	m_sNickname = sNickname;

	if (GameNetwork()->IsHost() || m_bGotClientInfo)
		SendNickname.RunCommand(m_sNickname);
}

void CGameServer::Initialize()
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	m_bGotClientInfo = false;
	m_bLoading = true;

	TMsg(_T("Initializing game server\n"));

	ReadLevels();

	GameNetwork()->ClearRegisteredFunctions();

	RegisterNetworkFunctions();

	DestroyAllEntities(eastl::vector<eastl::string>(), true);

	CParticleSystemLibrary::ClearInstances();

	if (!m_pCamera)
		m_pCamera = CreateCamera();

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();

	if (m_pWorkListener)
		m_pWorkListener->SetAction(_T("Pending network actions"), 0);
}

void CGameServer::ReadLevels()
{
	for (size_t i = 0; i < m_apLevels.size(); i++)
		delete m_apLevels[i];

	m_apLevels.clear();

	if (m_pWorkListener)
		m_pWorkListener->SetAction(_T("Reading meta-structures"), 0);

	ReadLevels(_T("levels"));

	TMsg(sprintf(tstring("Read %d levels from disk.\n"), m_apLevels.size()));
}

void CGameServer::ReadLevels(tstring sDirectory)
{
	eastl::vector<tstring> asFiles = ListDirectory(sDirectory);

	for (size_t i = 0; i < asFiles.size(); i++)
	{
		tstring sFile = sDirectory + _T("\\") + asFiles[i];

		if (IsFile(sFile) && sFile.substr(sFile.length()-4).compare(_T(".txt")) == 0)
			ReadLevel(sFile);

		if (IsDirectory(sFile))
			ReadLevels(sFile);
	}
}

void CGameServer::ReadLevel(tstring sFile)
{
	std::basic_ifstream<tchar> f(convertstring<tchar, char>(sFile).c_str());

	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	CLevel* pLevel = CreateLevel();
	pLevel->SetFile(str_replace(sFile, _T("\\"), _T("/")));
	pLevel->ReadFromData(pData);
	m_apLevels.push_back(pLevel);

	delete pData;
}

CLevel* CGameServer::GetLevel(tstring sFile)
{
	sFile = str_replace(sFile, _T("\\"), _T("/"));
	for (size_t i = 0; i < m_apLevels.size(); i++)
	{
		CLevel* pLevel = m_apLevels[i];
		tstring sLevelFile = pLevel->GetFile();
		if (sLevelFile == sFile)
			return pLevel;
		if (sLevelFile == sFile + _T(".txt"))
			return pLevel;
		if (sLevelFile == tstring(_T("levels/")) + sFile)
			return pLevel;
		if (sLevelFile == tstring(_T("levels/")) + sFile + _T(".txt"))
			return pLevel;
	}

	return NULL;
}

void CGameServer::Halt()
{
	m_bHalting = true;
}

void CGameServer::RegisterNetworkFunctions()
{
	GameNetwork()->RegisterFunction("UV", this, UpdateValueCallback, 2, NET_HANDLE, NET_HANDLE);

	GameNetwork()->RegisterFunction("ClientInfo", this, ClientInfoCallback, 2, NET_INT, NET_FLOAT);
	GameNetwork()->RegisterFunction("DestroyEntity", this, DestroyEntityCallback, 1, NET_INT);
	GameNetwork()->RegisterFunction("LoadingDone", this, LoadingDoneCallback, 0);
}

void CGameServer::ClientConnect(int iConnection, CNetworkParameters* p)
{
	TAssert(iConnection == CONNECTION_GAME);

	ClientConnect(p->i1);
}

void CGameServer::ClientEnterGame(int iConnection, CNetworkParameters* p)
{
	TAssert(iConnection == CONNECTION_GAME);

	ClientEnterGame(p->i1);
}

void CGameServer::LoadingDone(int iConnection, CNetworkParameters* p)
{
	TAssert(iConnection == CONNECTION_GAME);

	m_bLoading = false;
}

void CGameServer::ClientDisconnect(int iConnection, CNetworkParameters* p)
{
	TAssert(iConnection == CONNECTION_GAME);

	ClientDisconnect(p->i1);
}

void CGameServer::ClientConnect(int iClient)
{
	GameNetwork()->CallFunction(iClient, "ClientInfo", iClient, GetGameTime());

	if (GetGame())
		GetGame()->OnClientConnect(iClient);
}

SERVER_GAME_COMMAND(CreateEntity)
{
	if (pCmd->GetNumArguments() < 3)
	{
		TError("CreateEntity with too few arguments.");
		return;
	}

	GameServer()->CreateEntity(pCmd->Arg(0), pCmd->ArgAsUInt(1), pCmd->ArgAsUInt(2));
}

void CGameServer::ClientEnterGame(int iClient)
{
	TMsg(sprintf(tstring("Client %d (") + GameNetwork()->GetClientNickname(iClient) + _T(") entering game.\n"), iClient));

	if (GetGame())
		GetGame()->OnClientEnterGame(iClient);

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		::CreateEntity.RunCommand(sprintf(tstring("%s %d %d"), pEntity->GetClassName(), pEntity->GetHandle(), pEntity->GetSpawnSeed()), iClient);
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

	GameNetwork()->CallFunction(iClient, "EnterGame");

	GameNetwork()->CallFunction(iClient, "LoadingDone");
}

void CGameServer::ClientDisconnect(int iClient)
{
	if (!GameNetwork()->IsHost() && iClient == GameNetwork()->GetClientID())
	{
		TMsg(_T("Disconnected from server.\n"));
	}
	else
	{
		TMsg(sprintf(tstring("Client %d (") + GameNetwork()->GetClientNickname(iClient) + _T(") disconnected.\n"), iClient));

		CApplication::Get()->OnClientDisconnect(iClient);

		if (GetGame())
			GetGame()->OnClientDisconnect(iClient);
	}
}

void CGameServer::SetClientNickname(int iClient, const tstring& sNickname)
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

	TMsg(sprintf(tstring("Can't find client %d to give nickname %s.\n"), iClient, sNickname.c_str()));
}

void CGameServer::Think(float flHostTime)
{
	TPROF("CGameServer::Think");

	m_flFrameTime = flHostTime - m_flHostTime;

	// If the framerate drops, don't let too much happen without the player seeing
	if (GameNetwork()->IsConnected())
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

	if (GameNetwork()->IsConnected() && GameNetwork()->IsHost() && m_flHostTime > m_flNextClientInfoUpdate)
	{
		m_flNextClientInfoUpdate = m_flHostTime + 5.0f;

		size_t iClientsConnected = GameNetwork()->GetClientsConnected();
		for (size_t i = 0; i < iClientsConnected; i++)
		{
			size_t iClient = GameNetwork()->GetClientConnectionId(i);
			if (iClient == ~0)
				continue;

			GameNetwork()->CallFunction(iClient, "ClientInfo", iClient, GetGameTime());
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

	if (GameNetwork()->IsHost())
		CGameServerNetwork::UpdateNetworkVariables(NETWORK_TOCLIENTS);

	if (GameNetwork()->IsHost())
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

	GameWindow()->GetRenderer()->SetCameraPosition(m_pCamera->GetCameraPosition());
	GameWindow()->GetRenderer()->SetCameraTarget(m_pCamera->GetCameraTarget());
	GameWindow()->GetRenderer()->SetCameraFOV(m_pCamera->GetCameraFOV());
	GameWindow()->GetRenderer()->SetCameraNear(m_pCamera->GetCameraNear());
	GameWindow()->GetRenderer()->SetCameraFar(m_pCamera->GetCameraFar());

	GameWindow()->GetRenderer()->SetupFrame();
	GameWindow()->GetRenderer()->DrawBackground();
	GameWindow()->GetRenderer()->StartRendering();

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

		if (bFrustumCulling && !GameWindow()->GetRenderer()->IsSphereInFrustum(pEntity->GetRenderOrigin(), pEntity->GetRenderRadius()))
			continue;

		m_apRenderList.push_back(pEntity);
	}

	GameWindow()->GetRenderer()->BeginBatching();

	// First render all opaque objects
	size_t iEntites = m_apRenderList.size();
	for (size_t i = 0; i < iEntites; i++)
		m_apRenderList[i]->Render(false);

	GameWindow()->GetRenderer()->RenderBatches();

	// Now render all transparent objects. Should really sort this back to front but meh for now.
	for (size_t i = 0; i < iEntites; i++)
		m_apRenderList[i]->Render(true);

	CParticleSystemLibrary::Render();
	CModelDissolver::Render();

	GameWindow()->GetRenderer()->FinishRendering();
}

void CGameServer::GenerateSaveCRC(size_t iInput)
{
	mtsrand(m_iSaveCRC^iInput);
	m_iSaveCRC = mtrand();
}

void CGameServer::SaveToFile(const tchar* pFileName)
{
	if (!GameServer())
		return;

	std::ofstream o;
	o.open(convertstring<tchar, char>(pFileName).c_str(), std::ios_base::binary|std::ios_base::out);

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

bool CGameServer::LoadFromFile(const tchar* pFileName)
{
	if (!GameServer())
		return false;

	GameServer()->Initialize();

	// Erase all existing entites. We're going to load in new ones!
	GameServer()->DestroyAllEntities();

	std::ifstream i;
	i.open(convertstring<tchar, char>(pFileName).c_str(), std::ios_base::binary|std::ios_base::in);

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

	Game()->EnterGame();

	if (GameServer()->GetWorkListener())
		GameServer()->GetWorkListener()->SetAction(_T("Encountering resistance"), 0);

	GameServer()->SetLoading(false);

	return true;
}

CEntityHandle<CBaseEntity> CGameServer::Create(const char* pszEntityName)
{
	TAssert(GameNetwork()->IsHost());

	if (!GameNetwork()->ShouldRunClientFunction())
		return CEntityHandle<CBaseEntity>();

	CEntityHandle<CBaseEntity> hEntity(CreateEntity(pszEntityName));

	::CreateEntity.RunCommand(sprintf(tstring("%s %d %d"), pszEntityName, hEntity->GetHandle(), hEntity->GetSpawnSeed()));

	return hEntity;
}

size_t CGameServer::CreateEntity(const tstring& sClassName, size_t iHandle, size_t iSpawnSeed)
{
	if (CVar::GetCVarBool("net_debug"))
		TMsg(tstring("Creating entity: ") + sClassName + "\n");

	CBaseEntity::s_iOverrideEntityListIndex = iHandle;
	iHandle = CBaseEntity::GetEntityRegistration()[sClassName].m_pfnCreateCallback();
	CBaseEntity::s_iOverrideEntityListIndex = ~0;

	CEntityHandle<CBaseEntity> hEntity(iHandle);
	hEntity->m_sClassName = sClassName;

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
	TAssert(GameNetwork()->IsHost() || IsLoading());
	if (!(GameNetwork()->IsHost() || IsLoading()))
		TMsg("WARNING: CGameServer::Delete() when not host or not loading.\n");

	if (GameNetwork()->IsHost())
		GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "DestroyEntity", pEntity->GetHandle());

	CNetworkParameters p;
	p.i1 = (int)pEntity->GetHandle();
	DestroyEntity(CONNECTION_GAME, &p);
}

void CGameServer::DestroyEntity(int iConnection, CNetworkParameters* p)
{
	CBaseEntity* pEntity = CBaseEntity::GetEntity(p->i1);

	if (!pEntity)
		return;

	CSoundLibrary::EntityDeleted(pEntity);

	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		if (pEntity == m_ahDeletedEntities[i])
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
	if (!GameNetwork()->IsHost() && !IsLoading())
		return;

	if (m_pWorkListener)
		m_pWorkListener->SetAction(_T("Locating dead nodes"), GameServer()->GetMaxEntities());

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

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i);
	}

	if (m_pWorkListener)
		m_pWorkListener->SetAction(_T("Clearing buffers"), GameServer()->m_ahDeletedEntities.size());

	for (size_t i = 0; i < GameServer()->m_ahDeletedEntities.size(); i++)
	{
		delete GameServer()->m_ahDeletedEntities[i];

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i);
	}

	GameServer()->m_ahDeletedEntities.clear();

	if (CBaseEntity::GetNumEntities() == 0)
		CBaseEntity::s_iNextEntityListIndex = 0;

	if (bRemakeGame && GameNetwork()->IsHost())
		m_hGame = CreateGame();
}

void CGameServer::UpdateValue(int iConnection, CNetworkParameters* p)
{
	CEntityHandle<CBaseEntity> hEntity(p->ui1);

	if (!hEntity)
		return;

	CNetworkedVariableData* pVarData = hEntity->GetNetworkVariable((char*)p->m_pExtraData);

	if (!pVarData)
		return;

	CNetworkedVariableBase* pVariable = pVarData->GetNetworkedVariableBase(hEntity);

	if (!pVariable)
		return;

	void* pDataStart = (unsigned char*)p->m_pExtraData + strlen((char*)p->m_pExtraData)+1;
	pVariable->Unserialize(p->m_iExtraDataSize - (size_t)strlen((char*)p->m_pExtraData) - 1, pDataStart);

	if (pVarData->m_pfnChanged)
		pVarData->m_pfnChanged(pVariable);
}

void CGameServer::ClientInfo(int iConnection, CNetworkParameters* p)
{
	if (m_iClient != p->i1)
		CGame::ClearLocalTeams(NULL);

	m_iClient = p->i1;
	float flNewGameTime = p->fl2;
	if (flNewGameTime - m_flGameTime > 0.1f)
		TMsg(sprintf(tstring("New game time from server %.1f different!\n"), flNewGameTime - m_flGameTime));

	m_flGameTime = m_flSimulationTime = flNewGameTime;

	// Can't send any client commands until we've gotten the client info because we need m_iClient filled out properly.
	if (!m_bGotClientInfo)
	{
		GameNetwork()->SetRunningClientFunctions(false);
		SendNickname.RunCommand(m_sNickname);
	}

	m_bGotClientInfo = true;
}

CRenderer* CGameServer::GetRenderer()
{
	return GameWindow()->GetRenderer();
}

CGame* CGameServer::GetGame()
{
	return m_hGame;
}

void ShowStatus(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	TMsg(eastl::string("Level: ") + CVar::GetCVarValue("game_level") + "\n");
	TMsg(convertstring<tchar, char>(sprintf(tstring("Clients: %d Entities: %d/%d\n"), GameNetwork()->GetClientsConnected(), CBaseEntity::GetNumEntities(), GameServer()->GetMaxEntities())));

	for (size_t i = 0; i < Game()->GetNumTeams(); i++)
	{
		const CTeam* pTeam = Game()->GetTeam(i);
		if (!pTeam)
			continue;

		if (!pTeam->IsPlayerControlled())
			TMsg("Bot: ");
		else if (pTeam->GetClient() < 0)
			TMsg("Local: ");
		else
			TMsg(convertstring<tchar, char>(sprintf(tstring("%d: "), pTeam->GetClient())));

		TMsg(pTeam->GetTeamName());

		TMsg("\n");
	}
}

CCommand status(_T("status"), ::ShowStatus);

void KickPlayer(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	if (!asTokens.size())
		return;

	GameNetwork()->DisconnectClient(stoi(asTokens[0]));
}

CCommand kick(_T("kick"), ::KickPlayer);
