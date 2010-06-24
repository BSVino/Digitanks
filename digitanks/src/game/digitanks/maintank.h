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

	virtual float				RenderShieldScale() { return 1.3f; };

	virtual float				HealthRechargeRate() const { return 0.2f; };
	virtual float				ShieldRechargeRate() const { return 1.0f; };
	virtual float				GetTankSpeed() const { return 2.0f; };
	virtual float				TurnPerPower() const { return 45; };
	virtual float				GetMaxRange() const { return 70.0f; };
	virtual float				GetMinRange() const { return 35.0f; };
	virtual float				GetTransitionTime() const { return 2.0f; };
};

#endif