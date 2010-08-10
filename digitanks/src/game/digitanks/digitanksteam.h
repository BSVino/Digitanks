#ifndef DT_DIGITANKSTEAM_H
#define DT_DIGITANKSTEAM_H

#include <game/team.h>
#include "digitank.h"
#include "updates.h"

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

	void						CountBandwidth();
	size_t						GetBandwidth() { return m_iBandwidth; };

	virtual void				OnDeleted(class CBaseEntity* pEntity);

	size_t						GetNumTanksAlive();

	float						GetEntityVisibility(size_t iHandle);
	float						GetVisibilityAtPoint(Vector vecPoint);

	void						DownloadUpdate(int iX, int iY, bool bCheck = true);
	size_t						GetUpdateDownloaded() { return m_iUpdateDownloaded; };
	size_t						GetUpdateSize();
	void						DownloadComplete();
	bool						HasDownloadedUpdate(int iX, int iY);
	bool						CanDownloadUpdate(int iX, int iY);
	bool						IsDownloading(int iX, int iY);
	class CUpdateItem*			GetUpdateInstalling();
	size_t						GetTurnsToInstall();

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

	int							m_iCurrentUpdateX;
	int							m_iCurrentUpdateY;
	bool						m_abUpdates[UPDATE_GRID_SIZE][UPDATE_GRID_SIZE];
	size_t						m_iUpdateDownloaded;
	size_t						m_iBandwidth;
};

#endif