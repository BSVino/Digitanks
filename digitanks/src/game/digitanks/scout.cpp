#include "scout.h"

#include <network/network.h>

#include "digitanksgame.h"
#include "projectile.h"

CScout::CScout()
{
	SetModel(L"models/digitanks/scout.obj");

	m_flFrontMaxShieldStrength = m_flFrontShieldStrength = m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flRearMaxShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flRearShieldStrength = 0;

	m_bFortified = false;
}

void CScout::Precache()
{
	PrecacheModel(L"models/digitanks/scout.obj", true);
}

bool CScout::AllowControlMode(controlmode_t eMode) const
{
	if (eMode == MODE_FIRE)
		return false;

	if (eMode == MODE_AIM)
		return false;

	return BaseClass::AllowControlMode(eMode);
}

void CScout::Move()
{
	Vector vecStart = GetOrigin();
	Vector vecEnd = m_vecDesiredMove;

	BaseClass::Move();

	SetPreviewTurn(VectorAngles(vecEnd-vecStart).y);
	SetDesiredTurn();
	Turn();
}

float CScout::ShieldRechargeRate() const
{
	return 0;
}

float CScout::HealthRechargeRate() const
{
	return 0.2f + GetSupportHealthRechargeBonus();
}
