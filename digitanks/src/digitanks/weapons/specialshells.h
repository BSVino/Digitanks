#ifndef DT_SPECIALSHELLS_H
#define DT_SPECIALSHELLS_H

#include "projectile.h"

class CAirstrikeShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CAirstrikeShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_AIRSTRIKE; }
	virtual float				ShellRadius() { return 0.7f; };
	virtual float				ExplosionRadius() const { return 12.0f; };
	virtual float				PushRadius() { return 20.0f; };
	virtual float				PushDistance() { return 4.0f; };
	virtual float				RockIntensity() { return 0.8f; };
};

class CFireworks : public CProjectile
{
	REGISTER_ENTITY_CLASS(CFireworks, CProjectile);

public:
	virtual bool				ShouldTouch(CBaseEntity* pOther) const;

	virtual weapon_t			GetWeaponType() { return PROJECTILE_FIREWORKS; }
	virtual bool				BombDropNoise() { return false; };
	virtual float				PushDistance() { return 5.0f; };
	virtual float				RockIntensity() { return 1.0f; };
};

#endif
