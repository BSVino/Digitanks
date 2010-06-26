#ifndef DT_CPU_H
#define DT_CPU_H

#include "structure.h"

class CCPU : public CStructure
{
	REGISTER_ENTITY_CLASS(CCPU, CStructure);

public:
	virtual void				OnRender();

};

#endif