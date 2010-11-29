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

	virtual bool				ShouldSimulate() const { return true; };
	virtual bool				ShouldTouch(CBaseEntity* pOther) const;
	virtual bool				IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;
	virtual void				Touching(CBaseEntity* pOther);

	virtual void				Explode(CBaseEntity* pInstigator = NULL);

	virtual void				SetOwner(CDigitank* pOwner);
	virtual void				SetForce(Vector vecForce) { SetVelocity(vecForce); };
	virtual void				SetLandingSpot(Vector vecLandingSpot) { m_vecLandingSpot = vecLandingSpot; };

	virtual size_t				CreateParticleSystem();

	virtual void				ClientEnterGame();

	virtual projectile_t		GetProjectileType() { return PROJECTILE_SMALL; }
	virtual float				ShieldDamageScale() { return 1; };
	virtual float				HealthDamageScale() { return 1; };
	virtual float				ShellRadius() { return 0.5f; };
	virtual float				ExplosionRadius() { return 4.0f; };
	virtual float				PushDistance() { return 4.0f; };
	virtual bool				ShouldExplode() { return true; };
	virtual bool				CreatesCraters() { return true; };
	virtual bool				BombDropNoise() { return true; };
	virtual bool				SendsNotifications() { return true; };
	virtual bool				HasDamageFalloff() { return true; };

	static float				GetProjectileEnergy(projectile_t eProjectile);
	static float				GetProjectileDamage(projectile_t eProjectile);
	static char16_t*			GetProjectileName(projectile_t eProjectile);

protected:
	float						m_flTimeCreated;
	float						m_flTimeExploded;

	bool						m_bFallSoundPlayed;

	CEntityHandle<CDigitank>	m_hOwner;
	float						m_flDamage;
	Vector						m_vecLandingSpot;
	bool						m_bShouldRender;

	size_t						m_iParticleSystem;
};

class CSmallShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CSmallShell, CProjectile);

public:
	virtual projectile_t		GetProjectileType() { return PROJECTILE_SMALL; }
	virtual float				ShellRadius() { return 0.5f; };
	virtual float				ExplosionRadius() { return 4.0f; };
	virtual float				PushDistance() { return 2.0f; };
};

class CMediumShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CMediumShell, CProjectile);

public:
	virtual projectile_t		GetProjectileType() { return PROJECTILE_MEDIUM; }
	virtual float				ShellRadius() { return 1.0f; };
	virtual float				ExplosionRadius() { return 8.0f; };
	virtual float				PushDistance() { return 4.0f; };
};

class CLargeShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CLargeShell, CProjectile);

public:
	virtual projectile_t		GetProjectileType() { return PROJECTILE_LARGE; }
	virtual float				ShellRadius() { return 1.5f; };
	virtual float				ExplosionRadius() { return 12.0f; };
	virtual float				PushDistance() { return 6.0f; };
};

class CAOEShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CAOEShell, CProjectile);

public:
	virtual projectile_t		GetProjectileType() { return PROJECTILE_AOE; }
	virtual float				ShellRadius() { return 1.0f; };
	virtual float				ExplosionRadius() { return 30.0f; };
	virtual float				PushDistance() { return 4.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				HasDamageFalloff() { return false; };
};

class CArtilleryShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CArtilleryShell, CProjectile);

public:
	virtual projectile_t		GetProjectileType() { return PROJECTILE_ARTILLERY; }
	virtual bool				CreatesCraters() { return false; };
	virtual float				ShieldDamageScale() { return 2; };
	virtual float				HealthDamageScale() { return 0.5f; };
	virtual float				PushDistance() { return 0.0f; };
};

class CInfantryFlak : public CProjectile
{
	REGISTER_ENTITY_CLASS(CInfantryFlak, CProjectile);

public:
	virtual projectile_t		GetProjectileType() { return PROJECTILE_FLAK; }
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

	virtual projectile_t		GetProjectileType() { return PROJECTILE_TORPEDO; }
	virtual bool				MakesSounds() { return true; };
	virtual float				ShellRadius() { return 0.35f; };
	virtual bool				ShouldExplode() { return true; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				BombDropNoise() { return false; };
	virtual bool				SendsNotifications() { return false; };
	virtual float				PushDistance() { return 0.0f; };

protected:
	bool						m_bBurrowing;
};

class CFireworks : public CProjectile
{
	REGISTER_ENTITY_CLASS(CFireworks, CProjectile);

public:
	virtual bool				ShouldTouch(CBaseEntity* pOther) const;

	virtual projectile_t		GetProjectileType() { return PROJECTILE_FIREWORKS; }
	virtual bool				BombDropNoise() { return false; };
	virtual float				PushDistance() { return 5.0f; };
};

#endif
