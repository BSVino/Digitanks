#ifndef DT_INTRO_DIGITANK_H
#define DT_INTRO_DIGITANK_H

#include "introtank.h"

class CDigitank : public CIntroTank
{
	REGISTER_ENTITY_CLASS(CDigitank, CIntroTank);

public:
	void			Precache();
	virtual void	Spawn();

	virtual void	Think();

	DECLARE_ENTITY_INPUT(FireAt);

protected:
	Vector			m_vecNextAim;
	float			m_flNextFire;
	CEntityHandle<CBaseEntity> m_hTarget;
};

#endif
