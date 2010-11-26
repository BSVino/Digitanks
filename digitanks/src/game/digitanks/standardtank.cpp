#include "standardtank.h"

#include <maths.h>

#include <models/models.h>

NETVAR_TABLE_BEGIN(CStandardTank);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CStandardTank);
SAVEDATA_TABLE_END();

void CStandardTank::Precache()
{
	PrecacheModel(L"models/digitanks/digitank-body.obj", true);
	PrecacheModel(L"models/digitanks/digitank-turret.obj", true);
	PrecacheModel(L"models/digitanks/digitank-shield.obj", true);
}

void CStandardTank::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/digitank-body.obj");
	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-turret.obj");
	m_iShieldModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-shield.obj");
}

float CStandardTank::ProjectileCurve() const
{
	return RemapValClamped((GetLastAim() - GetOrigin()).Length(), 60, 100, -0.03f, -0.006f);
}
