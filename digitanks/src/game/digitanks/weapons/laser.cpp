#include "laser.h"

#include <geometry.h>

#include <digitanks/units/digitank.h>
#include <digitanks/digitanksgame.h>
#include <digitanks/dt_renderer.h>

NETVAR_TABLE_BEGIN(CLaser);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CLaser);
SAVEDATA_TABLE_END();

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

		pEntity->TakeDamage(pOwner, this, DAMAGE_LASER, m_flDamage, flDistance < pEntity->GetBoundingRadius()-2);
	}
}

void CLaser::OnRender(class CRenderingContext* pContext, bool bTransparent)
{
	if (!bTransparent)
		return;

	CRenderingContext r(DigitanksGame()->GetDigitanksRenderer());

	r.SetBlend(BLEND_ADDITIVE);
	r.SetColor(Color(255, 255, 255, 255*3/10));

	Vector vecForward, vecRight, vecUp;
	vecForward = Vector(1, 0, 0);
	vecRight = Vector(0, 0, 1);
	vecUp = Vector(0, 1, 0);

	r.BeginRenderQuads();

	r.Vertex(-vecRight*2);
	r.Vertex(vecRight*2);
	r.Vertex(vecForward*400 + vecUp*200 + vecRight*2);
	r.Vertex(vecForward*400 + vecUp*200 - vecRight*2);

	r.Vertex(-vecRight*2);
	r.Vertex(vecForward*400 - vecUp*200 - vecRight*2);
	r.Vertex(vecForward*400 - vecUp*200 + vecRight*2);
	r.Vertex(vecRight*2);

	r.EndRender();

	r.BeginRenderTris();

	r.Vertex(-vecRight*2);
	r.Vertex(vecForward*400 + vecUp*200 - vecRight*2);
	r.Vertex(vecForward*400 - vecUp*200 - vecRight*2);

	r.Vertex(vecForward*400 + vecUp*200 + vecRight*2);
	r.Vertex(vecRight*2);
	r.Vertex(vecForward*400 - vecUp*200 + vecRight*2);

	r.EndRender();
}

NETVAR_TABLE_BEGIN(CInfantryLaser);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CInfantryLaser);
SAVEDATA_TABLE_END();

void CInfantryLaser::OnSetOwner(CDigitank* pOwner)
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

	float flRange = 60.0f;
	if (m_hOwner != NULL)
		flRange = m_hOwner->GetMaxRange();

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

		if (pEntity->Distance(GetOrigin()) > flRange)
			continue;

		pEntity->TakeDamage(pOwner, this, DAMAGE_LASER, m_flDamage, flDistance < pEntity->GetBoundingRadius()-2);
	}
}

void CInfantryLaser::OnRender(class CRenderingContext* pContext, bool bTransparent)
{
	if (!bTransparent)
		return;

	CRenderingContext r(DigitanksGame()->GetDigitanksRenderer());

	r.SetBlend(BLEND_ADDITIVE);
	r.SetColor(Color(255, 255, 255, 255*3/10));

	Vector vecForward, vecRight, vecUp;
	vecForward = Vector(1, 0, 0);
	vecRight = Vector(0, 0, 1);
	vecUp = Vector(0, 1, 0);

	r.BeginRenderQuads();

	float flRange = 60.0f;
	if (m_hOwner != NULL)
		flRange = m_hOwner->GetMaxRange();

	r.Vertex(-vecRight*2);
	r.Vertex(vecRight*2);
	r.Vertex(vecForward*flRange + vecUp*flRange/2 + vecRight*2);
	r.Vertex(vecForward*flRange + vecUp*flRange/2 - vecRight*2);

	r.Vertex(-vecRight*2);
	r.Vertex(vecForward*flRange - vecUp*flRange/2 - vecRight*2);
	r.Vertex(vecForward*flRange - vecUp*flRange/2 + vecRight*2);
	r.Vertex(vecRight*2);

	r.EndRender();

	r.BeginRenderTris();

	r.Vertex(-vecRight*2);
	r.Vertex(vecForward*flRange + vecUp*flRange/2 - vecRight*2);
	r.Vertex(vecForward*flRange - vecUp*flRange/2 - vecRight*2);

	r.Vertex(vecForward*flRange + vecUp*flRange/2 + vecRight*2);
	r.Vertex(vecRight*2);
	r.Vertex(vecForward*flRange - vecUp*flRange/2 + vecRight*2);

	r.EndRender();
}
