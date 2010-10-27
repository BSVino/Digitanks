#include "digitank.h"

#include <sstream>

#include <GL/glew.h>
#include <maths.h>
#include <mtrand.h>
#include <models/models.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <glgui/glgui.h>

#include "digitanksgame.h"
#include "dt_camera.h"
#include "ui/digitankswindow.h"
#include "ui/instructor.h"
#include "powerup.h"
#include "ui/debugdraw.h"
#include "ui/hud.h"
#include "structure.h"
#include "supplyline.h"
#include "projectile.h"
#include "scout.h"

size_t CDigitank::s_iAimBeam = 0;
size_t CDigitank::s_iCancelIcon = 0;
size_t CDigitank::s_iMoveIcon = 0;
size_t CDigitank::s_iTurnIcon = 0;
size_t CDigitank::s_iAimIcon = 0;
size_t CDigitank::s_iFireIcon = 0;
size_t CDigitank::s_iEnergyIcon = 0;
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

NETVAR_TABLE_BEGIN(CDigitank);
	NETVAR_DEFINE(float, m_flStartingPower);
	NETVAR_DEFINE(float, m_flTotalPower);
	NETVAR_DEFINE(float, m_flAttackPower);
	NETVAR_DEFINE(float, m_flDefensePower);
	NETVAR_DEFINE(float, m_flMovementPower);
	NETVAR_DEFINE(float, m_flAttackSplit);

	NETVAR_DEFINE(float, m_flBonusAttackPower);
	NETVAR_DEFINE(float, m_flBonusDefensePower);
	NETVAR_DEFINE(float, m_flBonusMovementPower);
	NETVAR_DEFINE_CALLBACK(size_t, m_iBonusPoints, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(float, m_flRangeBonus);

	NETVAR_DEFINE(float, m_flFrontShieldStrength);
	NETVAR_DEFINE(float, m_flLeftShieldStrength);
	NETVAR_DEFINE(float, m_flRightShieldStrength);
	NETVAR_DEFINE(float, m_flRearShieldStrength);

	NETVAR_DEFINE(Vector, m_vecPreviousOrigin);

	NETVAR_DEFINE(float, m_flPreviousTurn);

	NETVAR_DEFINE(Vector, m_vecLastAim);

	NETVAR_DEFINE(bool, m_bGoalMovePosition);
	NETVAR_DEFINE(Vector, m_vecGoalMovePosition);

	NETVAR_DEFINE_CALLBACK(bool, m_bFortified, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(size_t, m_iFortifyLevel);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitank);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flStartingPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flTotalPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flAttackPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flDefensePower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flMovementPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flAttackSplit);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBonusAttackPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBonusDefensePower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBonusMovementPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iBonusPoints);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flRangeBonus);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flFrontMaxShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLeftMaxShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flRightMaxShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flRearMaxShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flFrontShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flLeftShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flRightShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flRearShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecPreviewMove);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecPreviousOrigin);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flStartedMove);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flPreviewTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flPreviousTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flStartedTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bPreviewAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecPreviewAim);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecLastAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bDisplayAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecDisplayAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flDisplayAimRadius);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bGoalMovePosition);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecGoalMovePosition);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bChoseFirepower);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bFiredWeapon);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flFireProjectileTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iFireProjectiles);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<class CProjectile>, m_hProjectile);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLastSpeech);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flNextIdle);
	//size_t						m_iTurretModel;	// Set in Spawn()
	//size_t						m_iShieldModel;	// Set in Spawn()
	//size_t						m_iHoverParticles;	// Dynamic
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bFortified);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iFortifyLevel);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flFortifyTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<class CSupplier>, m_hSupplier);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<class CSupplyLine>, m_hSupplyLine);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flBobOffset);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bFortifyPoint);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecFortifyPoint);
SAVEDATA_TABLE_END();

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
	s_iEnergyIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-energy.png");
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

void CDigitank::Spawn()
{
	BaseClass::Spawn();

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

	m_bGoalMovePosition = false;

	m_flStartedMove = 0;
	m_flStartedTurn = 0;

	m_bFortified = false;

	m_flFrontMaxShieldStrength = m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flRearMaxShieldStrength = 15;
	m_flFrontShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flRearShieldStrength = 15;

	m_flFireProjectileTime = 0;
	m_iFireProjectiles = 0;

	m_flLastSpeech = 0;
	m_flNextIdle = 10.0f;

	SetCollisionGroup(CG_ENTITY);

	m_iTurretModel = m_iShieldModel = ~0;

	m_iHoverParticles = ~0;

	m_bFortifyPoint = false;

	m_flFortifyTime = 0;

	m_flBobOffset = RandomFloat(0, 10);
}

float CDigitank::GetBaseAttackPower(bool bPreview)
{
	if (GetDigitanksTeam()->IsSelected(this) && bPreview && (DigitanksGame()->GetControlMode() == MODE_AIM || DigitanksGame()->GetControlMode() == MODE_FIRE))
		return m_flTotalPower * m_flAttackSplit;

	return m_flAttackPower;
}

float CDigitank::GetBaseDefensePower(bool bPreview)
{
	if (GetDigitanksTeam()->IsSelected(this) && bPreview && (DigitanksGame()->GetControlMode() == MODE_AIM || DigitanksGame()->GetControlMode() == MODE_FIRE))
		return m_flTotalPower * (1-m_flAttackSplit);

	if (GetDigitanksTeam()->IsSelected(this) && bPreview)
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
	if (GetDigitanksTeam()->IsSelected(this) && bPreview)
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
	return m_flStartingPower.Get() + GetBonusMovementPower();
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

	if (m_hSupplyLine == NULL)
		return 0;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = (float)m_hSupplier->EnergyBonus() * m_hSupplyLine->GetIntegrity();

	return flBonus;
}

float CDigitank::GetSupportDefensePowerBonus()
{
	if (m_hSupplier == NULL)
		return 0;

	if (m_hSupplyLine == NULL)
		return 0;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = (float)m_hSupplier->EnergyBonus() * m_hSupplyLine->GetIntegrity();

	return flBonus;
}

float CDigitank::GetSupportHealthRechargeBonus() const
{
	if (m_hSupplier == NULL)
		return 0.0f;

	if (m_hSupplyLine == NULL)
		return 0;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = m_hSupplier->RechargeBonus()/5 * m_hSupplyLine->GetIntegrity();

	return flBonus;
}

float CDigitank::GetSupportShieldRechargeBonus() const
{
	if (m_hSupplier == NULL)
		return 0.0f;

	if (m_hSupplyLine == NULL)
		return 0;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = m_hSupplier->RechargeBonus() * m_hSupplyLine->GetIntegrity();

	return flBonus;
}

