#include "terrain.h"

#include <simplex.h>
#include <maths.h>
#include <time.h>

#include <GL/glew.h>

#include "digitanksgame.h"
#include "projectile.h"
#include "ui/digitankswindow.h"
#include "shaders/shaders.h"

using namespace raytrace;

NETVAR_TABLE_BEGIN(CTerrain);
NETVAR_TABLE_END();

CTerrain::CTerrain()
{
	SetCollisionGroup(CG_TERRAIN);
	m_pTracer = NULL;
	m_iCallList = 0;
}

CTerrain::~CTerrain()
{
	if (m_pTracer)
		delete m_pTracer;

	if (m_iCallList)
		glDeleteLists((GLuint)m_iCallList, TERRAIN_GEN_SECTORS*TERRAIN_GEN_SECTORS+1);
}

void CTerrain::Spawn()
{
	BaseClass::Spawn();

	size_t iSeed = GetSpawnSeed();

	CSimplexNoise n1(iSeed);
	CSimplexNoise n2(iSeed+1);
	CSimplexNoise n3(iSeed+2);
	CSimplexNoise n4(iSeed+3);
	CSimplexNoise n5(iSeed+4);

	float flSpaceFactor1 = 0.01f;
	float flHeightFactor1 = 50.0f;
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
			float flX = ArrayToWorldSpace((int)x);
			float flY = ArrayToWorldSpace((int)y);
			float flX1 = ArrayToWorldSpace((int)x+1);
			float flY1 = ArrayToWorldSpace((int)y+1);

			Vector v1 = Vector(flX, m_aflHeights[x][y], flY);
			Vector v2 = Vector(flX, m_aflHeights[x][y+1], flY1);
			Vector v3 = Vector(flX1, m_aflHeights[x+1][y+1], flY1);
			Vector v4 = Vector(flX1, m_aflHeights[x+1][y], flY);

			m_pTracer->AddTriangle(v1, v2, v3);
			m_pTracer->AddTriangle(v1, v3, v4);
		}
	}

	m_pTracer->BuildTree();

	switch (mtrand()%4)
	{
	case 0:
		m_avecTerrainColors[0] = Vector(0.24f, 0.30f, 0.55f);
		m_avecTerrainColors[1] = Vector(0.24f, 0.28f, 0.55f);
		m_avecTerrainColors[2] = Vector(0.27f, 0.30f, 0.55f);
		m_avecTerrainColors[3] = Vector(0.27f, 0.32f, 0.55f);
		break;

	case 1:
		m_avecTerrainColors[0] = Vector(0.55f, 0.40f, 0.0f);
		m_avecTerrainColors[1] = Vector(0.55f, 0.38f, 0.0f);
		m_avecTerrainColors[2] = Vector(0.58f, 0.40f, 0.0f);
		m_avecTerrainColors[3] = Vector(0.58f, 0.42f, 0.0f);
		break;

	case 2:
		m_avecTerrainColors[0] = Vector(0.0, 0.55f, 0.40f);
		m_avecTerrainColors[1] = Vector(0.0, 0.55f, 0.38f);
		m_avecTerrainColors[2] = Vector(0.0, 0.58f, 0.40f);
		m_avecTerrainColors[3] = Vector(0.0, 0.58f, 0.42f);
		break;

	case 3:
		m_avecTerrainColors[0] = Vector(0.55f, 0.0f, 0.23f);
		m_avecTerrainColors[1] = Vector(0.55f, 0.0f, 0.21f);
		m_avecTerrainColors[2] = Vector(0.58f, 0.0f, 0.23f);
		m_avecTerrainColors[3] = Vector(0.58f, 0.0f, 0.25f);
		break;
	}

	for (size_t i = 0; i < TERRAIN_GEN_SECTORS; i++)
	{
		for (size_t j = 0; j < TERRAIN_GEN_SECTORS; j++)
			m_abTerrainNeedsRegenerate[i][j] = true;
	}

	GenerateCallLists();
}

