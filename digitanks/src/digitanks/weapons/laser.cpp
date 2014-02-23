#include "laser.h"

#include <geometry.h>

#include <textures/materiallibrary.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/roperenderer.h>

#include <units/digitank.h>
#include <digitanksgame.h>
#include <dt_renderer.h>

CMaterialHandle CLaser::s_hBeam;

REGISTER_ENTITY(CLaser);

NETVAR_TABLE_BEGIN(CLaser);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CLaser);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CLaser);
INPUTS_TABLE_END();

#define _T(x) x

void CLaser::Precache()
{
	s_hBeam = CMaterialLibrary::AddMaterial(_T("textures/beam-pulse.mat"));
}

void CLaser::ClientSpawn()
{
	BaseClass::ClientSpawn();

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetVisibilityAtPoint(GetGlobalOrigin()) < 0.1f)
	{
		if (DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetVisibilityAtPoint(GetGlobalOrigin() + AngleVector(GetGlobalAngles())*LaserLength()) < 0.1f)
			m_bShouldRender = false;
	}
}

void CLaser::OnSetOwner(CBaseEntity* pOwner)
{
	BaseClass::OnSetOwner(pOwner);

	CDigitank* pTank = dynamic_cast<CDigitank*>(pOwner);
	if (!pTank)
		return;

	SetGlobalAngles(VectorAngles((pTank->GetLastAim() - GetGlobalOrigin()).Normalized()));
	SetGlobalOrigin(pOwner->GetGlobalOrigin());
	SetGlobalVelocity(Vector(0,0,0));
	SetGlobalGravity(Vector(0,0,0));

	m_flTimeExploded = GameServer()->GetGameTime();

	Vector vecForward, vecRight;
	AngleVectors(GetGlobalAngles(), &vecForward, &vecRight, NULL);

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		if (!pEntity->TakesDamage())
			continue;

		if (pEntity->GetTeam() == pOwner->GetTeam())
			continue;

		float flDistance = DistanceToPlane(pEntity->GetGlobalOrigin(), GetGlobalOrigin(), vecRight);
		if (flDistance > 4 + pEntity->GetBoundingRadius())
			continue;

		// Cull objects behind
		if (vecForward.Dot(pEntity->GetGlobalOrigin() - GetGlobalOrigin()) < 0)
			continue;

		if (pEntity->Distance(GetGlobalOrigin()) > LaserLength())
			continue;

		pEntity->TakeDamage(pOwner, this, DAMAGE_LASER, m_flDamage, flDistance < pEntity->GetBoundingRadius()-2);

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		if (pTank)
		{
			float flRockIntensity = 0.5f;
			Vector vecDirection = (pTank->GetGlobalOrigin() - pOwner->GetGlobalOrigin()).Normalized();
			pTank->RockTheBoat(flRockIntensity, vecDirection);
		}
	}

	CDigitanksPlayer* pCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();

	if (pCurrentTeam && pCurrentTeam->GetVisibilityAtPoint(GetGlobalOrigin()) < 0.1f)
	{
		if (pCurrentTeam->GetVisibilityAtPoint(GetGlobalOrigin() + AngleVector(GetGlobalAngles())*LaserLength()) < 0.1f)
		{
			// If the start and end points are both in the fog of war, delete it now that we've aready done the damage so it doesn't get rendered later.
			if (GameNetwork()->IsHost())
				Delete();
		}
	}
}

void CLaser::PostRender() const
{
	BaseClass::PostRender();

	if (!GameServer()->GetRenderer()->IsRenderingTransparent())
		return;

	if (!m_bShouldRender)
		return;

	if (!m_hOwner)
		return;

	CRenderingContext r(DigitanksGame()->GetDigitanksRenderer());

	r.SetBlend(BLEND_ADDITIVE);

	Vector vecForward, vecRight, vecUp;

	float flLength = LaserLength();

	CDigitank* pOwner = dynamic_cast<CDigitank*>(GetOwner());
	Vector vecMuzzle = m_hOwner->GetGlobalOrigin();
	Vector vecTarget = vecMuzzle + AngleVector(GetGlobalAngles()) * flLength;
	if (pOwner)
	{
		Vector vecDirection = (pOwner->GetLastAim() - pOwner->GetGlobalOrigin()).Normalized();
		vecTarget = vecMuzzle + vecDirection * flLength;
		AngleVectors(VectorAngles(vecDirection), &vecForward, &vecRight, &vecUp);
		vecMuzzle = pOwner->GetGlobalOrigin() + vecDirection * 3 + Vector(0, 0, 3);
	}

	float flBeamWidth = 1.5;

	Vector avecRayColors[] =
	{
		Vector(1, 0, 0),
		Vector(0, 1, 0),
		Vector(0, 0, 1),
	};

	float flRayRamp = RemapValClamped((float)(GameServer()->GetGameTime() - GetSpawnTime()), 0.5f, 1.5f, 0.0f, 1);
	float flAlphaRamp = RemapValClamped((float)(GameServer()->GetGameTime() - GetSpawnTime()), 1, 2, 1.0f, 0);

	size_t iBeams = 21;
	for (size_t i = 0; i < iBeams; i++)
	{
		float flUp = RemapVal((float)i, 0, (float)iBeams, -flLength, flLength);

		Vector vecRay = LerpValue<Vector>(Vector(1, 1, 1), avecRayColors[i%3], flRayRamp);
		Color clrRay = vecRay;
		clrRay.SetAlpha((int)(200*flAlphaRamp));

		r.SetColor(clrRay);

		CRopeRenderer rope(DigitanksGame()->GetDigitanksRenderer(), s_hBeam, vecMuzzle, flBeamWidth);
		rope.SetTextureOffset(((float)i/20) - GameServer()->GetGameTime() - GetSpawnTime());
		rope.Finish(vecTarget + vecUp*flUp);
	}
}

REGISTER_ENTITY(CInfantryLaser);

NETVAR_TABLE_BEGIN(CInfantryLaser);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CInfantryLaser);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CInfantryLaser);
INPUTS_TABLE_END();

