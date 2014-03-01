#ifndef DT_BASEWEAPON_H
#define DT_BASEWEAPON_H

#include <game/entities/baseentity.h>

#include <dt_common.h>

class CDigitanksWeapon : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CDigitanksWeapon, CBaseEntity);

public:
	virtual void				Precache();
	virtual void				Spawn();
	virtual void				ClientSpawn();

	virtual class CDigitanksEntity*	GetOwner() const;
	virtual void				OnSetOwner(class CBaseEntity* pOwner);

	virtual bool				ShouldBeVisible() { return true; };

	virtual void				Think();

	virtual void				SpecialCommand() {};
	virtual bool				UsesSpecialCommand() { return false; };
	virtual tstring				SpecialCommandHint() { return ""; };
	virtual float				GetBonusDamage() { return 0; };

	void						Explode(CBaseEntity* pInstigator = NULL);
	virtual void				OnExplode(CBaseEntity* pInstigator) {};
	virtual bool				ShouldPlayExplosionSound() { return true; };
	virtual bool				HasExploded() { return m_flTimeExploded > 0; }
	virtual bool				HasFragmented() { return false; };

	virtual weapon_t			GetWeaponType() { return WEAPON_NONE; }
	virtual float				ExplosionRadius() const { return 4.0f; };
	virtual bool				MakesSounds() { return true; };

	virtual bool				CreatesCraters() { return true; };
	virtual float				PushRadius() { return 20.0f; };
	virtual float				RockIntensity() { return 0.5f; };
	virtual float				PushDistance() { return 4.0f; };
	virtual bool				HasDamageFalloff() { return true; };
	virtual float				ShakeCamera() { return 3.0f; };

	static float				GetWeaponEnergy(weapon_t eProjectile);
	static float				GetWeaponDamage(weapon_t eProjectile);
	static size_t				GetWeaponShells(weapon_t eProjectile);
	static float				GetWeaponFireInterval(weapon_t eProjectile);
	static const tchar*			GetWeaponName(weapon_t eProjectile);
	static const tchar*			GetWeaponDescription(weapon_t eProjectile);
	static bool					IsWeaponPrimarySelectionOnly(weapon_t eProjectile);

protected:
	CNetworkedVariable<double>	m_flTimeExploded;

	bool						m_bShouldRender;

	CNetworkedVariable<float>	m_flDamage;
};

#endif
