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
	double			m_flNextAim;
	double			m_flNextFire;
};

#endif
