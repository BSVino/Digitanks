#ifndef DT_INTRO_GENERAL_H
#define DT_INTRO_GENERAL_H

#include <game/baseentity.h>

class CIntroGeneral : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CIntroGeneral, CBaseEntity);

public:
	DECLARE_ENTITY_INPUT(Deploy);
	DECLARE_ENTITY_INPUT(RetryDebugging);
	DECLARE_ENTITY_INPUT(GiveUpDebugging);

protected:
};

#endif
