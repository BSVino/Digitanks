#ifndef DT_TEAM_H
#define DT_TEAM_H

#include "digitank.h"

#include "color.h"
#include <vector>

class CTeam
{
	friend class CDigitanksGame;

public:
								~CTeam();

public:
	Color						GetColor() { return m_clrTeam; };

	size_t						GetNumTanks() { return m_apTanks.size(); };
	CDigitank*					GetTank(size_t i) { return m_apTanks[i]; };

protected:
	Color						m_clrTeam;

	std::vector<CDigitank*>		m_apTanks;
};

#endif