#include "digitanksgame.h"

CDigitanksGame::~CDigitanksGame()
{
	for (size_t i = 0; i < m_apTeams.size(); i++)
		delete m_apTeams[i];
}

void CDigitanksGame::SetupDefaultGame()
{
	m_apTeams.push_back(new CTeam());
	m_apTeams.push_back(new CTeam());

	m_apTeams[0]->m_clrTeam = Color(255, 0, 0);
	m_apTeams[1]->m_clrTeam = Color(0, 0, 255);

	m_apTeams[0]->m_apTanks.push_back(new CDigitank());
	m_apTeams[1]->m_apTanks.push_back(new CDigitank());

	m_apTeams[0]->m_apTanks[0]->SetOrigin(Vector(0, 0, 50));
	m_apTeams[1]->m_apTanks[0]->SetOrigin(Vector(0, 0, -50));
}
