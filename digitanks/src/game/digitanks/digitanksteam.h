#ifndef DT_DIGITANKSTEAM_H
#define DT_DIGITANKSTEAM_H

#include <game/team.h>
#include "digitank.h"

class CDigitanksTeam : public CTeam
{
	friend class CDigitanksGame;

	REGISTER_ENTITY_CLASS(CDigitanksTeam, CTeam);

public:
								CDigitanksTeam();
								~CDigitanksTeam();

public:
	virtual void				OnAddEntity(CBaseEntity* pEntity);

	void						StartTurn();

	void						MoveTanks();
	void						FireTanks();

	virtual void				OnDeleted(class CBaseEntity* pEntity);

	size_t						GetNumTanksAlive();

	size_t						GetNumTanks() { return m_ahTanks.size(); };
	CDigitank*					GetTank(size_t i) { if (!m_ahTanks.size()) return NULL; return (CDigitank*)m_ahTanks[i]; };

protected:
	std::vector<CEntityHandle<CDigitank> >	m_ahTanks;
};

#endif