#include "digitanksteam.h"

#include <sstream>

#include <maths.h>
#include <mtrand.h>

#include <ui/digitankswindow.h>
#include <network/network.h>
#include <ui/instructor.h>

#include "digitank.h"
#include "structure.h"
#include "loader.h"
#include "collector.h"

NETVAR_TABLE_BEGIN(CDigitanksTeam);
	NETVAR_DEFINE(size_t, m_iScore);
NETVAR_TABLE_END();

CDigitanksTeam::CDigitanksTeam()
{
	m_iBuildPosition = 0;

	m_bLKV = false;

	m_iCurrentUpdateX = m_iCurrentUpdateY = -1;
	memset(&m_abUpdates[0][0], 0, sizeof(m_abUpdates));
	m_iUpdateDownloaded = 0;

	m_bCanBuildBuffers = m_bCanBuildPSUs = m_bCanBuildInfantryLoaders = m_bCanBuildTankLoaders = m_bCanBuildArtilleryLoaders = false;
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

void CDigitanksTeam::ClientUpdate(int iClient)
{
	BaseClass::ClientUpdate(iClient);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.CreateExtraData(sizeof(m_abUpdates));
	memcpy(p.m_pExtraData, &m_abUpdates[0][0], sizeof(m_abUpdates));

	CNetwork::CallFunctionParameters(iClient, "TeamUpdatesData", &p);
}

void CDigitanksTeam::ClientEnterGame()
{
	CalculateVisibility();
}

void CDigitanksTeam::TeamUpdatesData(CNetworkParameters* p)
{
	memcpy(&m_abUpdates[0][0], p->m_pExtraData, sizeof(m_abUpdates));
}

CSelectable* CDigitanksTeam::GetPrimarySelection()
{
	if (m_aiCurrentSelection.size() == 0)
		return NULL;

	CBaseEntity* pEntity = CBaseEntity::GetEntity(m_aiCurrentSelection[0]);

	if (!pEntity)
		return NULL;

	return dynamic_cast<CSelectable*>(pEntity);
}

CDigitank* CDigitanksTeam::GetPrimarySelectionTank()
{
	return dynamic_cast<CDigitank*>(GetPrimarySelection());
}

CStructure* CDigitanksTeam::GetPrimarySelectionStructure()
{
	return dynamic_cast<CStructure*>(GetPrimarySelection());
}

size_t CDigitanksTeam::GetPrimarySelectionId()
{
	if (m_aiCurrentSelection.size() == 0)
		return -1;

	return m_aiCurrentSelection[0];
}

void CDigitanksTeam::SetPrimarySelection(const CSelectable* pCurrent)
{
	m_aiCurrentSelection.clear();

	if (!pCurrent)
	{
		DigitanksGame()->SetControlMode(MODE_NONE);
		return;
	}

	if (pCurrent->GetVisibility() == 0)
		return;

	m_aiCurrentSelection.push_back(pCurrent->GetHandle());

	if (GetPrimarySelection())
	{
		GetPrimarySelection()->OnCurrentSelection();

		CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_SELECTION);
	}
}

bool CDigitanksTeam::IsPrimarySelection(const CSelectable* pEntity)
{
	return GetPrimarySelection() == pEntity;
}

void CDigitanksTeam::AddToSelection(const CSelectable* pEntity)
{
	if (!pEntity)
		return;

	if (GetPrimarySelection())
	{
		// Don't pick teamless entities up in this selection if there's a team up front.
		if (pEntity->GetTeam() && !GetPrimarySelection()->GetTeam())
		{
			SetPrimarySelection(pEntity);
			return;
		}

		// Prioritize our own team over enemy teams.
		if (pEntity->GetTeam() && DigitanksGame()->IsTeamControlledByMe(pEntity->GetTeam()) && !DigitanksGame()->IsTeamControlledByMe(GetPrimarySelection()->GetTeam()))
		{
			SetPrimarySelection(pEntity);
			return;
		}

		// Only pick up one team at a time.
		if (pEntity->GetTeam() != GetPrimarySelection()->GetTeam())
			return;
	}

	m_aiCurrentSelection.push_back(pEntity->GetHandle());
}

