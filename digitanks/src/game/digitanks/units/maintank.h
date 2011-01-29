#ifndef DT_MAINTANK_H
#define DT_MAINTANK_H

#include <digitanks/units/digitank.h>

class CMainBattleTank : public CDigitank
{
	REGISTER_ENTITY_CLASS(CMainBattleTank, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual eastl::string16		GetName() { return L"Digitank"; };

	virtual float				RenderShieldScale() const { return 1.3f; };

	virtual float				HealthRechargeRate() const { return 0.2f; };
	virtual float				ShieldRechargeRate() const { return 1.0f; };
	virtual float				GetTankSpeed() const { return 3.0f; };
	virtual float				TurnPerPower() const { return 45; };
	virtual float				BaseVisibleRange() const { return 75; };
	virtual float				InitialMaxRange() const { return 60.0f; };
	virtual float				InitialEffRange() const { return 30.0f; };
	virtual float				GetTransitionTime() const { return 2.0f; };
	virtual float				SlowMovementFactor() const { return 0.5f; };

	virtual size_t				FleetPoints() const { return MainTankFleetPoints(); };
	static size_t				MainTankFleetPoints() { return 4; };

	virtual unittype_t			GetUnitType() const { return UNIT_TANK; }
};

#endif