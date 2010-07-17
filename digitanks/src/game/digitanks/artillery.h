#ifndef DT_ARTILLERY_H
#define DT_ARTILLERY_H

#include <digitanks/digitank.h>

class CArtillery : public CDigitank
{
	REGISTER_ENTITY_CLASS(CArtillery, CDigitank);

public:
								CArtillery();

public:
	virtual void				Precache();

	virtual void				SetAttackPower(float flAttackPower);

	virtual void				Think();

	virtual void				Fire();
	virtual class CProjectile*	CreateProjectile();
	virtual float				GetProjectileDamage();

	virtual bool				OnControlModeChange(controlmode_t eOldMode, controlmode_t eNewMode);

	virtual bool				CanFortify() { return true; };
	virtual bool				IsArtillery() { return true; };
	virtual bool				UseFortifyMenuFire() { return true; };
	virtual bool				CanTurnFortified() { return true; };
	virtual bool				CanAimMobilized() { return false; };

	virtual float				ShieldRechargeRate() const;
	virtual float				HealthRechargeRate() const;
	virtual float				GetTankSpeed() const { return 1.3f; }
	virtual float				GetMinRange() const { return 50.0f; };
	virtual float				GetEffRange() const { return 100.0f; };
	virtual float				GetMaxRange() const { return 200.0f; };
	virtual float				TurnPerPower() const;
	virtual float				GetTransitionTime() const { return 2.5f; }
	virtual float				ProjectileCurve() const { return -0.006f; };
	virtual float				FiringCone() const { return 15; };
	virtual float				VisibleRange() const { return 45; };

protected:
	bool						m_bFortified;
	size_t						m_iFortifyLevel;

	size_t						m_iFireProjectiles;
	float						m_flLastProjectileFire;
};

#endif