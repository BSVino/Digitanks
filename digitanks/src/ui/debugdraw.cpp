#include "debugdraw.h"

#include <GL/glew.h>

void DebugLine(Vector a, Vector b, Color c)
{
	glColor4ubv(c);

	glLineWidth(1);
    
	glBegin(GL_LINES);

	glVertex3fv(a);
	glVertex3fv(b);

	glEnd(); 
}

void DebugCircle(Vector vecOrigin, float flRadius, Color c)
{
	DebugArc(vecOrigin, flRadius, 0, 360, c);
}

void DebugArc(Vector vecOrigin, float flRadius, float flStartDegree, float flEndDegree, Color c)
{
	int iLines = (int)flRadius;
	if (iLines < 20)
		iLines = 20;

	if (flStartDegree > flEndDegree)
	{
		float f = flStartDegree;
		flStartDegree = flEndDegree;
		flEndDegree = f;
	}

	glColor4ubv(c);

	glLineWidth(1);
    
	glBegin(GL_LINE_STRIP);

	float x = flRadius * cos(flStartDegree * M_PI/180.0f) + vecOrigin.x;
	float z = flRadius * sin(flStartDegree * M_PI/180.0f) + vecOrigin.z;

	glVertex3f(x, vecOrigin.y, z);

	for (int i = (int)flStartDegree*iLines/360; i <= (int)flEndDegree*iLines/360; i++)
	{
		float flDegree = i*360.0f/iLines;

		if (flDegree < flStartDegree || flDegree > flEndDegree)
			continue;

		x = flRadius * cos(flDegree * M_PI/180.0f) + vecOrigin.x;
		z = flRadius * sin(flDegree * M_PI/180.0f) + vecOrigin.z;

		glVertex3f(x, vecOrigin.y, z);
	}

	x = flRadius * cos(flEndDegree * M_PI/180.0f) + vecOrigin.x;
	z = flRadius * sin(flEndDegree * M_PI/180.0f) + vecOrigin.z;

	glVertex3f(x, vecOrigin.y, z);

	glEnd(); 
}
