#ifndef DT_CAMERAGUIDED_H
#define DT_CAMERAGUIDED_H

#include "baseweapon.h"

class CCameraGuidedMissile : public CBaseWeapon
{
	REGISTER_ENTITY_CLASS(CCameraGuidedMissile, CBaseWeapon);

public:
	virtual void				Spawn();

	virtual void				Think();

	virtual void				OnSetOwner(class CDigitank* pOwner);

	virtual void				SpecialCommand();
	virtual bool				UsesSpecialCommand() { return true; };
	virtual eastl::string16		SpecialCommandHint() { return L"Boost"; };

	virtual bool				ShouldTouch(CBaseEntity* pOther) const;
	virtual bool				IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;
	virtual void				Touching(CBaseEntity* pOther);

	virtual void				OnExplode(CBaseEntity* pInstigator);
	virtual void				OnDeleted();

	virtual weapon_t			GetWeaponType() { return PROJECTILE_CAMERAGUIDED; }
	virtual float				VelocityPerSecond() { return 50.0f; }
	virtual float				BoostVelocity() { return 50.0f; }
	virtual float				ExplosionRadius() { return 16.0f; };
	virtual float				BoostDamage() { return 5.0f; }

protected:
	float						m_flBoostTime;
	float						m_flBoostVelocityGoal;
	float						m_flBoostVelocity;
};

#endif
