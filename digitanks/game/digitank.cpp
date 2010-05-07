#include "digitank.h"

#include "maths.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "digitanksgame.h"
#include "ui/digitankswindow.h"
#include "powerup.h"

CDigitank::CDigitank()
{
	m_flBasePower = 10;
	m_flAttackPower = 4.5f;
	m_flDefensePower = 5.5f;
	m_flMovementPower = 0;

	m_flBonusAttackPower = m_flBonusDefensePower = m_flBonusMovementPower = 0;

	m_iBonusPoints = 0;

	m_flPreviewTurn = 0;

	m_bPreviewAim = false;

	m_bDesiredMove = false;
	m_bDesiredTurn = false;
	m_bDesiredAim = false;

	m_bTakeDamage = true;
	m_flTotalHealth = 10;
	m_flHealth = 10;

	m_flMaxShieldStrength = m_flFrontShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flBackShieldStrength = 15;

	m_pTeam = NULL;
}

float CDigitank::GetBaseAttackPower(bool bPreview)
{
	float flMovementLength;

	if (bPreview)
	{
		if (!HasDesiredAim() && !IsPreviewAimValid())
			return 0;

		flMovementLength = GetPreviewMoveTurnPower();

		if (flMovementLength > m_flBasePower)
			return m_flAttackPower;

		return RemapVal(flMovementLength, 0, m_flBasePower, m_flAttackPower/(m_flAttackPower+m_flDefensePower)*m_flBasePower, 0);
	}

	return m_flAttackPower;
}

float CDigitank::GetBaseDefensePower(bool bPreview)
{
	float flMovementLength;

	if (bPreview)
	{
		flMovementLength = GetPreviewMoveTurnPower();

		bool bPreviewAimValid = IsPreviewAimValid();

		if (flMovementLength > m_flBasePower)
		{
			if (!HasDesiredAim() && !bPreviewAimValid)
				return m_flBasePower;
			else
				return m_flDefensePower;
		}

		if (!HasDesiredAim() && !bPreviewAimValid)
			return m_flBasePower - flMovementLength;

		return RemapVal(flMovementLength, 0, m_flBasePower, m_flDefensePower/(m_flAttackPower+m_flDefensePower)*m_flBasePower, 0);
	}

	return m_flDefensePower;
}

float CDigitank::GetBaseMovementPower(bool bPreview)
{
	float flMovementLength;

	if (bPreview)
	{
		flMovementLength = GetPreviewMoveTurnPower();

		if (flMovementLength > m_flBasePower)
			return m_flMovementPower;

		return flMovementLength;
	}

	return m_flMovementPower;
}

float CDigitank::GetAttackPower(bool bPreview)
{
	return GetBaseAttackPower(bPreview)+m_flBonusAttackPower;
}

float CDigitank::GetDefensePower(bool bPreview)
{
	return GetBaseDefensePower(bPreview)+m_flBonusDefensePower;
}

float CDigitank::GetMovementPower(bool bPreview)
{
	return GetBaseMovementPower(bPreview)+m_flBonusMovementPower;
}

float CDigitank::GetTotalAttackPower()
{
	return m_flBasePower + m_flBonusAttackPower;
}

float CDigitank::GetTotalDefensePower()
{
	return m_flBasePower + m_flBonusDefensePower;
}

float CDigitank::GetTotalMovementPower()
{
	return m_flBasePower + m_flBonusMovementPower;
}

void CDigitank::SetAttackPower(float flAttackPower)
{
	if (flAttackPower > m_flBasePower - m_flMovementPower)
		return;

	m_flAttackPower = flAttackPower;
	m_flDefensePower = m_flBasePower - m_flMovementPower - m_flAttackPower;
}

