#ifndef POWERUP_H
#define POWERUP_H

#include "digitanksentity.h"

typedef enum {
	POWERUP_BONUS,
	POWERUP_AIRSTRIKE,
} powerup_type_t;

class CPowerup : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CPowerup, CDigitanksEntity);

public:
	virtual float	GetBoundingRadius() const { return 4; };

	void			Precache();
	virtual void	Spawn();

	virtual EAngle	GetRenderAngles() const;
	virtual void	PreRender();
	virtual void	ModifyContext(class CRenderingContext* pContext);

	powerup_type_t	GetPowerupType() { return m_ePowerupType; }

protected:
	powerup_type_t	m_ePowerupType;
};

#endif
