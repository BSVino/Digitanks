#include "powerup.h"

#include <color.h>
#include <mtrand.h>

#include <models/models.h>
#include <renderer/renderer.h>

#include <units/scout.h>
#include <units/standardtank.h>
#include <units/maintank.h>
#include <units/mechinf.h>
#include "units/digitank.h"
#include "digitanksgame.h"
#include <ui/digitankswindow.h>
#include <ui/hud.h>

REGISTER_ENTITY(CPowerup);

NETVAR_TABLE_BEGIN(CPowerup);
	NETVAR_DEFINE(powerup_type_t, m_ePowerupType);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CPowerup);
	SAVEDATA_DEFINE_OUTPUT(OnPickup);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, powerup_type_t, m_ePowerupType);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CPowerup);
INPUTS_TABLE_END();

void CPowerup::Precache()
{
	PrecacheModel(_T("models/powerup.obj"));
	PrecacheModel(_T("models/powerup-airstrike.obj"));
	PrecacheModel(_T("models/powerup-tank.obj"));
}

void CPowerup::Spawn()
{
	BaseClass::Spawn();

	m_bTakeDamage = false;

	SetCollisionGroup(CG_POWERUP);
	if (RandomInt(0, 2) == 0)
	{
		switch (RandomInt(0, 4))
		{
		case 0:
		case 1:
			SetPowerupType(POWERUP_AIRSTRIKE);
			break;

		case 4:
			SetPowerupType(POWERUP_TANK);
			break;
		}
	}
	else
		SetPowerupType(POWERUP_BONUS);
}

tstring CPowerup::GetEntityName() const
{
	switch (m_ePowerupType)
	{
	default:
	case POWERUP_BONUS:
		return _T("Promotion Powerup");

	case POWERUP_AIRSTRIKE:
		return _T("Airstrike Powerup");

	case POWERUP_TANK:
		return _T("New Unit Powerup");

	case POWERUP_MISSILEDEFENSE:
		return _T("Missile Defense Powerup");

	case POWERUP_WEAPON:
		return _T("New Weapon Powerup");
	}
}

EAngle CPowerup::GetRenderAngles() const
{
	float flRotate = fmod(GameServer()->GetGameTime(), 3.6f)*100.0f;
	return EAngle(0, flRotate, 0);
}

Vector CPowerup::GetRenderOrigin() const
{
	Vector vecOrigin = BaseClass::GetRenderOrigin();
	vecOrigin.y += RemapValClamped(GameServer()->GetGameTime() - GetSpawnTime(), 0, 3, 100, 0) + RemapVal(Lerp(Oscillate(GameServer()->GetGameTime() + GetSpawnTime(), 3), 0.8f), 0, 1, 3, 4);
	return vecOrigin;
}

void CPowerup::ModifyContext(class CRenderingContext* pContext, bool bTransparent) const
{
	BaseClass::ModifyContext(pContext, bTransparent);

	pContext->SetBlend(BLEND_ADDITIVE);
	pContext->SetDepthMask(false);

	pContext->SetAlpha(RemapValClamped(GameServer()->GetGameTime() - GetSpawnTime(), 0, 3, 0, 1));
}

void CPowerup::SetPowerupType(powerup_type_t eType)
{
	m_ePowerupType = eType;

	switch (m_ePowerupType)
	{
	default:
	case POWERUP_BONUS:
		SetModel(_T("models/powerup.obj"));
		break;

	case POWERUP_AIRSTRIKE:
		SetModel(_T("models/powerup-airstrike.obj"));
		break;

	case POWERUP_TANK:
		SetModel(_T("models/powerup-tank.obj"));
		break;

	case POWERUP_MISSILEDEFENSE:
		SetModel(_T("models/powerup-missiledefense.obj"));
		break;

	case POWERUP_WEAPON:
		SetModel(_T("models/powerup-airstrike.obj"));
		break;
	}
}

void CPowerup::Pickup(class CDigitank* pTank)
{
	CallOutput("OnPickup");

	Delete();

	switch (GetPowerupType())
	{
	case POWERUP_BONUS:
	default:
		pTank->GiveBonusPoints(1);
		break;

	case POWERUP_AIRSTRIKE:
		pTank->GiveAirstrike();
		DigitanksWindow()->GetHUD()->AddPowerupNotification(pTank, POWERUP_AIRSTRIKE);
		break;

//	case POWERUP_MISSILEDEFENSE:
//		m_iMissileDefenses += 3;
//		break;

	case POWERUP_TANK:
	{
		CDigitank* pNewTank;
		if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
			pNewTank = GameServer()->Create<CStandardTank>("CStandardTank");
		else
		{
			switch(RandomInt(0, 4))
			{
			default:
			case 0:
			case 1:
				pNewTank = GameServer()->Create<CScout>("CScout");
				break;

			case 2:
			case 3:
				pNewTank = GameServer()->Create<CMechInfantry>("CMechInfantry");
				break;

			case 4:
				pNewTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
				break;
			}
		}

		pTank->GetTeam()->AddEntity(pNewTank);

		Vector vecTank = m_vecOrigin - (GetOrigin().Normalized() * (GetBoundingRadius()*2));
		vecTank.y = pNewTank->FindHoverHeight(vecTank);
		EAngle angTank = VectorAngles(-vecTank.Normalized());

		pNewTank->SetOrigin(vecTank);
		pNewTank->SetAngles(angTank);
		pNewTank->StartTurn();

		pNewTank->CalculateVisibility();

		DigitanksWindow()->GetHUD()->AddPowerupNotification(pNewTank, POWERUP_TANK);
	}

	case POWERUP_WEAPON:
		DigitanksGame()->AllowLaser();
		break;
	}

//	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_POWERUP);
}
