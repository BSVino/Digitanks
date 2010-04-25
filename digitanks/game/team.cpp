#include "team.h"

CTeam::~CTeam()
{
	for (size_t i = 0; i < m_apTanks.size(); i++)
		delete m_apTanks[i];
}

void CTeam::MoveTanks()
{
	for (size_t i = 0; i < m_apTanks.size(); i++)
		m_apTanks[i]->Move();
}
