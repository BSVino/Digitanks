#include "turretmissile.h"

#include <maths.h>

#include <renderer/game_renderingcontext.h>

#include <digitanksgame.h>
#include <dt_renderer.h>
#include <weapons/projectile.h>

REGISTER_ENTITY(CTurretMissile);

NETVAR_TABLE_BEGIN(CTurretMissile);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTurretMissile);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hTarget);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hTrailParticles);	// Generated on load
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTurretMissile);
INPUTS_TABLE_END();

#define _T(x) x

void CTurretMissile::Spawn()
{
	BaseClass::Spawn();

	m_hTrailParticles.SetSystem(_T("shell-trail"), GetGlobalOrigin());
	m_hTrailParticles.FollowEntity(this);
	m_hTrailParticles.SetActive(true);
}

void CTurretMissile::SetTarget(CBaseEntity* pTarget)
{
	m_hTarget = pTarget;
}

const TVector CTurretMissile::GetGlobalOrigin() const
{
	if (!m_hOwner)
		return BaseClass::GetGlobalOrigin();

	float flTimeSinceFire = GameServer()->GetGameTime() - GetSpawnTime();

	Vector vecMissileAcceleration = Vector(0, 0, 100);

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

void CTurretMissile::Think()
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
		m_hTarget->TakeDamage(GetOwner(), this, DAMAGE_EXPLOSION, m_flDamage, true);
		DigitanksGame()->Explode(GetOwner(), this, ExplosionRadius(), m_flDamage, m_hTarget, GetOwner()?GetOwner()->GetPlayerOwner():NULL);
		CParticleSystemLibrary::AddInstance(_T("bolt-explosion"), GetGlobalOrigin());
		Delete();
	}
}

void CTurretMissile::OnRender(class CGameRenderingContext* pContext) const
{
	if (GameServer()->GetRenderer()->IsRenderingTransparent())
		return;

	CRenderingContext r(DigitanksGame()->GetDigitanksRenderer());

	r.Scale(0.5f, 0.5f, 0.5f);
	r.RenderSphere();
}
