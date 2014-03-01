#ifndef DT_DIGITANK_H
#define DT_DIGITANK_H

#include <tvector.h>

#include <selectable.h>
#include <structures/loader.h>
#include <weapons/projectile.h>
#include <common.h>
#include <renderer/particles.h>

// Tank speech
enum
{
	TANKSPEECH_SELECTED,
	TANKSPEECH_MOVED,
	TANKSPEECH_ATTACK,
	TANKSPEECH_DAMAGED,
	TANKSPEECH_KILL,
	TANKSPEECH_MISSED,
	TANKSPEECH_IDLE,
	TANKSPEECH_PROMOTED,
	TANKSPEECH_PARTY,
	TANKSPEECH_TAUNT,
	TANKSPEECH_DISABLED,
};

enum
{
	TANKLINE_CUTE,
	TANKLINE_LOVE,
	TANKLINE_HAPPY,
	TANKLINE_CHEER,
	TANKLINE_EVIL,
	TANKLINE_SQUINT,
	TANKLINE_COOL,
	TANKLINE_DEAD,
	TANKLINE_DEAD2,
	TANKLINE_DEAD3,
	TANKLINE_FROWN,
	TANKLINE_SAD,
	TANKLINE_ASLEEP,
	TANKLINE_CONFUSED,
	TANKLINE_DOTDOTDOT,
	TANKLINE_SURPRISED,
	TANKLINE_THRILLED,
	TANKLINE_TONGUE,
	TANKLINE_CATFACE,
};

class CDigitank : public CSelectable
{
	REGISTER_ENTITY_CLASS(CDigitank, CSelectable);

public:
	virtual void				Precache();
	virtual void				SetupSpeechLines();
	virtual void				Spawn();

	virtual const TFloat		GetBoundingRadius() const { return 4; };
	virtual const TFloat		GetRenderRadius() const { return GetBoundingRadius() + RenderShieldScale(); };

	float						GetTotalPower() const { return m_flTotalPower; };
	float						GetStartingPower() const { return m_flStartingPower; };
	float						GetBaseAttackPower(bool bPreview = false);
	float						GetBaseDefensePower(bool bPreview = false) const;

	float						GetAttackPower(bool bPreview = false);
	float						GetDefensePower(bool bPreview = false) const;
	float						GetTotalAttackPower();
	float						GetTotalDefensePower();

	float						GetBonusMovementEnergy() const { return m_flBonusMovementPower; };
	float						GetMaxMovementEnergy() const;
	float						GetMaxMovementDistance() const;
	float						GetUsedMovementEnergy(bool bPreview = false) const;
	float						GetRemainingMovementEnergy(bool bPreview = false) const;
	float						GetRemainingMovementDistance() const;
	float						GetRemainingTurningDistance() const;

	float						GetDefenseScale(bool bPreview = false) const { return GetDefensePower(bPreview) / 10; };

	virtual float				GetBonusAttackDamage();
	virtual float				GetBonusDefensePower(bool bPreview = false) const;

	virtual void				AddRangeBonus(float flAmount) { m_flRangeBonus += flAmount; };

	virtual float				GetSupportAttackPowerBonus();
	virtual float				GetSupportDefensePowerBonus() const;

	virtual float				GetSupportHealthRechargeBonus() const;
	virtual float				GetSupportShieldRechargeBonus() const;

	float						GetPreviewMoveTurnPower() const;
	float						GetPreviewMovePower() const;
	float						GetPreviewTurnPower() const;
	float						GetPreviewBaseMovePower() const;
	float						GetPreviewBaseTurnPower() const;

	bool						IsPreviewMoveValid() const;

	virtual float				GetShieldMaxStrength() const { return m_flMaxShieldStrength; };
	virtual float				GetShieldStrength() const;
	virtual float				GetShieldBlockRadius();

	virtual float				GetShieldValue() const;
	virtual void				SetShieldValue(float flValue);

