#ifndef DT_SCOUT_H
#define DT_SCOUT_H

#include <digitanks/digitank.h>

class CScout : public CDigitank
{
	REGISTER_ENTITY_CLASS(CScout, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual bool				AllowControlMode(controlmode_t eMode) const;

	virtual eastl::string16		GetName() { return L"Rogue"; };

	CSupplyLine*				FindClosestEnemySupplyLine(bool bInRange = false);

	virtual void				Move();
	virtual void				Fire();
	virtual class CProjectile*	CreateProjectile();
	virtual void				FireProjectile(class CNetworkParameters* p);

	virtual bool				IsScout() const { return true; };

	virtual float				ShieldRechargeRate() const;
	virtual float				HealthRechargeRate() const;
	virtual float				GetTankSpeed() const { return 2.0f; }
	virtual float				TurnPerPower() const { return 9999; };
	virtual float				GetMinRange() const { return 5.0f; };
	virtual float				InitialEffRange() const { return 27.0f; };
	virtual float				InitialMaxRange() const { return 30.0f; };
	virtual float				GetTransitionTime() const { return 1.5f; }
	virtual float				ProjectileCurve() const { return -0.006f; };
	virtual float				VisibleRange() const { return 75; };
	virtual bool				CanGetPowerups() const { return true; };
	virtual float				BobHeight() const { return 1.0f; };
	static float				TorpedoAttackPower() { return 3.0f; };
	virtual float				MaxRangeRadius() const { return 1; };

	virtual size_t				FleetPoints() const { return ScoutFleetPoints(); };
	static size_t				ScoutFleetPoints() { return 1; };

	virtual buildunit_t			GetBuildUnit() const { return BUILDUNIT_SCOUT; }
};

#endif