void CDigitank::SetAttackPower(float flAttackPower)
{
	if (flAttackPower > 1)
		flAttackPower = 1;
	if (flAttackPower < 0)
		flAttackPower = 0;

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(NETWORK_TOEVERYONE, "SetAttackPower", GetHandle(), flAttackPower);

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
	float flMovePower = 0;
	float flTurnPower = 0;

	if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		flMovePower = GetPreviewBaseMovePower();

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
		flTurnPower = GetPreviewBaseTurnPower();

	float flPower = flMovePower + flTurnPower - m_flBonusMovementPower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewMovePower() const
{
	float flMovePower = 0;

	if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		flMovePower = GetPreviewBaseMovePower();

	float flPower = flMovePower - m_flBonusMovementPower.Get();
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewTurnPower() const
{
	float flTurnPower = 0;

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
		flTurnPower = GetPreviewBaseTurnPower();

	float flPower = flTurnPower - m_flBonusMovementPower.Get();
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewBaseMovePower() const
{
	return (m_vecPreviewMove - GetOrigin()).Length() / GetTankSpeed();
}

float CDigitank::GetPreviewBaseTurnPower() const
{
	return fabs(AngleDifference(m_flPreviewTurn, GetAngles().y)/TurnPerPower());
}

bool CDigitank::IsPreviewMoveValid() const
{
	if (GetPreviewBaseMovePower() > GetTotalPower())
		return false;

	float flMapSize = DigitanksGame()->GetTerrain()->GetMapSize();

	if (GetPreviewMove().x > flMapSize)
		return false;
	if (GetPreviewMove().x < -flMapSize)
		return false;
	if (GetPreviewMove().z > flMapSize)
		return false;
	if (GetPreviewMove().z < -flMapSize)
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

float CDigitank::GetShieldValueForAttackDirection(Vector vecAttack)
{
	Vector vecForward, vecRight;
	AngleVectors(GetAngles(), &vecForward, &vecRight, NULL);

	float flForwardDot = vecForward.Dot(vecAttack);
	float flRightDot = vecRight.Dot(vecAttack);

	if (flForwardDot > 0.7f)
		return m_flFrontShieldStrength.Get();
	else if (flForwardDot < -0.7f)
		return m_flRearShieldStrength.Get();
	else if (flRightDot < -0.7f)
		return m_flRightShieldStrength.Get();
	else
		return m_flLeftShieldStrength.Get();
}

void CDigitank::SetShieldValueForAttackDirection(Vector vecAttack, float flValue)
{
	Vector vecForward, vecRight;
	AngleVectors(GetAngles(), &vecForward, &vecRight, NULL);

	float flForwardDot = vecForward.Dot(vecAttack);
	float flRightDot = vecRight.Dot(vecAttack);

	if (flForwardDot > 0.7f)
		m_flFrontShieldStrength = flValue;
	else if (flForwardDot < -0.7f)
		m_flRearShieldStrength = flValue;
	else if (flRightDot < -0.7f)
		m_flRightShieldStrength = flValue;
	else
		m_flLeftShieldStrength = flValue;
}

float CDigitank::GetShieldValue(size_t i)
{
	if (i == 0)
		return m_flFrontShieldStrength;
	else if (i == 1)
		return m_flLeftShieldStrength;
	else if (i == 2)
		return m_flRightShieldStrength;
	else if (i == 3)
		return m_flRearShieldStrength;

	return 0;
}

void CDigitank::SetShieldValue(size_t i, float flValue)
{
	if (i == 0)
		m_flFrontShieldStrength = flValue;
	else if (i == 1)
		m_flLeftShieldStrength = flValue;
	else if (i == 2)
		m_flRightShieldStrength = flValue;
	else if (i == 3)
		m_flRearShieldStrength = flValue;
}

void CDigitank::StartTurn()
{
	BaseClass::StartTurn();

	ManageSupplyLine();

	if (CNetwork::IsHost() && m_bFortified)
	{
		if (m_iFortifyLevel < 5)
			m_iFortifyLevel++;
	}

	for (size_t i = 0; i < TANK_SHIELDS; i++)
	{
		float flShieldStrength = GetShieldValue(i);
		SetShieldValue(i, Approach(m_flMaxShieldStrengths[i], flShieldStrength, ShieldRechargeRate()));

		if (flShieldStrength - GetShieldValue(i) < 0)
			DigitanksGame()->OnTakeShieldDamage(this, NULL, NULL, flShieldStrength - GetShieldValue(i), true, false);
	}

	m_vecPreviewMove = GetOrigin();
	m_flPreviewTurn = GetAngles().y;

	if (CNetwork::IsHost())
	{
		m_flTotalPower = m_flStartingPower;
		m_flMovementPower = m_flAttackPower = m_flDefensePower = 0;
	}

	m_bChoseFirepower = false;
	m_bFiredWeapon = false;

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	CDigitank* pClosestEnemy = FindClosestVisibleEnemyTank();

	if (HasGoalMovePosition() && pClosestEnemy)
	{
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_AUTOMOVEENEMY);
	}
	else
	// Artillery gets unit orders even if fortified but infantry doesn't.
	if (!HasGoalMovePosition() && (!IsFortified() || IsArtillery()))
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_UNITORDERS);
	else
	// Notify if infantry can see an enemy they can shoot.
	if (IsInfantry() && IsFortified() && FindClosestVisibleEnemyTank(true))
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_FORTIFIEDENEMY);

	if (HasGoalMovePosition())
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

	if (CNetwork::IsHost())
	{
		m_flDefensePower = m_flTotalPower;
		m_flTotalPower = 0;
	}
}

void CDigitank::ManageSupplyLine()
{
	if (!CNetwork::IsHost())
		return;

	// Scouts don't get supply lines. They give away your base's location in the early game!
	if (IsScout())
		return;

	CSupplier* pSupplier = CSupplier::FindClosestSupplier(this);
	CSupplyLine* pSupplyLine = m_hSupplyLine;

	if (pSupplyLine == NULL && pSupplier != NULL)
		pSupplyLine = GameServer()->Create<CSupplyLine>("CSupplyLine");

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = pSupplier?pSupplier->GetHandle():~0;
	p.ui3 = pSupplyLine?pSupplyLine->GetHandle():~0;

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "ManageSupplyLine", &p);

	ManageSupplyLine(&p);
}

void CDigitank::ManageSupplyLine(CNetworkParameters* p)
{
	m_hSupplier = CEntityHandle<CSupplier>(p->ui2);
	m_hSupplyLine = CEntityHandle<CSupplyLine>(p->ui3);

	if (m_hSupplyLine != NULL && m_hSupplier != NULL)
		m_hSupplyLine->SetEntities(m_hSupplier, this);
}

