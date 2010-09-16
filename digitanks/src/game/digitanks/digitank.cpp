#include "digitank.h"

#include <sstream>

#include <GL/glew.h>
#include <maths.h>
#include <mtrand.h>
#include <models/models.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>

#include "digitanksgame.h"
#include "camera.h"
#include "ui/digitankswindow.h"
#include "ui/instructor.h"
#include "powerup.h"
#include "ui/debugdraw.h"
#include "ui/hud.h"
#include "structure.h"
#include "supplyline.h"
#include "projectile.h"

size_t CDigitank::s_iAimBeam = 0;
size_t CDigitank::s_iCancelIcon = 0;
size_t CDigitank::s_iMoveIcon = 0;
size_t CDigitank::s_iTurnIcon = 0;
size_t CDigitank::s_iAimIcon = 0;
size_t CDigitank::s_iFireIcon = 0;
size_t CDigitank::s_iPromoteIcon = 0;
size_t CDigitank::s_iPromoteAttackIcon = 0;
size_t CDigitank::s_iPromoteDefenseIcon = 0;
size_t CDigitank::s_iPromoteMoveIcon = 0;
size_t CDigitank::s_iFortifyIcon = 0;
size_t CDigitank::s_iDeployIcon = 0;
size_t CDigitank::s_iMobilizeIcon = 0;

size_t CDigitank::s_iAutoMove = 0;

const char* CDigitank::s_apszTankLines[] =
{
	"^.^",	// TANKLINE_CUTE
	"<3",	// TANKLINE_LOVE
	":D!",	// TANKLINE_HAPPY
	"\\o/",	// TANKLINE_CHEER
	">D",	// TANKLINE_EVIL
	">.<",	// TANKLINE_SQUINT
	"8)",	// TANKLINE_COOL
	"X(",	// TANKLINE_DEAD
	"x.x",	// TANKLINE_DEAD2
	"#(",	// TANKLINE_DEAD3
	":/",	// TANKLINE_FROWN
	":(",	// TANKLINE_SAD
	"zzz",	// TANKLINE_ASLEEP
	"??",	// TANKLINE_CONFUSED
	"...",	// TANKLINE_DOTDOTDOT
	"!?",	// TANKLINE_SURPRISED
	"!!",	// TANKLINE_THRILLED
};

std::map<size_t, std::vector<size_t> > g_aiSpeechLines;

CDigitank::CDigitank()
{
	m_flStartingPower = 10;

	// If I haven't gotten a turn yet, shields up.
	m_flAttackPower = 0;
	m_flDefensePower = 10;
	m_flMovementPower = 0;
	m_flTotalPower = 0;

	m_flBonusAttackPower = m_flBonusDefensePower = m_flBonusMovementPower = 0;

	m_flAttackSplit = 0.5f;

	m_flRangeBonus = 0;

	m_iBonusPoints = 0;

	m_flPreviewTurn = 0;

	m_bPreviewAim = false;

	m_bDesiredMove = false;
	m_bDesiredTurn = false;
	m_bDesiredAim = false;
	m_bSelectedMove = false;

	m_bGoalMovePosition = false;

	m_flStartedMove = 0;
	m_flStartedTurn = 0;

	m_bFortified = false;

	m_flFrontMaxShieldStrength = m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flRearMaxShieldStrength = 15;
	m_flFrontShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flRearShieldStrength = 15;

	m_flFireProjectileTime = 0;

	m_flLastSpeech = 0;
	m_flNextIdle = 10.0f;

	SetCollisionGroup(CG_ENTITY);

	m_iTurretModel = m_iShieldModel = ~0;

	m_iHoverParticles = ~0;

	m_bFortifyPoint = false;

	m_flFortifyTime = 0;
}

CDigitank::~CDigitank()
{
	if (m_iHoverParticles != ~0)
		CParticleSystemLibrary::StopInstance(m_iHoverParticles);
}

void CDigitank::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem(L"tank-fire");
	PrecacheParticleSystem(L"promotion");
	PrecacheParticleSystem(L"tank-hover");
	PrecacheSound("sound/tank-fire.wav");
	PrecacheSound("sound/shield-damage.wav");
	PrecacheSound("sound/tank-damage.wav");
	PrecacheSound("sound/tank-active.wav");
	PrecacheSound("sound/tank-active2.wav");
	PrecacheSound("sound/tank-move.wav");
	PrecacheSound("sound/tank-aim.wav");
	PrecacheSound("sound/tank-promoted.wav");

	s_iAimBeam = CRenderer::LoadTextureIntoGL(L"textures/beam-pulse.png");
	s_iCancelIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-cancel.png");
	s_iMoveIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-move.png");
	s_iTurnIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-turn.png");
	s_iAimIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-aim.png");
	s_iFireIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-fire.png");
	s_iPromoteIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-promote.png");
	s_iPromoteAttackIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-promote-attack.png");
	s_iPromoteDefenseIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-promote-defense.png");
	s_iPromoteMoveIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-promote-move.png");
	s_iFortifyIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-fortify.png");
	s_iDeployIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-deploy.png");
	s_iMobilizeIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-mobilize.png");

	s_iAutoMove = CRenderer::LoadTextureIntoGL(L"textures/auto-move.png");

	g_aiSpeechLines[TANKSPEECH_SELECTED].push_back(TANKLINE_CUTE);
	g_aiSpeechLines[TANKSPEECH_SELECTED].push_back(TANKLINE_HAPPY);
	g_aiSpeechLines[TANKSPEECH_SELECTED].push_back(TANKLINE_COOL);
	g_aiSpeechLines[TANKSPEECH_MOVED].push_back(TANKLINE_CHEER);
	g_aiSpeechLines[TANKSPEECH_MOVED].push_back(TANKLINE_HAPPY);
	g_aiSpeechLines[TANKSPEECH_ATTACK].push_back(TANKLINE_EVIL);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_SQUINT);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_SURPRISED);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_DEAD);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_DEAD2);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_DEAD3);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_FROWN);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_SAD);
	g_aiSpeechLines[TANKSPEECH_KILL].push_back(TANKLINE_COOL);
	g_aiSpeechLines[TANKSPEECH_KILL].push_back(TANKLINE_EVIL);
	g_aiSpeechLines[TANKSPEECH_KILL].push_back(TANKLINE_THRILLED);
	g_aiSpeechLines[TANKSPEECH_MISSED].push_back(TANKLINE_FROWN);
	g_aiSpeechLines[TANKSPEECH_MISSED].push_back(TANKLINE_SAD);
	g_aiSpeechLines[TANKSPEECH_MISSED].push_back(TANKLINE_SURPRISED);
	g_aiSpeechLines[TANKSPEECH_MISSED].push_back(TANKLINE_DOTDOTDOT);
	g_aiSpeechLines[TANKSPEECH_IDLE].push_back(TANKLINE_ASLEEP);
	g_aiSpeechLines[TANKSPEECH_IDLE].push_back(TANKLINE_CONFUSED);
	g_aiSpeechLines[TANKSPEECH_IDLE].push_back(TANKLINE_DOTDOTDOT);
	g_aiSpeechLines[TANKSPEECH_PROMOTED].push_back(TANKLINE_HAPPY);
	g_aiSpeechLines[TANKSPEECH_PROMOTED].push_back(TANKLINE_LOVE);
	g_aiSpeechLines[TANKSPEECH_PROMOTED].push_back(TANKLINE_CUTE);
	g_aiSpeechLines[TANKSPEECH_PROMOTED].push_back(TANKLINE_COOL);
}

float CDigitank::GetBaseAttackPower(bool bPreview)
{
	return m_flAttackPower;
}

