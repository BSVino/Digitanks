#ifndef DT_CAMERAGUIDED_H
#define DT_CAMERAGUIDED_H

#include "baseweapon.h"

#include <renderer/particles.h>

class CCameraGuidedMissile : public CBaseWeapon
{
	REGISTER_ENTITY_CLASS(CCameraGuidedMissile, CBaseWeapon);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				Think();

	virtual void				OnSetOwner(class CDigitanksEntity* pOwner);

	virtual void				SpecialCommand();
	virtual bool				UsesSpecialCommand() { return true; };
	virtual tstring				SpecialCommandHint() { return _T("Space Bar\nTo Boost"); };

	virtual bool				ShouldTouch(CBaseEntity* pOther) const;
	virtual bool				IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;
	virtual void				Touching(CBaseEntity* pOther);

	virtual void				OnExplode(CBaseEntity* pInstigator);
	virtual void				OnDeleted();

	virtual bool				IsBoosting() { return m_flBoostTime > 0; }

	void						SetViewAngles(EAngle angView) { m_angView = angView; }
	EAngle						GetViewAngles() { return m_angView; }

	virtual weapon_t			GetWeaponType() { return PROJECTILE_CAMERAGUIDED; }
	virtual float				VelocityPerSecond() { return 50.0f; }
	virtual float				BoostVelocity() { return 50.0f; }
	virtual float				ExplosionRadius() { return 16.0f; };
	virtual float				BoostDamage() { return 5.0f; }

protected:
	bool						m_bLaunched;
	CNetworkedVariable<float>	m_flBoostTime;
	CNetworkedVariable<float>	m_flBoostVelocityGoal;
	CNetworkedVariable<float>	m_flBoostVelocity;

	CParticleSystemInstanceHandle m_hTrailParticles;

	EAngle						m_angView;
};

#endif
