#ifndef POWERUP_H
#define POWERUP_H

#include "digitanksentity.h"

typedef enum {
	POWERUP_BONUS,
	POWERUP_AIRSTRIKE,
	POWERUP_TANK,
	POWERUP_MISSILEDEFENSE,
} powerup_type_t;

class CPowerup : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CPowerup, CDigitanksEntity);

public:
	virtual float	GetBoundingRadius() const { return 4; };

	void			Precache();
	virtual void	Spawn();

	virtual EAngle	GetRenderAngles() const;
	virtual Vector	GetRenderOrigin() const;
	virtual void	PreRender(bool bTransparent);
	virtual void	ModifyContext(class CRenderingContext* pContext, bool bTransparent);

	powerup_type_t	GetPowerupType() { return m_ePowerupType; }

protected:
	powerup_type_t	m_ePowerupType;

	float			m_flSpawnTime;
};

#endif
