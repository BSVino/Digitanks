#include "digitank.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <maths.h>
#include <models/models.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <renderer/dissolver.h>

#include "digitanksgame.h"
#include "ui/digitankswindow.h"
#include "powerup.h"
#include "ui/debugdraw.h"
#include "camera.h"

REGISTER_ENTITY(CDigitank);

size_t CDigitank::s_iAimBeam = 0;

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
	m_bSelectedMove = false;

	m_flStartedMove = 0;

	m_bTakeDamage = true;
	m_flTotalHealth = 10;
	m_flHealth = 10;

	m_flMaxShieldStrength = m_flFrontShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flBackShieldStrength = 15;

	m_flFireProjectileTime = 0;

	m_pTeam = NULL;

	SetCollisionGroup(CG_TANK);

	SetModel(L"models/digitanks/digitank-body.obj");
	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-turret.obj");
	m_iShieldModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-shield.obj");
}

void CDigitank::Precache()
{
	PrecacheParticleSystem(L"tank-fire");
	PrecacheParticleSystem(L"promotion");
	PrecacheModel(L"models/digitanks/digitank-body.obj", false);
	PrecacheModel(L"models/digitanks/digitank-turret.obj");
	PrecacheModel(L"models/digitanks/digitank-shield.obj");
	PrecacheSound("sound/tank-fire.wav");
	PrecacheSound("sound/shield-damage.wav");
	PrecacheSound("sound/tank-damage.wav");
	PrecacheSound("sound/tank-active.wav");
	PrecacheSound("sound/tank-active2.wav");
	PrecacheSound("sound/tank-move.wav");
	PrecacheSound("sound/tank-aim.wav");
	PrecacheSound("sound/tank-promoted.wav");

	s_iAimBeam = CRenderer::LoadTextureIntoGL(L"textures/beam-pulse.png");
}

