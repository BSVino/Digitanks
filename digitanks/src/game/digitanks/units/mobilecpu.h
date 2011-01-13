#ifndef DT_MOBILECPU_H
#define DT_MOBILECPU_H

#include <digitanks/units/digitank.h>

class CMobileCPU : public CDigitank
{
	REGISTER_ENTITY_CLASS(CMobileCPU, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual eastl::string16		GetName() { return L"Mobile Construction Process"; };

	virtual bool				CanFortify() { return true; };
	virtual void				OnFortify();

	virtual bool				IsMobileCPU() const { return true; };
	virtual float				HealthRechargeRate() const { return 0.2f; };
	virtual float				GetTankSpeed() const { return 3.0f; };
	virtual float				TurnPerPower() const { return 45; };
	virtual float				GetTransitionTime() const { return 2.0f; };
	virtual float				MaxRangeRadius() const { return 30; };
	virtual float				SlowMovementFactor() const { return 0.8f; };

	virtual size_t				FleetPoints() const { return 0; };

	virtual unittype_t			GetUnitType() const { return UNIT_MOBILECPU; }
};

#endif