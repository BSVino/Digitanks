#include "cameraguided.h"

#include <digitanks/digitanksgame.h>
#include <digitanks/dt_camera.h>
#include <ui/digitankswindow.h>

NETVAR_TABLE_BEGIN(CCameraGuidedMissile);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCameraGuidedMissile);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CDigitank>, m_hOwner);
SAVEDATA_TABLE_END();

void CCameraGuidedMissile::Spawn()
{
	BaseClass::Spawn();

	DigitanksGame()->GetDigitanksCamera()->SetCameraGuidedMissile(this);
	DigitanksWindow()->SetMouseCursorEnabled(false);
}

void CCameraGuidedMissile::Think()
{
	BaseClass::Think();

	if (GameServer()->GetGameTime() - m_flTimeCreated > 3.0f)
		SetVelocity(AngleVector(GetAngles()) * VelocityPerSecond());

	if (m_flTimeExploded == 0 && GameServer()->GetGameTime() - m_flTimeCreated > 13.0f)
		Explode();

	if (GetOrigin().y < DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x, GetOrigin().z) - 20 || GetOrigin().y < -100)
	{
		Delete();
		DigitanksWindow()->SetMouseCursorEnabled(true);
	}
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
		vecPoint = GetOrigin();
		if ((pOther->GetOrigin() - GetOrigin()).LengthSqr() < pOther->GetBoundingRadius()*pOther->GetBoundingRadius())
			return true;
		break;

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

	pOther->TakeDamage(m_hOwner, this, m_flDamage);

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
