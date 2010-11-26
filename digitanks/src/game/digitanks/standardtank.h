#ifndef DT_STANDARDTANK_H
#define DT_STANDARDTANK_H

#include <digitanks/digitank.h>

class CStandardTank : public CDigitank
{
	REGISTER_ENTITY_CLASS(CStandardTank, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual eastl::string16		GetName() { return L"Digitank"; };

	virtual float				RenderShieldScale() { return 1.3f; };

	virtual float				HealthRechargeRate() const { return 0.2f; };
	virtual float				ShieldRechargeRate() const { return 1.0f; };
	virtual float				GetTankSpeed() const { return 3.0f; };
	virtual float				TurnPerPower() const { return 45; };
	virtual float				InitialMaxRange() const { return 200.0f; };
	virtual float				InitialEffRange() const { return 80.0f; };
	virtual float				GetTransitionTime() const { return 2.0f; };
	virtual float				ProjectileCurve() const;

	virtual size_t				FleetPoints() const { return 0; };

	virtual buildunit_t			GetBuildUnit() const { return BUILDUNIT_TANK; }
};

#endif