float CDigitank::GetBaseDefensePower(bool bPreview)
{
	if (GetDigitanksTeam()->IsCurrentSelection(this) && bPreview)
	{
		float flMovementLength = GetPreviewMoveTurnPower();

		if (flMovementLength > m_flDefensePower + m_flTotalPower)
			return 0;

		// Any power that hasn't been used so far goes to defense.
		return m_flDefensePower + m_flTotalPower - flMovementLength;
	}

	// Any unallocated power will go into defense anyway.
	return m_flDefensePower + m_flTotalPower;
}

float CDigitank::GetBaseMovementPower(bool bPreview)
{
	if (GetDigitanksTeam()->IsCurrentSelection(this) && bPreview)
	{
		float flMovementLength = GetPreviewMoveTurnPower();

		if (flMovementLength > m_flTotalPower)
			return m_flTotalPower + m_flMovementPower;

		return flMovementLength + m_flMovementPower;
	}

	return m_flMovementPower;
}

float CDigitank::GetAttackPower(bool bPreview)
{
	return GetBaseAttackPower(bPreview)+GetBonusAttackPower(bPreview);
}

float CDigitank::GetDefensePower(bool bPreview)
{
	return GetBaseDefensePower(bPreview)+GetBonusDefensePower(bPreview);
}

float CDigitank::GetMovementPower(bool bPreview)
{
	return GetBaseMovementPower(bPreview)+GetBonusMovementPower();
}

float CDigitank::GetTotalAttackPower()
{
	return m_flStartingPower + GetBonusAttackPower();
}

float CDigitank::GetTotalDefensePower()
{
	return m_flStartingPower + GetBonusDefensePower();
}

float CDigitank::GetTotalMovementPower() const
{
	return m_flStartingPower + GetBonusMovementPower();
}

float CDigitank::GetMaxMovementDistance() const
{
	return (GetTotalPower() + GetBonusMovementPower()) * GetTankSpeed();
}

float CDigitank::GetBonusAttackScale(bool bPreview)
{
	return m_flAttackPower/m_flStartingPower;
}

float CDigitank::GetBonusDefenseScale(bool bPreview)
{
	return m_flDefensePower/m_flStartingPower;
}

float CDigitank::GetBonusAttackPower(bool bPreview)
{
	return (m_flBonusAttackPower + GetSupportAttackPowerBonus())*GetBonusAttackScale(bPreview);
}

float CDigitank::GetBonusDefensePower(bool bPreview)
{
	return (m_flBonusDefensePower + GetSupportDefensePowerBonus())*GetBonusDefenseScale(bPreview);
}

float CDigitank::GetSupportAttackPowerBonus()
{
	if (m_hSupplier == NULL)
		return 0;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = (float)m_hSupplier->EnergyBonus();

	return 2 + flBonus;
}

float CDigitank::GetSupportDefensePowerBonus()
{
	if (m_hSupplier == NULL)
		return 0;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = (float)m_hSupplier->EnergyBonus();

	return 2 + flBonus;
}

float CDigitank::GetSupportHealthRechargeBonus() const
{
	if (m_hSupplier == NULL)
		return 0.0f;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = m_hSupplier->RechargeBonus()/5;

	return 0.1f + flBonus;
}

float CDigitank::GetSupportShieldRechargeBonus() const
{
	if (m_hSupplier == NULL)
		return 0.0f;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = m_hSupplier->RechargeBonus();

	return 0.5f + flBonus;
}

void CDigitank::SetAttackPower(float flAttackPower)
{
	if (flAttackPower > 1)
		flAttackPower = 1;
	if (flAttackPower < 0)
		flAttackPower = 0;

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(-1, "SetAttackPower", GetHandle(), flAttackPower);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = flAttackPower;
	SetAttackPower(&p);
}

void CDigitank::SetAttackPower(CNetworkParameters* p)
{
	m_flAttackSplit = p->fl2;

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

bool CDigitank::IsPreviewMoveValid() const
{
	if (GetPreviewMovePower() > GetTotalPower())
		return false;
	return true;
}

float CDigitank::GetFrontShieldStrength()
{
	if (GetFrontShieldMaxStrength() == 0)
		return 0;

	return m_flFrontShieldStrength/GetFrontShieldMaxStrength() * GetDefenseScale(true);
}

float CDigitank::GetLeftShieldStrength()
{
	if (GetLeftShieldMaxStrength() == 0)
		return 0;

	return m_flLeftShieldStrength/GetLeftShieldMaxStrength() * GetDefenseScale(true);
}

float CDigitank::GetRightShieldStrength()
{
	if (GetRightShieldMaxStrength() == 0)
		return 0;

	return m_flRightShieldStrength/GetRightShieldMaxStrength() * GetDefenseScale(true);
}

float CDigitank::GetRearShieldStrength()
{
	if (GetRearShieldMaxStrength() == 0)
		return 0;

	return m_flRearShieldStrength/GetRearShieldMaxStrength() * GetDefenseScale(true);
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
		return &m_flRearShieldStrength;
	else if (flRightDot < -0.7f)
		return &m_flRightShieldStrength;
	else
		return &m_flLeftShieldStrength;
}

void CDigitank::StartTurn()
{
	BaseClass::StartTurn();

	m_hSupplier = CSupplier::FindClosestSupplier(this);

	if (m_hSupplyLine == NULL && m_hSupplier != NULL)
		m_hSupplyLine = Game()->Create<CSupplyLine>("CSupplyLine");

	if (m_hSupplyLine != NULL && m_hSupplier != NULL)
		m_hSupplyLine->SetEntities(m_hSupplier, this);

	m_flTotalPower = m_flStartingPower;
	m_flMovementPower = m_flAttackPower = m_flDefensePower = 0;

	if (m_bFortified)
	{
		if (m_iFortifyLevel < 5)
			m_iFortifyLevel++;
	}

	for (size_t i = 0; i < TANK_SHIELDS; i++)
	{
		float flShieldStrength = m_flShieldStrengths[i];
		m_flShieldStrengths[i] = Approach(m_flMaxShieldStrengths[i], m_flShieldStrengths[i], ShieldRechargeRate());

		if (flShieldStrength - m_flShieldStrengths[i] < 0)
			DigitanksGame()->OnTakeShieldDamage(this, NULL, NULL, flShieldStrength - m_flShieldStrengths[i], true, false);
	}

	m_vecPreviewMove = GetOrigin();
	m_flPreviewTurn = GetAngles().y;

	m_bDesiredMove = false;
	m_bDesiredTurn = false;
	m_bDesiredAim = false;
	m_bSelectedMove = false;
	m_bChoseFirepower = false;
	m_bFiredWeapon = false;

	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);

	CDigitank* pClosestEnemy = NULL;
	while (true)
	{
		pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(GetOrigin(), pClosestEnemy);

		if (!pClosestEnemy)
			break;

		if (pClosestEnemy->GetTeam() == GetTeam())
			continue;

		if ((pClosestEnemy->GetOrigin() - GetOrigin()).Length() > VisibleRange())
		{
			pClosestEnemy = NULL;
			break;
		}

		break;
	}

	if (HasGoalMovePosition() && !pClosestEnemy)
		MoveTowardsGoalMovePosition();

	if (DigitanksGame()->GetLocalDigitanksTeam() == GetDigitanksTeam())
	{
		size_t iTutorial = CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial();
		if (iTutorial == CInstructor::TUTORIAL_ENTERKEY)
			CDigitanksWindow::Get()->GetInstructor()->NextTutorial();
	}
}

void CDigitank::EndTurn()
{
	BaseClass::EndTurn();

	m_flDefensePower = m_flTotalPower;
	m_flTotalPower = 0;
}

void CDigitank::SetPreviewMove(Vector vecPreviewMove)
{
	if (IsFortified() && !CanMoveFortified())
		return;

	m_vecPreviewMove = vecPreviewMove;
}

