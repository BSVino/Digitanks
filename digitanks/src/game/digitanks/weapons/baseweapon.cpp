#include "baseweapon.h"

#include <digitanks/digitanksgame.h>
#include <digitanks/dt_camera.h>
#include <digitanks/dt_renderer.h>

NETVAR_TABLE_BEGIN(CBaseWeapon);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBaseWeapon);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flTimeCreated);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flTimeExploded);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CDigitank>, m_hOwner);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flDamage);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bShouldRender);
SAVEDATA_TABLE_END();

void CBaseWeapon::Precache()
{
	PrecacheSound(L"sound/explosion.wav");
}

void CBaseWeapon::Spawn()
{
	BaseClass::Spawn();

	m_flTimeCreated = GameServer()?GameServer()->GetGameTime():0;

	m_flTimeExploded = 0;
	m_bShouldRender = true;
	m_flDamage = 0;
}

void CBaseWeapon::SetOwner(CDigitank* pOwner)
{
	m_hOwner = pOwner;
	if (pOwner)
		SetOrigin(pOwner->GetOrigin() + Vector(0, 1, 0));

	if (ShouldBeVisible() || DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_hOwner->GetOrigin()) > 0)
		m_bShouldRender = true;
	else
		m_bShouldRender = false;

	m_flDamage = GetWeaponDamage(GetWeaponType())/GetWeaponShells(GetWeaponType());

	OnSetOwner(pOwner);
}

void CBaseWeapon::Think()
{
	BaseClass::Think();

	if (m_flTimeExploded != 0.0f && GameServer()->GetGameTime() - m_flTimeExploded > 2.0f)
		Delete();
}

void CBaseWeapon::Explode(CBaseEntity* pInstigator)
{
	bool bHit = false;
	if (m_flDamage > 0)
		bHit = DigitanksGame()->Explode(m_hOwner, this, ExplosionRadius(), m_flDamage, pInstigator, (m_hOwner == NULL)?NULL:m_hOwner->GetTeam());

	OnExplode(pInstigator);

	SetVelocity(Vector());
	SetGravity(Vector());

	m_flTimeExploded = GameServer()->GetGameTime();

	if (m_bShouldRender)
		DigitanksGame()->GetDigitanksCamera()->Shake(GetOrigin(), 3);

	bool bCanSeeOwner;
	if (m_hOwner != NULL && DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_hOwner->GetOrigin()) > 0)
		bCanSeeOwner = true;
	else
		bCanSeeOwner = false;

	if (MakesSounds() && ShouldPlayExplosionSound() || bCanSeeOwner)
		EmitSound(L"sound/explosion.wav");

	if (m_hOwner != NULL && dynamic_cast<CTerrain*>(pInstigator) && !bHit)
		m_hOwner->Speak(TANKSPEECH_MISSED);

	if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), GetOrigin()) > 0.5f)
		DigitanksGame()->GetDigitanksRenderer()->BloomPulse();
}

static float g_aflWeaponEnergies[WEAPON_MAX] =
{
	0.0f,

	2.0f,	// small
	5.0f,	// medium
	8.0f,	// large
	6.0f,	// AoE
	6.0f,	// emp
	4.0f,	// tractor bomb
	3.0f,	// splooge
	6.0f,	// icbm
	4.0f,	// grenade
	7.0f,	// daisy chain
	7.0f,	// clusterbomb
	4.0f,	// earthshaker

	6.0f,	// machine gun
	3.0f,	// torpedo
	8.0f,	// artillery

	7.0f,	// camera guided missile
	7.0f,	// laser
	0.0f,	// missile defense

	0.0f,	// airstrike
	0.0f,	// fireworks
};

static float g_aflWeaponDamages[WEAPON_MAX] =
{
	0.0f,

	2.0f,	// small
	5.0f,	// medium
	8.0f,	// large
	4.0f,	// AoE
	6.0f,	// emp
	1.0f,	// tractor bomb
	7.0f,	// splooge
	7.0f,	// icbm
	8.0f,	// grenade
	4.0f,	// daisy chain
	7.0f,	// clusterbomb
	1.0f,	// earthshaker

	0.12f,	// machine gun
	0.0f,	// torpedo
	1.3f,	// artillery

	7.0f,	// camera guided missile
	5.0f,	// laser
	0.0f,	// missile defense

	5.0f,	// airstrike
	0.0f,	// fireworks
};

