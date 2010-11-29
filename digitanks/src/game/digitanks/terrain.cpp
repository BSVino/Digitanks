#include "terrain.h"

#include <simplex.h>
#include <maths.h>
#include <time.h>

#include <GL/glew.h>

#include <raytracer/raytracer.h>

#include "dt_renderer.h"
#include "digitanksgame.h"
#include <digitanks/projectiles/projectile.h>
#include "ui/digitankswindow.h"
#include "shaders/shaders.h"

using namespace raytrace;

NETVAR_TABLE_BEGIN(CTerrain);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTerrain);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bHeightsInitialized);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flHighest);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLowest);
	//SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iCallList);
	//raytrace::CRaytracer*	m_pTracer;	// Regenerated procedurally
	SAVEDATA_DEFINE(CSaveData::DATA_COPYARRAY, Vector, m_avecTerrainColors);
	//SAVEDATA_DEFINE(CSaveData::DATA_COPYARRAY, CTerrainChunk, m_aTerrainChunks);	// Onserialize
SAVEDATA_TABLE_END();

CTerrain::CTerrain()
{
	SetCollisionGroup(CG_TERRAIN);
}

CTerrain::~CTerrain()
{
}

void CTerrain::Spawn()
{
	BaseClass::Spawn();

	switch (mtrand()%4)
	{
	case 0:
		m_avecTerrainColors[0] = Vector(0.40f, 0.41f, 0.55f);
		m_avecTerrainColors[1] = Vector(0.40f, 0.40f, 0.55f);
		m_avecTerrainColors[2] = Vector(0.41f, 0.41f, 0.55f);
		m_avecTerrainColors[3] = Vector(0.41f, 0.40f, 0.55f);
		break;

	case 1:
		m_avecTerrainColors[0] = Vector(0.55f, 0.47f, 0.28f);
		m_avecTerrainColors[1] = Vector(0.55f, 0.45f, 0.28f);
		m_avecTerrainColors[2] = Vector(0.58f, 0.47f, 0.28f);
		m_avecTerrainColors[3] = Vector(0.58f, 0.49f, 0.28f);
		break;

	case 2:
		m_avecTerrainColors[0] = Vector(0.28f, 0.55f, 0.47f);
		m_avecTerrainColors[1] = Vector(0.28f, 0.55f, 0.45f);
		m_avecTerrainColors[2] = Vector(0.28f, 0.58f, 0.47f);
		m_avecTerrainColors[3] = Vector(0.28f, 0.58f, 0.49f);
		break;

	case 3:
		m_avecTerrainColors[0] = Vector(0.55f, 0.25f, 0.27f);
		m_avecTerrainColors[1] = Vector(0.55f, 0.25f, 0.29f);
		m_avecTerrainColors[2] = Vector(0.58f, 0.25f, 0.27f);
		m_avecTerrainColors[3] = Vector(0.58f, 0.25f, 0.25f);
		break;
	}

	m_bHeightsInitialized = false;
}

void CTerrain::GenerateTerrain(float flHeight)
{
	CSimplexNoise n1(m_iSpawnSeed);
	CSimplexNoise n2(m_iSpawnSeed+1);
	CSimplexNoise n3(m_iSpawnSeed+2);
	CSimplexNoise n4(m_iSpawnSeed+3);
	CSimplexNoise n5(m_iSpawnSeed+4);

	float flSpaceFactor1 = 0.01f;
	float flHeightFactor1 = flHeight;
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
			float flHeight;
			flHeight  = n1.Noise(x*flSpaceFactor1, y*flSpaceFactor1) * flHeightFactor1;
			flHeight += n2.Noise(x*flSpaceFactor2, y*flSpaceFactor2) * flHeightFactor2;
			flHeight += n3.Noise(x*flSpaceFactor3, y*flSpaceFactor3) * flHeightFactor3;
			flHeight += n4.Noise(x*flSpaceFactor4, y*flSpaceFactor4) * flHeightFactor4;
			flHeight += n5.Noise(x*flSpaceFactor5, y*flSpaceFactor5) * flHeightFactor5;
			SetRealHeight(x, y, flHeight);

			if (!m_bHeightsInitialized)
				m_flHighest = m_flLowest = flHeight;
			m_bHeightsInitialized = true;

			if (flHeight < m_flLowest)
				m_flLowest = flHeight;

			if (flHeight > m_flHighest)
				m_flHighest = flHeight;
		}
	}
}

