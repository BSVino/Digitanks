#ifndef DT_COLLECTOR_H
#define DT_COLLECTOR_H

#include "resource.h"

class CCollector : public CStructure
{
	REGISTER_ENTITY_CLASS(CCollector, CStructure);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual void				UpdateInfo(eastl::string16& sInfo);

	resource_t					GetResourceType() { return RESOURCE_ELECTRONODE; };
	void						SetResource(class CResource* pResource) { m_hResource = pResource; };
	class CResource*			GetResource() { return m_hResource; };
	virtual size_t				GetProduction();

	virtual eastl::string16		GetName() { return L"Power Supply Unit"; };
	virtual unittype_t			GetUnitType() { return STRUCTURE_PSU; };
	virtual size_t				ConstructionCost() const { return GetCollectorConstructionCost(); };

	static size_t				GetCollectorConstructionCost() { return 60; };

protected:
	CNetworkedHandle<CResource>	m_hResource;
};

class CBattery : public CCollector
{
	REGISTER_ENTITY_CLASS(CBattery, CCollector);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual void				UpdateInfo(eastl::string16& sInfo);

	virtual bool				CanStructureUpgrade();
	virtual void				UpgradeComplete();

	resource_t					GetResourceType() { return RESOURCE_ELECTRONODE; };
	void						SetResource(class CResource* pResource) { m_hResource = pResource; };
	class CResource*			GetResource() { return m_hResource; };
	virtual size_t				GetProduction();

	virtual eastl::string16		GetName() { return L"Battery"; };
	virtual unittype_t			GetUnitType() { return STRUCTURE_BATTERY; };
	virtual size_t				ConstructionCost() const { return GetBatteryConstructionCost(); };
	virtual size_t				UpgradeCost() const { return GetBatteryUpgradeCost(); };

	static size_t				GetBatteryConstructionCost() { return 15; };
	static size_t				GetBatteryUpgradeCost() { return 50; };

protected:
	static size_t				s_iUpgradeIcon;
	static size_t				s_iCancelIcon;
};

#endif
