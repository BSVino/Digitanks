#include "barbarians.h"

#include <models/models.h>
#include <renderer/renderer.h>

#include <digitanksgame.h>
#include <terrain.h>

REGISTER_ENTITY(CBugTurret);

NETVAR_TABLE_BEGIN(CBugTurret);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBugTurret);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBugTurret);
INPUTS_TABLE_END();

void CBugTurret::Precache()
{
	PrecacheModel(_T("models/digitanks/autoturret.obj"), true);
	PrecacheModel(_T("models/digitanks/autoturret-turret.obj"), true);
}

void CBugTurret::Spawn()
{
	BaseClass::Spawn();

	SetModel(_T("models/digitanks/autoturret.obj"));
	m_iTurretModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/autoturret-turret.obj"));

	m_flMaxShieldStrength = m_flShieldStrength = 0;

	if (GameNetwork()->IsHost())
		Fortify();

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_eWeapon = PROJECTILE_SMALL;
}

void CBugTurret::ModifyContext(CRenderingContext* pContext, bool bTransparent) const
{
	BaseClass::ModifyContext(pContext, bTransparent);

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetTerrain())
		return;

	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		pContext->SetColorSwap(Vector(DigitanksGame()->GetTerrain()->GetPrimaryTerrainColor())*2/3);
	else
		pContext->SetColorSwap(GetTeam()->GetColor());
}

float CBugTurret::GetFortifyAttackPowerBonus()
{
	return 0;
}

float CBugTurret::GetFortifyDefensePowerBonus()
{
	return 0;
}

REGISTER_ENTITY(CGridBug);

NETVAR_TABLE_BEGIN(CGridBug);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGridBug);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CGridBug);
INPUTS_TABLE_END();

void CGridBug::Precache()
{
	PrecacheModel(_T("models/digitanks/gridbug.obj"), true);
	PrecacheModel(_T("models/digitanks/gridbug-turret.obj"), true);
}

void CGridBug::Spawn()
{
	BaseClass::Spawn();

	SetModel(_T("models/digitanks/gridbug.obj"));
	m_iTurretModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/gridbug-turret.obj"));

	m_flMaxShieldStrength = m_flShieldStrength = 0;

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_eWeapon = PROJECTILE_SMALL;
}

void CGridBug::ModifyContext(CRenderingContext* pContext, bool bTransparent) const
{
	BaseClass::ModifyContext(pContext, bTransparent);

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetTerrain())
		return;

	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		pContext->SetColorSwap(Vector(DigitanksGame()->GetTerrain()->GetPrimaryTerrainColor())*2/3);
	else
		pContext->SetColorSwap(GetTeam()->GetColor());
}
