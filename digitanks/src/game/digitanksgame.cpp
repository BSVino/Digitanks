#include "digitanksgame.h"

#include <assert.h>
#include <maths.h>
#include <models/models.h>
#include <sound/sound.h>
#include <renderer/particles.h>

#include "powerup.h"
#include "terrain.h"
#include "camera.h"

CDigitanksGame::CDigitanksGame()
{
	m_iCurrentTeam = 0;
	m_iCurrentTank = 0;

	m_pListener = NULL;

	m_bWaitingForMoving = false;

	m_bWaitingForProjectiles = false;
	m_iWaitingForProjectiles = 0;

	m_iPowerups = 0;

	m_iDifficulty = 1;
}

CDigitanksGame::~CDigitanksGame()
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
		m_ahTeams[i]->Delete();
}

void CDigitanksGame::SetupGame(int iPlayers, int iTanks)
{
	CSoundLibrary::StopSound();
	CParticleSystemLibrary::ClearInstances();

	for (size_t i = 0; i < m_ahTeams.size(); i++)
		m_ahTeams[i]->Delete();

	m_ahTeams.clear();

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i))->Delete();

	m_hTerrain = new CTerrain();

	if (iPlayers > 8)
		iPlayers = 8;
	if (iPlayers < 2)
		iPlayers = 2;

	if (iTanks > 5)
		iTanks = 5;
	if (iTanks < 1)
		iTanks = 1;

	Vector avecStartingPositions[] =
	{
		Vector(45, 0, 110),
		Vector(-45, 0, 110),
		Vector(-110, 0, 45),
		Vector(-110, 0, -45),
		Vector(-45, 0, -110),
		Vector(45, 0, -110),
		Vector(110, 0, -45),
		Vector(110, 0, 45),
	};

	Vector avecTankPositions[] =
	{
		Vector(0, 0, 0),
		Vector(15, 0, 15),
		Vector(-15, 0, -15),
		Vector(15, 0, -15),
		Vector(-15, 0, 15),
	};

	Color aclrTeamColors[] =
	{
		Color(0, 0, 255),
		Color(255, 255, 0),
		Color(255, 0, 255),
		Color(255, 0, 0),
		Color(0, 255, 0),
		Color(0, 255, 255),
		Color(0, 0, 0),
		Color(255, 255, 255),
	};

	std::vector<Vector> avecRandomStartingPositions;
	for (int i = 0; i < iPlayers; i++)
	{
		// 8 random starting positions.
		if (rand()%2)
			avecRandomStartingPositions.push_back(avecStartingPositions[i]);
		else
			avecRandomStartingPositions.insert(avecRandomStartingPositions.begin(), avecStartingPositions[i]);
	}

	for (int i = 0; i < iPlayers; i++)
	{
		m_ahTeams.push_back(new CTeam());

		m_ahTeams[i]->m_clrTeam = aclrTeamColors[i];

		for (int j = 0; j < iTanks; j++)
		{
			Vector vecTank = avecRandomStartingPositions[i] + avecTankPositions[j];
			EAngle angTank = VectorAngles(-vecTank.Normalized());

			m_ahTeams[i]->AddTank(new CDigitank());
			CDigitank* pTank = m_ahTeams[i]->m_ahTanks[j];

			vecTank.y = pTank->FindHoverHeight(vecTank);

			pTank->SetOrigin(vecTank);
			pTank->SetAngles(angTank);
			pTank->GiveBonusPoints(1, false);
		}
	}

	CPowerup* pPowerup = new CPowerup();
	pPowerup->SetOrigin(Vector(70, m_hTerrain->GetHeight(70, 70), 70));
	pPowerup = new CPowerup();
	pPowerup->SetOrigin(Vector(70, m_hTerrain->GetHeight(70, -70), -70));
	pPowerup = new CPowerup();
	pPowerup->SetOrigin(Vector(-70, m_hTerrain->GetHeight(-70, 70), 70));
	pPowerup = new CPowerup();
	pPowerup->SetOrigin(Vector(-70, m_hTerrain->GetHeight(-70, -70), -70));

	m_iPowerups = 4;

	StartGame();
}

void CDigitanksGame::StartGame()
{
	if (m_pListener)
		m_pListener->GameStart();

	m_iCurrentTeam = 0;
	m_iCurrentTank = 0;
	m_bWaitingForMoving = false;
	m_bWaitingForProjectiles = false;

	if (m_pListener)
	{
		m_pListener->SetHUDActive(true);
		m_pListener->NewCurrentTeam();
	}

	GetCurrentTeam()->StartTurn();

	if (m_pListener)
		m_pListener->NewCurrentTank();

	// Point the camera in to the center
	EAngle angCamera = VectorAngles(GetCurrentTank()->GetOrigin().Normalized());
	angCamera.p = 45;
	GetCamera()->SnapAngle(angCamera);
}

