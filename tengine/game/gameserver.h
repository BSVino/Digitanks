#include "baseentity.h"
// Always include baseentity.h first, it has some stuff this file relies on
#ifndef TINKER_BASEENTITY_H
#include "baseentity.h"
#endif

#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <vector>

#include <network/network.h>

#include "team.h"

typedef enum
{
	SERVER_LOCAL,
	SERVER_HOST,
	SERVER_CLIENT,
} servertype_t;

class CGameServer : public INetworkListener
{
public:
												CGameServer(class IWorkListener* pWorkListener = NULL);
	virtual										~CGameServer();

public:
	void										SetServerType(servertype_t eServerType) { m_eServerType = eServerType; };
	void										SetServerPort(int iPort) { m_iPort = iPort; };

	void										SetPlayerNickname(const tstring& sNickname);
	tstring								GetPlayerNickname() { return m_sNickname; }

	void										Initialize();

	void										SetupFromLobby(bool bFromLobby) { m_bSetupFromLobby = bFromLobby; };
	bool										ShouldSetupFromLobby() { return m_bSetupFromLobby; };

	void										ReadLevels();
	void										ReadLevels(tstring sDirectory);
	void										ReadLevel(tstring sFile);

	size_t										GetNumLevels() { return m_apLevels.size(); }
	class CLevel*								GetLevel(size_t i) { return m_apLevels[i]; }
	class CLevel*								GetLevel(tstring sFile);

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

	void										Think(float flHostTime);
	void										Simulate();
	void										Render();

	virtual void								Think() {};

	void										GenerateSaveCRC(size_t iInput);
	static void									SaveToFile(const tchar* pFileName);
	static bool									LoadFromFile(const tchar* pFileName);

	CEntityHandle<CBaseEntity>					Create(const char* pszEntityName);
	size_t										CreateEntity(size_t iRegisteredEntity, size_t iHandle = ~0, size_t iSpawnSeed = 0);
	void										Delete(class CBaseEntity* pEntity);

	NET_CALLBACK(CGameServer,					CreateEntity);
	NET_CALLBACK(CGameServer,					DestroyEntity);

	template<class T>
	T*											Create(const char* pszEntityName)
	{
		return dynamic_cast<T*>(Create(pszEntityName).GetPointer());
	}

	void										DestroyAllEntities(const eastl::vector<eastl::string>& asSpare = eastl::vector<eastl::string>(), bool bRemakeGame = false);

	NET_CALLBACK(CGameServer,					UpdateValue);
	NET_CALLBACK(CGameServer,					ClientInfo);

	float										GetFrameTime() { return m_flFrameTime; };
	float										GetGameTime() { return m_flGameTime; };

	class CRenderer*							GetRenderer();
	class CCamera*								GetCamera() { return m_pCamera; };

	bool										IsLoading() { return m_bLoading; };
	void										SetLoading(bool bLoading) { m_bLoading = bLoading; };
	size_t										GetClientIndex() { return m_iClient; };
	bool										IsClient() { return m_iClient >= 0; };
	size_t										GetMaxEntities() { return m_iMaxEnts; };

	void										SetWorkListener(IWorkListener* pListener);
	IWorkListener*								GetWorkListener() { return m_pWorkListener; };

	static CGameServer*							GetGameServer() { return s_pGameServer; };
	class CGame*								GetGame();

protected:
	tstring				m_sNickname;

	eastl::vector<CEntityHandle<CBaseEntity> >	m_ahDeletedEntities;

	static CGameServer*							s_pGameServer;

	CEntityHandle<class CGame>					m_hGame;

	size_t										m_iSaveCRC;

	float										m_flGameTime;		// This is how time passes for the game entities
	float										m_flSimulationTime;	// This is a higher resolution of game time for physics
	float										m_flFrameTime;		// This is the delta of each frame of game time
	float										m_flHostTime;		// This is the current time for the computer

	eastl::vector<CEntityHandle<CBaseEntity> >	m_apSimulateList;
	eastl::vector<CBaseEntity*>					m_apRenderList;

	class CCamera*								m_pCamera;

	bool										m_bLoading;

	int											m_iClient;
	bool										m_bGotClientInfo;

	servertype_t								m_eServerType;
	int											m_iPort;

	bool										m_bSetupFromLobby;

	bool										m_bHalting;

	eastl::vector<class CLevel*>				m_apLevels;

	size_t										m_iMaxEnts;

	float										m_flNextClientInfoUpdate;

	IWorkListener*								m_pWorkListener;
};

inline class CGameServer* GameServer()
{
	return CGameServer::GetGameServer();
}

// Let the game directory define this.
extern class CGame* CreateGame();
extern class CRenderer* CreateRenderer();
extern class CCamera* CreateCamera();
extern class CLevel* CreateLevel();

#endif
