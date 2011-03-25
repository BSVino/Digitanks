#ifndef DT_AUTOTURRET_H
#define DT_AUTOTURRET_H

#include <digitanks/units/digitank.h>

class CAutoTurret : public CDigitank
{
	REGISTER_ENTITY_CLASS(CAutoTurret, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent);

	virtual eastl::string16		GetName() { return L"Auto-Turret"; };

	virtual bool				CanFortify() { return true; };

	virtual float				TotalHealth() const { return 40; };
	virtual bool				TakesLavaDamage() { return false; }
	virtual float				BaseHealthRechargeRate() const { return 0.0f; };
	virtual float				GetTankSpeed() const { return 0.0f; };
	virtual float				InitialEffRange() const { return 10.0f; };
	virtual float				InitialMaxRange() const { return 60.0f; };
	virtual float				MaxRangeRadius() const { return 40; };

	virtual size_t				FleetPoints() const { return 0; };

	virtual unittype_t			GetUnitType() const { return UNIT_AUTOTURRET; }
};

class CGridBug : public CDigitank
{
	REGISTER_ENTITY_CLASS(CGridBug, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent);

	virtual eastl::string16		GetName() { return L"Grid Bug"; };

	virtual float				TotalHealth() const { return 40; };
	virtual bool				TakesLavaDamage() { return false; }
	virtual float				BaseHealthRechargeRate() const { return 0.0f; };
	virtual float				GetTankSpeed() const { return 1.0f; };
	virtual float				InitialEffRange() const { return 30.0f; };
	virtual float				InitialMaxRange() const { return 60.0f; };
	virtual float				MaxRangeRadius() const { return 20; };
	virtual float				SlowMovementFactor() const { return 0.9f; };

	virtual size_t				FleetPoints() const { return 0; };

	virtual unittype_t			GetUnitType() const { return UNIT_GRIDBUG; }
};

#endif

