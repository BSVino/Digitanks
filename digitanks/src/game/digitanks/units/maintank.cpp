#include "maintank.h"

#include <models/models.h>

NETVAR_TABLE_BEGIN(CMainBattleTank);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMainBattleTank);
SAVEDATA_TABLE_END();

void CMainBattleTank::Precache()
{
	PrecacheModel(L"models/digitanks/digitank-body.obj", true);
	PrecacheModel(L"models/digitanks/digitank-turret.obj", true);
	PrecacheModel(L"models/digitanks/digitank-shield.obj", true);
}

void CMainBattleTank::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/digitank-body.obj");
	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-turret.obj");
	m_iShieldModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-shield.obj");

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_aeWeapons.push_back(PROJECTILE_MEDIUM);
	m_aeWeapons.push_back(PROJECTILE_LARGE);
	m_aeWeapons.push_back(PROJECTILE_AOE);
	m_aeWeapons.push_back(PROJECTILE_CLUSTERBOMB);
	m_aeWeapons.push_back(PROJECTILE_SPLOOGE);

	m_eWeapon = PROJECTILE_SMALL;
}