	virtual void				SetGoalTurretYaw(float flYaw) { m_flGoalTurretYaw = flYaw; }

	virtual void				StartTurn();
	virtual void				EndTurn();

	virtual CDigitank*			FindClosestVisibleEnemyTank(bool bInRange = false);

	Vector						GetPreviewMove() const { return m_vecPreviewMove; };
	virtual void				SetPreviewMove(Vector vecPreviewMove);
	void						ClearPreviewMove();

	float						GetPreviewTurn() const { return m_flPreviewTurn; };
	virtual void				SetPreviewTurn(float flPreviewTurn);
	void						ClearPreviewTurn();

	Vector						GetPreviewAim() const { return m_vecPreviewAim; };
	void						SetPreviewAim(Vector vecPreviewAim);
	void						ClearPreviewAim();
	bool						IsPreviewAimValid();
	Vector						GetLastAim() const { return m_vecLastAim; };
	bool						ShouldDisplayAim() const { return m_bDisplayAim; }
	Vector						GetDisplayAim() const { return m_vecDisplayAim; }

	virtual bool				CanCharge() const;
	virtual float				BaseChargeRadius() const { return 20.0f; }
	float						ChargeRadius() const;
	virtual float				ChargeEnergy() const { return 7.0f; }
	virtual float				ChargeDamage() const { return 70.0f; }
	virtual float				ChargePushDistance() const { return 50.0f; }
	CBaseEntity*				GetPreviewCharge() const { return m_hPreviewCharge; };
	virtual void				SetPreviewCharge(CBaseEntity* pPreviewCharge);
	void						ClearPreviewCharge();
	Vector						GetChargePosition(CBaseEntity* pTarget) const;

	bool						IsInsideMaxRange(Vector vecPoint);
	float						FindAimRadius(Vector vecPoint, float flMin = 2.0f);

	void						RockTheBoat(float flIntensity, Vector vecDirection);
	bool						IsRocking() const;

	void						Move();
	void						Move(class CNetworkParameters* p);
	bool						IsMoving() const;
	void						Move(Vector vecNewPosition, int iMoveType = 0);

	void						Turn();
	void						Turn(class CNetworkParameters* p);
	bool						IsTurning();
	void						Turn(EAngle angNewTurn);

	void						SetGoalMovePosition(const Vector& vecPosition);
	void						SetGoalMovePosition(CNetworkParameters* p);
	void						MoveTowardsGoalMovePosition();
	void						CancelGoalMovePosition();
	void						CancelGoalMovePosition(CNetworkParameters* p);
	bool						HasGoalMovePosition() const { return m_bGoalMovePosition; };
	Vector						GetGoalMovePosition() const { return m_vecGoalMovePosition; };

	virtual bool				IsArtillery() const { return false; };
	virtual bool				IsInfantry() const { return false; };
	virtual bool				IsScout() const { return false; };
	virtual bool				IsMobileCPU() const { return false; };

	void						Fortify();
	void						Fortify(CNetworkParameters* p);
	virtual void				OnFortify() {};
	virtual bool				CanFortify() { return false; };
	virtual bool				IsFortified() const { return m_bFortified && m_iFortifyLevel; };
	virtual bool				IsFortifying() const { return m_bFortified && m_iFortifyLevel == (size_t)0; };
	virtual bool				CanMoveFortified() { return false; };
	virtual bool				CanTurnFortified() { return false; };
	virtual bool				CanAimMobilized() const { return true; };
	virtual bool				CanAim() const;
	virtual float				GetFortifyAttackPowerBonus();
	virtual float				GetFortifyDefensePowerBonus() const;
	virtual bool				CanGetPowerups() const { return true; };
	virtual size_t				GetFortifyLevel() { return m_iFortifyLevel; }

	void						StayPut() { m_bStayPut = true; }
	bool						ShouldStayPut() { return m_bStayPut; }

	void						Sentry();
	void						Sentry(CNetworkParameters* p);
	virtual bool				CanSentry() { return true; };
	virtual bool				IsSentried() const { return m_bSentried; };