void CDigitank::ClearPreviewMove()
{
	m_vecPreviewMove = GetOrigin();
}

void CDigitank::SetPreviewTurn(float flPreviewTurn)
{
	if (IsFortified() && !CanTurnFortified())
		return;

	m_flPreviewTurn = flPreviewTurn;
}

void CDigitank::ClearPreviewTurn()
{
	m_flPreviewTurn = GetAngles().y;
}

void CDigitank::SetPreviewAim(Vector vecPreviewAim)
{
	if (CanFortify() && !IsFortified() && !CanAimMobilized())
		return;

	m_bPreviewAim = true;

	if ((vecPreviewAim - GetDesiredMove()).LengthSqr() > GetMaxRange()*GetMaxRange())
	{
		vecPreviewAim = GetDesiredMove() + (vecPreviewAim - GetDesiredMove()).Normalized() * GetMaxRange() * 0.99f;
		vecPreviewAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecPreviewAim.x, vecPreviewAim.z);
	}

	if ((vecPreviewAim - GetDesiredMove()).LengthSqr() < GetMinRange()*GetMinRange())
	{
		vecPreviewAim = GetDesiredMove() + (vecPreviewAim - GetDesiredMove()).Normalized() * GetMinRange() * 1.01f;
		vecPreviewAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecPreviewAim.x, vecPreviewAim.z);
	}

	if (fabs(AngleDifference(GetDesiredTurn(), VectorAngles((vecPreviewAim-GetDesiredMove()).Normalized()).y)) > FiringCone())
	{
		m_bPreviewAim = false;
		return;
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

	if ((GetPreviewAim() - GetDesiredMove()).LengthSqr() > GetMaxRange()*GetMaxRange())
		return false;

	if ((GetPreviewAim() - GetDesiredMove()).LengthSqr() < GetMinRange()*GetMinRange())
		return false;

	return true;
}

