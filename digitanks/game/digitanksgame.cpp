#include "digitanksgame.h"

#include <assert.h>
#include <maths.h>
#include "powerup.h"
#include "terrain.h"

CDigitanksGame::CDigitanksGame()
{
	m_iCurrentTeam = 0;
	m_iCurrentTank = 0;

	m_pListener = NULL;

	m_bWaitingForProjectiles = false;
	m_iWaitingForProjectiles = 0;
}

CDigitanksGame::~CDigitanksGame()
{
	for (size_t i = 0; i < m_apTeams.size(); i++)
		delete m_apTeams[i];
}

void CDigitanksGame::SetupDefaultGame()
{
	for (size_t i = 0; i < m_apTeams.size(); i++)
		delete m_apTeams[i];

	m_apTeams.clear();

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i))->Delete();

	m_hTerrain = new CTerrain();

	Vector avecStartingPositions[] =
	{
		Vector(-80, 0, -80),
		Vector(-80, 0, 0),
		Vector(-80, 0, 80),
		Vector(0, 0, 80),
		Vector(80, 0, 80),
		Vector(80, 0, 0),
		Vector(80, 0, -80),
		Vector(0, 0, -80),
	};

	Color aclrTeamColors[] =
	{
		Color(0, 0, 255),
		Color(255, 0, 0),
		Color(0, 255, 0),
		Color(255, 255, 0),
		Color(255, 0, 255),
		Color(0, 255, 255),
		Color(0, 0, 0),
		Color(255, 255, 255),
	};

	std::vector<Vector> avecRandomStartingPositions;
	for (size_t i = 0; i < 8; i++)
	{
		// 8 random starting positions.
		if (rand()%2)
			avecRandomStartingPositions.push_back(avecStartingPositions[i]);
		else
			avecRandomStartingPositions.insert(avecRandomStartingPositions.begin(), avecStartingPositions[i]);
	}

	for (size_t i = 0; i < 8; i++)
	{
		m_apTeams.push_back(new CTeam());

		m_apTeams[i]->m_clrTeam = aclrTeamColors[i];

		m_apTeams[i]->AddTank(new CDigitank());
		m_apTeams[i]->AddTank(new CDigitank());
		m_apTeams[i]->AddTank(new CDigitank());

		Vector vecTank1 = avecRandomStartingPositions[i];
		vecTank1.y = m_hTerrain->GetHeight(vecTank1.x, vecTank1.z);
		Vector vecTank2 = avecRandomStartingPositions[i] + Vector(10, 0, 10);
		vecTank2.y = m_hTerrain->GetHeight(vecTank2.x, vecTank2.z);
		Vector vecTank3 = avecRandomStartingPositions[i] - Vector(10, 0, 10);
		vecTank3.y = m_hTerrain->GetHeight(vecTank3.x, vecTank3.z);

		m_apTeams[i]->m_ahTanks[0]->SetOrigin(vecTank1);
		m_apTeams[i]->m_ahTanks[1]->SetOrigin(vecTank2);
		m_apTeams[i]->m_ahTanks[2]->SetOrigin(vecTank3);

		EAngle angTanks = VectorAngles(-avecRandomStartingPositions[i].Normalized());
		m_apTeams[i]->m_ahTanks[0]->SetAngles(angTanks);
		m_apTeams[i]->m_ahTanks[1]->SetAngles(angTanks);
		m_apTeams[i]->m_ahTanks[2]->SetAngles(angTanks);

		m_apTeams[i]->m_ahTanks[0]->GiveBonusPoints(2);
		m_apTeams[i]->m_ahTanks[1]->GiveBonusPoints(1);
		m_apTeams[i]->m_ahTanks[2]->GiveBonusPoints(1);
	}

	CPowerup* pPowerup = new CPowerup();
	pPowerup->SetOrigin(Vector(10, m_hTerrain->GetHeight(10, 10), 10));
	pPowerup = new CPowerup();
	pPowerup->SetOrigin(Vector(10, m_hTerrain->GetHeight(10, -10), -10));
	pPowerup = new CPowerup();
	pPowerup->SetOrigin(Vector(-10, m_hTerrain->GetHeight(-10, 10), 10));
	pPowerup = new CPowerup();
	pPowerup->SetOrigin(Vector(-10, m_hTerrain->GetHeight(-10, -10), -10));

	StartGame();
}

void CDigitanksGame::StartGame()
{
	if (m_pListener)
		m_pListener->GameStart();

	m_iCurrentTeam = 0;
	m_iCurrentTank = 0;

	if (m_pListener)
		m_pListener->NewCurrentTeam();

	GetCurrentTeam()->StartTurn();

	if (m_pListener)
		m_pListener->NewCurrentTank();
}

