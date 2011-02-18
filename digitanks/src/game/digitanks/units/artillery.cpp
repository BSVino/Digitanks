#include "artillery.h"

#include <maths.h>
#include <mtrand.h>

#include <network/network.h>
#include <models/models.h>

#include "ui/digitankswindow.h"
#include "ui/hud.h"
#include <digitanks/digitanksgame.h>
#include <digitanks/weapons/projectile.h>

REGISTER_ENTITY(CArtillery);

NETVAR_TABLE_BEGIN(CArtillery);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CArtillery);
SAVEDATA_TABLE_END();

void CArtillery::Precache()
{
	PrecacheModel(L"models/digitanks/artillery.obj", true);
	PrecacheModel(L"models/digitanks/artillery-move.obj", true);
}

void CArtillery::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/artillery-move.obj");

	m_flMaxShieldStrength = m_flShieldStrength = 0;

	m_aeWeapons.push_back(PROJECTILE_ARTILLERY);

	m_eWeapon = PROJECTILE_ARTILLERY;
}

bool CArtillery::AllowControlMode(controlmode_t eMode) const
{
	if (!IsFortified() && eMode == MODE_AIM)
		return false;

	return BaseClass::AllowControlMode(eMode);
}

void CArtillery::OnFortify()
{
	BaseClass::OnFortify();

	if (IsFortified() || IsFortifying())
		SetModel(L"models/digitanks/artillery.obj");
	else
		SetModel(L"models/digitanks/artillery-move.obj");
}

float CArtillery::ShieldRechargeRate() const
{
	return 0;
}

float CArtillery::HealthRechargeRate() const
{
	return 0.2f + GetSupportHealthRechargeBonus();
}

float CArtillery::FirstProjectileTime() const
{
	return RandomFloat(0.1f, 0.15f);
}

float CArtillery::TurnPerPower() const
{
	if (IsFortified())
		return 5;

	return BaseClass::TurnPerPower();
}
