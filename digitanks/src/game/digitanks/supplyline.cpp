#include "supplyline.h"

#include <geometry.h>

#include "dt_renderer.h"
#include "structures/structure.h"
#include <team.h>
#include "digitanksgame.h"

#include <GL/glew.h>

size_t CSupplyLine::s_iSupplyBeam = 0;

NETVAR_TABLE_BEGIN(CSupplyLine);
	NETVAR_DEFINE(CEntityHandle<CSupplier>, m_hSupplier);
	NETVAR_DEFINE(CEntityHandle<CBaseEntity>, m_hEntity);
	NETVAR_DEFINE(float, m_flIntegrity);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CSupplyLine);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CSupplier>, m_hSupplier);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CBaseEntity>, m_hEntity);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flIntegrity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bDelayRecharge);
SAVEDATA_TABLE_END();

void CSupplyLine::Precache()
{
	BaseClass::Precache();

	s_iSupplyBeam = CRenderer::LoadTextureIntoGL(L"textures/tendril.png");
}

void CSupplyLine::Spawn()
{
	BaseClass::Spawn();

	m_flIntegrity = 1.0f;
	m_bDelayRecharge= false;
}

void CSupplyLine::SetEntities(CSupplier* pSupplier, CBaseEntity* pEntity)
{
	if (pSupplier == m_hSupplier && pEntity == m_hEntity)
		return;

	pSupplier->GetTeam()->AddEntity(this);

	m_hSupplier = pSupplier;
	m_hEntity = pEntity;
}

Vector CSupplyLine::GetOrigin() const
{
	if (m_hSupplier == NULL)
		return BaseClass::GetOrigin();

	return m_hSupplier->GetOrigin();
}

float CSupplyLine::Distance(Vector vecSpot)
{
	if (m_hSupplier == NULL || m_hEntity == NULL)
		return BaseClass::Distance(vecSpot);

	Vector vecSupplier = m_hSupplier->GetOrigin();
	Vector vecEntity = m_hEntity->GetOrigin();

	Vector vecSupplierFlat = vecSupplier;
	Vector vecEntityFlat = vecEntity;
	Vector vecSpotFlat = vecSpot;

	vecSupplierFlat.y = 0;
	vecEntityFlat.y = 0;
	vecSpotFlat.y = 0;

	Vector vecIntersection;
	float flDistance = DistanceToLineSegment(vecSpotFlat, vecSupplierFlat, vecEntityFlat, &vecIntersection);

	DigitanksGame()->GetTerrain()->SetPointHeight(vecIntersection);

	return vecSpot.Distance(vecIntersection);
}

void CSupplyLine::StartTurn()
{
	BaseClass::StartTurn();

	if (m_hEntity == NULL)
		return;

	// Supplier got blowed up?
	if (m_hSupplier == NULL)
		return;

	if (!CNetwork::IsHost())
		return;

	if (!m_bDelayRecharge)
	{
		m_flIntegrity += 0.2f;
		if (m_flIntegrity > 1)
			m_flIntegrity = 1;
	}

	m_bDelayRecharge = false;
}

void CSupplyLine::Intercept(float flIntercept)
{
	if (!CNetwork::IsHost())
		return;

	m_flIntegrity -= flIntercept;
	if (m_flIntegrity < 0)
		m_flIntegrity = 0;

	m_bDelayRecharge = true;

	if (GetEntity() && m_flIntegrity < MinimumIntegrity() && dynamic_cast<CStructure*>(GetEntity()) && GetEntity()->GetTeam())
	{
		GetEntity()->GetTeam()->RemoveEntity(GetEntity());
	}
}

void CSupplyLine::PostRender(bool bTransparent)
{
	BaseClass::PostRender(bTransparent);

	if (!bTransparent)
		return;

	if (m_hSupplier == NULL || m_hEntity == NULL)
		return;

	Vector vecDestination = m_hEntity->GetOrigin();

	Vector vecPath = vecDestination - m_hSupplier->GetOrigin();
	vecPath.y = 0;

	float flDistance = vecPath.Length2D();
	Vector vecDirection = vecPath.Normalized();
	size_t iSegments = (size_t)(flDistance/3);

	CRenderingContext r(GameServer()->GetRenderer());
	if (DigitanksGame()->ShouldRenderFogOfWar())
		r.UseFrameBuffer(DigitanksGame()->GetDigitanksRenderer()->GetVisibilityMaskedBuffer());

	Color clrTeam = GetTeam()->GetColor();

	CRopeRenderer oRope(GameServer()->GetRenderer(), s_iSupplyBeam, DigitanksGame()->GetTerrain()->SetPointHeight(m_hSupplier->GetOrigin()) + Vector(0, 2, 0));
	oRope.SetWidth(2.5);
	oRope.SetTextureScale(5);
	oRope.SetTextureOffset(-fmod(GameServer()->GetGameTime(), 1)*2);

	for (size_t i = 1; i < iSegments; i++)
	{
		if (m_flIntegrity < 1 && i%2 == 0)
			clrTeam.SetAlpha((int)(50 * m_flIntegrity));
		else
			clrTeam.SetAlpha((int)(255 * m_flIntegrity));

		oRope.SetColor(clrTeam);

		float flCurrentDistance = ((float)i*flDistance)/iSegments;
		oRope.AddLink(DigitanksGame()->GetTerrain()->SetPointHeight(m_hSupplier->GetOrigin() + vecDirection*flCurrentDistance) + Vector(0, 2, 0));
	}

	oRope.Finish(DigitanksGame()->GetTerrain()->SetPointHeight(vecDestination) + Vector(0, 2, 0));

	glEnd();
}

CSupplier* CSupplyLine::GetSupplier()
{
	if (m_hSupplier == NULL)
		return NULL;

	return m_hSupplier;
}

CBaseEntity* CSupplyLine::GetEntity()
{
	if (m_hEntity == NULL)
		return NULL;

	return m_hEntity;
}
