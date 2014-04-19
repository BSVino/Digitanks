#include "entities/game.h"

#include <iostream>
#include <fstream>

#include <mtrand.h>
#include <tinker_platform.h>
#include <strutils.h>
#include <worklistener.h>

#include <tinker/application.h>
#include <tinker/profiler.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/particles.h>
#include <sound/sound.h>
#include <network/network.h>
#include <network/commands.h>
#include <datamanager/data.h>
#include <datamanager/dataserializer.h>
#include <models/models.h>
#include <textures/texturelibrary.h>
#include <portals/portal.h>
#include <lobby/lobby_server.h>
#include <tinker/cvar.h>
#include <ui/gamewindow.h>
#include <physics/physics.h>
#include <textures/materiallibrary.h>

#ifndef TINKER_NO_TOOLS
#include <tools/workbench.h>
#endif

#include "cameramanager.h"
#include "level.h"

tmap<tstring, CPrecacheItem> CGameServer::s_aPrecacheClasses;
CGameServer* CGameServer::s_pGameServer = NULL;

CGameServer::CGameServer(IWorkListener* pWorkListener)
{
	TAssert(!s_pGameServer);
	s_pGameServer = this;

	m_bAllowPrecaches = false;

	GameNetwork()->SetCallbacks(this, CGameServer::ClientConnectCallback, CGameServer::ClientEnterGameCallback, CGameServer::ClientDisconnectCallback);

	m_pWorkListener = pWorkListener;

	m_iMaxEnts = 1024;

	CBaseEntity::s_apEntityList.resize(m_iMaxEnts);

	m_pCameraManager = NULL;

	m_iSaveCRC = 0;

	m_bLoading = true;
	m_bRestartLevel = false;

	m_flHostTime = 0;
	m_flGameTime = 0;
	m_flFrameTime = 0;
	m_flTimeScale = 1;
	m_flNextClientInfoUpdate = 0;
	m_iFrame = 0;

	size_t iPostSeed = mtrand();

	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	TMsg("Creating physics model... ");
	GamePhysics();	// Just make sure it exists.
	TMsg("Done.\n");

	TMsg("Registering entities... ");

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Registering entities", CBaseEntity::GetEntityRegistration().size());

	tmap<tstring, bool> abRegistered;

	for (tmap<tstring, CEntityRegistration>::iterator it = CBaseEntity::GetEntityRegistration().begin(); it != CBaseEntity::GetEntityRegistration().end(); it++)
		abRegistered[it->first] = false;

	tvector<tstring> asRegisterStack;

	size_t i = 0;
	for (tmap<tstring, CEntityRegistration>::iterator it = CBaseEntity::GetEntityRegistration().begin(); it != CBaseEntity::GetEntityRegistration().end(); it++)
	{
		CEntityRegistration* pRegistration = &it->second;

		if (abRegistered[it->first])
			continue;

		asRegisterStack.clear();
		asRegisterStack.push_back(it->first);

		// Make sure I register all parent classes before I register this one.
		if (pRegistration->m_pszParentClass)
		{
			tstring sThisClass = it->first;
			tstring sParentClass = pRegistration->m_pszParentClass;
			while (!abRegistered[sParentClass])
			{
				// Push to the top, we'll register from the top first.
				asRegisterStack.push_back(sParentClass);

				CEntityRegistration* pRegistration = &CBaseEntity::GetEntityRegistration()[sParentClass];

				sThisClass = sParentClass;
				sParentClass = pRegistration->m_pszParentClass?pRegistration->m_pszParentClass:"";

				if (!sParentClass.length())
					break;
			}
		}

		// The top of the stack is the highest entity on the tree that I must register first.
		while (asRegisterStack.size())
		{
			CBaseEntity::GetEntityRegistration()[asRegisterStack.back()].m_pfnRegisterCallback();
			abRegistered[asRegisterStack.back()] = true;
			asRegisterStack.pop_back();
		}

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(++i);
	}
	TMsg("Done.\n");

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
		m_pWorkListener->SetAction("Scrubbing database", CBaseEntity::GetEntityRegistration().size());

	DestroyAllEntities(tvector<tstring>());

	GamePhysics()->RemoveAllEntities();

	for (size_t i = 0; i < m_apLevels.size(); i++)
		m_apLevels[i].reset();

	if (m_pCameraManager)
		delete m_pCameraManager;

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();

	TAssert(s_pGameServer == this);
	s_pGameServer = NULL;
}