void CTerrain::GenerateTerrainCallLists()
{
	// Break it up into sectors of smaller size so that when it comes time to regenerate,
	// it can be done only for the sector that needs it and it won't take too long
	for (size_t i = 0; i < TERRAIN_GEN_SECTORS; i++)
	{
		for (size_t j = 0; j < TERRAIN_GEN_SECTORS; j++)
		{
			if (!m_abTerrainNeedsRegenerate[i][j])
				continue;

			glNewList((GLuint)m_iCallList+i*TERRAIN_GEN_SECTORS+j+1, GL_COMPILE);
			glBegin(GL_QUADS);
			for (size_t x = TERRAIN_SECTOR_SIZE*i; x < TERRAIN_SECTOR_SIZE*(i+1); x++)
			{
				if (x >= TERRAIN_SIZE-1)
					continue;

				for (size_t y = TERRAIN_SECTOR_SIZE*j; y < TERRAIN_SECTOR_SIZE*(j+1); y++)
				{
					if (y >= TERRAIN_SIZE-1)
						continue;

					float flColor = RemapVal(m_aflHeights[x][y], m_flLowest, m_flHighest, 0.0f, 0.98f);

					float flX = ArrayToWorldSpace((int)x);
					float flX1 = ArrayToWorldSpace((int)x+1);
					float flY = ArrayToWorldSpace((int)y);
					float flY1 = ArrayToWorldSpace((int)y+1);

					glColor3fv(flColor*m_avecTerrainColors[0]);
					glVertex3f(flX, m_aflHeights[x][y], flY);

					glColor3fv(flColor*m_avecTerrainColors[1]);
					glVertex3f(flX, m_aflHeights[x][y+1], flY1);

					glColor3fv(flColor*m_avecTerrainColors[2]);
					glVertex3f(flX1, m_aflHeights[x+1][y+1], flY1);

					glColor3fv(flColor*m_avecTerrainColors[3]);
					glVertex3f(flX1, m_aflHeights[x+1][y], flY);
				}
			}
			glEnd();
			glEndList();

			m_abTerrainNeedsRegenerate[i][j] = false;
		}
	}
}

