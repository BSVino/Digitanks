#ifndef GAME_H
#define GAME_H

#include <tvector.h>

#include <network/replication.h>
#include <game/gameserver.h>
#include <game/entities/player.h>

#include "baseentity.h"

class CGame : public CBaseEntity, public INetworkListener
{
	REGISTER_ENTITY_CLASS(CGame, CBaseEntity);

public:
												CGame();
	virtual										~CGame();

public:
	virtual void								Spawn();

	virtual void								SetupGame(tstring sType) {};

	DECLARE_ENTITY_INPUT(ReloadLevel);
	DECLARE_ENTITY_INPUT(LoadLevel);

	virtual void								Simulate() {};
	virtual bool								ShouldRunSimulation() { return true; };

	virtual void								RegisterNetworkFunctions();

	virtual void								OnClientConnect(int iClient);
	virtual void								OnClientEnterGame(int iClient);
	virtual void								OnClientDisconnect(int iClient);

	virtual void                                OnDisplayLesson(const tstring& sTutorial) {};

	virtual void								EnterGame();

	void										AddPlayer(CPlayer* pPlayer);
	void										RemovePlayer(const CPlayer* pPlayer);

	virtual void                                OnEntityKilled(CBaseEntity* pKilledBy) {};

	virtual void								OnDeleted();
	virtual void								OnDeleted(const CBaseEntity* pEntity);

	virtual bool                                TakesDamageFrom(CBaseEntity* pVictim, CBaseEntity* pAttacker);
	virtual void								OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled) {};

	size_t										GetNumPlayers() const { return m_ahPlayers.size(); };
	CPlayer*									GetPlayer(size_t i) const;
	bool										IsTeamControlledByMe(const class CPlayer* pPlayer);

	const tvector<CEntityHandle<CPlayer> >&		GetLocalPlayers();
	size_t										GetNumLocalPlayers();
	CPlayer*									GetLocalPlayer(size_t i);
	CPlayer*									GetLocalPlayer();

	static void									ClearLocalPlayers(CNetworkedVariableBase* pVariable);

	virtual bool								AllowCheats();

protected:
	CNetworkedSTLVector<CEntityHandle<CPlayer> >	m_ahPlayers;

	tvector<CEntityHandle<CPlayer> >			m_ahLocalPlayers;
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