CDigitank* CDigitank::FindClosestVisibleEnemyTank(bool bInRange)
{
	CDigitank* pClosestEnemy = NULL;
	while (true)
	{
		pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(GetOrigin(), pClosestEnemy);

		if (!pClosestEnemy)
			break;

		if (pClosestEnemy->GetTeam() == GetTeam())
			continue;

		if (bInRange)
		{
			if ((pClosestEnemy->GetOrigin() - GetOrigin()).Length() > GetMaxRange())
				return NULL;
		}
		else
		{
			if ((pClosestEnemy->GetOrigin() - GetOrigin()).Length() > VisibleRange()+DigitanksGame()->FogPenetrationDistance())
				return NULL;
		}

		break;
	}

	return pClosestEnemy;
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

	if ((vecPreviewAim - GetOrigin()).LengthSqr() > GetMaxRange()*GetMaxRange())
	{
		vecPreviewAim = GetOrigin() + (vecPreviewAim - GetOrigin()).Normalized() * GetMaxRange() * 0.99f;
		vecPreviewAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecPreviewAim.x, vecPreviewAim.z);
	}

	if ((vecPreviewAim - GetOrigin()).LengthSqr() < GetMinRange()*GetMinRange())
	{
		vecPreviewAim = GetOrigin() + (vecPreviewAim - GetOrigin()).Normalized() * GetMinRange() * 1.01f;
		vecPreviewAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecPreviewAim.x, vecPreviewAim.z);
	}

	if (fabs(AngleDifference(GetAngles().y, VectorAngles((vecPreviewAim-GetOrigin()).Normalized()).y)) > FiringCone())
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

	if ((GetPreviewAim() - GetOrigin()).LengthSqr() > GetMaxRange()*GetMaxRange())
		return false;

	if ((GetPreviewAim() - GetOrigin()).LengthSqr() < GetMinRange()*GetMinRange())
		return false;

	return true;
}

void CDigitank::Move()
{
	if (IsFortified() && !CanMoveFortified())
		return;

	if (!IsPreviewMoveValid())
		return;

	if (fabs(m_vecPreviewMove.x) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	if (fabs(m_vecPreviewMove.z) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	Speak(TANKSPEECH_MOVED);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(NETWORK_TOEVERYONE, "Move", GetHandle(), m_vecPreviewMove.x, m_vecPreviewMove.y, m_vecPreviewMove.z);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = m_vecPreviewMove.x;
	p.fl3 = m_vecPreviewMove.y;
	p.fl4 = m_vecPreviewMove.z;
	Move(&p);
}

void CDigitank::Move(CNetworkParameters* p)
{
	m_vecPreviewMove = Vector(p->fl2, p->fl3, p->fl4);

	float flMovePower = GetPreviewBaseMovePower();

	m_vecPreviousOrigin = GetOrigin();
	m_flStartedMove = GameServer()->GetGameTime();
	SetOrigin(m_vecPreviewMove);

	if (GetVisibility() > 0)
	{
		EmitSound("sound/tank-move.wav");
		SetSoundVolume("sound/tank-move.wav", 0.5f);

		m_iHoverParticles = CParticleSystemLibrary::AddInstance(L"tank-hover", GetOrigin());
		if (m_iHoverParticles != ~0)
			CParticleSystemLibrary::GetInstance(m_iHoverParticles)->FollowEntity(this);
	}

	if (m_iHoverParticles != ~0)
		CParticleSystemLibrary::StopInstance(m_iHoverParticles);

	// Am I waiting to fire something? Fire now. Shoot and scoot baby!
	if (m_flFireProjectileTime)
		m_flFireProjectileTime = GameServer()->GetGameTime();

	if (CNetwork::IsHost())
	{
		m_flTotalPower -= flMovePower;
		m_flMovementPower += flMovePower;
	}

	if (CNetwork::IsHost() && CanGetPowerups())
	{
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
	}

	for (size_t i = 0; i < Game()->GetNumTeams(); i++)
	{
		if (Game()->GetTeam(i))
			DigitanksGame()->GetDigitanksTeam(i)->CalculateVisibility();
	}

	InterceptSupplyLines();

	CDigitank* pClosestEnemy = FindClosestVisibleEnemyTank();
	if (HasGoalMovePosition() && pClosestEnemy)
	{
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_AUTOMOVEENEMY);
	}

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	CDigitanksWindow::Get()->GetHUD()->UpdateTurnButton();
}

bool CDigitank::IsMoving()
{
	float flTransitionTime = GetTransitionTime();

	if (GetVisibility() == 0)
		flTransitionTime = 0;

	float flTimeSinceMove = GameServer()->GetGameTime() - m_flStartedMove;
	if (m_flStartedMove && flTimeSinceMove < flTransitionTime)
		return true;

	return false;
}

void CDigitank::Turn()
{
	if (IsFortified() && !CanTurnFortified())
		return;

	float flMovePower = GetPreviewBaseTurnPower();

	if (flMovePower > m_flTotalPower)
		return;

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(NETWORK_TOEVERYONE, "Turn", GetHandle(), m_flPreviewTurn);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = m_flPreviewTurn;
	Turn(&p);
}

void CDigitank::Turn(CNetworkParameters* p)
{
	m_flPreviewTurn = p->fl2;

	m_flPreviousTurn = GetAngles().y;
	m_flStartedTurn = GameServer()->GetGameTime();
	SetAngles(EAngle(0, m_flPreviewTurn, 0));

	float flMovePower = GetPreviewBaseTurnPower();

	if (CNetwork::IsHost())
	{
		m_flTotalPower -= flMovePower;
		m_flMovementPower += flMovePower;
	}

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	CDigitanksWindow::Get()->GetHUD()->UpdateTurnButton();
}

void CDigitank::SetGoalMovePosition(const Vector& vecPosition)
{
	if (IsFortified() && !CanMoveFortified())
		return;

	if (fabs(vecPosition.x) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	if (fabs(vecPosition.z) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	if (CNetwork::IsHost())
	{
		CNetworkParameters p;
		p.fl2 = vecPosition.x;
		p.fl3 = vecPosition.y;
		p.fl4 = vecPosition.z;
		SetGoalMovePosition(&p);
	}
	else
		CNetwork::CallFunction(NETWORK_TOSERVER, "SetGoalMovePosition", GetHandle(), vecPosition.x, vecPosition.y, vecPosition.z);
}

void CDigitank::SetGoalMovePosition(CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	m_bGoalMovePosition = true;
	m_vecGoalMovePosition = Vector(p->fl2, p->fl3, p->fl4);

	MoveTowardsGoalMovePosition();
}

void CDigitank::MoveTowardsGoalMovePosition()
{
	if (!CNetwork::IsHost())
		return;

	CNetwork::SetRunningClientFunctions(false);

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
	Move();

	if ((GetOrigin() - GetGoalMovePosition()).LengthSqr() < 1)
	{
		m_bGoalMovePosition = false;
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_UNITAUTOMOVE);
		return;
	}
}

void CDigitank::CancelGoalMovePosition()
{
	if (CNetwork::IsHost())
		CancelGoalMovePosition(NULL);
	else
		CNetwork::CallFunction(NETWORK_TOSERVER, "CancelGoalMovePosition", GetHandle());
}

void CDigitank::CancelGoalMovePosition(CNetworkParameters* p)
{
	m_bGoalMovePosition = false;
}

void CDigitank::Fortify()
{
	if (!CanFortify())
		return;

	CNetwork::CallFunction(NETWORK_TOEVERYONE, "Fortify", GetHandle());

	CNetworkParameters p;
	p.ui1 = GetHandle();
	Fortify(&p);
}

void CDigitank::Fortify(CNetworkParameters* p)
{
	m_flFortifyTime = GameServer()->GetGameTime();

	if (m_bFortified)
	{
		m_bFortified = false;
		CDigitanksWindow::Get()->GetHUD()->UpdateTurnButton();
		return;
	}

	m_bFortified = true;

	m_iFortifyLevel = 0;

	OnFortify();

	CDigitanksWindow::Get()->GetHUD()->UpdateTurnButton();

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_FORTIFYING);
	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_DEPLOYING, true);
}