void CTerrain::GenerateCallLists()
{
	if (m_iCallList)
		glDeleteLists((GLuint)m_iCallList, TERRAIN_GEN_SECTORS*TERRAIN_GEN_SECTORS+1);

	m_iCallList = glGenLists(TERRAIN_GEN_SECTORS*TERRAIN_GEN_SECTORS+1);

	GenerateTerrainCallLists();

	glNewList((GLuint)m_iCallList, GL_COMPILE);
	float flLo = ArrayToWorldSpace(0);
	float flHi = ArrayToWorldSpace(TERRAIN_SIZE-1);

	float flWallHeight = 7.0f;
	float flWallSlant = 5.0f;
	Vector vecWallColor(0.6f, 0.6f, 0.6f);

	glBegin(GL_QUADS);

	for (int x = -10; x < TERRAIN_SIZE-1+10; x++)
	{
		float flX = ArrayToWorldSpace(x);
		float flX1 = ArrayToWorldSpace(x+1);

		float flAverage = 0;
		float flAverage1 = 0;

		for (int i = ((int)x)-3; i <= ((int)x)+3; i++)
			flAverage += GetRealHeight(i, 0);

		flAverage /= 7;

		for (int i = ((int)x+1)-3; i <= ((int)x)+1+3; i++)
			flAverage1 += GetRealHeight(i, 0);

		flAverage1 /= 7;

		Vector vecColor = vecWallColor/2;
		glColor3fv(vecColor);
		glVertex3f(flX, GetRealHeight(x, 0), flLo);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3f(flX1, GetRealHeight(x+1, 0), flLo);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3f(flX1, flAverage1+flWallHeight, flLo);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3f(flX, flAverage+flWallHeight, flLo);

		Vector vecA(flX, flAverage+flWallHeight, flLo);
		Vector vecB(flX1, flAverage1+flWallHeight, flLo);
		Vector vecC(flX1, flAverage1+flWallHeight+flWallSlant, flLo-flWallSlant);
		Vector vecD(flX, flAverage+flWallHeight+flWallSlant, flLo-flWallSlant);
		vecColor = vecWallColor * Vector(0, -1, 0).Dot((vecA-vecB).Normalized().Cross((vecC-vecB).Normalized()));
		glColor3fv(vecColor);
		glVertex3fv(vecA);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecB);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3fv(vecC);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecD);

		vecA = Vector(flX, flAverage+flWallHeight+flWallSlant, flLo-flWallSlant);
		vecB = Vector(flX1, flAverage1+flWallHeight+flWallSlant, flLo-flWallSlant);
		vecC = Vector(flX1, flAverage1+flWallHeight+flWallSlant, flLo-20);
		vecD = Vector(flX, flAverage+flWallHeight+flWallSlant, flLo-20);
		vecColor = vecWallColor * Vector(0, -1, 0).Dot((vecA-vecB).Normalized().Cross((vecC-vecB).Normalized()));
		glColor3fv(vecColor);
		glVertex3fv(vecA);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecB);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3fv(vecC);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecD);

		flAverage = 0;
		for (int i = x-3; i <= x+3; i++)
			flAverage += GetRealHeight(i, TERRAIN_SIZE-1);

		flAverage /= 7;

		flAverage1 = 0;
		for (int i = x+1-3; i <= x+1+3; i++)
			flAverage1 += GetRealHeight(i, TERRAIN_SIZE-1);

		flAverage1 /= 7;

		vecColor = vecWallColor/2;
		glColor3fv(vecColor);
		glVertex3f(flX, GetRealHeight(x, TERRAIN_SIZE-1), flHi);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3f(flX, flAverage+flWallHeight, flHi);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3f(flX1, flAverage1+flWallHeight, flHi);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3f(flX1, GetRealHeight(x+1, TERRAIN_SIZE-1), flHi);

		vecA = Vector(flX, flAverage+flWallHeight, flHi);
		vecB = Vector(flX, flAverage+flWallHeight+flWallSlant, flHi+flWallSlant);
		vecC = Vector(flX1, flAverage1+flWallHeight+flWallSlant, flHi+flWallSlant);
		vecD = Vector(flX1, flAverage1+flWallHeight, flHi);
		vecColor = vecWallColor * Vector(0, -1, 0).Dot((vecA-vecB).Normalized().Cross((vecC-vecB).Normalized()));
		glColor3fv(vecColor);
		glVertex3fv(vecA);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecB);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3fv(vecC);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecD);

		vecA = Vector(flX, flAverage+flWallHeight+flWallSlant, flHi+flWallSlant);
		vecB = Vector(flX, flAverage+flWallHeight+flWallSlant, flHi+20);
		vecC = Vector(flX1, flAverage1+flWallHeight+flWallSlant, flHi+20);
		vecD = Vector(flX1, flAverage1+flWallHeight+flWallSlant, flHi+flWallSlant);
		vecColor = vecWallColor * Vector(0, -1, 0).Dot((vecA-vecB).Normalized().Cross((vecC-vecB).Normalized()));
		glColor3fv(vecColor);
		glVertex3fv(vecA);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecB);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3fv(vecC);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecD);
	}

	for (int z = -10; z < TERRAIN_SIZE-1+10; z++)
	{
		float flZ = ArrayToWorldSpace(z);
		float flZ1 = ArrayToWorldSpace(z+1);

		float flAverage = 0;
		float flAverage1 = 0;

		for (int i = z-3; i <= z+3; i++)
			flAverage += GetRealHeight(0, i);

		flAverage /= 7;

		for (int i = z+1-3; i <= z+1+3; i++)
			flAverage1 += GetRealHeight(0, i);

		flAverage1 /= 7;

		Vector vecColor = vecWallColor/2;
		glColor3fv(vecColor);
		glVertex3f(flLo, GetRealHeight(0, z), flZ);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3f(flLo, flAverage+flWallHeight, flZ);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3f(flLo, flAverage1+flWallHeight, flZ1);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3f(flLo, GetRealHeight(0, z+1), flZ1);

		Vector vecA(flLo, flAverage+flWallHeight, flZ);
		Vector vecB(flLo-flWallSlant, flAverage+flWallHeight+flWallSlant, flZ);
		Vector vecC(flLo-flWallSlant, flAverage1+flWallHeight+flWallSlant, flZ1);
		Vector vecD(flLo, flAverage1+flWallHeight, flZ1);
		vecColor = vecWallColor * Vector(0, -1, 0).Dot((vecA-vecB).Normalized().Cross((vecC-vecB).Normalized()));
		glColor3fv(vecColor);
		glVertex3fv(vecA);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecB);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3fv(vecC);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecD);

		vecA = Vector(flLo-flWallSlant, flAverage+flWallHeight+flWallSlant, flZ);
		vecB = Vector(flLo-20, flAverage+flWallHeight+flWallSlant, flZ);
		vecC = Vector(flLo-20, flAverage1+flWallHeight+flWallSlant, flZ1);
		vecD = Vector(flLo-flWallSlant, flAverage1+flWallHeight+flWallSlant, flZ1);
		vecColor = vecWallColor * Vector(0, -1, 0).Dot((vecA-vecB).Normalized().Cross((vecC-vecB).Normalized()));
		glColor3fv(vecColor);
		glVertex3fv(vecA);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecB);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3fv(vecC);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecD);

		flAverage = 0;
		for (int i = ((int)z)-3; i <= ((int)z)+3; i++)
			flAverage += GetRealHeight(TERRAIN_SIZE-1, i);

		flAverage /= 7;

		flAverage1 = 0;
		for (int i = ((int)z+1)-3; i <= ((int)z)+1+3; i++)
			flAverage1 += GetRealHeight(TERRAIN_SIZE-1, i);

		flAverage1 /= 7;

		vecColor = vecWallColor/2;
		glColor3fv(vecColor);
		glVertex3f(flHi, GetRealHeight(TERRAIN_SIZE-1, z), flZ);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3f(flHi, GetRealHeight(TERRAIN_SIZE-1, z+1), flZ1);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3f(flHi, flAverage1+flWallHeight, flZ1);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3f(flHi, flAverage+flWallHeight, flZ);

		vecA = Vector(flHi, flAverage+flWallHeight, flZ);
		vecB = Vector(flHi, flAverage1+flWallHeight, flZ1);
		vecC = Vector(flHi+flWallSlant, flAverage1+flWallHeight+flWallSlant, flZ1);
		vecD = Vector(flHi+flWallSlant, flAverage+flWallHeight+flWallSlant, flZ);
		vecColor = vecWallColor * Vector(0, -1, 0).Dot((vecA-vecB).Normalized().Cross((vecC-vecB).Normalized()));
		glColor3fv(vecColor);
		glVertex3fv(vecA);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecB);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3fv(vecC);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecD);

		vecA = Vector(flHi+flWallSlant, flAverage+flWallHeight+flWallSlant, flZ);
		vecB = Vector(flHi+flWallSlant, flAverage1+flWallHeight+flWallSlant, flZ1);
		vecC = Vector(flHi+20, flAverage1+flWallHeight+flWallSlant, flZ1);
		vecD = Vector(flHi+20, flAverage+flWallHeight+flWallSlant, flZ);
		vecColor = vecWallColor * Vector(0, -1, 0).Dot((vecA-vecB).Normalized().Cross((vecC-vecB).Normalized()));
		glColor3fv(vecColor);
		glVertex3fv(vecA);
		glColor3fv(vecColor + Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecB);
		glColor3fv(vecColor + Vector(0.01f, 0.01f, 0.01f));
		glVertex3fv(vecC);
		glColor3fv(vecColor - Vector(0.02f, 0.02f, 0.02f));
		glVertex3fv(vecD);
	}

	glEnd();

	// Now draw the blue haze. Draw it completely after the wall because it's transparent.
	float flBlueAlpha = 0.3f;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);

	for (int x = -10; x < TERRAIN_SIZE-1+10; x++)
	{
		float flX = ArrayToWorldSpace(x);
		float flX1 = ArrayToWorldSpace(x+1);

		float flAverage = 0;
		float flAverage1 = 0;

		for (int i = ((int)x)-3; i <= ((int)x)+3; i++)
			flAverage += GetRealHeight(i, 0);

		flAverage /= 7;

		for (int i = ((int)x+1)-3; i <= ((int)x)+1+3; i++)
			flAverage1 += GetRealHeight(i, 0);

		flAverage1 /= 7;

		if (flX >= -GetMapSize()-15 && flX1 <= GetMapSize()+15)
		{
			// Blue energy field
			glColor4f(0.5f, 0.5f, 1.0f, flBlueAlpha);
			glVertex3f(flX, flAverage+flWallHeight+flWallSlant, flLo-15);
			glVertex3f(flX1, flAverage1+flWallHeight+flWallSlant, flLo-15);
			glColor4f(0.5f, 0.5f, 1.0f, 0.0f);
			glVertex3f(flX1, flAverage1+flWallHeight+100, flLo-15);
			glVertex3f(flX, flAverage+flWallHeight+100, flLo-15);
		}

		flAverage = 0;
		for (int i = x-3; i <= x+3; i++)
			flAverage += GetRealHeight(i, TERRAIN_SIZE-1);

		flAverage /= 7;

		flAverage1 = 0;
		for (int i = x+1-3; i <= x+1+3; i++)
			flAverage1 += GetRealHeight(i, TERRAIN_SIZE-1);

		flAverage1 /= 7;

		if (flX >= -GetMapSize()-15 && flX1 <= GetMapSize()+15)
		{
			// Blue energy field
			glColor4f(0.5f, 0.5f, 1.0f, flBlueAlpha);
			glVertex3f(flX, flAverage+flWallHeight+flWallSlant, flHi+15);
			glColor4f(0.5f, 0.5f, 1.0f, 0.0f);
			glVertex3f(flX, flAverage+flWallHeight+100, flHi+15);
			glVertex3f(flX1, flAverage1+flWallHeight+100, flHi+15);
			glColor4f(0.5f, 0.5f, 1.0f, flBlueAlpha);
			glVertex3f(flX1, flAverage1+flWallHeight+flWallSlant, flHi+15);
		}
	}

	for (int z = -10; z < TERRAIN_SIZE-1+10; z++)
	{
		float flZ = ArrayToWorldSpace(z);
		float flZ1 = ArrayToWorldSpace(z+1);

		float flAverage = 0;
		float flAverage1 = 0;

		for (int i = z-3; i <= z+3; i++)
			flAverage += GetRealHeight(0, i);

		flAverage /= 7;

		for (int i = z+1-3; i <= z+1+3; i++)
			flAverage1 += GetRealHeight(0, i);

		flAverage1 /= 7;

		if (flZ >= -GetMapSize()-15 && flZ1 <= GetMapSize()+15)
		{
			// Blue energy field
			glColor4f(0.5f, 0.5f, 1.0f, flBlueAlpha);
			glVertex3f(flLo-15, flAverage+flWallHeight+flWallSlant, flZ);
			glColor4f(0.5f, 0.5f, 1.0f, 0.0f);
			glVertex3f(flLo-15, flAverage+flWallHeight+100, flZ);
			glVertex3f(flLo-15, flAverage1+flWallHeight+100, flZ1);
			glColor4f(0.5f, 0.5f, 1.0f, flBlueAlpha);
			glVertex3f(flLo-15, flAverage1+flWallHeight+flWallSlant, flZ1);
		}

		flAverage = 0;
		for (int i = ((int)z)-3; i <= ((int)z)+3; i++)
			flAverage += GetRealHeight(TERRAIN_SIZE-1, i);

		flAverage /= 7;

		flAverage1 = 0;
		for (int i = ((int)z+1)-3; i <= ((int)z)+1+3; i++)
			flAverage1 += GetRealHeight(TERRAIN_SIZE-1, i);

		flAverage1 /= 7;

		if (flZ >= -GetMapSize()-15 && flZ1 <= GetMapSize()+15)
		{
			// Blue energy field
			glColor4f(0.5f, 0.5f, 1.0f, flBlueAlpha);
			glVertex3f(flHi+15, flAverage+flWallHeight+flWallSlant, flZ);
			glVertex3f(flHi+15, flAverage1+flWallHeight+flWallSlant, flZ1);
			glColor4f(0.5f, 0.5f, 1.0f, 0.0f);
			glVertex3f(flHi+15, flAverage1+flWallHeight+100, flZ1);
			glVertex3f(flHi+15, flAverage+flWallHeight+100, flZ);
		}
	}

	glEnd();

	glDisable(GL_BLEND);

	glEndList();
}

