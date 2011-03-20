#include "baseweapon.h"

#include <digitanks/digitanksgame.h>
#include <digitanks/dt_camera.h>
#include <digitanks/dt_renderer.h>

REGISTER_ENTITY(CBaseWeapon);

NETVAR_TABLE_BEGIN(CBaseWeapon);
	NETVAR_DEFINE(float, m_flTimeExploded);
	NETVAR_DEFINE(CEntityHandle<CDigitank>, m_hOwner);
	NETVAR_DEFINE(float, m_flDamage);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBaseWeapon);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flTimeExploded);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CDigitank>, m_hOwner);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flDamage);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bShouldRender);
SAVEDATA_TABLE_END();

void CBaseWeapon::Precache()
{
	PrecacheSound(L"sound/explosion.wav");
}

void CBaseWeapon::Spawn()
{
	BaseClass::Spawn();

	m_flTimeExploded = 0;
	m_bShouldRender = true;
	m_flDamage = 0;
}

void CBaseWeapon::ClientSpawn()
{
	BaseClass::ClientSpawn();

	Vector vecOrigin = GetOrigin();
	if (m_hOwner != NULL)
		vecOrigin = m_hOwner->GetOrigin();

	if (ShouldBeVisible() || DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), vecOrigin) > 0)
		m_bShouldRender = true;
	else
		m_bShouldRender = false;
}

void CBaseWeapon::SetOwner(CDigitank* pOwner)
{
	m_hOwner = pOwner;
	if (pOwner)
		SetOrigin(pOwner->GetOrigin() + Vector(0, 1, 0));

	float flBonusDamage = 0;
	if (pOwner)
		flBonusDamage = pOwner->GetBonusAttackDamage();

	m_flDamage = GetWeaponDamage(GetWeaponType())/(float)GetWeaponShells(GetWeaponType()) + flBonusDamage;

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
		bHit = DigitanksGame()->Explode(m_hOwner, this, ExplosionRadius(), m_flDamage + GetBonusDamage(), pInstigator, (m_hOwner == NULL)?NULL:m_hOwner->GetTeam());

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

	1.0f,	// small
	4.0f,	// medium
	8.0f,	// large
	6.0f,	// AoE
	6.0f,	// emp
	2.0f,	// tractor bomb
	2.0f,	// splooge
	6.0f,	// icbm
	4.0f,	// grenade
	7.0f,	// daisy chain
	7.0f,	// clusterbomb
	3.0f,	// earthshaker

	3.0f,	// machine gun
	3.0f,	// treecutter
	7.0f,	// laser
	3.0f,	// torpedo
	3.0f,	// artillery
	5.0f,	// artillery aoe
	7.0f,	// artillery icbm
	9.0f,	// artillery devastator

	7.0f,	// camera guided missile
	7.0f,	// laser
	0.0f,	// missile defense
	7.0f,	// charging ram

	0.0f,	// airstrike
	0.0f,	// fireworks
};

static float g_aflWeaponDamages[WEAPON_MAX] =
{
	0.0f,

	20.0f,	// small
	50.0f,	// medium
	80.0f,	// large
	40.0f,	// AoE
	60.0f,	// emp
	10.0f,	// tractor bomb
	70.0f,	// splooge
	20.0f,	// icbm
	80.0f,	// grenade
	40.0f,	// daisy chain
	70.0f,	// clusterbomb
	10.0f,	// earthshaker

	60.0f,	// machine gun
	10.0f,	// tree cutter
	50.0f,	// laser
	30.0f,	// torpedo
	15.0f,	// artillery
	25.0f,	// artillery aoe
	70.0f,	// artillery icbm
	90.0f,	// artillery devastator

	50.0f,	// camera guided missile
	50.0f,	// laser
	0.0f,	// missile defense
	0.0f,	// charging ram

	50.0f,	// airstrike
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
	1,	// tree cutter
	1,	// laser
	1,	// torpedo
	3,	// artillery
	3,	// artillery aoe
	3,	// artillery icbm
	1,	// artillery devastator


	1,	// camera guided missile
	1,	// laser
	1,	// missile defense
	1,	// charging ram

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
	0,		// tree cutter
	0,		// laser
	0,		// torpedo
	0.25f,	// artillery
	0.25f,	// artillery aoe
	0.25f,	// artillery icbm
	0,		// artillery devastator


	0,		// camera guided missile
	0,		// laser
	0,		// missile defense
	0,		// charging ram

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
	L"Repulsor Bomb",
	L"Grapeshot",
	L"WAN Bomb",
	L"Grenade",
	L"Daisy Chain",
	L"Cluster Bomb",
	L"Earthshaker",

	L"Machine Gun",
	L"Tree Cutter",
	L"Fragmentation Ray",
	L"Torpedo",
	L"Artillery Shell",
	L"Artillery Plasma Charge",
	L"Artillery WAN Bomb",
	L"The Devastator",

	L"Camera-Guided Missile",
	L"Fragmentation Ray",
	L"Missile Defense",
	L"Charging RAM Attack",

	L"Airstrike",
	L"Fireworks",
};