	void						Charge();
	void						Charge(CBaseEntity* pTarget);

	void						GiveCloak() { m_bHasCloak = true; }
	bool						HasCloak() { return m_bHasCloak; }
	bool						IsCloaked() const { return m_bCloaked; }
	void						Cloak();
	void						Uncloak();
	virtual float				GetCloakConcealment() const;
	virtual bool				HasLostConcealment() const { return m_bLostConcealment; }

	virtual bool				MovesWith(CDigitank* pOther) const;
	virtual bool				TurnsWith(CDigitank* pOther) const;
	virtual bool				AimsWith(CDigitank* pOther) const;

	virtual void				Think();

	// CSelectable
	virtual void				OnCurrentSelection();
	virtual bool				AllowControlMode(controlmode_t eMode) const;
	virtual void				OnControlModeChange(controlmode_t eOldMode, controlmode_t eNewMode);

	virtual const char*			GetPowerBar1Text() { return "Attack"; }
	virtual const char*			GetPowerBar2Text() { return "Defense"; }
	virtual const char*			GetPowerBar3Text() { return "Movement"; }

	virtual float				GetPowerBar1Value();
	virtual float				GetPowerBar2Value();
	virtual float				GetPowerBar3Value();

	virtual float				GetPowerBar1Size();
	virtual float				GetPowerBar2Size();
	virtual float				GetPowerBar3Size();

	void						DirtyNeedsOrders();
	virtual bool				NeedsOrders();

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual void				Fire();
	virtual void				Fire(class CNetworkParameters* p);
	void						FireWeapon();
	virtual void				FireWeapon(class CNetworkParameters* p);
	virtual void				FireProjectile(class CProjectile* pProjectile, Vector vecLandingSpot);
	virtual class CDigitanksWeapon*	CreateWeapon();
	weapon_t					GetCurrentWeapon() const { return m_eWeapon; }
	void						SetCurrentWeapon(weapon_t e, bool bNetworked = true);
	float						GetWeaponEnergy() const;
	size_t						GetNumWeapons() const;
	size_t						GetNumAllowedWeapons() const;
	weapon_t					GetWeapon(size_t iProjectile) const { return m_aeWeapons[iProjectile]; };
	bool						HasWeapon(weapon_t eWeapon) const;
	virtual bool				IsWaitingToFire() const { return m_flFireWeaponTime != 0; };

	void						FireSpecial();
	void						FireSpecial(Vector vecAim);
	bool						HasSpecialWeapons();

	void						FireMissileDefense(CProjectile* pTarget);
	bool						CanFireMissileDefense();

	virtual void				ClientUpdate(int iClient);

