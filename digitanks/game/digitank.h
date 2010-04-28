#ifndef DT_DIGITANK_H
#define DT_DIGITANK_H

#include "baseentity.h"
#include <common.h>

#define TANK_SHIELDS 4

class CDigitank : public CBaseEntity
{
	DECLARE_CLASS(CDigitank, CBaseEntity);

public:
								CDigitank();

public:
	float						GetTotalPower() { return m_flTotalPower; };
	float						GetAttackPower(bool bPreview = false);
	float						GetDefensePower(bool bPreview = false);
	float						GetMovementPower(bool bPreview = false);

	float						GetFrontShieldStrength();
	float						GetLeftShieldStrength();
	float						GetRightShieldStrength();
	float						GetRearShieldStrength();

	void						StartTurn();

	void						PreviewMove(Vector vecPreviewMove) { m_vecPreviewMove = vecPreviewMove; };

	void						SetDesiredMove();
	bool						HasDesiredMove() { return m_bDesiredMove; };
	Vector						GetDesiredMove() { return m_vecDesiredMove; };

	void						SetTarget(CDigitank* pTarget) { m_hTarget = pTarget; };

	void						Move();
	void						Fire();

	virtual void				TakeDamage(CBaseEntity* pAttacker, float flDamage);

	virtual float				ShieldRechargeRate() { return 1.0f; }
	virtual float				HealthRechargeRate() { return 0.2f; }

protected:
	float						m_flTotalPower;
	float						m_flAttackPower;
	float						m_flDefensePower;
	float						m_flMovementPower;

	float						m_flMaxShieldStrength;

	union {
		struct {
			float				m_flFrontShieldStrength;
			float				m_flLeftShieldStrength;
			float				m_flRightShieldStrength;
			float				m_flBackShieldStrength;
		};
		float					m_flShieldStrengths[TANK_SHIELDS];
	};

	Vector						m_vecPreviewMove;

	bool						m_bDesiredMove;
	Vector						m_vecDesiredMove;

	CEntityHandle<CDigitank>	m_hTarget;
};

#endif