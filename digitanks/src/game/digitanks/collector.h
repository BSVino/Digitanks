#ifndef DT_COLLECTOR_H
#define DT_COLLECTOR_H

#include "resource.h"

class CCollector : public CStructure
{
	REGISTER_ENTITY_CLASS(CCollector, CStructure);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual void				UpdateInfo(std::wstring& sInfo);

	resource_t					GetResourceType() { return RESOURCE_ELECTRONODE; };
	void						SetResource(class CResource* pResource) { m_hResource = pResource; };
	class CResource*			GetResource() { return m_hResource; };
	virtual size_t				GetProduction() { return 4; };

	virtual const wchar_t*		GetName() { return L"Power Supply Unit"; };
	virtual unittype_t			GetUnitType() { return STRUCTURE_PSU; };
	virtual size_t				ConstructionCost() const { return GetCollectorConstructionCost(); };

	static size_t				GetCollectorConstructionCost() { return 60; };

protected:
	CEntityHandle<CResource>	m_hResource;
};

class CBattery : public CCollector
{
	REGISTER_ENTITY_CLASS(CBattery, CCollector);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual void				UpdateInfo(std::wstring& sInfo);

	virtual bool				CanStructureUpgrade();
	virtual void				UpgradeComplete();

	resource_t					GetResourceType() { return RESOURCE_ELECTRONODE; };
	void						SetResource(class CResource* pResource) { m_hResource = pResource; };
	class CResource*			GetResource() { return m_hResource; };
	virtual size_t				GetProduction() { return 2; };

	virtual const wchar_t*		GetName() { return L"Battery"; };
	virtual unittype_t			GetUnitType() { return STRUCTURE_BATTERY; };
	virtual size_t				ConstructionCost() const { return GetBatteryConstructionCost(); };

	static size_t				GetBatteryConstructionCost() { return 15; };

protected:
	static size_t				s_iCancelIcon;
};

#endif
