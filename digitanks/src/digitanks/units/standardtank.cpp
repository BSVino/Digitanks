#include "standardtank.h"

#include <maths.h>

#include <models/models.h>
#include <tinker/application.h>
#include <renderer/renderer.h>

#include "digitanksgame.h"

REGISTER_ENTITY(CStandardTank);

NETVAR_TABLE_BEGIN(CStandardTank);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CStandardTank);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CStandardTank);
INPUTS_TABLE_END();

void CStandardTank::Precache()
{
	PrecacheModel(_T("models/digitanks/digitank-body.toy"), true);
	PrecacheModel(_T("models/digitanks/digitank-turret.toy"), true);
	PrecacheModel(_T("models/digitanks/digitank-shield.toy"), true);
}

void CStandardTank::Spawn()
{
	BaseClass::Spawn();

	SetModel(_T("models/digitanks/digitank-body.toy"));
	m_iTurretModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/digitank-turret.toy"));
	m_iShieldModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/digitank-shield.toy"));

	m_aeWeapons.push_back(PROJECTILE_MEDIUM);
	m_aeWeapons.push_back(PROJECTILE_LARGE);
	m_aeWeapons.push_back(PROJECTILE_EMP);
	m_aeWeapons.push_back(PROJECTILE_ICBM);
	m_aeWeapons.push_back(PROJECTILE_GRENADE);
	m_aeWeapons.push_back(PROJECTILE_DAISYCHAIN);
	m_aeWeapons.push_back(PROJECTILE_CLUSTERBOMB);
	m_aeWeapons.push_back(PROJECTILE_SPLOOGE);
	m_aeWeapons.push_back(PROJECTILE_TRACTORBOMB);
	m_aeWeapons.push_back(PROJECTILE_EARTHSHAKER);
	m_aeWeapons.push_back(WEAPON_LASER);
#ifndef __linux__
	m_aeWeapons.push_back(PROJECTILE_CAMERAGUIDED);
#endif
	m_aeWeapons.push_back(WEAPON_CHARGERAM);

	m_eWeapon = PROJECTILE_MEDIUM;

	m_flMaxShieldStrength = m_flShieldStrength = 200;
}

void CStandardTank::ModifyContext(CRenderingContext* pContext) const
{
	BaseClass::ModifyContext(pContext);

	pContext->Scale(2, 2, 2);
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
