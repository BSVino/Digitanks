#include "wreckage.h"

#include <mtrand.h>
#include <renderer/particles.h>
#include <renderer/renderer.h>

#include "digitanksgame.h"
#include "terrain.h"

REGISTER_ENTITY(CWreckage);

NETVAR_TABLE_BEGIN(CWreckage);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CWreckage);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bFallingIntoHole);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iTurretModel);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecColorSwap);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Color, m_clrSwap);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, EAngle, m_angList);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hBurnParticles);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bCrashed);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CDigitanksTeam>, m_hOldTeam);
SAVEDATA_TABLE_END();

void CWreckage::Precache()
{
	PrecacheParticleSystem(L"wreckage-burn");
	PrecacheParticleSystem(L"wreckage-crash");
}

void CWreckage::Spawn()
{
	BaseClass::Spawn();

	m_bFallingIntoHole = false;
	m_bCrashed = false;

	m_clrSwap = Color(255, 255, 255);
	m_vecColorSwap = Vector(1, 1, 1);

	m_angList = EAngle(RandomFloat(-90, 90), RandomFloat(-180, 180), RandomFloat(-90, 90));

	m_hBurnParticles.SetSystem(L"wreckage-burn", GetOrigin());
	m_hBurnParticles.FollowEntity(this);
	m_iTurretModel = ~0;
	m_bTakeDamage = false;

	SetSimulated(true);
}

void CWreckage::Think()
{
	BaseClass::Think();

	bool bOverHole = DigitanksGame()->GetTerrain()->IsPointOverHole(GetOrigin());

	if (!m_bCrashed && !bOverHole && GetOrigin().y < DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x, GetOrigin().z))
	{
		m_bCrashed = true;

		SetGravity(Vector(0, 0, 0));
		SetVelocity(Vector(0, 0, 0));
		m_angList = EAngle(0, 0, 0);

		if (GetVisibility() > 0)
			CParticleSystemLibrary::AddInstance(L"wreckage-crash", GetOrigin());
	}

	SetAngles(GetAngles() + m_angList*GameServer()->GetFrameTime());

	// Slowly burn to black.
	m_vecColorSwap = m_vecColorSwap - Vector(0.01f, 0.01f, 0.01f) * GameServer()->GetFrameTime();
	if (m_vecColorSwap.x < 0)
		m_vecColorSwap.x = 0;
	if (m_vecColorSwap.y < 0)
		m_vecColorSwap.y = 0;
	if (m_vecColorSwap.z < 0)
		m_vecColorSwap.z = 0;

	m_clrSwap = m_vecColorSwap;

	m_hBurnParticles.SetActive(GetVisibility() > 0.1f && !m_bFallingIntoHole);

	// Stick around for 30 minutes before disappearing
	if (GameServer()->GetGameTime() - GetSpawnTime() > 30*60)
		Delete();

	else if (m_bFallingIntoHole && GameServer()->GetGameTime() - GetSpawnTime() > 10)
		Delete();
}

void CWreckage::ModifyContext(CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);

	pContext->SetColorSwap(m_clrSwap);
}

void CWreckage::OnRender(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::OnRender(pContext, bTransparent);

	if (m_iTurretModel == ~0)
		return;

	if (GetVisibility() == 0)
		return;

	float flVisibility = GetVisibility();

	if (bTransparent && flVisibility == 1)
		return;

	if (!bTransparent && flVisibility < 1)
		return;

	CRenderingContext r(GameServer()->GetRenderer());

	if (bTransparent && flVisibility < 1)
	{
		r.SetAlpha(flVisibility);
		r.SetBlend(BLEND_ALPHA);
	}

	r.Translate(Vector(-0.0f, 0.810368f, 0));

	r.Rotate(-35, Vector(0, 0, 1));

	if (!GameServer()->GetRenderer()->ShouldUseShaders())
		r.SetColorSwap(m_clrSwap);

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
