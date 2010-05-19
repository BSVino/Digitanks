#include "terrain.h"

#include <simplex.h>
#include <maths.h>
#include <time.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "digitanksgame.h"
#include "ui/digitankswindow.h"
#include "shaders/shaders.h"

using namespace raytrace;

REGISTER_ENTITY(CTerrain);

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
	float flHeightFactor1 = 60.0f;
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
			float flX = ArrayToWorldSpace(x);
			float flY = ArrayToWorldSpace(y);

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

			float flX = ArrayToWorldSpace(x);
			float flX1 = ArrayToWorldSpace(x+1);
			float flY = ArrayToWorldSpace(y);
			float flY1 = ArrayToWorldSpace(y+1);

			glColor4f(flColor*0.40f, flColor*0.50f, flColor, 0.5f);
			glVertex3f(flX, m_aflHeights[x][y], flY);

			glColor4f(flColor*0.40f, flColor*0.48f, flColor, 0.5f);
			glVertex3f(flX, m_aflHeights[x][y+1], flY1);

			glColor4f(flColor*0.43f, flColor*0.50f, flColor, 0.5f);
			glVertex3f(flX1, m_aflHeights[x+1][y+1], flY1);

			glColor4f(flColor*0.43f, flColor*0.52f, flColor, 0.5f);
			glVertex3f(flX1, m_aflHeights[x+1][y], flY);
		}
	}
	glEnd();
	glEndList();
}

void CTerrain::OnRender()
{
	glPushAttrib(GL_ENABLE_BIT);

	GLuint iTerrainProgram = (GLuint)CShaderLibrary::GetTerrainProgram();
	glUseProgram(iTerrainProgram);

	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	if (pCurrentTank && CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
	{
		GLuint vecTankOrigin = glGetUniformLocation(iTerrainProgram, "vecTankOrigin");
		glUniform3fv(vecTankOrigin, 1, pCurrentTank->GetOrigin());

		GLuint flMoveDistance = glGetUniformLocation(iTerrainProgram, "flMoveDistance");
		glUniform1f(flMoveDistance, pCurrentTank->GetTotalMovementPower());

		GLuint bMovement = glGetUniformLocation(iTerrainProgram, "bMovement");
		glUniform1i(bMovement, true);
	}
	else
	{
		GLuint bMovement = glGetUniformLocation(iTerrainProgram, "bMovement");
		glUniform1i(bMovement, false);
	}

	if (pCurrentTank && CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
	{
		Vector vecPoint;
		bool bMouseOnGrid = CDigitanksWindow::Get()->GetMouseGridPosition(vecPoint);

		if (bMouseOnGrid)
		{
			GLuint vecTankOrigin = glGetUniformLocation(iTerrainProgram, "vecTankOrigin");
			glUniform3fv(vecTankOrigin, 1, pCurrentTank->GetDesiredMove());

			GLuint vecTurnPosition = glGetUniformLocation(iTerrainProgram, "vecTurnPosition");
			glUniform3fv(vecTurnPosition, 1, vecPoint);

			GLuint bTurnValid = glGetUniformLocation(iTerrainProgram, "bTurnValid");

			GLuint flTankYaw = glGetUniformLocation(iTerrainProgram, "flTankYaw");
			glUniform1f(flTankYaw, pCurrentTank->GetAngles().y);

			float flMaxTurnWithLeftoverPower = (pCurrentTank->GetTotalMovementPower() - pCurrentTank->GetMovementPower()) * pCurrentTank->TurnPerPower();

			GLuint flTankMaxYaw = glGetUniformLocation(iTerrainProgram, "flTankMaxYaw");
			glUniform1f(flTankMaxYaw, flMaxTurnWithLeftoverPower);

			Vector vecDirection = (vecPoint - pCurrentTank->GetDesiredMove()).Normalized();
			float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

			float flTankTurn = AngleDifference(flYaw, pCurrentTank->GetAngles().y);
			if (pCurrentTank->GetPreviewMovePower() + fabs(flTankTurn)/pCurrentTank->TurnPerPower() > pCurrentTank->GetTotalMovementPower())
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
		glUniform1i(bFocusRanges, CDigitanksWindow::Get()->GetControlMode() == MODE_AIM);

		Vector vecRangeOrigin = pCurrentTank->GetDesiredMove();
		if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE && pCurrentTank->GetPreviewMovePower() <= pCurrentTank->GetBasePower())
			vecRangeOrigin = pCurrentTank->GetPreviewMove();

		GLuint vecTankPreviewOrigin = glGetUniformLocation(iTerrainProgram, "vecTankPreviewOrigin");
		glUniform3fv(vecTankPreviewOrigin, 1, vecRangeOrigin);

		GLuint flTankMaxRange = glGetUniformLocation(iTerrainProgram, "flTankMaxRange");
		glUniform1f(flTankMaxRange, pCurrentTank->GetMaxRange());

		GLuint flTankMinRange = glGetUniformLocation(iTerrainProgram, "flTankMinRange");
		glUniform1f(flTankMinRange, pCurrentTank->GetMinRange());
	}
	else
	{
		GLuint bShowRanges = glGetUniformLocation(iTerrainProgram, "bShowRanges");
		glUniform1i(bShowRanges, false);
	}
	
//	glDisable(GL_DEPTH_TEST);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glCallList((GLuint)m_iCallList);

	glUseProgram(0);

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

void CTerrain::SetPointHeight(Vector& vecPoint)
{
	vecPoint.y = GetHeight(vecPoint.x, vecPoint.z);
}

float CTerrain::GetMapSize()
{
	return 200;
}

float CTerrain::ArrayToWorldSpace(size_t i)
{
	return RemapVal((float)i, 0, TERRAIN_SIZE, -GetMapSize(), GetMapSize());
}

size_t CTerrain::WorldToArraySpace(float f)
{
	return (size_t)RemapVal(f, -GetMapSize(), GetMapSize(), 0, TERRAIN_SIZE);
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