void CTerrain::GenerateCollision()
{
	assert(m_bHeightsInitialized);

	// Don't need the collision mesh in the menu
	if (DigitanksGame()->GetGameType() != GAMETYPE_MENU)
	{
		for (size_t x = 0; x < TERRAIN_GEN_SECTORS; x++)
		{
			for (size_t y = 0; y < TERRAIN_GEN_SECTORS; y++)
			{
				CTerrainChunk* pChunk = &m_aTerrainChunks[x][y];

				if (pChunk->m_pTracer)
					delete pChunk->m_pTracer;

				pChunk->m_pTracer = new raytrace::CRaytracer();

				for (size_t i = 0; i < TERRAIN_SECTOR_SIZE; i++)
				{
					for (size_t j = 0; j < TERRAIN_SECTOR_SIZE; j++)
					{
						float flX = ChunkToWorldSpace(x, i);
						float flY = ChunkToWorldSpace(y, j);
						float flX1 = ChunkToWorldSpace(x, i+1);
						float flY1 = ChunkToWorldSpace(y, j+1);

						Vector v1 = Vector(flX, GetRealHeight(ChunkToArraySpace(x, i), ChunkToArraySpace(y, j)), flY);
						Vector v2 = Vector(flX, GetRealHeight(ChunkToArraySpace(x, i), ChunkToArraySpace(y, j+1)), flY1);
						Vector v3 = Vector(flX1, GetRealHeight(ChunkToArraySpace(x, i+1), ChunkToArraySpace(y, j+1)), flY1);
						Vector v4 = Vector(flX1, GetRealHeight(ChunkToArraySpace(x, i+1), ChunkToArraySpace(y, j)), flY);

						pChunk->m_pTracer->AddTriangle(v1, v2, v3);
						pChunk->m_pTracer->AddTriangle(v1, v3, v4);
					}
				}

				pChunk->m_bNeedsRegenerate = true;
				pChunk->m_pTracer->BuildTree();
			}
		}
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
			GenerateTerrainCallList(i, j);
		}
	}
}

void CTerrain::GenerateTerrainCallList(int i, int j)
{
	CTerrainChunk* pChunk = &m_aTerrainChunks[i][j];
	if (!pChunk->m_bNeedsRegenerate)
		return;

	glNewList((GLuint)pChunk->m_iCallList, GL_COMPILE);
	glBegin(GL_QUADS);

	glPushAttrib(GL_CURRENT_BIT);

	for (int x = TERRAIN_SECTOR_SIZE*i; x < TERRAIN_SECTOR_SIZE*(i+1); x++)
	{
		if (x >= TERRAIN_SIZE-1)
			continue;

		for (int y = TERRAIN_SECTOR_SIZE*j; y < TERRAIN_SECTOR_SIZE*(j+1); y++)
		{
			if (y >= TERRAIN_SIZE-1)
				continue;

			float flColor = RemapVal(GetRealHeight(x, y), m_flLowest, m_flHighest, 0.0f, 0.98f);

			float flX = ArrayToWorldSpace((int)x);
			float flX1 = ArrayToWorldSpace((int)x+1);
			float flY = ArrayToWorldSpace((int)y);
			float flY1 = ArrayToWorldSpace((int)y+1);

			glColor3fv(flColor*m_avecTerrainColors[0]);
			glVertex3f(flX, GetRealHeight(x, y), flY);

			glColor3fv(flColor*m_avecTerrainColors[1]);
			glVertex3f(flX, GetRealHeight(x, y+1), flY1);

			glColor3fv(flColor*m_avecTerrainColors[2]);
			glVertex3f(flX1, GetRealHeight(x+1, y+1), flY1);

			glColor3fv(flColor*m_avecTerrainColors[3]);
			glVertex3f(flX1, GetRealHeight(x+1, y), flY);
		}
	}

	glPopAttrib();

	glEnd();
	glEndList();

	pChunk->m_bNeedsRegenerate = false;
}

void CTerrain::GenerateCallLists()
{
	for (size_t i = 0; i < TERRAIN_GEN_SECTORS; i++)
	{
		for (size_t j = 0; j < TERRAIN_GEN_SECTORS; j++)
		{
			CTerrainChunk* pChunk = &m_aTerrainChunks[i][j];

			if (pChunk->m_iCallList)
				glDeleteLists((GLuint)pChunk->m_iCallList, 1);

			pChunk->m_iCallList = glGenLists(1);
		}
	}

	GenerateTerrainCallLists();
}