void CGameServer::AllowPrecaches()
{
	m_bAllowPrecaches = true;

	for (tmap<tstring, CEntityRegistration>::iterator it = CBaseEntity::GetEntityRegistration().begin(); it != CBaseEntity::GetEntityRegistration().end(); it++)
	{
		CEntityRegistration* pRegistration = &it->second;
		pRegistration->m_asPrecaches.clear();
		pRegistration->m_ahMaterialPrecaches.clear();
	}

	CModelLibrary::ResetReferenceCounts();
	CSoundLibrary::ResetReferenceCounts();
	CParticleSystemLibrary::ResetReferenceCounts();
}

void CGameServer::AddToPrecacheList(const tstring& sClass)
{
	TAssert(m_bAllowPrecaches || IsLoading());

	auto it = s_aPrecacheClasses.find(sClass);
	if (it != s_aPrecacheClasses.end())
		return;

	auto it2 = CBaseEntity::GetEntityRegistration().find(sClass);
	if (it2 == CBaseEntity::GetEntityRegistration().end())
		return;

	CPrecacheItem o;
	o.m_sClass = sClass;

	s_aPrecacheClasses[sClass] = o;

	if (it2->second.m_pszParentClass)
		AddToPrecacheList(it2->second.m_pszParentClass);
}

void CGameServer::AddAllToPrecacheList()
{
	for (auto it = CBaseEntity::GetEntityRegistration().begin(); it != CBaseEntity::GetEntityRegistration().end(); it++)
		AddToPrecacheList(it->first);
}

void CGameServer::PrecacheList()
{
	TMsg("Precaching entities... ");

	size_t iPrecaches = s_aPrecacheClasses.size();
	if (m_pWorkListener)
		m_pWorkListener->SetAction("Precaching entities", iPrecaches);

	size_t i = 0;

	for (auto it = s_aPrecacheClasses.begin(); it != s_aPrecacheClasses.end(); it++)
	{
		CPrecacheItem* pPrecacheItem = &it->second;

		CEntityRegistration* pRegistration = &CBaseEntity::GetEntityRegistration()[pPrecacheItem->m_sClass];
		pRegistration->m_pfnPrecacheCallback();

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(++i);
	}

	s_aPrecacheClasses.clear();

	// Do this in this order, dependencies matter
	CParticleSystemLibrary::ClearUnreferenced();
	CModelLibrary::ClearUnreferenced();
	CTextureLibrary::ClearUnreferenced();
	CMaterialLibrary::ClearUnreferenced();
	CSoundLibrary::ClearUnreferenced();

	TMsg("Done.\n");
	TMsg(tsprintf("%d models, %d materials, %d textures, %d sounds and %d particle systems precached.\n", CModelLibrary::GetNumModelsLoaded(), CMaterialLibrary::GetNumMaterials(), CTextureLibrary::GetNumTextures(), CSoundLibrary::GetNumSoundsLoaded(), CParticleSystemLibrary::GetNumParticleSystemsLoaded()));

	m_bAllowPrecaches = false;
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

	TMsg("Initializing game server\n");

	ReadLevels();

	GameNetwork()->ClearRegisteredFunctions();

	RegisterNetworkFunctions();

	DestroyAllEntities(tvector<tstring>(), true);

	CParticleSystemLibrary::ClearInstances();

	if (!m_pCameraManager)
		m_pCameraManager = new CCameraManager();

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Pending network actions", 0);
}

void CGameServer::LoadLevel(tstring sFile)
{
	CHandle<CLevel> pLevel = GetLevel(sFile);

	if (pLevel.expired())
		return;

	if (m_bRestartLevel)
		LoadLevel(pLevel);
	else
	{
		FILE* fp = tfopen_asset(sFile, "r");

		CData* pData = new CData();
		CDataSerializer::Read(fp, pData);

		pLevel->CreateEntitiesFromData(pData);

		LoadLevel(pLevel);

		delete pData;
	}
}

