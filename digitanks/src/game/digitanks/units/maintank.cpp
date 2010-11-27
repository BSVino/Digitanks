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

	m_aeProjectiles.push_back(PROJECTILE_SMALL);
	m_aeProjectiles.push_back(PROJECTILE_MEDIUM);
	m_aeProjectiles.push_back(PROJECTILE_LARGE);

	m_eProjectile = PROJECTILE_SMALL;
}
