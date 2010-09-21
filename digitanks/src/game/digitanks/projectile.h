#ifndef DT_PROJECTILE_H
#define DT_PROJECTILE_H

#include <baseentity.h>
#include "digitank.h"

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
	virtual float				ShellRadius() { return 0.5f; };
	virtual bool				ShouldRender() const { return true; };
	virtual void				OnRender();

	virtual void				OnDeleted();

	virtual bool				ShouldSimulate() const { return true; };
	virtual bool				ShouldTouch(CBaseEntity* pOther) const;
	virtual bool				IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;
	virtual void				Touching(CBaseEntity* pOther);

	virtual bool				ShouldExplode() { return true; };
	virtual bool				CreatesCraters() { return true; };
	virtual bool				BombDropNoise() { return true; };

	virtual bool				SendsNotifications() { return true; };

	virtual void				SetOwner(CDigitank* pOwner);
	virtual void				SetDamage(float flDamage) { m_flDamage = flDamage; };
	virtual void				SetForce(Vector vecForce) { SetVelocity(vecForce); };
	virtual void				SetLandingSpot(Vector vecLandingSpot) { m_vecLandingSpot = vecLandingSpot; };

	virtual size_t				CreateParticleSystem();

	virtual float				ShieldDamageScale() { return 1; };
	virtual float				HealthDamageScale() { return 1; };

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

class CShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CShell, CProjectile);
};

class CArtilleryShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CArtilleryShell, CProjectile);

public:
	virtual bool				CreatesCraters() { return false; };

	virtual float				ShieldDamageScale() { return 2; };
	virtual float				HealthDamageScale() { return 0.5f; };
};

class CInfantryFlak : public CProjectile
{
	REGISTER_ENTITY_CLASS(CInfantryFlak, CProjectile);

public:
	virtual bool				MakesSounds() { return true; };
	virtual float				ShellRadius() { return 0.2f; };
	virtual bool				ShouldExplode() { return false; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				BombDropNoise() { return false; };
	virtual bool				SendsNotifications() { return false; };
	virtual size_t				CreateParticleSystem();
};

#endif