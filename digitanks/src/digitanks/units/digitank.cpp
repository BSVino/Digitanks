#include "digitank.h"

#include <sstream>

#include <maths.h>
#include <mtrand.h>
#include <strutils.h>
#include <models/models.h>
#include <renderer/game_renderer.h>
#include <renderer/particles.h>
#include <glgui/glgui.h>
#include <renderer/shaders.h>
#include <textures/texturelibrary.h>
#include <network/commands.h>
#include <sound/sound.h>
#include <renderer/game_renderingcontext.h>
#include <glgui/rootpanel.h>

#include <game/networkedeffect.h>
#include <digitanksgame.h>
#include <dt_camera.h>
#include "ui/digitankswindow.h"
#include "ui/instructor.h"
#include <powerup.h>
#include "ui/hud.h"
#include <structures/structure.h>
#include <supplyline.h>
#include <weapons/projectile.h>
#include <weapons/cameraguided.h>
#include <weapons/laser.h>
#include <weapons/missiledefense.h>
#include <units/scout.h>
#include <campaign/userfile.h>
#include <wreckage.h>

// Don't feel like changing all these.
#define _T(x) x

CMaterialHandle CDigitank::s_hAimBeam;
CMaterialHandle CDigitank::s_hAutoMove;
CMaterialHandle CDigitank::s_hSupportGlow;

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
	":P",	// TANKLINE_TONGUE
	":3",	// TANKLINE_CATFACE
};

tmap<size_t, tvector<size_t> > g_aiSpeechLines;

REGISTER_ENTITY(CDigitank);

NETVAR_TABLE_BEGIN(CDigitank);
	NETVAR_DEFINE(float, m_flStartingPower);
	NETVAR_DEFINE(float, m_flTotalPower);
	NETVAR_DEFINE(float, m_flAttackPower);
	NETVAR_DEFINE(float, m_flDefensePower);
	NETVAR_DEFINE(float, m_flMovementPower);

	NETVAR_DEFINE(float, m_flBonusAttackPower);
	NETVAR_DEFINE(float, m_flBonusDefensePower);
	NETVAR_DEFINE(float, m_flBonusMovementPower);
	NETVAR_DEFINE_CALLBACK(size_t, m_iBonusPoints, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(size_t, m_iBonusLevel);
	NETVAR_DEFINE(float, m_flRangeBonus);

	NETVAR_DEFINE(float, m_flShieldStrength);

	NETVAR_DEFINE_CALLBACK(bool, m_bNeedsOrders, &CDigitanksGame::UpdateHUD);

	NETVAR_DEFINE(float, m_flGoalTurretYaw);

	NETVAR_DEFINE(float, m_flStartedRock);
	NETVAR_DEFINE(float, m_flRockIntensity);
	NETVAR_DEFINE(Vector, m_vecRockDirection);

	NETVAR_DEFINE(Vector, m_vecPreviousOrigin);
	NETVAR_DEFINE(float, m_flStartedMove);
	NETVAR_DEFINE(int, m_iMoveType);

	NETVAR_DEFINE(float, m_flPreviousTurn);
	NETVAR_DEFINE(float, m_flStartedTurn);

	NETVAR_DEFINE(Vector, m_vecLastAim);

	NETVAR_DEFINE(float, m_flBeginCharge);
	NETVAR_DEFINE(float, m_flEndCharge);

	NETVAR_DEFINE_CALLBACK(bool, m_bHasCloak, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(bool, m_bCloaked);

	NETVAR_DEFINE_CALLBACK(bool, m_bGoalMovePosition, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(Vector, m_vecGoalMovePosition);

	NETVAR_DEFINE(bool, m_bFiredWeapon);
	NETVAR_DEFINE(bool, m_bActionTaken);
	NETVAR_DEFINE_CALLBACK(bool, m_bLostConcealment, &CDigitanksGame::UpdateHUD);

	NETVAR_DEFINE(float, m_flShieldPulse);

	NETVAR_DEFINE_CALLBACK(bool, m_bFortified, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(size_t, m_iFortifyLevel);
	NETVAR_DEFINE(float, m_flFortifyTime);

	NETVAR_DEFINE_CALLBACK(bool, m_bSentried, &CDigitanksGame::UpdateHUD);

	NETVAR_DEFINE_CALLBACK(weapon_t, m_eWeapon, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(tvector<weapon_t>, m_aeWeapons, &CDigitanksGame::UpdateHUD);

	NETVAR_DEFINE_CALLBACK(size_t, m_iAirstrikes, &CDigitanksGame::UpdateHUD);

	NETVAR_DEFINE(size_t, m_iTurnsDisabled);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitank);
	SAVEDATA_DEFINE_OUTPUT(OnTakeShieldDamage);
	SAVEDATA_DEFINE_OUTPUT(OnTakeLaserDamage);
	SAVEDATA_DEFINE_OUTPUT(OnDisable);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flStartingPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flTotalPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flAttackPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flDefensePower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flMovementPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBonusAttackPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBonusDefensePower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBonusMovementPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iBonusPoints);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iBonusLevel);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flRangeBonus);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flMaxShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bNeedsOrdersDirty);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bNeedsOrders);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flCurrentTurretYaw);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flGoalTurretYaw);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flStartedRock);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flRockIntensity);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecRockDirection);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecPreviewMove);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecPreviousOrigin);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flStartedMove);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_iMoveType);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flPreviewTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flPreviousTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flStartedTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bPreviewAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecPreviewAim);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecLastAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bDisplayAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecDisplayAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flDisplayAimRadius);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<class CBaseEntity>, m_hPreviewCharge);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBeginCharge);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flEndCharge);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<class CBaseEntity>, m_hChargeTarget);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bHasCloak);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCloaked);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bGoalMovePosition);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecGoalMovePosition);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bFiredWeapon);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bActionTaken);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bLostConcealment);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flFireWeaponTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iFireWeapons);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CDigitanksWeapon>, m_hWeapon);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLastSpeech);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flNextIdle);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, size_t, m_iTurretModel);	// Set in Spawn()
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, size_t, m_iShieldModel);	// Set in Spawn()
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flShieldPulse);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hHoverParticles);	// Dynamic
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hSmokeParticles);	// Dynamic
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hFireParticles);		// Dynamic
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hChargeParticles);	// Dynamic
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bFortified);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iFortifyLevel);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flFortifyTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bStayPut);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bSentried);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flBobOffset);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, weapon_t, m_eWeapon);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, weapon_t, m_aeWeapons);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iAirstrikes);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iMissileDefenses);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flNextMissileDefense);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTurnsDisabled);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flGlowYaw);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flNextHoverHeightCheck);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bInAttackTeam);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bFortifyPoint);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecFortifyPoint);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<class CStructure>, m_hFortifyDefending);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CDigitank);
INPUTS_TABLE_END();

void CDigitank::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem("tank-fire");
	PrecacheParticleSystem("promotion");
	PrecacheParticleSystem("tank-hover");
	PrecacheParticleSystem("digitank-smoke");
	PrecacheParticleSystem("digitank-fire");
	PrecacheParticleSystem("charge-burst");
	PrecacheParticleSystem("charge-charge");
	PrecacheSound("sound/tank-fire.wav");
	PrecacheSound("sound/shield-damage.wav");
	PrecacheSound("sound/tank-damage.wav");
	PrecacheSound("sound/tank-active.wav");
	PrecacheSound("sound/tank-active2.wav");
	PrecacheSound("sound/tank-move.wav");
	PrecacheSound("sound/tank-aim.wav");
	PrecacheSound("sound/tank-promoted.wav");

	s_hAimBeam = CMaterialLibrary::AddMaterial("textures/beam-pulse.mat");
	s_hAutoMove = CMaterialLibrary::AddMaterial("textures/auto-move.mat");
	s_hSupportGlow = CMaterialLibrary::AddMaterial("textures/particles/support.mat");

	SetupSpeechLines();
}

void CDigitank::SetupSpeechLines()
{
	if (g_aiSpeechLines.size())
		return;

	g_aiSpeechLines[TANKSPEECH_SELECTED].push_back(TANKLINE_CUTE);
	g_aiSpeechLines[TANKSPEECH_SELECTED].push_back(TANKLINE_HAPPY);
	g_aiSpeechLines[TANKSPEECH_SELECTED].push_back(TANKLINE_COOL);
	g_aiSpeechLines[TANKSPEECH_SELECTED].push_back(TANKLINE_CATFACE);
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
	g_aiSpeechLines[TANKSPEECH_PROMOTED].push_back(TANKLINE_CATFACE);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_HAPPY);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_LOVE);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_CUTE);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_COOL);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_THRILLED);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_CHEER);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_CATFACE);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_CUTE);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_CHEER);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_EVIL);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_COOL);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_SURPRISED);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_THRILLED);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_TONGUE);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_CATFACE);
	g_aiSpeechLines[TANKSPEECH_DISABLED].push_back(TANKLINE_ASLEEP);
	g_aiSpeechLines[TANKSPEECH_DISABLED].push_back(TANKLINE_SQUINT);
	g_aiSpeechLines[TANKSPEECH_DISABLED].push_back(TANKLINE_DEAD);
	g_aiSpeechLines[TANKSPEECH_DISABLED].push_back(TANKLINE_DEAD2);
	g_aiSpeechLines[TANKSPEECH_DISABLED].push_back(TANKLINE_DEAD3);
}

void CDigitank::Spawn()
{
	m_flStartingPower = 10;
	m_flAttackPower = 0;
	m_flDefensePower = 10;
	m_flMovementPower = 0;
	m_flTotalPower = 0;
	m_flBonusAttackPower = m_flBonusDefensePower = m_flBonusMovementPower = 0;
	m_flRangeBonus = 0;
	m_iBonusPoints = 0;
	m_iBonusLevel = 0;
	m_flPreviewTurn = 0;
	m_bPreviewAim = false;
	m_bGoalMovePosition = false;
	m_bFiredWeapon = false;
	m_bActionTaken = false;
	m_flStartedMove = 0;
	m_flStartedTurn = 0;
	m_bFortified = false;
	m_bStayPut = false;
	m_bSentried = false;
	m_flMaxShieldStrength = 150;
	m_flShieldStrength = 150;
	m_bNeedsOrdersDirty = true;
	m_bNeedsOrders = true;
	m_flFireWeaponTime = 0;
	m_iFireWeapons = 0;
	m_flLastSpeech = 0;
	m_flNextIdle = 10.0f;
	m_iTurretModel = m_iShieldModel = ~0;
	m_bInAttackTeam = false;
	m_bFortifyPoint = false;
	m_flFortifyTime = 0;
	m_flBobOffset = RandomFloat(0, 10);
	m_flStartedRock = -100;
	m_flBeginCharge = -1;
	m_flEndCharge = -1;
	m_iAirstrikes = 0;
	m_iMissileDefenses = 0;
	m_flNextMissileDefense = 0;
	m_iTurnsDisabled = 0;
	m_flCurrentTurretYaw = 0;
	m_flGoalTurretYaw = 0;
	m_flGlowYaw = 0;
	m_flNextHoverHeightCheck = GameServer()->GetGameTime() + RandomFloat(0, 1);
	m_flShieldPulse = 0;
	m_bCloaked = false;
	m_bHasCloak = false;
	m_bLostConcealment = false;

	BaseClass::Spawn();

	m_hHoverParticles.SetSystem("tank-hover", GetGlobalOrigin());
	m_hHoverParticles.FollowEntity(this);

	m_hSmokeParticles.SetSystem("digitank-smoke", GetGlobalOrigin());
	m_hSmokeParticles.FollowEntity(this);

	m_hFireParticles.SetSystem("digitank-fire", GetGlobalOrigin());
	m_hFireParticles.FollowEntity(this);

	m_hChargeParticles.SetSystem("charge-charge", GetGlobalOrigin());
	m_hChargeParticles.FollowEntity(this);
}

float CDigitank::GetBaseAttackPower(bool bPreview)
{
	if (GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer() && GetDigitanksPlayer()->IsSelected(this) && bPreview && DigitanksGame()->GetControlMode() == MODE_AIM)
		return GetWeaponEnergy();

	return m_flAttackPower;
}

float CDigitank::GetBaseDefensePower(bool bPreview) const
{
	if (GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer() && GetDigitanksPlayer()->IsSelected(this) && bPreview && DigitanksGame()->GetControlMode() == MODE_AIM)
		return m_flTotalPower - GetWeaponEnergy();

	// Any unallocated power will go into defense.
	return m_flDefensePower + m_flTotalPower;
}

float CDigitank::GetAttackPower(bool bPreview)
{
	return GetBaseAttackPower(bPreview) + m_flBonusAttackPower;
}

float CDigitank::GetDefensePower(bool bPreview) const
{
	Vector vecOrigin = GetGlobalOrigin();

	if (bPreview)
		vecOrigin = m_vecPreviewMove;

	if (IsDisabled())
		return 0;

	if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(vecOrigin.x), CTerrain::WorldToArraySpace(vecOrigin.y), TB_WATER))
		return 0;

	float flDefenseBonus = 0;
	if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(vecOrigin.x), CTerrain::WorldToArraySpace(vecOrigin.y), TB_TREE))
		flDefenseBonus = 5;

	return GetBaseDefensePower(bPreview)+GetBonusDefensePower(bPreview)+flDefenseBonus;
}

float CDigitank::GetTotalAttackPower()
{
	return m_flStartingPower + m_flBonusAttackPower;
}

float CDigitank::GetTotalDefensePower()
{
	return m_flStartingPower + GetBonusDefensePower();
}


float CDigitank::GetMaxMovementEnergy() const
{
	float flNetworkBonus = 0;

	if (CSupplier::GetDataFlow(GetGlobalOrigin(), GetPlayerOwner()) > 0)
		flNetworkBonus = 4.0f;

	return GetStartingPower() + GetBonusMovementEnergy() + flNetworkBonus;
}

float CDigitank::GetMaxMovementDistance() const
{
	float flDistance = GetMaxMovementEnergy() * GetTankSpeed();

	if (flDistance < 0)
		return 0;

	return flDistance;
}

float CDigitank::GetUsedMovementEnergy(bool bPreview) const
{
	if (bPreview)
	{
		float flPreviewPower = GetPreviewMoveTurnPower();
		float flRemainingPower = GetRemainingMovementEnergy();
		if (flPreviewPower > flRemainingPower)
			return GetMaxMovementEnergy();
		else
			return m_flMovementPower + flPreviewPower;
	}

	return m_flMovementPower;
}

float CDigitank::GetRemainingMovementEnergy(bool bPreview) const
{
	return GetMaxMovementEnergy() - GetUsedMovementEnergy(bPreview);
}

float CDigitank::GetRemainingMovementDistance() const
{
	float flDistance = GetRemainingMovementEnergy() * GetTankSpeed();

	if (flDistance < 0)
		return 0;

	return flDistance;
}

float CDigitank::GetRemainingTurningDistance() const
{
	return GetRemainingMovementEnergy() * TurnPerPower();
}

float CDigitank::GetBonusAttackDamage()
{
	return (m_flBonusAttackPower + GetSupportAttackPowerBonus() + GetFortifyAttackPowerBonus()) * 10;
}

float CDigitank::GetBonusDefensePower(bool bPreview) const
{
	return (m_flBonusDefensePower + GetSupportDefensePowerBonus() + GetFortifyDefensePowerBonus());
}

