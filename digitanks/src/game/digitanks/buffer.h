#ifndef DT_BUFFER_H
#define DT_BUFFER_H

#include "structure.h"

class CBuffer : public CStructure
{
	REGISTER_ENTITY_CLASS(CBuffer, CStructure);

public:
	virtual void				OnRender();
};

#endif