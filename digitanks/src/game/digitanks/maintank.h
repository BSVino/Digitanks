#ifndef DT_MAINTANK_H
#define DT_MAINTANK_H

#include <digitanks/digitank.h>

class CMainBattleTank : public CDigitank
{
	REGISTER_ENTITY_CLASS(CMainBattleTank, CDigitank);

public:
								CMainBattleTank();

public:
	virtual void				Precache();

	virtual const wchar_t*		GetName() { return L"Main Battle Tank"; };

	virtual float				RenderShieldScale() { return 1.3f; };

	virtual float				HealthRechargeRate() const { return 0.2f; };
	virtual float				ShieldRechargeRate() const { return 1.0f; };
	virtual float				GetTankSpeed() const { return 2.0f; };
	virtual float				TurnPerPower() const { return 45; };
	virtual float				VisibleRange() const { return 75; };
	virtual float				InitialMaxRange() const { return 60.0f; };
	virtual float				InitialEffRange() const { return 30.0f; };
	virtual float				GetTransitionTime() const { return 2.0f; };

	virtual size_t				FleetPoints() const { return MainTankFleetPoints(); };
	static size_t				MainTankFleetPoints() { return 4; };

	virtual buildunit_t			GetBuildUnit() const { return BUILDUNIT_TANK; }
};

#endif