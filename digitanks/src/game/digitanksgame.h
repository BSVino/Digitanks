#ifndef DT_DIGITANKSGAME_H
#define DT_DIGITANKSGAME_H

#include <vector>

#include "game.h"
#include "team.h"
#include <common.h>
#include "terrain.h"

class IDigitanksGameListener
{
public:
	virtual void			GameStart()=0;
	virtual void			GameOver(bool bPlayerWon)=0;

	virtual void			NewCurrentTeam()=0;
	virtual void			NewCurrentTank()=0;

	virtual void			OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit)=0;
	virtual void			OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit)=0;

	virtual void			SetHUDActive(bool bActive)=0;
};

class CDigitanksGame : public CGame
{
	DECLARE_CLASS(CDigitanksGame, CGame);

public:
							CDigitanksGame();
							~CDigitanksGame();

public:
	void					SetListener(IDigitanksGameListener* pListener) { m_pListener = pListener; };

	void					SetupGame(int iPlayers, int iTanks);

	void					StartGame();

	virtual void			Think();

	void					SetCurrentTank(CDigitank* pTank);

	void					SetDesiredMove(bool bAllTanks = false);
	void					SetDesiredTurn(bool bAllTanks = false, Vector vecLookAt = Vector());
	void					SetDesiredAim(bool bAllTanks = false);

	void					NextTank();
	void					EndTurn();
	void					StartTurn();

	void					Bot_ExecuteTurn();

	virtual void			Explode(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flRadius, float flDamage, CBaseEntity* pIgnore, CTeam* pTeamIgnore);

	virtual void			OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit);

	virtual void			OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit);
	virtual void			OnKilled(class CBaseEntity* pEntity);
	void					CheckWinConditions();

	virtual void			OnDeleted(class CBaseEntity* pEntity);

	size_t					GetNumTeams() { return m_ahTeams.size(); };
	CTeam*					GetTeam(size_t i) { return m_ahTeams[i]; };

	CTeam*					GetCurrentTeam();
	CDigitank*				GetCurrentTank();
	size_t					GetCurrentTankId();

	CTerrain*				GetTerrain() { return m_hTerrain; };

	float					GetGravity();

	void					AddProjectileToWaitFor() { m_iWaitingForProjectiles++; };

	void					AddTankAim(Vector vecAim, float flRadius, bool bFocus);
	void					GetTankAims(std::vector<Vector>& avecAims, std::vector<float>& aflAimRadius, size_t& iFocus);
	void					ClearTankAims();

	void					SetDifficulty(size_t iDifficulty) { m_iDifficulty = iDifficulty; };
	size_t					GetDifficulty() { return m_iDifficulty; };

protected:
	std::vector<CEntityHandle<CTeam>>	m_ahTeams;

	size_t					m_iCurrentTeam;
	size_t					m_iCurrentTank;

	CEntityHandle<CTerrain>	m_hTerrain;

	IDigitanksGameListener*	m_pListener;

	bool					m_bWaitingForMoving;

	bool					m_bWaitingForProjectiles;
	size_t					m_iWaitingForProjectiles;

	size_t					m_iPowerups;

	std::vector<Vector>		m_avecTankAims;
	std::vector<float>		m_aflTankAimRadius;
	size_t					m_iTankAimFocus;

	size_t					m_iDifficulty;
};

inline class CDigitanksGame* DigitanksGame()
{
	if (!Game())
		return NULL;

	return dynamic_cast<CDigitanksGame*>(Game());
}

enum
{
	CG_TANK = 1,
	CG_TERRAIN,
	CG_PROJECTILE,
	CG_POWERUP,
};

#endif