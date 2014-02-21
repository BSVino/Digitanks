#ifndef DT_MISSILEDEFENSE_H
#define DT_MISSILEDEFENSE_H

#include "baseweapon.h"

class CMissileDefense : public CBaseWeapon
{
	REGISTER_ENTITY_CLASS(CMissileDefense, CBaseWeapon);

public:
	void						SetTarget(class CProjectile* pTarget);

	virtual Vector				GetOrigin() const;

	virtual void				Think();

	virtual bool				ShouldRender() const { return true; };
	virtual void				OnRender(class CGameRenderingContext* pContext);

	virtual weapon_t			GetWeaponType() { return WEAPON_MISSILEDEFENSE; }
	virtual float				ExplosionRadius() { return 4.0f; };
	virtual bool				MakesSounds() { return true; };

	virtual bool				CreatesCraters() { return false; };
	virtual float				PushRadius() { return 0.0f; };

	virtual float				InterceptTime() const { return 0.7f; };

protected:
	CEntityHandle<CProjectile>	m_hTarget;
};

#endif
