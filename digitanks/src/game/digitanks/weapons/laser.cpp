#include "laser.h"

#include <geometry.h>

#include <digitanks/units/digitank.h>
#include <digitanks/digitanksgame.h>
#include <digitanks/dt_renderer.h>

size_t CLaser::s_iBeam = 0;

REGISTER_ENTITY(CLaser);

NETVAR_TABLE_BEGIN(CLaser);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CLaser);
SAVEDATA_TABLE_END();

void CLaser::Precache()
{
	s_iBeam = CRenderer::LoadTextureIntoGL(L"textures/beam-pulse.png");
}

void CLaser::ClientSpawn()
{
	BaseClass::ClientSpawn();

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetVisibilityAtPoint(GetOrigin()) < 0.1f)
	{
		if (DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetVisibilityAtPoint(GetOrigin() + AngleVector(GetAngles())*LaserLength()) < 0.1f)
			m_bShouldRender = false;
	}
}

void CLaser::OnSetOwner(CDigitank* pOwner)
{
	BaseClass::OnSetOwner(pOwner);

	SetAngles(VectorAngles((pOwner->GetLastAim() - GetOrigin()).Normalized()));
	SetOrigin(pOwner->GetOrigin());
	SetSimulated(false);
	SetVelocity(Vector(0,0,0));
	SetGravity(Vector(0,0,0));

	m_flTimeExploded = GameServer()->GetGameTime();

	Vector vecForward, vecRight;
	AngleVectors(GetAngles(), &vecForward, &vecRight, NULL);

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		if (!pEntity->TakesDamage())
			continue;

		if (pEntity->GetTeam() == pOwner->GetTeam())
			continue;

		float flDistance = DistanceToPlane(pEntity->GetOrigin(), GetOrigin(), vecRight);
		if (flDistance > 4 + pEntity->GetBoundingRadius())
			continue;

		// Cull objects behind
		if (vecForward.Dot(pEntity->GetOrigin() - GetOrigin()) < 0)
			continue;

		if (pEntity->Distance(GetOrigin()) > LaserLength())
			continue;

		pEntity->TakeDamage(pOwner, this, DAMAGE_LASER, m_flDamage, flDistance < pEntity->GetBoundingRadius()-2);

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		if (pTank)
		{
			float flRockIntensity = 0.5f;
			Vector vecDirection = (pTank->GetOrigin() - pOwner->GetOrigin()).Normalized();
			pTank->RockTheBoat(flRockIntensity, vecDirection);
		}
	}

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetVisibilityAtPoint(GetOrigin()) < 0.1f)
	{
		if (DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetVisibilityAtPoint(GetOrigin() + AngleVector(GetAngles())*LaserLength()) < 0.1f)
		{
			// If the start and end points are both in the fog of war, delete it now that we've aready done the damage so it doesn't get rendered later.
			if (CNetwork::IsHost())
				Delete();
		}
	}
}

void CLaser::PostRender(bool bTransparent)
{
	BaseClass::PostRender(bTransparent);

	if (!bTransparent)
		return;

	if (!m_bShouldRender)
		return;

	CRenderingContext r(DigitanksGame()->GetDigitanksRenderer());

	r.SetBlend(BLEND_ADDITIVE);

	Vector vecForward, vecRight, vecUp;

	float flLength = LaserLength();

	Vector vecMuzzle = m_hOwner->GetOrigin();
	Vector vecTarget = vecMuzzle + AngleVector(GetAngles()) * flLength;
	if (m_hOwner != NULL)
	{
		Vector vecDirection = (m_hOwner->GetLastAim() - m_hOwner->GetOrigin()).Normalized();
		vecTarget = vecMuzzle + vecDirection * flLength;
		AngleVectors(VectorAngles(vecDirection), &vecForward, &vecRight, &vecUp);
		vecMuzzle = m_hOwner->GetOrigin() + vecDirection * 3 + Vector(0, 3, 0);
	}

	float flBeamWidth = 1.5;

	Vector avecRayColors[] =
	{
		Vector(1, 0, 0),
		Vector(0, 1, 0),
		Vector(0, 0, 1),
	};

	float flRayRamp = RemapValClamped(GameServer()->GetGameTime() - GetSpawnTime(), 0.5, 1.5, 0, 1);
	float flAlphaRamp = RemapValClamped(GameServer()->GetGameTime() - GetSpawnTime(), 1, 2, 1, 0);

	size_t iBeams = 21;
	for (size_t i = 0; i < iBeams; i++)
	{
		float flUp = RemapVal((float)i, 0, (float)iBeams, -flLength, flLength);

		Vector vecRay = avecRayColors[i%3] * flRayRamp + Vector(1, 1, 1) * (1-flRayRamp);
		Color clrRay = vecRay;
		clrRay.SetAlpha((int)(200*flAlphaRamp));

		r.SetColor(clrRay);

		CRopeRenderer rope(DigitanksGame()->GetDigitanksRenderer(), s_iBeam, vecMuzzle, flBeamWidth);
		rope.SetTextureOffset(((float)i/20) - GameServer()->GetGameTime() - GetSpawnTime());
		rope.Finish(vecTarget + vecUp*flUp);
	}
}

REGISTER_ENTITY(CInfantryLaser);

NETVAR_TABLE_BEGIN(CInfantryLaser);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CInfantryLaser);
SAVEDATA_TABLE_END();