void CTerrain::OnRender()
{
	BaseClass::OnRender();

	glPushAttrib(GL_ENABLE_BIT);

	GLuint iTerrainProgram = (GLuint)CShaderLibrary::GetTerrainProgram();
	glUseProgram(iTerrainProgram);

	CDigitank* pCurrentTank = DigitanksGame()->GetPrimarySelectionTank();

	bool bIsCurrentTeam = false;
	if (pCurrentTank && pCurrentTank->GetTeam() == Game()->GetLocalTeam())
		bIsCurrentTeam = true;

	if (bIsCurrentTeam && pCurrentTank && !pCurrentTank->IsFortified() && DigitanksGame()->GetControlMode() == MODE_MOVE)
	{
		GLuint vecTankOrigin = glGetUniformLocation(iTerrainProgram, "vecTankOrigin");
		glUniform3fv(vecTankOrigin, 1, pCurrentTank->GetOrigin());

		GLuint flMoveDistance = glGetUniformLocation(iTerrainProgram, "flMoveDistance");
		glUniform1f(flMoveDistance, pCurrentTank->GetMaxMovementDistance());

		GLuint bMovement = glGetUniformLocation(iTerrainProgram, "bMovement");
		glUniform1i(bMovement, true);
	}
	else
	{
		GLuint bMovement = glGetUniformLocation(iTerrainProgram, "bMovement");
		glUniform1i(bMovement, false);
	}

	if (bIsCurrentTeam && pCurrentTank && !pCurrentTank->IsFortified() && DigitanksGame()->GetControlMode() == MODE_TURN)
	{
		Vector vecPoint;
		bool bMouseOnGrid = CDigitanksWindow::Get()->GetMouseGridPosition(vecPoint);

		if (bMouseOnGrid)
		{
			GLuint vecTankOrigin = glGetUniformLocation(iTerrainProgram, "vecTankOrigin");
			glUniform3fv(vecTankOrigin, 1, pCurrentTank->GetOrigin());

			GLuint vecTurnPosition = glGetUniformLocation(iTerrainProgram, "vecTurnPosition");
			glUniform3fv(vecTurnPosition, 1, vecPoint);

			GLuint bTurnValid = glGetUniformLocation(iTerrainProgram, "bTurnValid");

			GLuint flTankYaw = glGetUniformLocation(iTerrainProgram, "flTankYaw");
			glUniform1f(flTankYaw, pCurrentTank->GetAngles().y);

			float flMaxTurnWithLeftoverPower = (pCurrentTank->GetTotalMovementPower() - pCurrentTank->GetMovementPower()) * pCurrentTank->TurnPerPower();

			GLuint flTankMaxYaw = glGetUniformLocation(iTerrainProgram, "flTankMaxYaw");
			glUniform1f(flTankMaxYaw, flMaxTurnWithLeftoverPower);

			Vector vecDirection = (vecPoint - pCurrentTank->GetOrigin()).Normalized();
			float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

			float flTankTurn = AngleDifference(flYaw, pCurrentTank->GetAngles().y);

			if (!pCurrentTank->IsPreviewMoveValid())
				glUniform1i(bTurnValid, true);
			else if (pCurrentTank->GetPreviewMovePower() + fabs(flTankTurn)/pCurrentTank->TurnPerPower() > pCurrentTank->GetTotalMovementPower())
				glUniform1i(bTurnValid, false);
			else
				glUniform1i(bTurnValid, true);
		}

		GLuint bTurning = glGetUniformLocation(iTerrainProgram, "bTurning");
		glUniform1i(bTurning, bMouseOnGrid);
	}
	else
	{
		GLuint bTurning = glGetUniformLocation(iTerrainProgram, "bTurning");
		glUniform1i(bTurning, false);
	}

	if (pCurrentTank)
	{
		GLuint bShowRanges = glGetUniformLocation(iTerrainProgram, "bShowRanges");
		glUniform1i(bShowRanges, true);

		GLuint bFocusRanges = glGetUniformLocation(iTerrainProgram, "bFocusRanges");
		glUniform1i(bFocusRanges, bIsCurrentTeam && DigitanksGame()->GetControlMode() == MODE_AIM);

		Vector vecRangeOrigin = pCurrentTank->GetOrigin();
		if (bIsCurrentTeam && DigitanksGame()->GetControlMode() == MODE_MOVE && pCurrentTank->GetPreviewMovePower() <= pCurrentTank->GetTotalPower())
			vecRangeOrigin = pCurrentTank->GetPreviewMove();

		GLuint vecTankPreviewOrigin = glGetUniformLocation(iTerrainProgram, "vecTankPreviewOrigin");
		glUniform3fv(vecTankPreviewOrigin, 1, vecRangeOrigin);

		GLuint flTankMaxRange = glGetUniformLocation(iTerrainProgram, "flTankMaxRange");
		glUniform1f(flTankMaxRange, pCurrentTank->GetMaxRange());

		GLuint flTankEffRange = glGetUniformLocation(iTerrainProgram, "flTankEffRange");
		glUniform1f(flTankEffRange, pCurrentTank->GetEffRange());

		GLuint flTankMinRange = glGetUniformLocation(iTerrainProgram, "flTankMinRange");
		glUniform1f(flTankMinRange, pCurrentTank->GetMinRange());

		GLuint flTankYaw = glGetUniformLocation(iTerrainProgram, "flTankYaw");
		if (bIsCurrentTeam && DigitanksGame()->GetControlMode() == MODE_TURN)
			glUniform1f(flTankYaw, pCurrentTank->GetPreviewTurn());
		else
			glUniform1f(flTankYaw, pCurrentTank->GetAngles().y);

		GLuint flTankFiringCone = glGetUniformLocation(iTerrainProgram, "flTankFiringCone");
		glUniform1f(flTankFiringCone, pCurrentTank->FiringCone());
	}
	else
	{
		GLuint bShowRanges = glGetUniformLocation(iTerrainProgram, "bShowRanges");
		glUniform1i(bShowRanges, false);
	}

	if (m_avecCraterMarks.size())
	{
		GLuint avecCraterMarks = glGetUniformLocation(iTerrainProgram, "avecCraterMarks");
		glUniform3fv(avecCraterMarks, (GLint)m_avecCraterMarks.size(), m_avecCraterMarks[0]);
	}

	GLuint iCraterMarks = glGetUniformLocation(iTerrainProgram, "iCraterMarks");
	glUniform1i(iCraterMarks, (GLint)m_avecCraterMarks.size());

	std::vector<Vector> avecTankAims;
	std::vector<float> aflTankAimRadius;
	size_t iTankAimFocus;

	DigitanksGame()->GetTankAims(avecTankAims, aflTankAimRadius, iTankAimFocus);
	DigitanksGame()->ClearTankAims();

	GLuint iAimTargets = glGetUniformLocation(iTerrainProgram, "iAimTargets");
	glUniform1i(iAimTargets, (GLint)avecTankAims.size());

	if (avecTankAims.size())
	{
		GLuint avecAimTargets = glGetUniformLocation(iTerrainProgram, "avecAimTargets");
		GLuint aflAimTargetRadius = glGetUniformLocation(iTerrainProgram, "aflAimTargetRadius");
		GLuint iFocusTarget = glGetUniformLocation(iTerrainProgram, "iFocusTarget");

		glUniform3fv(avecAimTargets, (GLint)avecTankAims.size(), avecTankAims[0]);
		glUniform1fv(aflAimTargetRadius, (GLint)aflTankAimRadius.size(), &aflTankAimRadius[0]);

		if (DigitanksGame()->GetControlMode() == MODE_AIM)
			glUniform1i(iFocusTarget, (GLint)iTankAimFocus);
		else
			glUniform1i(iFocusTarget, -1);
	}

	for (size_t i = 0; i < TERRAIN_GEN_SECTORS*TERRAIN_GEN_SECTORS; i++)
		glCallList((GLuint)m_iCallList+i+1);

	glUseProgram(0);

	glCallList((GLuint)m_iCallList);

	glPopAttrib();
}

