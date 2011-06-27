#ifndef DT_INSTRUCTOR_ENTITY_H
#define DT_INSTRUCTOR_ENTITY_H

#include <game/baseentity.h>

// An entity which can take entity inputs and passes through to the actual instructor
class CInstructorEntity : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CInstructorEntity, CBaseEntity);

public:
	virtual void				Spawn();

	DECLARE_ENTITY_INPUT(DisplayLesson);
};

#endif
