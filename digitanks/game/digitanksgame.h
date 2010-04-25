#ifndef DT_DIGITANKSGAME_H
#define DT_DIGITANKSGAME_H

#include "team.h"

#include <vector>

class CDigitanksGame
{
public:
							~CDigitanksGame();

public:
	void					SetupDefaultGame();

	void					StartGame();

	void					Think();

	void					SetDesiredMove();

	void					NextTank();
	void					Turn();

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