void CDigitank::SetDesiredMove()
{
	m_bSelectedMove = true;

	if (IsFortified() && !CanMoveFortified())
		return;

	if (!IsPreviewMoveValid())
		return;

	if (fabs(m_vecPreviewMove.x) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	if (fabs(m_vecPreviewMove.z) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	Speak(TANKSPEECH_MOVED);
	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(-1, "SetDesiredMove", GetHandle(), m_vecPreviewMove.x, m_vecPreviewMove.y, m_vecPreviewMove.z);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = m_vecPreviewMove.x;
	p.fl3 = m_vecPreviewMove.y;
	p.fl4 = m_vecPreviewMove.z;
	SetDesiredMove(&p);
}

void CDigitank::SetDesiredMove(CNetworkParameters* p)
{
	m_vecPreviewMove = Vector(p->fl2, p->fl3, p->fl4);

	m_vecPreviousOrigin = GetOrigin();
	m_vecDesiredMove = m_vecPreviewMove;

	m_bDesiredMove = true;

	m_flStartedMove = DigitanksGame()->GetGameTime();

	if (GetVisibility() > 0)
	{
		EmitSound("sound/tank-move.wav");
		SetSoundVolume("sound/tank-move.wav", 0.5f);
	}

	if (m_iHoverParticles != ~0)
		CParticleSystemLibrary::StopInstance(m_iHoverParticles);

	if (GetVisibility() > 0)
	{
		m_iHoverParticles = CParticleSystemLibrary::AddInstance(L"tank-hover", GetOrigin());
		if (m_iHoverParticles != ~0)
			CParticleSystemLibrary::GetInstance(m_iHoverParticles)->FollowEntity(this);
	}
}

void CDigitank::CancelDesiredMove()
{
	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(-1, "CancelDesiredMove", GetHandle());

	CancelDesiredMove(NULL);
}

void CDigitank::CancelDesiredMove(CNetworkParameters* p)
{
	m_bDesiredMove = false;
	m_bSelectedMove = false;

	ClearPreviewMove();

	if (m_iHoverParticles != ~0)
	{
		CParticleSystemLibrary::StopInstance(m_iHoverParticles);
		m_iHoverParticles = ~0;
	}

	CancelGoalMovePosition();
}

Vector CDigitank::GetDesiredMove() const
{
	float flTransitionTime = GetTransitionTime();

	if (GetVisibility() == 0)
		flTransitionTime = 0;

	float flTimeSinceMove = DigitanksGame()->GetGameTime() - m_flStartedMove;
	if (m_flStartedMove && flTimeSinceMove < flTransitionTime)
	{
		float flLerp = SLerp(RemapVal(flTimeSinceMove, 0, flTransitionTime, 0, 1), 0.2f);
		Vector vecNewOrigin = m_vecPreviousOrigin * (1-flLerp) + m_vecDesiredMove * flLerp;
		vecNewOrigin.y = FindHoverHeight(vecNewOrigin);
		return vecNewOrigin;
	}

	if (!HasDesiredMove())
		return GetOrigin();

	return m_vecDesiredMove;
}

bool CDigitank::IsMoving()
{
	float flTransitionTime = GetTransitionTime();

	if (GetVisibility() == 0)
		flTransitionTime = 0;

	float flTimeSinceMove = DigitanksGame()->GetGameTime() - m_flStartedMove;
	if (m_flStartedMove && flTimeSinceMove < flTransitionTime)
		return true;

	return false;
}

void CDigitank::SetDesiredTurn()
{
	if (IsFortified() && !CanTurnFortified())
		return;

	float flMovePower = GetPreviewMoveTurnPower();

	if (!IsPreviewMoveValid())
		flMovePower = GetPreviewTurnPower();

	if (flMovePower > m_flTotalPower)
		return;

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(-1, "SetDesiredTurn", GetHandle(), m_flPreviewTurn);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = m_flPreviewTurn;
	SetDesiredTurn(&p);
}

void CDigitank::SetDesiredTurn(CNetworkParameters* p)
{
	m_flPreviewTurn = p->fl2;

	m_flDesiredTurn = m_flPreviewTurn;

	m_flPreviousTurn = GetAngles().y;
	m_flStartedTurn = DigitanksGame()->GetGameTime();

	float flMovePower = GetPreviewMoveTurnPower();

	if (!IsPreviewMoveValid())
		flMovePower = GetPreviewTurnPower();

	m_bDesiredTurn = true;
}

void CDigitank::CancelDesiredTurn()
{
	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(-1, "CancelDesiredTurn", GetHandle());

	CancelDesiredTurn(NULL);
}

void CDigitank::CancelDesiredTurn(CNetworkParameters* p)
{
	m_bDesiredTurn = false;

	ClearPreviewTurn();
}

float CDigitank::GetDesiredTurn() const
{
	float flTransitionTime = GetTransitionTime();

	if (GetVisibility() == 0)
		flTransitionTime = 0;

	float flTimeSinceTurn = DigitanksGame()->GetGameTime() - m_flStartedTurn;
	if (m_flStartedTurn && flTimeSinceTurn < flTransitionTime)
	{
		float flLerp = SLerp(RemapVal(flTimeSinceTurn, 0, flTransitionTime, 0, 1), 0.2f);
		float flNewTurn = m_flPreviousTurn * (1-flLerp) + m_flDesiredTurn * flLerp;
		return flNewTurn;
	}

	if (!HasDesiredTurn())
		return GetAngles().y;

	return m_flDesiredTurn;
}

void CDigitank::SetDesiredAim()
{
	if (CanFortify() && !IsFortified() && !CanAimMobilized())
		return;

	m_bDesiredAim = false;

	if ((GetPreviewAim() - GetDesiredMove()).Length() > GetMaxRange())
		return;

	if ((GetPreviewAim() - GetDesiredMove()).Length() < GetMinRange())
		return;

	if (fabs(AngleDifference(GetDesiredTurn(), VectorAngles((GetPreviewAim()-GetDesiredMove()).Normalized()).y)) > FiringCone())
		return;

	if (m_bFiredWeapon)
		return;

	Speak(TANKSPEECH_ATTACK);
	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(-1, "SetDesiredAim", GetHandle(), m_vecPreviewAim.x, m_vecPreviewAim.y, m_vecPreviewAim.z);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = m_vecPreviewAim.x;
	p.fl3 = m_vecPreviewAim.y;
	p.fl4 = m_vecPreviewAim.z;
	SetDesiredAim(&p);
}

void CDigitank::SetDesiredAim(CNetworkParameters* p)
{
	m_vecPreviewAim = Vector(p->fl2, p->fl3, p->fl4);

	m_vecDesiredAim = m_vecPreviewAim;

	m_bDesiredAim = true;

	if (GetVisibility() > 0)
	{
		EmitSound("sound/tank-aim.wav");
		SetSoundVolume("sound/tank-aim.wav", 0.5f);
	}
}

void CDigitank::CancelDesiredAim()
{
	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(-1, "CancelDesiredAim", GetHandle());

	CancelDesiredAim(NULL);
}

void CDigitank::CancelDesiredAim(CNetworkParameters* p)
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

void CDigitank::SetGoalMovePosition(const Vector& vecPosition)
{
	if (IsFortified() && !CanMoveFortified())
		return;

	if (fabs(vecPosition.x) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	if (fabs(vecPosition.z) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	m_bGoalMovePosition = true;
	m_vecGoalMovePosition = vecPosition;

	MoveTowardsGoalMovePosition();
}

void CDigitank::MoveTowardsGoalMovePosition()
{
	if ((GetOrigin() - GetGoalMovePosition()).LengthSqr() < 1)
	{
		m_bGoalMovePosition = false;
		return;
	}

	Vector vecGoal = GetGoalMovePosition();
	Vector vecOrigin = GetOrigin();
	Vector vecMove = vecGoal - vecOrigin;

	Vector vecNewPosition = GetOrigin() + vecMove;

	do
	{
		vecMove = vecMove * 0.95f;

		if (vecMove.Length() < 1)
			break;

		vecNewPosition = GetOrigin() + vecMove;
		vecNewPosition.y = FindHoverHeight(vecNewPosition);

		SetPreviewMove(vecNewPosition);
	}
	while (!IsPreviewMoveValid());

	SetPreviewMove(vecNewPosition);
	SetDesiredMove();
}

void CDigitank::CancelGoalMovePosition()
{
	m_bGoalMovePosition = false;
}

void CDigitank::Fortify()
{
	if (!CanFortify())
		return;

	if (m_bFortified)
	{
		m_bFortified = false;
		return;
	}

	m_bFortified = true;

	m_iFortifyLevel = 0;
	m_flFortifyTime = Game()->GetGameTime();
}

bool CDigitank::CanAim()
{
	return AllowControlMode(MODE_AIM);
}

bool CDigitank::MovesWith(CDigitank* pOther)
{
	if (!pOther)
		return false;

	if (this == pOther)
		return true;

	// Only same team.
	if (GetTeam() != pOther->GetTeam())
		return false;

	// Not fortified tanks.
	if (IsFortified() || IsFortifying())
		return false;

	// Only closeby tanks.
	if ((GetOrigin() - pOther->GetOrigin()).Length() > 40)
		return false;

	// Only same class tanks.
	if (GetBuildUnit() != pOther->GetBuildUnit())
		return false;

	return true;
}

bool CDigitank::TurnsWith(CDigitank* pOther)
{
	if (!pOther)
		return false;

	if (this == pOther)
		return true;

	// Only same team.
	if (GetTeam() != pOther->GetTeam())
		return false;

	// Only closeby tanks.
	if ((GetOrigin() - pOther->GetOrigin()).Length() > 40)
		return false;

	// Only same class tanks.
	if (GetBuildUnit() != pOther->GetBuildUnit())
		return false;

	return true;
}

bool CDigitank::AimsWith(CDigitank* pOther)
{
	if (!pOther)
		return false;

	if (this == pOther)
		return true;

	// Only same team.
	if (GetTeam() != pOther->GetTeam())
		return false;

	// Only closeby tanks.
	if ((GetOrigin() - pOther->GetOrigin()).Length() > 40)
		return false;

	// Only same class tanks.
	if (GetBuildUnit() != pOther->GetBuildUnit())
		return false;

	if (!CanAim())
		return false;

	if (!IsFortified() && !CanAimMobilized())
		return false;

	// If we're fortified artillery, only aim with other artillery that are pointing in our general direction.
	if (IsArtillery() && IsFortified() && fabs(AngleDifference(pOther->GetAngles().y, GetAngles().y)) > 15)
		return false;

	return true;
}

void CDigitank::Think()
{
	BaseClass::Think();

	m_bDisplayAim = false;

	bool bShiftDown = CDigitanksWindow::Get()->IsShiftDown();
	bool bAimMode = DigitanksGame()->GetControlMode() == MODE_AIM;
	bool bShowThisTank = HasDesiredAim();
	if (bAimMode && (GetDigitanksTeam()->IsCurrentSelection(this) || bShiftDown) && AimsWith(GetDigitanksTeam()->GetCurrentTank()))
		bShowThisTank = true;
	if (GetDigitanksTeam() != DigitanksGame()->GetCurrentTeam())
		bShowThisTank = false;

	if (bShowThisTank)
	{
		Vector vecMouseAim;
		CBaseEntity* pHit = NULL;
		bool bMouseOK = CDigitanksWindow::Get()->GetMouseGridPosition(vecMouseAim, &pHit);

		Vector vecTankAim;
		if (HasDesiredAim())
			vecTankAim = GetDesiredAim();

		if (bMouseOK && bAimMode)
		{
			if (AimsWith(GetDigitanksTeam()->GetCurrentTank()) && (GetDigitanksTeam()->IsCurrentSelection(this) || bShiftDown))
			{
				if (pHit && dynamic_cast<CDigitanksEntity*>(pHit) && pHit->GetTeam() && pHit->GetTeam() != GetTeam())
					vecTankAim = pHit->GetOrigin();
				else
					vecTankAim = vecMouseAim;
			}
		}

		if (HasDesiredAim() || bMouseOK)
		{
			if ((vecTankAim - GetDesiredMove()).Length() > GetMaxRange())
			{
				vecTankAim = GetDesiredMove() + (vecTankAim - GetDesiredMove()).Normalized() * GetMaxRange() * 0.99f;
				vecTankAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecTankAim.x, vecTankAim.z);
			}

			if ((vecTankAim - GetDesiredMove()).Length() < GetMinRange())
			{
				vecTankAim = GetDesiredMove() + (vecTankAim - GetDesiredMove()).Normalized() * GetMinRange() * 1.01f;
				vecTankAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecTankAim.x, vecTankAim.z);
			}

			if (fabs(AngleDifference(GetDesiredTurn(), VectorAngles((vecTankAim-GetDesiredMove()).Normalized()).y)) > FiringCone())
				m_bDisplayAim = false;
			else
			{
				float flDistance = (vecTankAim - GetDesiredMove()).Length();

				float flRadius = RemapValClamped(flDistance, GetEffRange(), GetMaxRange(), 2, TANK_MAX_RANGE_RADIUS);
				DigitanksGame()->AddTankAim(vecTankAim, flRadius, GetDigitanksTeam()->IsCurrentSelection(this) && DigitanksGame()->GetControlMode() == MODE_AIM);

				m_bDisplayAim = true;
				m_vecDisplayAim = vecTankAim;
				m_flDisplayAimRadius = flRadius;
			}
		}
	}

	if (m_flFireProjectileTime && Game()->GetGameTime() > m_flFireProjectileTime)
	{
		m_flFireProjectileTime = 0;
		FireProjectile();
	}

	if (m_iHoverParticles != ~0)
	{
		float flTransitionTime = GetTransitionTime();
		float flTimeSinceMove = DigitanksGame()->GetGameTime() - m_flStartedMove;
		float flTimeSinceTurn = DigitanksGame()->GetGameTime() - m_flStartedTurn;
		if (m_flStartedMove && flTimeSinceMove > flTransitionTime || m_flStartedTurn && flTimeSinceTurn > flTransitionTime)
		{
			CParticleSystemLibrary::StopInstance(m_iHoverParticles);
			m_iHoverParticles = ~0;
		}
	}

	if (IsAlive() && Game()->GetGameTime() > m_flNextIdle)
	{
		// A little bit less often if we're not on the current team.
		if (DigitanksGame()->GetCurrentTeam() == GetTeam() && rand()%2 == 0 || rand()%4 == 0)
			Speak(TANKSPEECH_IDLE);

		m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::OnCurrentSelection()
{
	BaseClass::OnCurrentSelection();

	if (GetDigitanksTeam() == DigitanksGame()->GetLocalDigitanksTeam() && GetVisibility() > 0)
	{
		if (rand()%2 == 0)
			EmitSound("sound/tank-active.wav");
		else
			EmitSound("sound/tank-active2.wav");
	}

	Speak(TANKSPEECH_SELECTED);
	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);

	// So the escape key works.
	if (CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial() != CInstructor::TUTORIAL_THEEND_BASICS)
	{
		CDigitank* pClosestEnemy = NULL;
		while (true)
		{
			pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(GetOrigin(), pClosestEnemy);

			if (!pClosestEnemy)
				break;

			if (pClosestEnemy->GetTeam() == GetTeam())
				continue;

			if ((pClosestEnemy->GetOrigin() - GetOrigin()).Length() > VisibleRange())
			{
				pClosestEnemy = NULL;
				break;
			}

			break;
		}

		if (!HasDesiredMove() && !IsFortified() && !HasDesiredAim())
			DigitanksGame()->SetControlMode(MODE_MOVE);
		else if (!HasDesiredAim() && CanAim() && pClosestEnemy)
			DigitanksGame()->SetControlMode(MODE_AIM);
		else
			DigitanksGame()->SetControlMode(MODE_NONE);
	}
}

bool CDigitank::AllowControlMode(controlmode_t eMode)
{
	if (eMode == MODE_MOVE)
		return true;

	if (eMode == MODE_TURN)
		return true;

	if (eMode == MODE_AIM)
		return true;

	if (eMode == MODE_FIRE)
		return true;

	return BaseClass::AllowControlMode(eMode);
}

void CDigitank::OnControlModeChange(controlmode_t eOldMode, controlmode_t eNewMode)
{
	if (eOldMode == MODE_MOVE)
		ClearPreviewMove();

	if (eOldMode == MODE_TURN)
		ClearPreviewTurn();

	if (eOldMode == MODE_AIM)
		ClearPreviewAim();

	if (eNewMode == MODE_MOVE)
	{
		CancelDesiredMove();
		CancelDesiredTurn();
		CancelDesiredAim();
	}

	if (eNewMode == MODE_TURN)
		CancelDesiredTurn();

	if (eNewMode == MODE_AIM)
	{
		CancelDesiredAim();

		if (IsArtillery())
		{
			Vector vecCenter = DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + AngleVector(GetAngles()) * GetMaxRange()/2);
			DigitanksGame()->GetCamera()->SetTarget(vecCenter);
		}
	}
}

float CDigitank::GetPowerBar1Value()
{
	return GetAttackPower(true) / m_flStartingPower;
}

float CDigitank::GetPowerBar2Value()
{
	return GetDefensePower(true) / m_flStartingPower;
}

float CDigitank::GetPowerBar3Value()
{
	return GetMovementPower(true) / m_flStartingPower;
}

float CDigitank::GetPowerBar1Size()
{
	return GetAttackPower(true) / GetTotalAttackPower();
}

float CDigitank::GetPowerBar2Size()
{
	return GetDefensePower(true) / GetTotalDefensePower();
}

float CDigitank::GetPowerBar3Size()
{
	return GetMovementPower(true) / GetTotalMovementPower();
}

void CDigitank::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	if (eMenuMode == MENUMODE_MAIN)
	{
		if (!IsFortified() && m_flTotalPower > 1)
		{
			pHUD->SetButtonListener(0, CHUD::Move);

			if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_MOVE)
				pHUD->SetButtonColor(0, Color(150, 150, 0));
			else
				pHUD->SetButtonColor(0, Color(100, 100, 100));

			if (DigitanksGame()->GetControlMode() == MODE_MOVE)
				pHUD->SetButtonTexture(0, s_iCancelIcon);
			else
				pHUD->SetButtonTexture(0, s_iMoveIcon);
		}

		if ((!IsFortified() || CanTurnFortified()) && m_flTotalPower > 1)
		{
			pHUD->SetButtonListener(1, CHUD::Turn);

			if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_TURN)
				pHUD->SetButtonColor(1, Color(150, 150, 0));
			else
				pHUD->SetButtonColor(1, Color(100, 100, 100));

			if (DigitanksGame()->GetControlMode() == MODE_TURN)
				pHUD->SetButtonTexture(1, s_iCancelIcon);
			else
				pHUD->SetButtonTexture(1, s_iTurnIcon);
		}

		if (CanFortify())
		{
			if (IsFortified() || IsFortifying())
			{
				pHUD->SetButtonTexture(8, s_iMobilizeIcon);
			}
			else
			{
				if (IsArtillery())
					pHUD->SetButtonTexture(8, s_iDeployIcon);
				else
					pHUD->SetButtonTexture(8, s_iFortifyIcon);
			}
			pHUD->SetButtonListener(8, CHUD::Fortify);
			pHUD->SetButtonColor(8, Color(0, 0, 150));
		}

		if (CanAimMobilized() || IsFortified())
		{
			pHUD->SetButtonListener(2, CHUD::Aim);

			if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_AIM)
				pHUD->SetButtonColor(2, Color(150, 0, 0));
			else
				pHUD->SetButtonColor(2, Color(100, 100, 100));

			if (DigitanksGame()->GetControlMode() == MODE_AIM)
				pHUD->SetButtonTexture(2, s_iCancelIcon);
			else
				pHUD->SetButtonTexture(2, s_iAimIcon);
		}

		pHUD->SetButtonListener(3, CHUD::Fire);

		if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_FIRE)
			pHUD->SetButtonColor(3, Color(150, 0, 150));
		else
			pHUD->SetButtonColor(3, Color(100, 100, 100));

		if (DigitanksGame()->GetControlMode() == MODE_FIRE)
			pHUD->SetButtonTexture(3, s_iCancelIcon);
		else
			pHUD->SetButtonTexture(3, s_iFireIcon);

		if (HasBonusPoints())
		{
			pHUD->SetButtonListener(4, CHUD::Promote);
			pHUD->SetButtonTexture(4, s_iPromoteIcon);
			pHUD->SetButtonColor(4, glgui::g_clrBox);
		}
	}
	else if (eMenuMode == MENUMODE_PROMOTE)
	{
		pHUD->SetButtonListener(0, CHUD::PromoteAttack);
		pHUD->SetButtonTexture(0, s_iPromoteAttackIcon);
		pHUD->SetButtonColor(0, Color(150, 150, 150));

		std::wstringstream s1;
		s1 << "UPGRADE ATTACK ENERGY\n \n"
			<< "This upgrade amplifies your tank's arsenal, increasing the maximum Attack Energy available to your tank past its normal levels. With greater Attack Energy, this tank's shells will deal more damage.\n \n"
			<< "Attack Energy increase: 10%\n";
		pHUD->SetButtonInfo(0, s1.str().c_str());

		if (!IsArtillery())
		{
			pHUD->SetButtonListener(1, CHUD::PromoteDefense);
			pHUD->SetButtonTexture(1, s_iPromoteDefenseIcon);
			pHUD->SetButtonColor(1, Color(150, 150, 150));

			std::wstringstream s;
			s << "UPGRADE DEFENSE ENERGY\n \n"
				<< "This upgrade strengthens your tank's shield generator, increasing the maximum Defense Energy available to your tank past its normal levels. As a result, your tank's shields will take more damage before they fail.\n \n"
				<< "Defense Energy increase: 10%\n";
			pHUD->SetButtonInfo(1, s.str().c_str());
		}

		pHUD->SetButtonListener(2, CHUD::PromoteMovement);
		pHUD->SetButtonTexture(2, s_iPromoteMoveIcon);
		pHUD->SetButtonColor(2, Color(150, 150, 150));

		std::wstringstream s2;
		s2 << "UPGRADE MOVEMENT ENERGY\n \n"
			<< "This upgrade overclocks your tank's engines, increasing the maximum Movement Energy available to your tank past its normal levels. With this you'll spend less energy moving your tank around.\n \n"
			<< "Movement Energy increase: 10%\n";
		pHUD->SetButtonInfo(2, s2.str().c_str());

		pHUD->SetButtonListener(4, CHUD::GoToMain);
		pHUD->SetButtonTexture(4, s_iCancelIcon);
		pHUD->SetButtonColor(4, Color(100, 0, 0));
	}
}