float CDigitank::GetSupportAttackPowerBonus()
{
	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetGlobalOrigin(), GetPlayerOwner()) > 0)
	{
		CSupplier* pSupplier = CSupplier::FindClosestSupplier(GetRealOrigin(), GetPlayerOwner());
		if (pSupplier)
			flBonus = (float)pSupplier->EnergyBonus();
	}

	return flBonus;
}

float CDigitank::GetSupportDefensePowerBonus() const
{
	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetGlobalOrigin(), GetPlayerOwner()) > 0)
	{
		CSupplier* pSupplier = CSupplier::FindClosestSupplier(GetRealOrigin(), GetPlayerOwner());
		if (pSupplier)
			flBonus = (float)pSupplier->EnergyBonus();
	}

	return flBonus;
}

float CDigitank::GetSupportHealthRechargeBonus() const
{
	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetGlobalOrigin(), GetPlayerOwner()) > 0)
	{
		CSupplier* pSupplier = CSupplier::FindClosestSupplier(GetRealOrigin(), GetPlayerOwner());
		if (pSupplier)
			flBonus = pSupplier->RechargeBonus();
	}

	return flBonus;
}

float CDigitank::GetSupportShieldRechargeBonus() const
{
	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetGlobalOrigin(), GetPlayerOwner()) > 0)
	{
		CSupplier* pSupplier = CSupplier::FindClosestSupplier(GetRealOrigin(), GetPlayerOwner());
		if (pSupplier)
			flBonus = pSupplier->RechargeBonus()*5;
	}

	return flBonus;
}

float CDigitank::GetPreviewMoveTurnPower() const
{
	float flMovePower = 0;
	float flTurnPower = 0;

	if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		flMovePower = GetPreviewBaseMovePower();

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
		flTurnPower = GetPreviewBaseTurnPower();

	float flPower = flMovePower + flTurnPower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewMovePower() const
{
	float flMovePower = 0;

	if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		flMovePower = GetPreviewBaseMovePower();

	float flPower = flMovePower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewTurnPower() const
{
	float flTurnPower = 0;

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
		flTurnPower = GetPreviewBaseTurnPower();

	float flPower = flTurnPower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewBaseMovePower() const
{
	bool bHalfMovement = false;
	if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(m_vecPreviewMove.x), CTerrain::WorldToArraySpace(m_vecPreviewMove.y), TB_TREE))
		bHalfMovement = true;
	if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(m_vecPreviewMove.x), CTerrain::WorldToArraySpace(m_vecPreviewMove.y), TB_WATER))
		bHalfMovement = true;

	if (bHalfMovement)
		return (m_vecPreviewMove - GetGlobalOrigin()).Length() / (GetTankSpeed()*SlowMovementFactor());
	else
		return (m_vecPreviewMove - GetGlobalOrigin()).Length() / GetTankSpeed();
}

float CDigitank::GetPreviewBaseTurnPower() const
{
	return fabs(AngleDifference(m_flPreviewTurn, GetGlobalAngles().y) / TurnPerPower());
}

bool CDigitank::IsPreviewMoveValid() const
{
	if (GetPreviewBaseMovePower() > GetRemainingMovementEnergy())
		return false;

	if (DigitanksGame()->GetTerrain()->IsPointOverHole(GetPreviewMove()))
		return false;

	return DigitanksGame()->GetTerrain()->IsPointOnMap(GetPreviewMove());
}

float CDigitank::GetShieldStrength() const
{
	if (GetShieldMaxStrength() == 0)
		return 0;

	return m_flShieldStrength/GetShieldMaxStrength() * GetDefenseScale(true);
}

float CDigitank::GetShieldBlockRadius()
{
	if (GetShieldMaxStrength() == 0)
		return GetBoundingRadius();

	if (m_flShieldStrength < 5)
		return GetBoundingRadius();

	return RenderShieldScale();
}

float CDigitank::GetShieldValue() const
{
	return m_flShieldStrength;
}

void CDigitank::SetShieldValue(float flValue)
{
	m_flShieldStrength = flValue;
}

void CDigitank::StartTurn()
{
	BaseClass::StartTurn();

	DirtyNeedsOrders();

	if (GameNetwork()->IsHost() && m_bFortified)
	{
		if (m_iFortifyLevel < 5)
			m_iFortifyLevel++;
	}

	m_vecPreviewMove = GetGlobalOrigin();
	m_flPreviewTurn = GetGlobalAngles().y;

	if (GameNetwork()->IsHost())
	{
		m_flTotalPower = m_flStartingPower;
		m_flMovementPower = m_flAttackPower = m_flDefensePower = 0;
	}

	m_bActionTaken = false;
	m_bFiredWeapon = false;
	m_bLostConcealment = false;

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	CDigitank* pClosestEnemy = FindClosestVisibleEnemyTank();

	if (!IsDisabled() && GetDigitanksPlayer())
	{
		if (HasGoalMovePosition() && pClosestEnemy)
			GetDigitanksPlayer()->AddActionItem(this, ACTIONTYPE_AUTOMOVEENEMY);
		else
		// Artillery gets unit orders even if fortified but infantry doesn't.
		if (!HasGoalMovePosition() && (!IsFortified() || IsArtillery()) && !IsSentried())
			GetDigitanksPlayer()->AddActionItem(this, ACTIONTYPE_UNITORDERS);
		else
		// Notify if infantry can see an enemy they can shoot.
		if (IsInfantry() && IsFortified())
		{
			CDigitank* pClosestEnemyTank = FindClosestVisibleEnemyTank(true);
			if (pClosestEnemyTank && pClosestEnemyTank->GetVisibility() > 0.3f)
				GetDigitanksPlayer()->AddActionItem(this, ACTIONTYPE_FORTIFIEDENEMY);
		}
	}

	if (HasGoalMovePosition())
		MoveTowardsGoalMovePosition();

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() == GetDigitanksPlayer())
	{
//		size_t iTutorial = DigitanksWindow()->GetInstructor()->GetCurrentTutorial();
//		if (iTutorial == CInstructor::TUTORIAL_ENTERKEY)
//			DigitanksWindow()->GetInstructor()->NextTutorial();
	}

	if (!DigitanksGame()->IsWeaponAllowed(GetCurrentWeapon(), this))
	{
		for (size_t i = 0; i < m_aeWeapons.size(); i++)
		{
			if (DigitanksGame()->IsWeaponAllowed(m_aeWeapons[i], this))
			{
				SetCurrentWeapon(m_aeWeapons[i]);
				break;
			}
		}
	}
}

void CDigitank::EndTurn()
{
	BaseClass::EndTurn();

	if (GameNetwork()->IsHost())
	{
		m_flDefensePower = m_flTotalPower;
		m_flTotalPower = 0;

		if (TakesLavaDamage() && DigitanksGame()->GetTerrain()->IsPointOverLava(GetGlobalOrigin()))
			TakeDamage(NULL, NULL, DAMAGE_BURN, DigitanksGame()->LavaDamage(), false);

		float flShieldStrength = GetShieldValue();
		SetShieldValue(Approach(m_flMaxShieldStrength, flShieldStrength, ShieldRechargeRate()));

		if (flShieldStrength - GetShieldValue() < 0)
			DigitanksGame()->OnTakeShieldDamage(this, NULL, NULL, (flShieldStrength - GetShieldValue())*GetDefenseScale(), true, false);
	}

	if (m_iTurnsDisabled)
	{
		DirtyNeedsOrders();
		m_iTurnsDisabled--;
	}
}

