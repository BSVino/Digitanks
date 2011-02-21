#include "digitanksteam.h"

#include <maths.h>
#include <mtrand.h>

#include <ui/digitankswindow.h>
#include <network/network.h>
#include <ui/instructor.h>
#include <ui/hud.h>

#include "units/digitank.h"
#include "structures/structure.h"
#include "structures/loader.h"
#include "structures/collector.h"
#include "units/mobilecpu.h"
#include "wreckage.h"

REGISTER_ENTITY(CDigitanksTeam);

NETVAR_TABLE_BEGIN(CDigitanksTeam);
	NETVAR_DEFINE_CALLBACK(float, m_flPowerPerTurn, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(float, m_flPower, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(size_t, m_iTotalFleetPoints, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(size_t, m_iUsedFleetPoints, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(size_t, m_iScore, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bLost, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(int, m_iCurrentUpdateX, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(int, m_iCurrentUpdateY, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(float, m_flUpdateDownloaded, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(float, m_flMegabytes, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(float, m_flBandwidth, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bCanBuildBuffers, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bCanBuildPSUs, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bCanBuildInfantryLoaders, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bCanBuildTankLoaders, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bCanBuildArtilleryLoaders, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(bool, m_bIncludeInScoreboard);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitanksTeam);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CDigitank>, m_ahTanks);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, size_t, m_aiCurrentSelection);
	//std::map<size_t, float>		m_aflVisibilities;	// Automatically generated
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flPowerPerTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTotalFleetPoints);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iUsedFleetPoints);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iScore);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bLost);

	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CCPU>, m_hPrimaryCPU);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecExplore);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bLKV);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecLKV);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iFleetPointAttackQuota);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYARRAY, CEntityHandle<CDigitank>, m_ahAttackTeam);

	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_iCurrentUpdateX);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_iCurrentUpdateY);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYARRAY, bool, m_abUpdates);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flUpdateDownloaded);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flMegabytes);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBandwidth);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCanBuildBuffers);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCanBuildPSUs);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCanBuildInfantryLoaders);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCanBuildTankLoaders);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCanBuildArtilleryLoaders);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bUseArtilleryAI);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, losecondition_t, m_eLoseCondition);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bIncludeInScoreboard);
SAVEDATA_TABLE_END();

CDigitanksTeam::CDigitanksTeam()
{
	m_bLKV = false;
	m_bCanUpgrade = true;

	memset(&m_abUpdates[0][0], 0, sizeof(m_abUpdates));
}

CDigitanksTeam::~CDigitanksTeam()
{
}

void CDigitanksTeam::Spawn()
{
	BaseClass::Spawn();

	m_bLost = false;

	m_flPower = 0;

	m_flMegabytes = 0;
	m_flUpdateDownloaded = 0;
	m_iCurrentUpdateX = m_iCurrentUpdateY = -1;
	m_bCanBuildBuffers = m_bCanBuildPSUs = m_bCanBuildInfantryLoaders = m_bCanBuildTankLoaders = m_bCanBuildArtilleryLoaders = false;

	m_bUseArtilleryAI = false;
	m_eLoseCondition = LOSE_NOTANKS;
	m_bIncludeInScoreboard = true;

	m_iFleetPointAttackQuota = ~0;
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

	DigitanksWindow()->GetHUD()->OnAddEntityToTeam(this, pEntity);
}

void CDigitanksTeam::OnRemoveEntity(CBaseEntity* pEntity)
{
	DigitanksWindow()->GetHUD()->OnRemoveEntityFromTeam(this, pEntity);
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
	BaseClass::ClientEnterGame();

	CalculateVisibility();
}

void CDigitanksTeam::TeamUpdatesData(CNetworkParameters* p)
{
	memcpy(&m_abUpdates[0][0], p->m_pExtraData, sizeof(m_abUpdates));
}

CSelectable* CDigitanksTeam::GetPrimarySelection()
{
	if (DigitanksGame()->GetCurrentTeam() != this)
		return NULL;

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
	if (DigitanksGame()->GetCurrentTeam() != this)
		return -1;

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

		DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_SELECTION);
		DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_ARTILLERY_SELECT, true);

		if (dynamic_cast<CMobileCPU*>(GetPrimarySelection()))
			DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_STRATEGY_SELECT, true);
	}
}

