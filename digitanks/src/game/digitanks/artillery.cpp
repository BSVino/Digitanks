#include "artillery.h"

#include <maths.h>
#include <mtrand.h>

#include <network/network.h>
#include <models/models.h>

#include "ui/digitankswindow.h"
#include "ui/hud.h"
#include "digitanksgame.h"
#include "projectile.h"

NETVAR_TABLE_BEGIN(CArtillery);
NETVAR_TABLE_END();

void CArtillery::Precache()
{
	PrecacheModel(L"models/digitanks/artillery.obj", true);
	PrecacheModel(L"models/digitanks/artillery-move.obj", true);
}

void CArtillery::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/artillery-move.obj");

	m_flFrontMaxShieldStrength = m_flFrontShieldStrength = m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flRearMaxShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flRearShieldStrength = 0;

	m_bFortified = false;
}

void CArtillery::SetAttackPower(float flAttackPower)
{
	// No defense power.
	BaseClass::SetAttackPower(1);
}

CProjectile* CArtillery::CreateProjectile()
{
	return GameServer()->Create<CArtilleryShell>("CArtilleryShell");
}

float CArtillery::GetProjectileDamage()
{
	return GetAttackPower()/6;
}

bool CArtillery::AllowControlMode(controlmode_t eMode) const
{
	if (eMode == MODE_FIRE)
		return false;

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

float CArtillery::FireProjectileTime() const
{
	return RandomFloat(0.25f, 0.3f);
}

float CArtillery::TurnPerPower() const
{
	if (IsFortified())
		return 5;

	return BaseClass::TurnPerPower();
}