static void UnserializeParameter(const tstring& sHandle, const tstring& sValue, CBaseEntity* pEntity)
{
	CSaveData oSaveDataValues;
	CSaveData* pSaveData = CBaseEntity::FindSaveDataValuesByHandle(pEntity->GetClassName(), sHandle.c_str(), &oSaveDataValues);
	TAssert(pSaveData && pSaveData->m_pszHandle);
	if (!pSaveData || !pSaveData->m_pszHandle)
	{
		TError("Unknown handle '" + sHandle + "'\n");
		return;
	}

	if (!pSaveData->m_pfnUnserializeString)
		return;

	pSaveData->m_pfnUnserializeString(sValue, pSaveData, pEntity);
}

void CGameServer::LoadLevel(const CHandle<CLevel>& pLevel)
{
	// Create and name the entities first and add them to this array. This way we avoid a problem where
	// one entity needs to connect to another entity which has not yet been created.
	tmap<size_t, CBaseEntity*> apEntities;

	// Do a quick precache now to load default models and such. Another precache will come later.
	const auto& aEntities = pLevel->GetEntityData();
	for (size_t i = 0; i < aEntities.size(); i++)
		AddToPrecacheList("C" + aEntities[i].GetClass());

	PrecacheList();

	m_bAllowPrecaches = true;

	for (size_t i = 0; i < aEntities.size(); i++)
	{
		const CLevelEntity* pLevelEntity = &aEntities[i];

		tstring sClass = "C" + pLevelEntity->GetClass();

		auto it = CBaseEntity::GetEntityRegistration().find(sClass);
		TAssert(it != CBaseEntity::GetEntityRegistration().end());
		if (it == CBaseEntity::GetEntityRegistration().end())
		{
			TError("Unregistered entity '" + sClass + "'\n");
			continue;
		}

		AddToPrecacheList("C" + aEntities[i].GetClass());

		CBaseEntity* pEntity = Create<CBaseEntity>(sClass.c_str());

		apEntities[i] = pEntity;

		pEntity->SetName(pLevelEntity->GetName());

		// Process outputs here so that they exist when handle callbacks run.
		for (size_t k = 0; k < pLevelEntity->GetOutputs().size(); k++)
		{
			auto pOutput = &pLevelEntity->GetOutputs()[k];
			tstring sValue = pOutput->m_sOutput;

			CSaveData* pSaveData = CBaseEntity::FindOutput(pEntity->GetClassName(), sValue);
			TAssert(pSaveData);
			if (!pSaveData)
			{
				TError("Unknown output '" + sValue + "'\n");
				continue;
			}

			tstring sTarget = pOutput->m_sTargetName;
			tstring sInput = pOutput->m_sInput;
			tstring sArgs = pOutput->m_sArgs;
			bool bKill = pOutput->m_bKill;

			if (!sTarget.length())
			{
				TUnimplemented();
				TError("Output '" + sValue + "' of entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ") is missing a target.\n");
				continue;
			}

			if (!sInput.length())
			{
				TUnimplemented();
				TError("Output '" + sValue + "' of entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ") is missing an input.\n");
				continue;
			}

			pEntity->AddOutputTarget(sValue, sTarget, sInput, sArgs, bKill);
		}
	}

	for (auto it = apEntities.begin(); it != apEntities.end(); it++)
	{
		auto pLevelEntity = &aEntities[it->first];
		CBaseEntity* pEntity = it->second;

		// Force physics related stuff first so it's available if there's a physics model.
		auto itScale = pLevelEntity->GetParameters().find("Scale");
		if (itScale != pLevelEntity->GetParameters().end())
			UnserializeParameter("Scale", itScale->second, pEntity);

		auto itOrigin = pLevelEntity->GetParameters().find("Origin");
		if (itOrigin != pLevelEntity->GetParameters().end())
			UnserializeParameter("Origin", itOrigin->second, pEntity);

		for (auto it = pLevelEntity->GetParameters().begin(); it != pLevelEntity->GetParameters().end(); it++)
		{
			tstring sHandle = it->first;
			tstring sValue = it->second;

			if (sHandle == "MoveParent")
				continue;

			if (sHandle == "Scale")
				continue;

			if (sHandle == "Origin")
				continue;

			UnserializeParameter(sHandle, sValue, pEntity);
		}
	}

	for (auto it = apEntities.begin(); it != apEntities.end(); it++)
	{
		auto pLevelEntity = &aEntities[it->first];
		CBaseEntity* pEntity = it->second;

		// Force MoveParent last so that global -> local conversion is performed.
		auto itMoveParent = pLevelEntity->GetParameters().find("MoveParent");
		if (itMoveParent != pLevelEntity->GetParameters().end())
			UnserializeParameter("MoveParent", itMoveParent->second, pEntity);
	}

	for (size_t i = 0; i < apEntities.size(); i++)
		apEntities[i]->PostLoad();

#ifndef TINKER_NO_TOOLS
	if (CWorkbench::IsActive())
		CWorkbench::LoadLevel(pLevel);
#endif
}

