#ifndef GAME_H
#define GAME_H

#include <EASTL/vector.h>

#include <network/network.h>

#include "baseentity.h"
#include "gameserver.h"
#include "team.h"

class CGame : public CBaseEntity, public INetworkListener
{
	REGISTER_ENTITY_CLASS(CGame, CBaseEntity);

public:
												CGame();
	virtual										~CGame();

public:
	virtual void								Spawn();

	virtual void								RegisterNetworkFunctions();

	virtual void								OnClientConnect(CNetworkParameters* p);
	virtual void								OnClientDisconnect(CNetworkParameters* p);

	virtual void								EnterGame();

	NET_CALLBACK(CGame,							SetAngles);

	void										AddTeam(CTeam* pTeam);
	void										RemoveTeam(CTeam* pTeam);

	virtual void								OnDeleted();

	virtual void								OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled) {};
	virtual void								OnDeleted(class CBaseEntity* pEntity);

	virtual bool								TraceLine(const Vector& s1, const Vector& s2, Vector& vecHit, CBaseEntity** pHit, int iCollisionGroup = 0);

	size_t										GetNumTeams() { return m_ahTeams.size(); };
	CTeam*										GetTeam(size_t i) { if (i >= GetNumTeams()) return NULL; return m_ahTeams[i]; };
	bool										IsTeamControlledByMe(CTeam* pTeam);

	const eastl::vector<CEntityHandle<CTeam> >&	GetLocalTeams();
	size_t										GetNumLocalTeams();
	CTeam*										GetLocalTeam(size_t i);

	virtual bool								AllowCheats();

protected:
	CNetworkedSTLVector<CEntityHandle<CTeam> >	m_ahTeams;

	eastl::vector<CEntityHandle<CTeam> >		m_ahLocalTeams;
};

inline class CGame* Game()
{
	CGameServer* pGameServer = GameServer();
	if (!pGameServer)
		return NULL;

	CGame* pGame = pGameServer->GetGame();
	if (!pGame)
		return NULL;

	return pGame;
}

#endif