bool CDigitank::CanAim() const
{
	return AllowControlMode(MODE_AIM);
}

bool CDigitank::MovesWith(CDigitank* pOther) const
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

	// Only same class tanks.
	if (GetBuildUnit() != pOther->GetBuildUnit())
		return false;

	if (!GetDigitanksTeam()->IsSelected(this))
		return false;

	return true;
}

bool CDigitank::TurnsWith(CDigitank* pOther) const
{
	if (!pOther)
		return false;

	if (this == pOther)
		return true;

	// Only same team.
	if (GetTeam() != pOther->GetTeam())
		return false;

	if (!GetDigitanksTeam()->IsSelected(this))
		return false;

	// Only same class tanks.
	if (GetBuildUnit() != pOther->GetBuildUnit())
		return false;

	return true;
}

bool CDigitank::AimsWith(CDigitank* pOther) const
{
	if (!pOther)
		return false;

	if (this == pOther)
		return true;

	// Only same team.
	if (GetTeam() != pOther->GetTeam())
		return false;

	if (!GetDigitanksTeam()->IsSelected(this))
		return false;

	if (!CanAim())
		return false;

	if (!IsFortified() && !CanAimMobilized())
		return false;

	return true;
}

void CDigitank::Think()
{
	BaseClass::Think();

	m_bDisplayAim = false;

	bool bAimMode = DigitanksGame()->GetControlMode() == MODE_AIM;
	bool bShowThisTank = m_bFiredWeapon;
	if (bAimMode && GetDigitanksTeam()->IsSelected(this) && AimsWith(GetDigitanksTeam()->GetPrimarySelectionTank()))
		bShowThisTank = true;
	if (GetDigitanksTeam() != DigitanksGame()->GetCurrentTeam())
		bShowThisTank = false;

	if (bShowThisTank)
	{
		Vector vecMouseAim;
		CBaseEntity* pHit = NULL;
		bool bMouseOK = CDigitanksWindow::Get()->GetMouseGridPosition(vecMouseAim, &pHit);

		Vector vecTankAim;
		if (m_bFiredWeapon)
			vecTankAim = m_vecLastAim.Get();

		if (bMouseOK && bAimMode)
		{
			if (AimsWith(GetDigitanksTeam()->GetPrimarySelectionTank()) && GetDigitanksTeam()->IsSelected(this))
			{
				if (pHit && dynamic_cast<CDigitanksEntity*>(pHit) && pHit->GetTeam() && pHit->GetTeam() != GetTeam())
					vecTankAim = pHit->GetOrigin();
				else
					vecTankAim = vecMouseAim;
			}
		}

		if (m_bFiredWeapon || bMouseOK)
		{
			if ((vecTankAim - GetOrigin()).Length() > GetMaxRange())
			{
				vecTankAim = GetOrigin() + (vecTankAim - GetOrigin()).Normalized() * GetMaxRange() * 0.99f;
				vecTankAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecTankAim.x, vecTankAim.z);
			}

			if ((vecTankAim - GetOrigin()).Length() < GetMinRange())
			{
				vecTankAim = GetOrigin() + (vecTankAim - GetOrigin()).Normalized() * GetMinRange() * 1.01f;
				vecTankAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecTankAim.x, vecTankAim.z);
			}

			if (fabs(AngleDifference(GetAngles().y, VectorAngles((vecTankAim-GetOrigin()).Normalized()).y)) > FiringCone())
				m_bDisplayAim = false;
			else
			{
				float flDistance = (vecTankAim - GetOrigin()).Length();

				float flRadius = RemapValClamped(flDistance, GetEffRange(), GetMaxRange(), 2, MaxRangeRadius());
				DigitanksGame()->AddTankAim(vecTankAim, flRadius, GetDigitanksTeam()->IsSelected(this) && DigitanksGame()->GetControlMode() == MODE_AIM);

				m_bDisplayAim = true;
				m_vecDisplayAim = vecTankAim;
				m_flDisplayAimRadius = flRadius;
			}
		}
	}

	if (m_flFireProjectileTime && GameServer()->GetGameTime() > m_flFireProjectileTime)
	{
		m_iFireProjectiles--;
		FireProjectile();

		if (m_iFireProjectiles)
			m_flFireProjectileTime = GameServer()->GetGameTime() + FireProjectileTime();
		else
			m_flFireProjectileTime = 0;
	}

	if (m_iHoverParticles != ~0)
	{
		float flTransitionTime = GetTransitionTime();
		float flTimeSinceMove = GameServer()->GetGameTime() - m_flStartedMove;
		float flTimeSinceTurn = GameServer()->GetGameTime() - m_flStartedTurn;
		if (m_flStartedMove && flTimeSinceMove > flTransitionTime || m_flStartedTurn && flTimeSinceTurn > flTransitionTime)
		{
			CParticleSystemLibrary::StopInstance(m_iHoverParticles);
			m_iHoverParticles = ~0;
		}
	}

	if (IsAlive() && GameServer()->GetGameTime() > m_flNextIdle)
	{
		// A little bit less often if we're not on the current team.
		if (DigitanksGame()->GetCurrentTeam() == GetTeam() && rand()%2 == 0 || rand()%4 == 0)
			Speak(TANKSPEECH_IDLE);

		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
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
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

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

		if (!IsFortified() && !IsFortifying() && !m_bFiredWeapon && !HasGoalMovePosition())
			DigitanksGame()->SetControlMode(MODE_MOVE);
		//else if (!HasDesiredAim() && CanAim() && pClosestEnemy)
		//	DigitanksGame()->SetControlMode(MODE_AIM);
		else
			DigitanksGame()->SetControlMode(MODE_NONE);
	}
}

