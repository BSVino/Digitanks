#ifndef DT_SCOUT_H
#define DT_SCOUT_H

#include <digitanks/units/digitank.h>

class CScout : public CDigitank
{
	REGISTER_ENTITY_CLASS(CScout, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual float				GetBoundingRadius() const { return 2.5; };

	virtual bool				AllowControlMode(controlmode_t eMode) const;

	virtual eastl::string16		GetName() { return L"Rogue"; };

	CSupplyLine*				FindClosestEnemySupplyLine(bool bInRange = false);

	virtual void				Move();
	virtual void				Fire();
	virtual void				FireWeapon(class CNetworkParameters* p);

	virtual bool				IsScout() const { return true; };

	virtual float				FindHoverHeight(Vector vecPosition) const;

	virtual float				TotalHealth() const { return 80; };
	virtual float				BaseShieldRechargeRate() const;
	virtual float				BaseHealthRechargeRate() const;
	virtual float				GetTankSpeed() const { return 3.5f; }
	virtual float				TurnPerPower() const { return 9999; };
	virtual float				GetMinRange() const { return 5.0f; };
	virtual float				InitialEffRange() const { return 32.0f; };
	virtual float				InitialMaxRange() const { return 35.0f; };
	virtual float				GetTransitionTime() const { return 1.5f; }
	virtual float				ProjectileCurve() const { return -0.006f; };
	virtual float				BaseVisibleRange() const { return 75; };
	virtual bool				TakesLavaDamage() { return false; }
	virtual float				TreesReduceVisibility() const { return false; };
	virtual bool				CanGetPowerups() const { return true; };
	virtual float				BobHeight() const { return 1.0f; };
	virtual float				MaxRangeRadius() const { return 1; };
	virtual float				SlowMovementFactor() const { return 0.85f; };
	bool						IsRammable() const { return false; }

	virtual size_t				FleetPoints() const { return ScoutFleetPoints(); };
	static size_t				ScoutFleetPoints() { return 1; };

	virtual unittype_t			GetUnitType() const { return UNIT_SCOUT; }
};

#endif