void CTerrain::OnRender()
{
	BaseClass::OnRender();

	if (GameServer()->GetRenderer()->ShouldUseShaders())
		RenderWithShaders();
	else
		RenderWithoutShaders();
}

void CTerrain::RenderWithShaders()
{
	glPushAttrib(GL_ENABLE_BIT);

	GLuint iTerrainProgram = (GLuint)CShaderLibrary::GetTerrainProgram();
	GameServer()->GetRenderer()->UseProgram(iTerrainProgram);

	CDigitank* pCurrentTank = DigitanksGame()->GetPrimarySelectionTank();

	bool bIsCurrentTeam = false;
	if (pCurrentTank && pCurrentTank->GetTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
		bIsCurrentTeam = true;

	if (bIsCurrentTeam && pCurrentTank && !pCurrentTank->IsFortified() && DigitanksGame()->GetControlMode() == MODE_MOVE)
	{
		GLuint vecTankOrigin = glGetUniformLocation(iTerrainProgram, "vecTankOrigin");
		glUniform3fv(vecTankOrigin, 1, pCurrentTank->GetOrigin());

		GLuint flMoveDistance = glGetUniformLocation(iTerrainProgram, "flMoveDistance");
		glUniform1f(flMoveDistance, pCurrentTank->GetMaxMovementDistance());

		float flMoveFire = pCurrentTank->GetMaxMovementDistance() - pCurrentTank->GetProjectileEnergy() * pCurrentTank->GetTankSpeed();
		GLuint flMoveFireDistance = glGetUniformLocation(iTerrainProgram, "flMoveFireDistance");
		glUniform1f(flMoveFireDistance, flMoveFire>0?flMoveFire:0);

		GLuint bMovement = glGetUniformLocation(iTerrainProgram, "bMovement");
		glUniform1i(bMovement, true);
	}
	else if (bIsCurrentTeam && pCurrentTank && !pCurrentTank->IsFortified() && DigitanksGame()->GetControlMode() == MODE_CHARGE)
	{
		GLuint vecTankOrigin = glGetUniformLocation(iTerrainProgram, "vecTankOrigin");
		glUniform3fv(vecTankOrigin, 1, pCurrentTank->GetOrigin());

		GLuint flMoveDistance = glGetUniformLocation(iTerrainProgram, "flMoveDistance");
		glUniform1f(flMoveDistance, pCurrentTank->ChargeRadius());

		GLuint flMoveFireDistance = glGetUniformLocation(iTerrainProgram, "flMoveFireDistance");
		glUniform1f(flMoveFireDistance, pCurrentTank->ChargeRadius());

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
		bool bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecPoint);

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

	eastl::vector<Vector> avecTankAims;
	eastl::vector<float> aflTankAimRadius;
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

	for (size_t i = 0; i < TERRAIN_GEN_SECTORS; i++)
	{
		for (size_t j = 0; j < TERRAIN_GEN_SECTORS; j++)
			glCallList((GLuint)m_aTerrainChunks[i][j].m_iCallList);
	}

	GameServer()->GetRenderer()->ClearProgram();

	glPopAttrib();
}

void CTerrain::RenderWithoutShaders()
{
	for (size_t i = 0; i < TERRAIN_GEN_SECTORS; i++)
	{
		for (size_t j = 0; j < TERRAIN_GEN_SECTORS; j++)
			glCallList((GLuint)m_aTerrainChunks[i][j].m_iCallList);
	}
}

void CTerrain::GetChunk(float x, float y, int& i, int& j)
{
	int iIndex;
	i = WorldToChunkSpace(x, iIndex);
	j = WorldToChunkSpace(y, iIndex);
}

CTerrainChunk* CTerrain::GetChunk(int x, int y)
{
	if (x >= TERRAIN_GEN_SECTORS)
		return NULL;

	if (y >= TERRAIN_GEN_SECTORS)
		return NULL;

	if (x < 0 || y < 0)
		return NULL;

	return &m_aTerrainChunks[x][y];
}

CTerrainChunk* CTerrain::GetChunk(float x, float y)
{
	int i, j;
	GetChunk(x, y, i, j);
	return GetChunk(i, j);
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

	int i, j;
	CTerrainChunk* pChunk = &m_aTerrainChunks[ArrayToChunkSpace(x, i)][ArrayToChunkSpace(y, j)];
	return pChunk->m_aflHeights[i][j];
}

void CTerrain::SetRealHeight(int x, int y, float flHeight)
{
	int iXIndex, iYIndex;
	int iChunkX = ArrayToChunkSpace(x, iXIndex);
	int iChunkY = ArrayToChunkSpace(y, iYIndex);

	CTerrainChunk* pChunk = GetChunk(iChunkX, iChunkY);

	pChunk->m_aflHeights[iXIndex][iYIndex] = flHeight;
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

int CTerrain::ArrayToChunkSpace(int i, int& iIndex)
{
	iIndex = i%TERRAIN_SECTOR_SIZE;
	return i/TERRAIN_SECTOR_SIZE;
}

int CTerrain::ChunkToArraySpace(int iChunk, int i)
{
	return iChunk * TERRAIN_SECTOR_SIZE + i;
}

float CTerrain::ChunkToWorldSpace(int iChunk, int i)
{
	return ArrayToWorldSpace(ChunkToArraySpace(iChunk, i));
}

int CTerrain::WorldToChunkSpace(float f, int& iIndex)
{
	return ArrayToChunkSpace(WorldToArraySpace(f), iIndex);
}

bool CTerrain::Collide(const Vector& v1, const Vector& v2, Vector& vecPoint)
{
	vecPoint = v2;
	bool bReturn = false;
	for (int i = 0; i < TERRAIN_GEN_SECTORS; i++)
	{
		for (int j = 0; j < TERRAIN_GEN_SECTORS; j++)
		{
			CTerrainChunk* pChunk = GetChunk(i, j);
			if (pChunk->m_pTracer)
			{
				raytrace::CTraceResult tr;
				bool bHit = pChunk->m_pTracer->Raytrace(v1, v2, &tr);
				if (bHit)
				{
					if ((v1-tr.m_vecHit).LengthSqr() < (v1-vecPoint).LengthSqr())
						vecPoint = tr.m_vecHit;
					bReturn = true;
				}
			}
		}
	}

	return bReturn;
}

void CTerrain::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->CreatesCraters())
		return;

	float flRadius = 4.0f;
	if (pProjectile)
		flRadius = pProjectile->ExplosionRadius();

	int iRadius = WorldToArraySpace(flRadius)-WorldToArraySpace(0)+1;

	Vector vecOrigin = pInflictor->GetOrigin();

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
				float flAbove = GetRealHeight(x, z) - (flSqrt + vecOrigin.y);
				if (flAbove > 0)
					flNewY += flAbove;

				if (flNewY > GetRealHeight(x, z))
					continue;

				SetRealHeight(x, z, flNewY);

				int iIndex;
				int iChunkX = ArrayToChunkSpace(x, iIndex);
				int iChunkY = ArrayToChunkSpace(z, iIndex);

				CTerrainChunk* pChunk = GetChunk(iChunkX, iChunkY);
				if (pChunk && !pChunk->m_bNeedsRegenerate)
					pChunk->m_bNeedsRegenerate = true;

				// Also regenerate nearby chunks which may have been affected.
				iChunkX = ArrayToChunkSpace(x-1, iIndex);
				iChunkY = ArrayToChunkSpace(z, iIndex);
				pChunk = GetChunk(iChunkX, iChunkY);
				if (pChunk && !pChunk->m_bNeedsRegenerate)
					pChunk->m_bNeedsRegenerate = true;

				iChunkX = ArrayToChunkSpace(x, iIndex);
				iChunkY = ArrayToChunkSpace(z-1, iIndex);
				pChunk = GetChunk(iChunkX, iChunkY);
				if (pChunk && !pChunk->m_bNeedsRegenerate)
					pChunk->m_bNeedsRegenerate = true;

				iChunkX = ArrayToChunkSpace(x-1, iIndex);
				iChunkY = ArrayToChunkSpace(z-1, iIndex);
				pChunk = GetChunk(iChunkX, iChunkY);
				if (pChunk && !pChunk->m_bNeedsRegenerate)
					pChunk->m_bNeedsRegenerate = true;
			}
		}
	}

	bool bTerrainDeformed = false;
	for (size_t i = 0; i < TERRAIN_GEN_SECTORS; i++)
	{
		for (size_t j = 0; j < TERRAIN_GEN_SECTORS; j++)
		{
			CTerrainChunk* pChunk = GetChunk((int)i, (int)j);
			if (pChunk->m_bNeedsRegenerate)
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
			CTerrainChunk* pChunk = GetChunk((int)i, (int)j);
			if (!pChunk->m_bNeedsRegenerate)
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
					flHeightData[iPosition++] = GetRealHeight(x, z);
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

	CTerrainChunk* pChunk = GetChunk((int)i, j);

	// Unserialize the height data
	for (int x = TERRAIN_SECTOR_SIZE*i; x < (int)(TERRAIN_SECTOR_SIZE*(i+1)); x++)
	{
		for (int z = TERRAIN_SECTOR_SIZE*j; z < (int)(TERRAIN_SECTOR_SIZE*(j+1)); z++)
		{
			if (fabs(GetRealHeight(x, z) - flHeightData[iPosition]) > 0.01f)
			{
				pChunk->m_bNeedsRegenerate = true;
				SetRealHeight(x, z, flHeightData[iPosition]);

				float flHeight = flHeightData[iPosition];

				if (!m_bHeightsInitialized)
				{
					m_flHighest = m_flLowest = flHeight;
					m_bHeightsInitialized = true;
				}
				else
				{
					if (flHeight < m_flLowest)
						m_flLowest = flHeight;

					if (flHeight > m_flHighest)
						m_flHighest = flHeight;
				}
			}

			iPosition++;
		}
	}

	if (!pChunk->m_bNeedsRegenerate)
		return;

	if (pChunk->m_pTracer)
		delete pChunk->m_pTracer;

	pChunk->m_pTracer = new raytrace::CRaytracer();

	int iXMin = (int)(TERRAIN_SECTOR_SIZE*i);
	int iYMin = (int)(TERRAIN_SECTOR_SIZE*j);
	int iXMax = (int)(TERRAIN_SECTOR_SIZE*(i+1));
	int iYMax = (int)(TERRAIN_SECTOR_SIZE*(j+1));

	for (int x = iXMin; x < iXMax; x++)
	{
		for (int z = iYMin; z < iYMax; z++)
		{
			float flX = ArrayToWorldSpace(x);
			float flZ = ArrayToWorldSpace(z);

			float flX1 = ArrayToWorldSpace((int)x+1);
			float flZ1 = ArrayToWorldSpace((int)z+1);

			Vector v1 = Vector(flX, GetRealHeight(x, z), flZ);
			Vector v2 = Vector(flX, GetRealHeight(x, z+1), flZ1);
			Vector v3 = Vector(flX1, GetRealHeight(x+1, z+1), flZ1);
			Vector v4 = Vector(flX1, GetRealHeight(x+1, z), flZ);

			pChunk->m_pTracer->AddTriangle(v1, v2, v3);
			pChunk->m_pTracer->AddTriangle(v1, v3, v4);
		}
	}

	if (!GameServer()->IsLoading())
	{
		pChunk->m_pTracer->BuildTree();
		GenerateTerrainCallList(i, j);
	}
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
					flHeightData[iPosition++] = GetRealHeight(x, z);
			}

			CNetwork::CallFunctionParameters(iClient, "TerrainData", &p);
		}
	}
}