bool CDigitank::AllowControlMode(controlmode_t eMode) const
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

	if (eNewMode == MODE_AIM)
	{
		if (IsArtillery())
		{
			Vector vecCenter = DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + AngleVector(GetAngles()) * GetMaxRange()/2);
			DigitanksGame()->GetDigitanksCamera()->SetTarget(vecCenter);
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

bool CDigitank::NeedsOrders()
{
	bool bNeedsToMove = true;
	if (GetBaseMovementPower() > 0)
		bNeedsToMove = false;
	else
	{
		if (IsFortified() || IsFortifying())
			bNeedsToMove = false;
	}

	bool bNeedsToAttack = true;
	if (GetBaseAttackPower() > 0)
		bNeedsToAttack = false;
	else if (!IsScout())
	{
		CDigitanksEntity* pClosestEnemy = NULL;
		while (true)
		{
			pClosestEnemy = CBaseEntity::FindClosest<CDigitanksEntity>(GetOrigin(), pClosestEnemy);

			if (pClosestEnemy)
			{
				if ((pClosestEnemy->GetOrigin() - GetOrigin()).Length() > GetMaxRange())
				{
					pClosestEnemy = NULL;
					break;
				}

				if (pClosestEnemy->GetTeam() == GetTeam())
					continue;

				if (!pClosestEnemy->GetTeam())
					continue;

				if (pClosestEnemy->GetVisibility() == 0)
					continue;

				if (IsArtillery() && fabs(AngleDifference(GetAngles().y, VectorAngles((pClosestEnemy->GetOrigin()-GetOrigin()).Normalized()).y)) > FiringCone())
					continue;
			}
			break;
		}

		if (!pClosestEnemy)
			bNeedsToAttack = false;
	}
	else
	{
		// I'm a scout. BAM!
		CSupplyLine* pClosest = dynamic_cast<CScout*>(this)->FindClosestEnemySupplyLine(true);

		if (!pClosest)
			bNeedsToAttack = false;
	}

	if (IsArtillery() && !IsFortified())
		bNeedsToAttack = false;

	if (m_bFiredWeapon)
		bNeedsToMove = false;

	return bNeedsToMove || bNeedsToAttack;
}

void CDigitank::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	if (eMenuMode == MENUMODE_MAIN)
	{
		if (HasGoalMovePosition())
		{
			pHUD->SetButtonListener(0, CHUD::CancelAutoMove);
			pHUD->SetButtonColor(0, Color(100, 100, 100));
			pHUD->SetButtonTexture(0, s_iCancelIcon);
			pHUD->SetButtonInfo(0, L"CANCEL AUTO MOVE\n \nCancel this unit's auto move command.\n \nShortcut: Q");
		}
		else if (!IsFortified() && !IsFortifying())
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

			pHUD->SetButtonInfo(0, L"MOVE UNIT\n \nGo into Move mode. Right click inside the yellow area to move this unit.\n \nShortcut: Q");
		}

		if (!IsScout() && (!IsFortified() && !IsFortifying() || CanTurnFortified()) && m_flTotalPower > 1)
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
			pHUD->SetButtonInfo(1, L"ROTATE UNIT\n \nGo into Rotate mode. Right click any spot on the terrain to have this unit face that spot.\n \nShortcut: W");
		}

		if (CanFortify())
		{
			if (IsFortified() || IsFortifying())
			{
				pHUD->SetButtonTexture(8, s_iMobilizeIcon);
				pHUD->SetButtonInfo(8, L"MOBILIZE\n \nAllows this unit to move again.\n \nShortcut: F");
			}
			else
			{
				if (IsArtillery())
				{
					pHUD->SetButtonTexture(8, s_iDeployIcon);
					pHUD->SetButtonInfo(8, L"DEPLOY UNIT\n \nHave this artillery deploy. Artillery must be deployed before they can be fired.\n \nShortcut: F");
				}
				else
				{
					pHUD->SetButtonTexture(8, s_iFortifyIcon);
					pHUD->SetButtonInfo(8, L"FORTIFY UNIT\n \nHave this unit fortify his position mode. Offers combat bonuses that accumulate over the next few turns.\n \nShortcut: F");
				}
			}
			pHUD->SetButtonListener(8, CHUD::Fortify);
			pHUD->SetButtonColor(8, Color(0, 0, 150));
		}

		if ((CanAimMobilized() || IsFortified()) && !m_bFiredWeapon)
		{
			pHUD->SetButtonListener(2, CHUD::Aim);

			if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_AIM)
				pHUD->SetButtonColor(2, Color(150, 0, 0));
			else
				pHUD->SetButtonColor(2, Color(100, 100, 100));

			pHUD->SetButtonTexture(2, s_iAimIcon);

			if (DigitanksGame()->GetControlMode() == MODE_AIM)
				pHUD->SetButtonTexture(2, s_iCancelIcon);

			std::wstringstream s;
			if (IsInfantry())
				s << L"AIM AND FIRE MOUNTED GUN\n \nClick to enter Aim mode. Right click any spot on the terrain to fire on that location.";
			else if (IsScout())
				s << L"AIM AND FIRE TORPEDO\n \nClick to enter Aim mode. Right click any spot on the terrain to fire on that location.\n \nThe Torpedo damages supply lines, cutting units and structures off from their support. It doesn't do any physical damage to structures or units.";
			else
				s << L"AIM AND FIRE CANON\n \nClick to enter Aim mode. Right click any spot on the terrain to fire on that location.";

			if (IsScout() && m_flTotalPower < CScout::TorpedoAttackPower() || m_flTotalPower < 1)
			{
				pHUD->SetButtonColor(2, glgui::g_clrBox);
				s << "\n \nNOT ENOUGH ENERGY";

				pHUD->SetButtonListener(2, NULL);
			}

			s << "\n \nShortcut: E";

			pHUD->SetButtonInfo(2, s.str().c_str());
		}

		if (GetFrontShieldMaxStrength() > 0)
		{
			pHUD->SetButtonListener(3, CHUD::Fire);

			if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_FIRE)
				pHUD->SetButtonColor(3, Color(150, 0, 150));
			else
				pHUD->SetButtonColor(3, Color(100, 100, 100));

			if (DigitanksGame()->GetControlMode() == MODE_FIRE)
				pHUD->SetButtonTexture(3, s_iCancelIcon);
			else
				pHUD->SetButtonTexture(3, s_iEnergyIcon);
			pHUD->SetButtonInfo(3, L"SET UNIT ENERGY\n \nChoose how this unit's energy is split between attacking energy and shield energy.\n \nShortcut: R");
		}

		if (HasBonusPoints())
		{
			pHUD->SetButtonListener(4, CHUD::Promote);
			pHUD->SetButtonTexture(4, s_iPromoteIcon);
			pHUD->SetButtonColor(4, glgui::g_clrBox);
			pHUD->SetButtonInfo(4, L"UPGRADE UNIT\n \nThis unit has upgrades available. Click to open the upgrades menu.\n \nShortcut: T");
		}
	}
	else if (eMenuMode == MENUMODE_PROMOTE)
	{
		if (!IsScout())
		{
			pHUD->SetButtonListener(0, CHUD::PromoteAttack);
			pHUD->SetButtonTexture(0, s_iPromoteAttackIcon);
			pHUD->SetButtonColor(0, Color(150, 150, 150));

			std::wstringstream s1;
			s1 << "UPGRADE ATTACK ENERGY\n \n"
				<< "This upgrade amplifies your tank's arsenal, increasing the maximum Attack Energy available to your tank past its normal levels. With greater Attack Energy, this tank's shells will deal more damage.\n \n"
				<< "Attack Energy increase: 10%\n \n"
				<< "Shortcut: Q";
			pHUD->SetButtonInfo(0, s1.str().c_str());
		}

		if (!IsArtillery() && !IsScout())
		{
			pHUD->SetButtonListener(1, CHUD::PromoteDefense);
			pHUD->SetButtonTexture(1, s_iPromoteDefenseIcon);
			pHUD->SetButtonColor(1, Color(150, 150, 150));

			std::wstringstream s;
			s << "UPGRADE DEFENSE ENERGY\n \n"
				<< "This upgrade strengthens your tank's shield generator, increasing the maximum Defense Energy available to your tank past its normal levels. As a result, your tank's shields will take more damage before they fail.\n \n"
				<< "Defense Energy increase: 10%\n \n"
				<< "Shortcut: W";
			pHUD->SetButtonInfo(1, s.str().c_str());
		}

		pHUD->SetButtonListener(2, CHUD::PromoteMovement);
		pHUD->SetButtonTexture(2, s_iPromoteMoveIcon);
		pHUD->SetButtonColor(2, Color(150, 150, 150));

		std::wstringstream s2;
		s2 << "UPGRADE MOVEMENT ENERGY\n \n"
			<< "This upgrade overclocks your tank's engines, increasing the maximum Movement Energy available to your tank past its normal levels. With this you'll spend less energy moving your tank around.\n \n"
			<< "Movement Energy increase: 10%\n \n"
			<< "Shortcut: E";
		pHUD->SetButtonInfo(2, s2.str().c_str());

		pHUD->SetButtonListener(9, CHUD::GoToMain);
		pHUD->SetButtonTexture(9, s_iCancelIcon);
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, L"RETURN\n \nShortcut: G");
	}
}

