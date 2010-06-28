#include "supplyline.h"

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
	size_t iSegments = (size_t)(flDistance/2);

	CRenderingContext r(Game()->GetRenderer());
	r.SetColor(GetTeam()->GetColor());

	glBegin(GL_LINE_STRIP);

	glVertex3fv(DigitanksGame()->GetTerrain()->SetPointHeight(m_hSupplier->GetOrigin()) + Vector(0, 1, 0));

	for (size_t i = 0; i < iSegments; i++)
	{
		float flCurrentDistance = ((float)i*flDistance)/iSegments;
		glVertex3fv(DigitanksGame()->GetTerrain()->SetPointHeight(m_hSupplier->GetOrigin() + vecDirection*flCurrentDistance) + Vector(0, 1, 0));
	}

	glVertex3fv(DigitanksGame()->GetTerrain()->SetPointHeight(vecDestination) + Vector(0, 1, 0));

	glEnd();
}