float CTerrain::GetRealHeight(int x, int y)
{
	if (x < 0)
		x = 0;
	if (x >= TERRAIN_SIZE)
		x = TERRAIN_SIZE-1;
	if (y < 0)
		y = 0;
	if (y >= TERRAIN_SIZE)
		y = TERRAIN_SIZE-1;

	return m_aflHeights[x][y];
}

float CTerrain::GetHeight(float flX, float flY)
{
	float flX2 = RemapVal(flX, -GetMapSize(), GetMapSize(), 0, TERRAIN_SIZE);
	float flY2 = RemapVal(flY, -GetMapSize(), GetMapSize(), 0, TERRAIN_SIZE);

	int iX = (int)flX2;
	int iY = (int)flY2;

	float a = GetRealHeight(iX, iY);
	float b = GetRealHeight(iX, iY+1);
	float c = GetRealHeight(iX+1, iY);
	float d = GetRealHeight(iX+1, iY+1);

	float flXLerp = fmod(flX2, 1);
	float flYLerp = fmod(flY2, 1);

	float l1 = RemapVal(flYLerp, 0, 1, a, b);
	float l2 = RemapVal(flYLerp, 0, 1, c, d);

	return RemapVal(flXLerp, 0, 1, l1, l2);
}

Vector CTerrain::SetPointHeight(Vector& vecPoint)
{
	vecPoint.y = GetHeight(vecPoint.x, vecPoint.z);
	return vecPoint;
}

