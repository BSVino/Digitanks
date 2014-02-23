#include "barbarians.h"

#include <models/models.h>
#include <renderer/renderer.h>
#include <renderer/game_renderingcontext.h>

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
	PrecacheModel("models/digitanks/autoturret.toy");
	PrecacheModel("models/digitanks/autoturret-turret.toy");
}

void CBugTurret::Spawn()
{
	BaseClass::Spawn();

	SetModel("models/digitanks/autoturret.toy");
	m_iTurretModel = CModelLibrary::Get()->FindModel("models/digitanks/autoturret-turret.toy");

	m_flMaxShieldStrength = m_flShieldStrength = 0;

	if (GameNetwork()->IsHost())
		Fortify();

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_eWeapon = PROJECTILE_SMALL;
}

void CBugTurret::ModifyContext(CRenderingContext* pContext) const
{
	BaseClass::ModifyContext(pContext);

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetTerrain())
		return;

	pContext->SetUniform("bColorSwapInAlpha", true);

	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		pContext->SetUniform("vecColorSwap", Vector(DigitanksGame()->GetTerrain()->GetPrimaryTerrainColor())*2/3);
	else
		pContext->SetUniform("vecColorSwap", GetPlayerOwner()->GetColor());
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
	PrecacheModel("models/digitanks/gridbug.toy");
	PrecacheModel("models/digitanks/gridbug-turret.toy");
}

void CGridBug::Spawn()
{
	BaseClass::Spawn();

	SetModel("models/digitanks/gridbug.toy");
	m_iTurretModel = CModelLibrary::Get()->FindModel("models/digitanks/gridbug-turret.toy");

	m_flMaxShieldStrength = m_flShieldStrength = 0;

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_eWeapon = PROJECTILE_SMALL;
}

void CGridBug::ModifyContext(CRenderingContext* pContext) const
{
	BaseClass::ModifyContext(pContext);

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetTerrain())
		return;

	pContext->SetUniform("bColorSwapInAlpha", true);

	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		pContext->SetUniform("vecColorSwap", Vector(DigitanksGame()->GetTerrain()->GetPrimaryTerrainColor())*2/3);
	else
		pContext->SetUniform("vecColorSwap", GetPlayerOwner()->GetColor());
}
