#include "turretmissile.h"

#include <maths.h>

#include <digitanks/digitanksgame.h>
#include <digitanks/dt_renderer.h>
#include <digitanks/weapons/projectile.h>

REGISTER_ENTITY(CTurretMissile);

NETVAR_TABLE_BEGIN(CTurretMissile);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTurretMissile);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hTarget);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hTrailParticles);	// Generated on load
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTurretMissile);
INPUTS_TABLE_END();

void CTurretMissile::Spawn()
{
	BaseClass::Spawn();

	m_hTrailParticles.SetSystem(L"shell-trail", GetOrigin());
	m_hTrailParticles.FollowEntity(this);
	m_hTrailParticles.SetActive(true);
}

void CTurretMissile::SetTarget(CBaseEntity* pTarget)
{
	m_hTarget = pTarget;
}

Vector CTurretMissile::GetOrigin() const
{
	if (m_hOwner == NULL)
		return BaseClass::GetOrigin();

	float flTimeSinceFire = GameServer()->GetGameTime() - GetSpawnTime();

	Vector vecMissileAcceleration = Vector(0, 100, 0);

	// Standard constant acceleration formula.
	Vector vecMissilePosition = m_hOwner->GetOrigin() + 0.5f*vecMissileAcceleration*flTimeSinceFire*flTimeSinceFire;

	if (m_hTarget == NULL)
		return vecMissilePosition;

	float flTimeUntilIntercept = (GetSpawnTime() + InterceptTime()) - GameServer()->GetGameTime();

	// Standard constant acceleration formula.
	Vector vecInterceptLocation = m_hTarget->GetOrigin() + m_hTarget->GetVelocity() * flTimeUntilIntercept + 0.5f * m_hTarget->GetGravity() * flTimeUntilIntercept * flTimeUntilIntercept;

	float flLerp = Lerp(RemapVal(flTimeSinceFire, 0, InterceptTime(), 0, 1), 0.2f);

	Vector vecPosition = LerpValue<Vector>(vecMissilePosition, vecInterceptLocation, flLerp);

	return vecPosition;
}

void CTurretMissile::Think()
{
	BaseClass::Think();

	if (m_hTarget == NULL)
	{
		Delete();
		return;
	}

	float flTimeSinceFire = GameServer()->GetGameTime() - GetSpawnTime();
	if (flTimeSinceFire > InterceptTime())
	{
		m_hTarget->TakeDamage(GetOwner(), this, DAMAGE_EXPLOSION, m_flDamage, true);
		DigitanksGame()->Explode(GetOwner(), this, ExplosionRadius(), m_flDamage, m_hTarget, GetOwner()?GetOwner()->GetTeam():NULL);
		CParticleSystemLibrary::AddInstance(L"bolt-explosion", GetOrigin());
		Delete();
	}
}

void CTurretMissile::OnRender(class CRenderingContext* pContext, bool bTransparent) const
{
	if (bTransparent)
		return;

	CRenderingContext r(DigitanksGame()->GetDigitanksRenderer());

	r.Scale(0.5f, 0.5f, 0.5f);
	r.RenderSphere();
}
