#ifndef DT_CPU_H
#define DT_CPU_H

#include "structure.h"

class CCPU : public CSupplier
{
	REGISTER_ENTITY_CLASS(CCPU, CSupplier);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual bool				GetsConcealmentBonus() const { return false; };
	virtual size_t				InitialDataStrength() { return 3200; };
	virtual size_t				BaseDataFlowPerTurn() { return 50; };
	virtual float				TotalHealth() const { return 100; };
	virtual size_t				InitialFleetPoints() const { return 4; };
	virtual size_t				InitialBandwidth() const { return 6; };
	virtual float				InitialPower() const { return 4; };
	virtual bool				IsDataFlowSource() { return true; };
	virtual size_t				EfficientChildren() { return 0; };

	virtual float				GetBoundingRadius() const { return 8; };

	virtual bool				AllowControlMode(controlmode_t eMode) const;

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual bool				NeedsOrders();

	bool						IsPreviewBuildValid() const;

	Vector						GetPreviewBuild() const { return m_vecPreviewBuild; };
	virtual void				SetPreviewBuild(Vector vecPreviewBuild);
	virtual void				SetPreviewStructure(unittype_t ePreviewStructure) { m_ePreviewStructure = ePreviewStructure; };
	virtual unittype_t			GetPreviewStructure() { return m_ePreviewStructure; };
	void						ClearPreviewBuild();

	bool						BeginConstruction();
	void						BeginConstruction(class CNetworkParameters* p);
	void						CancelConstruction();
	void						CancelConstruction(class CNetworkParameters* p);
	float						GetPowerToConstruct(unittype_t eStructure, Vector vecLocation);

	void						BeginRogueProduction();
	void						BeginRogueProduction(class CNetworkParameters* p);
	void						CancelRogueProduction();
	void						CancelRogueProduction(class CNetworkParameters* p);
	bool						IsProducing() { return m_bProducing; }

	virtual void				StartTurn();

	virtual void				OnRender(class CRenderingContext* pContext, bool bTransparent);
	virtual void				PostRender(bool bTransparent);

	virtual void				UpdateInfo(eastl::string16& sInfo);

	virtual void				OnDeleted();

	virtual eastl::string16		GetName() { return L"Central Processing Unit"; };
	virtual unittype_t			GetUnitType() const { return STRUCTURE_CPU; };

protected:
	Vector						m_vecPreviewBuild;

	unittype_t					m_ePreviewStructure;

	CNetworkedVariable<bool>	m_bProducing;
	CNetworkedVariable<size_t>	m_iTurnsToProduceRogue;

	size_t						m_iFanModel;
	float						m_flFanRotation;

	static size_t				s_iCancelIcon;
	static size_t				s_iBuildPSUIcon;
	static size_t				s_iBuildBufferIcon;
	static size_t				s_iBuildLoaderIcon;
	static size_t				s_iBuildInfantryLoaderIcon;
	static size_t				s_iBuildTankLoaderIcon;
	static size_t				s_iBuildArtilleryLoaderIcon;
	static size_t				s_iBuildRogueIcon;
	static size_t				s_iInstallIcon;
	static size_t				s_iInstallPowerIcon;
	static size_t				s_iInstallBandwidthIcon;
	static size_t				s_iInstallFleetSupplyIcon;
};

#endif