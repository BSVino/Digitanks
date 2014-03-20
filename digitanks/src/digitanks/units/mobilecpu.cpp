#include "mobilecpu.h"

#include <mtrand.h>

#include <renderer/game_renderer.h>
#include <models/models.h>
#include <renderer/game_renderingcontext.h>

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

#define _T(x) x

void CMobileCPU::Precache()
{
	PrecacheModel(_T("models/digitanks/mobile-cpu.toy"));
	PrecacheModel(_T("models/digitanks/mobile-cpu-fan.toy"));
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
		pCPU->SetGlobalOrigin(DigitanksGame()->GetTerrain()->GetPointHeight(GetRealOrigin()));
		pCPU->CalculateVisibility();
		GetPlayerOwner()->AddUnit(pCPU);
		pCPU->FindGround();

		// 8 free power to get the player started.
		GetDigitanksPlayer()->AddPower(8);

		GetDigitanksPlayer()->CountBandwidth();
		GetDigitanksPlayer()->CountProducers();
		GetDigitanksPlayer()->CountFleetPoints();
		GetDigitanksPlayer()->CountScore();

		DigitanksWindow()->GetHUD()->Layout();

		GetDigitanksPlayer()->SetPrimarySelection(pCPU);
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
	DigitanksWindow()->GetInstructor()->FinishedLesson("strategy-command", true);
	DigitanksWindow()->GetInstructor()->FinishedLesson("strategy-deploy", true);

	if (DigitanksWindow()->GetInstructor()->GetCurrentLesson() && DigitanksWindow()->GetInstructor()->GetCurrentLesson()->m_sLessonName != "strategy-buildbuffer")
		DigitanksWindow()->GetInstructor()->DisplayLesson("strategy-buildbuffer");
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
	
	float flOscillate = Oscillate((float)GameServer()->GetGameTime()+m_flBobOffset, 4);
	flLerp = Gain(flOscillate, 0.2f);
	flHoverHeight = 1 + flLerp*BobHeight();

	return GetGlobalOrigin() + Vector(0, 0, flHoverHeight);
}

void CMobileCPU::ModifyContext(class CRenderingContext* pContext) const
{
	BaseClass::ModifyContext(pContext);

	if (m_flFortifyTime > 0 && GameServer()->GetGameTime() - m_flFortifyTime < 1)
	{
		pContext->SetBlend(BLEND_ALPHA);

		float flTimeSinceFortify = (float)(GameServer()->GetGameTime() - m_flFortifyTime);
		pContext->Translate(-Vector(0, 0, RemapValClamped(flTimeSinceFortify, 0, 1, 0.0f, 5)));
		pContext->SetAlpha(GetVisibility() * RemapValClamped(flTimeSinceFortify, 0, 1, 1.0f, 0));
		pContext->Rotate(flTimeSinceFortify * 90, Vector(0, 0, 1));
	}
}

void CMobileCPU::OnRender(class CGameRenderingContext* pContext) const
{
	BaseClass::OnRender(pContext);

	if (m_iFanModel == ~0)
		return;

	if (GetVisibility() == 0)
		return;

	CGameRenderingContext r(GameServer()->GetRenderer(), true);

	r.SetUniform("bColorSwapInAlpha", true);

	if (GetDigitanksPlayer())
		r.SetUniform("vecColorSwap", GetDigitanksPlayer()->GetColor());
	else
		r.SetUniform("vecColorSwap", Color(255, 255, 255, 255));

	float flVisibility = GetVisibility();
	float flTimeSinceFortify = (float)(GameServer()->GetGameTime() - m_flFortifyTime);
	if (m_flFortifyTime > 0 && GameServer()->GetGameTime() - m_flFortifyTime < 1)
		flVisibility *= RemapValClamped(flTimeSinceFortify, 0, 1, 1, 0);

	if (flVisibility < 1 && !GameServer()->GetRenderer()->IsRenderingTransparent())
		return;

	if (flVisibility == 1 && GameServer()->GetRenderer()->IsRenderingTransparent())
		return;

	if (GameServer()->GetRenderer()->IsRenderingTransparent())
	{
		r.SetAlpha(flVisibility);
		if (r.GetAlpha() < 1)
			r.SetBlend(BLEND_ALPHA);
	}

	r.Rotate(m_flFanRotation, Vector(1, 0, 0));
	r.Translate(Vector(-3.9f, 0, 0));

	r.RenderModel(m_iFanModel);
}
