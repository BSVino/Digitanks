#include "terrain.h"

#include <simplex.h>
#include <maths.h>
#include <time.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "digitanksgame.h"

using namespace raytrace;

CTerrain::CTerrain()
{
	SetCollisionGroup(CG_TERRAIN);

	size_t iSeed = (size_t)time(NULL);

	CSimplexNoise n1(iSeed);
	CSimplexNoise n2(iSeed+1);
	CSimplexNoise n3(iSeed+2);
	CSimplexNoise n4(iSeed+3);
	CSimplexNoise n5(iSeed+4);

	float flSpaceFactor1 = 0.01f;
	float flHeightFactor1 = 30.0f;
	float flSpaceFactor2 = flSpaceFactor1*3;
	float flHeightFactor2 = flHeightFactor1/3;
	float flSpaceFactor3 = flSpaceFactor2*3;
	float flHeightFactor3 = flHeightFactor2/3;
	float flSpaceFactor4 = flSpaceFactor3*3;
	float flHeightFactor4 = flHeightFactor3/3;
	float flSpaceFactor5 = flSpaceFactor4*3;
	float flHeightFactor5 = flHeightFactor4/3;

	for (size_t x = 0; x < TERRAIN_SIZE; x++)
	{
		for (size_t y = 0; y < TERRAIN_SIZE; y++)
		{
			m_aflHeights[x][y]  = n1.Noise(x*flSpaceFactor1, y*flSpaceFactor1) * flHeightFactor1;
			m_aflHeights[x][y] += n2.Noise(x*flSpaceFactor2, y*flSpaceFactor2) * flHeightFactor2;
			m_aflHeights[x][y] += n3.Noise(x*flSpaceFactor3, y*flSpaceFactor3) * flHeightFactor3;
			m_aflHeights[x][y] += n4.Noise(x*flSpaceFactor4, y*flSpaceFactor4) * flHeightFactor4;
			m_aflHeights[x][y] += n5.Noise(x*flSpaceFactor5, y*flSpaceFactor5) * flHeightFactor5;

			if (x == 0 && y == 0)
				m_flHighest = m_flLowest = m_aflHeights[x][y];

			if (m_aflHeights[x][y] < m_flLowest)
				m_flLowest = m_aflHeights[x][y];

			if (m_aflHeights[x][y] > m_flHighest)
				m_flHighest = m_aflHeights[x][y];
		}
	}

	m_pTracer = new raytrace::CRaytracer();

	for (size_t x = 0; x < TERRAIN_SIZE-1; x++)
	{
		for (size_t y = 0; y < TERRAIN_SIZE-1; y++)
		{
			float flX = x-(TERRAIN_SIZE/2.0f);
			float flY = y-(TERRAIN_SIZE/2.0f);

			Vector v1 = Vector(flX, m_aflHeights[x][y], flY);
			Vector v2 = Vector(flX, m_aflHeights[x][y+1], flY+1);
			Vector v3 = Vector(flX+1, m_aflHeights[x+1][y+1], flY+1);
			Vector v4 = Vector(flX+1, m_aflHeights[x+1][y], flY);

			m_pTracer->AddTriangle(v1, v2, v3);
			m_pTracer->AddTriangle(v1, v3, v4);
		}
	}

	m_pTracer->BuildTree();

	GenerateCallLists();
}

CTerrain::~CTerrain()
{
	delete m_pTracer;
	glDeleteLists((GLuint)m_iCallList, 1);
}

void CTerrain::GenerateCallLists()
{
	if (m_iCallList)
		glDeleteLists((GLuint)m_iCallList, 1);

	m_iCallList = glGenLists(1);
	glNewList((GLuint)m_iCallList, GL_COMPILE);
	glBegin(GL_QUADS);
	for (size_t x = 0; x < TERRAIN_SIZE-1; x++)
	{
		for (size_t y = 0; y < TERRAIN_SIZE-1; y++)
		{
			float flColor = RemapVal(m_aflHeights[x][y], m_flLowest, m_flHighest, 0.0f, 0.98f);
			glColor4f(flColor*0.40f, flColor*0.50f, flColor, 0.5f);
			glVertex3f(x-(TERRAIN_SIZE/2.0f), m_aflHeights[x][y], y-(TERRAIN_SIZE/2.0f));

			glColor4f(flColor*0.40f, flColor*0.48f, flColor, 0.5f);
			glVertex3f(x-(TERRAIN_SIZE/2.0f), m_aflHeights[x][y+1], y-(TERRAIN_SIZE/2.0f)+1);

			glColor4f(flColor*0.43f, flColor*0.50f, flColor, 0.5f);
			glVertex3f(x-(TERRAIN_SIZE/2.0f)+1, m_aflHeights[x+1][y+1], y-(TERRAIN_SIZE/2.0f)+1);

			glColor4f(flColor*0.43f, flColor*0.52f, flColor, 0.5f);
			glVertex3f(x-(TERRAIN_SIZE/2.0f)+1, m_aflHeights[x+1][y], y-(TERRAIN_SIZE/2.0f));
		}
	}
	glEnd();
	glEndList();
}

void CTerrain::Render()
{
	glPushAttrib(GL_ENABLE_BIT);

//	glDisable(GL_DEPTH_TEST);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glCallList((GLuint)m_iCallList);

	glPopAttrib();
}

float CTerrain::GetHeight(float flX, float flY)
{
	int iX = (int)(flX+(TERRAIN_SIZE/2.0f));
	int iY = (int)(flY+(TERRAIN_SIZE/2.0f));

	if (iX < 0)
		iX = 0;
	if (iX >= TERRAIN_SIZE)
		iX = TERRAIN_SIZE-1;
	if (iY < 0)
		iY = 0;
	if (iY >= TERRAIN_SIZE)
		iY = TERRAIN_SIZE-1;

	return m_aflHeights[iX][iY];
}

float CTerrain::GetMapSize()
{
	return 100;
}

bool CTerrain::Collide(const Ray& rayTrace, Vector &vecHit)
{
	CTraceResult tr;
	bool bHit = m_pTracer->Raytrace(rayTrace, &tr);
	if (bHit)
		vecHit = tr.m_vecHit;
	return bHit;
}

bool CTerrain::Collide(const Vector& s1, const Vector& s2, Vector &vecHit)
{
	CTraceResult tr;
	bool bHit = m_pTracer->Raytrace(s1, s2, &tr);
	if (bHit)
		vecHit = tr.m_vecHit;
	return bHit;
}
