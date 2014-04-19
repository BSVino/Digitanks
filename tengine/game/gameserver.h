#include "entities/baseentity.h"
// Always include baseentity.h first, it has some stuff this file relies on
#ifndef TINKER_BASEENTITY_H
#include "entities/baseentity.h"
#endif

#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <vector>

#include <tinker_memory.h>

#include <network/network.h>

#include "entities/team.h"

class CLevel;

typedef enum
{
	SERVER_LOCAL,
	SERVER_HOST,
	SERVER_CLIENT,
} servertype_t;

class CPrecacheItem
{
public:
	CPrecacheItem()
	{
	}

public:
	tstring						m_sClass;
	CEntityHandle<CBaseEntity>	m_hEntity;
};

class CGameServer : public INetworkListener
{
public:
												CGameServer(class IWorkListener* pWorkListener = NULL);
	virtual										~CGameServer();

public:
	void										AllowPrecaches();
	bool										PrecachesAllowed() { return m_bAllowPrecaches; };
	void										AddToPrecacheList(const tstring& sClass);
	void										AddAllToPrecacheList();
	void										PrecacheList();

	void										SetServerType(servertype_t eServerType) { m_eServerType = eServerType; };
	void										SetServerPort(int iPort) { m_iPort = iPort; };

	void										SetPlayerNickname(const tstring& sNickname);
	tstring										GetPlayerNickname() { return m_sNickname; }

	void										Initialize();

	void										LoadLevel(tstring sFile);
	void										LoadLevel(const CHandle<CLevel>& pLevel);
	void										RestartLevel();

	void										SetupFromLobby(bool bFromLobby) { m_bSetupFromLobby = bFromLobby; };
	bool										ShouldSetupFromLobby() { return m_bSetupFromLobby; };

	void										ReadLevels();
	void										ReadLevels(tstring sDirectory);
	void										ReadLevelInfo(tstring sFile);

	size_t										GetNumLevels() { return m_apLevels.size(); }
	class CHandle<CLevel>						GetLevel(size_t i) { if (i >= m_apLevels.size()) return CHandle<CLevel>(); return CHandle<CLevel>(m_apLevels[i]); }
	class CHandle<CLevel>						GetLevel(tstring sFile);

	void										Halt();
	bool										IsHalting() { return m_bHalting; };

	void										RegisterNetworkFunctions();

	NET_CALLBACK(CGameServer,					ClientConnect);
	NET_CALLBACK(CGameServer,					ClientEnterGame);
	NET_CALLBACK(CGameServer,					LoadingDone);
	NET_CALLBACK(CGameServer,					ClientDisconnect);
	void										ClientConnect(int iClient);
	void										ClientEnterGame(int iClient);
	void										ClientDisconnect(int iClient);
	void										SetClientNickname(int iClient, const tstring& sNickname);

	void										Think(double flHostTime);
	void										Simulate();
	void										Render();

	virtual void								Think() {};

	void										GenerateSaveCRC(size_t iInput);
	static void									SaveToFile(const tchar* pFileName);
	static bool									LoadFromFile(const tchar* pFileName);

	CEntityHandle<CBaseEntity>					Create(const char* pszEntityName);
	size_t										CreateEntity(const tstring& sClassName, size_t iHandle = ~0, size_t iSpawnSeed = 0);
	void										Delete(class CBaseEntity* pEntity);

	NET_CALLBACK(CGameServer,					DestroyEntity);

	template<class T>
	T*											Create(const char* pszEntityName)
	{
		return dynamic_cast<T*>(Create(pszEntityName).GetPointer());
	}

	void										DestroyAllEntities(const tvector<tstring>& asSpare = tvector<tstring>(), bool bRemakeGame = false);

	NET_CALLBACK(CGameServer,					UpdateValue);
	NET_CALLBACK(CGameServer,					ClientInfo);

	float										GetFrameTime() { return (float)m_flFrameTime; };
	double										GetGameTime() { return m_flGameTime; };
	size_t										GetFrame() { return m_iFrame; }
	float                                       GetTimeScale() { return m_flTimeScale; }
	void                                        SetTimeScale(float flTimeScale) { m_flTimeScale = flTimeScale; }

	class CGameRenderer*						GetRenderer();
	class CCameraManager*						GetCameraManager();

	bool										IsLoading() { return m_bLoading; };
	void										SetLoading(bool bLoading) { m_bLoading = bLoading; };
	size_t										GetClientIndex() { return m_iClient; };
	bool										IsClient() { return m_iClient >= 0; };
	size_t										GetMaxEntities() { return m_iMaxEnts; };

	void										SetWorkListener(IWorkListener* pListener)  { m_pWorkListener = pListener; };
	IWorkListener*								GetWorkListener() { return m_pWorkListener; };

	static CGameServer*							GetGameServer() { return s_pGameServer; };
	class CGame*								GetGame();

protected:
	bool										m_bAllowPrecaches;
	static tmap<tstring, CPrecacheItem>			s_aPrecacheClasses;

	tstring										m_sNickname;

	tvector<CEntityHandle<CBaseEntity> >		m_ahDeletedEntities;

	static CGameServer*							s_pGameServer;

	CEntityHandle<class CGame>					m_hGame;

	size_t										m_iSaveCRC;

	double										m_flGameTime;		// This is how time passes for the game entities
	double										m_flFrameTime;		// This is the delta of each frame of game time
	double										m_flHostTime;		// This is the current time for the computer
	float                                       m_flTimeScale;      // This is a scalar applied to the frame time
	size_t										m_iFrame;

	class CCameraManager*						m_pCameraManager;

	bool										m_bLoading;
	bool										m_bRestartLevel;

	int											m_iClient;
	bool										m_bGotClientInfo;

	servertype_t								m_eServerType;
	int											m_iPort;

	bool										m_bSetupFromLobby;

	bool										m_bHalting;

	tvector<CResource<CLevel>>					m_apLevels;

	size_t										m_iMaxEnts;

	double										m_flNextClientInfoUpdate;

	IWorkListener*								m_pWorkListener;
};

inline class CGameServer* GameServer()
{
	return CGameServer::GetGameServer();
}

// Let the game directory define this.
extern class CGame* CreateGame();
extern CResource<CLevel> CreateLevel();
extern class CHUDViewport* CreateHUD();
extern tstring GetInitialGameMode();

#endif
