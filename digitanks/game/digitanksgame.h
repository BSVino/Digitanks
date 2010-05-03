#ifndef DT_DIGITANKSGAME_H
#define DT_DIGITANKSGAME_H

#include "game.h"
#include "team.h"

#include <vector>

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
public:
							CDigitanksGame();
							~CDigitanksGame();

public:
	void					SetListener(IDigitanksGameListener* pListener) { m_pListener = pListener; };

	void					SetupDefaultGame();

	void					StartGame();

	void					SetDesiredMove();
	void					SetDesiredTurn();

	void					NextTank();
	void					Turn();

	virtual void			OnKilled(class CBaseEntity* pEntity);
	void					CheckWinConditions();

	virtual void			OnDeleted(class CBaseEntity* pEntity);

	size_t					GetNumTeams() { return m_apTeams.size(); };
	CTeam*					GetTeam(size_t i) { return m_apTeams[i]; };

	CTeam*					GetCurrentTeam();
	CDigitank*				GetCurrentTank();

protected:
	std::vector<CTeam*>		m_apTeams;

	size_t					m_iCurrentTeam;
	size_t					m_iCurrentTank;

	IDigitanksGameListener*	m_pListener;
};

inline class CDigitanksGame* DigitanksGame()
{
	if (!Game())
		return NULL;

	return dynamic_cast<CDigitanksGame*>(Game());
}

#endif