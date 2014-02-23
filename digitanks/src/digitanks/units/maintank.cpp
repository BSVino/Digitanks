#include "maintank.h"

#include <models/models.h>

REGISTER_ENTITY(CMainBattleTank);

NETVAR_TABLE_BEGIN(CMainBattleTank);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMainBattleTank);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CMainBattleTank);
INPUTS_TABLE_END();

void CMainBattleTank::Precache()
{
	PrecacheModel("models/digitanks/digitank-body.toy");
	PrecacheModel("models/digitanks/digitank-turret.toy");
	PrecacheModel("models/digitanks/digitank-shield.toy");
}

void CMainBattleTank::Spawn()
{
	BaseClass::Spawn();

	SetModel("models/digitanks/digitank-body.toy");
	m_iTurretModel = CModelLibrary::Get()->FindModel("models/digitanks/digitank-turret.toy");
	m_iShieldModel = CModelLibrary::Get()->FindModel("models/digitanks/digitank-shield.toy");

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_aeWeapons.push_back(PROJECTILE_MEDIUM);
	m_aeWeapons.push_back(PROJECTILE_LARGE);

	m_eWeapon = PROJECTILE_SMALL;
}
