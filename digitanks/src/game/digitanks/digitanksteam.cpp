#include "digitanksteam.h"

#include <network/network.h>

#include "digitank.h"
#include "structure.h"

REGISTER_ENTITY(CDigitanksTeam);

CDigitanksTeam::CDigitanksTeam()
{
}

CDigitanksTeam::~CDigitanksTeam()
{
}

void CDigitanksTeam::OnAddEntity(CBaseEntity* pEntity)
{
	CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
	if (pTank)
		m_ahTanks.push_back(pTank);
}

void CDigitanksTeam::PreStartTurn()
{
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(m_ahMembers[i].GetPointer());
		if (pTank)
		{
			pTank->PreStartTurn();
			continue;
		}

		CStructure* pStructure = dynamic_cast<CStructure*>(m_ahMembers[i].GetPointer());
		if (pStructure)
		{
			pStructure->PreStartTurn();
			continue;
		}
	}
}

void CDigitanksTeam::StartTurn()
{
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(m_ahMembers[i].GetPointer());
		if (pTank)
		{
			pTank->StartTurn();
			continue;
		}

		CStructure* pStructure = dynamic_cast<CStructure*>(m_ahMembers[i].GetPointer());
		if (pStructure)
		{
			pStructure->StartTurn();
			continue;
		}
	}
}

void CDigitanksTeam::PostStartTurn()
{
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(m_ahMembers[i].GetPointer());
		if (pTank)
		{
			pTank->PostStartTurn();
			continue;
		}

		CStructure* pStructure = dynamic_cast<CStructure*>(m_ahMembers[i].GetPointer());
		if (pStructure)
		{
			pStructure->PostStartTurn();
			continue;
		}
	}
}

void CDigitanksTeam::MoveTanks()
{
	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if (m_ahTanks[i] != NULL)
			m_ahTanks[i]->Move();
	}
}

void CDigitanksTeam::FireTanks()
{
	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if (m_ahTanks[i] != NULL)
			m_ahTanks[i]->Fire();
	}
}

void CDigitanksTeam::OnDeleted(CBaseEntity* pEntity)
{
	BaseClass::OnDeleted(pEntity);

	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if ((CBaseEntity*)m_ahTanks[i] == pEntity)
			m_ahTanks.erase(m_ahTanks.begin()+i);
	}
}

size_t CDigitanksTeam::GetNumTanksAlive()
{
	size_t iTanksAlive = 0;
	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if (m_ahTanks[i]->IsAlive())
			iTanksAlive++;
	}

	return iTanksAlive;
}
