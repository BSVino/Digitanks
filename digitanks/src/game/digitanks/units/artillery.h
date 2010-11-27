#ifndef DT_ARTILLERY_H
#define DT_ARTILLERY_H

#include <digitanks/units/digitank.h>

class CArtillery : public CDigitank
{
	REGISTER_ENTITY_CLASS(CArtillery, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual float				GetProjectileDamage();

	virtual bool				AllowControlMode(controlmode_t eMode) const;

	virtual eastl::string16		GetName() { return L"Artillery"; };

	virtual void				OnFortify();
	virtual bool				CanFortify() { return true; };
	virtual bool				IsArtillery() const { return true; };
	virtual bool				CanTurnFortified() { return true; };
	virtual bool				CanAimMobilized() const { return false; };

	virtual float				ShieldRechargeRate() const;
	virtual float				HealthRechargeRate() const;
	virtual float				GetTankSpeed() const { return 1.3f; }
	virtual float				GetMinRange() const { return 50.0f; };
	virtual float				InitialEffRange() const { return 100.0f; };
	virtual float				InitialMaxRange() const { return 200.0f; };
	virtual float				TurnPerPower() const;
	virtual float				GetTransitionTime() const { return 2.5f; }
	virtual float				ProjectileCurve() const { return -0.006f; };
	virtual float				FiringCone() const { return 15; };
	virtual float				VisibleRange() const { return 45; };
	virtual size_t				ProjectileCount() const { return 3; };
	virtual float				FirstProjectileTime() const;
	virtual float				FireProjectileTime() const;

	virtual size_t				FleetPoints() const { return ArtilleryFleetPoints(); };
	static size_t				ArtilleryFleetPoints() { return 5; };

	virtual buildunit_t			GetBuildUnit() const { return BUILDUNIT_ARTILLERY; }
};

#endif