bool CDigitanksTeam::IsPrimarySelection(const CSelectable* pEntity)
{
	if (!DigitanksGame())
		return false;

	if (DigitanksGame()->GetCurrentTeam() != this)
		return false;

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

void CDigitanksTeam::StartNewRound()
{
	m_bLost = false;
}

void CDigitanksTeam::StartTurn()
{
	m_iTotalFleetPoints = m_iUsedFleetPoints = 0;

	for (size_t i = 0; i < Game()->GetNumTeams(); i++)
	{
		if (Game()->GetTeam(i))
			DigitanksGame()->GetDigitanksTeam(i)->CalculateVisibility();
	}

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

	eastl::vector<CDigitanksEntity*> apMembers;

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

	m_flMegabytes += m_flBandwidth;

	if (GetUpdateDownloading())
	{
		if (CNetwork::IsHost())
		{
			m_flUpdateDownloaded += m_flMegabytes;
			m_flMegabytes = 0;
		}

		if (GetUpdateDownloaded() >= GetUpdateSize())
		{
			// Return the excess.
			m_flMegabytes = GetUpdateDownloaded() - GetUpdateSize();

			DigitanksGame()->AppendTurnInfo(L"'" + GetUpdateDownloading()->GetName() + L"' finished downloading.");

			DownloadComplete();
		}
		else
		{
			eastl::string16 s;
			s.sprintf((L"Downloading '" + GetUpdateDownloading()->GetName() + L"' (%d turns left)").c_str(), GetTurnsToDownload());
			DigitanksGame()->AppendTurnInfo(s);
		}
	}
	else if (DigitanksGame()->GetUpdateGrid())
	{
		if (DigitanksGame()->GetTurn() > 1 && !DigitanksGame()->GetCurrentTeam()->GetUpdateDownloading())
			DigitanksGame()->AddActionItem(NULL, ACTIONTYPE_DOWNLOADUPDATES);
	}

	CountProducers();
	CountFleetPoints();
	CountBandwidth();
	CountScore();

	m_flPower += m_flPowerPerTurn;

	DigitanksWindow()->GetHUD()->Layout();
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
	if (!CNetwork::IsHost())
		return;

	m_flPowerPerTurn = 0;

	// Find and count producers and accumulate production points
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahMembers[i].GetPointer());
		if (!pEntity)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(m_ahMembers[i].GetPointer());
		if (pStructure && pStructure->Power())
			AddPowerPerTurn(pStructure->Power());

		CCollector* pCollector = dynamic_cast<CCollector*>(m_ahMembers[i].GetPointer());
		if (pCollector && !pCollector->IsConstructing() && !pCollector->IsUpgrading() && pCollector->GetSupplier())
			AddPowerPerTurn(pCollector->GetPowerProduced() * pCollector->GetSupplier()->GetChildEfficiency());
	}
}

void CDigitanksTeam::AddPowerPerTurn(float flPower)
{
	if (!CNetwork::IsHost())
		return;

	m_flPowerPerTurn += flPower;
}

void CDigitanksTeam::ConsumePower(float flPower)
{
	assert(m_flPower >= flPower);

	if (flPower > m_flPower)
		m_flPower = 0;
	else
		m_flPower -= flPower;
}

void CDigitanksTeam::CountFleetPoints()
{
	if (!CNetwork::IsHost())
		return;

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
	if (!CNetwork::IsHost())
		return;

	if (!m_bIncludeInScoreboard)
		return;

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
			m_iScore += (size_t)DigitanksGame()->GetConstructionCost(pTank->GetUnitType());

		if (pStructure && !pStructure->IsConstructing())
		{
			m_iScore += (size_t)pStructure->ConstructionCost();
		}
	}
}

void CDigitanksTeam::YouLoseSirGoodDay()
{
	m_bLost = true;

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() == this)
		DigitanksGame()->GetListener()->GameOver(false);
}

void CDigitanksTeam::CountBandwidth()
{
	if (!CNetwork::IsHost())
		return;

	m_flBandwidth = 0;

	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahMembers[i].GetPointer());
		if (!pEntity)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);

		if (pStructure)
			m_flBandwidth += pStructure->Bandwidth();
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
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
		CalculateEntityVisibility(CBaseEntity::GetEntity(i));
}

void CDigitanksTeam::CalculateEntityVisibility(CBaseEntity* pEntity)
{
	if (!pEntity)
		return;

	if (pEntity->GetTeam() == ((CTeam*)this))
	{
		m_aflVisibilities[pEntity->GetHandle()] = 1;
		return;
	}

	CWreckage* pWreckage = dynamic_cast<CWreckage*>(pEntity);
	if (pWreckage)
	{
		if (pWreckage->GetOldTeam() == this && GameServer()->GetGameTime() - pWreckage->GetSpawnTime() < 10)
		{
			// We can continue to see our wreckage even after it dies, just in case we die in a remote place.
			m_aflVisibilities[pEntity->GetHandle()] = 1;
			return;
		}
	}

	Vector vecOrigin = pEntity->GetOrigin();
	CDigitank* pDigitank = dynamic_cast<CDigitank*>(pEntity);
	if (pDigitank)
		vecOrigin = pDigitank->GetRealOrigin();

	bool bCloak = false;
	if (pDigitank && pDigitank->IsCloaked())
		bCloak = true;

	float flVisibility = GetVisibilityAtPoint(vecOrigin, bCloak);

	m_aflVisibilities[pEntity->GetHandle()] = flVisibility;
}

