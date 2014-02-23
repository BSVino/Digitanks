#ifndef DT_STANDARDTANK_H
#define DT_STANDARDTANK_H

#include <units/digitank.h>

class CStandardTank : public CDigitank
{
	REGISTER_ENTITY_CLASS(CStandardTank, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual tstring				GetEntityName() const { return "Digitank"; };

	virtual const TFloat		GetBoundingRadius() const { return 6; };

	virtual void				ModifyContext(class CRenderingContext* pContext) const;

	virtual float				BaseHealthRechargeRate() const { return 10.0f; };
	virtual float				BaseShieldRechargeRate() const { return 50.0f; };
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