void CTerrain::OnSerialize(std::ostream& o)
{
	for (size_t x = 0; x < TERRAIN_GEN_SECTORS; x++)
	{
		for (size_t y = 0; y < TERRAIN_GEN_SECTORS; y++)
		{
			CTerrainChunk* pChunk = &m_aTerrainChunks[x][y];
			o.write((char*)&pChunk->m_aflHeights[0][0], sizeof(pChunk->m_aflHeights));
		}
	}

	BaseClass::OnSerialize(o);
}

bool CTerrain::OnUnserialize(std::istream& i)
{
	for (size_t x = 0; x < TERRAIN_GEN_SECTORS; x++)
	{
		for (size_t y = 0; y < TERRAIN_GEN_SECTORS; y++)
		{
			CTerrainChunk* pChunk = &m_aTerrainChunks[x][y];
			i.read((char*)&pChunk->m_aflHeights[0][0], sizeof(pChunk->m_aflHeights));
			pChunk->m_bNeedsRegenerate = true;
		}
	}

	return BaseClass::OnUnserialize(i);
}

void CTerrain::ClientEnterGame()
{
	BaseClass::ClientEnterGame();

	GenerateCollision();
}

CTerrainChunk::CTerrainChunk()
{
	m_pTracer = NULL;
	m_iCallList = 0;
	m_bNeedsRegenerate = true;
}

CTerrainChunk::~CTerrainChunk()
{
	if (m_iCallList)
		glDeleteLists((GLuint)m_iCallList, 1);
}