CDigitank* CDigitank::FindClosestVisibleEnemyTank(bool bInRange)
{
	CDigitank* pClosestEnemy = NULL;
	while (true)
	{
		pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(GetGlobalOrigin(), pClosestEnemy);

		if (!pClosestEnemy)
			break;

		if (pClosestEnemy->GetPlayerOwner() == GetPlayerOwner())
			continue;

		if (bInRange)
		{
			if (!IsInsideMaxRange(pClosestEnemy->GetGlobalOrigin()))
				return NULL;
		}
		else
		{
			if ((pClosestEnemy->GetGlobalOrigin() - GetGlobalOrigin()).Length() > VisibleRange()+DigitanksGame()->FogPenetrationDistance())
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
	m_vecPreviewMove = GetGlobalOrigin();
}

void CDigitank::SetPreviewTurn(float flPreviewTurn)
{
	if (IsFortified() && !CanTurnFortified())
		return;

	m_flPreviewTurn = flPreviewTurn;
}

void CDigitank::ClearPreviewTurn()
{
	m_flPreviewTurn = GetGlobalAngles().y;
}

void CDigitank::SetPreviewAim(Vector vecPreviewAim)
{
	if (CanFortify() && !IsFortified() && !CanAimMobilized())
		return;

	if (DigitanksGame()->GetAimType() == AIM_NORANGE)
	{
		// Special weapons can be aimed anywhere the player can see.
		m_vecPreviewAim = vecPreviewAim;
		m_bPreviewAim = GetDigitanksPlayer()->GetVisibilityAtPoint(m_vecPreviewAim) > 0;
		return;
	}

	m_bPreviewAim = true;

	vecPreviewAim = FindNearestTerrainPointInRange(vecPreviewAim);

	Vector vecRealDistance = vecPreviewAim - GetRealOrigin();
	if (vecRealDistance.LengthSqr() == 0)
	{
		m_bPreviewAim = false;
		return;
	}

	if (vecRealDistance.Length2DSqr() < GetMinRange()*GetMinRange())
	{
		vecPreviewAim = GetRealOrigin() + vecRealDistance.Normalized() * GetMinRange() * 1.01f;
		vecPreviewAim.z = DigitanksGame()->GetTerrain()->GetHeight(vecPreviewAim.x, vecPreviewAim.y);
	}

	if (fabs(AngleDifference(GetGlobalAngles().y, VectorAngles((vecPreviewAim - GetGlobalOrigin()).Normalized()).y)) > FiringCone())
	{
		m_bPreviewAim = false;
		return;
	}

	m_vecPreviewAim = vecPreviewAim;
}

void CDigitank::ClearPreviewAim()
{
	m_bPreviewAim = false;
	m_vecPreviewAim = GetGlobalOrigin();
}

bool CDigitank::IsPreviewAimValid()
{
	if (!m_bPreviewAim)
		return false;

	if (!IsInsideMaxRange(GetPreviewAim()))
		return false;

	if ((GetPreviewAim() - GetGlobalOrigin()).LengthSqr() < GetMinRange()*GetMinRange())
		return false;

	return true;
}

const Vector CDigitank::FindNearestTerrainPointInRange(const Vector& vecAim)
{
	TAssert(GetCurrentWeapon() != WEAPON_CHARGERAM && GetCurrentWeapon() != PROJECTILE_CAMERAGUIDED);

	if (IsInsideMaxRange(vecAim))
		return vecAim;

	Vector vecDirection = (vecAim - GetRealOrigin()).Normalized();

	Vector vecNear = GetRealOrigin();
	Vector vecFar = vecAim;

	Vector vecHalfway = DigitanksGame()->GetTerrain()->GetPointHeight((vecNear + vecFar) / 2);

	while ((vecNear - vecFar).LengthSqr() > 0.1f)
	{
		if (IsInsideMaxRange(vecHalfway))
			vecNear = vecHalfway;
		else
			vecFar = vecHalfway;

		vecHalfway = DigitanksGame()->GetTerrain()->GetPointHeight((vecNear + vecFar) / 2);
	}

	return vecNear;
}

bool CDigitank::CanCharge() const
{
	for (size_t i = 0; i < m_aeWeapons.size(); i++)
		if (m_aeWeapons[i] == WEAPON_CHARGERAM)
			return true;

	return false;
}

float CDigitank::ChargeRadius() const
{
	return BaseChargeRadius() * RemapValClamped(m_flMovementPower, 0, m_flStartingPower, 1.0f, 0.5f);
}

void CDigitank::SetPreviewCharge(CBaseEntity* pChargeTarget)
{
	if (!pChargeTarget)
	{
		m_hPreviewCharge = NULL;
		return;
	}

	if (dynamic_cast<CTerrain*>(pChargeTarget))
	{
		m_hPreviewCharge = NULL;
		return;
	}

	CDigitanksEntity* pDTEnt = dynamic_cast<CDigitanksEntity*>(pChargeTarget);
	if (pDTEnt && !pDTEnt->IsRammable())
		return;

	if (pChargeTarget->GetOwner() == GetPlayerOwner())
		return;

	if (pChargeTarget->Distance(GetGlobalOrigin()) > ChargeRadius())
		return;

	m_hPreviewCharge = pChargeTarget;
}

void CDigitank::ClearPreviewCharge()
{
	m_hPreviewCharge = NULL;
}

Vector CDigitank::GetChargePosition(CBaseEntity* pTarget) const
{
	float flTargetSize = pTarget->GetBoundingRadius() + GetBoundingRadius();
	Vector vecChargeDirection = (pTarget->GetGlobalOrigin() - GetGlobalOrigin()).Normalized();
	Vector vecChargePosition = pTarget->GetGlobalOrigin() - vecChargeDirection*flTargetSize;
	vecChargePosition.z = FindHoverHeight(vecChargePosition);
	return vecChargePosition;
}

bool CDigitank::IsInsideMaxRange(Vector vecPoint)
{
	if (GetCurrentWeapon() == WEAPON_CHARGERAM)
		return (vecPoint - GetRealOrigin()).LengthSqr() < ChargeRadius()*ChargeRadius();

	if (GetCurrentWeapon() == PROJECTILE_CAMERAGUIDED)
		return true;

	Vector vecDirection = vecPoint - GetRealOrigin();
	float flPreviewDistanceSqr = vecDirection.LengthSqr();
	float flPreviewDistance2DSqr = vecDirection.Length2DSqr();
	float flHeightToTank = vecDirection.z;
	if (flHeightToTank*flHeightToTank > ((flPreviewDistanceSqr/2) * (flPreviewDistanceSqr/2)))
	{
		if (flPreviewDistanceSqr > GetMaxRange()*GetMaxRange())
			return false;
		else
			return true;
	}
	else
	{
		float flMaxRange = GetMaxRange()*1.115f-(flHeightToTank/2);
		if (flPreviewDistance2DSqr > flMaxRange*flMaxRange)
			return false;
		else
			return true;
	}
}

float CDigitank::FindAimRadius(Vector vecPoint, float flMin)
{
	Vector vecDirection = vecPoint - GetGlobalOrigin();
	float flPreviewDistanceSqr = vecDirection.LengthSqr();
	float flPreviewDistance2DSqr = vecDirection.Length2DSqr();
	float flHeightToTank = vecDirection.z;
	if (flHeightToTank*flHeightToTank > ((flPreviewDistanceSqr/2) * (flPreviewDistanceSqr/2)))
		return RemapValClamped(flPreviewDistanceSqr, GetEffRange()*GetEffRange(), GetMaxRange()*GetMaxRange(), flMin, MaxRangeRadius());
	else
	{
		float flMaxRange = GetMaxRange()*1.115f-(flHeightToTank/2);
		float flEffRange = GetEffRange()*1.115f-(flHeightToTank/2);
		return RemapValClamped(flPreviewDistance2DSqr, flEffRange*flEffRange, flMaxRange*flMaxRange, flMin, MaxRangeRadius());
	}
}

void CDigitank::RockTheBoat(float flIntensity, Vector vecDirection)
{
	if (IsFortified() || IsFortifying())
		return;

	m_flStartedRock = GameServer()->GetGameTime();
	m_flRockIntensity = flIntensity;
	m_vecRockDirection = vecDirection;
}

bool CDigitank::IsRocking() const
{
	float flTransitionTime = 1;

	float flTimeSinceRock = (float)(GameServer()->GetGameTime() - m_flStartedRock);
	if (m_flStartedRock && flTimeSinceRock < flTransitionTime)
		return true;

	return false;
}

void CDigitank::Move()
{
	if (IsFortified() && !CanMoveFortified())
		return;

	if (!IsPreviewMoveValid())
		return;

	if (DigitanksGame()->GetTerrain()->IsPointOverHole(m_vecPreviewMove))
		return;

	if (!DigitanksGame()->GetTerrain()->IsPointOnMap(m_vecPreviewMove))
		return;

	if (IsDisabled())
		return;

	Speak(TANKSPEECH_MOVED);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	if (GameNetwork()->ShouldReplicateClientFunction())
		GameNetwork()->CallFunction(NETWORK_TOEVERYONE, "Move", GetHandle(), m_vecPreviewMove.x, m_vecPreviewMove.y, m_vecPreviewMove.z);

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

	Vector vecStart = GetGlobalOrigin();
	Vector vecEnd = m_vecPreviewMove;

	Move(m_vecPreviewMove);

	Turn(EAngle(0, VectorAngles(vecEnd-vecStart).y, 0));

	if (GetVisibility() > 0)
	{
		EmitSound("sound/tank-move.wav");
		SetSoundVolume("sound/tank-move.wav", 0.5f);

		m_hHoverParticles.SetActive(true);
	}

	// Am I waiting to fire something? Fire now. Shoot and scoot baby!
	if (m_flFireWeaponTime)
		m_flFireWeaponTime = GameServer()->GetGameTime();

	if (GameNetwork()->IsHost())
	{
		m_flMovementPower += flMovePower;
	}

	if (GameNetwork()->IsHost() && CanGetPowerups())
	{
		for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

			if (!pEntity)
				continue;

			CPowerup* pPowerup = dynamic_cast<CPowerup*>(pEntity);

			if (pPowerup)
			{
				Vector vecDistance = pPowerup->GetGlobalOrigin() - GetRealOrigin();
				vecDistance.y = 0;
				if (vecDistance.Length() < pPowerup->GetBoundingRadius() + GetBoundingRadius())
					pPowerup->Pickup(this);

				continue;
			}

			CUserFile* pUserFile = dynamic_cast<CUserFile*>(pEntity);
			Vector vecTouchingPoint;
			if (pUserFile && pUserFile->IsTouching(this, vecTouchingPoint))
				pUserFile->Pickup(this);

			CDigitanksEntity* pOtherEntity = dynamic_cast<CDigitanksEntity*>(pEntity);
			if (pOtherEntity && pOtherEntity->IsTouching(this, vecTouchingPoint))
				pOtherEntity->Rescue(this);
		}
	}

	for (size_t i = 0; i < Game()->GetNumPlayers(); i++)
	{
		if (Game()->GetPlayer(i))
			DigitanksGame()->GetDigitanksPlayer(i)->CalculateVisibility();
	}

	InterceptSupplyLines();

	CDigitank* pClosestEnemy = FindClosestVisibleEnemyTank();
	if (HasGoalMovePosition() && pClosestEnemy)
	{
		GetDigitanksPlayer()->AddActionItem(this, ACTIONTYPE_AUTOMOVEENEMY);
	}

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	m_flGoalTurretYaw = 0;

	DirtyNeedsOrders();
	GetDigitanksPlayer()->HandledActionItem(this);

	DigitanksWindow()->GetHUD()->UpdateTurnButton();
}

bool CDigitank::IsMoving() const
{
	float flTransitionTime = GetTransitionTime();

	// If we're in multiplayer and this tank belongs to real player, never skip its movement.
	if (!GameNetwork()->IsConnected() || GetDigitanksPlayer() && !GetDigitanksPlayer()->IsHumanControlled())
	{
		if (GetVisibility() == 0)
			flTransitionTime = 0;
	}

	float flTimeSinceMove = (float)(GameServer()->GetGameTime() - m_flStartedMove);
	if (m_flStartedMove && flTimeSinceMove < flTransitionTime)
		return true;

	return false;
}

void CDigitank::Move(Vector vecNewPosition, int iMoveType)
{
	m_vecPreviousOrigin = GetGlobalOrigin();
	m_flStartedMove = GameServer()->GetGameTime();
	SetGlobalOrigin(vecNewPosition);
	m_iMoveType = iMoveType;

	if (TakesLavaDamage() && DigitanksGame()->GetTerrain()->IsPointOverLava(vecNewPosition))
		TakeDamage(NULL, NULL, DAMAGE_BURN, DigitanksGame()->LavaDamage(), false);

	if (IsSentried())
		Sentry();

	DirtyVisibility();
}

void CDigitank::Turn()
{
	if (IsFortified() && !CanTurnFortified())
		return;

	if (IsDisabled())
		return;

	float flMovePower = GetPreviewBaseTurnPower();

	if (flMovePower > GetRemainingMovementEnergy())
		return;

	if (GameNetwork()->ShouldReplicateClientFunction())
		GameNetwork()->CallFunction(NETWORK_TOEVERYONE, "Turn", GetHandle(), m_flPreviewTurn);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = m_flPreviewTurn;
	Turn(&p);

	DigitanksWindow()->SetContextualCommandsOverride(true);
}

void CDigitank::Turn(CNetworkParameters* p)
{
	m_flPreviewTurn = p->fl2;

	Turn(EAngle(0, m_flPreviewTurn, 0));

	float flMovePower = GetPreviewBaseTurnPower();

	if (GameNetwork()->IsHost())
	{
		m_flMovementPower += flMovePower;
	}

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	m_flGoalTurretYaw = 0;

	DirtyNeedsOrders();

	DigitanksWindow()->GetHUD()->UpdateTurnButton();
}

bool CDigitank::IsTurning()
{
	float flTransitionTime = GetTransitionTime();

	if (!GameNetwork()->IsConnected() || GetDigitanksPlayer() && !GetDigitanksPlayer()->IsHumanControlled())
	{
		if (GetVisibility() == 0)
			flTransitionTime = 0;
	}

	float flTimeSinceTurn = (float)(GameServer()->GetGameTime() - m_flStartedTurn);
	if (m_flStartedTurn > 0 && flTimeSinceTurn < flTransitionTime)
		return true;

	return false;
}

void CDigitank::Turn(EAngle angNewTurn)
{
	m_flPreviousTurn = GetGlobalAngles().y;
	m_flStartedTurn = GameServer()->GetGameTime();
	SetGlobalAngles(angNewTurn);

	if (IsSentried())
		Sentry();
}

void CDigitank::SetGoalMovePosition(const Vector& vecPosition)
{
	if (IsFortified() && !CanMoveFortified())
		return;

	if (fabs(vecPosition.x) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	if (fabs(vecPosition.y) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	if (GameNetwork()->IsHost())
	{
		CNetworkParameters p;
		p.fl2 = vecPosition.x;
		p.fl3 = vecPosition.y;
		p.fl4 = vecPosition.z;
		SetGoalMovePosition(&p);
	}
	else
		GameNetwork()->CallFunction(NETWORK_TOSERVER, "SetGoalMovePosition", GetHandle(), vecPosition.x, vecPosition.y, vecPosition.z);
}

void CDigitank::SetGoalMovePosition(CNetworkParameters* p)
{
	if (!GameNetwork()->IsHost())
		return;

	m_bGoalMovePosition = true;
	m_vecGoalMovePosition = Vector(p->fl2, p->fl3, p->fl4);

	MoveTowardsGoalMovePosition();
}

void CDigitank::MoveTowardsGoalMovePosition()
{
	if (!GameNetwork()->IsHost())
		return;

	GameNetwork()->SetRunningClientFunctions(false);

	bool bIsAHoleInTheWay = false;

	Vector vecGoal = GetGoalMovePosition();
	Vector vecOrigin = GetGlobalOrigin();
	Vector vecMove = vecGoal - vecOrigin;

	Vector vecNewPosition = GetGlobalOrigin() + vecMove;

	float flDistance = vecMove.Length2D();
	float flDistancePerSegment = 5;
	Vector vecPathFlat = vecMove;
	vecPathFlat.y = 0;
	Vector vecDirection = vecPathFlat.Normalized();
	size_t iSegments = (size_t)(flDistance/flDistancePerSegment);

	Vector vecLastSegmentStart = GetRealOrigin();
	float flMaxMoveDistance = GetMaxMovementDistance();
	float flSegmentLength = flMaxMoveDistance;

	if ((vecGoal - GetRealOrigin()).LengthSqr() < flMaxMoveDistance*flMaxMoveDistance)
		flSegmentLength = (vecGoal - GetRealOrigin()).Length();

	for (size_t i = 1; i < iSegments; i++)
	{
		float flCurrentDistance = ((float)i*flDistancePerSegment);

		Vector vecLink = GetRealOrigin() + vecDirection*flCurrentDistance;

		if (DigitanksGame()->GetTerrain()->IsPointOverHole(vecLink))
		{
			bIsAHoleInTheWay = true;
			break;
		}
	}

	if (bIsAHoleInTheWay)
		vecMove = DigitanksGame()->GetTerrain()->FindPath(GetGlobalOrigin(), vecGoal, this) - vecOrigin;

	do
	{
		vecMove = vecMove * 0.95f;

		if (vecMove.Length() < 1)
			break;

		vecNewPosition = GetGlobalOrigin() + vecMove;
		vecNewPosition.z = FindHoverHeight(vecNewPosition);

		SetPreviewMove(vecNewPosition);
	}
	while (!IsPreviewMoveValid());

	SetPreviewMove(vecNewPosition);
	Move();

	if ((GetRealOrigin() - GetGoalMovePosition()).Length2DSqr() < 1)
	{
		CancelGoalMovePosition();
		GetDigitanksPlayer()->AddActionItem(this, ACTIONTYPE_UNITAUTOMOVE);
		return;
	}
}

void CDigitank::CancelGoalMovePosition()
{
	if (GameNetwork()->IsHost())
		CancelGoalMovePosition(NULL);
	else
		GameNetwork()->CallFunction(NETWORK_TOSERVER, "CancelGoalMovePosition", GetHandle());
}

void CDigitank::CancelGoalMovePosition(CNetworkParameters* p)
{
	m_bGoalMovePosition = false;
}

void CDigitank::Fortify()
{
	if (!CanFortify())
		return;

	if (IsDisabled())
		return;

	GameNetwork()->CallFunction(NETWORK_TOEVERYONE, "Fortify", GetHandle());

	CNetworkParameters p;
	p.ui1 = GetHandle();
	Fortify(&p);
}

void CDigitank::Fortify(CNetworkParameters* p)
{
	m_flFortifyTime = GameServer()->GetGameTime();

	m_flMovementPower = GetMaxMovementEnergy();

	if (m_bFortified)
	{
		m_bFortified = false;
		DigitanksWindow()->GetHUD()->UpdateTurnButton();
		return;
	}

	if (IsSentried())
		Sentry();

	m_bFortified = true;

	m_iFortifyLevel = 0;

	OnFortify();

	DirtyNeedsOrders();

	if (!GameServer()->IsLoading())
		DigitanksWindow()->GetHUD()->UpdateTurnButton();

//	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_FORTIFYING);
//	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_DEPLOYING, true);
}

bool CDigitank::CanAim() const
{
	return AllowControlMode(MODE_AIM);
}

float CDigitank::GetFortifyAttackPowerBonus()
{
	if (m_bFortified)
		return (float)m_iFortifyLevel;
	else
		return 0;
}

float CDigitank::GetFortifyDefensePowerBonus() const
{
	if (m_bFortified)
		return (float)m_iFortifyLevel;
	else
		return 0;
}

void CDigitank::Sentry()
{
	if (!CanSentry())
		return;

	if (IsDisabled())
		return;

	GameNetwork()->CallFunction(NETWORK_TOEVERYONE, "Sentry", GetHandle());

	CNetworkParameters p;
	p.ui1 = GetHandle();
	Sentry(&p);
}

void CDigitank::Sentry(CNetworkParameters* p)
{
	if (m_bSentried)
	{
		m_bSentried = false;
		DigitanksWindow()->GetHUD()->UpdateTurnButton();
		return;
	}

	m_bSentried = true;

	CancelGoalMovePosition();

	DirtyNeedsOrders();

	DigitanksWindow()->GetHUD()->UpdateTurnButton();
}

CLIENT_GAME_COMMAND(Charge)
{
	if (pCmd->GetNumArguments() < 2)
	{
		TMsg("Charge with less than 2 parameters.\n");
		return;
	}

	CEntityHandle<CDigitank> hTank(pCmd->ArgAsUInt(0));
	CEntityHandle<CBaseEntity> hTarget(pCmd->ArgAsUInt(1));

	if (!hTank)
	{
		TMsg("Charge on invalid tank.\n");
		return;
	}

	if ((!hTank->GetPlayerOwner() || hTank->GetPlayerOwner()->GetClient() != (int)iClient))
	{
		TMsg("Charge for a tank the player doesn't own.\n");
		return;
	}

	if (!hTarget)
	{
		TMsg("Charge with an invalid target.\n");
		return;
	}

	hTank->Charge(hTarget);
}

void CDigitank::Charge()
{
	if (IsFortified() && !CanTurnFortified())
		return;

	if (ChargeEnergy() > m_flTotalPower)
		return;

	if (m_bFiredWeapon || m_bActionTaken)
		return;

	if (!m_hPreviewCharge)
		return;

	if (IsDisabled())
		return;

	::Charge.RunCommand(tsprintf(tstring("%d %d"), GetHandle(), m_hPreviewCharge->GetHandle()));

	DigitanksGame()->SetControlMode(MODE_NONE);
	DigitanksWindow()->SetContextualCommandsOverride(true);
}

void CDigitank::Charge(CBaseEntity* pTarget)
{
	m_hChargeTarget = pTarget;

	if (pTarget == NULL)
		return;

	Vector vecChargeDirection = (pTarget->GetGlobalOrigin() - GetGlobalOrigin()).Normalized();

	Turn(VectorAngles(vecChargeDirection));

	float flDistanceToTarget = pTarget->Distance(GetGlobalOrigin());
	if (flDistanceToTarget < 10)
	{
		Vector vecChargeStart = pTarget->GetGlobalOrigin() - vecChargeDirection * 20;
		vecChargeStart.z = FindHoverHeight(vecChargeStart);
		Move(vecChargeStart);
	}

	m_flTotalPower -= ChargeEnergy();
	m_flAttackPower += ChargeEnergy();
	m_flMovementPower += ChargeEnergy();

	m_flGoalTurretYaw = 0;

	m_bActionTaken = true;
	m_bFiredWeapon = true;

	DirtyNeedsOrders();

	m_flBeginCharge = GameServer()->GetGameTime() + GetTransitionTime();

	if (IsSentried())
		Sentry();
}

void CDigitank::Cloak()
{
	m_bCloaked = true;
	DirtyVisibility();
	DigitanksWindow()->GetHUD()->Layout();
}

void CDigitank::Uncloak()
{
	m_bCloaked = false;
	DirtyVisibility();
	DigitanksWindow()->GetHUD()->Layout();
}

float CDigitank::GetCloakConcealment() const
{
	if (IsCloaked())
		return 0.5f;

	return 0;
}

bool CDigitank::MovesWith(CDigitank* pOther) const
{
	if (!pOther)
		return false;

	if (this == pOther)
		return true;

	// Only same team.
	if (GetPlayerOwner() != pOther->GetPlayerOwner())
		return false;

	// Not fortified tanks.
	if (IsFortified() || IsFortifying())
		return false;

	// Only same class tanks.
	if (GetUnitType() != pOther->GetUnitType())
		return false;

	if (!GetDigitanksPlayer()->IsSelected(this))
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
	if (GetPlayerOwner() != pOther->GetPlayerOwner())
		return false;

	if (!GetDigitanksPlayer()->IsSelected(this))
		return false;

	// Only same class tanks.
	if (GetUnitType() != pOther->GetUnitType())
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
	if (GetPlayerOwner() != pOther->GetPlayerOwner())
		return false;

	if (!GetDigitanksPlayer()->IsSelected(this))
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

	if (!DigitanksGame())
		return;

	if ((GetDigitanksPlayer() && GetDigitanksPlayer()->IsSelected(this) && DigitanksGame()->GetControlMode() == MODE_AIM) || m_bFiredWeapon)
	{
		if (GetCurrentWeapon() != WEAPON_CHARGERAM)
		{
			Vector vecAimTarget;
			if (GetDigitanksPlayer()->IsSelected(this) && DigitanksGame()->GetControlMode() == MODE_AIM)
				vecAimTarget = GetPreviewAim();
			else
				vecAimTarget = m_vecLastAim;

			Vector vecDirection = (vecAimTarget - GetRenderOrigin()).Flattened();

			if (vecDirection.LengthSqr() > 0)
			{
				Vector vecTarget = vecDirection.Normalized();
				m_flGoalTurretYaw = atan2(vecTarget.y, vecTarget.x) * 180 / M_PI - GetRenderAngles().y;
			}
		}
	}

	float flSpeed = fabs(AngleDifference(m_flGoalTurretYaw, m_flCurrentTurretYaw)) * GameServer()->GetFrameTime() * 10;
	m_flCurrentTurretYaw = AngleApproach(m_flGoalTurretYaw, m_flCurrentTurretYaw, flSpeed);

	m_flGlowYaw += (float)fmod(GameServer()->GetFrameTime()*10, 360);

	if (!IsMoving() && IsAlive() && GameServer()->GetGameTime() > m_flNextHoverHeightCheck)
	{
		// Checking the hover height so often can be expensive, so we only do i once in a while.
		m_flNextHoverHeightCheck = GameServer()->GetGameTime() + 0.3f;

		if (!DigitanksGame()->GetTerrain()->IsPointOnMap(GetGlobalOrigin()) || DigitanksGame()->GetTerrain()->IsPointOverHole(GetGlobalOrigin()))
			Kill();
		else
		{
			float flHoverHeight = FindHoverHeight(GetGlobalOrigin());
			if (fabs(GetGlobalOrigin().z - flHoverHeight) > 1.0f)
				Move(Vector(GetGlobalOrigin().x, GetGlobalOrigin().y, flHoverHeight));
		}
	}

	m_bDisplayAim = false;

	bool bAimMode = DigitanksGame()->GetControlMode() == MODE_AIM && DigitanksGame()->GetAimType() == AIM_NORMAL;
	bool bShowThisTank = m_bFiredWeapon;
	if (bAimMode && GetDigitanksPlayer() && GetDigitanksPlayer()->IsSelected(this) && AimsWith(GetDigitanksPlayer()->GetPrimarySelectionTank()))
		bShowThisTank = true;
	if (GetDigitanksPlayer() != DigitanksGame()->GetCurrentPlayer())
		bShowThisTank = false;

	if (bShowThisTank)
	{
		Vector vecMouseAim;
		CBaseEntity* pHit = NULL;
		bool bMouseOK = DigitanksWindow()->GetMouseGridPosition(vecMouseAim, &pHit);

		Vector vecTankAim;
		if (m_bFiredWeapon)
			vecTankAim = m_vecLastAim;

		if (bMouseOK && bAimMode)
		{
			if (AimsWith(GetDigitanksPlayer()->GetPrimarySelectionTank()) && GetDigitanksPlayer()->IsSelected(this))
			{
				if (pHit && dynamic_cast<CDigitanksEntity*>(pHit) && pHit->GetOwner() && pHit->GetOwner() != GetOwner())
					vecTankAim = pHit->GetGlobalOrigin();
				else
					vecTankAim = vecMouseAim;
			}
		}

		if (GetCurrentWeapon() != WEAPON_CHARGERAM && (m_bFiredWeapon || bMouseOK))
		{
			vecTankAim = FindNearestTerrainPointInRange(vecTankAim);

			if ((vecTankAim - GetGlobalOrigin()).Length() < GetMinRange())
			{
				vecTankAim = GetGlobalOrigin() + (vecTankAim - GetGlobalOrigin()).Normalized() * GetMinRange() * 1.01f;
				vecTankAim.z = DigitanksGame()->GetTerrain()->GetHeight(vecTankAim.x, vecTankAim.y);
			}

			if (fabs(AngleDifference(GetGlobalAngles().y, VectorAngles((vecTankAim - GetGlobalOrigin()).Normalized()).y)) > FiringCone())
				m_bDisplayAim = false;
			else
			{
				float flRadius = FindAimRadius(vecTankAim);
				DigitanksGame()->AddTankAim(vecTankAim, flRadius, GetDigitanksPlayer()->IsSelected(this) && DigitanksGame()->GetControlMode() == MODE_AIM);

				m_bDisplayAim = true;
				m_vecDisplayAim = vecTankAim;
				m_flDisplayAimRadius = flRadius;
			}
		}
	}

	if (m_flFireWeaponTime && GameServer()->GetGameTime() > m_flFireWeaponTime)
	{
		m_iFireWeapons--;
		FireWeapon();

		if (m_iFireWeapons)
			m_flFireWeaponTime = GameServer()->GetGameTime() + CProjectile::GetWeaponFireInterval(GetCurrentWeapon());
		else
			m_flFireWeaponTime = 0;
	}

	// Only stay on if it was turned on before. We don't want to activate for all moves and turns.
	m_hHoverParticles.SetActive(m_hHoverParticles.IsActive() && (IsMoving() || IsTurning()));
	m_hChargeParticles.SetActive(m_flBeginCharge > 0 || m_flEndCharge > 0);

	if (IsAlive() && GameServer()->GetGameTime() > m_flNextIdle)
	{
		// A little bit less often if we're not on the current team.
		if (DigitanksGame()->GetCurrentPlayer() == GetPlayerOwner() && rand()%2 == 0 || rand()%4 == 0)
		{
			if (DigitanksGame()->IsPartyMode())
				Speak(TANKSPEECH_PARTY);
			else if (IsDisabled())
				Speak(TANKSPEECH_DISABLED);
			else
				Speak(TANKSPEECH_IDLE);
		}

		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}

	if (m_flBeginCharge > 0 && GameServer()->GetGameTime() > m_flBeginCharge)
	{
		m_flBeginCharge = -1;

		if (m_hChargeTarget != NULL)
		{
			Move(GetChargePosition(m_hChargeTarget), 1);
			m_flEndCharge = GameServer()->GetGameTime() + GetTransitionTime();

			m_flGoalTurretYaw = 0;
		}
	}

	if (m_flEndCharge > 0 && GameServer()->GetGameTime() > m_flEndCharge)
	{
		m_flEndCharge = -1;

		if (GameNetwork()->IsHost() && m_hChargeTarget != NULL)
		{
			m_hChargeTarget->TakeDamage(this, this, DAMAGE_COLLISION, ChargeDamage() + GetBonusAttackDamage(), true);

			Vector vecPushDirection = (m_hChargeTarget->GetGlobalOrigin() - GetGlobalOrigin()).Normalized();

			CDigitank* pDigitank = dynamic_cast<CDigitank*>(m_hChargeTarget.GetPointer());
			if (pDigitank)
			{
				if (pDigitank->IsFortified())
				{
					TakeDamage(this, this, DAMAGE_COLLISION, ChargeDamage()/2, true);
					Move(GetGlobalOrigin() - vecPushDirection * ChargePushDistance()/2, 2);
					RockTheBoat(1, -vecPushDirection);
				}
				else
				{
					pDigitank->Move(pDigitank->GetGlobalOrigin() + vecPushDirection * ChargePushDistance(), 2);
					pDigitank->RockTheBoat(1, vecPushDirection);
				}

				Vector vecLookAt = (GetGlobalOrigin() - pDigitank->GetGlobalOrigin()).Normalized();
				pDigitank->m_flGoalTurretYaw = atan2(vecLookAt.y, vecLookAt.x) * 180/M_PI - pDigitank->GetRenderAngles().y;

				CNetworkedEffect::AddInstance("charge-burst", GetGlobalOrigin() + vecLookAt * 3, GetGlobalAngles());
			}

			RockTheBoat(1, vecPushDirection);
			Turn(EAngle(0, GetGlobalAngles().y, 0));

			m_flGoalTurretYaw = 0;

			DigitanksWindow()->GetHUD()->UpdateTurnButton();
		}
	}

	float flTransitionTime = GetTransitionTime();

	if (!GameNetwork()->IsConnected() || GetDigitanksPlayer() && !GetDigitanksPlayer()->IsHumanControlled())
	{
		if (GetVisibility() == 0)
			flTransitionTime = 0;
	}

	float flTimeSinceMove = (float)(GameServer()->GetGameTime() - m_flStartedMove);
	if (m_flStartedMove && flTimeSinceMove > flTransitionTime)
	{
		// We were moving but now we're done.
		DigitanksGame()->GetTerrain()->CalculateVisibility();

		m_flStartedMove = 0.0;
	}

	if (IsAlive())
	{
		bool bRunFire = GetHealth() < GetTotalHealth()/3;
		bool bRunSmoke = !bRunFire && GetHealth() <= GetTotalHealth()*4/5;

		m_hSmokeParticles.SetActive(GetVisibility() > 0.1f && bRunSmoke);
		m_hFireParticles.SetActive(GetVisibility() > 0.1f && bRunFire);
	}
}

void CDigitank::OnCurrentSelection()
{
	BaseClass::OnCurrentSelection();

	if (GetDigitanksPlayer() != DigitanksGame()->GetCurrentLocalDigitanksPlayer())
	{
		DigitanksGame()->SetControlMode(MODE_NONE);
		return;
	}

	if (GetVisibility() > 0)
	{
		if (rand()%2 == 0)
			CSoundLibrary::PlaySound(this, "sound/tank-active.wav");
		else
			CSoundLibrary::PlaySound(this, "sound/tank-active2.wav");
	}

	Speak(TANKSPEECH_SELECTED);

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	CDigitank* pClosestEnemy = NULL;
	while (true)
	{
		pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(GetGlobalOrigin(), pClosestEnemy);

		if (!pClosestEnemy)
			break;

		if (pClosestEnemy->GetPlayerOwner() == GetPlayerOwner())
			continue;

		if ((pClosestEnemy->GetGlobalOrigin() - GetGlobalOrigin()).Length() > VisibleRange())
		{
			pClosestEnemy = NULL;
			break;
		}

		break;
	}

	DigitanksGame()->SetControlMode(MODE_NONE);
}

bool CDigitank::AllowControlMode(controlmode_t eMode) const
{
	if (eMode == MODE_MOVE)
		return true;

	if (eMode == MODE_TURN)
		return true;

	if (eMode == MODE_AIM)
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
	{
		ClearPreviewAim();
		ClearPreviewCharge();
		DigitanksGame()->SetAimType(AIM_NONE);
	}

	if (eNewMode == MODE_AIM)
	{
		if (IsArtillery())
		{
			Vector vecCenter = DigitanksGame()->GetTerrain()->GetPointHeight(GetGlobalOrigin() + AngleVector(GetGlobalAngles()) * GetMaxRange() / 2);
			DigitanksGame()->GetOverheadCamera()->SetTarget(vecCenter);
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
	return GetRemainingMovementEnergy(true) / m_flStartingPower;
}

float CDigitank::GetPowerBar1Size()
{
	float flPower = GetAttackPower(true) / GetTotalAttackPower();

	if (flPower > 1)
		return 1;

	if (flPower < 0)
		return 0;

	return flPower;
}

float CDigitank::GetPowerBar2Size()
{
	float flPower = GetDefensePower(true) / GetTotalDefensePower();

	if (flPower > 1)
		return 1;

	if (flPower < 0)
		return 0;

	return flPower;
}

float CDigitank::GetPowerBar3Size()
{
	float flPower = 1-(GetUsedMovementEnergy(true) / GetMaxMovementEnergy());

	if (flPower > 1)
		return 1;

	if (flPower < 0)
		return 0;

	return flPower;
}

void CDigitank::DirtyNeedsOrders()
{
	m_bNeedsOrdersDirty = true;
}

bool CDigitank::NeedsOrders()
{
	if (!m_bNeedsOrdersDirty)
		return m_bNeedsOrders;

	if (IsDisabled())
		return false;

	bool bNeedsToMove = true;
	if (GetUsedMovementEnergy() > 0)
		bNeedsToMove = false;
	else
	{
		if (IsFortified() || IsFortifying())
			bNeedsToMove = false;

		if (IsSentried())
			bNeedsToMove = false;
	}

	bool bNeedsToAttack = true;
	if (HasFiredWeapon())
		bNeedsToAttack = false;
	else if (!IsScout())
	{
		CDigitanksEntity* pClosestEnemy = NULL;
		while (true)
		{
			pClosestEnemy = CBaseEntity::FindClosest<CDigitanksEntity>(GetGlobalOrigin(), pClosestEnemy);

			if (pClosestEnemy)
			{
				if (!IsInsideMaxRange(pClosestEnemy->GetGlobalOrigin()))
				{
					pClosestEnemy = NULL;
					break;
				}

				if (pClosestEnemy->GetPlayerOwner() == GetPlayerOwner())
					continue;

				if (!pClosestEnemy->GetPlayerOwner())
					continue;

				if (pClosestEnemy->GetVisibility() == 0)
					continue;

				if (IsArtillery() && fabs(AngleDifference(GetGlobalAngles().y, VectorAngles((pClosestEnemy->GetGlobalOrigin() - GetGlobalOrigin()).Normalized()).y)) > FiringCone())
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

	if (m_bActionTaken)
		bNeedsToMove = false;

	m_bNeedsOrders = bNeedsToMove || bNeedsToAttack;
	m_bNeedsOrdersDirty = false;

	return m_bNeedsOrders;
}

void CDigitank::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = DigitanksWindow()->GetHUD();

	if (IsDisabled())
		return;

	if (eMenuMode == MENUMODE_MAIN)
	{
		if (HasGoalMovePosition())
		{
			pHUD->SetButtonListener(0, CHUD::CancelAutoMove);
			pHUD->SetButtonColor(0, Color(50, 50, 50));
			pHUD->SetButtonTexture(0, "Cancel");
			pHUD->SetButtonInfo(0, "CANCEL AUTO MOVE\n \nCancel this unit's auto move command.\n \nShortcut: Q");
			pHUD->SetButtonTooltip(0, "Cancel Auto-Move");
		}
		else if (!IsFortified() && !IsFortifying())
		{
			if (GetRemainingMovementEnergy() < 1)
			{
				pHUD->SetButtonListener(0, CHUD::Move);

				if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_MOVE)
					pHUD->SetButtonColor(0, Color(150, 150, 0));
				else
					pHUD->SetButtonColor(0, Color(50, 50, 50));

				if (DigitanksGame()->GetControlMode() == MODE_MOVE)
					pHUD->SetButtonTexture(0, "Cancel");
				else
					pHUD->SetButtonTexture(0, "AutoMove");

				pHUD->SetButtonInfo(0, "AUTO MOVE\n \nThis tank is out of move energy for this turn.\n \nSet a move command for this tank to execute over the next few turns.\n \nShortcut: Q");
				pHUD->SetButtonTooltip(0, "Auto-Move");
			}
			else
			{
				pHUD->SetButtonListener(0, CHUD::Move);

				if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_MOVE)
					pHUD->SetButtonColor(0, Color(150, 150, 0));
				else
					pHUD->SetButtonColor(0, Color(50, 50, 50));

				if (DigitanksGame()->GetControlMode() == MODE_MOVE)
					pHUD->SetButtonTexture(0, "Cancel");
				else
					pHUD->SetButtonTexture(0, "Move");

				pHUD->SetButtonInfo(0, "MOVE UNIT\n \nGo into Move mode. Click inside the yellow area to move this unit.\n \nShortcut: Q");
				pHUD->SetButtonTooltip(0, "Move");
			}
		}

		if (TurningMatters())
		{
			if (GetRemainingMovementEnergy() < 1)
			{
				pHUD->SetButtonListener(1, NULL);
				pHUD->SetButtonColor(1, Color(100, 100, 100));
				pHUD->SetButtonTexture(1, "Rotate");
				pHUD->SetButtonInfo(1, "ROTATE UNIT\n \nGo into Rotate mode. Click any spot on the terrain to have this unit face that spot.\n \nNOT ENOUGH ENERGY\n \nShortcut: W");
				pHUD->SetButtonTooltip(1, "Rotate");
			}
			else if ((!IsFortified() && !IsFortifying() || CanTurnFortified()))
			{
				pHUD->SetButtonListener(1, CHUD::Turn);

				if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_TURN)
					pHUD->SetButtonColor(1, Color(150, 150, 0));
				else
					pHUD->SetButtonColor(1, Color(50, 50, 50));

				if (DigitanksGame()->GetControlMode() == MODE_TURN)
					pHUD->SetButtonTexture(1, "Cancel");
				else
					pHUD->SetButtonTexture(1, "Rotate");
				pHUD->SetButtonInfo(1, "ROTATE UNIT\n \nGo into Rotate mode. Click any spot on the terrain to have this unit face that spot.\n \nShortcut: W");
				pHUD->SetButtonTooltip(1, "Rotate");
			}
		}

		if (CanSentry() && !IsFortified() && DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		{
			if (IsSentried())
			{
				pHUD->SetButtonTexture(5, "Mobilize");
				pHUD->SetButtonInfo(5, "MOBILIZE\n \nCancel the 'Hold Position' order.\n \nShortcut: A");
				pHUD->SetButtonTooltip(5, "Mobilize");
			}
			else
			{
				pHUD->SetButtonTexture(5, "Sentry");
				pHUD->SetButtonInfo(5, "HOLD POSITION\n \nThis unit will hold position and not require orders until told otherwise.\n \nShortcut: A");
				pHUD->SetButtonTooltip(5, "Hold Position");
			}
			pHUD->SetButtonListener(5, CHUD::Sentry);
			pHUD->SetButtonColor(5, Color(0, 0, 150));
		}

		if (HasCloak() && !HasFiredWeapon())
		{
			if (IsCloaked())
			{
				pHUD->SetButtonInfo(6, "DEACTIVATE CLOAKING DEVICE\n \nClick to turn off this tank's cloaking device.\n \nStay away from enemies to retain your cloak's effectiveness.\n \nShortcut: S");
				pHUD->SetButtonTooltip(6, _T("Uncloak"));
				pHUD->SetButtonTexture(6, "Unstealth");
			}
			else
			{
				pHUD->SetButtonInfo(6, _T("ACTIVATE CLOAKING DEVICE\n \nThis tank has a cloaking device available. Click to activate it.\n \nShortcut: S"));
				pHUD->SetButtonTooltip(6, _T("Cloak"));
				pHUD->SetButtonTexture(6, "Stealth");
			}
			pHUD->SetButtonListener(6, CHUD::Cloak);
			pHUD->SetButtonColor(6, Color(0, 0, 150));
		}

		if (GetNumAllowedWeapons() > 1)
		{
			pHUD->SetButtonTexture(7, "ChooseWeapon");
			pHUD->SetButtonInfo(7, _T("CHOOSE WEAPON\n \nThis tank has multiple weapons available. Click to choose a weapon.\n \nShortcut: D"));
			pHUD->SetButtonTooltip(7, _T("Choose Weapon"));
			pHUD->SetButtonListener(7, CHUD::ChooseWeapon);
			pHUD->SetButtonColor(7, Color(50, 50, 50));

			if (HasFiredWeapon())
			{
				pHUD->SetButtonColor(7, Color(100, 100, 100));
				pHUD->SetButtonListener(7, NULL);
				pHUD->SetButtonInfo(7, _T("CHOOSE WEAPON\n \n(Unavailable)\n \nYou may only fire this tank's weapon once per turn.\n \nShortcut: D"));
			}
		}

		if (CanFortify())
		{
			if (IsFortified() || IsFortifying())
			{
				pHUD->SetButtonTexture(8, "Mobilize");
				pHUD->SetButtonInfo(8, _T("MOBILIZE\n \nAllows this unit to move again.\n \nShortcut: F"));
				pHUD->SetButtonTooltip(8, _T("Mobilize"));
			}
			else
			{
				if (IsMobileCPU())
				{
					pHUD->SetButtonTexture(8, "DeployCPU");
					pHUD->SetButtonInfo(8, _T("DEPLOY UNIT\n \nHave this MCP deploy and create a CPU. The CPU will be the center of operations for your base. This action cannot be undone.\n \nShortcut: F"));
					pHUD->SetButtonTooltip(8, _T("Deploy"));
				}
				else if (IsArtillery())
				{
					pHUD->SetButtonTexture(8, "Deploy");
					pHUD->SetButtonInfo(8, _T("DEPLOY UNIT\n \nHave this artillery deploy. Artillery must be deployed before they can be fired.\n \nShortcut: F"));
					pHUD->SetButtonTooltip(8, _T("Deploy"));
				}
				else
				{
					pHUD->SetButtonTexture(8, "Fortify");
					pHUD->SetButtonInfo(8, _T("FORTIFY UNIT\n \nHave this unit fortify his position mode. Offers combat bonuses that accumulate over the next few turns.\n \nShortcut: F"));
					pHUD->SetButtonTooltip(8, _T("Fortify"));
				}
			}
			pHUD->SetButtonListener(8, CHUD::Fortify);
			pHUD->SetButtonColor(8, Color(0, 0, 150));
		}

		if (CanAimMobilized() || IsFortified())
		{
			if (m_bFiredWeapon || m_bActionTaken)
			{
				pHUD->SetButtonListener(2, NULL);
				pHUD->SetButtonColor(2, glgui::g_clrBox);
				pHUD->SetButtonTexture(2, "Aim");

				tstring s = _T("AIM AND FIRE\n \n(Unavailable)\n \nYou may only fire this tank's weapon once per turn.");

				s += _T("\n \nShortcut: E");

				pHUD->SetButtonInfo(2, s);
				pHUD->SetButtonTooltip(2, _T("Aim"));
			}
			else if (m_eWeapon != WEAPON_NONE)
			{
				pHUD->SetButtonListener(2, CHUD::Aim);

				if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_AIM)
					pHUD->SetButtonColor(2, Color(150, 0, 0));
				else
					pHUD->SetButtonColor(2, Color(50, 50, 50));

/*				CInstructor* pInstructor = DigitanksWindow()->GetInstructor();
				if (pInstructor->GetActive() && pInstructor->GetCurrentTutorial() >= CInstructor::TUTORIAL_INGAME_ARTILLERY_SELECT && pInstructor->GetCurrentTutorial() < CInstructor::TUTORIAL_INGAME_ARTILLERY_COMMAND)
				{
					pHUD->SetButtonListener(2, NULL);
					pHUD->SetButtonColor(2, glgui::g_clrBox);
				}*/

				pHUD->SetButtonTexture(2, "Aim");

				if (DigitanksGame()->GetControlMode() == MODE_AIM)
					pHUD->SetButtonTexture(2, "Cancel");

				tstring s;
				if (IsInfantry())
					s += _T("AIM AND FIRE MOUNTED GUN\n \nClick to enter Aim mode. Click any spot on the terrain to fire on that location.");
				else if (IsScout())
					s += _T("AIM AND FIRE TORPEDO\n \nClick to enter Aim mode. Click any spot on the terrain to fire on that location.\n \nTorpedos contain a special EMP that can disable enemy tanks and sever support lines. It can't hit other Rogues and only does physical damage to structures or units without shields.");
				else
					s += _T("AIM AND FIRE CANNON\n \nClick to enter Aim mode. Click any spot on the terrain to fire on that location.");

				if (m_flTotalPower < GetWeaponEnergy())
				{
					pHUD->SetButtonColor(2, Color(100, 100, 100));
					s += _T("\n \nNOT ENOUGH ENERGY");

					pHUD->SetButtonListener(2, NULL);
				}

				s += _T("\n \nShortcut: E");

				pHUD->SetButtonInfo(2, s);
				pHUD->SetButtonTooltip(2, _T("Aim"));
			}
		}

		if (HasBonusPoints() && m_iBonusLevel < 5)
		{
			pHUD->SetButtonListener(4, CHUD::Promote);
			pHUD->SetButtonTexture(4, "Promote");
			pHUD->SetButtonColor(4, Color(50, 50, 50));
			pHUD->SetButtonInfo(4, _T("UPGRADE UNIT\n \nThis unit has upgrades available. Click to open the upgrades menu.\n \nShortcut: T"));
			pHUD->SetButtonTooltip(4, _T("Upgrade"));
		}

		if (m_iAirstrikes && !m_bActionTaken)
		{
			if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_AIM)
				pHUD->SetButtonColor(9, Color(150, 0, 0));
			else
				pHUD->SetButtonColor(9, Color(50, 50, 50));

			if (DigitanksGame()->GetControlMode() == MODE_AIM)
				pHUD->SetButtonTexture(9, "Cancel");
			else
				pHUD->SetButtonTexture(9, "Airstrike");

			pHUD->SetButtonListener(9, CHUD::FireSpecial);
			pHUD->SetButtonInfo(9, _T("CALL AN AIRSTRIKE\n \nThis unit can call an airstrike. Click to aim and fire the airstrike.\n \nShortcut: G"));
			pHUD->SetButtonTooltip(9, _T("Airstrike"));
		}
	}
	else if (eMenuMode == MENUMODE_PROMOTE)
	{
		pHUD->SetButtonListener(0, CHUD::PromoteAttack);
		pHUD->SetButtonTexture(0, "UpgradeAttack");
		pHUD->SetButtonColor(0, Color(150, 50, 50));

		tstring s1;
		s1 += _T("UPGRADE ATTACK ENERGY\n \n");
		s1 += _T("This upgrade amplifies your tank's arsenal, increasing the maximum Attack Energy available to your tank past its normal levels. With greater Attack Energy, this tank's shells will deal more damage.\n \n");
		s1 += _T("Attack Energy increase: 10%\n \n");
		s1 += _T("Shortcut: Q");
		pHUD->SetButtonInfo(0, s1);
		pHUD->SetButtonTooltip(0, _T("Upgrade Attack"));

		if (!IsArtillery() && !IsScout())
		{
			pHUD->SetButtonListener(1, CHUD::PromoteDefense);
			pHUD->SetButtonTexture(1, "UpgradeShields");
			pHUD->SetButtonColor(1, Color(50, 50, 150));

			tstring s;
			s += _T("UPGRADE SHIELD ENERGY\n \n");
			s += _T("This upgrade strengthens your tank's shield generator, increasing the maximum Shield Energy available to your tank past its normal levels. As a result, your tank's shields will take more damage before they fail.\n \n");
			s += _T("Shield Energy increase: 10%\n \n");
			s += _T("Shortcut: W");
			pHUD->SetButtonInfo(1, s);
			pHUD->SetButtonTooltip(1, _T("Upgrade Shields"));
		}

		pHUD->SetButtonListener(2, CHUD::PromoteMovement);
		pHUD->SetButtonTexture(2, "UpgradeMovement");
		pHUD->SetButtonColor(2, Color(150, 150, 50));

		tstring s2;
		s2 += _T("UPGRADE MOVEMENT ENERGY\n \n");
		s2 += _T("This upgrade overclocks your tank's engines, increasing the maximum Movement Energy available to your tank past its normal levels. With this you'll spend less energy moving your tank around.\n \n");
		s2 += _T("Movement Energy increase: 10%\n \n");
		s2 += _T("Shortcut: E");
		pHUD->SetButtonInfo(2, s2);
		pHUD->SetButtonTooltip(2, _T("Upgrade Movement"));

		pHUD->SetButtonListener(9, CHUD::GoToMain);
		pHUD->SetButtonTexture(9, "Cancel");
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, _T("RETURN\n \nShortcut: G"));
		pHUD->SetButtonTooltip(9, _T("Return"));
	}
}

void CDigitank::Fire()
{
	float flDistanceSqr = (m_vecPreviewAim - GetRealOrigin()).LengthSqr();
	if (!IsInsideMaxRange(m_vecPreviewAim))
		return;

	if (flDistanceSqr < GetMinRange()*GetMinRange())
		return;

	if (fabs(AngleDifference(GetGlobalAngles().y, VectorAngles((GetPreviewAim() - GetRealOrigin()).Normalized()).y)) > FiringCone())
		return;

	if (m_bFiredWeapon || m_bActionTaken)
		return;

	if (m_flTotalPower < GetWeaponEnergy())
		return;

	if (CanFortify() && !IsFortified() && !CanAimMobilized())
		return;

	if (IsDisabled())
		return;

	if (GameNetwork()->ShouldReplicateClientFunction())
		GameNetwork()->CallFunction(NETWORK_TOEVERYONE, "Fire", GetHandle(), m_vecPreviewAim.x, m_vecPreviewAim.y, m_vecPreviewAim.z);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = m_vecPreviewAim.x;
	p.fl3 = m_vecPreviewAim.y;
	p.fl4 = m_vecPreviewAim.z;
	Fire(&p);

	DigitanksWindow()->SetContextualCommandsOverride(true);
}

void CDigitank::Fire(CNetworkParameters* p)
{
	m_vecPreviewAim = Vector(p->fl2, p->fl3, p->fl4);

	m_vecLastAim = m_vecPreviewAim;
	m_bFiredWeapon = true;

	float flAttackPower = GetWeaponEnergy();

	if (GameNetwork()->IsHost())
	{
		m_flTotalPower -= flAttackPower;
		m_flAttackPower += flAttackPower;
	}

	if (GetVisibility() > 0)
	{
		EmitSound(_T("sound/tank-aim.wav"));
		SetSoundVolume(_T("sound/tank-aim.wav"), 0.5f);
	}

	if (GameNetwork()->IsHost())
	{
		if (IsMoving())
			m_flFireWeaponTime += m_flStartedMove.Get() + GetTransitionTime() + FirstProjectileTime();
		else
			m_flFireWeaponTime = GameServer()->GetGameTime() + FirstProjectileTime();
		m_iFireWeapons = CProjectile::GetWeaponShells(GetCurrentWeapon());

		if (GetCurrentWeapon() == PROJECTILE_CAMERAGUIDED)
			m_flFireWeaponTime = GameServer()->GetGameTime();

		m_bLostConcealment = true;
		m_bCloaked = false;
	}

	Speak(TANKSPEECH_ATTACK);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	DirtyNeedsOrders();

	DigitanksWindow()->GetHUD()->UpdateTurnButton();
}

void CDigitank::FireWeapon()
{
	float flDistanceSqr = (m_vecLastAim - GetGlobalOrigin()).LengthSqr();
	if (!IsInsideMaxRange(m_vecLastAim))
		return;

	if (flDistanceSqr < GetMinRange()*GetMinRange())
		return;

	Vector vecLandingSpot = m_vecLastAim;

	float flFactor = FindAimRadius(m_vecLastAim, MinRangeRadius());

	float flYaw = RandomFloat(0, 360);
	float flRadius = RandomFloat(1, flFactor);

	// Don't use uniform distribution, I like how it's clustered on the target.
	vecLandingSpot += Vector(flRadius*cos(flYaw), 0, flRadius*sin(flYaw));

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	m_hWeapon = CreateWeapon();

	if (!m_hWeapon)
		return;

	CProjectile* pProjectile = dynamic_cast<CProjectile*>(m_hWeapon.GetPointer());
	if (pProjectile)
		DigitanksGame()->AddProjectileToWaitFor();

	if (GameNetwork()->ShouldReplicateClientFunction())
		GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "FireWeapon", GetHandle(), m_hWeapon->GetHandle(), vecLandingSpot.x, vecLandingSpot.y, vecLandingSpot.z);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = m_hWeapon->GetHandle();
	p.fl3 = vecLandingSpot.x;
	p.fl4 = vecLandingSpot.y;
	p.fl5 = vecLandingSpot.z;
	FireWeapon(&p);
}

void CDigitank::FireWeapon(CNetworkParameters* p)
{
	m_hWeapon = CEntityHandle<CDigitanksWeapon>(p->ui2);

	Vector vecLandingSpot = Vector(p->fl3, p->fl4, p->fl5);

	m_hWeapon->SetOwner(this);

	CProjectile* pProjectile = dynamic_cast<CProjectile*>(m_hWeapon.GetPointer());
	if (pProjectile)
		FireProjectile(pProjectile, vecLandingSpot);

	if (GetVisibility() > 0)
	{
		EmitSound(_T("sound/tank-fire.wav"));
	}

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
}

void CDigitank::FireProjectile(CProjectile* pProjectile, Vector vecLandingSpot)
{
	CDigitanksPlayer* pCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();

	bool bIsVisible = false;
	if (pCurrentTeam && pCurrentTeam->GetVisibilityAtPoint(GetGlobalOrigin()) > 0.3f)
		bIsVisible = true;

	if (pCurrentTeam && pCurrentTeam->GetVisibilityAtPoint(vecLandingSpot) > 0.3f)
		bIsVisible = true;

	if (GetDigitanksPlayer() && GetDigitanksPlayer()->IsHumanControlled())
		bIsVisible = true;

	float flGravity = DigitanksGame()->GetGravity();

	if (!bIsVisible)
		flGravity *= 20;

	float flTime;
	Vector vecForce;
	FindLaunchVelocity(GetGlobalOrigin(), vecLandingSpot, flGravity, vecForce, flTime, ProjectileCurve());

	pProjectile->SetGlobalVelocity(vecForce);
	pProjectile->SetGlobalGravity(Vector(0, 0, flGravity));
	pProjectile->SetLandingSpot(vecLandingSpot);

	if (GetVisibility() > 0)
	{
		Vector vecMuzzle = (vecLandingSpot - GetGlobalOrigin()).Normalized() * 3 + Vector(0, 0, 3);
		CNetworkedEffect::AddInstance(_T("tank-fire"), GetGlobalOrigin() + vecMuzzle);
	}

	RockTheBoat(0.8f, -vecForce.Normalized());

	// Follow the first if there's only one or the third if there's many, such as the resistor machine gun.
	size_t iProjectiles = CProjectile::GetWeaponShells(GetCurrentWeapon());
	if (iProjectiles == 1)
		DigitanksGame()->GetOverheadCamera()->ProjectileFired(pProjectile);
	else
	{
		if (iProjectiles <= 3 && m_iFireWeapons == iProjectiles-1)
			DigitanksGame()->GetOverheadCamera()->ProjectileFired(pProjectile);
		else if (m_iFireWeapons == 3)
			DigitanksGame()->GetOverheadCamera()->ProjectileFired(pProjectile);
	}
}

CDigitanksWeapon* CDigitank::CreateWeapon()
{
	if (GetCurrentWeapon() == PROJECTILE_ARTILLERY)
		return GameServer()->Create<CArtilleryShell>("CArtilleryShell");
	else if (GetCurrentWeapon() == PROJECTILE_ARTILLERY_AOE)
		return GameServer()->Create<CArtilleryAoE>("CArtilleryAoE");
	else if (GetCurrentWeapon() == PROJECTILE_ARTILLERY_ICBM)
		return GameServer()->Create<CArtilleryICBM>("CArtilleryICBM");
	else if (GetCurrentWeapon() == PROJECTILE_DEVASTATOR)
		return GameServer()->Create<CDevastator>("CDevastator");
	else if (GetCurrentWeapon() == PROJECTILE_FLAK)
		return GameServer()->Create<CInfantryFlak>("CInfantryFlak");
	else if (GetCurrentWeapon() == PROJECTILE_TREECUTTER)
		return GameServer()->Create<CTreeCutter>("CTreeCutter");
	else if (GetCurrentWeapon() == WEAPON_INFANTRYLASER)
		return GameServer()->Create<CInfantryLaser>("CInfantryLaser");
	else if (GetCurrentWeapon() == PROJECTILE_TORPEDO)
		return GameServer()->Create<CTorpedo>("CTorpedo");
	else if (GetCurrentWeapon() == PROJECTILE_SMALL)
		return GameServer()->Create<CSmallShell>("CSmallShell");
	else if (GetCurrentWeapon() == PROJECTILE_MEDIUM)
		return GameServer()->Create<CMediumShell>("CMediumShell");
	else if (GetCurrentWeapon() == PROJECTILE_LARGE)
		return GameServer()->Create<CLargeShell>("CLargeShell");
	else if (GetCurrentWeapon() == PROJECTILE_AOE)
		return GameServer()->Create<CAOEShell>("CAOEShell");
	else if (GetCurrentWeapon() == PROJECTILE_TRACTORBOMB)
		return GameServer()->Create<CTractorBomb>("CTractorBomb");
	else if (GetCurrentWeapon() == PROJECTILE_SPLOOGE)
		return GameServer()->Create<CSploogeShell>("CSploogeShell");
	else if (GetCurrentWeapon() == PROJECTILE_ICBM)
		return GameServer()->Create<CICBM>("CICBM");
	else if (GetCurrentWeapon() == PROJECTILE_EMP)
		return GameServer()->Create<CEMP>("CEMP");
	else if (GetCurrentWeapon() == PROJECTILE_GRENADE)
		return GameServer()->Create<CGrenade>("CGrenade");
	else if (GetCurrentWeapon() == PROJECTILE_EARTHSHAKER)
		return GameServer()->Create<CEarthshaker>("CEarthshaker");
	else if (GetCurrentWeapon() == PROJECTILE_CAMERAGUIDED)
		return GameServer()->Create<CCameraGuidedMissile>("CCameraGuidedMissile");
	else if (GetCurrentWeapon() == PROJECTILE_DAISYCHAIN)
		return GameServer()->Create<CDaisyChain>("CDaisyChain");
	else if (GetCurrentWeapon() == PROJECTILE_CLUSTERBOMB)
		return GameServer()->Create<CClusterBomb>("CClusterBomb");
	else if (GetCurrentWeapon() == WEAPON_LASER)
		return GameServer()->Create<CLaser>("CLaser");
	else if (GetCurrentWeapon() == WEAPON_CHARGERAM)
		return NULL;

	TAssert(!"Unrecognized projectile");
	return GameServer()->Create<CSmallShell>("CSmallShell");
}

CLIENT_GAME_COMMAND(SetCurrentWeapon)
{
	if (pCmd->GetNumArguments() < 2)
	{
		TMsg("SetCurrentWeapon with less than 2 parameters.\n");
		return;
	}

	CEntityHandle<CDigitank> hTank(pCmd->ArgAsUInt(0));
	weapon_t eWeapon = (weapon_t)pCmd->ArgAsInt(1);

	if (!hTank)
	{
		TMsg("SetCurrentWeapon on invalid tank.\n");
		return;
	}

	if (GameNetwork()->IsRunningClientFunctions() && (!hTank->GetPlayerOwner() || hTank->GetPlayerOwner()->GetClient() != (int)iClient))
	{
		TMsg("SetCurrentWeapon for a tank the player doesn't own.\n");
		return;
	}

	if (!hTank->HasWeapon(eWeapon))
	{
		TMsg("SetCurrentWeapon for a weapon that tank doesn't own.\n");
		return;
	}

	hTank->SetCurrentWeapon(eWeapon, false);
}

void CDigitank::SetCurrentWeapon(weapon_t e, bool bNetworked)
{
	if (bNetworked)
		::SetCurrentWeapon.RunCommand(tsprintf(tstring("%d %d"), GetHandle(), e));

	if (!DigitanksGame()->IsWeaponAllowed(e, this))
		return;

	m_eWeapon = e;
}

float CDigitank::GetWeaponEnergy() const
{
	return CProjectile::GetWeaponEnergy(m_eWeapon);
}

size_t CDigitank::GetNumWeapons() const
{
	return m_aeWeapons.size();
}

size_t CDigitank::GetNumAllowedWeapons() const
{
	size_t iWeapons = 0;
	size_t iTotalWeapons = m_aeWeapons.size();
	for (size_t i = 0; i < iTotalWeapons; i++)
	{
		if (DigitanksGame()->IsWeaponAllowed(m_aeWeapons[i], this))
			iWeapons++;
	}

	return iWeapons;
}

bool CDigitank::HasWeapon(weapon_t eWeapon) const
{
	for (size_t i = 0; i < m_aeWeapons.size(); i++)
		if (m_aeWeapons[i] == eWeapon)
			return true;

	return false;
}

CLIENT_GAME_COMMAND(FireSpecial)
{
	if (pCmd->GetNumArguments() < 4)
	{
		TMsg("FireSpecial with not enough arguments.\n");
		return;
	}

	CEntityHandle<CDigitank> hTank(pCmd->ArgAsInt(0));

	if (!hTank)
	{
		TMsg("FireSpecial with invalid tank.\n");
		return;
	}

	hTank->FireSpecial(Vector(pCmd->ArgAsFloat(1), pCmd->ArgAsFloat(2), pCmd->ArgAsFloat(3)));
}

void CDigitank::FireSpecial()
{
	::FireSpecial.RunCommand(tsprintf(tstring("%d %f %f %f"), GetHandle(), GetPreviewAim().x, GetPreviewAim().y, GetPreviewAim().z));

	DigitanksGame()->SetControlMode(MODE_NONE);
}

void CDigitank::FireSpecial(Vector vecAim)
{
	if (m_iAirstrikes <= 0)
		return;

	if (m_bActionTaken)
		return;

	m_iAirstrikes--;

	DigitanksGame()->BeginAirstrike(vecAim);

	m_bActionTaken = true;
	m_bFiredWeapon = true;

	DirtyNeedsOrders();

	DigitanksGame()->SetControlMode(MODE_NONE);
}

bool CDigitank::HasSpecialWeapons()
{
	return m_iAirstrikes > 0;
}

void CDigitank::FireMissileDefense(CProjectile* pTarget)
{
	if (m_iMissileDefenses <= 0)
		return;

	if (!pTarget)
		return;

	if (GameServer()->GetGameTime() < m_flNextMissileDefense)
		return;

	m_iMissileDefenses--;

	CMissileDefense* pMissileDefense = GameServer()->Create<CMissileDefense>("CMissileDefense");
	pMissileDefense->SetOwner(this);
	pMissileDefense->SetTarget(pTarget);

	// Can't fire more than one at a time. This way the missile defenses can be overloaded with multiple projectiles.
	m_flNextMissileDefense = GameServer()->GetGameTime() + 1.0f;
}

bool CDigitank::CanFireMissileDefense()
{
	if (GameServer()->GetGameTime() < m_flNextMissileDefense)
		return false;

	return m_iMissileDefenses > 0;
}

void CDigitank::ClientUpdate(int iClient)
{
	BaseClass::ClientUpdate(iClient);
}

void CDigitank::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit)
{
	if (!m_bTakeDamage)
		return;

	if (eDamageType == DAMAGE_LASER)
		CallOutput("OnTakeLaserDamage");

	if (GetDigitanksPlayer())
	{
		if (flDamage > 0)
		{
			CancelGoalMovePosition();
			GetDigitanksPlayer()->AddActionItem(this, ACTIONTYPE_AUTOMOVECANCELED);
		}
		else
			GetDigitanksPlayer()->AddActionItem(this, ACTIONTYPE_UNITDAMAGED);
	}

	if (pInflictor)
	{
		Vector vecLookAt = (pInflictor->GetGlobalOrigin() - GetGlobalOrigin()).Normalized();
		m_flGoalTurretYaw = atan2(vecLookAt.z, vecLookAt.x) * 180/M_PI - GetRenderAngles().y;
	}

	Speak(TANKSPEECH_DAMAGED);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	if (IsSentried())
		Sentry();

	size_t iDifficulty = DigitanksGame()->GetDifficulty();

	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);

	if (pProjectile && pProjectile->GetBonusDamage() > 0)
	{
		DigitanksGame()->OnCritical(this, pAttacker, pInflictor);
		bDirectHit = false;
	}

	if (!GameNetwork()->IsConnected())
	{
		if (DigitanksGame()->IsTeamControlledByMe(GetPlayerOwner()))
		{
			if (iDifficulty == 0)
				flDamage *= 0.5f;
			else if (iDifficulty == 1)
				flDamage *= 0.75f;
			else if (iDifficulty == 2)
				flDamage *= 1.0f;
			else if (iDifficulty == 2)
				flDamage *= 1.2f;
		}
		else if (dynamic_cast<CDigitank*>(pAttacker) && DigitanksGame()->IsTeamControlledByMe(dynamic_cast<CDigitank*>(pAttacker)->GetPlayerOwner()))
		{
			if (iDifficulty == 0)
				flDamage *= 2.0f;
			else if (iDifficulty == 1)
				flDamage *= 1.5f;
			else if (iDifficulty == 2)
				flDamage *= 1.0f;
			else if (iDifficulty == 3)
				flDamage *= 0.8f;
		}
	}

	Vector vecAttackOrigin;
	if (bDirectHit)
	{
		if (pAttacker)
			vecAttackOrigin = pAttacker->GetGlobalOrigin();
	}
	else if (pInflictor)
		vecAttackOrigin = pInflictor->GetGlobalOrigin();

	float flShield = GetShieldValue();

	float flDamageBlocked = flShield * GetDefenseScale();

	// Lava burns bypass shields
	if (eDamageType == DAMAGE_BURN)
		flDamageBlocked = 0;
	else if (eDamageType == DAMAGE_COLLISION)
	{
		// Looks like a charge to me. Charges bypass shields.
		flDamageBlocked = 0;

		// Force the tank damage sound, suppress the shield damage sound.
		flShield = 0;
	}

	if (GameNetwork()->IsHost() && (eDamageType == DAMAGE_COLLISION || eDamageType == DAMAGE_EXPLOSION || eDamageType == DAMAGE_LASER || eDamageType == DAMAGE_GENERIC))
	{
		size_t iDebris = (int)(flDamage/10);
		if (iDebris < 0)
			iDebris = 1;

		if (iDebris > 10)
			iDebris = 10;

		for (size_t i = 0; i < iDebris; i++)
		{
			CDebris* pDebris = GameServer()->Create<CDebris>("CDebris");
			pDebris->SetGlobalOrigin(GetGlobalOrigin());
		}
	}

	float flShieldDamageScale = 1;
	if (pProjectile)
		flShieldDamageScale = pProjectile->ShieldDamageScale();

	if (flDamage*flShieldDamageScale - flDamageBlocked < 0)
	{
		SetShieldValue(flShield - flDamage*flShieldDamageScale / GetDefenseScale());

		if (GetVisibility() > 0)
		{
			EmitSound(_T("sound/shield-damage.wav"));
			SetSoundVolume(_T("sound/shield-damage.wav"), RemapValClamped(flDamage*flShieldDamageScale, 0, 50, 0, 0.5f));
		}

		CallOutput("OnTakeShieldDamage");
		DigitanksGame()->OnTakeShieldDamage(this, pAttacker, pInflictor, flDamage*flShieldDamageScale, bDirectHit, true);

		m_flShieldPulse = GameServer()->GetGameTime();

		return;
	}

	if (flDamageBlocked > 0)
		m_flShieldPulse = GameServer()->GetGameTime();

	if (flShieldDamageScale > 0)
		flDamage -= flDamageBlocked/flShieldDamageScale;
	else
		flDamage -= flDamageBlocked;

	if (pProjectile)
		flDamage *= pProjectile->HealthDamageScale();

	if (GetVisibility() > 0)
	{
		EmitSound(_T("sound/tank-damage.wav"));
		SetSoundVolume(_T("sound/tank-damage.wav"), RemapValClamped(flDamage, 0.0f, 50.0f, 0.0f, 1.0f));
	}

	if (flShield > 1.0f && !IsDisabled())
	{
		if (GetVisibility() > 0)
		{
			EmitSound(_T("sound/shield-damage.wav"));
			SetSoundVolume(_T("sound/shield-damage.wav"), RemapValClamped(flShield, 0.0f, 50.0f, 0.0f, 1.0f));
		}
	}

	// Disabled tanks don't take damage to shields. When their shields come back up they're perfectly okay.
	if (!IsDisabled())
	{
		if (eDamageType != DAMAGE_BURN)
			SetShieldValue(0);

		CallOutput("OnTakeShieldDamage");
		DigitanksGame()->OnTakeShieldDamage(this, pAttacker, pInflictor, flDamageBlocked, bDirectHit, false);
	}

	BaseClass::TakeDamage(pAttacker, pInflictor, eDamageType, flDamage, bDirectHit);
}

