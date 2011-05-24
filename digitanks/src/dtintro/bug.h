#ifndef DT_INTRO_BUG_H
#define DT_INTRO_BUG_H

#include "introtank.h"

class CBug : public CIntroTank
{
	REGISTER_ENTITY_CLASS(CBug, CIntroTank);

public:
	void			Precache();
	virtual void	Spawn();
};

#endif
