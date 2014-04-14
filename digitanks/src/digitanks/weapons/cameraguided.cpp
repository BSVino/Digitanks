#include "cameraguided.h"

#include <maths.h>

#include <digitanksgame.h>
#include <dt_camera.h>
#include <ui/digitankswindow.h>

REGISTER_ENTITY(CCameraGuidedMissile);

NETVAR_TABLE_BEGIN(CCameraGuidedMissile);
	NETVAR_DEFINE(double, m_flBoostTime);
	NETVAR_DEFINE(float, m_flBoostVelocityGoal);
	NETVAR_DEFINE(float, m_flBoostVelocity);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCameraGuidedMissile);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bLaunched);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, double, m_flBoostTime);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBoostVelocityGoal);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBoostVelocity);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hTrailParticles);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, EAngle, m_angView);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CCameraGuidedMissile);
INPUTS_TABLE_END();

#define _T(x) x

void CCameraGuidedMissile::Precache()
{
	PrecacheSound(_T("sound/missile-launch.wav"));
	PrecacheSound(_T("sound/missile-flight.wav"));
}

void CCameraGuidedMissile::Spawn()
{
	BaseClass::Spawn();

	m_flBoostVelocityGoal = 0;
	m_flBoostVelocity = 0;
	m_flBoostTime = 0;
	m_bLaunched = false;

	m_hTrailParticles.SetSystem("shell-trail", GetGlobalOrigin());
	m_hTrailParticles.FollowEntity(this);
}

void CCameraGuidedMissile::Think()
{
	BaseClass::Think();

	if (m_flBoostTime > 0 && GameServer()->GetGameTime() - m_flBoostTime > 1.0f)
	{
		m_flBoostVelocityGoal = 0;
		m_flBoostTime = 0;
	}

	float flFactor = 1;
	if (m_flBoostTime > 0)
		flFactor = 10;

	m_flBoostVelocity = Approach(m_flBoostVelocityGoal, m_flBoostVelocity, BoostVelocity() * flFactor * GameServer()->GetFrameTime());

	if (GameServer()->GetGameTime() - GetSpawnTime() > 3.0f || m_flBoostTime > 0.0f)
	{
		if (!m_bLaunched)
		{
			m_bLaunched = true;
			EmitSound(_T("sound/missile-launch.wav"));
			EmitSound(_T("sound/missile-flight.wav"), 1.0f, true);
		}

		SetGlobalVelocity(AngleVector(GetViewAngles()) * (VelocityPerSecond() + m_flBoostVelocity));
	}

	if (m_flTimeExploded == 0.0 && GameServer()->GetGameTime() - GetSpawnTime() > 13.0)
		Explode();

	m_hTrailParticles.SetActive(m_flTimeExploded == 0.0 && GetOwner() && GetOwner()->GetDigitanksPlayer() != DigitanksGame()->GetCurrentLocalDigitanksPlayer());
}

void CCameraGuidedMissile::OnSetOwner(CBaseEntity* pOwner)
{
	BaseClass::OnSetOwner(pOwner);

	CDigitank* pTank = dynamic_cast<CDigitank*>(pOwner);

	if (pTank)
	{
		SetGlobalOrigin(pOwner->GetGlobalOrigin() + Vector(0, 0, 5));
		EAngle angMissile = VectorAngles((pTank->GetLastAim() - pOwner->GetGlobalOrigin()).Normalized());
		angMissile.p = 30;
		SetViewAngles(angMissile);
		SetGlobalAngles(angMissile);
		SetGlobalVelocity(Vector());
		SetGlobalGravity(Vector());
	}

	if (pTank && pTank->GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
	{
		DigitanksGame()->GetOverheadCamera()->SetCameraGuidedMissile(this);
		DigitanksWindow()->SetMouseCursorEnabled(false);
	}
}

void CCameraGuidedMissile::SpecialCommand()
{
	if (m_flBoostTime > 0)
		return;

	m_flBoostVelocityGoal = BoostVelocity();
	m_flBoostVelocity = BoostVelocity()/2;
	m_flBoostTime = GameServer()->GetGameTime();
}

bool CCameraGuidedMissile::ShouldTouch(CBaseEntity* pOther) const
{
	if (!pOther)
		return false;

	if (pOther == m_hOwner)
		return false;

	TStubbed("CCameraGuidedMissile::ShouldTouch");
#if 0
	if (pOther->GetCollisionGroup() == CG_PROP)
		return true;

	if (pOther->GetCollisionGroup() == CG_ENTITY)
	{
		if (m_hOwner != NULL && pOther->GetPlayerOwner() == m_hOwner->GetPlayerOwner())
			return false;

		return true;
	}

	if (pOther->GetCollisionGroup() == CG_TERRAIN)
		return true;
#endif

	return false;
}

bool CCameraGuidedMissile::IsTouching(CBaseEntity* pOther, Vector& vecPoint) const
{
	switch (pOther->GetCollisionGroup())
	{
	case CG_ENTITY:
	{
		vecPoint = GetGlobalOrigin();

		CDigitank* pTank = dynamic_cast<CDigitank*>(pOther);
		float flBoundingRadius = pOther->GetBoundingRadius();
		if (pTank)
			flBoundingRadius = pTank->GetShieldBlockRadius();

		if ((pOther->GetGlobalOrigin() - GetGlobalOrigin()).LengthSqr() < flBoundingRadius*flBoundingRadius)
			return true;
		break;
	}

	case CG_TERRAIN:
		TStubbed("CCameraGuidedMissile::IsTouching");
		//return DigitanksGame()->GetTerrain()->Collide(GetGlobalOrigin(), GetGlobalOrigin(), vecPoint);

	case CG_PROP:;
		//return pOther->Collide(GetGlobalOrigin(), GetGlobalOrigin(), vecPoint);
	}

	return false;
}

void CCameraGuidedMissile::Touching(CBaseEntity* pOther)
{
	if (m_flTimeExploded != 0.0)
		return;

	pOther->TakeDamage(m_hOwner, this, DAMAGE_EXPLOSION, m_flDamage + BoostDamage()*(m_flBoostVelocity/BoostVelocity()));

	Explode(pOther);

	bool bCanSeeOwner;
	if (m_hOwner != NULL && DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), m_hOwner->GetGlobalOrigin()) > 0)
		bCanSeeOwner = true;
	else
		bCanSeeOwner = false;

	if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), GetGlobalOrigin()) > 0 || bCanSeeOwner)
		EmitSound(_T("sound/explosion.wav"));
}

void CCameraGuidedMissile::OnExplode(CBaseEntity* pInstigator)
{
	BaseClass::OnExplode(pInstigator);

	if (GetOwner() && GetOwner()->GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
	{
		DigitanksGame()->GetOverheadCamera()->SetCameraGuidedMissile(NULL);
		DigitanksGame()->GetOverheadCamera()->SnapTarget(GetGlobalOrigin());
		DigitanksWindow()->SetMouseCursorEnabled(true);
	}

	StopSound(_T("sound/missile-flight.wav"));
}

void CCameraGuidedMissile::OnDeleted()
{
	DigitanksWindow()->SetMouseCursorEnabled(true);
	StopSound(_T("sound/missile-flight.wav"));
}