void CDigitank::OnKilled(CBaseEntity* pKilledBy)
{
	CDigitank* pKiller = dynamic_cast<CDigitank*>(pKilledBy);

	if (pKiller)
	{
		pKiller->GiveBonusPoints(1);
		pKiller->Speak(TANKSPEECH_KILL);
		pKiller->m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}

	// Make sure we can see that we got a promotion.
	DigitanksWindow()->GetHUD()->Layout();

	if (m_hFortifyDefending != NULL)
		m_hFortifyDefending->RemoveDefender(this);
}

const TVector CDigitank::GetGlobalOrigin() const
{
	TStubbed("CDigitank::GetGlobalOrigin()");
	return BaseClass::GetGlobalOrigin();

#if 0
	float flTransitionTime = GetTransitionTime();

	if (!GameNetwork()->IsConnected() || GetDigitanksPlayer() && !GetDigitanksPlayer()->IsHumanControlled())
	{
		if (GetVisibility() == 0)
			flTransitionTime = 0;
	}

	float flTimeSinceMove = (float)(GameServer()->GetGameTime() - m_flStartedMove);
	if (m_flStartedMove && flTimeSinceMove < flTransitionTime)
	{
		float flLerp = 0;
		float flRamp = RemapValClamped(flTimeSinceMove, 0.0f, flTransitionTime, 0.0f, 1.0f);
		if (m_iMoveType == 0)
			flLerp = Bias(flRamp, 0.2f);
		else if (m_iMoveType == 1)
			flLerp = Gain(flRamp, 0.2f);
		else
			flLerp = Gain(flRamp, 0.8f);

		Vector vecNewOrigin = LerpValue<Vector>(m_vecPreviousOrigin, BaseClass::GetGlobalOrigin(), flLerp);

		float flHoverHeight = FindHoverHeight(vecNewOrigin);
		if (vecNewOrigin.y < flHoverHeight)
			vecNewOrigin.y = flHoverHeight;

		return vecNewOrigin;
	}

	return BaseClass::GetGlobalOrigin();
#endif
}

