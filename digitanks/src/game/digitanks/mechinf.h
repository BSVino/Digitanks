#ifndef DT_MECHINF_H
#define DT_MECHINF_H

#include <digitanks/digitank.h>

class CMechInfantry : public CDigitank
{
	REGISTER_ENTITY_CLASS(CMechInfantry, CDigitank);

public:
								CMechInfantry();

public:
	virtual void				Precache();

	virtual float				GetLeftShieldMaxStrength();
	virtual float				GetRightShieldMaxStrength();
	virtual float				GetRearShieldMaxStrength();

	virtual float*				GetShieldForAttackDirection(Vector vecAttack);

	virtual void				Think();
	virtual void				Fire();
	virtual class CProjectile*	CreateProjectile();
	virtual float				GetProjectileDamage();

	virtual float				RenderShieldScale() { return 2.0f; };

	virtual void				StartTurn();

	virtual void				SetPreviewMove(Vector vecPreviewMove);
	virtual void				SetPreviewTurn(float flPreviewTurn);

	virtual void				SetDesiredMove();
	virtual void				SetDesiredTurn();

	virtual void				Fortify();
	virtual bool				CanFortify() { return true; };
	virtual bool				UseFortifyMenu() { return true; };
	virtual bool				IsFortified() const { return m_bFortified && m_iFortifyLevel; };
	virtual bool				IsFortifying() const { return m_bFortified && m_iFortifyLevel == 0; };

	virtual float				GetBonusAttackPower();
	virtual float				GetBonusDefensePower();
	virtual float				GetFortifyAttackPowerBonus();
	virtual float				GetFortifyDefensePowerBonus();

	virtual float				ShieldRechargeRate() const;
	virtual float				HealthRechargeRate() const;
	virtual float				GetTankSpeed() const { return 1.5f; }
	virtual float				GetMinRange() const { return 30.0f; };
	virtual float				GetMaxRange() const { return 40.0f; };
	virtual float				TurnPerPower() const { return 45; }
	virtual float				GetTransitionTime() const { return 2.5f; }

protected:
	bool						m_bFortified;
	size_t						m_iFortifyLevel;

	size_t						m_iFireProjectiles;
	float						m_flLastProjectileFire;
};

#endif