static size_t g_aiWeaponShells[WEAPON_MAX] =
{
	0,

	1,	// small
	1,	// medium
	1,	// large
	1,	// AoE
	1,	// emp
	1,	// tractor bomb
	20,	// splooge
	1,	// icbm
	1,	// grenade
	1,	// daisy chain
	1,	// clusterbomb
	1,	// earthshaker

	20,	// machine gun
	1,	// torpedo
	3,	// artillery

	1,	// camera guided missile
	1,	// laser
	1,	// missile defense

	1,	// airstrike
	1,	// fireworks
};

static float g_aflWeaponFireInterval[WEAPON_MAX] =
{
	0,

	0,		// small
	0,		// medium
	0,		// large
	0,		// AoE
	0,		// emp
	0,		// tractor bomb
	0.01f,	// splooge
	0,		// icbm
	0,		// grenade
	0,		// daisy chain
	0,		// clusterbomb
	0,		// earthshaker

	0.1f,	// machine gun
	0,		// torpedo
	0.25f,	// artillery

	0,		// camera guided missile
	0,		// laser
	0,		// missile defense

	0,		// airstrike
	0,		// fireworks
};

static char16_t* g_apszWeaponNames[WEAPON_MAX] =
{
	L"None",

	L"Little Boy",
	L"Fat Man",
	L"Big Mama",
	L"Plasma Charge",
	L"Electro-Magnetic Pulse",
	L"Tractor Bomb",
	L"Grapeshot",
	L"ICBM",
	L"Grenade",
	L"Daisy Chain",
	L"Cluster Bomb",
	L"Earthshaker",

	L"Flak Cannon",
	L"Torpedo",
	L"Artillery Shell",

	L"Camera-Guided Missile",
	L"Laser",
	L"Missile Defense",

	L"Airstrike",
	L"Fireworks",
};

static char16_t* g_apszWeaponDescriptions[WEAPON_MAX] =
{
	L"None",

	L"This light projectile bomb does just enough damage to keep the enemy from regenerating his shields next turn.",
	L"This medium projectile bomb is a good tradeoff between firepower and defense.",
	L"This heavy projectile bomb packs a mean punch at the cost of your defense for the next turn.",
	L"This large area of effect projectile bomb is good for attacking a group of tanks.",
	L"This medium projectile bomb sends out an electonic pulse that does extra damage against shields but relatively little damage against tank hulls.",
	L"This light projectile bomb does very little damage, but can knock tanks around a great deal.",
	L"This light attack fires a stream of small projectiles at your enemy. It can be deadly at close range, but loses effectiveness with distance.",
	L"This heavy projectile breaks into multiple fragments before it falls down onto its target.",
	L"This heavy projectile bounces three times before it explodes. Chunk it into holes to find out-of-reach targets.",
	L"This medium projectile can't be stopped. Even after it explodes it continues on its previous path through the cleared terrain. Good for punching through dirt.",
	L"This heavy projectile breaks into a large number of smaller pieces on impact for maximum distruction.",
	L"This projectile bomb does very little damage but is effective at creating a rather large hole in the ground.",

	L"The infantry's light mounted gun is its main firepower.",
	L"This special attack targets supply lines. It does no damage but it can sever structures from the enemy network and force them to become neutral.",
	L"The artillery fires a salvo of shells which do double damage against shields but half damage against structures.",

	L"This special missile can be steered by using your mouse. Just aim it in the general direction and use your mouse to control the missile from first person.",
	L"This weapon emits a wall of lasers in one direction. Great for hitting multiple tanks in a line. This weapon also has no range limitations.",
	L"These small anti-air missiles can detonate an incoming projectile before it reaches the tank. Warning: Not effective against some projectile types!",

	L"Rain fire and brimstone upon your enemies.",
	L"You won! Fireworks are in order.",
};

float CBaseWeapon::GetWeaponEnergy(weapon_t eProjectile)
{
	return g_aflWeaponEnergies[eProjectile];
}

float CBaseWeapon::GetWeaponDamage(weapon_t eProjectile)
{
	return g_aflWeaponDamages[eProjectile];
}

size_t CBaseWeapon::GetWeaponShells(weapon_t eProjectile)
{
	return g_aiWeaponShells[eProjectile];
}

float CBaseWeapon::GetWeaponFireInterval(weapon_t eProjectile)
{
	return g_aflWeaponFireInterval[eProjectile];
}

char16_t* CBaseWeapon::GetWeaponName(weapon_t eProjectile)
{
	return g_apszWeaponNames[eProjectile];
}

char16_t* CBaseWeapon::GetWeaponDescription(weapon_t eProjectile)
{
	return g_apszWeaponDescriptions[eProjectile];
}

