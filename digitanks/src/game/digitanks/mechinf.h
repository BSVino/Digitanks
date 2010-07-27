#ifndef DT_MECHINF_H
#define DT_MECHINF_H

#include <digitanks/digitank.h>

class CMechInfantry : public CDigitank
{
	REGISTER_ENTITY_CLASS(CMechInfantry, CDigitank);

public:
								CMechInfantry();

public:
	virtual void				Precache();

	virtual float				GetLeftShieldMaxStrength();
	virtual float				GetRightShieldMaxStrength();
	virtual float				GetRearShieldMaxStrength();

	virtual void				Think();
	virtual void				Fire();
	virtual class CProjectile*	CreateProjectile();
	virtual float				GetProjectileDamage();

	virtual bool				AllowControlMode(controlmode_t eMode);

	virtual const char*			GetName() { return "Mechanized Infantry"; };

	virtual float				RenderShieldScale() { return 2.0f; };

	virtual bool				CanFortify() { return true; };
	virtual bool				UseFortifyMenuAim() { return true; };

	virtual float				GetBonusAttackPower();
	virtual float				GetBonusDefensePower();
	virtual float				GetFortifyAttackPowerBonus();
	virtual float				GetFortifyDefensePowerBonus();

	virtual float				ShieldRechargeRate() const;
	virtual float				HealthRechargeRate() const;
	virtual float				GetTankSpeed() const { return 1.5f; }
	virtual float				GetEffRange() const { return 30.0f; };
	virtual float				GetMaxRange() const { return 55.0f; };
	virtual float				TurnPerPower() const { return 45; }
	virtual float				GetTransitionTime() const { return 2.5f; }
	virtual float				ProjectileCurve() const { return -0.006f; };
	virtual float				VisibleRange() const { return 50; };

protected:
	size_t						m_iFireProjectiles;
	float						m_flLastProjectileFire;
};

#endif