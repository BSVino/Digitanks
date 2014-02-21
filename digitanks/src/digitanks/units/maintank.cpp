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
	PrecacheModel(_T("models/digitanks/digitank-body.toy"), true);
	PrecacheModel(_T("models/digitanks/digitank-turret.toy"), true);
	PrecacheModel(_T("models/digitanks/digitank-shield.toy"), true);
}

void CMainBattleTank::Spawn()
{
	BaseClass::Spawn();

	SetModel(_T("models/digitanks/digitank-body.toy"));
	m_iTurretModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/digitank-turret.toy"));
	m_iShieldModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/digitank-shield.toy"));

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_aeWeapons.push_back(PROJECTILE_MEDIUM);
	m_aeWeapons.push_back(PROJECTILE_LARGE);

	m_eWeapon = PROJECTILE_SMALL;
}
