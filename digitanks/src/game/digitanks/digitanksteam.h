#ifndef DT_DIGITANKSTEAM_H
#define DT_DIGITANKSTEAM_H

#include <game/team.h>
#include "digitank.h"

// AI stuff
#include "cpu.h"

class CDigitanksTeam : public CTeam
{
	friend class CDigitanksGame;

	REGISTER_ENTITY_CLASS(CDigitanksTeam, CTeam);

public:
								CDigitanksTeam();
								~CDigitanksTeam();

public:
	virtual void				OnAddEntity(CBaseEntity* pEntity);

	void						PreStartTurn();
	void						StartTurn();
	void						PostStartTurn();

	void						MoveTanks();
	void						FireTanks();

	void						AddProduction(size_t iProduction);
	size_t						GetProduction() { return m_iProduction; };

	virtual void				OnDeleted(class CBaseEntity* pEntity);

	size_t						GetNumTanksAlive();

	float						GetEntityVisibility(size_t iHandle);

	// AI stuff
	void						Bot_ExecuteTurn();
	void						Bot_ExpandBase();

	size_t						GetNumTanks() { return m_ahTanks.size(); };
	CDigitank*					GetTank(size_t i) { if (!m_ahTanks.size()) return NULL; return m_ahTanks[i]; };

protected:
	std::vector<CEntityHandle<CDigitank> >	m_ahTanks;

	std::map<size_t, float>		m_aflVisibilities;

	size_t						m_iProduction;

	// AI stuff
	CEntityHandle<CCPU>			m_hPrimaryCPU;
};

#endif