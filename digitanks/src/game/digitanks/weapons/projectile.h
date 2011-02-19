#ifndef DT_PROJECTILE_H
#define DT_PROJECTILE_H

#include "baseweapon.h"
#include <digitanks/units/digitank.h>

class CProjectile : public CBaseWeapon
{
	REGISTER_ENTITY_CLASS(CProjectile, CBaseWeapon);

public:
								CProjectile();

public:
	virtual void				Precache();

	virtual void				Think();

	virtual float				GetBoundingRadius() const { return ShellRadius() + 1.0f; }

	virtual void				SpecialCommand();
	virtual bool				UsesSpecialCommand() { return true; };
	virtual eastl::string16		SpecialCommandHint() { return L"Space Bar To\nDetonate Bomb"; };
	virtual void				Fragment();

	virtual bool				MakesSounds() { return true; };
	virtual bool				UsesStandardExplosion() { return true; };

	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent);
	virtual bool				ShouldRender() const { return true; };
	virtual void				OnRender(class CRenderingContext* pContext, bool bTransparent);

	virtual void				OnDeleted();

	virtual bool				ShouldTouch(CBaseEntity* pOther) const;
	virtual bool				IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;
	virtual void				Touching(CBaseEntity* pOther);

	virtual void				OnExplode(CBaseEntity* pInstigator);
	virtual bool				ShouldPlayExplosionSound();

	virtual void				OnSetOwner(CDigitank* pOwner);
	virtual bool				ShouldBeVisible();

	virtual void				SetLandingSpot(Vector vecLandingSpot) { m_vecLandingSpot = vecLandingSpot; };

	virtual size_t				CreateTrailSystem();
	virtual void				CreateExplosionSystem();

	virtual void				ClientEnterGame();

	virtual float				ShieldDamageScale() { return 1; };
	virtual float				HealthDamageScale() { return 1; };
	virtual float				ShellRadius() const { return 0.5f; };
	virtual bool				ShouldExplode() { return true; };
	virtual bool				BombDropNoise() { return true; };
	virtual bool				SendsNotifications() { return true; };
	virtual size_t				Fragments() { return 0; };
	virtual size_t				Bounces() { return 0; };

protected:
	bool						m_bFallSoundPlayed;

	Vector						m_vecLandingSpot;

	size_t						m_iParticleSystem;

	bool						m_bFragmented;
	size_t						m_iBounces;

	bool						m_bMissileDefensesNotified;
};

class CSmallShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CSmallShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_SMALL; }
	virtual float				ShellRadius() { return 0.5f; };
	virtual float				ExplosionRadius();
	virtual float				PushDistance() { return 3.0f; };
	virtual float				RockIntensity() { return 0.4f; };
};

class CMediumShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CMediumShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_MEDIUM; }
	virtual float				ShellRadius() { return 1.0f; };
	virtual float				ExplosionRadius();
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };
};

class CLargeShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CLargeShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_LARGE; }
	virtual float				ShellRadius() { return 1.5f; };
	virtual float				ExplosionRadius();
	virtual float				PushDistance() { return 9.0f; };
	virtual float				RockIntensity() { return 1.0f; };
};

class CAOEShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CAOEShell, CProjectile);

public:
	virtual void				Precache();

	virtual void				CreateExplosionSystem();
	virtual size_t				CreateTrailSystem();

	virtual bool				UsesStandardExplosion() { return false; };
	virtual weapon_t			GetWeaponType() { return PROJECTILE_AOE; }
	virtual float				ShellRadius() { return 1.2f; };
	virtual float				ExplosionRadius();
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				HasDamageFalloff() { return false; };
};

class CEMP : public CProjectile
{
	REGISTER_ENTITY_CLASS(CEMP, CProjectile);

public:
	virtual void				Precache();

	virtual void				CreateExplosionSystem();
	virtual size_t				CreateTrailSystem();