Vector CDigitank::GetRealOrigin() const
{
	return BaseClass::GetGlobalOrigin();
}

const EAngle CDigitank::GetGlobalAngles() const
{
	TStubbed("CDigitank::GetGlobalAngles()");
	return BaseClass::GetGlobalAngles();

#if 0
	float flTransitionTime = GetTransitionTime();

	if (!GameNetwork()->IsConnected() || GetDigitanksPlayer() && !GetDigitanksPlayer()->IsHumanControlled())
	{
		if (GetVisibility() == 0)
			flTransitionTime = 0;
	}

	float flTimeSinceTurn = (float)(GameServer()->GetGameTime() - m_flStartedTurn);
	if (m_flStartedTurn && flTimeSinceTurn < flTransitionTime)
	{
		float flLerp = Gain(RemapVal(flTimeSinceTurn, 0, flTransitionTime, 0, 1), 0.2f);
		float flAngleDiff = AngleDifference(BaseClass::GetGlobalAngles().y, m_flPreviousTurn);
		float flNewTurn = m_flPreviousTurn + flAngleDiff * flLerp;
		return EAngle(0, flNewTurn, 0);
	}

	return BaseClass::GetGlobalAngles();
#endif
}

void CDigitank::PreRender() const
{
	BaseClass::PreRender();

	if (GameServer()->GetRenderer()->IsRenderingTransparent() && DigitanksGame()->GetGameType() == GAMETYPE_STANDARD && GetPlayerOwner() && CSupplier::GetDataFlow(GetGlobalOrigin(), GetPlayerOwner()) > 0)
	{
		CRenderer* pRenderer = GameServer()->GetRenderer();

		Vector vecForward, vecRight, vecUp;
		pRenderer->GetCameraVectors(&vecForward, &vecRight, &vecUp);

		CRenderingContext c(pRenderer, true);

		Vector vecOrigin = GetRenderOrigin();
		Vector vecParticleUp, vecParticleRight;
		float flRadius = GetBoundingRadius()*2;

		c.UseMaterial(s_hSupportGlow);
		c.SetAlpha(0.5f);
		c.SetDepthMask(false);
		c.SetDepthTest(false);
		c.SetBlend(BLEND_ADDITIVE);
		Color clrTeam = GetPlayerOwner()->GetColor();
		clrTeam.SetAlpha((int)(255 * GetVisibility()));
		c.SetColor(clrTeam);
		c.BeginRenderTris();

			float flYaw = m_flGlowYaw*M_PI/180;
			float flSin = sin(flYaw);
			float flCos = cos(flYaw);

			vecParticleUp = (flCos*vecUp + flSin*vecRight)*flRadius;
			vecParticleRight = (flCos*vecRight - flSin*vecUp)*flRadius;

			Vector vecTL = vecOrigin - vecParticleRight + vecParticleUp;
			Vector vecTR = vecOrigin + vecParticleRight + vecParticleUp;
			Vector vecBL = vecOrigin - vecParticleRight - vecParticleUp;
			Vector vecBR = vecOrigin + vecParticleRight - vecParticleUp;

			c.TexCoord(Vector2D(0, 1));
			c.Vertex(vecTL);
			c.TexCoord(Vector2D(0, 0));
			c.Vertex(vecBL);
			c.TexCoord(Vector2D(1, 0));
			c.Vertex(vecBR);

			c.TexCoord(Vector2D(0, 1));
			c.Vertex(vecTL);
			c.TexCoord(Vector2D(1, 0));
			c.Vertex(vecBR);
			c.TexCoord(Vector2D(1, 1));
			c.Vertex(vecTR);

			// Do it again rotating the other way.
			flYaw = -m_flGlowYaw*M_PI/180;
			flSin = sin(flYaw);
			flCos = cos(flYaw);

			vecParticleUp = (flCos*vecUp + flSin*vecRight)*flRadius;
			vecParticleRight = (flCos*vecRight - flSin*vecUp)*flRadius;

			vecTL = vecOrigin - vecParticleRight + vecParticleUp;
			vecTR = vecOrigin + vecParticleRight + vecParticleUp;
			vecBL = vecOrigin - vecParticleRight - vecParticleUp;
			vecBR = vecOrigin + vecParticleRight - vecParticleUp;
			
			c.TexCoord(Vector2D(0, 1));
			c.Vertex(vecTL);
			c.TexCoord(Vector2D(0, 0));
			c.Vertex(vecBL);
			c.TexCoord(Vector2D(1, 0));
			c.Vertex(vecBR);

			c.TexCoord(Vector2D(0, 1));
			c.Vertex(vecTL);
			c.TexCoord(Vector2D(1, 0));
			c.Vertex(vecBR);
			c.TexCoord(Vector2D(1, 1));
			c.Vertex(vecTR);
			
		c.EndRender();
	}
}

