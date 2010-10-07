#include "maintank.h"

#include <models/models.h>

NETVAR_TABLE_BEGIN(CMainBattleTank);
NETVAR_TABLE_END();

CMainBattleTank::CMainBattleTank()
{
	SetModel(L"models/digitanks/digitank-body.obj");
	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-turret.obj");
	m_iShieldModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-shield.obj");
}

void CMainBattleTank::Precache()
{
	PrecacheModel(L"models/digitanks/digitank-body.obj", true);
	PrecacheModel(L"models/digitanks/digitank-turret.obj", true);
	PrecacheModel(L"models/digitanks/digitank-shield.obj", true);
}
