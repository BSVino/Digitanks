#include "maintank.h"

#include <models/models.h>

REGISTER_ENTITY(CMainBattleTank);

CMainBattleTank::CMainBattleTank()
{
	SetModel(L"models/digitanks/digitank-body.obj");
	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-turret.obj");
	m_iShieldModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-shield.obj");
}

void CMainBattleTank::Precache()
{
	PrecacheModel(L"models/digitanks/digitank-body.obj");
	PrecacheModel(L"models/digitanks/digitank-turret.obj");
	PrecacheModel(L"models/digitanks/digitank-shield.obj");
}
