#include "menumarcher.h"

#include <maths.h>
#include <mtrand.h>
#include <models/models.h>
#include <renderer/game_renderer.h>
#include <glgui/glgui.h>
#include <renderer/game_renderingcontext.h>

#include "digitanksgame.h"

REGISTER_ENTITY(CMenuMarcher);

NETVAR_TABLE_BEGIN(CMenuMarcher);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMenuMarcher);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, float, m_flNextSpeech);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, size_t, m_iTurretModel);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hHoverParticles);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, float, m_flBobOffset);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CMenuMarcher);
INPUTS_TABLE_END();

void CMenuMarcher::Precache()
{
	PrecacheModel("models/digitanks/digitank-body.toy");
	PrecacheModel("models/digitanks/digitank-turret.toy");
	PrecacheParticleSystem("tank-hover");
}

void CMenuMarcher::Spawn()
{
	BaseClass::Spawn();

	SetModel("models/digitanks/digitank-body.toy");

	m_iTurretModel = CModelLibrary::Get()->FindModel("models/digitanks/digitank-turret.toy");

	m_hHoverParticles.SetSystem("tank-hover", GetGlobalOrigin());
	m_hHoverParticles.FollowEntity(this);

	m_flBobOffset = RandomFloat(0, 10);
	m_flNextSpeech = RandomFloat(0, 10);

	m_hHoverParticles.SetActive(true);
}

void CMenuMarcher::OnDeleted()
{
	BaseClass::OnDeleted();
}

void CMenuMarcher::Think()
{
	BaseClass::Think();

	Speak();

	Vector vecNewOrigin = GetGlobalOrigin() + Vector(0, 5, 0) * (float)GameServer()->GetFrameTime();

	if (vecNewOrigin.y > 80)
		vecNewOrigin.y -= 160;

	vecNewOrigin.z = vecNewOrigin.z + GetGlobalVelocity().z * (float)GameServer()->GetFrameTime();

	float flNewHoverHeight = FindHoverHeight(vecNewOrigin) + 2;

	if (vecNewOrigin.z > flNewHoverHeight)
	{
		SetGlobalVelocity(Vector(0, 0, GetGlobalVelocity().z - 5 * (float)GameServer()->GetFrameTime()));
	}
	else
	{
		vecNewOrigin.z = flNewHoverHeight;
		SetGlobalVelocity(Vector(0, 0, Approach(0, GetGlobalVelocity().z, 1 * (float)GameServer()->GetFrameTime())));
	}

	SetGlobalOrigin(vecNewOrigin);
}

Vector CMenuMarcher::GetRenderOrigin() const
{
	float flLerp = 0;
	
	float flOscillate = Oscillate((float)GameServer()->GetGameTime()+m_flBobOffset, 4);
	flLerp = Gain(flOscillate, 0.2f);

	return GetGlobalOrigin() + Vector(0, 0, 1 + flLerp*0.5f);
}

bool CMenuMarcher::ModifyShader(CRenderingContext* pContext) const
{
	bool bReturn = BaseClass::ModifyShader(pContext);

	pContext->SetUniform("bColorSwapInAlpha", true);
	pContext->SetUniform("vecColorSwap", Color(0, 0, 255));

	return bReturn;
}

void CMenuMarcher::OnRender(CGameRenderingContext* pContext) const
{
	BaseClass::OnRender(pContext);

	if (!GameServer()->GetRenderer()->IsRenderingTransparent())
		RenderTurret();
}

void CMenuMarcher::RenderTurret() const
{
	if (m_iTurretModel == ~0)
		return;

	CGameRenderingContext r(GameServer()->GetRenderer(), true);
	r.Translate(Vector(0, 0, 0.810368f));

	float flScale = 1.3f;
	r.Scale(flScale, flScale, flScale);

	r.RenderModel(m_iTurretModel, this);
}

void CMenuMarcher::Speak()
{
	if (GameServer()->GetGameTime() < m_flNextSpeech)
		return;

	m_flNextSpeech = GameServer()->GetGameTime() + RandomFloat(5, 10);

	if (rand()%6 != 0)
		return;

	const char* apszTankLines[] =
	{
		"^.^",
		"<3",
		":D!",
		"\\o/",
		">D",
		"8)",
		"!!",
	};

	DigitanksGame()->TankSpeak(this, apszTankLines[RandomInt(0, 6)]);
}

float CMenuMarcher::FindHoverHeight(Vector vecPosition) const
{
	CTerrain* pTerrain = DigitanksGame()->GetTerrain();

	float flHighestTerrain = pTerrain->GetHeight(vecPosition.x, vecPosition.y);

	float flTerrain;

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
