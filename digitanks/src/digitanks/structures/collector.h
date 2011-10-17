#ifndef DT_COLLECTOR_H
#define DT_COLLECTOR_H

#include "resource.h"

class CCollector : public CStructure
{
	REGISTER_ENTITY_CLASS(CCollector, CStructure);

public:
	virtual void				Spawn();
	virtual void				Precache();
	virtual void				ClientSpawn();

	virtual void				UpdateInfo(tstring& sInfo);

	resource_t					GetResourceType() { return RESOURCE_ELECTRONODE; };
	void						SetResource(class CResource* pResource) { m_hResource = pResource; };
	class CResource*			GetResource() { return m_hResource; };
	virtual float				GetPowerProduced() const;

	virtual size_t				InitialTurnsToConstruct() { return 2; };
	virtual tstring				GetEntityName() const { return _T("Power Supply Unit"); };
	virtual unittype_t			GetUnitType() const { return STRUCTURE_PSU; };

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

	virtual void				UpdateInfo(tstring& sInfo);

	virtual bool				CanStructureUpgrade();
	virtual void				UpgradeComplete();

	resource_t					GetResourceType() { return RESOURCE_ELECTRONODE; };
	void						SetResource(class CResource* pResource) { m_hResource = pResource; };
	class CResource*			GetResource() { return m_hResource; };
	virtual float				GetPowerProduced() const;

	virtual size_t				InitialTurnsToConstruct() { return 1; };
	virtual tstring				GetEntityName() const { return _T("Capacitor"); };
	virtual unittype_t			GetUnitType() const { return STRUCTURE_BATTERY; };
	virtual unittype_t			GetUpgradeType() const { return STRUCTURE_PSU; };
};

#endif
