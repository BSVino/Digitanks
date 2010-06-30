#include "supplyline.h"

#include <geometry.h>

#include <renderer/renderer.h>
#include "structure.h"
#include <team.h>
#include "digitanksgame.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

REGISTER_ENTITY(CSupplyLine);

void CSupplyLine::SetEntities(CSupplier* pSupplier, CBaseEntity* pEntity)
{
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

void CSupplyLine::StartTurn()
{
	BaseClass::StartTurn();

	if (m_hEntity == NULL)
		return;

	m_bIntercepted = false;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CDigitank* pDigitank = dynamic_cast<CDigitank*>(pEntity);
		if (!pDigitank)
			continue;

		if (pDigitank->GetTeam() == GetTeam())
			continue;

		if (DistanceToLineSegment(pDigitank->GetOrigin(), m_hSupplier->GetOrigin(), m_hEntity->GetOrigin()) < pDigitank->GetBoundingRadius()*2)
		{
			m_bIntercepted = true;
			break;
		}
	}
}

void CSupplyLine::OnRender()
{
	if (m_hSupplier == NULL || m_hEntity == NULL)
		return;

	Vector vecDestination;
	CDigitank* pTank = dynamic_cast<CDigitank*>(m_hEntity.GetPointer());
	if (pTank)
		vecDestination = pTank->GetDesiredMove();
	else
		vecDestination = m_hEntity->GetOrigin();

	Vector vecPath = vecDestination - m_hSupplier->GetOrigin();
	vecPath.y = 0;

	float flDistance = vecPath.Length2D();
	Vector vecDirection = vecPath.Normalized();
	size_t iSegments = (size_t)(flDistance/3);

	CRenderingContext r(Game()->GetRenderer());
	Color clrTeam = GetTeam()->GetColor();
	r.SetColor(clrTeam);
	r.SetBlend(BLEND_ALPHA);

	glBegin(GL_LINE_STRIP);

	glVertex3fv(DigitanksGame()->GetTerrain()->SetPointHeight(m_hSupplier->GetOrigin()) + Vector(0, 1, 0));

	for (size_t i = 0; i < iSegments; i++)
	{
		if (m_bIntercepted && i%2 == 0)
			clrTeam.SetAlpha(50);
		else
			clrTeam.SetAlpha(255);

		r.SetColor(clrTeam);

		float flCurrentDistance = ((float)i*flDistance)/iSegments;
		glVertex3fv(DigitanksGame()->GetTerrain()->SetPointHeight(m_hSupplier->GetOrigin() + vecDirection*flCurrentDistance) + Vector(0, 1, 0));
	}

	glVertex3fv(DigitanksGame()->GetTerrain()->SetPointHeight(vecDestination) + Vector(0, 1, 0));

	glEnd();
}
