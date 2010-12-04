#ifndef DT_BASEWEAPON_H
#define DT_BASEWEAPON_H

#include <baseentity.h>

#include <digitanks/dt_common.h>

class CBaseWeapon : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CBaseWeapon, CBaseEntity);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				SetOwner(class CDigitank* pOwner);
	virtual void				OnSetOwner(class CDigitank* pOwner) {};
	virtual bool				ShouldBeVisible() { return true; };

	virtual void				Think();

	void						Explode(CBaseEntity* pInstigator = NULL);
	virtual void				OnExplode(CBaseEntity* pInstigator) {};
	virtual bool				ShouldPlayExplosionSound() { return true; };

	virtual weapon_t			GetWeaponType() { return WEAPON_NONE; }
	virtual float				ExplosionRadius() { return 4.0f; };
	virtual bool				MakesSounds() { return true; };

	virtual bool				CreatesCraters() { return true; };
	virtual float				PushRadius() { return 20.0f; };
	virtual float				RockIntensity() { return 0.5f; };
	virtual float				PushDistance() { return 4.0f; };
	virtual bool				HasDamageFalloff() { return true; };

	static float				GetWeaponEnergy(weapon_t eProjectile);
	static float				GetWeaponDamage(weapon_t eProjectile);
	static size_t				GetWeaponShells(weapon_t eProjectile);
	static float				GetWeaponFireInterval(weapon_t eProjectile);
	static char16_t*			GetWeaponName(weapon_t eProjectile);
	static char16_t*			GetWeaponDescription(weapon_t eProjectile);

protected:
	float						m_flTimeCreated;
	float						m_flTimeExploded;

	CEntityHandle<CDigitank>	m_hOwner;
	float						m_flDamage;

	bool						m_bShouldRender;
};

#endif
