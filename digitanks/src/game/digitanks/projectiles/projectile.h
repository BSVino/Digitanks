#ifndef DT_PROJECTILE_H
#define DT_PROJECTILE_H

#include <baseentity.h>
#include <digitanks/units/digitank.h>

class CProjectile : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CProjectile, CBaseEntity);

public:
								CProjectile();

public:
	virtual void				Precache();

	virtual void				Think();

	virtual bool				MakesSounds() { return true; };

	virtual void				ModifyContext(class CRenderingContext* pContext);
	virtual bool				ShouldRender() const { return true; };
	virtual void				OnRender();

	virtual void				OnDeleted();

	virtual bool				ShouldTouch(CBaseEntity* pOther) const;
	virtual bool				IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;
	virtual void				Touching(CBaseEntity* pOther);

	virtual void				Explode(CBaseEntity* pInstigator = NULL);

	virtual void				SetOwner(CDigitank* pOwner);
	virtual void				SetForce(Vector vecForce) { SetVelocity(vecForce); };
	virtual void				SetLandingSpot(Vector vecLandingSpot) { m_vecLandingSpot = vecLandingSpot; };

	virtual size_t				CreateParticleSystem();

	virtual void				ClientEnterGame();

	virtual weapon_t			GetWeaponType() { return PROJECTILE_SMALL; }
	virtual float				ShieldDamageScale() { return 1; };
	virtual float				HealthDamageScale() { return 1; };
	virtual float				ShellRadius() { return 0.5f; };
	virtual float				ExplosionRadius() { return 4.0f; };
	virtual float				PushRadius() { return 20.0f; };
	virtual float				PushDistance() { return 4.0f; };
	virtual float				RockIntensity() { return 0.5f; };
	virtual bool				ShouldExplode() { return true; };
	virtual bool				CreatesCraters() { return true; };
	virtual bool				BombDropNoise() { return true; };
	virtual bool				SendsNotifications() { return true; };
	virtual bool				HasDamageFalloff() { return true; };
	virtual size_t				Fragments() { return 0; };

	static float				GetWeaponEnergy(weapon_t eProjectile);
	static float				GetWeaponDamage(weapon_t eProjectile);
	static size_t				GetWeaponShells(weapon_t eProjectile);
	static float				GetWeaponFireInterval(weapon_t eProjectile);
	static char16_t*			GetWeaponName(weapon_t eProjectile);
	static char16_t*			GetWeaponDescription(weapon_t eProjectile);

protected:
	float						m_flTimeCreated;
	float						m_flTimeExploded;

	bool						m_bFallSoundPlayed;

	CEntityHandle<CDigitank>	m_hOwner;
	float						m_flDamage;
	Vector						m_vecLandingSpot;
	bool						m_bShouldRender;

	size_t						m_iParticleSystem;

	bool						m_bFragmented;
};

class CSmallShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CSmallShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_SMALL; }
	virtual float				ShellRadius() { return 0.5f; };
	virtual float				ExplosionRadius() { return 6.0f; };
	virtual float				PushDistance() { return 3.0f; };
	virtual float				RockIntensity() { return 0.4f; };
};

class CMediumShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CMediumShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_MEDIUM; }
	virtual float				ShellRadius() { return 1.0f; };
	virtual float				ExplosionRadius() { return 12.0f; };
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };
};

class CLargeShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CLargeShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_LARGE; }
	virtual float				ShellRadius() { return 1.5f; };
	virtual float				ExplosionRadius() { return 18.0f; };
	virtual float				PushDistance() { return 9.0f; };
	virtual float				RockIntensity() { return 1.0f; };
};

class CAOEShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CAOEShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_AOE; }
	virtual float				ShellRadius() { return 1.2f; };
	virtual float				ExplosionRadius() { return 30.0f; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				HasDamageFalloff() { return false; };
};

class CEMP : public CProjectile
{
	REGISTER_ENTITY_CLASS(CEMP, CProjectile);

public:
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
	virtual weapon_t			GetWeaponType() { return PROJECTILE_ICBM; }
	virtual float				ShellRadius() { return 1.2f; };
	virtual float				ExplosionRadius() { return 12.0f; };
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };
	virtual size_t				Fragments() { return 6; };
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
};

class CTractorBomb : public CProjectile
{
	REGISTER_ENTITY_CLASS(CTractorBomb, CProjectile);

public:
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
	virtual weapon_t			GetWeaponType() { return PROJECTILE_ARTILLERY; }
	virtual bool				CreatesCraters() { return false; };
	virtual float				ShieldDamageScale() { return 2; };
	virtual float				HealthDamageScale() { return 0.5f; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
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
	virtual bool				SendsNotifications() { return false; };
	virtual size_t				CreateParticleSystem();
};

class CTorpedo : public CProjectile
{
	REGISTER_ENTITY_CLASS(CTorpedo, CProjectile);

public:
								CTorpedo();

public:
	virtual void				Think();

	virtual bool				ShouldTouch(CBaseEntity* pOther) const;
	virtual void				Touching(CBaseEntity* pOther);

	virtual void				Explode(CBaseEntity* pInstigator = NULL);

	virtual weapon_t			GetWeaponType() { return PROJECTILE_TORPEDO; }
	virtual bool				MakesSounds() { return true; };
	virtual float				ShellRadius() { return 0.35f; };
	virtual bool				ShouldExplode() { return true; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				BombDropNoise() { return false; };
	virtual bool				SendsNotifications() { return false; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };

protected:
	bool						m_bBurrowing;
};

#endif