void CDigitanksGame::Think()
{
	BaseClass::Think();

	if (m_bWaitingForMoving)
	{
		bool bMoving = false;
		for (size_t i = 0; i < GetCurrentTeam()->GetNumTanks(); i++)
		{
			if (GetCurrentTeam()->GetTank(i)->IsMoving())
			{
				bMoving = true;
				break;
			}
		}

		if (!bMoving)
		{
			m_bWaitingForMoving = false;
			m_iWaitingForProjectiles = 0;
			GetCurrentTeam()->FireTanks();
			m_bWaitingForProjectiles = true;
		}
	}

	if (m_bWaitingForProjectiles)
	{
		if (m_iWaitingForProjectiles == 0)
		{
			m_bWaitingForProjectiles = false;
			StartTurn();
		}
	}
}

void CDigitanksGame::SetCurrentTank(CDigitank* pCurrentTank)
{
	bool bFoundNew = false;
	for (size_t i = 0; i < GetNumTeams(); i++)
	{
		CTeam* pTeam = GetTeam(i);
		for (size_t j = 0; j < pTeam->GetNumTanks(); j++)
		{
			CDigitank* pTank = pTeam->GetTank(j);

			if (GetCurrentTeam() != pTank->GetTeam())
				continue;

			if (pTank == pCurrentTank)
			{
				m_iCurrentTeam = i;
				m_iCurrentTank = j;
				bFoundNew = true;
				break;
			}
		}

		if (bFoundNew)
			break;
	}

	if (GetCurrentTank())
		GetCurrentTank()->OnCurrentTank();

	if (m_pListener)
		m_pListener->NewCurrentTank();
}

void CDigitanksGame::SetDesiredMove(bool bAllTanks)
{
	if (!GetCurrentTank())
		return;

	if (bAllTanks)
	{
		float flMovePower = GetCurrentTank()->GetPreviewMovePower();

		if (flMovePower > GetCurrentTank()->GetBasePower())
			return;

		Vector vecPreview = GetCurrentTank()->GetPreviewMove();
		Vector vecOrigin = GetCurrentTank()->GetOrigin();
		Vector vecMove = vecPreview - vecOrigin;

		CTeam* pTeam = GetCurrentTeam();
		for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
		{
			CDigitank* pTank = pTeam->GetTank(i);

			Vector vecTankMove = vecMove;

			Vector vecNewPosition = pTank->GetOrigin() + vecTankMove;
			vecNewPosition.y = pTank->FindHoverHeight(vecNewPosition);

			pTank->SetPreviewMove(vecNewPosition);

			do
			{
				if (pTank->GetPreviewMovePower() > pTank->GetMaxMovementDistance())
					vecTankMove = vecTankMove * 0.95f;

				if (vecTankMove.Length() < 1)
					break;

				vecNewPosition = pTank->GetOrigin() + vecTankMove;
				vecNewPosition.y = pTank->FindHoverHeight(vecNewPosition);

				pTank->SetPreviewMove(vecNewPosition);
			}
			while (pTank->GetPreviewMovePower() > pTank->GetMaxMovementDistance());

			pTank->SetDesiredMove();
		}
	}
	else
		GetCurrentTank()->SetDesiredMove();
}

void CDigitanksGame::SetDesiredTurn(bool bAllTanks, Vector vecLookAt)
{
	if (!GetCurrentTank())
		return;

	if (bAllTanks)
	{
		bool bNoTurn = (vecLookAt - GetCurrentTank()->GetDesiredMove()).LengthSqr() < 3*3;

		CTeam* pTeam = GetCurrentTeam();
		for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
		{
			CDigitank* pTank = pTeam->GetTank(i);

			if (bNoTurn)
				pTank->SetPreviewTurn(pTank->GetAngles().y);
			else
			{
				Vector vecDirection = (vecLookAt - pTank->GetDesiredMove()).Normalized();
				float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

				float flTankTurn = AngleDifference(flYaw, pTank->GetAngles().y);
				if (pTank->GetPreviewMovePower() + fabs(flTankTurn)/pTank->TurnPerPower() > pTank->GetTotalMovementPower())
					flTankTurn = (flTankTurn / fabs(flTankTurn)) * (pTank->GetTotalMovementPower() - pTank->GetPreviewMovePower()) * pTank->TurnPerPower() * 0.95f;

				pTank->SetPreviewTurn(pTank->GetAngles().y + flTankTurn);
			}

			pTank->SetDesiredTurn();
		}
	}
	else
		GetCurrentTank()->SetDesiredTurn();
}