float CTerrain::GetMapSize() const
{
	return 200;
}

float CTerrain::ArrayToWorldSpace(int i)
{
	return RemapVal((float)i, 0, TERRAIN_SIZE, -GetMapSize(), GetMapSize());
}

int CTerrain::WorldToArraySpace(float f)
{
	return (int)RemapVal(f, -GetMapSize(), GetMapSize(), 0, TERRAIN_SIZE);
}

void CTerrain::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->CreatesCraters())
		return;

	float flRadius = 4.0f;
	int iRadius = WorldToArraySpace(flRadius)-WorldToArraySpace(0)+1;

	Vector vecOrigin = pInflictor->GetOrigin();

	m_avecCraterMarks.push_back(vecOrigin - Vector(0, flRadius, 0));
	if (m_avecCraterMarks.size() > 10)
		m_avecCraterMarks.erase(m_avecCraterMarks.begin());

	if (!CNetwork::IsHost())
		return;

	int iX = WorldToArraySpace(vecOrigin.x);
	int iZ = WorldToArraySpace(vecOrigin.z);

	Vector vecOriginFlat = vecOrigin;
	vecOriginFlat.y = 0;

	for (int x = iX-iRadius; x <= iX+iRadius; x++)
	{
		for (int z = iZ-iRadius; z <= iZ+iRadius; z++)
		{
			float flX = ArrayToWorldSpace(x);
			float flZ = ArrayToWorldSpace(z);

			float flX1 = ArrayToWorldSpace((int)x+1);
			float flZ1 = ArrayToWorldSpace((int)z+1);

			if ((Vector(flX, 0, flZ) - vecOriginFlat).LengthSqr() < flRadius*flRadius)
			{
				float flXDistance = (flX - vecOriginFlat.x);
				float flZDistance = (flZ - vecOriginFlat.z);

				float flSqrt = sqrt(flRadius*flRadius - flXDistance*flXDistance - flZDistance*flZDistance);
				float flNewY = -flSqrt + vecOrigin.y;

				// As if the dirt from above drops down into the hole.
				float flAbove = m_aflHeights[x][z] - (flSqrt + vecOrigin.y);
				if (flAbove > 0)
					flNewY += flAbove;

				// Stopgap to keep the raytracer from receiving triangles that go below its bounds.
				if (flNewY < m_flLowest)
					flNewY = m_flLowest+0.01f;

				if (flNewY > m_aflHeights[x][z])
					continue;

				m_aflHeights[x][z] = flNewY;

				m_abTerrainNeedsRegenerate[x/TERRAIN_SECTOR_SIZE][z/TERRAIN_SECTOR_SIZE] = true;
				m_abTerrainNeedsRegenerate[(x+1)/TERRAIN_SECTOR_SIZE][(z-1)/TERRAIN_SECTOR_SIZE] = true;
				m_abTerrainNeedsRegenerate[(x+1)/TERRAIN_SECTOR_SIZE][(z-1)/TERRAIN_SECTOR_SIZE] = true;
				m_abTerrainNeedsRegenerate[(x+1)/TERRAIN_SECTOR_SIZE][(z+1)/TERRAIN_SECTOR_SIZE] = true;
				m_abTerrainNeedsRegenerate[(x-1)/TERRAIN_SECTOR_SIZE][(z-1)/TERRAIN_SECTOR_SIZE] = true;
			}
		}
	}

	bool bTerrainDeformed = false;
	for (size_t i = 0; i < TERRAIN_GEN_SECTORS; i++)
	{
		for (size_t j = 0; j < TERRAIN_GEN_SECTORS; j++)
		{
			if (m_abTerrainNeedsRegenerate[i][j])
			{
				bTerrainDeformed = true;
				break;
			}
		}

		if (bTerrainDeformed)
			break;
	}

	if (!bTerrainDeformed)
		return;

	UpdateTerrainData();
}