void CDigitank::Move()
{
	float flMovePower = GetPreviewMovePower();

	if (flMovePower > m_flTotalPower)
		return;

	m_flTotalPower -= flMovePower;
	m_flMovementPower += flMovePower;

	if (m_bDesiredMove)
	{
		m_bDesiredMove = false;
		SetOrigin(m_vecDesiredMove);
	}

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i));

		if (!pEntity)
			continue;

		CPowerup* pPowerup = dynamic_cast<CPowerup*>(pEntity);

		if (!pPowerup)
			continue;

		if ((pPowerup->GetOrigin() - GetOrigin()).LengthSqr() < pPowerup->GetBoundingRadius()*pPowerup->GetBoundingRadius())
		{
			pPowerup->Delete();
			GiveBonusPoints(1);

			CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_POWERUP);
		}
	}

	GetDigitanksTeam()->CalculateVisibility();

	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);
}

void CDigitank::Turn()
{
	float flMovePower = GetPreviewTurnPower();

	if (flMovePower > m_flTotalPower)
		return;

	m_flTotalPower -= flMovePower;
	m_flMovementPower += flMovePower;

	if (m_bDesiredTurn)
	{
		m_bDesiredTurn = false;
		SetAngles(EAngle(0, m_flDesiredTurn, 0));
	}

	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);
}

