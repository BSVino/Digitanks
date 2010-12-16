#include "mobilecpu.h"

#include <game/digitanks/structures/cpu.h>
#include <game/digitanks/digitanksgame.h>

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
	Delete();

	CCPU* pCPU = GameServer()->Create<CCPU>("CCPU");
	pCPU->SetOrigin(DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin()));
	GetTeam()->AddEntity(pCPU);
	pCPU->FindGround();
}