void CDigitank::Fire()
{
	float flDistanceSqr = (m_vecPreviewAim - GetOrigin()).LengthSqr();
	if (flDistanceSqr > GetMaxRange()*GetMaxRange())
		return;

	if (flDistanceSqr < GetMinRange()*GetMinRange())
		return;

	if (fabs(AngleDifference(GetAngles().y, VectorAngles((GetPreviewAim()-GetOrigin()).Normalized()).y)) > FiringCone())
		return;

	if (m_bFiredWeapon)
		return;

	if (m_flTotalPower < 0.5f)
		return;

	if (CanFortify() && !IsFortified() && !CanAimMobilized())
		return;

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(NETWORK_TOEVERYONE, "Fire", GetHandle(), m_vecPreviewAim.x, m_vecPreviewAim.y, m_vecPreviewAim.z);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = m_vecPreviewAim.x;
	p.fl3 = m_vecPreviewAim.y;
	p.fl4 = m_vecPreviewAim.z;
	Fire(&p);
}

void CDigitank::Fire(CNetworkParameters* p)
{
	m_vecPreviewAim = Vector(p->fl2, p->fl3, p->fl4);

	m_vecLastAim = m_vecPreviewAim;
	m_bFiredWeapon = true;

	float flAttackPower = m_flTotalPower * m_flAttackSplit;

	if (CNetwork::IsHost())
	{
		m_flTotalPower -= flAttackPower;
		m_flAttackPower += flAttackPower;
	}

	if (GetVisibility() > 0)
	{
		EmitSound("sound/tank-aim.wav");
		SetSoundVolume("sound/tank-aim.wav", 0.5f);
	}

	if (CNetwork::IsHost())
	{
		m_flFireProjectileTime = GameServer()->GetGameTime() + FirstProjectileTime();
		m_iFireProjectiles = ProjectileCount();
	}

	Speak(TANKSPEECH_ATTACK);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	CDigitanksWindow::Get()->GetHUD()->UpdateTurnButton();

	if (IsArtillery())
		CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_FIRE_ARTILLERY, true);
}

void CDigitank::FireProjectile()
{
	float flDistanceSqr = (m_vecLastAim.Get() - GetOrigin()).LengthSqr();
	if (flDistanceSqr > GetMaxRange()*GetMaxRange())
		return;

	if (flDistanceSqr < GetMinRange()*GetMinRange())
		return;

	float flDistance = (m_vecLastAim.Get() - GetOrigin()).Length();
	Vector vecLandingSpot = m_vecLastAim.Get();

	float flFactor = 1;
	if (flDistance > GetEffRange())
		flFactor = RemapVal(flDistance, GetEffRange(), GetMaxRange(), 1, MaxRangeRadius());

	float flYaw = RandomFloat(0, 360);
	float flRadius = RandomFloat(1, flFactor);

	// Don't use uniform distribution, I like how it's clustered on the target.
	vecLandingSpot += Vector(flRadius*cos(flYaw), 0, flRadius*sin(flYaw));

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	m_hProjectile = CreateProjectile();

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(NETWORK_TOCLIENTS, "FireProjectile", GetHandle(), m_hProjectile->GetHandle(), vecLandingSpot.x, vecLandingSpot.y, vecLandingSpot.z);

	DigitanksGame()->AddProjectileToWaitFor();

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

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
}

