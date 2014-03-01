#include "baseweapon.h"

#include <digitanksgame.h>
#include <dt_camera.h>
#include <dt_renderer.h>

REGISTER_ENTITY(CDigitanksWeapon);

NETVAR_TABLE_BEGIN(CDigitanksWeapon);
	NETVAR_DEFINE(float, m_flTimeExploded);
	NETVAR_DEFINE(CEntityHandle<CDigitanksEntity>, m_hOwner);
	NETVAR_DEFINE(float, m_flDamage);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitanksWeapon);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flTimeExploded);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bShouldRender);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CDigitanksEntity>, m_hOwner);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flDamage);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CDigitanksWeapon);
INPUTS_TABLE_END();

#define _T(x) x

void CDigitanksWeapon::Precache()
{
	PrecacheSound(_T("sound/explosion.wav"));
}

void CDigitanksWeapon::Spawn()
{
	BaseClass::Spawn();

	m_flTimeExploded = 0;
	m_bShouldRender = true;
	m_flDamage = 0;
}

void CDigitanksWeapon::ClientSpawn()
{
	BaseClass::ClientSpawn();

	Vector vecOrigin = GetGlobalOrigin();
	if (m_hOwner != NULL)
		vecOrigin = m_hOwner->GetGlobalOrigin();

	if (ShouldBeVisible() || DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), vecOrigin) > 0)
		m_bShouldRender = true;
	else
		m_bShouldRender = false;
}

CDigitanksEntity* CDigitanksWeapon::GetOwner() const
{
	return dynamic_cast<CDigitanksEntity*>(m_hOwner.GetPointer());
}

void CDigitanksWeapon::OnSetOwner(CBaseEntity* pOwner)
{
	m_hOwner = pOwner;
	if (pOwner)
		SetGlobalOrigin(pOwner->GetGlobalOrigin() + Vector(0, 0, 1));

	float flBonusDamage = 0;
	CDigitank* pTank = dynamic_cast<CDigitank*>(pOwner);
	if (pOwner && pTank)
		flBonusDamage = pTank->GetBonusAttackDamage();

	m_flDamage = (GetWeaponDamage(GetWeaponType()) + flBonusDamage)/(float)GetWeaponShells(GetWeaponType());

	BaseClass::OnSetOwner(pOwner);
}

void CDigitanksWeapon::Think()
{
	BaseClass::Think();

	if (!GameNetwork()->IsHost())
		return;

	if (m_flTimeExploded != 0.0 && GameServer()->GetGameTime() - m_flTimeExploded > 2.0)
		Delete();
}

void CDigitanksWeapon::Explode(CBaseEntity* pInstigator)
{
	bool bHit = false;
	if (m_flDamage > 0)
		bHit = DigitanksGame()->Explode(m_hOwner, this, ExplosionRadius(), m_flDamage + GetBonusDamage(), pInstigator, ToDigitanksPlayer(m_hOwner.GetPointer()));

	OnExplode(pInstigator);

	SetGlobalVelocity(Vector());
	SetGlobalGravity(Vector());

	m_flTimeExploded = GameServer()->GetGameTime();

	if (m_bShouldRender)
		DigitanksGame()->GetDigitanksCamera()->Shake(GetGlobalOrigin(), ShakeCamera());

	bool bCanSeeOwner;
	if (m_hOwner != NULL && DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), m_hOwner->GetGlobalOrigin()) > 0)
		bCanSeeOwner = true;
	else
		bCanSeeOwner = false;

	if (MakesSounds() && ShouldPlayExplosionSound() || bCanSeeOwner)
		EmitSound(_T("sound/explosion.wav"));

	if (m_hOwner != NULL && dynamic_cast<CTerrain*>(pInstigator) && !bHit)
	{
		CDigitank* pOwner = dynamic_cast<CDigitank*>(GetOwner());
		if (pOwner)
			pOwner->Speak(TANKSPEECH_MISSED);
	}

	if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), GetGlobalOrigin()) > 0.5f)
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
	0.0f,	// turret missile
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
	30.0f,	// turret missile
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
	1,	// turret missile
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
	0,		// turret missile
	0,		// charging ram

	0,		// airstrike
	0,		// fireworks
};

