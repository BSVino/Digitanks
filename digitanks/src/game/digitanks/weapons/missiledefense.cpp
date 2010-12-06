#include "missiledefense.h"

#include <maths.h>

#include <digitanks/digitanksgame.h>
#include <digitanks/dt_renderer.h>
#include <digitanks/weapons/projectile.h>

NETVAR_TABLE_BEGIN(CMissileDefense);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMissileDefense);
SAVEDATA_TABLE_END();

void CMissileDefense::SetTarget(CProjectile* pTarget)
{
	m_hTarget = pTarget;
}

Vector CMissileDefense::GetOrigin() const
{
	if (m_hOwner == NULL)
		return BaseClass::GetOrigin();

	float flTimeSinceFire = GameServer()->GetGameTime() - m_flTimeCreated;

	Vector vecMissileAcceleration = Vector(0, 200, 0);

	// Standard constant acceleration formula.
	Vector vecMissilePosition = m_hOwner->GetOrigin() + 0.5f*vecMissileAcceleration*flTimeSinceFire*flTimeSinceFire;

	if (m_hTarget == NULL)
		return vecMissilePosition;

	float flTimeUntilIntercept = (m_flTimeCreated + InterceptTime()) - GameServer()->GetGameTime();

	// Standard constant acceleration formula.
	Vector vecInterceptLocation = m_hTarget->GetOrigin() + m_hTarget->GetVelocity() * flTimeUntilIntercept + 0.5f * m_hTarget->GetGravity() * flTimeUntilIntercept * flTimeUntilIntercept;

	float flLerp = Lerp(RemapVal(flTimeSinceFire, 0, InterceptTime(), 0, 1), 0.2f);

	Vector vecPosition = vecInterceptLocation * flLerp + vecMissilePosition * (1-flLerp);

	return vecPosition;
}

void CMissileDefense::Think()
{
	BaseClass::Think();

	if (m_hTarget == NULL)
	{
		Delete();
		return;
	}

	float flTimeSinceFire = GameServer()->GetGameTime() - m_flTimeCreated;
	if (flTimeSinceFire > InterceptTime())
	{
		m_hTarget->Explode();
		Delete();
	}
}

void CMissileDefense::OnRender()
{
	CRenderingContext r(DigitanksGame()->GetDigitanksRenderer());

	r.RenderSphere();
}