CProjectile* CDigitank::CreateProjectile()
{
	return GameServer()->Create<CShell>("CShell");
}

float CDigitank::GetProjectileDamage()
{
	return GetAttackPower();
}

void CDigitank::ClientUpdate(int iClient)
{
	BaseClass::ClientUpdate(iClient);
}

void CDigitank::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit)
{
	if (flDamage > 0)
	{
		CancelGoalMovePosition();
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_AUTOMOVECANCELED);
	}
	else
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_UNITDAMAGED);

	size_t iTutorial = CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial();
	if (iTutorial == CInstructor::TUTORIAL_FINISHHIM)
	{
		// BOT MUST DIE
		if (GetDigitanksTeam() != DigitanksGame()->GetLocalDigitanksTeam())
			flDamage += 50;
	}

	Speak(TANKSPEECH_DAMAGED);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	size_t iDifficulty = DigitanksGame()->GetDifficulty();

	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);

	if (!CNetwork::IsConnected() && iDifficulty == 0)
	{
		if (DigitanksGame()->IsTeamControlledByMe(GetTeam()))
			flDamage *= 0.5f;
		else if (dynamic_cast<CDigitank*>(pAttacker) && DigitanksGame()->IsTeamControlledByMe(dynamic_cast<CDigitank*>(pAttacker)->GetTeam()))
			flDamage *= 2.0f;
	}

	Vector vecAttackOrigin;
	if (bDirectHit)
	{
		if (pAttacker)
			vecAttackOrigin = pAttacker->GetOrigin();
	}
	else if (pInflictor)
		vecAttackOrigin = pInflictor->GetOrigin();

	Vector vecAttackDirection = (vecAttackOrigin - GetOrigin()).Normalized();

	float flShield = GetShieldValueForAttackDirection(vecAttackDirection);

	float flDamageBlocked = flShield * GetDefenseScale();

	if (pProjectile)
		flDamageBlocked *= pProjectile->ShieldDamageScale();

	if (flDamage - flDamageBlocked <= 0)
	{
		SetShieldValueForAttackDirection(vecAttackDirection, flShield - flDamage / GetDefenseScale());

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

	if (flShield > 1.0f)
	{
		if (GetVisibility() > 0)
		{
			EmitSound("sound/shield-damage.wav");
			SetSoundVolume("sound/shield-damage.wav", RemapValClamped(flShield, 0, 5, 0, 1));
		}
	}

	SetShieldValueForAttackDirection(vecAttackDirection, 0);

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
		pKiller->m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}

	if (m_hSupplyLine != NULL)
		m_hSupplyLine->Delete();

	// Make sure we can see that we got a promotion.
	CDigitanksWindow::Get()->GetHUD()->Layout();
}

Vector CDigitank::GetOrigin() const
{
	float flTransitionTime = GetTransitionTime();

	if (GetVisibility() == 0)
		flTransitionTime = 0;

	float flTimeSinceMove = GameServer()->GetGameTime() - m_flStartedMove;
	if (m_flStartedMove && flTimeSinceMove < flTransitionTime)
	{
		float flLerp = SLerp(RemapVal(flTimeSinceMove, 0, flTransitionTime, 0, 1), 0.2f);
		Vector vecNewOrigin = m_vecPreviousOrigin.Get() * (1-flLerp) + BaseClass::GetOrigin() * flLerp;
		vecNewOrigin.y = FindHoverHeight(vecNewOrigin);
		return vecNewOrigin;
	}

	return BaseClass::GetOrigin();
}

EAngle CDigitank::GetAngles() const
{
	float flTransitionTime = GetTransitionTime();

	if (GetVisibility() == 0)
		flTransitionTime = 0;

	float flTimeSinceTurn = GameServer()->GetGameTime() - m_flStartedTurn;
	if (m_flStartedTurn && flTimeSinceTurn < flTransitionTime)
	{
		float flLerp = SLerp(RemapVal(flTimeSinceTurn, 0, flTransitionTime, 0, 1), 0.2f);
		float flAngleDiff = AngleDifference(BaseClass::GetAngles().y, m_flPreviousTurn.Get());
		float flNewTurn = m_flPreviousTurn.Get() + flAngleDiff * flLerp;
		return EAngle(0, flNewTurn, 0);
	}

	return BaseClass::GetAngles();
}

Vector CDigitank::GetRenderOrigin() const
{
	float flLerp = 0;
	
	if (!IsFortified() && !IsFortifying())
	{
		float flScale = fabs(fmod(GameServer()->GetGameTime()+m_flBobOffset, 4)-2)/2;
		flLerp = SLerp(flScale, 0.2f);
	}

	return GetOrigin() + Vector(0, 1 + flLerp*BobHeight(), 0);
}

EAngle CDigitank::GetRenderAngles() const
{
	if (GetDigitanksTeam()->IsPrimarySelection(this))
	{
		if (DigitanksGame()->GetControlMode() == MODE_TURN && GetPreviewTurnPower() <= GetTotalMovementPower())
			return EAngle(0, GetPreviewTurn(), 0);
	}

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
	{
		if (TurnsWith(DigitanksGame()->GetPrimarySelectionTank()))
		{
			Vector vecLookAt;
			bool bMouseOK = CDigitanksWindow::Get()->GetMouseGridPosition(vecLookAt);
			bool bNoTurn = bMouseOK && (vecLookAt - DigitanksGame()->GetPrimarySelectionTank()->GetOrigin()).LengthSqr() < 3*3;

			if (!bNoTurn && bMouseOK)
			{
				Vector vecDirection = (vecLookAt - GetOrigin()).Normalized();
				float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

				float flTankTurn = AngleDifference(flYaw, GetAngles().y);
				if (GetPreviewMovePower() + fabs(flTankTurn)/TurnPerPower() > GetTotalMovementPower())
					flTankTurn = (flTankTurn / fabs(flTankTurn)) * (GetTotalMovementPower() - GetPreviewMovePower()) * TurnPerPower() * 0.95f;

				return EAngle(0, GetAngles().y + flTankTurn, 0);
			}
		}
	}

	return EAngle(0, GetAngles().y, 0);
}