	virtual bool				UsesStandardExplosion() { return false; };
	virtual weapon_t			GetWeaponType() { return PROJECTILE_EMP; }
	virtual float				ShieldDamageScale() { return 2; };
	virtual float				HealthDamageScale() { return 0.5f; };
	virtual bool				CreatesCraters() { return false; };
	virtual float				ShellRadius() { return 0.7f; };
	virtual float				ExplosionRadius() { return 6.0f; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
};

class CICBM : public CProjectile
{
	REGISTER_ENTITY_CLASS(CICBM, CProjectile);

public:
	virtual eastl::string16		SpecialCommandHint() { return L"Space Bar\nTo Fragment"; };

	virtual weapon_t			GetWeaponType() { return PROJECTILE_ICBM; }
	virtual float				ShellRadius() { return 1.2f; };
	virtual float				ExplosionRadius() { return 12.0f; };
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };
	virtual size_t				Fragments() { return 6; };
};

class CGrenade : public CProjectile
{
	REGISTER_ENTITY_CLASS(CGrenade, CProjectile);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual EAngle				GetRenderAngles() const;

	virtual void				OnExplode(CBaseEntity* pInstigator);

	virtual size_t				CreateTrailSystem() { return ~0; };
	virtual weapon_t			GetWeaponType() { return PROJECTILE_GRENADE; }
	virtual float				ShellRadius() { return 0.8f; };
	virtual float				ExplosionRadius() { return 16.0f; };
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };
	virtual size_t				Bounces() { return 2; };

protected:
	EAngle						m_angAngle;
	EAngle						m_angRotation;
};

class CDaisyChain : public CProjectile
{
	REGISTER_ENTITY_CLASS(CDaisyChain, CProjectile);

public:
	virtual void				Spawn();

	virtual void				OnExplode(CBaseEntity* pInstigator);

	virtual weapon_t			GetWeaponType() { return PROJECTILE_DAISYCHAIN; }
	virtual float				ShellRadius() { return 0.8f; };
	virtual float				ExplosionRadius() { return m_flExplosionRadius; };
	virtual float				PushDistance() { return 3.0f; };
	virtual float				RockIntensity() { return 0.7f; };

protected:
	float						m_flExplosionRadius;
};

class CClusterBomb : public CProjectile
{
	REGISTER_ENTITY_CLASS(CClusterBomb, CProjectile);

public:
	virtual void				Spawn();

	virtual void				SpecialCommand();

	virtual void				OnExplode(CBaseEntity* pInstigator);

	virtual weapon_t			GetWeaponType() { return PROJECTILE_CLUSTERBOMB; }
	virtual float				ShellRadius() { return 1.3f; };
	virtual float				ExplosionRadius() { return m_flExplosionRadius; };
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };

protected:
	float						m_flExplosionRadius;
};

class CEarthshaker : public CProjectile
{
	REGISTER_ENTITY_CLASS(CEarthshaker, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_EARTHSHAKER; }
	virtual float				ShellRadius() { return 0.8f; };
	virtual float				ExplosionRadius() { return 22.0f; };
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };
};

class CSploogeShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CSploogeShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_SPLOOGE; }
	virtual float				ShellRadius() { return 0.2f; };
	virtual float				ExplosionRadius() { return 0.0f; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				UsesSpecialCommand() { return false; };
};

class CTractorBomb : public CProjectile
{
	REGISTER_ENTITY_CLASS(CTractorBomb, CProjectile);

public:
	virtual void				Precache();

	virtual void				CreateExplosionSystem();
	virtual size_t				CreateTrailSystem();

	virtual bool				UsesStandardExplosion() { return false; };
	virtual weapon_t			GetWeaponType() { return PROJECTILE_TRACTORBOMB; }
	virtual float				ShellRadius() { return 0.8f; };
	virtual float				ExplosionRadius() { return 0.0f; };
	virtual float				PushRadius() { return 40.0f; };
	virtual float				PushDistance() { return 20.0f; };
	virtual float				RockIntensity() { return 1.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				HasDamageFalloff() { return false; };
};

class CArtilleryShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CArtilleryShell, CProjectile);