void CGameServer::RestartLevel()
{
	SetLoading(false);
	AllowPrecaches();
	DestroyAllEntities(tvector<tstring>(), true);
	m_bRestartLevel = true;
	Game()->SetupGame(CVar::GetCVarValue("game_mode"));
	m_bRestartLevel = false;
	SetLoading(false);
}

void CGameServer::ReadLevels()
{
	for (size_t i = 0; i < m_apLevels.size(); i++)
		m_apLevels[i].reset();

	m_apLevels.clear();

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Reading meta-structures", 0);

	ReadLevels("levels");

	TMsg(tsprintf("Read %d levels from disk.\n", m_apLevels.size()));
}

void CGameServer::ReadLevels(tstring sDirectory)
{
	tvector<tstring> asFiles = ListDirectory(T_ASSETS_PREFIX + sDirectory);

	for (size_t i = 0; i < asFiles.size(); i++)
	{
		tstring sFile = sDirectory + "/" + asFiles[i];

		if (IsFile(sFile) && sFile.endswith(".txt"))
			ReadLevelInfo(sFile);

		if (IsDirectory(sFile))
			ReadLevels(sFile);
	}
}

void CGameServer::ReadLevelInfo(tstring sFile)
{
	FILE* fp = tfopen_asset(sFile, "r");

	CData* pData = new CData();
	CDataSerializer::Read(fp, pData);

	CResource<CLevel> pLevel = CreateLevel();
	pLevel->SetFile(sFile.replace("\\", "/"));
	pLevel->ReadInfoFromData(pData);
	m_apLevels.push_back(pLevel);

	delete pData;
}

CHandle<CLevel> CGameServer::GetLevel(tstring sFile)
{
	sFile = sFile.replace("\\", "/");
	sFile.trim();
	for (size_t i = 0; i < m_apLevels.size(); i++)
	{
		CResource<CLevel>& pLevel = m_apLevels[i];
		tstring sLevelFile = pLevel->GetFile();
		if (sLevelFile == sFile)
			return pLevel;
		if (sLevelFile == sFile + ".txt")
			return pLevel;
		if (sLevelFile == tstring("levels/") + sFile)
			return pLevel;
		if (sLevelFile == tstring("levels/") + sFile + ".txt")
			return pLevel;
	}

	return CHandle<CLevel>();
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
	TMsg(tsprintf("Client %d (" + GameNetwork()->GetClientNickname(iClient) + ") entering game.\n", iClient));

	if (GetGame())
		GetGame()->OnClientEnterGame(iClient);

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		::CreateEntity.RunCommand(tsprintf("%s %d %d", pEntity->GetClassName(), pEntity->GetHandle(), pEntity->GetSpawnSeed()), iClient);
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
		TMsg("Disconnected from server.\n");
	}
	else
	{
		TMsg(tsprintf("Client %d (" + GameNetwork()->GetClientNickname(iClient) + ") disconnected.\n", iClient));

		CApplication::Get()->OnClientDisconnect(iClient);

		if (GetGame())
			GetGame()->OnClientDisconnect(iClient);
	}
}