static const tchar* g_apszWeaponNames[WEAPON_MAX] =
{
	_T("None"),

	_T("Little Boy"),
	_T("Fat Man"),
	_T("Big Mama"),
	_T("Plasma Charge"),
	_T("Electro-Magnetic Pulse"),
	_T("Repulsor Bomb"),
	_T("Grapeshot"),
	_T("WAN Bomb"),
	_T("Grenade"),
	_T("Daisy Chain"),
	_T("Cluster Bomb"),
	_T("Earthshaker"),

	_T("Machine Gun"),
	_T("Tree Cutter"),
	_T("Fragmentation Ray"),
	_T("Torpedo"),
	_T("Artillery Shell"),
	_T("Artillery Plasma Charge"),
	_T("Artillery WAN Bomb"),
	_T("The Devastator"),

	_T("Camera-Guided Missile"),
	_T("Fragmentation Ray"),
	_T("Missile Defense"),
	_T("Turret Missile"),
	_T("Charging RAM Attack"),

	_T("Airstrike"),
	_T("Fireworks"),
};

static const tchar* g_apszWeaponDescriptions[WEAPON_MAX] =
{
	_T("None"),

	_T("This light projectile bomb does just enough damage to keep the enemy from regenerating his shields next turn."),
	_T("This medium projectile bomb is a good trade-off between firepower and defense."),
	_T("This heavy projectile bomb packs a mean punch at the cost of your shield energy."),
	_T("This large area of effect projectile bomb is good for attacking a group of tanks."),
	_T("This medium projectile bomb sends out an electonic pulse that does extra damage against shields but relatively little damage against tank hulls."),
	_T("This light projectile bomb does very little damage, but can knock tanks around a great deal."),
	_T("This light attack fires a stream of small projectiles at your enemy. It can be deadly at close range, but loses effectiveness with distance."),
	_T("This heavy projectile breaks into multiple fragments before it falls down onto its target."),
	_T("This heavy projectile bounces three times before it explodes. Chuck it into holes to find out-of-reach targets."),
	_T("This medium projectile can't be stopped. Even after it explodes, it continues on its previous path through the cleared terrain. Good for punching through dirt."),
	_T("This heavy projectile breaks into a large number of smaller pieces on impact for maximum distruction."),
	_T("This projectile bomb does very little damage but is effective at creating a rather large hole in the ground."),

	_T("The Resistor's light mounted gun is its main firepower. However, it can't hit flying units such as the Rogue."),
	_T("The Tree Cutter is not a weapon, but a tool that can be used to clear trees to make a path or remove hiding spots. It also deals a small amount of damage to enemy units."),
	_T("This weapon emits a wall of rays in one direction. It's not blocked by objects or terrain and can hit flying units with no problem, so it's great for taking care of those pesky enemy Rogues."),
	_T("Torpedos do damage only if the target's shields are down, but they also disable units and sever structures from their network, forcing them to become neutral."),
	_T("The Artillery fires a salvo of shells which do double damage against shields but half damage against structures."),
	_T("These charges are similar to the standard shells but have a large radius and pack a larger whallop."),
	_T("The Artillery Wide Area Network Bomb fragments into many pieces before raining chaos and destruction down on your foes."),
	_T("The Devastor is the ultimate weapon of destruction."),

	_T("This special missile can be steered by using your mouse. Just aim it in the general direction and use your mouse to control the missile from first-person."),
	_T("This weapon emits a wall of rays in one direction. Great for hitting multiple tanks in a line. This weapon goes through any objects or terrian and has no range limitations."),
	_T("These small anti-air missiles can detonate an incoming projectile before it reaches the tank. Warning: Not effective against some projectile types!"),
	_T("Firewalls use these small missiles to defend their base."),
	_T("Combine your engine and attack energies to charge an enemy unit with a RAM attack that bypasses shields. This attack requires your movement energy, if you move your tank you won't be able to execute it."),

	_T("Rain fire and brimstone upon your enemies."),
	_T("You won! Fireworks are in order."),
};

float CDigitanksWeapon::GetWeaponEnergy(weapon_t eProjectile)
{
	return g_aflWeaponEnergies[eProjectile];
}

float CDigitanksWeapon::GetWeaponDamage(weapon_t eProjectile)
{
	return g_aflWeaponDamages[eProjectile];
}

size_t CDigitanksWeapon::GetWeaponShells(weapon_t eProjectile)
{
	return g_aiWeaponShells[eProjectile];
}

float CDigitanksWeapon::GetWeaponFireInterval(weapon_t eProjectile)
{
	return g_aflWeaponFireInterval[eProjectile];
}

const tchar* CDigitanksWeapon::GetWeaponName(weapon_t eProjectile)
{
	return g_apszWeaponNames[eProjectile];
}

const tchar* CDigitanksWeapon::GetWeaponDescription(weapon_t eProjectile)
{
	return g_apszWeaponDescriptions[eProjectile];
}

bool CDigitanksWeapon::IsWeaponPrimarySelectionOnly(weapon_t eProjectile)
{
	if (eProjectile == PROJECTILE_CAMERAGUIDED)
		return true;

	if (eProjectile == WEAPON_CHARGERAM)
		return true;

	return false;
}

