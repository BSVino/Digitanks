#include "collector.h"

#include "digitanksteam.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

REGISTER_ENTITY(CCollector);

void CCollector::PreStartTurn()
{
	BaseClass::PreStartTurn();

	if (!IsConstructing() && GetTeam())
	{
		GetDigitanksTeam()->AddProduction(m_hResource->GetProduction());
	}
}

void CCollector::OnRender()
{
	if (GetVisibility() == 0)
		return;

	glutSolidCube(8);
}
