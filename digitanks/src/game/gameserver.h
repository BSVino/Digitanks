#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <vector>

#include <network/network.h>

#include "baseentity.h"
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
												CGameServer();
	virtual										~CGameServer();

public:
	void										SetServerType(servertype_t eServerType) { m_eServerType = eServerType; };
	void										SetConnectHost(const eastl::string sHost) { m_sConnectHost = sHost; };
	void										SetServerPort(int iPort) { m_iPort = iPort; };

	void										Initialize();

	void										ReadLevels();
	void										ReadLevels(eastl::string16 sDirectory);
	void										ReadLevel(eastl::string16 sFile);

	size_t										GetNumLevels() { return m_apLevels.size(); }
	class CLevel*								GetLevel(size_t i) { return m_apLevels[i]; }

	void										Halt() { m_bHalting = true; };
	bool										IsHalting() { return m_bHalting; };

	void										RegisterNetworkFunctions();

	NET_CALLBACK(CGameServer,					ClientConnect);
	NET_CALLBACK(CGameServer,					LoadingDone);
	NET_CALLBACK(CGameServer,					ClientDisconnect);

	void										Think(float flRealTime);
	void										Simulate();
	void										Render();

	virtual void								Think() {};

	void										GenerateSaveCRC(size_t iInput);
	static void									SaveToFile(const wchar_t* pFileName);
	static bool									LoadFromFile(const wchar_t* pFileName);

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

	static CGameServer*							GetGameServer() { return s_pGameServer; };
	class CGame*								GetGame();

protected:
	eastl::vector<CEntityHandle<CBaseEntity> >	m_ahDeletedEntities;

	static CGameServer*							s_pGameServer;

	CEntityHandle<class CGame>					m_hGame;

	size_t										m_iSaveCRC;

	float										m_flGameTime;
	float										m_flSimulationTime;
	float										m_flFrameTime;
	float										m_flRealTime;

	class CCamera*								m_pCamera;
	class CRenderer*							m_pRenderer;

	bool										m_bLoading;

	int											m_iClient;

	servertype_t								m_eServerType;
	eastl::string								m_sConnectHost;
	int											m_iPort;

	bool										m_bHalting;

	eastl::vector<class CLevel*>				m_apLevels;
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
