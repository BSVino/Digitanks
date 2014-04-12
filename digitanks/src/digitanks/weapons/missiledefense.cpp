#include "missiledefense.h"

#include <maths.h>
#include <renderer/game_renderingcontext.h>

#include <digitanksgame.h>
#include <dt_renderer.h>
#include <weapons/projectile.h>

REGISTER_ENTITY(CMissileDefense);

NETVAR_TABLE_BEGIN(CMissileDefense);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMissileDefense);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CProjectile>, m_hTarget);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CMissileDefense);
INPUTS_TABLE_END();

void CMissileDefense::SetTarget(CProjectile* pTarget)
{
	m_hTarget = pTarget;
}

const TVector CMissileDefense::GetGlobalOrigin() const
{
	if (!m_hOwner)
		return BaseClass::GetGlobalOrigin();

	float flTimeSinceFire = GameServer()->GetGameTime() - GetSpawnTime();

	Vector vecMissileAcceleration = Vector(0, 0, 200);

	// Standard constant acceleration formula.
	Vector vecMissilePosition = m_hOwner->GetGlobalOrigin() + 0.5f*vecMissileAcceleration*flTimeSinceFire*flTimeSinceFire;

	if (!m_hTarget)
		return vecMissilePosition;

	float flTimeUntilIntercept = (GetSpawnTime() + InterceptTime()) - GameServer()->GetGameTime();

	// Standard constant acceleration formula.
	Vector vecInterceptLocation = m_hTarget->GetGlobalOrigin() + m_hTarget->GetGlobalVelocity() * flTimeUntilIntercept + 0.5f * m_hTarget->GetGlobalGravity() * flTimeUntilIntercept * flTimeUntilIntercept;

	float flLerp = Bias(RemapVal(flTimeSinceFire, 0, InterceptTime(), 0, 1), 0.2f);

	Vector vecPosition = LerpValue<Vector>(vecMissilePosition, vecInterceptLocation, flLerp);

	return vecPosition;
}

void CMissileDefense::Think()
{
	BaseClass::Think();

	if (!m_hTarget)
	{
		Delete();
		return;
	}

	float flTimeSinceFire = GameServer()->GetGameTime() - GetSpawnTime();
	if (flTimeSinceFire > InterceptTime())
	{
		m_hTarget->Explode();
		Delete();
	}
}

void CMissileDefense::OnRender(class CGameRenderingContext* pContext)
{
	if (GameServer()->GetRenderer()->IsRenderingTransparent())
		return;

	CRenderingContext r(DigitanksGame()->GetDigitanksRenderer(), true);

	r.UseProgram("model");
	r.RenderSphere();
}