float CDigitank::GetPreviewMoveTurnPower()
{
	float flPower = GetPreviewBaseMovePower() + GetPreviewBaseTurnPower() - m_flBonusMovementPower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewMovePower()
{
	float flPower = GetPreviewBaseMovePower() - m_flBonusMovementPower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewTurnPower()
{
	float flPower = GetPreviewBaseTurnPower() - m_flBonusMovementPower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewBaseMovePower()
{
	if (HasDesiredMove())
		return (m_vecDesiredMove - GetOrigin()).Length();

	return (m_vecPreviewMove - GetOrigin()).Length();
}

float CDigitank::GetPreviewBaseTurnPower()
{
	if (HasDesiredTurn())
		return fabs(AngleDifference(m_flDesiredTurn, GetAngles().y)/TurnPerPower());

	return fabs(AngleDifference(m_flPreviewTurn, GetAngles().y)/TurnPerPower());
}

void CDigitank::CalculateAttackDefense()
{
	m_flAttackPower = RemapVal(m_flMovementPower, 0, m_flBasePower, m_flAttackPower/(m_flAttackPower+m_flDefensePower)*m_flBasePower, 0);
	m_flDefensePower = m_flBasePower - m_flMovementPower - m_flAttackPower;
}

float CDigitank::GetFrontShieldStrength()
{
	return m_flFrontShieldStrength/m_flMaxShieldStrength * GetDefenseScale(true);
}

float CDigitank::GetLeftShieldStrength()
{
	return m_flLeftShieldStrength/m_flMaxShieldStrength * GetDefenseScale(true);
}

float CDigitank::GetRightShieldStrength()
{
	return m_flRightShieldStrength/m_flMaxShieldStrength * GetDefenseScale(true);
}

float CDigitank::GetRearShieldStrength()
{
	return m_flBackShieldStrength/m_flMaxShieldStrength * GetDefenseScale(true);
}

float* CDigitank::GetShieldForAttackDirection(Vector vecAttack)
{
	Vector vecForward, vecRight;
	AngleVectors(GetAngles(), &vecForward, &vecRight, NULL);

	float flForwardDot = vecForward.Dot(vecAttack);
	float flRightDot = vecRight.Dot(vecAttack);

	if (flForwardDot > 0.5f)
		return &m_flFrontShieldStrength;
	else if (flForwardDot < -0.5f)
		return &m_flBackShieldStrength;
	else if (flRightDot < -0.5f)
		return &m_flRightShieldStrength;
	else
		return &m_flLeftShieldStrength;
}

void CDigitank::StartTurn()
{
	m_flMovementPower = 0;
	CalculateAttackDefense();

	for (size_t i = 0; i < TANK_SHIELDS; i++)
		m_flShieldStrengths[i] = Approach(m_flMaxShieldStrength, m_flShieldStrengths[i], ShieldRechargeRate());

	m_flHealth = Approach(m_flTotalHealth, m_flHealth, HealthRechargeRate());

	m_vecPreviewMove = GetOrigin();
	m_flPreviewTurn = GetAngles().y;

	m_bDesiredMove = false;
	m_bDesiredTurn = false;
	m_bDesiredAim = false;
}

void CDigitank::ClearPreviewMove()
{
	m_vecPreviewMove = GetOrigin();
}

void CDigitank::ClearPreviewTurn()
{
	m_flPreviewTurn = GetAngles().y;
}

void CDigitank::SetPreviewAim(Vector vecPreviewAim)
{
	m_bPreviewAim = true;
	m_vecPreviewAim = vecPreviewAim;
}

void CDigitank::ClearPreviewAim()
{
	m_bPreviewAim = false;
	m_vecPreviewAim = GetOrigin();
}

bool CDigitank::IsPreviewAimValid()
{
	if (!m_bPreviewAim)
		return false;

	Vector vecOrigin = GetOrigin();
	if (HasDesiredMove())
		vecOrigin = GetDesiredMove();

	return (GetPreviewAim() - vecOrigin).LengthSqr() < GetMaxRange()*GetMaxRange();
}

void CDigitank::SetDesiredMove()
{
	float flMovePower = GetPreviewMovePower();

	if (flMovePower > m_flBasePower)
		return;

	m_vecDesiredMove = m_vecPreviewMove;

	m_flMovementPower = flMovePower;
	CalculateAttackDefense();

	m_bDesiredMove = true;
}

void CDigitank::CancelDesiredMove()
{
	m_bDesiredMove = false;

	m_flMovementPower = GetPreviewTurnPower();
	CalculateAttackDefense();

	ClearPreviewMove();
}

Vector CDigitank::GetDesiredMove()
{
	if (!HasDesiredMove())
		return GetOrigin();

	return m_vecDesiredMove;
}

void CDigitank::SetDesiredTurn()
{
	float flMovePower = GetPreviewMoveTurnPower();

	if (flMovePower > m_flBasePower)
		return;

	m_flDesiredTurn = m_flPreviewTurn;

	m_flMovementPower = flMovePower;
	CalculateAttackDefense();

	m_bDesiredTurn = true;
}

void CDigitank::CancelDesiredTurn()
{
	m_bDesiredTurn = false;

	if (HasDesiredMove())
		m_flMovementPower = (GetOrigin() - GetDesiredMove()).Length();
	else
		m_flMovementPower = 0;

	CalculateAttackDefense();

	ClearPreviewTurn();
}

float CDigitank::GetDesiredTurn()
{
	if (!HasDesiredTurn())
		return GetAngles().y;

	return m_flDesiredTurn;
}

void CDigitank::SetDesiredAim()
{
	m_bDesiredAim = false;

	Vector vecOrigin = GetOrigin();
	if (HasDesiredMove())
		vecOrigin = GetDesiredMove();

	if ((GetPreviewAim() - vecOrigin).Length() > GetMaxRange())
		return;

	m_vecDesiredAim = m_vecPreviewAim;

	m_bDesiredAim = true;
}

void CDigitank::CancelDesiredAim()
{
	m_bDesiredAim = false;

	ClearPreviewAim();
}

Vector CDigitank::GetDesiredAim()
{
	if (!HasDesiredAim())
		return GetOrigin();

	return m_vecDesiredAim;
}

void CDigitank::Move()
{
	if (m_bDesiredMove)
	{
		m_bDesiredMove = false;
		SetOrigin(m_vecDesiredMove);
	}

	if (m_bDesiredTurn)
	{
		m_bDesiredTurn = false;
		SetAngles(EAngle(0, m_flDesiredTurn, 0));
	}

	if (!m_bDesiredAim)
	{
		m_flAttackPower = 0;
		CalculateAttackDefense();
	}

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i));

		if (!pEntity)
			continue;

		CPowerup* pPowerup = dynamic_cast<CPowerup*>(pEntity);

		if (!pPowerup)
			continue;

		if ((pPowerup->GetOrigin() - GetOrigin()).LengthSqr() < 3*3)
		{
			pPowerup->Delete();
			GiveBonusPoints(1);
		}
	}
}

