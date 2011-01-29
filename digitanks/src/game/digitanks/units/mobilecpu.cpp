#include "mobilecpu.h"

#include <game/digitanks/structures/cpu.h>
#include <game/digitanks/digitanksgame.h>
#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <ui/hud.h>

REGISTER_ENTITY(CMobileCPU);

NETVAR_TABLE_BEGIN(CMobileCPU);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMobileCPU);
SAVEDATA_TABLE_END();

void CMobileCPU::Precache()
{
	PrecacheModel(L"models/digitanks/mobile-cpu.obj", true);
}

void CMobileCPU::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/mobile-cpu.obj");

	m_flFrontMaxShieldStrength = m_flFrontShieldStrength = m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flRearMaxShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flRearShieldStrength = 0;

	m_eWeapon = WEAPON_NONE;
}

void CMobileCPU::OnFortify()
{
	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_STRATEGY_DEPLOY, true);

	Delete();

	CCPU* pCPU = GameServer()->Create<CCPU>("CCPU");
	pCPU->SetOrigin(DigitanksGame()->GetTerrain()->SetPointHeight(GetRealOrigin()));
	GetTeam()->AddEntity(pCPU);
	pCPU->FindGround();

	// 8 free power to get the player started.
	GetDigitanksTeam()->AddPower(8);

	GetDigitanksTeam()->CountBandwidth();
	GetDigitanksTeam()->CountProducers();
	GetDigitanksTeam()->CountFleetPoints();
	GetDigitanksTeam()->CountScore();

	DigitanksWindow()->GetHUD()->Layout();

	GetDigitanksTeam()->SetPrimarySelection(pCPU);
}