float CDigitank::GetBaseAttackPower(bool bPreview)
{
	float flMovementLength;

	if (DigitanksGame()->GetCurrentTank() == this && bPreview)
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

	if (DigitanksGame()->GetCurrentTank() == this && bPreview)
	{
		flMovementLength = GetPreviewMoveTurnPower();

		bool bPreviewAimValid = IsPreviewAimValid();

		if (flMovementLength > m_flBasePower)
		{
			if (!HasDesiredAim() && !bPreviewAimValid)
			{
				if (!HasDesiredMove())
					return m_flBasePower;

				float flPower = m_flBasePower - GetPreviewMovePower();
				if (flPower < 0)
					return 0;
				else
					return flPower;
			}
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

	if (DigitanksGame()->GetCurrentTank() == this && bPreview)
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

float CDigitank::GetTotalMovementPower() const
{
	return m_flBasePower + m_flBonusMovementPower;
}

float CDigitank::GetMaxMovementDistance() const
{
	return GetTotalMovementPower() * GetTankSpeed();
}

void CDigitank::SetAttackPower(float flAttackPower)
{
	if (flAttackPower > m_flBasePower - m_flMovementPower)
		return;

	m_flAttackPower = flAttackPower;
	m_flDefensePower = m_flBasePower - m_flMovementPower - m_flAttackPower;

	m_bChoseFirepower = true;
}

float CDigitank::GetPreviewMoveTurnPower()
{
	float flPower = GetPreviewBaseMovePower() + GetPreviewBaseTurnPower() - m_flBonusMovementPower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewMovePower() const
{
	float flPower = GetPreviewBaseMovePower() - m_flBonusMovementPower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewTurnPower() const
{
	float flPower = GetPreviewBaseTurnPower() - m_flBonusMovementPower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewBaseMovePower() const
{
	if (HasDesiredMove())
		return (m_vecDesiredMove - GetOrigin()).Length() / GetTankSpeed();

	return (m_vecPreviewMove - GetOrigin()).Length() / GetTankSpeed();
}

float CDigitank::GetPreviewBaseTurnPower() const
{
	if (HasDesiredTurn())
		return fabs(AngleDifference(m_flDesiredTurn, GetAngles().y)/TurnPerPower());

	return fabs(AngleDifference(m_flPreviewTurn, GetAngles().y)/TurnPerPower());
}

void CDigitank::CalculateAttackDefense()
{
	if (m_flAttackPower + m_flDefensePower == 0)
		m_flAttackPower = 0;
	else
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

	if (flForwardDot > 0.7f)
		return &m_flFrontShieldStrength;
	else if (flForwardDot < -0.7f)
		return &m_flBackShieldStrength;
	else if (flRightDot < -0.7f)
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
	m_bSelectedMove = false;
	m_bChoseFirepower = false;
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

	if ((vecPreviewAim - GetDesiredMove()).LengthSqr() > GetMaxRange()*GetMaxRange())
	{
		vecPreviewAim = GetDesiredMove() + (vecPreviewAim - GetDesiredMove()).Normalized() * GetMaxRange() * 0.99f;
		vecPreviewAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecPreviewAim.x, vecPreviewAim.z);
	}

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

	return (GetPreviewAim() - GetDesiredMove()).LengthSqr() < GetMaxRange()*GetMaxRange();
}

void CDigitank::SetDesiredMove()
{
	m_bSelectedMove = true;

	float flMovePower = GetPreviewMovePower();

	if (flMovePower > m_flBasePower)
		return;

	if (fabs(m_vecPreviewMove.x) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	if (fabs(m_vecPreviewMove.z) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	m_vecPreviousOrigin = GetOrigin();
	m_vecDesiredMove = m_vecPreviewMove;

	m_flMovementPower = flMovePower;
	CalculateAttackDefense();

	m_bDesiredMove = true;

	m_flStartedMove = DigitanksGame()->GetGameTime();

	EmitSound("sound/tank-move.wav");
	SetSoundVolume("sound/tank-move.wav", 0.5f);
}

void CDigitank::CancelDesiredMove()
{
	m_bDesiredMove = false;
	m_bSelectedMove = false;

	m_flMovementPower = GetPreviewTurnPower();
	CalculateAttackDefense();

	ClearPreviewMove();
}

Vector CDigitank::GetDesiredMove() const
{
	float flTransitionTime = GetTransitionTime();
	float flTimeSinceMove = DigitanksGame()->GetGameTime() - m_flStartedMove;
	if (m_flStartedMove && flTimeSinceMove < flTransitionTime)
	{
		float flLerp = SLerp(RemapVal(flTimeSinceMove, 0, flTransitionTime, 0, 1), 0.2f);
		Vector vecNewOrigin = m_vecPreviousOrigin * (1-flLerp) + m_vecDesiredMove * flLerp;
		DigitanksGame()->GetTerrain()->SetPointHeight(vecNewOrigin);
		return vecNewOrigin;
	}

	if (!HasDesiredMove())
		return GetOrigin();

	return m_vecDesiredMove;
}

bool CDigitank::IsMoving()
{
	float flTransitionTime = GetTransitionTime();
	float flTimeSinceMove = DigitanksGame()->GetGameTime() - m_flStartedMove;
	if (m_flStartedMove && flTimeSinceMove < flTransitionTime)
		return true;

	return false;
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
		m_flMovementPower = (GetOrigin() - m_vecDesiredMove).Length();
	else
		m_flMovementPower = 0;

	CalculateAttackDefense();

	ClearPreviewTurn();
}

float CDigitank::GetDesiredTurn() const
{
	if (!HasDesiredTurn())
		return GetAngles().y;

	return m_flDesiredTurn;
}

void CDigitank::SetDesiredAim()
{
	m_bDesiredAim = false;

	if ((GetPreviewAim() - GetDesiredMove()).Length() > GetMaxRange())
		return;

	m_vecDesiredAim = m_vecPreviewAim;

	m_bDesiredAim = true;

	EmitSound("sound/tank-aim.wav");
	SetSoundVolume("sound/tank-aim.wav", 0.5f);
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

void CDigitank::Think()
{
	m_bDisplayAim = false;

	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	bool bShiftDown = CDigitanksWindow::Get()->IsShiftDown();
	bool bAimMode = CDigitanksWindow::Get()->GetControlMode() == MODE_AIM;
	bool bShowThisTank = HasDesiredAim() || (bAimMode && (this == pCurrentTank || bShiftDown));
	if (GetTeam() != DigitanksGame()->GetCurrentTeam())
		bShowThisTank = false;

	if (bShowThisTank)
	{
		Vector vecMouseAim;
		bool bMouseOK = CDigitanksWindow::Get()->GetMouseGridPosition(vecMouseAim);

		Vector vecTankAim;
		if (HasDesiredAim())
			vecTankAim = GetDesiredAim();

		if (bMouseOK && bAimMode)
		{
			if (this == pCurrentTank)
				vecTankAim = vecMouseAim;

			if (this != pCurrentTank && bShiftDown)
				vecTankAim = vecMouseAim;
		}

		if (HasDesiredAim() || bMouseOK)
		{
			if ((vecTankAim - GetDesiredMove()).Length() > GetMaxRange())
			{
				vecTankAim = GetDesiredMove() + (vecTankAim - GetDesiredMove()).Normalized() * GetMaxRange() * 0.99f;
				vecTankAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecTankAim.x, vecTankAim.z);
			}

			float flDistance = (vecTankAim - GetDesiredMove()).Length();

			float flRadius = RemapValClamped(flDistance, GetMinRange(), GetMaxRange(), 2, TANK_MAX_RANGE_RADIUS);
			DigitanksGame()->AddTankAim(vecTankAim, flRadius, this == pCurrentTank && CDigitanksWindow::Get()->GetControlMode() == MODE_AIM);

			m_bDisplayAim = true;
			m_vecDisplayAim = vecTankAim;
			m_flDisplayAimRadius = flRadius;
		}
	}

	if (m_flFireProjectileTime && Game()->GetGameTime() > m_flFireProjectileTime)
	{
		m_flFireProjectileTime = 0;
		FireProjectile();
	}
}

void CDigitank::OnCurrentTank()
{
	if (rand()%2 == 0)
		EmitSound("sound/tank-active.wav");
	else
		EmitSound("sound/tank-active2.wav");
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

	DigitanksGame()->AddProjectileToWaitFor();
	m_flFireProjectileTime = Game()->GetGameTime() + RemapVal((float)(rand()%100), 0, 100, 0, 1);
}

void CDigitank::FireProjectile()
{
	if (!HasDesiredAim())
		return;

	float flDistanceSqr = (GetDesiredAim() - GetOrigin()).LengthSqr();
	if (flDistanceSqr > GetMaxRange()*GetMaxRange())
		return;

	float flDistance = sqrt(flDistanceSqr);
	Vector vecLandingSpot = GetDesiredAim();

	if (flDistance > GetMinRange())
	{
		float flFactor = RemapVal(flDistance, GetMinRange(), GetMaxRange(), 0, TANK_MAX_RANGE_RADIUS);
		float x = RemapVal((float)(rand()%1000), 0, 1000, -flFactor, flFactor);
		float z = RemapVal((float)(rand()%1000), 0, 1000, -flFactor, flFactor);
		vecLandingSpot += Vector(x, 0, z);
	}

	float flGravity = DigitanksGame()->GetGravity();
	float flTime;
	Vector vecForce;
	FindLaunchVelocity(GetOrigin(), vecLandingSpot, flGravity, vecForce, flTime);

	CProjectile* pProjectile = new CProjectile(this, GetAttackPower(), vecForce);
	pProjectile->SetGravity(Vector(0, flGravity, 0));

	EmitSound("sound/tank-fire.wav");

	Vector vecMuzzle = (vecLandingSpot - GetOrigin()).Normalized() * 3 + Vector(0, 3, 0);
	size_t iFire = CParticleSystemLibrary::AddInstance(L"tank-fire", GetOrigin() + vecMuzzle);
	if (iFire != ~0)
		CParticleSystemLibrary::Get()->GetInstance(iFire)->SetInheritedVelocity(vecForce);
}

void CDigitank::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit)
{
	size_t iDifficulty = DigitanksGame()->GetDifficulty();

	if (iDifficulty == 0)
	{
		if (GetTeam() == DigitanksGame()->GetTeam(0))
			flDamage *= 0.5f;
		else if (dynamic_cast<CDigitank*>(pAttacker) && dynamic_cast<CDigitank*>(pAttacker)->GetTeam() == DigitanksGame()->GetTeam(0))
			flDamage *= 2.0f;
	}

	Vector vecAttackDirection = ((bDirectHit?pAttacker:pInflictor)->GetOrigin() - GetOrigin()).Normalized();

	float* pflShield = GetShieldForAttackDirection(vecAttackDirection);

	float flDamageBlocked = (*pflShield) * GetDefenseScale();

	if (flDamage - flDamageBlocked <= 0)
	{
		*pflShield -= flDamage;

		EmitSound("sound/shield-damage.wav");
		SetSoundVolume("sound/shield-damage.wav", RemapValClamped(flDamage, 0, 5, 0, 0.5f));

		DigitanksGame()->OnTakeShieldDamage(this, pAttacker, pInflictor, flDamage, bDirectHit);

		return;
	}

	flDamage -= flDamageBlocked;

	EmitSound("sound/tank-damage.wav");
	SetSoundVolume("sound/tank-damage.wav", RemapValClamped(flDamage, 0, 5, 0, 1));

	if (*pflShield > 1.0f)
	{
		EmitSound("sound/shield-damage.wav");
		SetSoundVolume("sound/shield-damage.wav", RemapValClamped(*pflShield, 0, 5, 0, 1));
	}

	*pflShield = 0;

	DigitanksGame()->OnTakeShieldDamage(this, pAttacker, pInflictor, flDamageBlocked, bDirectHit);

	BaseClass::TakeDamage(pAttacker, pInflictor, flDamage, bDirectHit);
}

void CDigitank::OnKilled(CBaseEntity* pKilledBy)
{
	CModelDissolver::AddModel(this);

	if (dynamic_cast<CDigitank*>(pKilledBy))
		dynamic_cast<CDigitank*>(pKilledBy)->GiveBonusPoints(1);
}

Vector CDigitank::GetRenderOrigin() const
{
	return GetDesiredMove() + Vector(0, 1, 0);
}

EAngle CDigitank::GetRenderAngles() const
{
	if (this == DigitanksGame()->GetCurrentTank())
	{
		if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN && GetPreviewTurnPower() <= GetTotalMovementPower())
			return EAngle(0, GetPreviewTurn(), 0);
	}

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
	{
		if (CDigitanksWindow::Get()->IsShiftDown() && GetTeam() == DigitanksGame()->GetCurrentTank()->GetTeam())
		{
			Vector vecLookAt;
			bool bMouseOK = CDigitanksWindow::Get()->GetMouseGridPosition(vecLookAt);
			bool bNoTurn = bMouseOK && (vecLookAt - DigitanksGame()->GetCurrentTank()->GetDesiredMove()).LengthSqr() < 3*3;

			if (!bNoTurn && bMouseOK)
			{
				Vector vecDirection = (vecLookAt - GetDesiredMove()).Normalized();
				float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

				float flTankTurn = AngleDifference(flYaw, GetAngles().y);
				if (GetPreviewMovePower() + fabs(flTankTurn)/TurnPerPower() > GetTotalMovementPower())
					flTankTurn = (flTankTurn / fabs(flTankTurn)) * (GetTotalMovementPower() - GetPreviewMovePower()) * TurnPerPower() * 0.95f;

				return EAngle(0, GetAngles().y + flTankTurn, 0);
			}
		}
	}

	return EAngle(0, GetDesiredTurn(), 0);
}

void CDigitank::ModifyContext(CRenderingContext* pContext)
{
	pContext->SetColorSwap(GetTeam()->GetColor());
}

void CDigitank::OnRender()
{
	RenderTurret();

	RenderShield(GetFrontShieldStrength(), 0);
	RenderShield(GetLeftShieldStrength(), 90);
	RenderShield(GetRearShieldStrength(), 180);
	RenderShield(GetRightShieldStrength(), 270);
}

void CDigitank::RenderTurret(float flAlpha)
{
	CRenderingContext r(Game()->GetRenderer());
	r.SetAlpha(flAlpha);
	r.SetBlend(BLEND_ALPHA);
	r.Translate(Vector(-0.527677f, 0.810368f, 0));

	if ((this == DigitanksGame()->GetCurrentTank() && CDigitanksWindow::Get()->GetControlMode() == MODE_AIM) || HasDesiredAim())
	{
		Vector vecAimTarget;
		if (this == DigitanksGame()->GetCurrentTank() && CDigitanksWindow::Get()->GetControlMode() == MODE_AIM)
			vecAimTarget = GetPreviewAim();
		else
			vecAimTarget = GetDesiredAim();
		Vector vecTarget = (vecAimTarget - GetRenderOrigin()).Normalized();
		float flAngle = atan2(vecTarget.z, vecTarget.x) * 180/M_PI - GetRenderAngles().y;

		r.Rotate(-flAngle, Vector(0, 1, 0));
	}

	float flScale = RemapVal(GetAttackPower(true), 0, 10, 1, 1.5f);
	r.Scale(flScale, flScale, flScale);

	r.RenderModel(m_iTurretModel);
}

void CDigitank::RenderShield(float flAlpha, float flAngle)
{
	CRenderingContext r(Game()->GetRenderer());
	r.SetAlpha(flAlpha);
	r.Rotate(flAngle, Vector(0, 1, 0));
	r.SetBlend(BLEND_ADDITIVE);
	r.RenderModel(m_iShieldModel);
}

void CDigitank::PostRender()
{
	if (HasDesiredMove() || HasDesiredTurn())
	{
		CRenderingContext r(Game()->GetRenderer());
		r.Translate(GetOrigin() + Vector(0, 1, 0));
		r.Rotate(-GetAngles().y, Vector(0, 1, 0));
		r.SetAlpha(50.0f/255);
		r.SetBlend(BLEND_ALPHA);
		r.SetColorSwap(GetTeam()->GetColor());
		r.RenderModel(GetModel());

		RenderTurret(50.0f/255);
	}

	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
	{
		EAngle angTurn = EAngle(0, GetDesiredTurn(), 0);
		if (CDigitanksWindow::Get()->IsShiftDown() && GetTeam() == pCurrentTank->GetTeam())
		{
			Vector vecLookAt;
			bool bMouseOK = CDigitanksWindow::Get()->GetMouseGridPosition(vecLookAt);
			bool bNoTurn = bMouseOK && (vecLookAt - DigitanksGame()->GetCurrentTank()->GetDesiredMove()).LengthSqr() < 3*3;

			if (!bNoTurn && bMouseOK)
			{
				Vector vecDirection = (vecLookAt - GetDesiredMove()).Normalized();
				float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

				float flTankTurn = AngleDifference(flYaw, GetAngles().y);
				if (GetPreviewMovePower() + fabs(flTankTurn)/TurnPerPower() > GetTotalMovementPower())
					flTankTurn = (flTankTurn / fabs(flTankTurn)) * (GetTotalMovementPower() - GetPreviewMovePower()) * TurnPerPower() * 0.95f;

				angTurn = EAngle(0, GetAngles().y + flTankTurn, 0);
			}

			CRenderingContext r(Game()->GetRenderer());
			r.Translate(GetRenderOrigin());
			r.Rotate(-GetAngles().y, Vector(0, 1, 0));
			r.SetAlpha(50.0f/255);
			r.SetBlend(BLEND_ALPHA);
			r.SetColorSwap(GetTeam()->GetColor());
			r.RenderModel(GetModel());

			RenderTurret(50.0f/255);
		}
	}

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
	{
		if (pCurrentTank->GetPreviewMovePower() <= pCurrentTank->GetBasePower())
		{
			if (this == pCurrentTank)
			{
				CRenderingContext r(Game()->GetRenderer());
				r.Translate(GetPreviewMove() + Vector(0, 1, 0));
				r.Rotate(-GetAngles().y, Vector(0, 1, 0));
				r.SetAlpha(50.0f/255);
				r.SetBlend(BLEND_ALPHA);
				r.SetColorSwap(GetTeam()->GetColor());
				r.RenderModel(GetModel());

				RenderTurret(50.0f/255);
			}
			else if (CDigitanksWindow::Get()->IsShiftDown() && GetTeam() == pCurrentTank->GetTeam())
			{
				Vector vecTankMove = pCurrentTank->GetPreviewMove() - pCurrentTank->GetOrigin();
				if (vecTankMove.Length() > GetMaxMovementDistance())
					vecTankMove = vecTankMove.Normalized() * GetTotalMovementPower() * 0.95f;

				Vector vecNewPosition = GetOrigin() + vecTankMove;
				vecNewPosition.y = DigitanksGame()->GetTerrain()->GetHeight(vecNewPosition.x, vecNewPosition.z);

				CRenderingContext r(Game()->GetRenderer());
				r.Translate(vecNewPosition + Vector(0, 1, 0));
				r.Rotate(-GetAngles().y, Vector(0, 1, 0));
				r.SetAlpha(50.0f/255);
				r.SetBlend(BLEND_ALPHA);
				r.SetColorSwap(GetTeam()->GetColor());
				r.RenderModel(GetModel());

				RenderTurret(50.0f/255);
			}
		}
	}

	if (m_bDisplayAim)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		int iAlpha = 100;
		if (this == pCurrentTank && CDigitanksWindow::Get()->GetControlMode() == MODE_AIM)
			iAlpha = 255;

		Vector vecTankOrigin = GetDesiredMove();

		float flGravity = DigitanksGame()->GetGravity();
		float flTime;
		Vector vecForce;
		FindLaunchVelocity(vecTankOrigin, m_vecDisplayAim, flGravity, vecForce, flTime);

		CRopeRenderer oRope(Game()->GetRenderer(), s_iAimBeam, vecTankOrigin);
		oRope.SetWidth(0.5f);
		oRope.SetColor(Color(255, 0, 0, iAlpha));
		oRope.SetTextureScale(50);
		oRope.SetTextureOffset(-fmod(Game()->GetGameTime(), 1));

		Vector vecProjectile = vecTankOrigin;
		size_t iLinks = 20;
		for (size_t i = 0; i < iLinks; i++)
		{
			oRope.AddLink(vecProjectile + vecForce*(flTime/iLinks));
			vecProjectile += vecForce*(flTime/iLinks);
			vecForce.y += flGravity*flTime/iLinks;
		}

		oRope.Finish(m_vecDisplayAim);

		glPopAttrib();
	}
}

void CDigitank::GiveBonusPoints(size_t i, bool bPlayEffects)
{
	m_iBonusPoints += i;

	if (bPlayEffects)
	{
		EmitSound("sound/tank-promoted.wav");
		CParticleSystemLibrary::AddInstance(L"promotion", GetOrigin());
	}
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

REGISTER_ENTITY(CProjectile);

CProjectile::CProjectile(CDigitank* pOwner, float flDamage, Vector vecForce)
{
	m_flTimeCreated = DigitanksGame()->GetGameTime();
	m_flTimeExploded = 0;

	m_bFallSoundPlayed = false;

	m_hOwner = pOwner;
	m_flDamage = flDamage;
	SetVelocity(vecForce);
	SetOrigin(pOwner->GetOrigin() + Vector(0, 1, 0));
	SetSimulated(true);
	SetCollisionGroup(CG_POWERUP);

	m_iParticleSystem = CParticleSystemLibrary::AddInstance(L"shell-trail", GetOrigin());
	if (m_iParticleSystem != ~0)
		CParticleSystemLibrary::GetInstance(m_iParticleSystem)->FollowEntity(this);
}

void CProjectile::Precache()
{
	PrecacheParticleSystem(L"shell-trail");
	PrecacheSound("sound/bomb-drop.wav");
	PrecacheSound("sound/explosion.wav");
}

void CProjectile::Think()
{
	if (GetVelocity().y < 10.0f && !m_bFallSoundPlayed && m_flTimeExploded == 0.0f)
	{
		EmitSound("sound/bomb-drop.wav");
		SetSoundVolume("sound/bomb-drop.wav", 0.5f);
		m_bFallSoundPlayed = true;
	}

	if (DigitanksGame()->GetGameTime() - m_flTimeCreated > 5.0f && m_flTimeExploded == 0.0f)
		Delete();

	else if (m_flTimeExploded != 0.0f && DigitanksGame()->GetGameTime() - m_flTimeExploded > 2.0f)
		Delete();
}

void CProjectile::ModifyContext(class CRenderingContext* pContext)
{
	if (m_flTimeExploded > 0.0f)
	{
		pContext->UseFrameBuffer(Game()->GetRenderer()->GetExplosionBuffer()->m_iFB);
	}
}

void CProjectile::OnRender()
{
	if (m_flTimeExploded == 0.0f)
	{
		glColor4ubv(Color(255, 255, 255));
		glutSolidSphere(0.5f, 4, 4);
	}
	else
	{
		float flAlpha = RemapValClamped(DigitanksGame()->GetGameTime()-m_flTimeExploded, 0.2f, 1.2f, 1, 0);
		if (flAlpha > 0)
		{
			glColor4ubv(Color(255, 255, 255, (int)(flAlpha*255)));
			glutSolidSphere(4.0f, 20, 10);
		}
	}
}

void CProjectile::OnDeleted()
{
	if (m_iParticleSystem != ~0)
		CParticleSystemLibrary::StopInstance(m_iParticleSystem);
}

bool CProjectile::ShouldTouch(CBaseEntity* pOther) const
{
	if (m_flTimeExploded != 0)
		return false;

	if (pOther == m_hOwner)
		return false;

	if (pOther->GetCollisionGroup() == CG_TANK)
	{
		CDigitank* pTank = dynamic_cast<CDigitank*>(pOther);
		if (!pTank)	// ?
			return false;

		if (pTank->GetTeam() == m_hOwner->GetTeam())
			return false;

		return true;
	}

	if (pOther->GetCollisionGroup() == CG_TERRAIN)
		return true;

	return false;
}

bool CProjectile::IsTouching(CBaseEntity* pOther, Vector& vecPoint) const
{
	switch (pOther->GetCollisionGroup())
	{
	case CG_TANK:
		vecPoint = GetOrigin();
		if ((pOther->GetOrigin() - GetOrigin()).LengthSqr() < pOther->GetBoundingRadius()*pOther->GetBoundingRadius())
			return true;
		break;

	case CG_TERRAIN:
		return DigitanksGame()->GetTerrain()->Collide(GetLastOrigin(), GetOrigin(), vecPoint);
	}

	return false;
};

void CProjectile::Touching(CBaseEntity* pOther)
{
	pOther->TakeDamage(m_hOwner, this, m_flDamage);

	SetVelocity(Vector());
	SetGravity(Vector());

	if (m_iParticleSystem != ~0)
		CParticleSystemLibrary::StopInstance(m_iParticleSystem);

	DigitanksGame()->Explode(m_hOwner, this, 4, m_flDamage, pOther, m_hOwner->GetTeam());

	m_flTimeExploded = Game()->GetGameTime();

	Game()->GetCamera()->Shake(GetOrigin(), 3);
	StopSound("sound/bomb-drop.wav");
	EmitSound("sound/explosion.wav");
}