void CDigitank::PreRender()
{
#ifdef _DEBUG
	if (HasFortifyPoint())
	{
		CRenderingContext r(GameServer()->GetRenderer());
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

	if (GetFrontShieldStrength() > 0 && !(IsFortifying() || IsFortified()))
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

	CRenderingContext r(GameServer()->GetRenderer());
	r.SetAlpha(flAlpha*GetVisibility());
	r.SetBlend(BLEND_ALPHA);
	r.Translate(Vector(-0.527677f, 0.810368f, 0));

	if ((GetDigitanksTeam()->IsSelected(this) && DigitanksGame()->GetControlMode() == MODE_AIM) || m_bFiredWeapon)
	{
		Vector vecAimTarget;
		if (GetDigitanksTeam()->IsSelected(this) && DigitanksGame()->GetControlMode() == MODE_AIM)
			vecAimTarget = GetPreviewAim();
		else
			vecAimTarget = m_vecLastAim.Get();
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

	CRenderingContext r(GameServer()->GetRenderer());
	r.SetAlpha(flAlpha*GetVisibility());
	r.Rotate(flAngle, Vector(0, 1, 0));
	r.SetBlend(BLEND_ADDITIVE);
	r.Scale(RenderShieldScale(), 1, RenderShieldScale());
	r.RenderModel(m_iShieldModel);
}

void CDigitank::PostRender()
{
	CSelectable* pCurrentSelection = DigitanksGame()->GetPrimarySelection();
	CDigitank* pCurrentTank = DigitanksGame()->GetPrimarySelectionTank();

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
	{
		EAngle angTurn = EAngle(0, GetAngles().y, 0);
		if (TurnsWith(pCurrentTank))
		{
			Vector vecLookAt;
			bool bMouseOK = CDigitanksWindow::Get()->GetMouseGridPosition(vecLookAt);
			bool bNoTurn = bMouseOK && (vecLookAt - DigitanksGame()->GetPrimarySelectionTank()->GetOrigin()).LengthSqr() < 3*3;

			if (!bNoTurn && bMouseOK)
			{
				Vector vecDirection = (vecLookAt - GetOrigin()).Normalized();
				float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

				float flTankTurn = AngleDifference(flYaw, GetAngles().y);
				if (GetPreviewMovePower() + fabs(flTankTurn)/TurnPerPower() > GetTotalMovementPower())
					flTankTurn = (flTankTurn / fabs(flTankTurn)) * (GetTotalMovementPower() - GetPreviewMovePower()) * TurnPerPower() * 0.95f;

				angTurn = EAngle(0, GetAngles().y + flTankTurn, 0);
			}

			CRenderingContext r(GameServer()->GetRenderer());
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
			if (GetDigitanksTeam()->IsSelected(this) && MovesWith(pCurrentTank))
			{
				Vector vecTankMove = pCurrentTank->GetPreviewMove() - pCurrentTank->GetOrigin();
				if (vecTankMove.Length() > GetMaxMovementDistance())
					vecTankMove = vecTankMove.Normalized() * GetTotalMovementPower() * 0.95f;

				Vector vecNewPosition = GetOrigin() + vecTankMove;
				vecNewPosition.y = FindHoverHeight(vecNewPosition);

				CRenderingContext r(GameServer()->GetRenderer());
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
		if (GetDigitanksTeam()->IsSelected(this) && DigitanksGame()->GetControlMode() == MODE_AIM)
			iAlpha = 255;

		Vector vecTankOrigin = GetOrigin();

		float flGravity = DigitanksGame()->GetGravity();
		float flTime;
		Vector vecForce;
		FindLaunchVelocity(vecTankOrigin, m_vecDisplayAim, flGravity, vecForce, flTime, ProjectileCurve());

		CRopeRenderer oRope(GameServer()->GetRenderer(), s_iAimBeam, vecTankOrigin);
		oRope.SetWidth(0.5f);
		oRope.SetColor(Color(255, 0, 0, iAlpha));
		oRope.SetTextureScale(50);
		oRope.SetTextureOffset(-fmod(GameServer()->GetGameTime(), 1));

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
		CRenderingContext r(GameServer()->GetRenderer());
		r.SetBlend(BLEND_ALPHA);

		CRopeRenderer oRope(GameServer()->GetRenderer(), s_iAutoMove, GetOrigin());
		oRope.SetWidth(2.0f);
		oRope.SetColor(Color(255, 255, 255, iAlpha));
		oRope.SetTextureScale(3);
		oRope.SetTextureOffset(-fmod(GameServer()->GetGameTime(), 1));

		Vector vecPath = vecGoalMove - GetOrigin();
		vecPath.y = 0;

		float flDistance = vecPath.Length2D();
		Vector vecDirection = vecPath.Normalized();
		size_t iSegments = (size_t)(flDistance/3);

		for (size_t i = 0; i < iSegments; i++)
		{
			float flCurrentDistance = ((float)i*flDistance)/iSegments;
			oRope.AddLink(DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + vecDirection*flCurrentDistance) + Vector(0, 1, 0));
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
		CNetwork::CallFunction(NETWORK_TOCLIENTS, "TankPromoted", GetHandle());
	}

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	Speak(TANKSPEECH_PROMOTED);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
}

void CDigitank::PromoteAttack()
{
	if (!CNetwork::IsHost())
	{
		CNetwork::CallFunction(NETWORK_TOSERVER, "PromoteAttack", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusAttackPower++;

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	if (GetTeam()->IsPlayerControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::PromoteDefense()
{
	if (!CNetwork::IsHost())
	{
		CNetwork::CallFunction(NETWORK_TOSERVER, "PromoteDefense", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusDefensePower++;

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	if (GetTeam()->IsPlayerControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::PromoteMovement()
{
	if (!CNetwork::IsHost())
	{
		CNetwork::CallFunction(NETWORK_TOSERVER, "PromoteMovement", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusMovementPower++;

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	if (GetTeam()->IsPlayerControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
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

	if (GameServer()->GetGameTime() < m_flLastSpeech + 5.0f)
		return;

	if (GetVisibility() == 0)
		return;

	size_t iLine = g_aiSpeechLines[iSpeech][rand()%g_aiSpeechLines[iSpeech].size()];

	m_flLastSpeech = GameServer()->GetGameTime();
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "TankSpeak", GetHandle(), iLine);

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
	if (!DigitanksGame())
		return vecPosition.y;

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

	return LineSegmentIntersectsSphere(v1, v2, GetOrigin(), GetBoundingRadius(), vecPoint);
}

float CDigitank::HealthRechargeRate() const
{
	return 0.2f + GetSupportHealthRechargeBonus();
}

float CDigitank::ShieldRechargeRate() const
{
	return 0.2f + GetSupportShieldRechargeBonus();
}

float CDigitank::FirstProjectileTime() const
{
	return RandomFloat(0, 1);
}

float CDigitank::FireProjectileTime() const
{
	return RandomFloat(0.25f, 0.3f);
}

void CDigitank::SetFortifyPoint(Vector vecFortify)
{
	m_bFortifyPoint = true;
	m_vecFortifyPoint = vecFortify;
}