bool CDigitanksTeam::IsSelected(const CSelectable* pEntity)
{
	if (!pEntity)
		return false;

	for (size_t i = 0; i < m_aiCurrentSelection.size(); i++)
	{
		if (pEntity->GetHandle() == m_aiCurrentSelection[i])
			return true;
	}

	return false;
}

void CDigitanksTeam::StartTurn()
{
	m_iTotalFleetPoints = m_iUsedFleetPoints = 0;

	CalculateVisibility();

	CountProducers();

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

	std::vector<CDigitanksEntity*> apMembers;

	// Form a list so that members added during another member's startturn aren't considered this turn.
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahMembers[i].GetPointer());
		if (pEntity)
			apMembers.push_back(pEntity);
	}

	// Construct and produce and update and shit.
	for (size_t i = 0; i < apMembers.size(); i++)
	{
		if (apMembers[i] == NULL)
			continue;

		apMembers[i]->StartTurn();
	}

	// Recount producers in case updates have been installed.
	CountProducers();
	CountFleetPoints();
	CountBandwidth();

	if (GetUpdateDownloading())
	{
		m_iUpdateDownloaded += m_iBandwidth;
		if (GetUpdateDownloaded() >= GetUpdateSize())
		{
			std::wstringstream s;
			s << L"'" << GetUpdateDownloading()->GetName() << L"' finished downloading.";
			DigitanksGame()->AppendTurnInfo(s.str().c_str());

			DownloadComplete();
		}
		else
		{
			std::wstringstream s;
			s << L"Downloading '" << GetUpdateDownloading()->GetName() << L"' (" << GetTurnsToDownload() << L" turns left)";
			DigitanksGame()->AppendTurnInfo(s.str().c_str());
		}
	}

	CountScore();
}

void CDigitanksTeam::EndTurn()
{
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahMembers[i].GetPointer());
		if (pEntity)
		{
			pEntity->EndTurn();
			continue;
		}
	}
}

void CDigitanksTeam::CountProducers()
{
	m_iProduction = 0;
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

		CCPU* pCPU = dynamic_cast<CCPU*>(m_ahMembers[i].GetPointer());
		if (pCPU && pCPU->IsProducing())
			AddProducer();

		CStructure* pStructure = dynamic_cast<CStructure*>(m_ahMembers[i].GetPointer());
		if (pStructure && pStructure->IsConstructing())
			AddProducer();

		if (pStructure && (pStructure->IsInstalling() || pStructure->IsUpgrading()))
			AddProducer();

		if (pStructure && pStructure->Power())
			AddProduction(pStructure->Power());

		CCollector* pCollector = dynamic_cast<CCollector*>(m_ahMembers[i].GetPointer());
		if (pCollector && !pCollector->IsConstructing() && !pCollector->IsUpgrading() && pCollector->GetSupplier())
			AddProduction((size_t)(pCollector->GetProduction() * pCollector->GetSupplier()->GetChildEfficiency()));
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

void CDigitanksTeam::CountFleetPoints()
{
	m_iTotalFleetPoints = 0;
	m_iUsedFleetPoints = 0;

	// Find and count fleet points
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahMembers[i].GetPointer());
		if (!pEntity)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);

		if (pTank)
			m_iUsedFleetPoints += pTank->FleetPoints();

		if (pStructure && !pStructure->IsConstructing())
			m_iTotalFleetPoints += pStructure->FleetPoints();

		CLoader* pLoader = dynamic_cast<CLoader*>(pStructure);
		if (pLoader && pLoader->IsProducing())
			m_iUsedFleetPoints += pLoader->GetFleetPointsRequired();
	}
}

