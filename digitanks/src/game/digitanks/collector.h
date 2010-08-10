#ifndef DT_COLLECTOR_H
#define DT_COLLECTOR_H

#include "resource.h"

class CCollector : public CStructure
{
	REGISTER_ENTITY_CLASS(CCollector, CStructure);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual void				UpdateInfo(std::string& sInfo);

	resource_t					GetResourceType() { return RESOURCE_ELECTRONODE; };
	void						SetResource(class CResource* pResource) { m_hResource = pResource; };
	class CResource*			GetResource() { return m_hResource; };

	virtual const char*			GetName() { return "Power Supply Unit"; };
	virtual unittype_t			GetUnitType() { return STRUCTURE_PSU; };
	virtual size_t				ConstructionCost() const { return 30; };

protected:
	CEntityHandle<CResource>	m_hResource;
};

#endif
