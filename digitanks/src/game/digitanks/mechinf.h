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

	virtual const wchar_t*		GetName() { return L"Mechanized Infantry"; };

	virtual float				RenderShieldScale() { return 2.0f; };

	virtual bool				CanFortify() { return true; };
	virtual bool				UseFortifyMenuAim() { return true; };

	virtual float				GetBonusAttackPower(bool bPreview = false);
	virtual float				GetBonusDefensePower(bool bPreview = false);
	virtual float				GetFortifyAttackPowerBonus();
	virtual float				GetFortifyDefensePowerBonus();

	virtual float				ShieldRechargeRate() const;
	virtual float				HealthRechargeRate() const;
	virtual float				GetTankSpeed() const { return 1.5f; }
	virtual float				InitialEffRange() const { return 30.0f; };
	virtual float				InitialMaxRange() const { return 60.0f; };
	virtual float				TurnPerPower() const { return 45; }
	virtual float				GetTransitionTime() const { return 2.5f; }
	virtual float				ProjectileCurve() const { return -0.006f; };
	virtual float				VisibleRange() const { return 60.0f; };

	virtual size_t				FleetPoints() const { return InfantryFleetPoints(); };
	static size_t				InfantryFleetPoints() { return 2; };

	virtual buildunit_t			GetBuildUnit() { return BUILDUNIT_INFANTRY; }

protected:
	size_t						m_iFireProjectiles;
	float						m_flLastProjectileFire;
};

#endif