	virtual bool				TakesLavaDamage() { return true; }
	virtual void				TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit = true);
	virtual void				OnKilled(CBaseEntity* pKilledBy);
	DECLARE_ENTITY_OUTPUT(OnTakeShieldDamage);
	DECLARE_ENTITY_OUTPUT(OnTakeLaserDamage);

	virtual const TVector		GetGlobalOrigin() const;
	virtual Vector				GetRealOrigin() const;
	virtual EAngle				GetAngles() const;

	virtual void				PreRender() const;
	virtual Vector				GetRenderOrigin() const;
	virtual EAngle				GetRenderAngles() const;
	virtual void				ModifyContext(class CRenderingContext* pContext) const;
	virtual void				OnRender(class CGameRenderingContext* pContext) const;
	virtual void				RenderTurret(float flAlpha = 1.0f) const;
	virtual void				RenderShield() const;
	virtual float				RenderShieldScale() const { return 20.0f; };

	virtual bool				IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;

	virtual float				AvailableArea(int iArea) const;
	virtual int					GetNumAvailableAreas() const { return 1; };
	virtual bool				IsAvailableAreaActive(int iArea) const;

	virtual void				DrawSchema(float x, float y, float w, float h);

	virtual void				UpdateInfo(tstring& sInfo);

	void						GiveBonusPoints(size_t i, bool bPlayEffects = true);
	bool						HasBonusPoints();
	size_t						GetBonusPoints() { return m_iBonusPoints; };
	void						PromoteAttack();
	void						PromoteDefense();
	void						PromoteMovement();
	void						SetBonusPoints(class CNetworkParameters* p);
	void						TankPromoted(class CNetworkParameters* p);
	void						PromoteAttack(class CNetworkParameters* p);
	void						PromoteDefense(class CNetworkParameters* p);
	void						PromoteMovement(class CNetworkParameters* p);

	virtual void				DownloadComplete(size_t x, size_t y);

	void						Speak(size_t iSpeech);
	void						Speak(class CNetworkParameters* p);

	virtual float				FindHoverHeight(Vector vecPosition) const;

	float						HealthRechargeRate() const;
	float						ShieldRechargeRate() const;
	virtual float				BaseHealthRechargeRate() const { return 3.0f; };
	virtual float				BaseShieldRechargeRate() const { return 30.0f; };
	virtual float				GetTankSpeed() const { return 2.0f; };
	virtual float				TurnPerPower() const { return 9999; };
	virtual float				InitialMaxRange() const { return 70.0f; };
	virtual float				GetMaxRange() const { return InitialMaxRange() + m_flRangeBonus; };
	virtual float				InitialEffRange() const { return 50.0f; };
	virtual float				GetEffRange() const { return InitialEffRange() + m_flRangeBonus/2; };
	virtual float				GetMinRange() const { return 4.0f; };
	virtual float				GetTransitionTime() const { return 2.0f; };
	virtual float				ProjectileCurve() const { return -0.03f; };
	virtual float				FiringCone() const { return 360; };
	virtual float				BaseVisibleRange() const { return 75; };
	virtual size_t				FleetPoints() const { return 2; };
	virtual float				BobHeight() const { return 0.5f; };
	virtual float				MinRangeRadius() const { return 1; };
	virtual float				MaxRangeRadius() const { return 10; };
	virtual double				FirstProjectileTime() const;
	virtual float				SlowMovementFactor() const { return 0.5f; };
	virtual bool				TurningMatters() const { return false; };

	virtual bool				HasFiredWeapon() const { return m_bFiredWeapon; }

	bool						IsDisabled() const { return !!m_iTurnsDisabled; }
	void						Disable(size_t iTurns);
	DECLARE_ENTITY_OUTPUT(OnDisable);

	virtual size_t				GetTurretModel() { return m_iTurretModel; }

	virtual unittype_t			GetUnitType() const { return UNIT_TANK; }

	// AI stuff
	virtual void				SetInAttackTeam(bool b) { m_bInAttackTeam = b; };
	virtual bool				IsInAttackTeam() { return m_bInAttackTeam; };

	virtual void				SetFortifyPoint(CStructure* pStructure, Vector vecFortify);
	virtual void				RemoveFortifyPoint();
	virtual bool				HasFortifyPoint() { return m_bFortifyPoint; };
	virtual Vector				GetFortifyPoint() { return m_vecFortifyPoint; }

	void						GiveAirstrike() { m_iAirstrikes++; }

	static CMaterialHandle      GetAimBeamMaterial() { return s_hAimBeam; }
	static CMaterialHandle      GetAutoMoveMaterial() { return s_hAutoMove; }

