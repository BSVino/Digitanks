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
}

float CDigitank::GetAttackPower()
{
	float flPreviewLength = (m_vecPreviewMove - GetOrigin()).LengthSqr();
	if (flPreviewLength > m_flTotalPower * m_flTotalPower)
		return m_flAttackPower/m_flTotalPower;

	flPreviewLength = sqrt(flPreviewLength);

	return RemapVal(flPreviewLength, 0, m_flTotalPower, m_flAttackPower/(m_flAttackPower+m_flDefensePower), 0);
}

float CDigitank::GetDefensePower()
{
	float flPreviewLength = (m_vecPreviewMove - GetOrigin()).LengthSqr();
	if (flPreviewLength > m_flTotalPower * m_flTotalPower)
		return m_flDefensePower/m_flTotalPower;

	flPreviewLength = sqrt(flPreviewLength);

	return RemapVal(flPreviewLength, 0, m_flTotalPower, m_flDefensePower/(m_flAttackPower+m_flDefensePower), 0);
}

float CDigitank::GetMovementPower()
{
	float flPreviewLength = (m_vecPreviewMove - GetOrigin()).LengthSqr();
	if (flPreviewLength > m_flTotalPower * m_flTotalPower)
		return m_flMovementPower/m_flTotalPower;

	return sqrt(flPreviewLength)/m_flTotalPower;
}

void CDigitank::StartTurn()
{
	m_flMovementPower = 0;
	m_flAttackPower = m_flAttackPower/(m_flAttackPower+m_flDefensePower)*m_flTotalPower;
	m_flDefensePower = m_flTotalPower - m_flAttackPower;
}

void CDigitank::SetDesiredMove()
{
	m_vecDesiredMove = m_vecPreviewMove;

	float flMoveLength = (m_vecDesiredMove - GetOrigin()).Length();

	if (flMoveLength > m_flTotalPower)
		return;

	m_flMovementPower = flMoveLength;
	m_flAttackPower = RemapVal(flMoveLength, 0, m_flTotalPower, m_flAttackPower/(m_flAttackPower+m_flDefensePower)*m_flTotalPower, 0);
	m_flDefensePower = m_flTotalPower - m_flMovementPower - m_flAttackPower;

	m_bDesiredMove = true;
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
