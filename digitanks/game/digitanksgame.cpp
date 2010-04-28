#include "digitanksgame.h"

#include <assert.h>

CDigitanksGame::CDigitanksGame()
{
	m_iCurrentTeam = 0;
	m_iCurrentTank = 0;

	m_pListener = NULL;
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

	m_apTeams.push_back(new CTeam());
	m_apTeams.push_back(new CTeam());

	m_apTeams[0]->m_clrTeam = Color(255, 0, 0);
	m_apTeams[1]->m_clrTeam = Color(0, 0, 255);

	m_apTeams[0]->m_ahTanks.push_back(new CDigitank());
	m_apTeams[1]->m_ahTanks.push_back(new CDigitank());

	m_apTeams[0]->m_ahTanks[0]->SetOrigin(Vector(0, 0, 30));
	m_apTeams[1]->m_ahTanks[0]->SetOrigin(Vector(0, 0, -30));

	m_apTeams[0]->m_ahTanks[0]->SetAngles(EAngle(0, -90, 0));
	m_apTeams[1]->m_ahTanks[0]->SetAngles(EAngle(0, 90, 0));

	m_apTeams[0]->m_ahTanks[0]->SetTarget(m_apTeams[1]->m_ahTanks[0]);
	m_apTeams[1]->m_ahTanks[0]->SetTarget(m_apTeams[0]->m_ahTanks[0]);

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

void CDigitanksGame::SetDesiredMove()
{
	if (!GetCurrentTank())
		return;

	GetCurrentTank()->SetDesiredMove();

	NextTank();
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

void CDigitanksGame::Turn()
{
	GetCurrentTeam()->MoveTanks();
	GetCurrentTeam()->FireTanks();

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