bool CTerrain::Collide(const Vector& s1, const Vector& s2, Vector &vecHit)
{
	CTraceResult tr;
	bool bHit = m_pTracer->Raytrace(s1, s2, &tr);
	if (bHit)
		vecHit = tr.m_vecHit;
	return bHit;
}

Color CTerrain::GetPrimaryTerrainColor()
{
	Color clr = Color((int)(m_avecTerrainColors[0].x*255), (int)(m_avecTerrainColors[0].y*255), (int)(m_avecTerrainColors[0].z*255), 255);
	return clr;
}

void CTerrain::UpdateTerrainData()
{
	for (size_t i = 0; i < TERRAIN_GEN_SECTORS; i++)
	{
		for (size_t j = 0; j < TERRAIN_GEN_SECTORS; j++)
		{
			if (!m_abTerrainNeedsRegenerate[i][j])
				continue;

			CNetworkParameters p;
			p.ui1 = i;
			p.ui2 = j;

			p.CreateExtraData(sizeof(float)*TERRAIN_SECTOR_SIZE*TERRAIN_SECTOR_SIZE);

			size_t iPosition = 0;
			float* flHeightData = (float*)p.m_pExtraData;

			// Serialize the height data
			for (int x = TERRAIN_SECTOR_SIZE*i; x < (int)(TERRAIN_SECTOR_SIZE*(i+1)); x++)
			{
				for (int z = TERRAIN_SECTOR_SIZE*j; z < (int)(TERRAIN_SECTOR_SIZE*(j+1)); z++)
					flHeightData[iPosition++] = m_aflHeights[x][z];
			}

			if (CNetwork::ShouldReplicateClientFunction())
				CNetwork::CallFunctionParameters(NETWORK_TOCLIENTS, "TerrainData", &p);

			TerrainData(&p);
		}
	}
}

