#include "debugdraw.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

void DebugCircle(Vector vecOrigin, float flRadius, Color c)
{
	int iLines = (int)flRadius;
	if (iLines < 20)
		iLines = 20;

	glColor4ubv(c);

	glLineWidth(1);
    
	glBegin(GL_LINE_LOOP);

	for (int i = 0; i <= iLines; i++)
	{
		float x = flRadius * cos(i * 360 / iLines * M_PI/180.0f) + vecOrigin.x;
		float z = flRadius * sin(i * 360 / iLines * M_PI/180.0f) + vecOrigin.z;

		glVertex3f(x, 0, z);
	}

	glEnd(); 
}
