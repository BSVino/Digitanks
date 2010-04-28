#include "digitank.h"

#include "maths.h"

CDigitank::CDigitank()
{
	m_flTotalPower = 10;
	m_flAttackPower = 3;
	m_flDefensePower = 4;
	m_flMovementPower = 3;

	m_bDesiredMove = false;

	m_flTotalHealth = 10;
	m_flHealth = 10;

	m_flMaxShieldStrength = m_flFrontShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flBackShieldStrength = 5;
}

float CDigitank::GetAttackPower(bool bPreview)
{
	float flMovementLength;
	
	if (bPreview)
	{
		flMovementLength = (m_vecPreviewMove - GetOrigin()).LengthSqr();

		if (flMovementLength > m_flTotalPower * m_flTotalPower)
			return m_flAttackPower/m_flTotalPower;

		flMovementLength = sqrt(flMovementLength);

		return RemapVal(flMovementLength, 0, m_flTotalPower, m_flAttackPower/(m_flAttackPower+m_flDefensePower), 0);
	}

	return m_flAttackPower/m_flTotalPower;
}

float CDigitank::GetDefensePower(bool bPreview)
{
	float flMovementLength;

	if (bPreview)
	{
		flMovementLength = (m_vecPreviewMove - GetOrigin()).LengthSqr();

		if (flMovementLength > m_flTotalPower * m_flTotalPower)
			return m_flDefensePower/m_flTotalPower;

		flMovementLength = sqrt(flMovementLength);

		return RemapVal(flMovementLength, 0, m_flTotalPower, m_flDefensePower/(m_flAttackPower+m_flDefensePower), 0);
	}

	return m_flDefensePower/m_flTotalPower;
}

float CDigitank::GetMovementPower(bool bPreview)
{
	float flMovementLength;
	
	if (bPreview)
	{
		flMovementLength = (m_vecPreviewMove - GetOrigin()).LengthSqr();

		if (flMovementLength > m_flTotalPower * m_flTotalPower)
			return m_flMovementPower/m_flTotalPower;

		flMovementLength = sqrt(flMovementLength);
		return flMovementLength/m_flTotalPower;
	}

	return m_flMovementPower/m_flTotalPower;
}

float CDigitank::GetFrontShieldStrength()
{
	return m_flFrontShieldStrength/m_flMaxShieldStrength;
}

float CDigitank::GetLeftShieldStrength()
{
	return m_flLeftShieldStrength/m_flMaxShieldStrength;
}

float CDigitank::GetRightShieldStrength()
{
	return m_flRightShieldStrength/m_flMaxShieldStrength;
}

float CDigitank::GetRearShieldStrength()
{
	return m_flBackShieldStrength/m_flMaxShieldStrength;
}

void CDigitank::StartTurn()
{
	m_flMovementPower = 0;
	m_flAttackPower = m_flAttackPower/(m_flAttackPower+m_flDefensePower)*m_flTotalPower;
	m_flDefensePower = m_flTotalPower - m_flAttackPower;

	for (size_t i = 0; i < TANK_SHIELDS; i++)
		m_flShieldStrengths[i] = Approach(m_flMaxShieldStrength, m_flShieldStrengths[i], ShieldRechargeRate());

	m_flHealth = Approach(m_flTotalHealth, m_flHealth, HealthRechargeRate());
}

void CDigitank::SetDesiredMove()
{
	float flMoveLength = (m_vecPreviewMove - GetOrigin()).Length();

	if (flMoveLength > m_flTotalPower)
		return;

	m_vecDesiredMove = m_vecPreviewMove;

	m_flMovementPower = flMoveLength;
	m_flAttackPower = RemapVal(flMoveLength, 0, m_flTotalPower, m_flAttackPower/(m_flAttackPower+m_flDefensePower)*m_flTotalPower, 0);
	m_flDefensePower = m_flTotalPower - m_flMovementPower - m_flAttackPower;

	m_bDesiredMove = true;
}

void CDigitank::CancelDesiredMove()
{
	m_bDesiredMove = false;

	m_flMovementPower = 0;
	m_flAttackPower = m_flAttackPower/(m_flAttackPower+m_flDefensePower)*m_flTotalPower;
	m_flDefensePower = m_flTotalPower - m_flAttackPower;
}

void CDigitank::Move()
{
	if (!m_bDesiredMove)
		return;

	m_bDesiredMove = false;
	SetOrigin(m_vecDesiredMove);
}

void CDigitank::Fire()
{
	if (m_hTarget == NULL)
		return;

	float flDistanceSqr = (m_hTarget->GetOrigin() - GetOrigin()).LengthSqr();
	if (flDistanceSqr > 50*50)
		return;

	bool bHit = false;
	float flDistance = sqrt(flDistanceSqr);

	if (flDistanceSqr < 30*30)
		bHit = true;
	else
		bHit = (rand()%100) > RemapVal(flDistance, 30, 50, 0, 100);

	if (bHit)
		m_hTarget->TakeDamage(this, m_flAttackPower);
}

void CDigitank::TakeDamage(CBaseEntity* pAttacker, float flDamage)
{
	Vector vecAttackDirection = (pAttacker->GetOrigin() - GetOrigin()).Normalized();

	Vector vecForward, vecRight;
	AngleVectors(GetAngles(), &vecForward, &vecRight, NULL);

	float flForwardDot = vecForward.Dot(vecAttackDirection);
	float flRightDot = vecRight.Dot(vecAttackDirection);

	float flDamageBlocked;

	float* pflShield;
	if (flForwardDot > 0.5f)
		pflShield = &m_flFrontShieldStrength;
	else if (flForwardDot < -0.5f)
		pflShield = &m_flBackShieldStrength;
	else if (flRightDot > 0.5f)
		pflShield = &m_flRightShieldStrength;
	else
		pflShield = &m_flLeftShieldStrength;

	flDamageBlocked = (*pflShield) * GetDefensePower();

	if (flDamage - flDamageBlocked <= 0)
	{
		*pflShield -= flDamage;
		return;
	}

	flDamage -= flDamageBlocked;

	*pflShield = 0;

	BaseClass::TakeDamage(pAttacker, flDamage);
}
