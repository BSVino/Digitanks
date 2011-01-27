#ifndef DT_STRUCTURE_H
#define DT_STRUCTURE_H

#include <digitanks/dt_common.h>
#include <digitanks/selectable.h>
#include <digitanks/supplyline.h>
#include <digitanks/updates.h>

typedef enum
{
	RESOURCE_ELECTRONODE,
	RESOURCE_BITWELL,
} resource_t;

class CStructure : public CSelectable
{
	REGISTER_ENTITY_CLASS(CStructure, CSelectable);

public:
								CStructure();

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				Think();

	virtual float				GetBoundingRadius() const { return 5; };

	virtual void				OnTeamChange();

	virtual void				StartTurn();
	virtual void				FindGround();

	virtual void				PostRender(bool bTransparent);

	void						BeginConstruction(Vector vecConstructionOrigin);
	void						BeginStructureConstruction(CNetworkParameters* p);
	virtual void				CompleteConstruction();
	virtual size_t				GetTurnsToConstruct() { return 1; };
	bool						IsConstructing() const { return m_bConstructing.Get(); };
	void						SetConstructing(bool bConstructing) { m_bConstructing = bConstructing; };
	virtual size_t				GetTurnsRemainingToConstruct() { return m_iTurnsToConstruct; };

	virtual void				InstallUpdate(size_t x, size_t y);
	void						InstallUpdate(CNetworkParameters* p);
	virtual void				DownloadComplete(size_t x, size_t y);

	virtual bool				CanStructureUpgrade() { return false; };
	virtual unittype_t			GetUpgradeType() const { return UNITTYPE_UNDEFINED; };
	void						BeginUpgrade();
	void						BeginUpgrade(CNetworkParameters* p);
	virtual void				UpgradeComplete() {};
	size_t						GetTurnsToUpgrade();
	bool						IsUpgrading() { return m_bUpgrading; };
	virtual size_t				GetTurnsRemainingToUpgrade() { return m_iTurnsToUpgrade; };

	virtual bool				NeedsOrders();

	virtual void				SetSupplier(class CSupplier* pSupplier);
	virtual class CSupplier*	GetSupplier() { if (m_hSupplier == NULL) return NULL; return m_hSupplier; };

	virtual class CSupplyLine*	GetSupplyLine() { if (m_hSupplyLine == NULL) return NULL; return m_hSupplyLine; };

	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent);

	virtual void				OnDeleted();
	virtual void				OnDeleted(CBaseEntity* pEntity) { BaseClass::OnDeleted(); };

	virtual void				ClientUpdate(int iClient);

	virtual void				OnSerialize(std::ostream& o);
	virtual bool				OnUnserialize(std::istream& i);

	virtual float				HealthRechargeRate() const { return 1.0f; };
	virtual float				VisibleRange() const;
	virtual float				BaseVisibleRange() const { return 50; };
	virtual float				ConstructionCost() const;
	virtual float				UpgradeCost() const;
	virtual float				TotalHealth() const { return 50; };

	virtual size_t				InitialFleetPoints() const { return 0; };
	virtual size_t				FleetPoints() const { return m_iFleetSupply.Get(); };
	virtual void				AddFleetPoints(size_t iAddPoints) { m_iFleetSupply += iAddPoints; };

	virtual size_t				InitialBandwidth() const { return 0; };
	virtual size_t				Bandwidth() const { return m_iBandwidth.Get(); };
	virtual void				AddBandwidth(size_t iAddBandwidth) { m_iBandwidth += iAddBandwidth; };

	virtual float				InitialPower() const { return 0; };
	virtual float				Power() const { return m_flPowerProduced.Get(); };

	virtual size_t				InitialEnergyBonus() const { return 4; };
	virtual size_t				EnergyBonus() const { return m_iEnergyBonus.Get(); };
	virtual void				AddEnergyBonus(size_t e) { m_iEnergyBonus += e; };

	virtual float				InitialRechargeBonus() const { return 0.5f; };
	virtual float				RechargeBonus() const { return m_flRechargeBonus.Get(); };
	virtual void				AddRechargeBonus(float r) { m_flRechargeBonus += r; };

	// AI stuff
	void						AddDefender(class CDigitank* pTank);
	void						RemoveDefender(class CDigitank* pTank);
	size_t						GetNumLivingDefenders();