void CDigitank::Fire()
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

	DigitanksGame()->AddProjectileToWaitFor();

	if (CNetwork::IsHost())
		m_flFireProjectileTime = Game()->GetGameTime() + RandomFloat(0, 1);

	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);
}

void CDigitank::FireProjectile()
{
	if (!HasDesiredAim())
		return;

	float flDistanceSqr = (GetDesiredAim() - GetOrigin()).LengthSqr();
	if (flDistanceSqr > GetMaxRange()*GetMaxRange())
		return;

	if (flDistanceSqr < GetMinRange()*GetMinRange())
		return;

	float flDistance = (GetDesiredAim() - GetOrigin()).Length();
	Vector vecLandingSpot = GetDesiredAim();

	float flFactor = 1;
	if (flDistance > GetEffRange())
		flFactor = RemapVal(flDistance, GetEffRange(), GetMaxRange(), 1, TANK_MAX_RANGE_RADIUS);

	float flYaw = RandomFloat(0, 360);
	float flRadius = RandomFloat(1, flFactor);

	// Don't use uniform distribution, I like how it's clustered on the target.
	vecLandingSpot += Vector(flRadius*cos(flYaw), 0, flRadius*sin(flYaw));

	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);

	m_hProjectile = CreateProjectile();

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(-1, "FireProjectile", GetHandle(), m_hProjectile->GetHandle(), vecLandingSpot.x, vecLandingSpot.y, vecLandingSpot.z);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = m_hProjectile->GetHandle();
	p.fl3 = vecLandingSpot.x;
	p.fl4 = vecLandingSpot.y;
	p.fl5 = vecLandingSpot.z;
	FireProjectile(&p);
}

void CDigitank::FireProjectile(CNetworkParameters* p)
{
	m_hProjectile = CEntityHandle<CProjectile>(p->ui2);

	Vector vecLandingSpot = Vector(p->fl3, p->fl4, p->fl5);

	float flGravity = DigitanksGame()->GetGravity();
	float flTime;
	Vector vecForce;
	FindLaunchVelocity(GetOrigin(), vecLandingSpot, flGravity, vecForce, flTime, ProjectileCurve());

	m_hProjectile->SetOwner(this);
	m_hProjectile->SetDamage(GetProjectileDamage());
	m_hProjectile->SetForce(vecForce);
	m_hProjectile->SetGravity(Vector(0, flGravity, 0));
	m_hProjectile->SetLandingSpot(vecLandingSpot);

	if (GetVisibility() > 0)
	{
		EmitSound("sound/tank-fire.wav");

		Vector vecMuzzle = (vecLandingSpot - GetOrigin()).Normalized() * 3 + Vector(0, 3, 0);
		size_t iFire = CParticleSystemLibrary::AddInstance(L"tank-fire", GetOrigin() + vecMuzzle);
		if (iFire != ~0)
			CParticleSystemLibrary::Get()->GetInstance(iFire)->SetInheritedVelocity(vecForce);
	}

	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);
}

CProjectile* CDigitank::CreateProjectile()
{
	return Game()->Create<CShell>("CShell");
}

float CDigitank::GetProjectileDamage()
{
	return GetAttackPower();
}

void CDigitank::ClientUpdate(int iClient)
{
	BaseClass::ClientUpdate(iClient);

	if (HasDesiredMove())
	{
		Vector vecDesiredMove = GetDesiredMove();
		CNetwork::CallFunction(iClient, "SetDesiredMove", GetHandle(), vecDesiredMove.x, vecDesiredMove.y, vecDesiredMove.z);
	}

	if (HasDesiredTurn())
		CNetwork::CallFunction(iClient, "SetDesiredTurn", GetHandle(), GetDesiredTurn());

	if (HasDesiredAim())
	{
		Vector vecDesiredAim = GetDesiredAim();
		CNetwork::CallFunction(iClient, "SetDesiredAim", GetHandle(), vecDesiredAim.x, vecDesiredAim.y, vecDesiredAim.z);
	}

	CNetwork::CallFunction(iClient, "SetAttackPower", GetHandle(), m_flAttackPower);
	CNetwork::CallFunction(iClient, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);
}