void CGameServer::SetClientNickname(int iClient, const tstring& sNickname)
{
	if (iClient == GetClientIndex() && Game()->GetNumLocalPlayers())
	{
		Game()->GetLocalPlayer(0)->SetPlayerName(sNickname);
		return;
	}

	for (size_t i = 0; i < Game()->GetNumPlayers(); i++)
	{
		if (Game()->GetPlayer(i)->GetClient() == iClient)
		{
			Game()->GetPlayer(i)->SetPlayerName(sNickname);
			return;
		}
	}

	TMsg(tsprintf("Can't find client %d to give nickname %s.\n", iClient, sNickname.c_str()));
}

void CGameServer::Think(double flHostTime)
{
	TPROF("CGameServer::Think");

	m_iFrame++;
	m_flFrameTime = flHostTime - m_flHostTime;

	m_flFrameTime *= m_flTimeScale;

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

#ifndef TINKER_NO_TOOLS
	if (CWorkbench::IsActive())
		Workbench()->Think();
#endif

	CNetwork::Think();

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

	Simulate();

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

	TPortal_Think();
}

void CGameServer::Simulate()
{
	Game()->Simulate();

	if (!Game()->ShouldRunSimulation())
		return;

	GamePhysics()->Simulate();
}

void CGameServer::Render()
{
	TPROF("CGameServer::Render");

	if (!GetCameraManager())
		return;

	GetCameraManager()->Think();

	GameWindow()->GetGameRenderer()->Render();
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
	o.open(pFileName, std::ios_base::binary|std::ios_base::out);

	o.write("GameSave", 8);

	CGameServer* pGameServer = GameServer();

	o.write((char*)&pGameServer->m_iSaveCRC, sizeof(pGameServer->m_iSaveCRC));

	o.write((char*)&pGameServer->m_flGameTime, sizeof(pGameServer->m_flGameTime));

	tvector<CBaseEntity*> apSaveEntities;
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
		GameServer()->GetWorkListener()->SetAction("Encountering resistance", 0);

	GameServer()->SetLoading(false);

	return true;
}

CEntityHandle<CBaseEntity> CGameServer::Create(const char* pszEntityName)
{
	TAssert(GameNetwork()->IsHost());

	if (!GameNetwork()->ShouldRunClientFunction())
		return CEntityHandle<CBaseEntity>();

	CEntityHandle<CBaseEntity> hEntity(CreateEntity(pszEntityName));

	TAssert(!GameNetwork()->IsConnected());
	// The below causes entities to be created twice on the server. Wasn't a
	// problem with Digitanks, but is a problem now that I'm adding physics.
	// If I ever go back to multiplayer, the code needs to be split into a
	// client portion and a server portion to help minimize bugs like this.
	//::CreateEntity.RunCommand(sprintf(tstring("%s %d %d"), pszEntityName, hEntity->GetHandle(), hEntity->GetSpawnSeed()));

	if (IsLoading())
		AddToPrecacheList(pszEntityName);

	return hEntity;
}

