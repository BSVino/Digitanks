#ifndef DT_LOADER_H
#define DT_LOADER_H

#include "structure.h"

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

	virtual void				DrawQueue(int x, int y, int w, int h);

	void						BeginProduction();
	void						BeginProduction(class CNetworkParameters* p);
	void						CompleteProduction();
	bool						IsProducing() { return m_bProducing; };
	float						GetUnitProductionCost();
	size_t						GetTurnsRemainingToProduce() { return m_iTurnsToProduce; };

	virtual void				InstallUpdate(size_t x, size_t y);

	size_t						GetFleetPointsRequired();
	bool						HasEnoughFleetPoints();
	bool						HasEnoughPower();

	virtual size_t				InitialTurnsToConstruct();
	size_t						GetTurnsToProduce();

	void						SetBuildUnit(unittype_t eBuildUnit);
	unittype_t					GetBuildUnit() const { return m_eBuildUnit; };

	virtual eastl::string16		GetEntityName() const;
	virtual unittype_t			GetUnitType() const;
	virtual float				TotalHealth() const { return 700; };

protected:
	CNetworkedVariable<unittype_t> m_eBuildUnit;
	CNetworkedVariable<size_t>	m_iBuildUnitModel;

	CNetworkedVariable<bool>	m_bProducing;
	CNetworkedVariable<size_t>	m_iTurnsToProduce;

	CNetworkedVariable<size_t>	m_iTankAttack;
	CNetworkedVariable<size_t>	m_iTankDefense;
	CNetworkedVariable<size_t>	m_iTankMovement;
	CNetworkedVariable<size_t>	m_iTankHealth;
	CNetworkedVariable<size_t>	m_iTankRange;
};

#endif