void CDigitanksGame::SetDesiredAim(bool bAllTanks)
{
	if (!GetCurrentTank())
		return;

	if (bAllTanks)
	{
		Vector vecPreviewAim = GetCurrentTank()->GetPreviewAim();

		CTeam* pTeam = GetCurrentTeam();
		for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
		{
			CDigitank* pTank = pTeam->GetTank(i);

			Vector vecTankAim = vecPreviewAim;
			if ((vecTankAim - pTank->GetDesiredMove()).Length() > pTank->GetMaxRange())
			{
				vecTankAim = pTank->GetDesiredMove() + (vecTankAim - pTank->GetDesiredMove()).Normalized() * pTank->GetMaxRange() * 0.99f;
				vecTankAim.y = pTank->FindHoverHeight(vecTankAim);
			}

			pTank->SetPreviewAim(vecTankAim);
			pTank->SetDesiredAim();
		}
	}
	else
		GetCurrentTank()->SetDesiredAim();
}

void CDigitanksGame::NextTank()
{
	assert(GetCurrentTank());
	if (!GetCurrentTank())
		return;

	if (++m_iCurrentTank >= GetCurrentTeam()->GetNumTanks())
		m_iCurrentTank = 0;

	if (GetCurrentTank())
		GetCurrentTank()->OnCurrentTank();

	if (m_pListener)
		m_pListener->NewCurrentTank();
}

void CDigitanksGame::EndTurn()
{
	if (m_bWaitingForProjectiles || m_bWaitingForMoving)
		return;

	if (m_pListener)
		m_pListener->SetHUDActive(false);

	GetCurrentTeam()->MoveTanks();
	m_bWaitingForMoving = true;

	size_t iSum = 0;
	Vector vecSum;
	for (size_t i = 0; i < GetCurrentTeam()->GetNumTanks(); i++)
	{
		CDigitank* pTank = GetCurrentTeam()->GetTank(i);
		if (!pTank)
			continue;
		if (!pTank->HasDesiredAim())
			continue;
		vecSum += pTank->GetDesiredMove() + pTank->GetDesiredAim();
		iSum += 2;
	}

	if (iSum)
		GetCamera()->SetTarget(vecSum/(float)iSum);
}

void CDigitanksGame::StartTurn()
{
	if (m_iPowerups < 10 && rand()%6 == 0)
	{
		float flX = RemapVal((float)(rand()%1000), 0, 1000, -GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize());
		float flZ = RemapVal((float)(rand()%1000), 0, 1000, -GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize());

		CPowerup* pPowerup = new CPowerup();
		pPowerup->SetOrigin(Vector(flX, m_hTerrain->GetHeight(flX, flZ), flZ));

		m_iPowerups++;
	}

	m_iCurrentTank = 0;

	if (++m_iCurrentTeam >= GetNumTeams())
		m_iCurrentTeam = 0;

	if (m_pListener)
	{
		m_pListener->SetHUDActive(true);
		m_pListener->NewCurrentTeam();
	}

	GetCurrentTeam()->StartTurn();

	if (m_iCurrentTeam == 0)
	{
		if (m_pListener)
			m_pListener->NewCurrentTank();
	}
	else
		Bot_ExecuteTurn();
}

void CDigitanksGame::Bot_ExecuteTurn()
{
	CDigitank* pHeadTank = GetCurrentTeam()->GetTank(0);

	CDigitank* pTarget = NULL;

	// Find the nearest enemy to the head tank, he's our target.
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		if (!pTank)
			continue;

		if (pTank->GetTeam() == pHeadTank->GetTeam())
			continue;

		if (!pTarget)
		{
			pTarget = pTank;
			continue;
		}

		if ((pHeadTank->GetOrigin() - pTank->GetOrigin()).LengthSqr() < (pHeadTank->GetOrigin() - pTarget->GetOrigin()).LengthSqr())
			pTarget = pTank;
	}

	for (size_t i = 0; i < GetCurrentTeam()->GetNumTanks(); i++)
	{
		CDigitank* pTank = GetCurrentTeam()->GetTank(i);

		// Use any promotion points.
		while (pTank->HasBonusPoints())
		{
			switch (rand()%3)
			{
			case 0:
				pTank->PromoteAttack();
				break;

			case 1:
				pTank->PromoteDefense();
				break;

			case 2:
				pTank->PromoteMovement();
				break;
			}
		}

		// If we are not within the min range, use 1/3 of our available movement power to move towards our target.
		if ((pTarget->GetOrigin() - pTank->GetOrigin()).LengthSqr() > pTank->GetMinRange()*pTank->GetMinRange())
		{
			float flMovementDistance = pTank->GetMaxMovementDistance();
			Vector vecDirection = pTarget->GetOrigin() - pTank->GetOrigin();
			vecDirection = vecDirection.Normalized() * (flMovementDistance/3);

			Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
			vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

			pTank->SetPreviewMove(vecDesiredMove);
			pTank->SetDesiredMove();
		}

		// If we are within the max range, try to fire.
		if ((pTarget->GetOrigin() - pTank->GetPreviewMove()).LengthSqr() < pTank->GetMaxRange()*pTank->GetMaxRange())
		{
			pTank->SetPreviewAim(pTarget->GetOrigin());
			pTank->SetDesiredAim();
		}

		pTank->SetAttackPower((pTank->GetBasePower() - pTank->GetMovementPower())/2);
	}

	EndTurn();
}