protected:
	// Power remaining for this turn
	CNetworkedVariable<float>	m_flStartingPower;
	CNetworkedVariable<float>	m_flTotalPower;

	// Power used so far
	CNetworkedVariable<float>	m_flAttackPower;
	CNetworkedVariable<float>	m_flDefensePower;
	CNetworkedVariable<float>	m_flMovementPower;

	CNetworkedVariable<float>	m_flBonusAttackPower;
	CNetworkedVariable<float>	m_flBonusDefensePower;
	CNetworkedVariable<float>	m_flBonusMovementPower;
	CNetworkedVariable<size_t>	m_iBonusPoints;
	CNetworkedVariable<size_t>	m_iBonusLevel;

	CNetworkedVariable<float>	m_flRangeBonus;

	float						m_flMaxShieldStrength;

	CNetworkedVariable<float>	m_flShieldStrength;

	bool						m_bNeedsOrdersDirty;
	CNetworkedVariable<bool>	m_bNeedsOrders;

	float						m_flCurrentTurretYaw;
	CNetworkedVariable<float>	m_flGoalTurretYaw;

	CNetworkedVariable<double>	m_flStartedRock;
	CNetworkedVariable<float>	m_flRockIntensity;
	CNetworkedVector			m_vecRockDirection;

	Vector						m_vecPreviewMove;
	CNetworkedVector			m_vecPreviousOrigin;
	CNetworkedVariable<double>	m_flStartedMove;
	CNetworkedVariable<int>		m_iMoveType;

	float						m_flPreviewTurn;
	CNetworkedVariable<float>	m_flPreviousTurn;
	CNetworkedVariable<double>	m_flStartedTurn;

	bool						m_bPreviewAim;
	Vector						m_vecPreviewAim;
	CNetworkedVector			m_vecLastAim;

	bool						m_bDisplayAim;
	Vector						m_vecDisplayAim;
	float						m_flDisplayAimRadius;

	CEntityHandle<CBaseEntity>	m_hPreviewCharge;
	CNetworkedVariable<double>	m_flBeginCharge;
	CNetworkedVariable<double>	m_flEndCharge;
	CEntityHandle<CBaseEntity>	m_hChargeTarget;

	CNetworkedVariable<bool>	m_bHasCloak;
	CNetworkedVariable<bool>	m_bCloaked;

	CNetworkedVariable<bool>	m_bGoalMovePosition;
	CNetworkedVector			m_vecGoalMovePosition;

	CNetworkedVariable<bool>	m_bFiredWeapon;
	CNetworkedVariable<bool>	m_bActionTaken;
	CNetworkedVariable<bool>	m_bLostConcealment;

	double						m_flFireWeaponTime;
	size_t						m_iFireWeapons;
	CEntityHandle<CDigitanksWeapon>	m_hWeapon;

	double						m_flLastSpeech;
	double						m_flNextIdle;

	size_t						m_iTurretModel;
	size_t						m_iShieldModel;

	CNetworkedVariable<double>	m_flShieldPulse;

	CParticleSystemInstanceHandle m_hHoverParticles;
	CParticleSystemInstanceHandle m_hSmokeParticles;
	CParticleSystemInstanceHandle m_hFireParticles;
	CParticleSystemInstanceHandle m_hChargeParticles;

	CNetworkedVariable<bool>	m_bFortified;
	CNetworkedVariable<size_t>	m_iFortifyLevel;
	CNetworkedVariable<double>	m_flFortifyTime;

	bool						m_bStayPut;

	CNetworkedVariable<bool>	m_bSentried;

	float						m_flBobOffset;

	CNetworkedVariable<weapon_t> m_eWeapon;
	CNetworkedSTLVector<weapon_t> m_aeWeapons;

	CNetworkedVariable<size_t>	m_iAirstrikes;
	size_t						m_iMissileDefenses;
	double						m_flNextMissileDefense;

	CNetworkedVariable<size_t>	m_iTurnsDisabled;

	float						m_flGlowYaw;

	double						m_flNextHoverHeightCheck;

	// AI stuff
	bool						m_bInAttackTeam;
	bool						m_bFortifyPoint;
	Vector						m_vecFortifyPoint;
	CEntityHandle<class CStructure>	m_hFortifyDefending;

	static CMaterialHandle      s_hAimBeam;

	static CMaterialHandle      s_hAutoMove;
	static CMaterialHandle      s_hSupportGlow;

	static const char*			s_apszTankLines[];
};

#endif