void CDigitank::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit)
{
	if (flDamage > 0)
		CancelGoalMovePosition();

	size_t iTutorial = CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial();
	if (iTutorial == CInstructor::TUTORIAL_FINISHHIM)
	{
		// BOT MUST DIE
		if (GetDigitanksTeam() != DigitanksGame()->GetLocalDigitanksTeam())
			flDamage += 50;
	}

	Speak(TANKSPEECH_DAMAGED);
	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);

	size_t iDifficulty = DigitanksGame()->GetDifficulty();

	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);

	if (!CNetwork::IsConnected() && iDifficulty == 0)
	{
		if (DigitanksGame()->IsTeamControlledByMe(GetTeam()))
			flDamage *= 0.5f;
		else if (dynamic_cast<CDigitank*>(pAttacker) && DigitanksGame()->IsTeamControlledByMe(dynamic_cast<CDigitank*>(pAttacker)->GetTeam()))
			flDamage *= 2.0f;
	}

	Vector vecAttackDirection = ((bDirectHit?pAttacker:pInflictor)->GetOrigin() - GetOrigin()).Normalized();

	float* pflShield = GetShieldForAttackDirection(vecAttackDirection);

	float flDamageBlocked = (*pflShield) * GetDefenseScale();

	if (pProjectile)
		flDamageBlocked *= pProjectile->ShieldDamageScale();

	if (flDamage - flDamageBlocked <= 0)
	{
		*pflShield -= flDamage / GetDefenseScale();

		if (GetVisibility() > 0)
		{
			EmitSound("sound/shield-damage.wav");
			SetSoundVolume("sound/shield-damage.wav", RemapValClamped(flDamage, 0, 5, 0, 0.5f));
		}

		DigitanksGame()->OnTakeShieldDamage(this, pAttacker, pInflictor, flDamage, bDirectHit, true);

		return;
	}

	flDamage -= flDamageBlocked;

	if (pProjectile)
		flDamage *= pProjectile->HealthDamageScale();

	if (GetVisibility() > 0)
	{
		EmitSound("sound/tank-damage.wav");
		SetSoundVolume("sound/tank-damage.wav", RemapValClamped(flDamage, 0, 5, 0, 1));
	}

	if (*pflShield > 1.0f)
	{
		if (GetVisibility() > 0)
		{
			EmitSound("sound/shield-damage.wav");
			SetSoundVolume("sound/shield-damage.wav", RemapValClamped(*pflShield, 0, 5, 0, 1));
		}
	}

	*pflShield = 0;

	DigitanksGame()->OnTakeShieldDamage(this, pAttacker, pInflictor, flDamageBlocked, bDirectHit, false);

	BaseClass::TakeDamage(pAttacker, pInflictor, flDamage, bDirectHit);
}

void CDigitank::OnKilled(CBaseEntity* pKilledBy)
{
	size_t iTutorial = CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial();
	if (iTutorial == CInstructor::TUTORIAL_FINISHHIM)
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();

	CDigitank* pKiller = dynamic_cast<CDigitank*>(pKilledBy);

	if (pKiller)
	{
		pKiller->GiveBonusPoints(1);
		pKiller->Speak(TANKSPEECH_KILL);
		pKiller->m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);
	}

	if (m_hSupplyLine != NULL)
		m_hSupplyLine->Delete();
}

Vector CDigitank::GetRenderOrigin() const
{
	return GetDesiredMove() + Vector(0, 1, 0);
}