void CDigitank::Fire()
{
	if (!HasDesiredAim())
		return;

	float flDistanceSqr = (GetDesiredAim() - GetOrigin()).LengthSqr();
	if (flDistanceSqr > GetMaxRange()*GetMaxRange())
		return;

	float flDistance = sqrt(flDistanceSqr);
	float flGravity = -flDistance*2;

	Vector vecLandingSpot = GetDesiredAim();

	if (flDistance > GetMinRange())
	{
		float flFactor = RemapVal(flDistance, GetMinRange(), GetMaxRange(), 0, TANK_MAX_RANGE_RADIUS);
		float x = RemapVal((float)(rand()%1000), 0, 1000, -flFactor, flFactor);
		float z = RemapVal((float)(rand()%1000), 0, 1000, -flFactor, flFactor);
		vecLandingSpot += Vector(x, 0, z);
	}

	Vector vecForce = vecLandingSpot - GetOrigin();
	vecForce.y = -flGravity * 0.45f;	// Not quite sure how this works, but it does

	CProjectile* pProjectile = new CProjectile(this, GetAttackPower(), vecForce);
	pProjectile->SetGravity(Vector(0, flGravity, 0));
	DigitanksGame()->AddProjectileToWaitFor();
}


void CDigitank::TakeDamage(CBaseEntity* pAttacker, float flDamage)
{
	Vector vecAttackDirection = (pAttacker->GetOrigin() - GetOrigin()).Normalized();

	float* pflShield = GetShieldForAttackDirection(vecAttackDirection);

	float flDamageBlocked = (*pflShield) * GetDefenseScale();

	if (flDamage - flDamageBlocked <= 0)
	{
		*pflShield -= flDamage;
		return;
	}

	flDamage -= flDamageBlocked;

	*pflShield = 0;

	BaseClass::TakeDamage(pAttacker, flDamage);
}

void CDigitank::Render()
{
	if (HasDesiredMove() || HasDesiredTurn())
	{
		EAngle angTurn = EAngle(0, GetDesiredTurn(), 0);

		if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN && GetPreviewMoveTurnPower() <= GetTotalMovementPower())
			angTurn = EAngle(0, GetPreviewTurn(), 0);

		CDigitanksWindow::Get()->RenderTank(this, GetDesiredMove(), angTurn, GetTeam()->GetColor());

		Color clrTeam = GetTeam()->GetColor();
		clrTeam.SetAlpha(50);
		CDigitanksWindow::Get()->RenderTank(this, GetOrigin(), GetAngles(), clrTeam);
	}
	else
	{
		EAngle angTurn = GetAngles();

		if (this == DigitanksGame()->GetCurrentTank())
		{
			if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN && GetPreviewTurnPower() <= GetTotalMovementPower())
				angTurn = EAngle(0, GetPreviewTurn(), 0);
		}

		CDigitanksWindow::Get()->RenderTank(this, GetOrigin(), angTurn, GetTeam()->GetColor());
	}
}

void CDigitank::GiveBonusPoints(size_t i)
{
	m_iBonusPoints += i;
}

void CDigitank::PromoteAttack()
{
	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusAttackPower++;
}

void CDigitank::PromoteDefense()
{
	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusDefensePower++;
}

void CDigitank::PromoteMovement()
{
	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusMovementPower++;
}

CProjectile::CProjectile(CDigitank* pOwner, float flDamage, Vector vecForce)
{
	m_hOwner = pOwner;
	m_flDamage = flDamage;
	SetVelocity(vecForce);
	SetOrigin(pOwner->GetOrigin());
	SetSimulated(true);
}

void CProjectile::Render()
{
	glPushMatrix();
	glTranslatef(GetOrigin().x, GetOrigin().y, GetOrigin().z);
	glutSolidSphere(0.5f, 4, 4);
	glPopMatrix();
}

void CProjectile::TouchedGround()
{
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i));

		if (pEntity == this)
			continue;

		if (pEntity == m_hOwner)
			continue;

		if ((pEntity->GetOrigin() - GetOrigin()).LengthSqr() < 4*4)
			pEntity->TakeDamage(m_hOwner, m_flDamage);
	}

	Delete();
}
