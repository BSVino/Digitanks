#include "buffer.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <game/team.h>

REGISTER_ENTITY(CBuffer);

void CBuffer::OnRender()
{
	glutSolidCube(6);
}

