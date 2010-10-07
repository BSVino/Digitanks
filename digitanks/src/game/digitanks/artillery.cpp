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

CArtillery::CArtillery()
{
	m_flFrontMaxShieldStrength = m_flFrontShieldStrength = m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flRearMaxShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flRearShieldStrength = 0;

	m_bFortified = false;

	m_iFireProjectiles = 0;
	m_flLastProjectileFire = 0;
}

void CArtillery::Precache()
{
	PrecacheModel(L"models/digitanks/artillery.obj", true);
	PrecacheModel(L"models/digitanks/artillery-move.obj", true);
}

void CArtillery::Spawn()
{
	SetModel(L"models/digitanks/artillery-move.obj");
}

void CArtillery::SetAttackPower(float flAttackPower)
{
	// No defense power.
	BaseClass::SetAttackPower(1);
}

void CArtillery::Think()
{
	BaseClass::Think();

	if (m_iFireProjectiles && Game()->GetGameTime() > m_flLastProjectileFire + 0.3f)
	{
		m_iFireProjectiles--;
		FireProjectile();
		m_flLastProjectileFire = Game()->GetGameTime();
	}
}

void CArtillery::Fire()
{
	if (!HasDesiredAim())
		return;

	float flDistanceSqr = (GetDesiredAim() - GetOrigin()).LengthSqr();
	if (flDistanceSqr > GetMaxRange()*GetMaxRange())
		return;

	if (flDistanceSqr < GetMinRange()*GetMinRange())
		return;

	if (m_bFiredWeapon)
		return;

	m_bFiredWeapon = true;

	float flAttackPower = m_flTotalPower * m_flAttackSplit;
	m_flTotalPower -= flAttackPower;
	m_flAttackPower += flAttackPower;

	if (CNetwork::IsHost())
		m_iFireProjectiles = 3;

	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);

	CDigitanksWindow::Get()->GetHUD()->UpdateTurnButton();
}

CProjectile* CArtillery::CreateProjectile()
{
	return Game()->Create<CArtilleryShell>("CArtilleryShell");
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

void CArtillery::Fortify()
{
	BaseClass::Fortify();

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

float CArtillery::TurnPerPower() const
{
	if (IsFortified())
		return 5;

	return BaseClass::TurnPerPower();
}
