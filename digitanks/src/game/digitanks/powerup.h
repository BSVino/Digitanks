#ifndef POWERUP_H
#define POWERUP_H

#include "digitanksentity.h"

class CPowerup : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CPowerup, CDigitanksEntity);

public:
					CPowerup();

public:
	void			Precache();

	virtual EAngle	GetRenderAngles() const;
	virtual void	PreRender();
	virtual void	ModifyContext(class CRenderingContext* pContext);
};

#endif
