#ifndef DT_MECHINF_H
#define DT_MECHINF_H

#include <digitanks/units/digitank.h>

class CMechInfantry : public CDigitank
{
	REGISTER_ENTITY_CLASS(CMechInfantry, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual float				GetLeftShieldMaxStrength();
	virtual float				GetRightShieldMaxStrength();
	virtual float				GetRearShieldMaxStrength();

	virtual void				PostRender(bool bTransparent);

	virtual eastl::string16		GetName() { return L"Resistor"; };

	virtual float				RenderShieldScale() { return 2.0f; };

	virtual bool				CanFortify() { return true; };
	virtual bool				IsInfantry() const { return true; };

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
	virtual float				ProjectileCurve() const;
	virtual float				BaseVisibleRange() const { return 60.0f; };
	virtual float				FirstProjectileTime() const;
	virtual float				SlowMovementFactor() const { return 0.7f; };

	virtual size_t				FleetPoints() const { return InfantryFleetPoints(); };
	static size_t				InfantryFleetPoints() { return 2; };

	virtual unittype_t			GetUnitType() const { return UNIT_INFANTRY; }

protected:
	size_t						m_iFortifyShieldModel;
	size_t						m_iFortifyWallModel;
};

#endif
