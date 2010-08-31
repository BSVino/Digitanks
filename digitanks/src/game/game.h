#ifndef GAME_H
#define GAME_H

#include <vector>

#include <network/network.h>

#include "baseentity.h"
#include "team.h"

class CGame : public INetworkListener
{
public:
												CGame();
												~CGame();

public:
	virtual void								RegisterNetworkFunctions();

	NET_CALLBACK(CGame,							ClientConnect);
	NET_CALLBACK(CGame,							LoadingDone);
	NET_CALLBACK(CGame,							ClientDisconnect);

	virtual void								OnClientConnect(CNetworkParameters* p) {};
	virtual void								OnClientUpdate(CNetworkParameters* p) {};
	virtual void								OnClientDisconnect(CNetworkParameters* p) {};

	void										Think(float flRealTime);
	void										Simulate();
	void										Render();

	virtual void								Think() {};

	virtual void								OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled) {};
	virtual void								OnKilled(class CBaseEntity* pEntity) {};
	virtual void								OnDeleted(class CBaseEntity* pEntity);

	CEntityHandle<CBaseEntity>					Create(const char* pszEntityName);
	size_t										CreateEntity(size_t iRegisteredEntity, size_t iHandle = ~0, size_t iSpawnSeed = 0);
	void										Delete(class CBaseEntity* pEntity);

	NET_CALLBACK(CGame,							CreateEntity);
	NET_CALLBACK(CGame,							DestroyEntity);

	template<class T>
	T*											Create(const char* pszEntityName)
	{
		return dynamic_cast<T*>(Create(pszEntityName).GetPointer());
	}

	NET_CALLBACK(CGame,							ClientInfo);
	NET_CALLBACK(CGame,							SetOrigin);
	NET_CALLBACK(CGame,							SetAngles);
	NET_CALLBACK(CGame,							AddTeam);

	NET_CALLBACK_ENTITY(CGame, CTeam,			SetTeamColor);
	NET_CALLBACK_ENTITY(CGame, CTeam,			SetTeamClient);
	NET_CALLBACK_ENTITY(CGame, CTeam,			AddEntityToTeam);

	virtual void								CreateRenderer();

	virtual bool								TraceLine(const Vector& s1, const Vector& s2, Vector& vecHit, CBaseEntity** pHit);

	size_t										GetNumTeams() { return m_ahTeams.size(); };
	CTeam*										GetTeam(size_t i) { if (i >= GetNumTeams()) return NULL; return m_ahTeams[i]; };
	bool										IsTeamControlledByMe(CTeam* pTeam);

	float										GetFrameTime() { return m_flFrameTime; };
	float										GetGameTime() { return m_flGameTime; };

	class CCamera*								GetCamera() { return m_pCamera; };
	class CRenderer*							GetRenderer() { return m_pRenderer; };

	bool										IsLoading() { return m_bLoading; };
	bool										IsClient() { return m_iClient >= 0; };

	static CGame*								GetGame() { return s_pGame; };

	static CTeam*								GetLocalTeam();

protected:
	std::vector<CEntityHandle<CTeam> >			m_ahTeams;

	std::vector<CEntityHandle<CBaseEntity> >	m_ahDeletedEntities;

	static CGame*								s_pGame;

	float										m_flGameTime;
	float										m_flSimulationTime;
	float										m_flFrameTime;
	float										m_flRealTime;

	class CCamera*								m_pCamera;
	class CRenderer*							m_pRenderer;

	bool										m_bLoading;

	int											m_iClient;

	static CEntityHandle<CTeam>					s_hLocalTeam;
};

inline class CGame* Game()
{
	return CGame::GetGame();
}

#endif