protected:
	CNetworkedVariable<bool>	m_bConstructing;
	CNetworkedVariable<size_t>	m_iTurnsToConstruct;

	CNetworkedVariable<bool>	m_bUpgrading;
	CNetworkedVariable<size_t>	m_iTurnsToUpgrade;

	CNetworkedHandle<CSupplier>		m_hSupplier;
	CNetworkedHandle<CSupplyLine>	m_hSupplyLine;

	CNetworkedVariable<size_t>	m_iFleetSupply;
	CNetworkedVariable<size_t>	m_iBandwidth;
	CNetworkedVariable<float>	m_flPowerProduced;
	CNetworkedVariable<size_t>	m_iEnergyBonus;
	CNetworkedVariable<float>	m_flRechargeBonus;

	size_t						m_iScaffolding;
	CNetworkedVariable<float>	m_flScaffoldingSize;

	float						m_flConstructionStartTime;

	typedef struct
	{
		CEntityHandle<CDigitank>	m_hDefender;
		float						m_flPosition;
	} defender_t;

	// AI stuff
	eastl::vector<defender_t>	m_aoDefenders;
};

class CSupplier : public CStructure
{
	REGISTER_ENTITY_CLASS(CSupplier, CStructure);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual float				GetRenderRadius() const { return GetBoundingRadius() + GetDataFlowRadius() + 5; };

	virtual void				ClientEnterGame();

	virtual void				CompleteConstruction();

	virtual size_t				InitialEnergyBonus() const { return 1; };
	virtual float				InitialRechargeBonus() const { return 0.5f; };

	virtual size_t				InitialDataStrength() { return 100; };
	virtual size_t				BaseDataFlowPerTurn() { return 20; };
	virtual bool				IsDataFlowSource() { return false; };
	virtual float				GetDataFlowRate();
	float						GetDataFlowRadius() const;
	float						GetDataFlow(Vector vecPoint);
	static float				GetDataFlow(Vector vecPoint, CTeam* pTeam, CSupplier* pIgnore = NULL);
	void						CalculateDataFlow();
	void						GiveDataStrength(size_t iStrength) { m_iDataStrength += iStrength; };
	virtual size_t				EfficientChildren() { return 2; };

	float						GetChildEfficiency();

	virtual void				OnTeamChange();

	virtual void				StartTurn();

	// Even if we're invisible our tendrils might not be, we should still render those.
	virtual bool				ShouldRender() const { return m_bShouldRender; };
	virtual void				PostRender(bool bTransparent);

	void						UpdateTendrils();
	void						BeginTendrilGrowth();

	void						AddChild(CStructure* pChild);
	void						AddChild(CNetworkParameters* p);
	void						RemoveChild(CStructure* pChild);
	void						RemoveChild(CNetworkParameters* p);
	size_t						GetNumChildren() { return m_ahChildren.size(); };
	CStructure*					GetChild(size_t i) { return m_ahChildren[i]; };

	virtual void				OnDeleted(class CBaseEntity* pEntity);

	virtual float				BaseVisibleRange() const;

	virtual float				BuildableArea() const;

	static CSupplier*			FindClosestSupplier(CBaseEntity* pUnit);
	static CSupplier*			FindClosestSupplier(Vector vecPoint, class CTeam* pTeam);

protected:
	CNetworkedVariable<size_t>	m_iDataStrength;
	CNetworkedVariable<float>	m_flBonusDataFlow;

	class CTendril
	{
	public:
		float					m_flLength;
		Vector					m_vecEndPoint;
		float					m_flScale;
		float					m_flOffset;
		float					m_flSpeed;
	};

	eastl::vector<CTendril>		m_aTendrils;
	eastl::vector<CEntityHandle<CStructure> >	m_ahChildren;

	size_t						m_iTendrilsCallList;
	float						m_flTendrilGrowthStartTime;

	bool						m_bShouldRender;

	static size_t				s_iTendrilBeam;
};

#endif
