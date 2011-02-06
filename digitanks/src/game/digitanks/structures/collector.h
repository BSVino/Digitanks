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
	virtual float				GetPowerProduced();

	virtual size_t				InitialTurnsToConstruct() { return 4; };
	virtual eastl::string16		GetName() { return L"Power Supply Unit"; };
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

	virtual void				UpdateInfo(eastl::string16& sInfo);

	virtual bool				CanStructureUpgrade();
	virtual void				UpgradeComplete();

	resource_t					GetResourceType() { return RESOURCE_ELECTRONODE; };
	void						SetResource(class CResource* pResource) { m_hResource = pResource; };
	class CResource*			GetResource() { return m_hResource; };
	virtual float				GetPowerProduced();

	virtual size_t				InitialTurnsToConstruct() { return 2; };
	virtual eastl::string16		GetName() { return L"Battery"; };
	virtual unittype_t			GetUnitType() const { return STRUCTURE_BATTERY; };
	virtual unittype_t			GetUpgradeType() const { return STRUCTURE_PSU; };

protected:
	static size_t				s_iUpgradeIcon;
	static size_t				s_iCancelIcon;
};

#endif
