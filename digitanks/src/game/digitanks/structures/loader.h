#ifndef DT_LOADER_H
#define DT_LOADER_H

#include "structure.h"

typedef enum
{
	BUILDUNIT_INFANTRY,
	BUILDUNIT_TANK,
	BUILDUNIT_ARTILLERY,
	BUILDUNIT_SCOUT,
} buildunit_t;

extern size_t g_aiTurnsToLoad[];

class CLoader : public CStructure
{
	REGISTER_ENTITY_CLASS(CLoader, CStructure);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				OnTeamChange();

	virtual void				StartTurn();

	virtual void				PostRender(bool bTransparent);

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual bool				NeedsOrders();

	virtual void				UpdateInfo(eastl::string16& sInfo);

	void						BeginProduction();
	void						BeginProduction(class CNetworkParameters* p);
	void						CancelProduction();
	void						CancelProduction(class CNetworkParameters* p);
	bool						IsProducing() { return m_bProducing; };
	void						AddProduction(size_t iProduction) { m_iProductionStored += iProduction; }
	size_t						GetUnitProductionCost();

	virtual void				InstallUpdate(updatetype_t eUpdate);
	virtual void				InstallComplete();
	virtual bool				HasUpdatesAvailable();

	size_t						GetFleetPointsRequired();
	bool						HasEnoughFleetPoints();

	size_t						GetTurnsToProduce();

	void						SetBuildUnit(buildunit_t eBuildUnit);
	buildunit_t					GetBuildUnit() const { return m_eBuildUnit.Get(); };

	virtual eastl::string16		GetName();
	virtual unittype_t			GetUnitType();
	virtual size_t				ConstructionCost() const { return GetLoaderConstructionCost(GetBuildUnit()); };
	virtual float				TotalHealth() const { return 70; };

	static size_t				GetUnitProductionCost(buildunit_t eBuildUnit);
	static size_t				GetLoaderConstructionCost(buildunit_t eBuildUnit);

protected:
	CNetworkedVariable<buildunit_t> m_eBuildUnit;
	CNetworkedVariable<size_t>	m_iBuildUnitModel;

	CNetworkedVariable<bool>	m_bProducing;
	CNetworkedVariable<size_t>	m_iProductionStored;

	CNetworkedVariable<size_t>	m_iTankAttack;
	CNetworkedVariable<size_t>	m_iTankDefense;
	CNetworkedVariable<size_t>	m_iTankMovement;
	CNetworkedVariable<size_t>	m_iTankHealth;
	CNetworkedVariable<size_t>	m_iTankRange;

	static size_t				s_iCancelIcon;
	static size_t				s_iInstallIcon;
	static size_t				s_iInstallAttackIcon;
	static size_t				s_iInstallDefenseIcon;
	static size_t				s_iInstallMovementIcon;
	static size_t				s_iInstallRangeIcon;
	static size_t				s_iInstallHealthIcon;
	static size_t				s_iBuildInfantryIcon;
	static size_t				s_iBuildTankIcon;
	static size_t				s_iBuildArtilleryIcon;
};

#endif
