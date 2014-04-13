#include "wreckage.h"

#include <mtrand.h>
#include <renderer/particles.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>

#include <game/networkedeffect.h>
#include "digitanksgame.h"
#include "terrain.h"

REGISTER_ENTITY(CWreckage);

NETVAR_TABLE_BEGIN(CWreckage);
	NETVAR_DEFINE(bool, m_bFallingIntoHole);
	NETVAR_DEFINE(float, m_flScale);
	NETVAR_DEFINE(Vector, m_vecColorSwap);
	NETVAR_DEFINE(bool, m_bCrashed);
	NETVAR_DEFINE(CEntityHandle<CDigitanksPlayer>, m_hOldPlayer);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CWreckage);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bFallingIntoHole);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iTurretModel);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flScale);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecColorSwap);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Color, m_clrSwap);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, EAngle, m_angList);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hBurnParticles);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCrashed);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CDigitanksPlayer>, m_hOldPlayer);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CWreckage);
INPUTS_TABLE_END();

void CWreckage::Precache()
{
	PrecacheParticleSystem("wreckage-burn");
	PrecacheParticleSystem("wreckage-crash");
}

void CWreckage::Spawn()
{
	BaseClass::Spawn();

	m_flScale = 1;

	m_bFallingIntoHole = false;
	m_bCrashed = false;

	m_clrSwap = Color(255, 255, 255);
	m_vecColorSwap = Vector(1, 1, 1);

	m_angList = EAngle(RandomFloat(-90, 90), RandomFloat(-180, 180), RandomFloat(-90, 90));

	m_hBurnParticles.SetSystem("wreckage-burn", GetGlobalOrigin());
	m_hBurnParticles.FollowEntity(this);
	m_iTurretModel = ~0;
	m_bTakeDamage = false;
}

void CWreckage::Think()
{
	BaseClass::Think();

	bool bOverHole = DigitanksGame()->GetTerrain()->IsPointOverHole(GetGlobalOrigin());

	if (!m_bCrashed && !bOverHole && GetGlobalOrigin().z < DigitanksGame()->GetTerrain()->GetHeight(GetGlobalOrigin().x, GetGlobalOrigin().y))
	{
		m_bCrashed = true;

		SetGlobalGravity(Vector(0, 0, 0));
		SetGlobalVelocity(Vector(0, 0, 0));
		m_angList = EAngle(0, 0, 0);

		if (GetVisibility() > 0)
			CNetworkedEffect::AddInstance("wreckage-crash", GetGlobalOrigin());
	}

	SetGlobalAngles(GetGlobalAngles() + m_angList*GameServer()->GetFrameTime());

	// Slowly burn to black.
	Vector vecColorSwap = m_vecColorSwap * RemapValClamped((float)(GameServer()->GetGameTime() - GetSpawnTime()), 0.0f, 30.0f, 1.0f, 0.0f);
	m_clrSwap = vecColorSwap;

	m_hBurnParticles.SetActive(GetVisibility() > 0.1f && !m_bFallingIntoHole);

	if (!GameNetwork()->IsHost())
		return;

	// Stick around for 30 minutes before disappearing
	if (GameServer()->GetGameTime() - GetSpawnTime() > 10*60)
		Delete();

	else if (m_bFallingIntoHole && GameServer()->GetGameTime() - GetSpawnTime() > 10)
		Delete();
}

bool CWreckage::ModifyShader(CRenderingContext* pContext) const
{
	bool bReturn = BaseClass::ModifyShader(pContext);

	pContext->SetUniform("bColorSwapInAlpha", true);
	pContext->SetUniform("vecColorSwap", m_clrSwap);
	pContext->Scale(m_flScale, m_flScale, m_flScale);
	pContext->SetBlend(BLEND_ALPHA);
	pContext->SetAlpha(Flicker("mmmmmmqtmmmmtfqmmmmmm", (float)GameServer()->GetGameTime() + ((float)GetSpawnSeed()/100), 2.0f) * 0.7f);

	return bReturn;
}

void CWreckage::OnRender(class CGameRenderingContext* pContext) const
{
	BaseClass::OnRender(pContext);

	if (m_iTurretModel == ~0)
		return;

	if (GetVisibility() == 0)
		return;

	float flVisibility = GetVisibility();

	if (!GameServer()->GetRenderer()->IsRenderingTransparent())
		return;

	CGameRenderingContext r(GameServer()->GetRenderer(), true);

	r.SetAlpha(pContext->GetAlpha() * flVisibility);
	r.SetBlend(BLEND_ALPHA);

	r.Translate(Vector(-0.0f, 0.810368f, 0));

	r.Rotate(-35, Vector(0, 1, 0));

	r.SetUniform("bColorSwapInAlpha", true);
	r.SetUniform("vecColorSwap", m_clrSwap);

	r.RenderModel(m_iTurretModel);
}

void CWreckage::Touching(CBaseEntity* pOther)
{
}

void CWreckage::SetColorSwap(Color clrSwap)
{
	m_clrSwap = clrSwap;
	m_vecColorSwap = clrSwap;
}

REGISTER_ENTITY(CDebris);

NETVAR_TABLE_BEGIN(CDebris);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDebris);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hBurnParticles);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CDebris);
INPUTS_TABLE_END();

void CDebris::Precache()
{
	PrecacheParticleSystem("debris-burn");
}

void CDebris::Spawn()
{
	BaseClass::Spawn();

	m_hBurnParticles.SetSystem("debris-burn", GetGlobalOrigin());
	m_hBurnParticles.FollowEntity(this);
	m_hBurnParticles.SetActive(true);
	m_bTakeDamage = false;

	SetGlobalVelocity(Vector(RandomFloat(-40, 40), RandomFloat(5, 40), RandomFloat(-40, 40)));
	SetGlobalGravity(Vector(0, 0, -50));
}

void CDebris::Think()
{
	BaseClass::Think();

	if (!GameNetwork()->IsHost())
		return;

	if (GetGlobalOrigin().z < DigitanksGame()->GetTerrain()->GetHeight(GetGlobalOrigin().x, GetGlobalOrigin().y))
		Delete();

	if (GameServer()->GetGameTime() - GetSpawnTime() > 10)
		Delete();
}

void CDebris::Touching(CBaseEntity* pOther)
{
	if (!GameNetwork()->IsHost())
		return;

	if (dynamic_cast<CTerrain*>(pOther))
		Delete();
}