void CDigitanksGame::Explode(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flRadius, float flDamage, CBaseEntity* pIgnore, CTeam* pTeamIgnore)
{
	std::vector<CBaseEntity*> apHit;
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);

		if (!pEntity)
			continue;

		if (pEntity == pIgnore)
			continue;

		if (dynamic_cast<CDigitank*>(pEntity) && dynamic_cast<CDigitank*>(pEntity)->GetTeam() == pTeamIgnore)
			continue;

		if (!pInflictor->ShouldTouch(pEntity))
			continue;

		float flDistanceSqr = (pInflictor->GetOrigin() - pEntity->GetOrigin()).LengthSqr();

		float flTotalRadius = flRadius + pEntity->GetBoundingRadius();
		if (flDistanceSqr < flTotalRadius*flTotalRadius)
			apHit.push_back(pEntity);
	}

	for (size_t i = 0; i < apHit.size(); i++)
	{
		float flDistance = (pInflictor->GetOrigin() - apHit[i]->GetOrigin()).Length();

		float flFalloffDamage = RemapVal(flDistance, 0, flRadius + apHit[i]->GetBoundingRadius(), flDamage, 0);
		if (flFalloffDamage <= 0)
			continue;

		apHit[i]->TakeDamage(pAttacker, pInflictor, flFalloffDamage, false);
	}
}

void CDigitanksGame::OnTakeShieldDamage(CDigitank* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit)
{
	if (m_pListener)
		m_pListener->OnTakeShieldDamage(pVictim, pAttacker, pInflictor, flDamage, bDirectHit);
}

void CDigitanksGame::OnTakeDamage(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit)
{
	if (m_pListener)
		m_pListener->OnTakeDamage(pVictim, pAttacker, pInflictor, flDamage, bDirectHit);
}

void CDigitanksGame::OnKilled(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
		m_ahTeams[i]->OnKilled(pEntity);

	CheckWinConditions();
}

void CDigitanksGame::CheckWinConditions()
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (m_ahTeams[i]->GetNumTanksAlive() == 0)
		{
			m_ahTeams[i]->Delete();
			m_ahTeams.erase(m_ahTeams.begin()+i);
		}
	}

	if (m_ahTeams.size() <= 1)
	{
		if (m_pListener)
			m_pListener->GameOver();
	}
}

void CDigitanksGame::OnDeleted(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
		m_ahTeams[i]->OnDeleted(pEntity);

	if (dynamic_cast<class CProjectile*>(pEntity) != NULL)
	{
		if (m_iWaitingForProjectiles > 0)
			m_iWaitingForProjectiles--;
	}

	if (dynamic_cast<class CPowerup*>(pEntity) != NULL)
		m_iPowerups--;
}

CTeam* CDigitanksGame::GetCurrentTeam()
{
	if (m_iCurrentTeam >= m_ahTeams.size())
		return NULL;

	return m_ahTeams[m_iCurrentTeam];
}

CDigitank* CDigitanksGame::GetCurrentTank()
{
	if (!GetCurrentTeam())
		return NULL;

	if (m_iCurrentTank >= GetCurrentTeam()->GetNumTanks())
		return NULL;

	return GetCurrentTeam()->GetTank(m_iCurrentTank);
}

size_t CDigitanksGame::GetCurrentTankId()
{
	return m_iCurrentTank;
}

float CDigitanksGame::GetGravity()
{
	return -20;
}

void CDigitanksGame::AddTankAim(Vector vecAim, float flRadius, bool bFocus)
{
	vecAim.y = 0;
	m_avecTankAims.push_back(vecAim);
	m_aflTankAimRadius.push_back(flRadius);
	if (bFocus)
		m_iTankAimFocus = m_avecTankAims.size()-1;
}

void CDigitanksGame::GetTankAims(std::vector<Vector>& avecAims, std::vector<float>& aflAimRadius, size_t& iFocus)
{
	avecAims = m_avecTankAims;
	aflAimRadius = m_aflTankAimRadius;
	iFocus = m_iTankAimFocus;
}

void CDigitanksGame::ClearTankAims()
{
	m_avecTankAims.clear();
	m_aflTankAimRadius.clear();
}
