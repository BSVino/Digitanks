#include "team.h"

CTeam::~CTeam()
{
	for (size_t i = 0; i < m_apTanks.size(); i++)
		delete m_apTanks[i];
}
