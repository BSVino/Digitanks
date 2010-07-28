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

	class CSelectable*			GetCurrentSelection();
	class CDigitank*			GetCurrentTank();
	class CStructure*			GetCurrentStructure();
	size_t						GetCurrentSelectionId();
	bool						IsCurrentSelection(const class CSelectable* pEntity);
	void						SetCurrentSelection(CSelectable* pCurrent);
	void						NextTank();

	void						StartTurn();

	void						MoveTanks();
	void						FireTanks();

	void						AddProduction(size_t iProduction);
	void						AddProducer() { m_iLoadersProducing++; };
	float						GetProductionPerLoader();
	size_t						GetTotalProduction() { return m_iProduction; };

	void						CountFleetPoints();
	size_t						GetTotalFleetPoints() { return m_iTotalFleetPoints; };
	size_t						GetUsedFleetPoints() { return m_iUsedFleetPoints; };
	size_t						GetUnusedFleetPoints() { return GetTotalFleetPoints() - GetUsedFleetPoints(); };

	virtual void				OnDeleted(class CBaseEntity* pEntity);

	size_t						GetNumTanksAlive();

	float						GetEntityVisibility(size_t iHandle);

	// AI stuff
	void						Bot_ExpandBase();
	void						Bot_BuildUnits();
	void						Bot_AssignDefenders();
	void						Bot_ExecuteTurn();
	CSupplier*					FindUnusedSupplier(size_t iDependents = ~0, bool bNoSuppliers = true);
	void						BuildCollector(CSupplier* pSupplier, class CResource* pResource);

	size_t						GetNumTanks() { return m_ahTanks.size(); };
	class CDigitank*			GetTank(size_t i) { if (!m_ahTanks.size()) return NULL; return m_ahTanks[i]; };

protected:
	std::vector<CEntityHandle<CDigitank> >	m_ahTanks;

	size_t						m_iCurrentSelection;

	std::map<size_t, float>		m_aflVisibilities;

	size_t						m_iProduction;
	size_t						m_iLoadersProducing;

	size_t						m_iTotalFleetPoints;
	size_t						m_iUsedFleetPoints;

	// AI stuff
	CEntityHandle<CCPU>			m_hPrimaryCPU;
	size_t						m_iBuildPosition;
	std::vector<CEntityHandle<CDigitank> >	m_ahAttackTeam;
	Vector						m_vecExplore;

	bool						m_bLKV;
	Vector						m_vecLKV;
};

#endif