#include "mobilecpu.h"

#include <mtrand.h>

#include <renderer/renderer.h>
#include <models/models.h>

#include <game/digitanks/structures/cpu.h>
#include <game/digitanks/digitanksgame.h>
#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <ui/hud.h>

REGISTER_ENTITY(CMobileCPU);

NETVAR_TABLE_BEGIN(CMobileCPU);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMobileCPU);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, size_t, m_iFanModel);		// Set in Spawn()
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flFanRotation);
SAVEDATA_TABLE_END();

void CMobileCPU::Precache()
{
	PrecacheModel(L"models/digitanks/mobile-cpu.obj", true);
	PrecacheModel(L"models/digitanks/mobile-cpu-fan.obj", true);
}

void CMobileCPU::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/mobile-cpu.obj");
	m_iFanModel = CModelLibrary::Get()->FindModel(L"models/digitanks/mobile-cpu-fan.obj");
	m_flFanRotation = RandomFloat(0, 1);

	m_flMaxShieldStrength = m_flShieldStrength = 0;

	m_eWeapon = WEAPON_NONE;
}

void CMobileCPU::OnFortify()
{
	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_STRATEGY_DEPLOY, true);

	if (!CNetwork::IsHost())
		return;

	Delete();

	CCPU* pCPU = GameServer()->Create<CCPU>("CCPU");
	pCPU->SetOrigin(DigitanksGame()->GetTerrain()->SetPointHeight(GetRealOrigin()));
	pCPU->CalculateVisibility();
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

float CMobileCPU::FindHoverHeight(Vector vecPosition) const
{
	float flHeight = BaseClass::FindHoverHeight(vecPosition);

	return flHeight + 3;
}

void CMobileCPU::OnRender(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::OnRender(pContext, bTransparent);

	if (m_iFanModel == ~0)
		return;

	if (GetVisibility() == 0)
		return;

	CRenderingContext r(GameServer()->GetRenderer());
	if (GetTeam())
		r.SetColorSwap(GetTeam()->GetColor());
	else
		r.SetColorSwap(Color(255, 255, 255, 255));

	float flVisibility = GetVisibility();

	if (flVisibility < 1 && !bTransparent)
		return;

	if (flVisibility == 1 && bTransparent)
		return;

	if (bTransparent)
	{
		r.SetAlpha(GetVisibility());
		if (r.GetAlpha() < 1)
			r.SetBlend(BLEND_ALPHA);
	}

	m_flFanRotation -= 100 * GameServer()->GetFrameTime();
	r.Rotate(m_flFanRotation, Vector(1, 0, 0));
	r.Translate(Vector(-3.9f, 0, 0));

	r.RenderModel(m_iFanModel);
}