void CDigitanksGame::Think()
{
	BaseClass::Think();

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
		{
			CTeam* pTeam = GetCurrentTeam();
			for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
			{
				CDigitank* pTank = pTeam->GetTank(i);
				pTank->SetPreviewMove(pTank->GetOrigin());
				pTank->SetDesiredMove();
			}
			return;
		}

		Vector vecPreview = GetCurrentTank()->GetPreviewMove();
		Vector vecOrigin = GetCurrentTank()->GetOrigin();
		Vector vecMove = vecPreview - vecOrigin;

		CTeam* pTeam = GetCurrentTeam();
		for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
		{
			CDigitank* pTank = pTeam->GetTank(i);

			Vector vecTankMove = vecMove;
			if (vecMove.Length() > pTank->GetTotalMovementPower())
				vecTankMove = vecMove.Normalized() * pTank->GetTotalMovementPower() * 0.95f;

			Vector vecNewPosition = pTank->GetOrigin() + vecTankMove;
			vecNewPosition.y = GetTerrain()->GetHeight(vecNewPosition.x, vecNewPosition.z);

			pTank->SetPreviewMove(vecNewPosition);
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
				vecTankAim = pTank->GetDesiredMove() + (vecTankAim - pTank->GetDesiredMove()).Normalized() * pTank->GetMaxRange() * 0.99f;

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

	if (m_pListener)
		m_pListener->NewCurrentTank();
}

void CDigitanksGame::EndTurn()
{
	GetCurrentTeam()->MoveTanks();
	GetCurrentTeam()->FireTanks();

	m_bWaitingForProjectiles = true;
}

void CDigitanksGame::StartTurn()
{
	m_iCurrentTank = 0;

	if (++m_iCurrentTeam >= GetNumTeams())
		m_iCurrentTeam = 0;

	if (m_pListener)
		m_pListener->NewCurrentTeam();

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
			float flMovementPower = pTank->GetBasePower() + pTank->GetBonusMovementPower();
			Vector vecDirection = pTarget->GetOrigin() - pTank->GetOrigin();
			vecDirection = vecDirection.Normalized() * (flMovementPower/3);

			Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
			vecDesiredMove.y = GetTerrain()->GetHeight(vecDesiredMove.x, vecDesiredMove.z);

			pTank->SetPreviewMove(vecDesiredMove);
			pTank->SetDesiredMove();
		}

		// If we are within the max range, try to fire.
		if ((pTarget->GetOrigin() - pTank->GetDesiredMove()).LengthSqr() < pTank->GetMaxRange()*pTank->GetMaxRange())
		{
			pTank->SetPreviewAim(pTarget->GetOrigin());
			pTank->SetDesiredAim();
		}

		pTank->SetAttackPower((pTank->GetBasePower() - pTank->GetMovementPower())/2);
	}

	EndTurn();
}

void CDigitanksGame::OnKilled(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_apTeams.size(); i++)
		m_apTeams[i]->OnKilled(pEntity);

	CheckWinConditions();
}

void CDigitanksGame::CheckWinConditions()
{
	for (size_t i = 0; i < m_apTeams.size(); i++)
	{
		if (m_apTeams[i]->GetNumTanksAlive() == 0)
		{
			delete m_apTeams[i];
			m_apTeams.erase(m_apTeams.begin()+i);
		}
	}

	if (m_apTeams.size() <= 1)
	{
		if (m_pListener)
			m_pListener->GameOver();

		SetupDefaultGame();
	}
}

void CDigitanksGame::OnDeleted(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_apTeams.size(); i++)
		m_apTeams[i]->OnDeleted(pEntity);

	if (dynamic_cast<class CProjectile*>(pEntity) != NULL)
	{
		if (m_iWaitingForProjectiles > 0)
			m_iWaitingForProjectiles--;
	}
}

CTeam* CDigitanksGame::GetCurrentTeam()
{
	if (m_iCurrentTeam > m_apTeams.size())
		return NULL;

	return m_apTeams[m_iCurrentTeam];
}

CDigitank* CDigitanksGame::GetCurrentTank()
{
	if (!GetCurrentTeam())
		return NULL;

	if (m_iCurrentTank > GetCurrentTeam()->GetNumTanks())
		return NULL;

	return GetCurrentTeam()->GetTank(m_iCurrentTank);
}

float CDigitanksGame::GetGravity()
{
	return -20;
}