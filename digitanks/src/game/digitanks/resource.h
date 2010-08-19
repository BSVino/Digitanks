#ifndef DT_RESOURCE_H
#define DT_RESOURCE_H

#include "structure.h"
#include "collector.h"

class CResource : public CStructure
{
	REGISTER_ENTITY_CLASS(CResource, CStructure);

public:
	virtual float				GetBoundingRadius() const { return 2; };

	virtual void				Precache();
	virtual void				Spawn();

	virtual void				UpdateInfo(std::string& sInfo);

	virtual void				ModifyContext(class CRenderingContext* pContext);

	resource_t					GetResource() { return RESOURCE_ELECTRONODE; };
	size_t						GetProduction() { return m_iProduction; };
	void						SetProduction(size_t iProduction) { m_iProduction = iProduction; };

	bool						HasCollector() { return m_hCollector != NULL; }
	class CCollector*			GetCollector() { return m_hCollector; }
	void						SetCollector(class CCollector* pCollector) { m_hCollector = pCollector; }

	virtual const char*			GetName() { return "Electronode"; };
	virtual unittype_t			GetUnitType() { return STRUCTURE_ELECTRONODE; };

	static CResource*			FindClosestResource(Vector vecPoint, resource_t eResource);

protected:
	CEntityHandle<CCollector>	m_hCollector;

	size_t						m_iProduction;
};

#endif
