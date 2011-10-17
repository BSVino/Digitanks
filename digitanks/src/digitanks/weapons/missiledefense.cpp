#include "missiledefense.h"

#include <maths.h>

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

Vector CMissileDefense::GetOrigin() const
{
	if (!m_hOwner)
		return BaseClass::GetOrigin();

	float flTimeSinceFire = GameServer()->GetGameTime() - GetSpawnTime();

	Vector vecMissileAcceleration = Vector(0, 200, 0);

	// Standard constant acceleration formula.
	Vector vecMissilePosition = m_hOwner->GetOrigin() + 0.5f*vecMissileAcceleration*flTimeSinceFire*flTimeSinceFire;

	if (!m_hTarget)
		return vecMissilePosition;

	float flTimeUntilIntercept = (GetSpawnTime() + InterceptTime()) - GameServer()->GetGameTime();

	// Standard constant acceleration formula.
	Vector vecInterceptLocation = m_hTarget->GetOrigin() + m_hTarget->GetVelocity() * flTimeUntilIntercept + 0.5f * m_hTarget->GetGravity() * flTimeUntilIntercept * flTimeUntilIntercept;

	float flLerp = Lerp(RemapVal(flTimeSinceFire, 0, InterceptTime(), 0, 1), 0.2f);

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

void CMissileDefense::OnRender(class CRenderingContext* pContext, bool bTransparent)
{
	if (bTransparent)
		return;

	CRenderingContext r(DigitanksGame()->GetDigitanksRenderer());

	r.RenderSphere();
}
