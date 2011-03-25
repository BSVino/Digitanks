#include "barbarians.h"

#include <models/models.h>
#include <renderer/renderer.h>

#include <digitanks/digitanksgame.h>
#include <digitanks/terrain.h>

REGISTER_ENTITY(CAutoTurret);

NETVAR_TABLE_BEGIN(CAutoTurret);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CAutoTurret);
SAVEDATA_TABLE_END();

void CAutoTurret::Precache()
{
	PrecacheModel(L"models/digitanks/autoturret.obj", true);
	PrecacheModel(L"models/digitanks/autoturret-turret.obj", true);
}

void CAutoTurret::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/autoturret.obj");
	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/autoturret-turret.obj");

	m_flMaxShieldStrength = m_flShieldStrength = 0;

	if (CNetwork::IsHost())
		Fortify();

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_eWeapon = PROJECTILE_SMALL;
}

void CAutoTurret::ModifyContext(CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetTerrain())
		return;

	pContext->SetColorSwap(DigitanksGame()->GetTerrain()->GetPrimaryTerrainColor());
}

REGISTER_ENTITY(CGridBug);

NETVAR_TABLE_BEGIN(CGridBug);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGridBug);
SAVEDATA_TABLE_END();

void CGridBug::Precache()
{
	PrecacheModel(L"models/digitanks/gridbug.obj", true);
	PrecacheModel(L"models/digitanks/gridbug-turret.obj", true);
}

void CGridBug::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/gridbug.obj");
	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/gridbug-turret.obj");

	m_flMaxShieldStrength = m_flShieldStrength = 0;

	m_aeWeapons.push_back(PROJECTILE_SMALL);
	m_eWeapon = PROJECTILE_SMALL;
}

void CGridBug::ModifyContext(CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetTerrain())
		return;

	pContext->SetColorSwap(DigitanksGame()->GetTerrain()->GetPrimaryTerrainColor());
}
