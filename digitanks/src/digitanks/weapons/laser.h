#ifndef DT_LASER_H
#define DT_LASER_H

#include "baseweapon.h"

class CLaser : public CBaseWeapon
{
	REGISTER_ENTITY_CLASS(CLaser, CBaseWeapon);

public:
	virtual void				Precache();
	virtual void				ClientSpawn();

	virtual void				OnSetOwner(class CDigitanksEntity* pOwner);

	virtual float				GetRenderRadius() const { return LaserLength(); };

	virtual bool				ShouldRender() const { return true; };
	virtual void				PostRender(bool bTransparent) const;

	virtual float				LaserLength() const { return 400; }

	virtual weapon_t			GetWeaponType() { return WEAPON_LASER; }
	virtual float				ExplosionRadius() { return 0.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual float				PushRadius() { return 0.0f; };
	virtual float				RockIntensity() { return 0.5f; };
	virtual float				PushDistance() { return 0.0f; };

protected:
	static size_t				s_iBeam;
};

class CInfantryLaser : public CLaser
{
	REGISTER_ENTITY_CLASS(CInfantryLaser, CLaser);

public:
	virtual float				LaserLength() const { return 60.0f; };

protected:
};

#endif