void CDigitanksTeam::CountScore()
{
	m_iScore = 0;

	// Find and count fleet points
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahMembers[i].GetPointer());
		if (!pEntity)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);

		if (pTank)
			m_iScore += CLoader::GetUnitProductionCost(pTank->GetBuildUnit());

		if (pStructure && !pStructure->IsConstructing())
		{
			m_iScore += pStructure->ConstructionCost();
			m_iScore += pStructure->GetUpdatesScore();
		}
	}
}

void CDigitanksTeam::CountBandwidth()
{
	m_iBandwidth = 0;

	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahMembers[i].GetPointer());
		if (!pEntity)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);

		if (pStructure)
			m_iBandwidth += pStructure->Bandwidth();
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

void CDigitanksTeam::CalculateVisibility()
{
	m_aflVisibilities.clear();
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

		m_aflVisibilities[pEntity->GetHandle()] = GetVisibilityAtPoint(pEntity->GetOrigin());
	}
}

float CDigitanksTeam::GetEntityVisibility(size_t iHandle)
{
	if (m_aflVisibilities.find(iHandle) == m_aflVisibilities.end())
		return 0;

	return m_aflVisibilities[iHandle];
}

float CDigitanksTeam::GetVisibilityAtPoint(Vector vecPoint)
{
	if (!DigitanksGame()->ShouldRenderFogOfWar())
		return 1;

	float flFinalVisibility = 0;

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

		float flVisibility = RemapValClamped((pTeammate->GetOrigin() - vecPoint).Length(), pTeammate->VisibleRange(), pTeammate->VisibleRange()+DigitanksGame()->FogPenetrationDistance(), 1, 0);

		// Use the brightest visibility
		if (flVisibility > flFinalVisibility)
			flFinalVisibility = flVisibility;
	}

	return flFinalVisibility;
}

void CDigitanksTeam::DownloadUpdate(int iX, int iY, bool bCheckValid)
{
	if (m_iCurrentUpdateX == iX && m_iCurrentUpdateY == iY)
		return;

	if (bCheckValid && !CanDownloadUpdate(iX, iY))
		return;

	m_iCurrentUpdateX = iX;
	m_iCurrentUpdateY = iY;
	m_iUpdateDownloaded = 0;
}

size_t CDigitanksTeam::GetUpdateSize()
{
	if (m_iCurrentUpdateX < 0 || m_iCurrentUpdateY < 0)
		return 0;

	if (!DigitanksGame()->GetUpdateGrid())
		return 0;

	return DigitanksGame()->GetUpdateGrid()->m_aUpdates[m_iCurrentUpdateX][m_iCurrentUpdateY].m_iSize;
}

void CDigitanksTeam::DownloadComplete(bool bInformMembers)
{
	if (m_iCurrentUpdateX < 0 || m_iCurrentUpdateY < 0)
		return;

	if (!DigitanksGame()->GetUpdateGrid())
		return;

	CUpdateItem* pItem = &DigitanksGame()->GetUpdateGrid()->m_aUpdates[m_iCurrentUpdateX][m_iCurrentUpdateY];

	m_abUpdates[m_iCurrentUpdateX][m_iCurrentUpdateY] = true;

	if (bInformMembers)
	{
		for (size_t i = 0; i < GetNumMembers(); i++)
		{
			CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(GetMember(i));
			if (!pEntity)
				continue;

			pEntity->DownloadComplete(pItem);
		}
	}

	if (pItem->m_eUpdateClass == UPDATECLASS_STRUCTURE)
	{
		switch (pItem->m_eStructure)
		{
		case STRUCTURE_BUFFER:
			m_bCanBuildBuffers = true;
			break;

		case STRUCTURE_PSU:
			m_bCanBuildPSUs = true;
			break;

		case STRUCTURE_INFANTRYLOADER:
			m_bCanBuildInfantryLoaders = true;
			break;

		case STRUCTURE_TANKLOADER:
			m_bCanBuildTankLoaders = true;
			break;

		case STRUCTURE_ARTILLERYLOADER:
			m_bCanBuildArtilleryLoaders = true;
			break;
		}
	}

	m_iCurrentUpdateX = m_iCurrentUpdateY = -1;

	m_iUpdateDownloaded = 0;
}

