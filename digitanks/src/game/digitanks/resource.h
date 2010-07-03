#ifndef DT_RESOURCE_H
#define DT_RESOURCE_H

#include "structure.h"

typedef enum
{
	RESOURCE_ELECTRONODE,
	RESOURCE_BITWELL,
} resource_t;

class CResource : public CStructure
{
	REGISTER_ENTITY_CLASS(CResource, CStructure);

public:
	virtual void				OnRender();

	resource_t					GetResource() { return RESOURCE_ELECTRONODE; };
	size_t						GetProduction() { return 8; };

	static CResource*			FindClosestResource(Vector vecPoint, resource_t eResource);
};

#endif
