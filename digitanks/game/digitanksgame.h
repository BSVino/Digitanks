#ifndef DT_DIGITANKSGAME_H
#define DT_DIGITANKSGAME_H

#include "game.h"
#include "team.h"

#include <vector>

class CDigitanksGame : public CGame
{
public:
							~CDigitanksGame();

public:
	void					SetupDefaultGame();

	void					StartGame();

	void					SetDesiredMove();

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
};

#endif