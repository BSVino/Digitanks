#include "autoturret.h"

#include <models/models.h>

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