float CDigitanksTeam::GetEntityVisibility(size_t iHandle)
{
	if (m_aflVisibilities.find(iHandle) == m_aflVisibilities.end())
		return 0;

	return m_aflVisibilities[iHandle];
}

float CDigitanksTeam::GetVisibilityAtPoint(Vector vecPoint, bool bCloak)
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

		Vector vecOrigin = pTeammate->GetOrigin();
		CDigitank* pDigitank = dynamic_cast<CDigitank*>(pTeammate);
		if (pDigitank)
			vecOrigin = pDigitank->GetRealOrigin();

		float flVisibileRange = pTeammate->VisibleRange();
		if (bCloak)
			flVisibileRange /= 2;

		float flVisibility = RemapValClamped((vecOrigin - vecPoint).Length(), flVisibileRange, flVisibileRange+DigitanksGame()->FogPenetrationDistance(), 1, 0);

		// Use the brightest visibility
		if (flVisibility > flFinalVisibility)
			flFinalVisibility = flVisibility;

		if (flFinalVisibility > 0.99f)
			return 1.0f;
	}

	return flFinalVisibility;
}

void CDigitanksTeam::DownloadUpdate(int iX, int iY, bool bCheckValid)
{
	if (m_iCurrentUpdateX == iX && m_iCurrentUpdateY == iY)
		return;

	if (bCheckValid && !CanDownloadUpdate(iX, iY))
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.i2 = iX;
	p.i3 = iY;
	p.i4 = !!bCheckValid;

	DownloadUpdate(&p);

	CNetwork::CallFunctionParameters(NETWORK_TOEVERYONE, "DownloadUpdate", &p);
}

void CDigitanksTeam::DownloadUpdate(class CNetworkParameters* p)
{
	int iX = p->i2;
	int iY = p->i3;
	bool bCheckValid = !!p->i4;

	if (m_iCurrentUpdateX == iX && m_iCurrentUpdateY == iY)
		return;

	if (bCheckValid && !CanDownloadUpdate(iX, iY))
		return;

	m_iCurrentUpdateX = iX;
	m_iCurrentUpdateY = iY;
	m_flUpdateDownloaded = m_flMegabytes;
	m_flMegabytes = 0;

	DigitanksGame()->HandledActionItem(ACTIONTYPE_DOWNLOADCOMPLETE);
	DigitanksGame()->HandledActionItem(ACTIONTYPE_DOWNLOADUPDATES);
}

float CDigitanksTeam::GetUpdateSize()
{
	if (m_iCurrentUpdateX < 0 || m_iCurrentUpdateY < 0)
		return 0;

	if (!DigitanksGame()->GetUpdateGrid())
		return 0;

	return DigitanksGame()->GetUpdateGrid()->m_aUpdates[m_iCurrentUpdateX][m_iCurrentUpdateY].m_flSize;
}

void CDigitanksTeam::DownloadComplete(bool bInformMembers)
{
	if (m_iCurrentUpdateX < 0 || m_iCurrentUpdateY < 0)
		return;

	if (!DigitanksGame()->GetUpdateGrid())
		return;

	if (!CNetwork::IsHost())
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.i2 = !!bInformMembers;

	DownloadComplete(&p);

	CNetwork::CallFunctionParameters(NETWORK_TOCLIENTS, "DownloadComplete", &p);
}

void CDigitanksTeam::DownloadComplete(class CNetworkParameters* p)
{
	bool bInformMembers = !!p->i2;

	CUpdateItem* pItem = &DigitanksGame()->GetUpdateGrid()->m_aUpdates[m_iCurrentUpdateX][m_iCurrentUpdateY];
	bool* pbTeamUpdate = &m_abUpdates[m_iCurrentUpdateX][m_iCurrentUpdateY];

	if (bInformMembers)
	{
		for (size_t i = 0; i < GetNumMembers(); i++)
		{
			CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(GetMember(i));
			if (!pEntity)
				continue;

			pEntity->DownloadComplete(m_iCurrentUpdateX, m_iCurrentUpdateY);
		}
	}

	m_iCurrentUpdateX = m_iCurrentUpdateY = -1;

	DigitanksGame()->AddActionItem(NULL, ACTIONTYPE_DOWNLOADCOMPLETE);

	if (!CNetwork::IsHost())
		return;

	// Host-only shit from here on out, gets auto-sent to the clients.

	*pbTeamUpdate = true;

	ClientUpdate(GetClient());	// Force my team to receive my m_abUpdates

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

	m_flUpdateDownloaded = 0;
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

	int iTurns = (int)((GetUpdateSize()-m_flUpdateDownloaded)/GetBandwidth())+1;

	if (iTurns < 1)
		return 1;

	return iTurns;
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
