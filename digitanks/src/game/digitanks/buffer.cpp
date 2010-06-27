#include "buffer.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <game/team.h>

REGISTER_ENTITY(CBuffer);

void CBuffer::OnRender()
{
	glColor4ubv(GetTeam()->GetColor());
	glutSolidCube(6);
}

