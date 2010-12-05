#ifndef DT_LASER_H
#define DT_LASER_H

#include "baseweapon.h"

class CLaser : public CBaseWeapon
{
	REGISTER_ENTITY_CLASS(CLaser, CBaseWeapon);

public:
	virtual void				OnSetOwner(class CDigitank* pOwner);

	virtual bool				ShouldRender() const { return true; };
	virtual void				OnRender();

	virtual weapon_t			GetWeaponType() { return WEAPON_LASER; }
	virtual float				ExplosionRadius() { return 0.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual float				PushRadius() { return 0.0f; };
	virtual float				RockIntensity() { return 0.5f; };
	virtual float				PushDistance() { return 0.0f; };

protected:
};

#endif
