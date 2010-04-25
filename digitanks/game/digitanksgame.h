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

	size_t					GetNumTeams() { return m_apTeams.size(); };
	CTeam*					GetTeam(size_t i) { return m_apTeams[i]; };

protected:
	std::vector<CTeam*>		m_apTeams;
};

#endif