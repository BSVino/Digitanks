#include "cameraguided.h"

#include <maths.h>

#include <digitanks/digitanksgame.h>
#include <digitanks/dt_camera.h>
#include <ui/digitankswindow.h>

REGISTER_ENTITY(CCameraGuidedMissile);

NETVAR_TABLE_BEGIN(CCameraGuidedMissile);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCameraGuidedMissile);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flBoostTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flBoostVelocityGoal);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flBoostVelocity);
SAVEDATA_TABLE_END();

void CCameraGuidedMissile::Precache()
{
	PrecacheSound(L"sound/missile-launch.wav");
	PrecacheSound(L"sound/missile-flight.wav");
}

void CCameraGuidedMissile::Spawn()
{
	BaseClass::Spawn();

	DigitanksGame()->GetDigitanksCamera()->SetCameraGuidedMissile(this);
	DigitanksWindow()->SetMouseCursorEnabled(false);

	m_flBoostVelocityGoal = 0;
	m_flBoostVelocity = 0;
	m_flBoostTime = 0;
	m_bLaunched = false;
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

	if (GameServer()->GetGameTime() - m_flTimeCreated > 3.0f || m_flBoostTime > 0.0f)
	{
		if (!m_bLaunched)
		{
			m_bLaunched = true;
			EmitSound(L"sound/missile-launch.wav");
			EmitSound(L"sound/missile-flight.wav", true);
		}

		SetVelocity(AngleVector(GetAngles()) * (VelocityPerSecond() + m_flBoostVelocity));
	}

	if (m_flTimeExploded == 0 && GameServer()->GetGameTime() - m_flTimeCreated > 13.0f)
		Explode();
}

void CCameraGuidedMissile::OnSetOwner(CDigitank* pOwner)
{
	BaseClass::OnSetOwner(pOwner);

	if (pOwner)
	{
		SetOrigin(pOwner->GetOrigin() + Vector(0, 5, 0));
		EAngle angMissile = VectorAngles((pOwner->GetLastAim() - pOwner->GetOrigin()).Normalized());
		angMissile.p = 30;
		SetAngles(angMissile);
		SetVelocity(Vector());
		SetGravity(Vector());
	}

	SetSimulated(true);
	SetCollisionGroup(CG_PROJECTILE);
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

	if (pOther->GetCollisionGroup() == CG_PROP)
		return true;

	if (pOther->GetCollisionGroup() == CG_ENTITY)
	{
		if (m_hOwner != NULL && pOther->GetTeam() == m_hOwner->GetTeam())
			return false;

		return true;
	}

	if (pOther->GetCollisionGroup() == CG_TERRAIN)
		return true;

	return false;
}

bool CCameraGuidedMissile::IsTouching(CBaseEntity* pOther, Vector& vecPoint) const
{
	switch (pOther->GetCollisionGroup())
	{
	case CG_ENTITY:
	{
		vecPoint = GetOrigin();

		CDigitank* pTank = dynamic_cast<CDigitank*>(pOther);
		float flBoundingRadius = pOther->GetBoundingRadius();
		if (pTank)
			flBoundingRadius = pTank->GetShieldBlockRadius();

		if ((pOther->GetOrigin() - GetOrigin()).LengthSqr() < flBoundingRadius*flBoundingRadius)
			return true;
		break;
	}

	case CG_TERRAIN:
		return DigitanksGame()->GetTerrain()->Collide(GetLastOrigin(), GetOrigin(), vecPoint);

	case CG_PROP:
		return pOther->Collide(GetLastOrigin(), GetOrigin(), vecPoint);
	}

	return false;
}

void CCameraGuidedMissile::Touching(CBaseEntity* pOther)
{
	if (m_flTimeExploded != 0)
		return;

	pOther->TakeDamage(m_hOwner, this, DAMAGE_EXPLOSION, m_flDamage + BoostDamage()*(m_flBoostVelocity/BoostVelocity()));

	Explode(pOther);

	bool bCanSeeOwner;
	if (m_hOwner != NULL && DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_hOwner->GetOrigin()) > 0)
		bCanSeeOwner = true;
	else
		bCanSeeOwner = false;

	if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), GetOrigin()) > 0 || bCanSeeOwner)
		EmitSound(L"sound/explosion.wav");
}

void CCameraGuidedMissile::OnExplode(CBaseEntity* pInstigator)
{
	BaseClass::OnExplode(pInstigator);

	DigitanksGame()->GetDigitanksCamera()->SetCameraGuidedMissile(NULL);
	DigitanksGame()->GetDigitanksCamera()->SnapTarget(GetOrigin());
	DigitanksWindow()->SetMouseCursorEnabled(true);
}

void CCameraGuidedMissile::OnDeleted()
{
	DigitanksWindow()->SetMouseCursorEnabled(true);
}
