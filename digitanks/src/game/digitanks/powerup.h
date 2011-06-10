#ifndef POWERUP_H
#define POWERUP_H

#include "digitanksentity.h"

typedef enum {
	POWERUP_BONUS,
	POWERUP_AIRSTRIKE,
	POWERUP_TANK,
	POWERUP_MISSILEDEFENSE,
	POWERUP_WEAPON,
} powerup_type_t;

class CPowerup : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CPowerup, CDigitanksEntity);

public:
	virtual float	GetBoundingRadius() const { return 4; };

	void			Precache();
	virtual void	Spawn();

	virtual eastl::string16	GetEntityName() const;

	virtual EAngle	GetRenderAngles() const;
	virtual Vector	GetRenderOrigin() const;
	virtual void	ModifyContext(class CRenderingContext* pContext, bool bTransparent) const;

	powerup_type_t	GetPowerupType() { return m_ePowerupType; }
	void			SetPowerupType(powerup_type_t eType);

	void			Pickup(class CDigitank* pTank);
	DECLARE_ENTITY_OUTPUT(OnPickup);

protected:
	CNetworkedVariable<powerup_type_t> m_ePowerupType;
};

#endif
