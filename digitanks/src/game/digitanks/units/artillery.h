#ifndef DT_ARTILLERY_H
#define DT_ARTILLERY_H

#include <digitanks/units/digitank.h>

class CArtillery : public CDigitank
{
	REGISTER_ENTITY_CLASS(CArtillery, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual bool				AllowControlMode(controlmode_t eMode) const;

	virtual eastl::string16		GetName() { return L"Artillery"; };

	virtual void				OnFortify();
	virtual bool				CanFortify() { return true; };
	virtual bool				IsArtillery() const { return true; };
	virtual bool				CanTurnFortified() { return true; };
	virtual bool				CanAimMobilized() const { return false; };

	virtual float				BaseShieldRechargeRate() const;
	virtual float				BaseHealthRechargeRate() const;
	virtual float				GetTankSpeed() const { return 1.5f; }
	virtual float				GetMinRange() const { return 50.0f; };
	virtual float				InitialEffRange() const { return 100.0f; };
	virtual float				InitialMaxRange() const { return 200.0f; };
	virtual float				TurnPerPower() const;
	virtual float				GetTransitionTime() const { return 2.5f; }
	virtual float				ProjectileCurve() const { return -0.006f; };
	virtual float				FiringCone() const { return 15; };
	virtual float				BaseVisibleRange() const { return 45; };
	virtual float				MinRangeRadius() const { return 15; };
	virtual float				MaxRangeRadius() const { return 25; };
	virtual float				FirstProjectileTime() const;
	virtual float				SlowMovementFactor() const { return 0.5f; };

	virtual size_t				FleetPoints() const { return ArtilleryFleetPoints(); };
	static size_t				ArtilleryFleetPoints() { return 5; };

	virtual unittype_t			GetUnitType() const { return UNIT_ARTILLERY; }
};

#endif
