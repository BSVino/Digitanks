#include "powerup.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

void CPowerup::Render()
{
	glPushMatrix();

	glTranslatef(GetOrigin().x, GetOrigin().y, GetOrigin().z);

	int iRotate = glutGet(GLUT_ELAPSED_TIME)%3600;
	glRotatef(iRotate/10.0f, 0, 1, 0);

	glutWireSphere(2, 4, 2);

	glPopMatrix();
}

