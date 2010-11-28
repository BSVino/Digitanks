#include "standardtank.h"

#include <maths.h>

#include <models/models.h>
#include "digitanks/digitanksgame.h"

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

	m_aeProjectiles.push_back(PROJECTILE_SMALL);
	m_aeProjectiles.push_back(PROJECTILE_MEDIUM);
	m_aeProjectiles.push_back(PROJECTILE_LARGE);
	m_aeProjectiles.push_back(PROJECTILE_AOE);

	m_eProjectile = PROJECTILE_SMALL;
}

float CStandardTank::ProjectileCurve() const
{
	Vector vecAim;
	if (DigitanksGame()->GetControlMode() == MODE_AIM && GetDigitanksTeam()->IsSelected(this))
		vecAim = GetPreviewAim();
	else
		vecAim = GetLastAim();

	return RemapValClamped((vecAim - GetOrigin()).Length(), 60, 100, -0.03f, -0.006f);
}