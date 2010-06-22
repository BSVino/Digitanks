#ifndef DT_TEAM_H
#define DT_TEAM_H

#include "digitank.h"
#include "baseentity.h"

#include "color.h"
#include <vector>

class CTeam : public CBaseEntity
{
	friend class CDigitanksGame;

	REGISTER_ENTITY_CLASS(CTeam, CBaseEntity);

public:
								CTeam();
								~CTeam();

public:
	void						AddTank(CDigitank* pTank);

	void						StartTurn();

	void						MoveTanks();
	void						FireTanks();

	virtual void				ClientUpdate(int iClient);

	virtual void				OnKilled(class CBaseEntity* pEntity);

	virtual void				OnDeleted(class CBaseEntity* pEntity);

	size_t						GetNumTanksAlive();

	bool						IsPlayerControlled() { return m_bClientControlled; };
	void						SetClient(int iClient);
	void						SetBot();
	int							GetClient() { return m_iClient; };

	Color						GetColor() { return m_clrTeam; };

	size_t						GetNumTanks() { return m_ahTanks.size(); };
	CDigitank*					GetTank(size_t i) { if (!m_ahTanks.size()) return NULL; return (CDigitank*)m_ahTanks[i]; };

protected:
	Color						m_clrTeam;

	std::vector<CEntityHandle<CDigitank> >	m_ahTanks;

	bool						m_bClientControlled;
	int							m_iClient;
};

#endif