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

	class CSelectable*			GetPrimarySelection();
	class CDigitank*			GetPrimarySelectionTank();
	class CStructure*			GetPrimarySelectionStructure();
	size_t						GetPrimarySelectionId();
	void						SetPrimarySelection(const CSelectable* pCurrent);
	bool						IsPrimarySelection(const class CSelectable* pEntity);
	void						AddToSelection(const CSelectable* pCurrent);
	bool						IsSelected(const class CSelectable* pEntity);
	size_t						GetNumSelected() { return m_aiCurrentSelection.size(); }

	void						StartTurn();
	void						EndTurn();

	void						CountProducers();
	void						AddProduction(size_t iProduction);
	void						AddProducer() { m_iLoadersProducing++; };
	size_t						GetNumProducers() { return m_iLoadersProducing; };
	float						GetProductionPerLoader();
	size_t						GetTotalProduction() { return m_iProduction; };

	void						CountFleetPoints();
	size_t						GetTotalFleetPoints() { return m_iTotalFleetPoints; };
	size_t						GetUsedFleetPoints() { return m_iUsedFleetPoints; };
	size_t						GetUnusedFleetPoints() { return GetTotalFleetPoints() - GetUsedFleetPoints(); };

	void						CountScore();
	size_t						GetScore() { return m_iScore; };

	void						CountBandwidth();
	size_t						GetBandwidth() { return m_iBandwidth; };

	virtual void				OnDeleted(class CBaseEntity* pEntity);

	size_t						GetNumTanksAlive();

	void						CalculateVisibility();
	float						GetEntityVisibility(size_t iHandle);
	float						GetVisibilityAtPoint(Vector vecPoint);

	void						DownloadUpdate(int iX, int iY, bool bCheck = true);
	size_t						GetUpdateDownloaded() { return m_iUpdateDownloaded; };
	size_t						GetUpdateSize();
	void						DownloadComplete(bool bInformMembers = true);
	bool						HasDownloadedUpdate(int iX, int iY);
	bool						CanDownloadUpdate(int iX, int iY);
	bool						IsDownloading(int iX, int iY);
	class CUpdateItem*			GetUpdateDownloading();
	size_t						GetTurnsToDownload();

	bool						CanBuildMiniBuffers();
	bool						CanBuildBuffers();
	bool						CanBuildBatteries();
	bool						CanBuildPSUs();
	bool						CanBuildLoaders();
	bool						CanBuildInfantryLoaders();
	bool						CanBuildTankLoaders();
	bool						CanBuildArtilleryLoaders();

	// AI stuff
	void						Bot_DownloadUpdates();
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

	std::vector<size_t>			m_aiCurrentSelection;

	std::map<size_t, float>		m_aflVisibilities;

	size_t						m_iProduction;
	size_t						m_iLoadersProducing;

	size_t						m_iTotalFleetPoints;
	size_t						m_iUsedFleetPoints;

	size_t						m_iScore;

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

	bool						m_bCanBuildBuffers;
	bool						m_bCanBuildPSUs;
	bool						m_bCanBuildInfantryLoaders;
	bool						m_bCanBuildTankLoaders;
	bool						m_bCanBuildArtilleryLoaders;
};

#endif