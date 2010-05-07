#ifndef DT_DIGITANKSGAME_H
#define DT_DIGITANKSGAME_H

#include <vector>

#include "game.h"
#include "team.h"
#include <common.h>

class IDigitanksGameListener
{
public:
	virtual void			GameStart()=0;
	virtual void			GameOver()=0;

	virtual void			NewCurrentTeam()=0;
	virtual void			NewCurrentTank()=0;
};

class CDigitanksGame : public CGame
{
	DECLARE_CLASS(CDigitanksGame, CGame);

public:
							CDigitanksGame();
							~CDigitanksGame();

public:
	void					SetListener(IDigitanksGameListener* pListener) { m_pListener = pListener; };

	void					SetupDefaultGame();

	void					StartGame();

	virtual void			Think();

	void					SetCurrentTank(CDigitank* pTank);

	void					SetDesiredMove();
	void					SetDesiredTurn();
	void					SetDesiredAim();

	void					NextTank();
	void					EndTurn();
	void					StartTurn();

	virtual void			OnKilled(class CBaseEntity* pEntity);
	void					CheckWinConditions();

	virtual void			OnDeleted(class CBaseEntity* pEntity);

	size_t					GetNumTeams() { return m_apTeams.size(); };
	CTeam*					GetTeam(size_t i) { return m_apTeams[i]; };

	CTeam*					GetCurrentTeam();
	CDigitank*				GetCurrentTank();

	void					AddProjectileToWaitFor() { m_iWaitingForProjectiles++; };

protected:
	std::vector<CTeam*>		m_apTeams;

	size_t					m_iCurrentTeam;
	size_t					m_iCurrentTank;

	IDigitanksGameListener*	m_pListener;

	bool					m_bWaitingForProjectiles;
	size_t					m_iWaitingForProjectiles;
};

inline class CDigitanksGame* DigitanksGame()
{
	if (!Game())
		return NULL;

	return dynamic_cast<CDigitanksGame*>(Game());
}

#endif