public:
	virtual void				Precache();

	virtual void				CreateExplosionSystem();
	virtual size_t				CreateTrailSystem();

	virtual bool				UsesStandardExplosion() { return false; };
	virtual weapon_t			GetWeaponType() { return PROJECTILE_ARTILLERY; }
	virtual float				ExplosionRadius() { return 6.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual float				ShieldDamageScale() { return 2; };
	virtual float				HealthDamageScale() { return 0.5f; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
	virtual bool				UsesSpecialCommand() { return false; };
};

class CArtilleryAoE : public CProjectile
{
	REGISTER_ENTITY_CLASS(CArtilleryAoE, CProjectile);

public:
	virtual void				Precache();

	virtual void				CreateExplosionSystem();
	virtual size_t				CreateTrailSystem();

	virtual bool				UsesStandardExplosion() { return false; };
	virtual weapon_t			GetWeaponType() { return PROJECTILE_ARTILLERY_AOE; }
	virtual float				ExplosionRadius() { return 12.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual float				ShieldDamageScale() { return 2; };
	virtual float				HealthDamageScale() { return 0.5f; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
	virtual bool				UsesSpecialCommand() { return false; };
	virtual bool				HasDamageFalloff() { return false; };
};

class CArtilleryICBM : public CProjectile
{
	REGISTER_ENTITY_CLASS(CArtilleryICBM, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_ARTILLERY_ICBM; }
	virtual float				ExplosionRadius() { return 8.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual float				ShieldDamageScale() { return 2; };
	virtual float				HealthDamageScale() { return 0.5f; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
	virtual size_t				Fragments() { return 4; };
	virtual bool				UsesSpecialCommand() { return false; };
};

class CDevastator : public CProjectile
{
	REGISTER_ENTITY_CLASS(CDevastator, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_DEVASTATOR; }
	virtual float				ExplosionRadius() { return 24.0f; };
	virtual bool				CreatesCraters() { return true; };
	virtual float				ShieldDamageScale() { return 2; };
	virtual float				HealthDamageScale() { return 1; };
	virtual float				PushDistance() { return 1.0f; };
	virtual float				RockIntensity() { return 1.0f; };
	virtual bool				UsesSpecialCommand() { return false; };
};

class CInfantryFlak : public CProjectile
{
	REGISTER_ENTITY_CLASS(CInfantryFlak, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_FLAK; }
	virtual bool				MakesSounds() { return true; };
	virtual float				ShellRadius() { return 0.2f; };
	virtual bool				ShouldExplode() { return false; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				BombDropNoise() { return false; };
	virtual bool				SendsNotifications() { return true; };
	virtual size_t				CreateTrailSystem();
	virtual bool				UsesSpecialCommand() { return false; };
};

class CTorpedo : public CProjectile
{
	REGISTER_ENTITY_CLASS(CTorpedo, CProjectile);

public:
								CTorpedo();

public:
	virtual void				Precache();

	virtual void				Spawn();
	virtual void				Think();

	virtual bool				ShouldTouch(CBaseEntity* pOther) const;
	virtual void				Touching(CBaseEntity* pOther);

	virtual void				Explode(CBaseEntity* pInstigator = NULL);

	virtual size_t				CreateTrailSystem();

	virtual bool				UsesStandardExplosion() { return false; };
	virtual weapon_t			GetWeaponType() { return PROJECTILE_TORPEDO; }
	virtual bool				MakesSounds() { return true; };
	virtual float				ShellRadius() { return 0.35f; };
	virtual bool				ShouldExplode() { return true; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				BombDropNoise() { return false; };
	virtual bool				SendsNotifications() { return true; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
	virtual bool				UsesSpecialCommand() { return false; };
	virtual float				ShieldDamageScale() { return 0; };
	virtual float				HealthDamageScale() { return 1; };

protected:
	bool						m_bBurrowing;
};

class CTreeCutter : public CProjectile
{
	REGISTER_ENTITY_CLASS(CTreeCutter, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_TREECUTTER; }
	virtual bool				MakesSounds() { return true; };
	virtual float				ShellRadius() { return 0.5f; };
	virtual float				ExplosionRadius() { return 8.0f; };
	virtual bool				ShouldExplode() { return true; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				BombDropNoise() { return true; };
	virtual bool				SendsNotifications() { return false; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
};

#endif