Vector CDigitank::GetRenderOrigin() const
{
	float flLerp = 0;
	float flHoverHeight = 0;
	
	if (!IsFortified() && !IsFortifying() && !IsImprisoned())
	{
		float flOscillate = Oscillate((float)GameServer()->GetGameTime()+m_flBobOffset, 4);
		flLerp = Gain(flOscillate, 0.2f);
		flHoverHeight = 1 + flLerp*BobHeight();
	}

	Vector vecChargeShake = Vector(0, 0, 0);
	Vector vecMaxChargeShake = Vector(0, 0, 0);
	if (m_flBeginCharge > 0 || m_flEndCharge > 0)
		vecMaxChargeShake = Vector(RandomFloat(-0.2f, 0.2f), RandomFloat(-0.2f, 0.2f), RandomFloat(-0.2f, 0.2f));

	if (m_flBeginCharge > 0)
		vecChargeShake = vecMaxChargeShake * (float)RemapValClamped(GameServer()->GetGameTime(), m_flBeginCharge.Get() - GetTransitionTime(), m_flBeginCharge, 0.0, 1.0);

	if (m_flEndCharge > 0)
		vecChargeShake = vecMaxChargeShake;

	return GetGlobalOrigin() + Vector(0, 0, flHoverHeight) + vecChargeShake;
}

