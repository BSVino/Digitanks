#ifndef DT_STRUCTURE_H
#define DT_STRUCTURE_H

#include "dt_common.h"
#include "selectable.h"
#include "supplyline.h"
#include "updates.h"

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

	virtual float				GetBoundingRadius() const { return 5; };

	virtual void				OnTeamChange();

	virtual void				StartTurn();
	virtual void				FindGround();

	virtual void				PostRender();

	void						BeginConstruction();
	virtual void				CompleteConstruction();
	size_t						GetTurnsToConstruct();
	bool						IsConstructing() { return m_bConstructing; };
	size_t						GetProductionToConstruct() { return m_iProductionToConstruct; };
	void						AddProduction(size_t iProduction);
	size_t						GetTurnsToConstruct(size_t iPower);

	virtual void				InstallUpdate(updatetype_t eUpdate);
	virtual void				InstallComplete();
	void						CancelInstall();
	size_t						GetTurnsToInstall(class CUpdateItem* pItem);
	size_t						GetTurnsToInstall();
	bool						IsInstalling() { return m_bInstalling; };
	size_t						GetProductionToInstall() { return m_iProductionToInstall; };
	int							GetFirstUninstalledUpdate(updatetype_t eUpdate);
	class CUpdateItem*			GetUpdateInstalling();
	virtual bool				HasUpdatesAvailable() { return false; };
	virtual void				DownloadComplete(class CUpdateItem* pItem);
	size_t						GetUpdatesScore();

	virtual bool				CanStructureUpgrade() { return false; };
	void						BeginUpgrade();
	void						CancelUpgrade();
	virtual void				UpgradeComplete() {};
	size_t						GetTurnsToUpgrade();
	bool						IsUpgrading() { return m_bUpgrading; };
	size_t						GetProductionToUpgrade() { return m_iProductionToUpgrade; };

	virtual bool				NeedsOrders();

	virtual void				SetSupplier(class CSupplier* pSupplier);
	virtual class CSupplier*	GetSupplier() { if (m_hSupplier == NULL) return NULL; return m_hSupplier; };

	virtual class CSupplyLine*	GetSupplyLine() { if (m_hSupplyLine == NULL) return NULL; return m_hSupplyLine; };

	virtual void				ModifyContext(class CRenderingContext* pContext);

	virtual void				OnDeleted();
	virtual void				OnDeleted(CBaseEntity* pEntity) { BaseClass::OnDeleted(); };

	virtual float				HealthRechargeRate() const { return 1.0f; };
	virtual float				VisibleRange() const { return 50; };
	virtual size_t				ConstructionCost() const { return 20; };
	virtual float				TotalHealth() const { return 50; };

	virtual size_t				InitialFleetPoints() const { return 0; };
	virtual size_t				FleetPoints() const { return m_iFleetSupply; };
	virtual void				AddFleetPoints(size_t iAddPoints) { m_iFleetSupply += iAddPoints; };

	virtual size_t				InitialBandwidth() const { return 0; };
	virtual size_t				Bandwidth() const { return m_iBandwidth; };
	virtual void				AddBandwidth(size_t iAddBandwidth) { m_iBandwidth += iAddBandwidth; };

	virtual size_t				InitialPower() const { return 0; };
	virtual size_t				Power() const { return m_iPower; };

	virtual size_t				InitialEnergyBonus() const { return 4; };
	virtual size_t				EnergyBonus() const { return m_iEnergyBonus; };

	virtual float				InitialRechargeBonus() const { return 0.5f; };
	virtual float				RechargeBonus() const { return m_flRechargeBonus; };

	// AI stuff
	void						AddDefender(class CDigitank* pTank);
	size_t						GetNumLivingDefenders();

protected:
	bool						m_bConstructing;
	size_t						m_iProductionToConstruct;

	bool						m_bInstalling;
	updatetype_t				m_eInstallingType;
	int							m_iInstallingUpdate;
	size_t						m_iProductionToInstall;

	bool						m_bUpgrading;
	size_t						m_iProductionToUpgrade;

	CEntityHandle<CSupplier>		m_hSupplier;
	CEntityHandle<CSupplyLine>		m_hSupplyLine;

	size_t						m_iFleetSupply;
	size_t						m_iBandwidth;
	size_t						m_iPower;
	size_t						m_iEnergyBonus;
	float						m_flRechargeBonus;

	std::map<size_t, std::vector<class CUpdateItem*> >	m_apUpdates;
	std::map<size_t, size_t>		m_aiUpdatesInstalled;

	size_t						m_iScaffolding;
	float						m_flScaffoldingSize;

	float						m_flConstructionStartTime;

	typedef struct
	{
		CEntityHandle<CDigitank>	m_hDefender;
		float						m_flPosition;
	} defender_t;

	// AI stuff
	std::vector<defender_t>		m_aoDefenders;
};

class CSupplier : public CStructure
{
	REGISTER_ENTITY_CLASS(CSupplier, CStructure);

public:
	virtual void				Precache();
	virtual void				Spawn();

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

	float						GetChildEfficiency();

	virtual void				OnTeamChange();

	virtual void				StartTurn();

	// Even if we're invisible our tendrils might not be, we should still render those.
	virtual bool				ShouldRender() const { return true; };
	virtual void				PostRender();

	void						UpdateTendrils();
	void						BeginTendrilGrowth();

	void						AddChild(CStructure* pChild);
	void						RemoveChild(CStructure* pChild);
	size_t						GetNumChildren() { return m_ahChildren.size(); };
	CStructure*					GetChild(size_t i) { return m_ahChildren[i]; };

	virtual void				OnDeleted(class CBaseEntity* pEntity);

	virtual float				VisibleRange() const;

	static CSupplier*			FindClosestSupplier(CBaseEntity* pUnit);
	static CSupplier*			FindClosestSupplier(Vector vecPoint, class CTeam* pTeam);

protected:
	size_t						m_iDataStrength;
	float						m_flBonusDataFlow;

	class CTendril
	{
	public:
		float					m_flLength;
		Vector					m_vecEndPoint;
		float					m_flScale;
		float					m_flOffset;
		float					m_flSpeed;
	};

	std::vector<CTendril>		m_aTendrils;
	std::vector<CEntityHandle<CStructure> >	m_ahChildren;

	size_t						m_iTendrilsCallList;
	float						m_flTendrilGrowthStartTime;

	static size_t				s_iTendrilBeam;
};

#endif
