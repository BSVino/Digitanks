#ifndef DT_MECHINF_H
#define DT_MECHINF_H

#include <digitanks/units/digitank.h>

class CMechInfantry : public CDigitank
{
	REGISTER_ENTITY_CLASS(CMechInfantry, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				PostRender(bool bTransparent) const;

	virtual eastl::string16		GetEntityName() const { return L"Resistor"; };

	virtual float				RenderShieldScale() const { return 10.0f; };

	virtual bool				CanFortify();
	virtual bool				IsInfantry() const { return true; };

	virtual float				BaseShieldRechargeRate() const;
	virtual float				BaseHealthRechargeRate() const;
	virtual float				GetTankSpeed() const { return 3.0f; }
	virtual float				InitialEffRange() const { return 30.0f; };
	virtual float				InitialMaxRange() const { return 60.0f; };
	virtual float				TurnPerPower() const { return 45; }
	virtual float				GetTransitionTime() const { return 2.5f; }
	virtual float				ProjectileCurve() const;
	virtual float				BaseVisibleRange() const { return 60.0f; };
	virtual float				FirstProjectileTime() const;
	virtual float				SlowMovementFactor() const { return 0.7f; };
	virtual float				BaseChargeRadius() const { return 50.0f; }
	virtual float				ChargeEnergy() const { return 9.0f; }
	virtual float				ChargeDamage() const { return 30.0f; }
	virtual float				ChargePushDistance() const { return 20.0f; }
	virtual bool				TurningMatters() const { return true; };

	virtual size_t				FleetPoints() const { return InfantryFleetPoints(); };
	static size_t				InfantryFleetPoints() { return 2; };

	virtual unittype_t			GetUnitType() const { return UNIT_INFANTRY; }

protected:
	size_t						m_iFortifyShieldModel;
	size_t						m_iFortifyWallModel;
};

#endif