bool CDigitanksTeam::HasDownloadedUpdate(int iX, int iY)
{
	return m_abUpdates[iX][iY];
}

bool CDigitanksTeam::CanDownloadUpdate(int iX, int iY)
{
	if (HasDownloadedUpdate(iX, iY))
		return false;

	CUpdateGrid* pGrid = DigitanksGame()->GetUpdateGrid();

	if (!pGrid)
		return false;

	if (pGrid->m_aUpdates[iX][iY].m_eUpdateClass == UPDATECLASS_EMPTY)
		return false;

	if (iX > 0 && m_abUpdates[iX-1][iY])
		return true;

	if (iY > 0 && m_abUpdates[iX][iY-1])
		return true;

	if (iX < UPDATE_GRID_SIZE-1 && m_abUpdates[iX+1][iY])
		return true;

	if (iY < UPDATE_GRID_SIZE-1 && m_abUpdates[iX][iY+1])
		return true;

	return false;
}

bool CDigitanksTeam::IsDownloading(int iX, int iY)
{
	return m_iCurrentUpdateX == iX && m_iCurrentUpdateY == iY;
}

CUpdateItem* CDigitanksTeam::GetUpdateDownloading()
{
	if (m_iCurrentUpdateX < 0 || m_iCurrentUpdateY < 0)
		return NULL;

	return &DigitanksGame()->GetUpdateGrid()->m_aUpdates[m_iCurrentUpdateX][m_iCurrentUpdateY];
}

size_t CDigitanksTeam::GetTurnsToDownload()
{
	if (GetBandwidth() == 0)
		return 0;

	return (size_t)((GetUpdateSize()-m_iUpdateDownloaded)/GetBandwidth())+1;
}

bool CDigitanksTeam::CanBuildMiniBuffers()
{
	if (!DigitanksGame()->CanBuildMiniBuffers())
		return false;

	return true;
}

bool CDigitanksTeam::CanBuildBuffers()
{
	if (!DigitanksGame()->CanBuildBuffers())
		return false;

	if (!DigitanksGame()->GetUpdateGrid())
		return true;

	return m_bCanBuildBuffers;
}

bool CDigitanksTeam::CanBuildBatteries()
{
	if (!DigitanksGame()->CanBuildBatteries())
		return false;

	return true;
}

bool CDigitanksTeam::CanBuildPSUs()
{
	if (!DigitanksGame()->CanBuildPSUs())
		return false;

	if (!DigitanksGame()->GetUpdateGrid())
		return true;

	return m_bCanBuildPSUs;
}

bool CDigitanksTeam::CanBuildLoaders()
{
	if (CanBuildInfantryLoaders())
		return true;

	if (CanBuildTankLoaders())
		return true;

	if (CanBuildArtilleryLoaders())
		return true;

	return false;
}

bool CDigitanksTeam::CanBuildInfantryLoaders()
{
	if (!DigitanksGame()->CanBuildInfantryLoaders())
		return false;

	if (!DigitanksGame()->GetUpdateGrid())
		return true;

	return true; //m_bCanBuildInfantryLoaders;
}

bool CDigitanksTeam::CanBuildTankLoaders()
{
	if (!DigitanksGame()->CanBuildTankLoaders())
		return false;

	if (!DigitanksGame()->GetUpdateGrid())
		return true;

	return m_bCanBuildTankLoaders;
}

bool CDigitanksTeam::CanBuildArtilleryLoaders()
{
	if (!DigitanksGame()->CanBuildArtilleryLoaders())
		return false;

	if (!DigitanksGame()->GetUpdateGrid())
		return true;

	return m_bCanBuildArtilleryLoaders;
}