EAngle CDigitank::GetRenderAngles() const
{
	if (!GetPlayerOwner())
		return BaseClass::GetRenderAngles();

	if (GetDigitanksPlayer()->IsPrimarySelection(this))
	{
		if (DigitanksGame()->GetControlMode() == MODE_TURN && GetPreviewTurnPower() <= GetRemainingMovementEnergy())
			return EAngle(0, GetPreviewTurn(), 0);
	}

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
	{
		if (TurnsWith(DigitanksGame()->GetPrimarySelectionTank()))
		{
			Vector vecLookAt;
			bool bMouseOK = DigitanksWindow()->GetMouseGridPosition(vecLookAt);
			bool bNoTurn = bMouseOK && (vecLookAt - DigitanksGame()->GetPrimarySelectionTank()->GetGlobalOrigin()).LengthSqr() < 3*3;

			if (!bNoTurn && bMouseOK)
			{
				Vector vecDirection = (vecLookAt - GetGlobalOrigin()).Normalized();
				float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

				float flTankTurn = AngleDifference(flYaw, GetGlobalAngles().y);
				if (fabs(flTankTurn)/TurnPerPower() > GetRemainingMovementEnergy())
					flTankTurn = (flTankTurn / fabs(flTankTurn)) * GetRemainingMovementEnergy() * TurnPerPower() * 0.95f;

				return EAngle(0, GetGlobalAngles().y + flTankTurn, 0);
			}
		}
	}

	if (IsRocking())
	{
		EAngle angReturn;
		angReturn.y = GetGlobalAngles().y;

		Vector vecForward, vecRight;
		AngleVectors(GetGlobalAngles(), &vecForward, &vecRight, NULL);
		float flDotForward = -m_vecRockDirection.Get().Dot(vecForward.Normalized());
		float flDotRight = -m_vecRockDirection.Get().Dot(vecRight.Normalized());

		float flLerp = Bias(1-Oscillate((float)(GameServer()->GetGameTime() - m_flStartedRock), 1), 0.7f);

		angReturn.p = flDotForward*flLerp*m_flRockIntensity*45;
		angReturn.r = flDotRight*flLerp*m_flRockIntensity*45;

		return angReturn;
	}

	return GetGlobalAngles();
}

bool CDigitank::ModifyShader(CRenderingContext* pContext) const
{
	if (GetPlayerOwner() && pContext->GetActiveShader())
	{
		pContext->SetUniform("bColorSwapInAlpha", true);
		pContext->SetUniform("vecColorSwap", GetPlayerOwner()->GetColor());
	}

	return BaseClass::ModifyShader(pContext);
}

void CDigitank::OnRender(class CGameRenderingContext* pContext) const
{
	BaseClass::OnRender(pContext);

	RenderTurret();

	if (GameServer()->GetRenderer()->IsRenderingTransparent())
		RenderShield();
}

void CDigitank::RenderTurret(float flAlpha) const
{
	if (m_iTurretModel == ~0)
		return;

	if (GetVisibility() == 0 || flAlpha == 0)
		return;

	float flVisibility = flAlpha*GetVisibility();

	if (GameServer()->GetRenderer()->IsRenderingTransparent() && flVisibility == 1)
		return;

	if (!GameServer()->GetRenderer()->IsRenderingTransparent() && flVisibility < 1)
		return;

	CGameRenderingContext r(GameServer()->GetRenderer(), true);

	r.UseProgram("model");

	if (GameServer()->GetRenderer()->IsRenderingTransparent() && flVisibility < 1)
	{
		r.SetAlpha(flVisibility);
		r.SetBlend(BLEND_ALPHA);
	}

	r.Translate(Vector(-0.0f, 0, 0.810368f));

	r.Rotate(m_flCurrentTurretYaw, Vector(0, 0, 1));

	if (IsDisabled())
		r.Rotate(-35, Vector(0, 1, 0));

	if (GetPlayerOwner())
	{
		r.SetUniform("bColorSwapInAlpha", true);
		r.SetUniform("vecColorSwap", GetPlayerOwner()->GetColor());
	}

	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD && (GetUnitType() == UNIT_GRIDBUG || GetUnitType() == UNIT_BUGTURRET))
	{
		r.SetUniform("bColorSwapInAlpha", true);
		r.SetUniform("vecColorSwap", Vector(DigitanksGame()->GetTerrain()->GetPrimaryTerrainColor())*2/3);
	}

	r.RenderModel(m_iTurretModel);
}

