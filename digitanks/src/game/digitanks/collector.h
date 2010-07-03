#ifndef DT_COLLECTOR_H
#define DT_COLLECTOR_H

#include "resource.h"

class CCollector : public CStructure
{
	REGISTER_ENTITY_CLASS(CCollector, CStructure);

public:
	virtual void				PreStartTurn();

	virtual void				OnRender();

	resource_t					GetResource() { return RESOURCE_ELECTRONODE; };
	void						SetResource(CResource* pResource) { m_hResource = pResource; };

protected:
	CEntityHandle<CResource>	m_hResource;
};

#endif