EAngle CDigitank::GetRenderAngles() const
{
	if (GetDigitanksTeam()->IsCurrentSelection(this))
	{
		if (DigitanksGame()->GetControlMode() == MODE_TURN && GetPreviewTurnPower() <= GetTotalMovementPower())
			return EAngle(0, GetPreviewTurn(), 0);
	}

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
	{
		if (CDigitanksWindow::Get()->IsShiftDown() && GetTeam() == DigitanksGame()->GetCurrentSelection()->GetTeam())
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

void CDigitank::PreRender()
{
#ifdef _DEBUG
	if (HasFortifyPoint())
	{
		CRenderingContext r(Game()->GetRenderer());
		r.Translate(GetFortifyPoint() + Vector(0, 2, 0));
		r.SetColor(GetTeam()->GetColor());
		//glutWireCube(4);
	}
#endif
}

void CDigitank::ModifyContext(CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	if (!GetTeam())
		return;

	pContext->SetColorSwap(GetTeam()->GetColor());
}

void CDigitank::OnRender()
{
	BaseClass::OnRender();

	RenderTurret();

	if (GetFrontShieldStrength() > 0)
		RenderShield(GetFrontShieldStrength(), 0);

	if (GetLeftShieldStrength() > 0)
		RenderShield(GetLeftShieldStrength(), 90);

	if (GetRearShieldStrength() > 0)
		RenderShield(GetRearShieldStrength(), 180);

	if (GetRightShieldStrength() > 0)
		RenderShield(GetRightShieldStrength(), 270);
}

void CDigitank::RenderTurret(float flAlpha)
{
	if (m_iTurretModel == ~0)
		return;

	if (GetVisibility() == 0 || flAlpha == 0)
		return;

	CRenderingContext r(Game()->GetRenderer());
	r.SetAlpha(flAlpha*GetVisibility());
	r.SetBlend(BLEND_ALPHA);
	r.Translate(Vector(-0.527677f, 0.810368f, 0));

	if ((GetDigitanksTeam()->IsCurrentSelection(this) && DigitanksGame()->GetControlMode() == MODE_AIM) || HasDesiredAim())
	{
		Vector vecAimTarget;
		if (GetDigitanksTeam()->IsCurrentSelection(this) && DigitanksGame()->GetControlMode() == MODE_AIM)
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
	if (m_iShieldModel == ~0)
		return;

	if (GetVisibility() == 0 || flAlpha == 0)
		return;

	CRenderingContext r(Game()->GetRenderer());
	r.SetAlpha(flAlpha*GetVisibility());
	r.Rotate(flAngle, Vector(0, 1, 0));
	r.SetBlend(BLEND_ADDITIVE);
	r.Scale(RenderShieldScale(), 1, RenderShieldScale());
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

	CSelectable* pCurrentSelection = DigitanksGame()->GetCurrentSelection();
	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
	{
		EAngle angTurn = EAngle(0, GetDesiredTurn(), 0);
		if (CDigitanksWindow::Get()->IsShiftDown() && TurnsWith(pCurrentTank))
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

	if (pCurrentTank && DigitanksGame()->GetControlMode() == MODE_MOVE)
	{
		if (pCurrentTank->IsPreviewMoveValid())
		{
			if (GetDigitanksTeam()->IsCurrentSelection(this))
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
			else if (CDigitanksWindow::Get()->IsShiftDown() && MovesWith(pCurrentTank))
			{
				Vector vecTankMove = pCurrentTank->GetPreviewMove() - pCurrentTank->GetOrigin();
				if (vecTankMove.Length() > GetMaxMovementDistance())
					vecTankMove = vecTankMove.Normalized() * GetTotalMovementPower() * 0.95f;

				Vector vecNewPosition = GetOrigin() + vecTankMove;
				vecNewPosition.y = FindHoverHeight(vecNewPosition);

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
		if (GetDigitanksTeam()->IsCurrentSelection(this) && DigitanksGame()->GetControlMode() == MODE_AIM)
			iAlpha = 255;

		Vector vecTankOrigin = GetDesiredMove();

		float flGravity = DigitanksGame()->GetGravity();
		float flTime;
		Vector vecForce;
		FindLaunchVelocity(vecTankOrigin, m_vecDisplayAim, flGravity, vecForce, flTime, ProjectileCurve());

		CRopeRenderer oRope(Game()->GetRenderer(), s_iAimBeam, vecTankOrigin);
		oRope.SetWidth(0.5f);
		oRope.SetColor(Color(255, 0, 0, iAlpha));
		oRope.SetTextureScale(50);
		oRope.SetTextureOffset(-fmod(Game()->GetGameTime(), 1));

		size_t iLinks = 20;
		float flTimePerLink = flTime/iLinks;
		for (size_t i = 1; i < iLinks; i++)
		{
			float flCurrentTime = flTimePerLink*i;
			Vector vecCurrentOrigin = vecTankOrigin + vecForce*flCurrentTime + Vector(0, flGravity*flCurrentTime*flCurrentTime/2, 0);
			oRope.AddLink(vecCurrentOrigin);
		}

		oRope.Finish(m_vecDisplayAim);

		glPopAttrib();
	}

	bool bShowGoalMove = false;
	Vector vecGoalMove;
	int iAlpha = 255;
	if (HasGoalMovePosition())
	{
		bShowGoalMove = true;
		vecGoalMove = GetGoalMovePosition();
	}
	else if (this == pCurrentTank && DigitanksGame()->GetControlMode() == MODE_MOVE)
	{
		if (!pCurrentTank->IsPreviewMoveValid())
		{
			bShowGoalMove = true;
			vecGoalMove = GetPreviewMove();
			iAlpha = 100;
		}
	}

	if (GetTeam() != Game()->GetLocalTeam())
		bShowGoalMove = false;

	if (bShowGoalMove)
	{
		CRenderingContext r(Game()->GetRenderer());
		r.SetBlend(BLEND_ALPHA);

		CRopeRenderer oRope(Game()->GetRenderer(), s_iAutoMove, GetDesiredMove());
		oRope.SetWidth(2.0f);
		oRope.SetColor(Color(255, 255, 255, iAlpha));
		oRope.SetTextureScale(3);
		oRope.SetTextureOffset(-fmod(Game()->GetGameTime(), 1));

		Vector vecPath = vecGoalMove - GetDesiredMove();
		vecPath.y = 0;

		float flDistance = vecPath.Length2D();
		Vector vecDirection = vecPath.Normalized();
		size_t iSegments = (size_t)(flDistance/3);

		for (size_t i = 0; i < iSegments; i++)
		{
			float flCurrentDistance = ((float)i*flDistance)/iSegments;
			oRope.AddLink(DigitanksGame()->GetTerrain()->SetPointHeight(GetDesiredMove() + vecDirection*flCurrentDistance) + Vector(0, 1, 0));
		}

		oRope.Finish(vecGoalMove);
	}
}

void CDigitank::UpdateInfo(std::wstring& sInfo)
{
	std::wstringstream s;

	s << GetName();

	if (IsFortified())
		s << L"\n[Fortified]";

	else if (IsFortifying())
		s << L"\n[Fortifying...]";

	if (HasBonusPoints())
	{
		if (GetBonusPoints() > 1)
			s << L"\n \n" << GetBonusPoints() << L" upgrades";
		else
			s << L"\n \n1 upgrade";
	}

	if (GetBonusAttackPower())
	{
		s << L"\n \n+" << (int)GetBonusAttackPower() << L" attack energy";

		if (IsFortified() && (int)GetFortifyAttackPowerBonus() > 0)
			s << L"\n (+" << (int)GetFortifyAttackPowerBonus() << L" from fortify)";

		if ((int)GetSupportAttackPowerBonus() > 0)
			s << L"\n (+" << (int)GetSupportAttackPowerBonus() << L" from support)";
	}

	if (GetBonusDefensePower())
	{
		s << L"\n \n+" << (int)GetBonusDefensePower() << L" defense energy";

		if (IsFortified() && (int)GetFortifyDefensePowerBonus() > 0)
			s << L"\n (+" << (int)GetFortifyDefensePowerBonus() << L" from fortify)";

		if ((int)GetSupportDefensePowerBonus() > 0)
			s << L"\n (+" << (int)GetSupportDefensePowerBonus() << L" from support)";
	}

	if (GetBonusMovementPower())
		s << L"\n \n+" << (int)GetBonusMovementPower() << L" movement energy";

	sInfo = s.str();
}

void CDigitank::GiveBonusPoints(size_t i, bool bPlayEffects)
{
	if (!CNetwork::IsHost())
		return;

	m_iBonusPoints += i;

	if (bPlayEffects)
	{
		TankPromoted(NULL);
		CNetwork::CallFunction(-1, "TankPromoted", GetHandle());
	}

	CNetwork::CallFunction(-1, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	Speak(TANKSPEECH_PROMOTED);
	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);
}

void CDigitank::PromoteAttack()
{
	if (!CNetwork::IsHost())
	{
		CNetwork::CallFunction(-1, "PromoteAttack", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusAttackPower++;

	CNetwork::CallFunction(-1, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	if (GetTeam()->IsPlayerControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::PromoteDefense()
{
	if (!CNetwork::IsHost())
	{
		CNetwork::CallFunction(-1, "PromoteDefense", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusDefensePower++;

	CNetwork::CallFunction(-1, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	if (GetTeam()->IsPlayerControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::PromoteMovement()
{
	if (!CNetwork::IsHost())
	{
		CNetwork::CallFunction(-1, "PromoteMovement", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusMovementPower++;

	CNetwork::CallFunction(-1, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	if (GetTeam()->IsPlayerControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::SetBonusPoints(class CNetworkParameters* p)
{
	m_iBonusPoints = p->i2;
	m_flBonusAttackPower = p->fl3;
	m_flBonusDefensePower = p->fl4;
	m_flBonusMovementPower = p->fl5;
}

void CDigitank::TankPromoted(class CNetworkParameters* p)
{
	if (GetVisibility() > 0)
	{
		EmitSound("sound/tank-promoted.wav");
		CParticleSystemLibrary::AddInstance(L"promotion", GetOrigin());
	}
}

void CDigitank::PromoteAttack(class CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	PromoteAttack();
}

void CDigitank::PromoteDefense(class CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	PromoteDefense();
}

void CDigitank::PromoteMovement(class CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	PromoteMovement();
}

void CDigitank::Speak(size_t iSpeech)
{
	if (!CNetwork::IsHost())
		return;

	if (rand()%4 != 0)
		return;

	if (Game()->GetGameTime() < m_flLastSpeech + 5.0f)
		return;

	if (GetVisibility() == 0)
		return;

	size_t iLine = g_aiSpeechLines[iSpeech][rand()%g_aiSpeechLines[iSpeech].size()];

	m_flLastSpeech = Game()->GetGameTime();
	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);

	CNetwork::CallFunction(-1, "TankSpeak", GetHandle(), iLine);

	CNetworkParameters p;
	p.i2 = (int)iLine;
	Speak(&p);
}

void CDigitank::Speak(class CNetworkParameters* p)
{
	DigitanksGame()->TankSpeak(this, s_apszTankLines[p->i2]);
}

float CDigitank::FindHoverHeight(Vector vecPosition) const
{
	CTerrain* pTerrain = DigitanksGame()->GetTerrain();

	float flHighestTerrain = pTerrain->GetHeight(vecPosition.x, vecPosition.z);

	float flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x+2, vecPosition.z+2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x+2, vecPosition.z-2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x-2, vecPosition.z+2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x-2, vecPosition.z-2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	return flHighestTerrain;
}

bool CDigitank::Collide(const Vector& v1, const Vector& v2, Vector& vecPoint)
{
	if (BaseClass::Collide(v1, v2, vecPoint))
		return true;

	if (GetBoundingRadius() == 0)
		return false;

	return LineSegmentIntersectsSphere(v1, v2, GetDesiredMove(), GetBoundingRadius(), vecPoint);
}

float CDigitank::HealthRechargeRate() const
{
	return 0.2f + GetSupportHealthRechargeBonus();
}

float CDigitank::ShieldRechargeRate() const
{
	return 0.2f + GetSupportShieldRechargeBonus();
}

void CDigitank::SetFortifyPoint(Vector vecFortify)
{
	m_bFortifyPoint = true;
	m_vecFortifyPoint = vecFortify;
}
