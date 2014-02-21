#include "mobilecpu.h"

#include <mtrand.h>

#include <renderer/renderer.h>
#include <models/models.h>

#include <structures/cpu.h>
#include <digitanksgame.h>
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

INPUTS_TABLE_BEGIN(CMobileCPU);
INPUTS_TABLE_END();

void CMobileCPU::Precache()
{
	PrecacheModel(_T("models/digitanks/mobile-cpu.toy"), true);
	PrecacheModel(_T("models/digitanks/mobile-cpu-fan.toy"), true);
}

void CMobileCPU::Spawn()
{
	BaseClass::Spawn();

	SetModel(_T("models/digitanks/mobile-cpu.toy"));
	m_iFanModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/mobile-cpu-fan.toy"));
	m_flFanRotation = RandomFloat(0, 1);

	m_flMaxShieldStrength = m_flShieldStrength = 0;

	m_eWeapon = WEAPON_NONE;
}

void CMobileCPU::Think()
{
	BaseClass::Think();

	m_flFanRotation -= 100 * GameServer()->GetFrameTime();

	if (GameNetwork()->IsHost() && m_flFortifyTime > 0 && GameServer()->GetGameTime() - m_flFortifyTime > 1)
	{
		Delete();

		GameNetwork()->SetRunningClientFunctions(false);

		CCPU* pCPU = GameServer()->Create<CCPU>("CCPU");
		pCPU->SetOrigin(DigitanksGame()->GetTerrain()->GetPointHeight(GetRealOrigin()));
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
}

bool CMobileCPU::CanFortify()
{
	// Don't let the player un-fortify and re-fortify again to make two CPU's. THAT WOULD MEAN THE END OF THE WORLD OR SOMETHING!
	if (IsFortified() || IsFortifying())
		return false;

	return true;
}

void CMobileCPU::OnFortify()
{
	DigitanksWindow()->GetInstructor()->FinishedTutorial("strategy-command", true);
	DigitanksWindow()->GetInstructor()->FinishedTutorial("strategy-deploy", true);

	if (DigitanksWindow()->GetInstructor()->GetCurrentTutorial() && DigitanksWindow()->GetInstructor()->GetCurrentTutorial()->m_sTutorialName != "strategy-buildbuffer")
		DigitanksWindow()->GetInstructor()->DisplayTutorial("strategy-buildbuffer");
}

float CMobileCPU::FindHoverHeight(Vector vecPosition) const
{
	float flHeight = BaseClass::FindHoverHeight(vecPosition);

	return flHeight + 3;
}

Vector CMobileCPU::GetRenderOrigin() const
{
	// Override CDigitank so it doesn't snap down when we deploy
	float flLerp = 0;
	float flHoverHeight = 0;
	
	float flOscillate = Oscillate(GameServer()->GetGameTime()+m_flBobOffset, 4);
	flLerp = SLerp(flOscillate, 0.2f);
	flHoverHeight = 1 + flLerp*BobHeight();

	return GetOrigin() + Vector(0, flHoverHeight, 0);
}

void CMobileCPU::ModifyContext(class CRenderingContext* pContext) const
{
	BaseClass::ModifyContext(pContext);

	if (m_flFortifyTime > 0 && GameServer()->GetGameTime() - m_flFortifyTime < 1)
	{
		pContext->SetBlend(BLEND_ALPHA);

		float flTimeSinceFortify = GameServer()->GetGameTime() - m_flFortifyTime;
		pContext->Translate(-Vector(0, RemapValClamped(flTimeSinceFortify, 0, 1, 0, 5), 0));
		pContext->SetAlpha(GetVisibility() * RemapValClamped(flTimeSinceFortify, 0, 1, 1, 0));
		pContext->Rotate(flTimeSinceFortify * 90, Vector(0, 1, 0));
	}
}

void CMobileCPU::OnRender(class CRenderingContext* pContext, bool bTransparent) const
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
	float flTimeSinceFortify = GameServer()->GetGameTime() - m_flFortifyTime;
	if (m_flFortifyTime > 0 && GameServer()->GetGameTime() - m_flFortifyTime < 1)
		flVisibility *= RemapValClamped(flTimeSinceFortify, 0, 1, 1, 0);

	if (flVisibility < 1 && !bTransparent)
		return;

	if (flVisibility == 1 && bTransparent)
		return;

	if (bTransparent)
	{
		r.SetAlpha(flVisibility);
		if (r.GetAlpha() < 1)
			r.SetBlend(BLEND_ALPHA);
	}

	r.Rotate(m_flFanRotation, Vector(1, 0, 0));
	r.Translate(Vector(-3.9f, 0, 0));

	r.RenderModel(m_iFanModel);
}