void CTerrain::TerrainData(class CNetworkParameters* p)
{
	size_t i = p->ui1;
	size_t j = p->ui2;

	size_t iPosition = 0;
	float* flHeightData = (float*)p->m_pExtraData;

	// Unserialize the height data
	for (int x = TERRAIN_SECTOR_SIZE*i; x < (int)(TERRAIN_SECTOR_SIZE*(i+1)); x++)
	{
		for (int z = TERRAIN_SECTOR_SIZE*j; z < (int)(TERRAIN_SECTOR_SIZE*(j+1)); z++)
		{
			if (fabs(m_aflHeights[x][z] - flHeightData[iPosition]) > 0.01f)
				m_abTerrainNeedsRegenerate[i][j] = true;

			m_aflHeights[x][z] = flHeightData[iPosition++];
		}
	}

	if (!m_abTerrainNeedsRegenerate[i][j])
		return;

	int iXMin = (int)(TERRAIN_SECTOR_SIZE*i);
	int iYMin = (int)(TERRAIN_SECTOR_SIZE*j);
	int iXMax = (int)(TERRAIN_SECTOR_SIZE*(i+1)-1);
	int iYMax = (int)(TERRAIN_SECTOR_SIZE*(j+1)-1);

	float flXMin = ArrayToWorldSpace(iXMin);
	float flYMin = ArrayToWorldSpace(iYMin);
	float flXMax = ArrayToWorldSpace(iXMax);
	float flYMax = ArrayToWorldSpace(iYMax);

	Vector vecMins(flXMin, -10000, flYMin);
	Vector vecMaxs(flXMax, 10000,  flYMax);

	m_pTracer->RemoveArea(AABB(vecMins, vecMaxs));
	for (int x = iXMin; x < iXMax; x++)
	{
		for (int z = iYMin; z < iYMax; z++)
		{
			float flX = ArrayToWorldSpace(x);
			float flZ = ArrayToWorldSpace(z);

			float flX1 = ArrayToWorldSpace((int)x+1);
			float flZ1 = ArrayToWorldSpace((int)z+1);

			Vector v1 = Vector(flX, m_aflHeights[x][z], flZ);
			Vector v2 = Vector(flX, m_aflHeights[x][z+1], flZ1);
			Vector v3 = Vector(flX1, m_aflHeights[x+1][z+1], flZ1);
			Vector v4 = Vector(flX1, m_aflHeights[x+1][z], flZ);

			m_pTracer->AddTriangle(v1, v2, v3);
			m_pTracer->AddTriangle(v1, v3, v4);
		}
	}

	GenerateTerrainCallLists();
}

void CTerrain::ResyncClientTerrainData(int iClient)
{
	for (size_t i = 0; i < TERRAIN_GEN_SECTORS; i++)
	{
		for (size_t j = 0; j < TERRAIN_GEN_SECTORS; j++)
		{
			CNetworkParameters p;
			p.ui1 = i;
			p.ui2 = j;

			p.CreateExtraData(sizeof(float)*TERRAIN_SECTOR_SIZE*TERRAIN_SECTOR_SIZE);

			size_t iPosition = 0;
			float* flHeightData = (float*)p.m_pExtraData;

			// Serialize the height data
			for (int x = TERRAIN_SECTOR_SIZE*i; x < (int)(TERRAIN_SECTOR_SIZE*(i+1)); x++)
			{
				for (int z = TERRAIN_SECTOR_SIZE*j; z < (int)(TERRAIN_SECTOR_SIZE*(j+1)); z++)
					flHeightData[iPosition++] = m_aflHeights[x][z];
			}

			CNetwork::CallFunctionParameters(iClient, "TerrainData", &p);
		}
	}
}
