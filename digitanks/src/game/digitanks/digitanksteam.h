#ifndef DT_DIGITANKSTEAM_H
#define DT_DIGITANKSTEAM_H

#include <game/team.h>

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

	virtual void				OnDeleted(class CBaseEntity* pEntity);

	size_t						GetNumTanksAlive();

	size_t						GetNumTanks() { return m_ahTanks.size(); };
	class CDigitank*			GetTank(size_t i) { if (!m_ahTanks.size()) return NULL; return (class CDigitank*)m_ahTanks[i]; };

protected:
	std::vector<CEntityHandle<class CDigitank> >	m_ahTanks;
};

#endif