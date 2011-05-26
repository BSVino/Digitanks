#ifndef DT_INTRO_BUG_H
#define DT_INTRO_BUG_H

#include "introtank.h"

class CBug : public CIntroTank
{
	REGISTER_ENTITY_CLASS(CBug, CIntroTank);

public:
	void			Precache();
	virtual void	Spawn();

	virtual void	Think();

	DECLARE_ENTITY_INPUT(FireRandomly);
	DECLARE_ENTITY_INPUT(Dissolve);

protected:
	bool			m_bFiringRandomly;
	Vector			m_vecNextAim;
	float			m_flNextAim;
	float			m_flNextFire;
};

#endif