size_t CGameServer::CreateEntity(const tstring& sClassName, size_t iHandle, size_t iSpawnSeed)
{
	if (CVar::GetCVarBool("net_debug"))
		TMsg(tstring("Creating entity: ") + sClassName + "\n");

	auto it = CBaseEntity::GetEntityRegistration().find(sClassName);
	if (it == CBaseEntity::GetEntityRegistration().end())
	{
		TAssert(!"Entity does not exist. Did you forget to REGISTER_ENTITY ?");
		return ~0;
	}

	CBaseEntity::s_iOverrideEntityListIndex = iHandle;
	iHandle = it->second.m_pfnCreateCallback();
	CBaseEntity::s_iOverrideEntityListIndex = ~0;

	CEntityHandle<CBaseEntity> hEntity(iHandle);
	hEntity->m_sClassName = sClassName;

	hEntity->SetSaveDataDefaults();

	size_t iPostSeed = mtrand();

	if (iSpawnSeed)
		hEntity->SetSpawnSeed(iSpawnSeed);
	else
		hEntity->SetSpawnSeed(mtrand()%99999);	// Don't pick a number so large that it can't fit in (int)

	hEntity->SetSpawnTime(GameServer()->GetGameTime());

	if (PrecachesAllowed())
		hEntity->Precache();

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

void CGameServer::DestroyAllEntities(const tvector<tstring>& asSpare, bool bRemakeGame)
{
	if (!GameNetwork()->IsHost() && !IsLoading())
		return;

	if (m_pWorkListener)
		m_pWorkListener->SetAction("Locating dead nodes", GameServer()->GetMaxEntities());

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
		m_pWorkListener->SetAction("Clearing buffers", GameServer()->m_ahDeletedEntities.size());

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

	CNetworkedVariableData* pVarData = hEntity->FindNetworkVariable((char*)p->m_pExtraData);

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
		CGame::ClearLocalPlayers(NULL);

	m_iClient = p->i1;
	float flNewGameTime = p->fl2;
	if (flNewGameTime - m_flGameTime > 0.1f)
		TMsg(tsprintf("New game time from server %.1f different!\n", flNewGameTime - m_flGameTime));

	m_flGameTime = flNewGameTime;

	// Can't send any client commands until we've gotten the client info because we need m_iClient filled out properly.
	if (!m_bGotClientInfo)
	{
		GameNetwork()->SetRunningClientFunctions(false);
		SendNickname.RunCommand(m_sNickname);
	}

	m_bGotClientInfo = true;
}

CGameRenderer* CGameServer::GetRenderer()
{
	return static_cast<CGameRenderer*>(GameWindow()->GetRenderer());
}

CCameraManager* CGameServer::GetCameraManager()
{
#ifndef TINKER_NO_TOOLS
	if (CWorkbench::IsActive())
		return CWorkbench::GetCameraManager();
#endif

	return m_pCameraManager;
}

CGame* CGameServer::GetGame()
{
	return m_hGame;
}

void ShowStatus(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	TMsg(tstring("Level: ") + CVar::GetCVarValue("game_level") + "\n");
	TMsg(tsprintf("Clients: %d Entities: %d/%d\n", GameNetwork()->GetClientsConnected(), CBaseEntity::GetNumEntities(), GameServer()->GetMaxEntities()));

	for (size_t i = 0; i < Game()->GetNumPlayers(); i++)
	{
		const CPlayer* pPlayer = Game()->GetPlayer(i);
		if (!pPlayer)
			continue;

		if (pPlayer->GetClient() < 0)
			TMsg("Local: ");
		else
			TMsg(tsprintf("%d: ", pPlayer->GetClient()));

		TMsg(pPlayer->GetPlayerName());

		TMsg("\n");
	}
}

CCommand status("status", ::ShowStatus);

void KickPlayer(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	if (!asTokens.size())
		return;

	GameNetwork()->DisconnectClient(stoi(asTokens[0]));
}

CCommand kick("kick", ::KickPlayer);

void FireInput(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	if (!CVar::GetCVarBool("cheats"))
		return;

	if (asTokens.size() < 3)
	{
		TMsg("Format: ent_input entityname input [optional args]\n");
		return;
	}

	tvector<CBaseEntity*> apEntities;
	CBaseEntity::FindEntitiesByName(asTokens[1], apEntities);

	if (!apEntities.size())
	{
		if (CVar::GetCVarBool("debug_entity_outputs"))
			TMsg("Console -> none\n");
		else
			TError("No entities found that match name \"" + asTokens[1] + "\".\n");

		return;
	}

	tstring sArgs;
	for (size_t i = 3; i < asTokens.size(); i++)
		sArgs += asTokens[i] + " ";

	for (size_t i = 0; i < apEntities.size(); i++)
	{
		CBaseEntity* pTargetEntity = apEntities[i];

		if (CVar::GetCVarBool("debug_entity_outputs"))
			TMsg("Console -> " + tstring(pTargetEntity->GetClassName()) + "(\"" + pTargetEntity->GetName() + "\")." + asTokens[2] + "(\"" + sArgs + "\")\n");

		pTargetEntity->CallInput(asTokens[2], sArgs);
	}
}

CCommand ent_input("ent_input", ::FireInput);
