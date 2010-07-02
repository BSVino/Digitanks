#include "structure.h"

#include <maths.h>

#include <renderer/renderer.h>
#include "digitanksgame.h"
#include "supplyline.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

REGISTER_ENTITY(CStructure);

CStructure::CStructure()
{
	m_bConstructing = false;
	m_iTurnsToConstruct = 0;
}

void CStructure::BeginConstruction(size_t iTurns)
{
	m_iTurnsToConstruct = iTurns;
	m_bConstructing = true;
}

void CStructure::PreStartTurn()
{
	BaseClass::PreStartTurn();

	if (IsConstructing())
	{
		if (--m_iTurnsToConstruct == 0)
			m_bConstructing = false;
	}
}

void CStructure::SetSupplier(class CSupplier* pSupplier)
{
	m_hSupplier = pSupplier;

	if (m_hSupplyLine == NULL && m_hSupplier != NULL)
		m_hSupplyLine = Game()->Create<CSupplyLine>("CSupplyLine");

	if (m_hSupplyLine != NULL && m_hSupplier == NULL)
		Game()->Delete(m_hSupplyLine);

	if (m_hSupplyLine != NULL && m_hSupplier != NULL)
		m_hSupplyLine->SetEntities(m_hSupplier, this);
}

void CStructure::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	if (IsConstructing())
	{
		pContext->SetBlend(BLEND_NONE);
		pContext->SetColor(Color(128, 128, 128));
	}
	else
		pContext->SetColor(GetTeam()->GetColor());
}

REGISTER_ENTITY(CSupplier);

void CSupplier::Spawn()
{
	BaseClass::Spawn();

	m_iDataStrength = InitialDataStrength();
	m_flBonusDataFlow = 0;
}

float CSupplier::GetDataFlowRate()
{
	return BaseDataFlowPerTurn() + m_flBonusDataFlow;
}

float CSupplier::GetDataFlowRadius() const
{
	// Opposite of formula for area of a circle.
	return sqrt(m_iDataStrength/M_PI) + GetBoundingRadius();
}

float CSupplier::GetDataFlow(Vector vecPoint)
{
	return RemapValClamped((vecPoint - GetOrigin()).Length2D(), GetBoundingRadius(), GetDataFlowRadius()+GetBoundingRadius(), (float)m_iDataStrength, 0);
}

float CSupplier::GetDataFlow(Vector vecPoint, CTeam* pTeam, CSupplier* pIgnore)
{
	float flDataStrength = 0;
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (pSupplier->GetTeam() != pTeam)
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (pSupplier == pIgnore)
			continue;

		flDataStrength += pSupplier->GetDataFlow(vecPoint);
	}

	return flDataStrength;
}

void CSupplier::CalculateDataFlow()
{
	if (IsConstructing())
		return;

	if (m_hSupplier != NULL)
	{
		// Use the radius of a circle with the area of the given data flow
		// so the flow doesn't get huge when you're close to a source.
		m_flBonusDataFlow = sqrt(m_hSupplier->GetDataFlow(GetOrigin())/M_PI);
	}
	else
		m_flBonusDataFlow = 0;

	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		CSupplier* pSupplier = dynamic_cast<CSupplier*>(m_ahChildren[i].GetPointer());
		if (pSupplier)
			pSupplier->CalculateDataFlow();
	}
}

void CSupplier::PostStartTurn()
{
	BaseClass::PostStartTurn();

	if (!IsConstructing())
		m_iDataStrength += (size_t)GetDataFlowRate();

	UpdateTendrils();
}

void CSupplier::PostRender()
{
	if (GetVisibility() == 0)
		return;

	for (size_t i = 0; i < m_aTendrils.size(); i++)
	{
		CTendril* pTendril = &m_aTendrils[i];

		Vector vecDestination = pTendril->m_vecEndPoint;

		Vector vecPath = vecDestination - GetOrigin();
		vecPath.y = 0;

		float flDistance = vecPath.Length2D();
		Vector vecDirection = vecPath.Normalized();
		size_t iSegments = (size_t)(flDistance/2);

		CRenderingContext r(Game()->GetRenderer());
		Color clrTeam = GetTeam()->GetColor();
		clrTeam.SetAlpha(100);
		r.SetAlpha(GetVisibility());
		r.SetColor(clrTeam);
		r.SetBlend(BLEND_ADDITIVE);

		glBegin(GL_LINE_STRIP);

		for (size_t i = 0; i < iSegments; i++)
		{
			float flCurrentDistance = ((float)i*flDistance)/iSegments;
			glVertex3fv(DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + vecDirection*flCurrentDistance) + Vector(0, 1, 0));
		}

		glVertex3fv(DigitanksGame()->GetTerrain()->SetPointHeight(vecDestination) + Vector(0, 1, 0));

		glEnd();
	}
}

void CSupplier::UpdateTendrils()
{
	if (IsConstructing())
		return;

	size_t iRadius = (size_t)GetDataFlowRadius();
	while (m_aTendrils.size() < iRadius)
	{
		m_aTendrils.push_back(CTendril());
		CTendril* pTendril = &m_aTendrils[m_aTendrils.size()-1];
		pTendril->m_flLength = (float)m_aTendrils.size() + GetBoundingRadius();
		pTendril->m_vecEndPoint = DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + AngleVector(EAngle(0, (float)(rand()%3600)/10, 0)) * pTendril->m_flLength);
	}
}

void CSupplier::AddChild(CStructure* pChild)
{
	m_ahChildren.push_back(pChild);
}

CSupplier* CSupplier::FindClosestSupplier(CBaseEntity* pUnit)
{
	CSupplier* pClosest = NULL;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		if (pEntity == pUnit)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (pSupplier->GetTeam() != pUnit->GetTeam())
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (!pClosest)
		{
			pClosest = pSupplier;
			continue;
		}

		if ((pSupplier->GetOrigin() - pUnit->GetOrigin()).Length() < (pClosest->GetOrigin() - pUnit->GetOrigin()).Length())
			pClosest = pSupplier;
	}

	return pClosest;
}

CSupplier* CSupplier::FindClosestSupplier(Vector vecPoint, CTeam* pTeam)
{
	CSupplier* pClosest = NULL;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (pSupplier->GetTeam() != pTeam)
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (!pClosest)
		{
			pClosest = pSupplier;
			continue;
		}

		if ((pSupplier->GetOrigin() - vecPoint).Length() < (pClosest->GetOrigin() - vecPoint).Length())
			pClosest = pSupplier;
	}

	return pClosest;
}

float CSupplier::VisibleRange() const
{
	if (GetDataFlowRadius() < 30)
		return 30;
	else
		return GetDataFlowRadius();
}
