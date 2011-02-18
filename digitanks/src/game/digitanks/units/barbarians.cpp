#include "barbarians.h"

#include <models/models.h>

REGISTER_ENTITY(CAutoTurret);

NETVAR_TABLE_BEGIN(CAutoTurret);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CAutoTurret);
SAVEDATA_TABLE_END();

void CAutoTurret::Precache()
{
	PrecacheModel(L"models/digitanks/autoturret.obj", true);
	PrecacheModel(L"models/digitanks/autoturret-turret.obj", true);
}

void CAutoTurret::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/autoturret.obj");
	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/autoturret-turret.obj");

	m_flFrontMaxShieldStrength = m_flFrontShieldStrength = m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flRearMaxShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flRearShieldStrength = 0;

	Fortify();

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_eWeapon = PROJECTILE_SMALL;
}


REGISTER_ENTITY(CGridBug);

NETVAR_TABLE_BEGIN(CGridBug);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGridBug);
SAVEDATA_TABLE_END();

void CGridBug::Precache()
{
	PrecacheModel(L"models/digitanks/digitank-body.obj", true);
	PrecacheModel(L"models/digitanks/digitank-turret.obj", true);
}

void CGridBug::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/digitank-body.obj");
	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-turret.obj");

	m_flFrontMaxShieldStrength = m_flFrontShieldStrength = m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flRearMaxShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flRearShieldStrength = 0;

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_eWeapon = PROJECTILE_SMALL;
}
