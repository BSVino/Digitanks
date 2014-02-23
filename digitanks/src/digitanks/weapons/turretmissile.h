#ifndef DT_MISSILEDEFENSE_H
#define DT_MISSILEDEFENSE_H

#include "baseweapon.h"
#include <renderer/particles.h>

class CTurretMissile : public CBaseWeapon
{
	REGISTER_ENTITY_CLASS(CTurretMissile, CBaseWeapon);

public:
	virtual void				Spawn();

	void						SetTarget(class CBaseEntity* pTarget);

	virtual Vector				GetGlobalOrigin() const;

	virtual void				Think();

	virtual bool				ShouldRender() const { return true; };
	virtual void				OnRender(class CGameRenderingContext* pContext) const;

	virtual void				SetDamage(float flDamage) { m_flDamage = flDamage; }

	virtual weapon_t			GetWeaponType() { return WEAPON_TURRETMISSILE; }
	virtual float				ExplosionRadius() { return 3.0f; };
	virtual bool				MakesSounds() { return true; };

	virtual bool				CreatesCraters() { return false; };
	virtual float				PushRadius() { return 0.0f; };

	virtual float				InterceptTime() const { return 1.5f; };

protected:
	CEntityHandle<CBaseEntity>	m_hTarget;

	CParticleSystemInstanceHandle m_hTrailParticles;
};

#endif
