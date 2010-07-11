#include "digitanksteam.h"

#include <maths.h>
#include <network/network.h>

#include "digitank.h"
#include "structure.h"
#include "loader.h"
#include "collector.h"

REGISTER_ENTITY(CDigitanksTeam);

CDigitanksTeam::CDigitanksTeam()
{
	m_iBuildPosition = 0;
}

CDigitanksTeam::~CDigitanksTeam()
{
}

void CDigitanksTeam::OnAddEntity(CBaseEntity* pEntity)
{
	CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
	if (pTank)
		m_ahTanks.push_back(pTank);

	m_aflVisibilities[pEntity->GetHandle()] = 1;

	CCPU* pCPU = dynamic_cast<CCPU*>(pEntity);
	if (m_hPrimaryCPU == NULL && pCPU)
		m_hPrimaryCPU = pCPU;
}

void CDigitanksTeam::StartTurn()
{
	m_iProduction = 0;
	m_aflVisibilities.clear();
	m_iLoadersProducing = 0;

	// Find and count producers and accumulate production points
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahMembers[i].GetPointer());
		if (!pEntity)
			continue;

		CLoader* pLoader = dynamic_cast<CLoader*>(m_ahMembers[i].GetPointer());
		if (pLoader && pLoader->IsProducing())
			AddProducer();

		CStructure* pStructure = dynamic_cast<CStructure*>(m_ahMembers[i].GetPointer());
		if (pStructure && pStructure->IsConstructing())
			AddProducer();

		CCollector* pCollector = dynamic_cast<CCollector*>(m_ahMembers[i].GetPointer());
		if (pCollector && !pCollector->IsConstructing())
			AddProduction((size_t)(pCollector->GetResource()->GetProduction() * pCollector->GetSupplier()->GetChildEfficiency()));

		CCPU* pCPU = dynamic_cast<CCPU*>(m_ahMembers[i].GetPointer());
		if (pCPU && !pCPU->IsConstructing())
			AddProduction(4);
	}

	// Tell CPU's to calculate data flow before StartTurn logic, which updates tendrils and data strengths.
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahMembers[i].GetPointer());
		if (!pEntity)
			continue;

		CCPU* pCPU = dynamic_cast<CCPU*>(m_ahMembers[i].GetPointer());
		if (pCPU && !pCPU->IsConstructing())
			pCPU->CalculateDataFlow();
	}

	// Construct and produce and update and shit.
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahMembers[i].GetPointer());
		if (pEntity)
		{
			pEntity->StartTurn();
			continue;
		}
	}

	// For every entity in the game, calculate the visibility to this team
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		if (pEntity->GetTeam() == ((CTeam*)this))
		{
			m_aflVisibilities[pEntity->GetHandle()] = 1;
			continue;
		}

		m_aflVisibilities[pEntity->GetHandle()] = 0;

		// For every entity on this team, see what the visibility is
		for (size_t j = 0; j < m_ahMembers.size(); j++)
		{
			if (m_ahMembers[j] == NULL)
				continue;

			CDigitanksEntity* pTeammate = dynamic_cast<CDigitanksEntity*>(m_ahMembers[j].GetPointer());
			if (!pTeammate)
				continue;

			if (pTeammate->VisibleRange() == 0)
				continue;

			float flVisibility = RemapValClamped((pTeammate->GetOrigin() - pEntity->GetOrigin()).Length(), pTeammate->VisibleRange(), pTeammate->VisibleRange()+10, 1, 0);

			// Use the brightest visibility
			if (flVisibility > m_aflVisibilities[pEntity->GetHandle()])
				m_aflVisibilities[pEntity->GetHandle()] = flVisibility;
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

void CDigitanksTeam::AddProduction(size_t iProduction)
{
	m_iProduction += iProduction;
}

float CDigitanksTeam::GetProductionPerLoader()
{
	if (m_iLoadersProducing == 0)
		return (float)m_iProduction;

	return (float)m_iProduction / (float)m_iLoadersProducing;
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

float CDigitanksTeam::GetEntityVisibility(size_t iHandle)
{
	if (m_aflVisibilities.find(iHandle) == m_aflVisibilities.end())
		return 0;

	return m_aflVisibilities[iHandle];
}
