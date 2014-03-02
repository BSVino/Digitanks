#include "supplyline.h"

#include <geometry.h>

#include <textures/texturelibrary.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/roperenderer.h>

#include "dt_renderer.h"
#include "structures/structure.h"
#include "digitanksgame.h"

CMaterialHandle CSupplyLine::s_hSupplyBeam;

REGISTER_ENTITY(CSupplyLine);

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

INPUTS_TABLE_BEGIN(CSupplyLine);
INPUTS_TABLE_END();

void CSupplyLine::Precache()
{
	BaseClass::Precache();

	s_hSupplyBeam = CMaterialLibrary::AddMaterial("textures/tendril.mat");
}

void CSupplyLine::Spawn()
{
	m_hSupplier = NULL;
	m_hEntity = NULL;

	BaseClass::Spawn();

	m_flIntegrity = 1.0f;
	m_bDelayRecharge = false;
}

void CSupplyLine::SetEntities(CSupplier* pSupplier, CBaseEntity* pEntity)
{
	if (pSupplier == m_hSupplier && pEntity == m_hEntity)
		return;

	pSupplier->GetPlayerOwner()->AddUnit(this);

	m_hSupplier = pSupplier;
	m_hEntity = pEntity;
}

const TVector CSupplyLine::GetGlobalOrigin() const
{
	if (!m_hSupplier)
		return BaseClass::GetGlobalOrigin();

	return m_hSupplier->GetGlobalOrigin();
}

const TFloat CSupplyLine::GetRenderRadius() const
{
	if (!m_hSupplier || !m_hEntity)
		return 0;

	Vector vecSupplier = m_hSupplier->GetRenderOrigin();
	Vector vecEntity = m_hEntity->GetRenderOrigin();

	// Much bigger than it really is but supply lines haven't been a perf problem yet.
	return vecEntity.Distance(vecSupplier);
}

float CSupplyLine::Distance(Vector vecSpot) const
{
	if (!m_hSupplier || !m_hEntity)
		return BaseClass::Distance(vecSpot);

	Vector vecSupplier = m_hSupplier->GetGlobalOrigin();
	Vector vecEntity = m_hEntity->GetGlobalOrigin();

	Vector vecSupplierFlat = vecSupplier;
	Vector vecEntityFlat = vecEntity;
	Vector vecSpotFlat = vecSpot;

	vecSupplierFlat.z = 0;
	vecEntityFlat.z = 0;
	vecSpotFlat.z = 0;

	Vector vecIntersection;
	float flDistance = DistanceToLineSegment(vecSpotFlat, vecSupplierFlat, vecEntityFlat, &vecIntersection);

	vecIntersection = DigitanksGame()->GetTerrain()->GetPointHeight(vecIntersection);

	return vecSpot.Distance(vecIntersection);
}

void CSupplyLine::StartTurn()
{
	BaseClass::StartTurn();

	if (!m_hEntity)
		return;

	// Supplier got blowed up?
	if (!m_hSupplier)
		return;

	if (!GameNetwork()->IsHost())
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
	if (!GameNetwork()->IsHost())
		return;

	m_flIntegrity -= flIntercept;
	if (m_flIntegrity < 0)
		m_flIntegrity = 0;

	m_bDelayRecharge = true;

	if (GetEntity() && m_flIntegrity < MinimumIntegrity() && dynamic_cast<CStructure*>(GetEntity()) && GetEntity()->GetOwner())
	{
		ToDigitanksPlayer(GetEntity()->GetOwner())->RemoveUnit(static_cast<CStructure*>(GetEntity()));
		DigitanksGame()->OnDisabled(GetEntity(), NULL, NULL);
	}
}

void CSupplyLine::PostRender() const
{
	BaseClass::PostRender();

	if (!GameServer()->GetRenderer()->IsRenderingTransparent())
		return;

	if (!m_hSupplier || !m_hEntity)
		return;

	Vector vecDestination = m_hEntity->GetGlobalOrigin();

	Vector vecPath = vecDestination - m_hSupplier->GetGlobalOrigin();
	vecPath.z = 0;

	float flDistance = vecPath.Length2D();
	Vector vecDirection = vecPath.Normalized();
	size_t iSegments = (size_t)(flDistance/3);

	CRenderingContext r(GameServer()->GetRenderer(), true);
	if (DigitanksGame()->ShouldRenderFogOfWar())
		r.UseFrameBuffer(DigitanksGame()->GetDigitanksRenderer()->GetVisibilityMaskedBuffer());

	Color clrTeam(255, 255, 255, 255);
	if (GetPlayerOwner())
		clrTeam = GetPlayerOwner()->GetColor();
	clrTeam = (Vector(clrTeam) + Vector(1,1,1))/2;

	CRopeRenderer oRope(GameServer()->GetRenderer(), s_hSupplyBeam, DigitanksGame()->GetTerrain()->GetPointHeight(m_hSupplier->GetGlobalOrigin()) + Vector(0, 0, 2), 2.5f);
	if (dynamic_cast<CStructure*>(m_hEntity.GetPointer()))
	{
		oRope.SetTextureScale(500000);
		oRope.SetTextureOffset(-(float)fmod(GameServer()->GetGameTime(), 1));
	}
	else
	{
		oRope.SetTextureScale(5);
		oRope.SetTextureOffset(-(float)fmod(GameServer()->GetGameTime(), 1)*2);
	}

	float flVisibility = 1;
	CDigitanksEntity* pDTEnt = dynamic_cast<CDigitanksEntity*>(m_hEntity.GetPointer());
	if (pDTEnt)
		flVisibility = pDTEnt->GetVisibility();

	for (size_t i = 1; i < iSegments; i++)
	{
		if (m_flIntegrity < 1 && i%2 == 0)
			clrTeam.SetAlpha((int)(50 * m_flIntegrity * flVisibility));
		else
			clrTeam.SetAlpha((int)(255 * m_flIntegrity * flVisibility));

		oRope.SetColor(clrTeam);

		float flCurrentDistance = ((float)i*flDistance)/iSegments;
		oRope.AddLink(DigitanksGame()->GetTerrain()->GetPointHeight(m_hSupplier->GetGlobalOrigin() + vecDirection*flCurrentDistance) + Vector(0, 0, 2));
	}

	oRope.Finish(DigitanksGame()->GetTerrain()->GetPointHeight(vecDestination) + Vector(0, 0, 2));
}

CSupplier* CSupplyLine::GetSupplier()
{
	if (!m_hSupplier)
		return NULL;

	return m_hSupplier;
}

CBaseEntity* CSupplyLine::GetEntity()
{
	if (!m_hEntity)
		return NULL;

	return m_hEntity;
}