void CDigitank::RenderShield() const
{
	if (m_iShieldModel == ~0)
		return;

	if (GetVisibility() == 0)
		return;

	float flFlicker = 1;
	
	if (GetShieldValue() < GetShieldMaxStrength()*3/4)
		flFlicker = Flicker("zzzzmmzzztzzzzzznzzz", (float)GameServer()->GetGameTime() + ((float)GetSpawnSeed()/100), 1.0f);

	CGameRenderingContext r(GameServer()->GetRenderer(), true);

	if (m_flShieldPulse == 0.0)
		return;

	float flPulseAlpha = (float)RemapValClamped(GameServer()->GetGameTime(), m_flShieldPulse, m_flShieldPulse + 1.0, 0.8, 0);

	float flFinalAlpha = flPulseAlpha*flFlicker*GetVisibility();

	if (flFinalAlpha <= 0)
		return;

	r.UseProgram("scroll");
	r.SetUniform("iTexture", 0);
	r.SetUniform("flAlpha", flFinalAlpha);
	r.SetUniform("flTime", -(float)GameServer()->GetGameTime());
	r.SetUniform("flSpeed", 1.0f);

	r.SetBlend(BLEND_ADDITIVE);
	r.Scale(RenderShieldScale(), RenderShieldScale(), RenderShieldScale());
	r.SetDepthTest(false);
	r.SetColor(Color(255, 255, 255, 255));

	r.RenderModel(m_iShieldModel);
}

bool CDigitank::IsTouching(CBaseEntity* pOther, Vector& vecPoint) const
{
	return BaseClass::IsTouching(pOther, vecPoint);
}

float CDigitank::AvailableArea(int iArea) const
{
	return GetBoundingRadius();
}

bool CDigitank::IsAvailableAreaActive(int iArea) const
{
	if (!GetDigitanksPlayer())
		return false;

	if (!DigitanksGame()->GetCurrentLocalDigitanksPlayer())
		return false;

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() == GetDigitanksPlayer())
		return false;

	if (DigitanksGame()->GetControlMode() != MODE_AIM)
		return false;

	CDigitank* pTank = DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetPrimarySelectionTank();

	if (!pTank)
		return false;

	if (!pTank->IsInsideMaxRange(GetGlobalOrigin()))
		return false;

	if (pTank->GetCurrentWeapon() == PROJECTILE_TREECUTTER)
		return false;

	if (pTank->GetCurrentWeapon() == PROJECTILE_EARTHSHAKER)
		return false;

	if (IsScout() && pTank->GetCurrentWeapon() != WEAPON_INFANTRYLASER)
		return false;

	if (GetVisibility(pTank->GetDigitanksPlayer()) < 0.4f)
		return false;

	if (pTank->FiringCone() < 360 && fabs(AngleDifference(pTank->GetGlobalAngles().y, VectorAngles((GetGlobalOrigin() - pTank->GetGlobalOrigin()).Normalized()).y)) > pTank->FiringCone())
		return false;

	return true;
}

void CDigitank::DrawSchema(float x, float y, float w, float h)
{
	CRenderingContext c(GameServer()->GetRenderer(), true);
	c.SetBlend(BLEND_ALPHA);

	float flSize = 36;
	DigitanksWindow()->GetHUD()->PaintWeaponSheet(GetCurrentWeapon(), x - 20, y + h - flSize + 10, flSize, flSize);

	int iIconFontSize = 11;
	tstring sFont = _T("text");

	float flYPosition = (float)y + h;
	float flXPosition = (float)x + w + 20;

	if (HasGoalMovePosition())
	{
		float flDistance = (GetRealOrigin() - GetGoalMovePosition()).Length();
		int iTurns = (int)(flDistance/GetMaxMovementDistance());

		tstring sTurns = tsprintf(tstring("Auto-Move: %d"), iTurns);
		float flWidth = glgui::RootPanel()->GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);
		glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, (float)y);
	}

	float flIconFontHeight = glgui::RootPanel()->GetFontHeight(sFont, iIconFontSize) + 2;

	if (IsFortified() || IsFortifying())
	{
		tstring sTurns = tsprintf(tstring("+%d"), GetFortifyLevel());
		float flWidth = glgui::RootPanel()->GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);

		const Rect& rArea = CHUD::GetButtonSheet().GetArea("Fortify");
		glgui::CBaseControl::PaintSheet(CHUD::GetButtonSheet().GetSheet("Fortify"), (flXPosition - flWidth - flIconFontHeight), (flYPosition - flIconFontHeight) + 5, flIconFontHeight, flIconFontHeight,
			rArea.x, rArea.y, rArea.w, rArea.h, CHUD::GetButtonSheet().GetSheetWidth("Fortify"), CHUD::GetButtonSheet().GetSheetHeight("Fortify"));
		glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, flYPosition);

		flYPosition -= flIconFontHeight;
	}
}

void CDigitank::UpdateInfo(tstring& s)
{
	s = _T("");
	tstring p;

	s += GetEntityName();
	s += _T("\n \n");

	if (GetPlayerOwner())
	{
		s += _T("Team: ") + GetPlayerOwner()->GetPlayerName() + _T("\n");
		if (GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
			s += _T(" Friendly\n \n");
		else
			s += _T(" Hostile\n \n");
	}
	else
	{
		s += _T("Team: Neutral\n \n");
	}

	if (IsDisabled())
		s += _T("[Disabled]\n \n");

	if (IsFortified())
		s += _T("[Fortified]\n \n");

	else if (IsFortifying())
		s += _T("[Fortifying...]\n \n");

	if (HasBonusPoints())
	{
		if (GetBonusPoints() > 1)
			s += tsprintf(tstring("%d upgrades\n \n"), GetBonusPoints());
		else
			s += _T("1 upgrade\n \n");
	}

	if (m_flBonusAttackPower > 0)
	{
		s += tsprintf(tstring("+%d attack"), (int)m_flBonusAttackPower.Get());

		if (IsFortified() && (int)GetFortifyAttackPowerBonus() > 0)
			s += tsprintf(tstring(" (+%d fortify)"), (int)GetFortifyAttackPowerBonus());

		if ((int)GetSupportAttackPowerBonus() > 0)
			s += tsprintf(tstring(" (+%d support)"), (int)GetSupportAttackPowerBonus());

		s += _T(" \n");
	}

	if (GetBonusDefensePower())
	{
		s += tsprintf(tstring("+%d shield"), (int)m_flBonusDefensePower.Get());

		if (IsFortified() && (int)GetFortifyDefensePowerBonus() > 0)
			s += tsprintf(tstring(" (+%d fortify)"), (int)GetFortifyDefensePowerBonus());

		if ((int)GetSupportDefensePowerBonus() > 0)
			s += tsprintf(tstring(" (+%d support)"), (int)GetSupportDefensePowerBonus());

		s += _T(" \n");
	}

	if (GetBonusMovementEnergy() > 0)
		s += tsprintf(tstring("+%d movement\n"), (int)GetBonusMovementEnergy());
}

void CDigitank::GiveBonusPoints(size_t i, bool bPlayEffects)
{
	if (!GameNetwork()->IsHost())
		return;

	if (m_iBonusLevel + m_iBonusPoints >= 5)
		return;

	m_iBonusPoints += i;

	if (bPlayEffects)
	{
		DigitanksWindow()->GetHUD()->AddPowerupNotification(this, POWERUP_BONUS);
		TankPromoted(NULL);
		GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "TankPromoted", GetHandle());
	}

	GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints.Get(), m_flBonusAttackPower.Get(), m_flBonusDefensePower.Get(), m_flBonusMovementPower.Get());

	Speak(TANKSPEECH_PROMOTED);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
}

bool CDigitank::HasBonusPoints()
{
	if (m_iBonusLevel >= 5)
		return false;

	return m_iBonusPoints > 0;
}

void CDigitank::PromoteAttack()
{
	if (!GameNetwork()->IsHost())
	{
		GameNetwork()->CallFunction(NETWORK_TOSERVER, "PromoteAttack", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	if (m_iBonusLevel >= 5)
		return;

	m_iBonusLevel++;
	m_iBonusPoints--;
	m_flBonusAttackPower++;

	GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints.Get(), m_flBonusAttackPower.Get(), m_flBonusDefensePower.Get(), m_flBonusMovementPower.Get());

	if (GetPlayerOwner()->IsHumanControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::PromoteDefense()
{
	if (!GameNetwork()->IsHost())
	{
		GameNetwork()->CallFunction(NETWORK_TOSERVER, "PromoteDefense", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	if (m_iBonusLevel >= 5)
		return;

	m_iBonusLevel++;
	m_iBonusPoints--;
	m_flBonusDefensePower++;

	GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints.Get(), m_flBonusAttackPower.Get(), m_flBonusDefensePower.Get(), m_flBonusMovementPower.Get());

	if (GetPlayerOwner()->IsHumanControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::PromoteMovement()
{
	if (!GameNetwork()->IsHost())
	{
		GameNetwork()->CallFunction(NETWORK_TOSERVER, "PromoteMovement", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	if (m_iBonusLevel >= 5)
		return;

	m_iBonusLevel++;
	m_iBonusPoints--;
	m_flBonusMovementPower++;

	GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints.Get(), m_flBonusAttackPower.Get(), m_flBonusDefensePower.Get(), m_flBonusMovementPower.Get());

	if (GetPlayerOwner()->IsHumanControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::DownloadComplete(size_t x, size_t y)
{
	CUpdateItem* pItem = &DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y];

	if (pItem->m_eStructure != GetUnitType())
		return;

	if (pItem->m_eUpdateClass != UPDATECLASS_UNITSKILL)
		return;

	if (pItem->m_eUpdateType == UPDATETYPE_SKILL_CLOAK)
		GiveCloak();
	else if (pItem->m_eUpdateType == UPDATETYPE_WEAPON_AOE)
	{
		if (pItem->m_eStructure == UNIT_TANK)
			m_aeWeapons.push_back(PROJECTILE_AOE);
		else if (pItem->m_eStructure == UNIT_ARTILLERY)
			m_aeWeapons.push_back(PROJECTILE_ARTILLERY_AOE);
	}
	else if (pItem->m_eUpdateType == UPDATETYPE_WEAPON_ICBM)
	{
		if (pItem->m_eStructure == UNIT_TANK)
			m_aeWeapons.push_back(PROJECTILE_ICBM);
		else if (pItem->m_eStructure == UNIT_ARTILLERY)
			m_aeWeapons.push_back(PROJECTILE_ARTILLERY_ICBM);
	}
	else if (pItem->m_eUpdateType == UPDATETYPE_WEAPON_CHARGERAM)
		m_aeWeapons.push_back(WEAPON_CHARGERAM);
	else if (pItem->m_eUpdateType == UPDATETYPE_WEAPON_CLUSTER)
		m_aeWeapons.push_back(PROJECTILE_CLUSTERBOMB);
	else if (pItem->m_eUpdateType == UPDATETYPE_WEAPON_DEVASTATOR)
		m_aeWeapons.push_back(PROJECTILE_DEVASTATOR);

	DigitanksWindow()->GetHUD()->Layout();
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
		EmitSound(_T("sound/tank-promoted.wav"));
		CNetworkedEffect::AddInstance(_T("promotion"), GetRealOrigin());
	}
}

void CDigitank::PromoteAttack(class CNetworkParameters* p)
{
	if (!GameNetwork()->IsHost())
		return;

	PromoteAttack();
}

void CDigitank::PromoteDefense(class CNetworkParameters* p)
{
	if (!GameNetwork()->IsHost())
		return;

	PromoteDefense();
}

void CDigitank::PromoteMovement(class CNetworkParameters* p)
{
	if (!GameNetwork()->IsHost())
		return;

	PromoteMovement();
}

void CDigitank::Speak(size_t iSpeech)
{
	if (!GameNetwork()->IsHost())
		return;

	if (rand()%4 != 0)
		return;

	if (GameServer()->GetGameTime() < m_flLastSpeech + 5.0f)
		return;

	if (GetVisibility() == 0)
		return;

	if (IsCloaked())
		return;

	// No talking when we're hiding in the trees!
	if (DigitanksGame()->GetTerrain()->IsPointInTrees(GetGlobalOrigin()))
		return;

	if (!g_aiSpeechLines.size())
		return;

	size_t iLine = g_aiSpeechLines[iSpeech][rand() % g_aiSpeechLines[iSpeech].size()];

	m_flLastSpeech = GameServer()->GetGameTime();
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "TankSpeak", GetHandle(), iLine);

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
		return vecPosition.z;

	CTerrain* pTerrain = DigitanksGame()->GetTerrain();

	if (!pTerrain)
		return 0;

	Vector vecHit;

	float flHighestTerrain = pTerrain->GetHeight(vecPosition.x, vecPosition.y);
	bool bHit;

	bHit = DigitanksGame()->TraceLine(vecPosition + Vector(0, 0, 100), vecPosition + Vector(0, 0, -100), vecHit, NULL, true);
	if (bHit)
		flHighestTerrain = vecHit.z;

	float flTerrain;

	// Only do one trace to see if we're on a structure. If we are then we'll just have to hope it's not too steep.
	// Five traces really slows things down.
	flTerrain = pTerrain->GetHeight(vecPosition.x+2, vecPosition.y+2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x+2, vecPosition.y-2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x-2, vecPosition.y+2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x-2, vecPosition.y-2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	return flHighestTerrain;
}

float CDigitank::HealthRechargeRate() const
{
	return BaseHealthRechargeRate() + GetSupportHealthRechargeBonus();
}

float CDigitank::ShieldRechargeRate() const
{
	return (BaseShieldRechargeRate() + GetSupportShieldRechargeBonus()) * (m_flDefensePower/10.f);
}

double CDigitank::FirstProjectileTime() const
{
	return RandomFloat(0, 1);
}

void CDigitank::Disable(size_t iTurns)
{
	if (m_iTurnsDisabled == (size_t)0 && iTurns > 0)
		CallOutput("OnDisable");

	if (m_iTurnsDisabled < iTurns)
		m_iTurnsDisabled = iTurns;

	DirtyNeedsOrders();

	DigitanksGame()->OnDisabled(this, NULL, NULL);
}

void CDigitank::SetFortifyPoint(CStructure* pStructure, Vector vecFortify)
{
	m_bFortifyPoint = true;
	m_vecFortifyPoint = vecFortify;

	m_hFortifyDefending = pStructure;
}

void CDigitank::RemoveFortifyPoint()
{
	m_bFortifyPoint = false;

	if (m_hFortifyDefending != NULL)
		m_hFortifyDefending->RemoveDefender(this);
}