static char16_t* g_apszWeaponDescriptions[WEAPON_MAX] =
{
	L"None",

	L"This light projectile bomb does just enough damage to keep the enemy from regenerating his shields next turn.",
	L"This medium projectile bomb is a good trade-off between firepower and defense.",
	L"This heavy projectile bomb packs a mean punch at the cost of your shield energy.",
	L"This large area of effect projectile bomb is good for attacking a group of tanks.",
	L"This medium projectile bomb sends out an electonic pulse that does extra damage against shields but relatively little damage against tank hulls.",
	L"This light projectile bomb does very little damage, but can knock tanks around a great deal.",
	L"This light attack fires a stream of small projectiles at your enemy. It can be deadly at close range, but loses effectiveness with distance.",
	L"This heavy projectile breaks into multiple fragments before it falls down onto its target.",
	L"This heavy projectile bounces three times before it explodes. Chuck it into holes to find out-of-reach targets.",
	L"This medium projectile can't be stopped. Even after it explodes, it continues on its previous path through the cleared terrain. Good for punching through dirt.",
	L"This heavy projectile breaks into a large number of smaller pieces on impact for maximum distruction.",
	L"This projectile bomb does very little damage but is effective at creating a rather large hole in the ground.",

	L"The Resistor's light mounted gun is its main firepower. However, it can't hit flying units such as the Rogue.",
	L"The Tree Cutter is not a weapon, but a tool that can be used to clear trees to make a path or remove hiding spots. It also deals a small amount of damage to enemy units.",
	L"This weapon emits a wall of rays in one direction. It's not blocked by objects or terrain and can hit flying units with no problem, so it's great for taking care of those pesky enemy Rogues.",
	L"Torpedos do damage only if the target's shields are down, but they also disable units and sever structures from their network, forcing them to become neutral.",
	L"The Artillery fires a salvo of shells which do double damage against shields but half damage against structures.",
	L"These charges are similar to the standard shells but have a large radius and pack a larger whallop.",
	L"The Artillery Wide Area Network Bomb fragments into many pieces before raining chaos and destruction down on your foes.",
	L"The Devastor is the ultimate weapon of destruction.",

	L"This special missile can be steered by using your mouse. Just aim it in the general direction and use your mouse to control the missile from first-person.",
	L"This weapon emits a wall of rays in one direction. Great for hitting multiple tanks in a line. This weapon goes through any objects or terrian and has no range limitations.",
	L"These small anti-air missiles can detonate an incoming projectile before it reaches the tank. Warning: Not effective against some projectile types!",
	L"Combine your engine and attack energies to charge an enemy unit with a RAM attack that bypasses shields. This attack requires your movement energy, if you move your tank you won't be able to execute it.",

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

bool CBaseWeapon::IsWeaponPrimarySelectionOnly(weapon_t eProjectile)
{
	if (eProjectile == PROJECTILE_CAMERAGUIDED)
		return true;

	if (eProjectile == WEAPON_CHARGERAM)
		return true;

	return false;
}

