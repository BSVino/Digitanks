#ifndef DT_STANDARDTANK_H
#define DT_STANDARDTANK_H

#include <digitanks/units/digitank.h>

class CStandardTank : public CDigitank
{
	REGISTER_ENTITY_CLASS(CStandardTank, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual eastl::string16		GetName() { return L"Digitank"; };

	virtual float				HealthRechargeRate() const { return 0.2f; };
	virtual float				ShieldRechargeRate() const { return 1.0f; };
	virtual float				GetTankSpeed() const { return 3.0f; };
	virtual float				TurnPerPower() const { return 45; };
	virtual float				InitialMaxRange() const { return 200.0f; };
	virtual float				InitialEffRange() const { return 50.0f; };
	virtual float				GetTransitionTime() const { return 2.0f; };
	virtual float				ProjectileCurve() const;
	virtual float				MaxRangeRadius() const { return 30; };
	virtual float				BaseChargeRadius() const { return 40.0f; }
	virtual float				ChargeEnergy() const { return 8.0f; }
	virtual float				ChargeDamage() const { return 60.0f; }
	virtual float				ChargePushDistance() const { return 50.0f; }
	virtual float				SlowMovementFactor() const { return 0.65f; };

	virtual size_t				FleetPoints() const { return 0; };

	virtual unittype_t			GetUnitType() const { return UNIT_TANK; }
};

#endif