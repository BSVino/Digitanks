#include "digitanksgame.h"

#include <assert.h>
#include "powerup.h"

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

	m_apTeams.push_back(new CTeam());
	m_apTeams.push_back(new CTeam());

	m_apTeams[0]->m_clrTeam = Color(255, 0, 0);
	m_apTeams[1]->m_clrTeam = Color(0, 0, 255);

	m_apTeams[0]->AddTank(new CDigitank());
	m_apTeams[0]->AddTank(new CDigitank());
	m_apTeams[0]->AddTank(new CDigitank());
	m_apTeams[1]->AddTank(new CDigitank());
	m_apTeams[1]->AddTank(new CDigitank());
	m_apTeams[1]->AddTank(new CDigitank());

	m_apTeams[0]->m_ahTanks[0]->SetOrigin(Vector(0, 0, 30));
	m_apTeams[0]->m_ahTanks[1]->SetOrigin(Vector(5, 0, 35));
	m_apTeams[0]->m_ahTanks[2]->SetOrigin(Vector(-5, 0, 35));
	m_apTeams[1]->m_ahTanks[0]->SetOrigin(Vector(0, 0, -30));
	m_apTeams[1]->m_ahTanks[1]->SetOrigin(Vector(5, 0, -35));
	m_apTeams[1]->m_ahTanks[2]->SetOrigin(Vector(-5, 0, -35));

	m_apTeams[0]->m_ahTanks[0]->SetAngles(EAngle(0, -90, 0));
	m_apTeams[0]->m_ahTanks[1]->SetAngles(EAngle(0, -90, 0));
	m_apTeams[0]->m_ahTanks[2]->SetAngles(EAngle(0, -90, 0));
	m_apTeams[1]->m_ahTanks[0]->SetAngles(EAngle(0, 90, 0));
	m_apTeams[1]->m_ahTanks[1]->SetAngles(EAngle(0, 90, 0));
	m_apTeams[1]->m_ahTanks[2]->SetAngles(EAngle(0, 90, 0));

	m_apTeams[0]->m_ahTanks[0]->GiveBonusPoints(2);
	m_apTeams[0]->m_ahTanks[1]->GiveBonusPoints(1);
	m_apTeams[0]->m_ahTanks[2]->GiveBonusPoints(1);
	m_apTeams[1]->m_ahTanks[0]->GiveBonusPoints(2);
	m_apTeams[1]->m_ahTanks[1]->GiveBonusPoints(1);
	m_apTeams[1]->m_ahTanks[2]->GiveBonusPoints(1);

	CPowerup* pPowerup = new CPowerup();
	pPowerup->SetOrigin(Vector(10, 0, 10));
	pPowerup = new CPowerup();
	pPowerup->SetOrigin(Vector(-10, 0, -10));

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

void CDigitanksGame::SetDesiredMove()
{
	if (!GetCurrentTank())
		return;

	GetCurrentTank()->SetDesiredMove();
}

void CDigitanksGame::SetDesiredTurn()
{
	if (!GetCurrentTank())
		return;

	GetCurrentTank()->SetDesiredTurn();
}

void CDigitanksGame::SetDesiredAim()
{
	if (!GetCurrentTank())
		return;

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

	if (m_pListener)
		m_pListener->NewCurrentTank();
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

	if (m_iCurrentTeam > GetCurrentTeam()->GetNumTanks())
		return NULL;

	return GetCurrentTeam()->GetTank(m_iCurrentTank);
}
