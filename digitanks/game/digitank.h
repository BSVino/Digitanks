#ifndef DT_DIGITANK_H
#define DT_DIGITANK_H

#include "baseentity.h"
#include <common.h>

#define TANK_SHIELDS 4
#define TANK_MAX_RANGE_RADIUS 10

class CDigitank : public CBaseEntity
{
	DECLARE_CLASS(CDigitank, CBaseEntity);

public:
								CDigitank();

public:
	float						GetBasePower() { return m_flBasePower; };
	float						GetBaseAttackPower(bool bPreview = false);
	float						GetBaseDefensePower(bool bPreview = false);
	float						GetBaseMovementPower(bool bPreview = false);

	float						GetAttackPower(bool bPreview = false);
	float						GetDefensePower(bool bPreview = false);
	float						GetMovementPower(bool bPreview = false);
	float						GetTotalAttackPower();
	float						GetTotalDefensePower();
	float						GetTotalMovementPower();

	float						GetAttackScale(bool bPreview = false) { return GetAttackPower(bPreview) / 10; };
	float						GetDefenseScale(bool bPreview = false) { return GetDefensePower(bPreview) / 10; };
	float						GetMovementScale(bool bPreview = false) { return GetMovementPower(bPreview) / 10; };

	float						GetBonusAttackPower() { return m_flBonusAttackPower; };
	float						GetBonusDefensePower() { return m_flBonusDefensePower; };
	float						GetBonusMovementPower() { return m_flBonusMovementPower; };

	void						SetAttackPower(float flAttackPower);

	float						GetPreviewMoveTurnPower();
	float						GetPreviewMovePower();
	float						GetPreviewTurnPower();
	float						GetPreviewBaseMovePower();
	float						GetPreviewBaseTurnPower();

	void						CalculateAttackDefense();

	float						GetShieldMaxStrength() { return m_flMaxShieldStrength; };
	float						GetFrontShieldStrength();
	float						GetLeftShieldStrength();
	float						GetRightShieldStrength();
	float						GetRearShieldStrength();

	float*						GetShieldForAttackDirection(Vector vecAttack);

	void						StartTurn();

	Vector						GetPreviewMove() { return m_vecPreviewMove; };
	void						SetPreviewMove(Vector vecPreviewMove) { m_vecPreviewMove = vecPreviewMove; };
	void						ClearPreviewMove();

	float						GetPreviewTurn() { return m_flPreviewTurn; };
	void						SetPreviewTurn(float flPreviewTurn) { m_flPreviewTurn = flPreviewTurn; };
	void						ClearPreviewTurn();

	Vector						GetPreviewAim() { return m_vecPreviewAim; };
	void						SetPreviewAim(Vector vecPreviewAim);
	void						ClearPreviewAim();
	bool						IsPreviewAimValid();

	void						SetDesiredMove();
	void						CancelDesiredMove();
	bool						HasDesiredMove() { return m_bDesiredMove; };
	Vector						GetDesiredMove();

	void						SetDesiredTurn();
	void						CancelDesiredTurn();
	bool						HasDesiredTurn() { return m_bDesiredTurn; };
	float						GetDesiredTurn();

	void						SetDesiredAim();
	void						CancelDesiredAim();
	bool						HasDesiredAim() { return m_bDesiredAim; };
	Vector						GetDesiredAim();

	void						Move();
	void						Fire();

	virtual void				TakeDamage(CBaseEntity* pAttacker, float flDamage);

	virtual void				Render();

	void						GiveBonusPoints(size_t i);
	bool						HasBonusPoints() { return m_iBonusPoints > 0; };
	size_t						GetBonusPoints() { return m_iBonusPoints; };
	void						PromoteAttack();
	void						PromoteDefense();
	void						PromoteMovement();

	class CTeam*				GetTeam() { return m_pTeam; };
	void						SetTeam(class CTeam* pTeam) { m_pTeam = pTeam; };

	virtual float				ShieldRechargeRate() { return 1.0f; }
	virtual float				HealthRechargeRate() { return 0.2f; }
	virtual float				GetMinRange() { return 30.0f; };
	virtual float				GetMaxRange() { return 50.0f; };
	virtual float				TurnPerPower() { return 45; }

protected:
	float						m_flBasePower;
	float						m_flAttackPower;
	float						m_flDefensePower;
	float						m_flMovementPower;

	float						m_flBonusAttackPower;
	float						m_flBonusDefensePower;
	float						m_flBonusMovementPower;
	size_t						m_iBonusPoints;

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

	float						m_flPreviewTurn;
	bool						m_bDesiredTurn;
	float						m_flDesiredTurn;

	bool						m_bPreviewAim;
	Vector						m_vecPreviewAim;
	bool						m_bDesiredAim;
	Vector						m_vecDesiredAim;

	class CTeam*				m_pTeam;
};

class CProjectile : public CBaseEntity
{
public:
								CProjectile(CDigitank* pOwner, float flDamage, Vector vecForce);

public:
	virtual void				Render();

	virtual void				TouchedGround();

protected:
	CEntityHandle<CDigitank>	m_hOwner;
	float						m_flDamage;
};

#endif