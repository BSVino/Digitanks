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
												~CGame();

public:
	virtual void								Spawn();

	virtual void								RegisterNetworkFunctions();

	virtual void								OnClientConnect(CNetworkParameters* p);
	virtual void								OnClientDisconnect(CNetworkParameters* p);

	NET_CALLBACK(CGame,							SetAngles);

	void										AddTeamToList(CTeam* pTeam);
	NET_CALLBACK(CGame,							AddTeam);

	void										RemoveTeamFromList(CTeam* pTeam);
	NET_CALLBACK(CGame,							RemoveTeam);

	NET_CALLBACK_ENTITY(CGame, CTeam,			SetTeamColor);
	NET_CALLBACK_ENTITY(CGame, CTeam,			SetTeamClient);
	NET_CALLBACK_ENTITY(CGame, CTeam,			AddEntityToTeam);

	virtual void								OnDeleted();

	virtual void								OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled) {};
	virtual void								OnDeleted(class CBaseEntity* pEntity);

	virtual bool								TraceLine(const Vector& s1, const Vector& s2, Vector& vecHit, CBaseEntity** pHit, int iCollisionGroup = 0);

	size_t										GetNumTeams() { return m_ahTeams.size(); };
	CTeam*										GetTeam(size_t i) { if (i >= GetNumTeams()) return NULL; return m_ahTeams[i]; };
	bool										IsTeamControlledByMe(CTeam* pTeam);

	CTeam*										GetLocalTeam();

protected:
	eastl::vector<CEntityHandle<CTeam> >		m_ahTeams;

	CEntityHandle<CTeam>						m_hLocalTeam;
};

inline class CGame* Game()
{
	if (!GameServer())
		return NULL;

	if (!GameServer()->GetGame())
		return NULL;

	return GameServer()->GetGame();
}

#endif
