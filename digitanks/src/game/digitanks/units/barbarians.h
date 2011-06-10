#ifndef DT_BUGTURRET_H
#define DT_BUGTURRET_H

#include <digitanks/units/digitank.h>

class CBugTurret : public CDigitank
{
	REGISTER_ENTITY_CLASS(CBugTurret, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent) const;

	virtual float				GetFortifyAttackPowerBonus();
	virtual float				GetFortifyDefensePowerBonus();

	virtual eastl::string16		GetEntityName() const { return L"Bug Turret"; };

	virtual bool				CanFortify() { return true; };

	virtual float				BaseVisibleRange() const { return 60.0f; };
	virtual float				TotalHealth() const { return 40; };
	virtual bool				TakesLavaDamage() { return false; }
	virtual float				BaseHealthRechargeRate() const { return 0.0f; };
	virtual float				GetTankSpeed() const { return 0.0f; };
	virtual float				InitialEffRange() const { return 10.0f; };
	virtual float				InitialMaxRange() const { return 60.0f; };
	virtual float				MaxRangeRadius() const { return 40; };

	virtual size_t				FleetPoints() const { return 0; };

	virtual unittype_t			GetUnitType() const { return UNIT_BUGTURRET; }
};

class CGridBug : public CDigitank
{
	REGISTER_ENTITY_CLASS(CGridBug, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent) const;

	virtual eastl::string16		GetEntityName() const { return L"Grid Bug"; };

	virtual float				BaseVisibleRange() const { return 60.0f; };
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

