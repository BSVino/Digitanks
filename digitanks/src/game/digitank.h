#ifndef DT_DIGITANK_H
#define DT_DIGITANK_H

#include "baseentity.h"
#include <common.h>

#define TANK_SHIELDS 4
#define TANK_MAX_RANGE_RADIUS 10

class CDigitank : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CDigitank, CBaseEntity);

public:
								CDigitank();
								~CDigitank();

public:
	void						Precache();

	virtual float				GetBoundingRadius() const { return 4; };

	float						GetBasePower() const { return m_flBasePower; };
	float						GetBaseAttackPower(bool bPreview = false);
	float						GetBaseDefensePower(bool bPreview = false);
	float						GetBaseMovementPower(bool bPreview = false);

	float						GetAttackPower(bool bPreview = false);
	float						GetDefensePower(bool bPreview = false);
	float						GetMovementPower(bool bPreview = false);
	float						GetTotalAttackPower();
	float						GetTotalDefensePower();
	float						GetTotalMovementPower() const;
	float						GetMaxMovementDistance() const;

	float						GetAttackScale(bool bPreview = false) { return GetAttackPower(bPreview) / 10; };
	float						GetDefenseScale(bool bPreview = false) { return GetDefensePower(bPreview) / 10; };
	float						GetMovementScale(bool bPreview = false) { return GetMovementPower(bPreview) / 10; };

	float						GetBonusAttackPower() { return m_flBonusAttackPower; };
	float						GetBonusDefensePower() { return m_flBonusDefensePower; };
	float						GetBonusMovementPower() { return m_flBonusMovementPower; };

	void						SetAttackPower(float flAttackPower);
	void						SetAttackPower(class CNetworkParameters* p);

	float						GetPreviewMoveTurnPower();
	float						GetPreviewMovePower() const;
	float						GetPreviewTurnPower() const;
	float						GetPreviewBaseMovePower() const;
	float						GetPreviewBaseTurnPower() const;

	bool						IsPreviewMoveValid() const;

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

	float						GetPreviewTurn() const { return m_flPreviewTurn; };
	void						SetPreviewTurn(float flPreviewTurn) { m_flPreviewTurn = flPreviewTurn; };
	void						ClearPreviewTurn();

	Vector						GetPreviewAim() { return m_vecPreviewAim; };
	void						SetPreviewAim(Vector vecPreviewAim);
	void						ClearPreviewAim();
	bool						IsPreviewAimValid();

	void						SetDesiredMove();
	void						SetDesiredMove(class CNetworkParameters* p);
	void						CancelDesiredMove();
	void						CancelDesiredMove(class CNetworkParameters* p);
	bool						HasDesiredMove() const { return m_bDesiredMove; };
	Vector						GetDesiredMove() const;
	bool						HasSelectedMove() { return m_bSelectedMove; };
	bool						IsMoving();

	void						SetDesiredTurn();
	void						SetDesiredTurn(class CNetworkParameters* p);
	void						CancelDesiredTurn();
	void						CancelDesiredTurn(class CNetworkParameters* p);
	bool						HasDesiredTurn() const { return m_bDesiredTurn; };
	float						GetDesiredTurn() const;

	void						SetDesiredAim();
	void						SetDesiredAim(class CNetworkParameters* p);
	void						CancelDesiredAim();
	void						CancelDesiredAim(class CNetworkParameters* p);
	bool						HasDesiredAim() { return m_bDesiredAim; };
	Vector						GetDesiredAim();

	bool						ChoseFirepower() { return m_bChoseFirepower; };

	virtual void				Think();

	void						OnCurrentTank();

	void						Move();
	void						Fire();
	void						FireProjectile();
	void						FireProjectile(class CNetworkParameters* p);

	virtual void				ClientUpdate(int iClient);

	virtual void				TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit = true);
	virtual void				OnKilled(CBaseEntity* pKilledBy);

	virtual Vector				GetRenderOrigin() const;
	virtual EAngle				GetRenderAngles() const;
	virtual void				ModifyContext(class CRenderingContext* pContext);
	virtual void				OnRender();
	virtual void				RenderTurret(float flAlpha = 1.0f);
	virtual void				RenderShield(float flAlpha, float flAngle);
	virtual void				PostRender();

	void						GiveBonusPoints(size_t i, bool bPlayEffects = true);
	bool						HasBonusPoints() { return m_iBonusPoints > 0; };
	size_t						GetBonusPoints() { return m_iBonusPoints; };
	void						PromoteAttack();
	void						PromoteDefense();
	void						PromoteMovement();
	void						SetBonusPoints(class CNetworkParameters* p);
	void						TankPromoted(class CNetworkParameters* p);
	void						PromoteAttack(class CNetworkParameters* p);
	void						PromoteDefense(class CNetworkParameters* p);
	void						PromoteMovement(class CNetworkParameters* p);

	float						FindHoverHeight(Vector vecPosition) const;

	class CTeam*				GetTeam() const { return m_pTeam; };
	void						SetTeam(class CTeam* pTeam) { m_pTeam = pTeam; };

	virtual float				ShieldRechargeRate() const { return 1.0f; }
	virtual float				HealthRechargeRate() const { return 0.2f; }
	virtual float				GetTankSpeed() const { return 2.0f; }
	virtual float				GetMinRange() const { return 50.0f; };
	virtual float				GetMaxRange() const { return 70.0f; };
	virtual float				TurnPerPower() const { return 45; }
	virtual float				GetTransitionTime() const { return 2.0f; }

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
	Vector						m_vecPreviousOrigin;
	Vector						m_vecDesiredMove;
	bool						m_bSelectedMove;
	float						m_flStartedMove;

	float						m_flPreviewTurn;
	bool						m_bDesiredTurn;
	float						m_flDesiredTurn;

	bool						m_bPreviewAim;
	Vector						m_vecPreviewAim;
	bool						m_bDesiredAim;
	Vector						m_vecDesiredAim;

	bool						m_bDisplayAim;
	Vector						m_vecDisplayAim;
	float						m_flDisplayAimRadius;

	bool						m_bChoseFirepower;

	float						m_flFireProjectileTime;
	CEntityHandle<class CProjectile>	m_hProjectile;

	class CTeam*				m_pTeam;

	size_t						m_iTurretModel;
	size_t						m_iShieldModel;

	size_t						m_iHoverParticles;

	static size_t				s_iAimBeam;
};

class CProjectile : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CProjectile, CBaseEntity);

public:
								CProjectile();

public:
	virtual void				Precache();

	virtual void				Think();

	virtual void				ModifyContext(class CRenderingContext* pContext);
	virtual void				OnRender();

	virtual void				OnDeleted();

	virtual bool				ShouldTouch(CBaseEntity* pOther) const;
	virtual bool				IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;
	virtual void				Touching(CBaseEntity* pOther);

	void						Explode();

	virtual void				SetOwner(CDigitank* pOwner);
	virtual void				SetDamage(float flDamage) { m_flDamage = flDamage; };
	virtual void				SetForce(Vector vecForce) { SetVelocity(vecForce); };

protected:
	float						m_flTimeCreated;
	float						m_flTimeExploded;

	bool						m_bFallSoundPlayed;

	CEntityHandle<CDigitank>	m_hOwner;
	float						m_flDamage;

	size_t						m_iParticleSystem;
};

#endif