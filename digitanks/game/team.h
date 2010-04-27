#ifndef DT_TEAM_H
#define DT_TEAM_H

#include "digitank.h"
#include "baseentity.h"

#include "color.h"
#include <vector>

class CTeam
{
	friend class CDigitanksGame;

public:
								~CTeam();

public:
	void						StartTurn();

	void						MoveTanks();
	void						FireTanks();

	virtual void				OnKilled(class CBaseEntity* pEntity);

	virtual void				OnDeleted(class CBaseEntity* pEntity);

	size_t						GetNumTanksAlive();

	Color						GetColor() { return m_clrTeam; };

	size_t						GetNumTanks() { return m_ahTanks.size(); };
	CDigitank*					GetTank(size_t i) { if (!m_ahTanks.size()) return NULL; return (CDigitank*)m_ahTanks[i]; };

protected:
	Color						m_clrTeam;

	std::vector<CEntityHandle<CDigitank> >	m_ahTanks;
};

#endif