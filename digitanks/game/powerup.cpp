#include "powerup.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <color.h>

#include "digitank.h"
#include "digitanksgame.h"

REGISTER_ENTITY(CPowerup);

CPowerup::CPowerup()
{
	SetCollisionGroup(CG_POWERUP);
}

void CPowerup::OnRender()
{
	Color clrPowerup(255, 255, 255);
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (pEntity == NULL)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		if (!pTank)
			continue;

		if ((pTank->GetDesiredMove() - GetOrigin()).LengthSqr() < 3*3)
			clrPowerup = Color(0, 255, 0);

		if (pTank->GetPreviewMovePower() <= pTank->GetTotalMovementPower() && (pTank->GetPreviewMove() - GetOrigin()).LengthSqr() < 3*3)
			clrPowerup = Color(0, 255, 0);
	}

	glTranslatef(0, 2, 0);

	int iRotate = glutGet(GLUT_ELAPSED_TIME)%3600;
	glRotatef(iRotate/10.0f, 0, 1, 0);

	glColor4ubv(clrPowerup);
	glutWireSphere(2, 4, 2);
}

