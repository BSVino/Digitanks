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
	PrecacheModel(_T("models/digitanks/digitank-body.obj"), true);
	PrecacheModel(_T("models/digitanks/digitank-turret.obj"), true);
	PrecacheModel(_T("models/digitanks/digitank-shield.obj"), true);
}

void CMainBattleTank::Spawn()
{
	BaseClass::Spawn();

	SetModel(_T("models/digitanks/digitank-body.obj"));
	m_iTurretModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/digitank-turret.obj"));
	m_iShieldModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/digitank-shield.obj"));

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_aeWeapons.push_back(PROJECTILE_MEDIUM);
	m_aeWeapons.push_back(PROJECTILE_LARGE);

	m_eWeapon = PROJECTILE_SMALL;
}
