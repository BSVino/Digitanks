#include "cpu.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <game/team.h>

REGISTER_ENTITY(CCPU);

void CCPU::OnRender()
{
	glColor4ubv(GetTeam()->GetColor());
	glutSolidCube(12);
}
