#ifndef DT_SCOUT_H
#define DT_SCOUT_H

#include <digitanks/digitank.h>

class CScout : public CDigitank
{
	REGISTER_ENTITY_CLASS(CScout, CDigitank);

public:
								CScout();

public:
	virtual void				Precache();

	virtual bool				AllowControlMode(controlmode_t eMode);

	virtual const wchar_t*		GetName() { return L"Rogue"; };

	virtual bool				CanAimMobilized() { return false; };

	virtual float				ShieldRechargeRate() const;
	virtual float				HealthRechargeRate() const;
	virtual float				GetTankSpeed() const { return 2.0f; }
	virtual float				GetMinRange() const { return 0.0f; };
	virtual float				InitialEffRange() const { return 0.0f; };
	virtual float				InitialMaxRange() const { return 0.0f; };
	virtual float				GetTransitionTime() const { return 1.5f; }
	virtual float				ProjectileCurve() const { return -0.006f; };
	virtual float				VisibleRange() const { return 65; };

	virtual size_t				FleetPoints() const { return ScoutFleetPoints(); };
	static size_t				ScoutFleetPoints() { return 1; };

	virtual buildunit_t			GetBuildUnit() { return BUILDUNIT_SCOUT; }
};

#endif
