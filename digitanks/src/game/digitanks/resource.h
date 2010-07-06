#ifndef DT_RESOURCE_H
#define DT_RESOURCE_H

#include "structure.h"
#include "collector.h"

class CResource : public CStructure
{
	REGISTER_ENTITY_CLASS(CResource, CStructure);

public:
	virtual void				OnRender();

	resource_t					GetResource() { return RESOURCE_ELECTRONODE; };
	size_t						GetProduction() { return 8; };

	bool						HasCollector() { return m_hCollector != NULL; }
	class CCollector*			GetCollector() { return m_hCollector; }
	void						SetCollector(class CCollector* pCollector) { m_hCollector = pCollector; }

	static CResource*			FindClosestResource(Vector vecPoint, resource_t eResource);

protected:
	CEntityHandle<CCollector>	m_hCollector;
};

#endif
