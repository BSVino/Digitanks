#include "digitanksplayer.h"

#include <maths.h>
#include <mtrand.h>

#include <ui/digitankswindow.h>
#include <network/network.h>
#include <network/commands.h>
#include <ui/instructor.h>
#include <ui/hud.h>
#include <glgui/rootpanel.h>
#include <sound/sound.h>

#include "units/digitank.h"
#include "structures/structure.h"
#include "structures/loader.h"
#include "structures/collector.h"
#include "units/mobilecpu.h"
#include "wreckage.h"
#include "ui/digitankswindow.h"
#include "ui/ui.h"

REGISTER_ENTITY(CDigitanksPlayer);

NETVAR_TABLE_BEGIN(CDigitanksPlayer);
	NETVAR_DEFINE_CALLBACK(CEntityHandle<CDigitank>, m_ahTanks, &CDigitanksGame::UpdateTeamMembers);
	NETVAR_DEFINE_CALLBACK(float, m_flPowerPerTurn, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(float, m_flPower, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(size_t, m_iTotalFleetPoints, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(size_t, m_iUsedFleetPoints, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(size_t, m_iScore, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bLost, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(actionitem_t, m_aActionItems, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(CEntityHandle<CCPU>, m_hPrimaryCPU);
	NETVAR_DEFINE_CALLBACK(int, m_iCurrentUpdateX, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(int, m_iCurrentUpdateY, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_abUpdates, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(float, m_flUpdateDownloaded, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(float, m_flMegabytes, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(float, m_flBandwidth, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(tstring, m_sTurnInfo, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bCanBuildBuffers, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bCanBuildPSUs, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bCanBuildInfantryLoaders, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bCanBuildTankLoaders, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bCanBuildArtilleryLoaders, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(losecondition_t, m_eLoseCondition);
	NETVAR_DEFINE(bool, m_bIncludeInScoreboard);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitanksPlayer);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CDigitank>, m_ahTanks);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, size_t, m_aiCurrentSelection);
	SAVEDATA_OMIT(m_aflVisibilities);	// Automatically generated
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flPowerPerTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTotalFleetPoints);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iUsedFleetPoints);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iScore);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bLost);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, actionitem_t, m_aActionItems);

	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CCPU>, m_hPrimaryCPU);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CLoader>, m_hInfantryLoader);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CLoader>, m_hTankLoader);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CLoader>, m_hArtilleryLoader);
	SAVEDATA_OMIT(m_aeBuildPriorities);	// Automatically generated
	SAVEDATA_OMIT(m_iBuildPrioritiesHead);	// Automatically generated
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bCanUpgrade);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecExplore);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bUseArtilleryAI);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bLKV);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecLKV);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iFleetPointAttackQuota);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CDigitank>, m_ahAttackTeam);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iLastScoutBuilt);

	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_iCurrentUpdateX);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_iCurrentUpdateY);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_abUpdates);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flUpdateDownloaded);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flMegabytes);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBandwidth);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, tstring, m_sTurnInfo);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCanBuildBuffers);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCanBuildPSUs);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCanBuildInfantryLoaders);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCanBuildTankLoaders);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCanBuildArtilleryLoaders);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, losecondition_t, m_eLoseCondition);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bIncludeInScoreboard);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CDigitanksPlayer);
INPUTS_TABLE_END();

CDigitanksPlayer::CDigitanksPlayer()
{
	m_bLKV = false;
	m_bCanUpgrade = true;

	for (size_t i = 0; i < UPDATE_GRID_SIZE*UPDATE_GRID_SIZE; i++)
		m_abUpdates[i] = false;

	m_iLastScoutBuilt = 0;

	m_iScore = 0;
	m_bLost = false;

	m_bBoxSelect = false;
}

CDigitanksPlayer::~CDigitanksPlayer()
{
}

void CDigitanksPlayer::Spawn()
{
	BaseClass::Spawn();

	m_bLost = false;

	m_flPower = 0;

	m_flMegabytes = 0;
	m_flBandwidth = 0;
	m_flUpdateDownloaded = 0;
	m_iCurrentUpdateX = m_iCurrentUpdateY = -1;
	m_bCanBuildBuffers = m_bCanBuildPSUs = m_bCanBuildInfantryLoaders = m_bCanBuildTankLoaders = m_bCanBuildArtilleryLoaders = false;

	m_bUseArtilleryAI = false;
	m_eLoseCondition = LOSE_NOTANKS;
	m_bIncludeInScoreboard = true;

	m_iFleetPointAttackQuota = ~0;

	m_hPrimaryCPU = NULL;
}

void CDigitanksPlayer::ClientEnterGame()
{
	BaseClass::ClientEnterGame();

	CalculateVisibility();
}

CSelectable* CDigitanksPlayer::GetPrimarySelection()
{
	if (m_aiCurrentSelection.size() == 0)
		return NULL;

	CBaseEntity* pEntity = CBaseEntity::GetEntity(m_aiCurrentSelection[0]);

	if (!pEntity)
		return NULL;

	return dynamic_cast<CSelectable*>(pEntity);
}

CDigitank* CDigitanksPlayer::GetPrimarySelectionTank()
{
	return dynamic_cast<CDigitank*>(GetPrimarySelection());
}

CStructure* CDigitanksPlayer::GetPrimarySelectionStructure()
{
	return dynamic_cast<CStructure*>(GetPrimarySelection());
}

size_t CDigitanksPlayer::GetPrimarySelectionId()
{
	if (m_aiCurrentSelection.size() == 0)
		return -1;

	return m_aiCurrentSelection[0];
}

void CDigitanksPlayer::SetPrimarySelection(const CSelectable* pCurrent)
{
	m_aiCurrentSelection.clear();

	DigitanksWindow()->SetContextualCommandsOverride(false);

	if (!pCurrent)
	{
		DigitanksGame()->SetControlMode(MODE_NONE);

		if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
			DigitanksWindow()->GetInstructor()->DisplayLesson("artillery-select");

		if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD && !GetPrimaryCPU())
		{
			tstring sLessonName;
			if (DigitanksWindow()->GetInstructor()->GetCurrentLesson())
				sLessonName = DigitanksWindow()->GetInstructor()->GetCurrentLesson()->m_sLessonName;

			if (DigitanksWindow()->GetInstructor()->GetActive() && sLessonName != "strategy-buildbuffer" && sLessonName != "strategy-placebuffer")
				DigitanksWindow()->GetInstructor()->DisplayLesson("strategy-select");
			else
				DigitanksWindow()->GetInstructor()->SetActive(false);
		}

		return;
	}

	if (pCurrent->GetVisibility() == 0)
		return;

	m_aiCurrentSelection.push_back(pCurrent->GetHandle());

	if (GetPrimarySelection())
	{
		GetPrimarySelection()->OnCurrentSelection();

		DigitanksWindow()->GetInstructor()->FinishedLesson("mission-1-selection");

		if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
		{
			if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() == GetPrimarySelection()->GetPlayerOwner())
			{
				DigitanksWindow()->GetInstructor()->FinishedLesson("artillery-select", true);
				DigitanksWindow()->GetInstructor()->FinishedLesson("artillery-onepertank");
			}
			else
				DigitanksWindow()->GetInstructor()->DisplayLesson("artillery-select");
		}

		if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD && !GetPrimaryCPU())
		{
			tstring sLessonName;
			if (DigitanksWindow()->GetInstructor()->GetCurrentLesson())
				sLessonName = DigitanksWindow()->GetInstructor()->GetCurrentLesson()->m_sLessonName;

			if (sLessonName != "strategy-buildbuffer" && sLessonName != "strategy-placebuffer")
			{
				if (dynamic_cast<CMobileCPU*>(GetPrimarySelection()) && DigitanksGame()->GetCurrentLocalDigitanksPlayer() == GetPrimarySelection()->GetPlayerOwner())
					DigitanksWindow()->GetInstructor()->FinishedLesson("strategy-select", true);
				else
					DigitanksWindow()->GetInstructor()->DisplayLesson("strategy-select");
			}
			else
			{
				if (!dynamic_cast<CCPU*>(GetPrimarySelection()))
					DigitanksWindow()->GetInstructor()->SetActive(false);
			}
		}
	}
}

bool CDigitanksPlayer::IsPrimarySelection(const CSelectable* pEntity)
{
	return GetPrimarySelection() == pEntity;
}

void CDigitanksPlayer::AddToSelection(const CSelectable* pEntity)
{
	if (!pEntity)
		return;

	if (GetPrimarySelection())
	{
		// Don't pick teamless entities up in this selection if there's a team up front.
		if (pEntity->GetPlayerOwner() && !GetPrimarySelection()->GetPlayerOwner())
		{
			SetPrimarySelection(pEntity);
			return;
		}

		// Prioritize our own team over enemy teams.
		if (pEntity->GetPlayerOwner() && DigitanksGame()->IsTeamControlledByMe(pEntity->GetPlayerOwner()) && !DigitanksGame()->IsTeamControlledByMe(GetPrimarySelection()->GetPlayerOwner()))
		{
			SetPrimarySelection(pEntity);
			return;
		}

		// Only pick up one team at a time.
		if (pEntity->GetPlayerOwner() != GetPrimarySelection()->GetPlayerOwner())
			return;
	}

	m_aiCurrentSelection.push_back(pEntity->GetHandle());
}

bool CDigitanksPlayer::IsSelected(const CSelectable* pEntity)
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

void CDigitanksPlayer::StartNewRound()
{
	m_bLost = false;
}

void CDigitanksPlayer::StartTurn()
{
	m_iTotalFleetPoints = m_iUsedFleetPoints = 0;

	for (size_t i = 0; i < Game()->GetNumPlayers(); i++)
	{
		if (Game()->GetPlayer(i))
			DigitanksGame()->GetDigitanksPlayer(i)->CalculateVisibility();
	}

	CountProducers();

	// Tell CPU's to calculate data flow before StartTurn logic, which updates tendrils and data strengths.
	for (size_t i = 0; i < m_ahUnits.size(); i++)
	{
		if (!m_ahUnits[i])
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahUnits[i].GetPointer());
		if (!pEntity)
			continue;

		CCPU* pCPU = dynamic_cast<CCPU*>(m_ahUnits[i].GetPointer());
		if (pCPU && !pCPU->IsConstructing())
			pCPU->CalculateDataFlow();
	}

	tvector<CDigitanksEntity*> apMembers;

	// Form a list so that members added during another member's startturn aren't considered this turn.
	for (size_t i = 0; i < m_ahUnits.size(); i++)
	{
		if (!m_ahUnits[i])
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahUnits[i].GetPointer());
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
		if (GameNetwork()->IsHost())
		{
			m_flUpdateDownloaded += m_flMegabytes;
			m_flMegabytes = 0;
		}

		if (GetUpdateDownloaded() >= GetUpdateSize())
		{
			// Return the excess.
			m_flMegabytes = GetUpdateDownloaded() - GetUpdateSize();

			AppendTurnInfo("'" + GetUpdateDownloading()->GetName() + "' finished downloading.");

			DownloadComplete();
		}
		else
		{
			tstring s;
			s.sprintf(("Downloading '" + GetUpdateDownloading()->GetName() + "' (%d turns left)").c_str(), GetTurnsToDownload());
			AppendTurnInfo(s);
		}
	}
	else if (DigitanksGame()->GetUpdateGrid())
	{
		if (DigitanksGame()->GetTurn() > 1 && !DigitanksGame()->GetCurrentPlayer()->GetUpdateDownloading())
			AddActionItem(NULL, ACTIONTYPE_DOWNLOADUPDATES);
	}

	CountProducers();
	CountFleetPoints();
	CountBandwidth();
	CountScore();

	m_flPower += m_flPowerPerTurn;

	DigitanksWindow()->GetHUD()->Layout();
}

void CDigitanksPlayer::EndTurn()
{
	m_sTurnInfo = "";
	m_aActionItems.clear();

	for (size_t i = 0; i < m_ahUnits.size(); i++)
	{
		if (!m_ahUnits[i])
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahUnits[i].GetPointer());
		if (pEntity)
		{
			pEntity->EndTurn();
			continue;
		}
	}
}

void CDigitanksPlayer::CountProducers()
{
	if (!GameNetwork()->IsHost())
		return;

	m_flPowerPerTurn = 0;

	// Find and count producers and accumulate production points
	for (size_t i = 0; i < m_ahUnits.size(); i++)
	{
		if (!m_ahUnits[i])
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahUnits[i].GetPointer());
		if (!pEntity)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(m_ahUnits[i].GetPointer());
		if (pStructure && pStructure->Power())
			AddPowerPerTurn(pStructure->Power());

		CCollector* pCollector = dynamic_cast<CCollector*>(m_ahUnits[i].GetPointer());
		if (pCollector && !pCollector->IsConstructing() && !pCollector->IsUpgrading())
			AddPowerPerTurn(pCollector->GetPowerProduced());
	}
}

void CDigitanksPlayer::AddPowerPerTurn(float flPower)
{
	if (!GameNetwork()->IsHost())
		return;

	m_flPowerPerTurn += flPower;
}

void CDigitanksPlayer::ConsumePower(float flPower)
{
	TAssert(m_flPower >= flPower);

	if (flPower > m_flPower)
		m_flPower = 0;
	else
		m_flPower -= flPower;
}

void CDigitanksPlayer::CountFleetPoints()
{
	if (!GameNetwork()->IsHost())
		return;

	m_iTotalFleetPoints = 0;
	m_iUsedFleetPoints = 0;

	// Find and count fleet points
	for (size_t i = 0; i < m_ahUnits.size(); i++)
	{
		if (!m_ahUnits[i])
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahUnits[i].GetPointer());
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

size_t CDigitanksPlayer::GetUnusedFleetPoints()
{
	if (GetUsedFleetPoints() > GetTotalFleetPoints())
		return 0;

	return GetTotalFleetPoints() - GetUsedFleetPoints();
}

void CDigitanksPlayer::CountScore()
{
	if (!GameNetwork()->IsHost())
		return;

	if (!m_bIncludeInScoreboard)
		return;

	m_iScore = 0;

	// Find and count fleet points
	for (size_t i = 0; i < m_ahUnits.size(); i++)
	{
		if (!m_ahUnits[i])
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahUnits[i].GetPointer());
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

SERVER_GAME_COMMAND(YouLoseSirGoodDay)
{
	DigitanksGame()->GetListener()->GameOver(false);
}

void CDigitanksPlayer::YouLoseSirGoodDay()
{
	m_bLost = true;

	::YouLoseSirGoodDay.RunCommand("", GetClient());
}

void CDigitanksPlayer::CountBandwidth()
{
	if (!GameNetwork()->IsHost())
		return;

	m_flBandwidth = 0;

	for (size_t i = 0; i < m_ahUnits.size(); i++)
	{
		if (!m_ahUnits[i])
			continue;

		CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(m_ahUnits[i].GetPointer());
		if (!pEntity)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);

		if (pStructure)
			m_flBandwidth += pStructure->Bandwidth();
	}
}

void CDigitanksPlayer::AppendTurnInfo(const tstring& sTurnInfo)
{
	if (m_sTurnInfo.length() == 0)
		m_sTurnInfo = "TURN REPORT\n \n";

	m_sTurnInfo += "* " + sTurnInfo + "\n";
}

tstring CDigitanksPlayer::GetTurnInfo()
{
	return m_sTurnInfo;
}

void CDigitanksPlayer::OnDeleted(CBaseEntity* pEntity)
{
	BaseClass::OnDeleted(pEntity);

	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if ((CBaseEntity*)m_ahTanks[i] == pEntity)
			m_ahTanks.erase(i);
	}
}

size_t CDigitanksPlayer::GetNumTanksAlive() const
{
	size_t iTanksAlive = 0;
	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if (m_ahTanks[i]->IsAlive())
			iTanksAlive++;
	}

	return iTanksAlive;
}

void CDigitanksPlayer::CalculateVisibility()
{
	m_aflVisibilities.clear();
	// For every entity in the game, calculate the visibility to this team
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
		CalculateEntityVisibility(CBaseEntity::GetEntity(i));
}

void CDigitanksPlayer::CalculateEntityVisibility(CBaseEntity* pEntity)
{
	if (!pEntity)
		return;

	CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);
	if (pDTEntity)
		pDTEntity->DirtyVisibility();

	if (pEntity->GetOwner() == this)
	{
		m_aflVisibilities[pEntity->GetHandle()] = 1;
		return;
	}

	CWreckage* pWreckage = dynamic_cast<CWreckage*>(pEntity);
	if (pWreckage)
	{
		if (pWreckage->GetOldPlayer() == this && GameServer()->GetGameTime() - pWreckage->GetSpawnTime() < 10)
		{
			// We can continue to see our wreckage even after it dies, just in case we die in a remote place.
			m_aflVisibilities[pEntity->GetHandle()] = 1;
			return;
		}
	}

	Vector vecOrigin;
	CDigitank* pDigitank = dynamic_cast<CDigitank*>(pEntity);
	if (pDigitank)
		vecOrigin = pDigitank->GetRealOrigin();
	else
		vecOrigin = pEntity->GetGlobalOrigin();

	bool bCloak = false;
	if (pDigitank && pDigitank->IsCloaked())
		bCloak = true;

	float flVisibility = GetVisibilityAtPoint(vecOrigin, bCloak);

	m_aflVisibilities[pEntity->GetHandle()] = flVisibility;
}

float CDigitanksPlayer::GetEntityVisibility(size_t iHandle)
{
	tmap<size_t, float>::iterator it = m_aflVisibilities.find(iHandle);
	if (it == m_aflVisibilities.end())
		return 0;

	return (*it).second;
}

float CDigitanksPlayer::GetVisibilityAtPoint(Vector vecPoint, bool bCloak) const
{
	if (IsHumanControlled() && !DigitanksGame()->ShouldRenderFogOfWar())
		return 1;

	float flFinalVisibility = 0;

	// For every entity on this team, see what the visibility is
	for (size_t j = 0; j < m_ahUnits.size(); j++)
	{
		if (!m_ahUnits[j])
			continue;

		CDigitanksEntity* pTeammate = dynamic_cast<CDigitanksEntity*>(m_ahUnits[j].GetPointer());
		if (!pTeammate)
			continue;

		if (pTeammate->VisibleRange() == 0)
			continue;

		Vector vecOrigin;
		CDigitank* pDigitank = dynamic_cast<CDigitank*>(pTeammate);
		if (pDigitank)
			vecOrigin = pDigitank->GetRealOrigin();
		else
			vecOrigin = pTeammate->GetGlobalOrigin();

		float flVisibileRange = pTeammate->VisibleRange();
		if (bCloak)
			flVisibileRange /= 2;

		float flVisibility = RemapValClamped((vecOrigin - vecPoint).Length(), flVisibileRange, flVisibileRange+DigitanksGame()->FogPenetrationDistance(), 1.0f, 0.0f);

		// Use the brightest visibility
		if (flVisibility > flFinalVisibility)
			flFinalVisibility = flVisibility;

		if (flFinalVisibility > 0.99f)
			return 1.0f;
	}

	return flFinalVisibility;
}

void CDigitanksPlayer::AddActionItem(const CSelectable* pUnit, actiontype_t eActionType)
{
	if (pUnit && pUnit->GetDigitanksPlayer() != this)
		return;

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD && DigitanksGame()->GetGameType() != GAMETYPE_CAMPAIGN)
		return;

	if (!GetPrimaryCPU())
		return;

	// Prevent duplicates
	for (size_t i = 0; i < m_aActionItems.size(); i++)
	{
		if (!pUnit && m_aActionItems[i].iUnit == ~0 && eActionType == m_aActionItems[i].eActionType)
			return;

		if (pUnit && m_aActionItems[i].iUnit == pUnit->GetHandle())
		{
			// Use the lowest value, that list is sorted that way.
			if (eActionType < m_aActionItems[i].eActionType)
				m_aActionItems.Index(i).eActionType = eActionType;

			return;
		}
	}

	actionitem_t* pActionItem = &m_aActionItems.push_back();
	pActionItem->iUnit = pUnit?pUnit->GetHandle():~0;
	pActionItem->eActionType = eActionType;
	pActionItem->bHandled = false;
	DigitanksWindow()->GetHUD()->OnAddNewActionItem();
}

void CDigitanksPlayer::ClearActionItems()
{
	m_aActionItems.clear();
}

void CDigitanksPlayer::ServerHandledActionItem(size_t i)
{
	if (i >= m_aActionItems.size())
		return;

	m_aActionItems.Index(i).bHandled = true;
	DigitanksWindow()->GetHUD()->Layout();
}

CLIENT_GAME_COMMAND(HandledActionItem)
{
	if (pCmd->GetNumArguments() < 2)
	{
		TMsg("HandledActionItem with less than 2 arguments.\n");
		return;
	}

	CEntityHandle<CDigitanksPlayer> hTeam(pCmd->ArgAsUInt(0));

	if (!hTeam)
	{
		TMsg("HandledActionItem with invalid team.\n");
		return;
	}

	if ((int)iClient >= 0 && hTeam->GetClient() != iClient)
	{
		TMsg("HandledActionItem with wrong team.\n");
		return;
	}

	hTeam->ServerHandledActionItem(pCmd->ArgAsUInt(1));
}

void CDigitanksPlayer::HandledActionItem(size_t i)
{
	::HandledActionItem.RunCommand(sprintf(tstring("%d %d"), GetHandle(), i));

	// Predict the handling so it happens immediately.
	if (!GameNetwork()->IsHost())
		ServerHandledActionItem(i);
}

void CDigitanksPlayer::HandledActionItem(CSelectable* pUnit)
{
	if (!pUnit)
		return;

	size_t iItem = ~0;
	for (size_t i = 0; i < m_aActionItems.size(); i++)
	{
		if (m_aActionItems[i].iUnit == pUnit->GetHandle())
		{
			iItem = i;
			break;
		}
	}

	if (iItem == ~0)
		return;

	if (!pUnit->NeedsOrders())
		HandledActionItem(iItem);
}

void CDigitanksPlayer::HandledActionItem(actiontype_t eItem)
{
	for (size_t i = 0; i < m_aActionItems.size(); i++)
	{
		if (m_aActionItems[i].eActionType == eItem)
		{
			HandledActionItem(i);
			return;
		}
	}
}

void CDigitanksPlayer::DownloadUpdate(int iX, int iY, bool bCheckValid)
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

	GameNetwork()->CallFunctionParameters(NETWORK_TOEVERYONE, "DownloadUpdate", &p);
}

void CDigitanksPlayer::DownloadUpdate(class CNetworkParameters* p)
{
	int iX = p->i2;
	int iY = p->i3;
	bool bCheckValid = !!p->i4;

	if (m_iCurrentUpdateX == iX && m_iCurrentUpdateY == iY)
		return;

	if (bCheckValid && !CanDownloadUpdate(iX, iY))
		return;

	// If we were downloading something, move all extra off and half of what's been downloaded so far.
	if (GetUpdateDownloading())
	{
		if (m_flUpdateDownloaded > GetUpdateDownloading()->m_flSize)
		{
			float flDifference = m_flUpdateDownloaded - GetUpdateDownloading()->m_flSize;
			m_flMegabytes += flDifference;
			m_flUpdateDownloaded -= flDifference;
		}
		m_flMegabytes += m_flUpdateDownloaded/2;
	}

	m_iCurrentUpdateX = iX;
	m_iCurrentUpdateY = iY;

	if (GameNetwork()->IsHost())
	{
		m_flUpdateDownloaded += m_flMegabytes;
		m_flMegabytes = 0;

		if (GetUpdateDownloaded() >= GetUpdateSize())
		{
			// Return the excess.
			m_flMegabytes = GetUpdateDownloaded() - GetUpdateSize();

			DownloadComplete();
		}
	}
}

float CDigitanksPlayer::GetUpdateSize()
{
	if (m_iCurrentUpdateX < 0 || m_iCurrentUpdateY < 0)
		return 0;

	if (!DigitanksGame()->GetUpdateGrid())
		return 0;

	return DigitanksGame()->GetUpdateGrid()->m_aUpdates[m_iCurrentUpdateX][m_iCurrentUpdateY].m_flSize;
}

void CDigitanksPlayer::DownloadComplete(bool bInformMembers)
{
	if (m_iCurrentUpdateX < 0 || m_iCurrentUpdateY < 0)
		return;

	if (!DigitanksGame()->GetUpdateGrid())
		return;

	if (!GameNetwork()->IsHost())
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.i2 = !!bInformMembers;

	DownloadComplete(&p);

	GameNetwork()->CallFunctionParameters(NETWORK_TOCLIENTS, "DownloadComplete", &p);
}

void CDigitanksPlayer::DownloadComplete(class CNetworkParameters* p)
{
	bool bInformMembers = !!p->i2;

	CUpdateItem* pItem = &DigitanksGame()->GetUpdateGrid()->m_aUpdates[m_iCurrentUpdateX][m_iCurrentUpdateY];
	bool* pbTeamUpdate = &m_abUpdates.Get2D(UPDATE_GRID_SIZE, m_iCurrentUpdateX, m_iCurrentUpdateY);

	if (bInformMembers)
	{
		for (size_t i = 0; i < GetNumUnits(); i++)
		{
			CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(GetUnit(i));
			if (!pEntity)
				continue;

			pEntity->DownloadComplete(m_iCurrentUpdateX, m_iCurrentUpdateY);
		}
	}

	m_iCurrentUpdateX = m_iCurrentUpdateY = -1;

	AddActionItem(NULL, ACTIONTYPE_DOWNLOADCOMPLETE);

	if (!GameNetwork()->IsHost())
		return;

	// Host-only shit from here on out, gets auto-sent to the clients.

	*pbTeamUpdate = true;

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

bool CDigitanksPlayer::HasDownloadedUpdate(int iX, int iY) const
{
	return m_abUpdates.Get2D(UPDATE_GRID_SIZE, iX, iY);
}

bool CDigitanksPlayer::CanDownloadUpdate(int iX, int iY) const
{
	if (HasDownloadedUpdate(iX, iY))
		return false;

	CUpdateGrid* pGrid = DigitanksGame()->GetUpdateGrid();

	if (!pGrid)
		return false;

	if (pGrid->m_aUpdates[iX][iY].m_eUpdateClass == UPDATECLASS_EMPTY)
		return false;

	if (iX > 0 && m_abUpdates.Get2D(UPDATE_GRID_SIZE, iX-1, iY))
		return true;

	if (iY > 0 && m_abUpdates.Get2D(UPDATE_GRID_SIZE, iX, iY-1))
		return true;

	if (iX < UPDATE_GRID_SIZE-1 && m_abUpdates.Get2D(UPDATE_GRID_SIZE, iX+1, iY))
		return true;

	if (iY < UPDATE_GRID_SIZE-1 && m_abUpdates.Get2D(UPDATE_GRID_SIZE, iX, iY+1))
		return true;

	return false;
}

bool CDigitanksPlayer::IsDownloading(int iX, int iY)
{
	return m_iCurrentUpdateX == iX && m_iCurrentUpdateY == iY;
}

CUpdateItem* CDigitanksPlayer::GetUpdateDownloading()
{
	if (m_iCurrentUpdateX < 0 || m_iCurrentUpdateY < 0)
		return NULL;

	return &DigitanksGame()->GetUpdateGrid()->m_aUpdates[m_iCurrentUpdateX][m_iCurrentUpdateY];
}

size_t CDigitanksPlayer::GetTurnsToDownload()
{
	if (GetBandwidth() == 0)
		return 0;

	int iTurns = (int)((GetUpdateSize()-m_flUpdateDownloaded)/GetBandwidth())+1;

	if (iTurns < 1)
		return 1;

	return iTurns;
}

bool CDigitanksPlayer::CanBuildMiniBuffers()
{
	if (!DigitanksGame()->CanBuildMiniBuffers())
		return false;

	return true;
}

bool CDigitanksPlayer::CanBuildBuffers()
{
	if (!DigitanksGame()->CanBuildBuffers())
		return false;

	if (!DigitanksGame()->GetUpdateGrid())
		return true;

	return m_bCanBuildBuffers;
}

bool CDigitanksPlayer::CanBuildBatteries()
{
	if (!DigitanksGame()->CanBuildBatteries())
		return false;

	return true;
}

bool CDigitanksPlayer::CanBuildPSUs()
{
	if (!DigitanksGame()->CanBuildPSUs())
		return false;

	if (!DigitanksGame()->GetUpdateGrid())
		return true;

	return m_bCanBuildPSUs;
}

bool CDigitanksPlayer::CanBuildLoaders()
{
	if (CanBuildInfantryLoaders())
		return true;

	if (CanBuildTankLoaders())
		return true;

	if (CanBuildArtilleryLoaders())
		return true;

	return false;
}

bool CDigitanksPlayer::CanBuildInfantryLoaders()
{
	if (!DigitanksGame()->CanBuildInfantryLoaders())
		return false;

	if (!DigitanksGame()->GetUpdateGrid())
		return true;

	return true; //m_bCanBuildInfantryLoaders;
}

bool CDigitanksPlayer::CanBuildTankLoaders()
{
	if (!DigitanksGame()->CanBuildTankLoaders())
		return false;

	if (!DigitanksGame()->GetUpdateGrid())
		return true;

	return m_bCanBuildTankLoaders;
}

bool CDigitanksPlayer::CanBuildArtilleryLoaders()
{
	if (!DigitanksGame()->CanBuildArtilleryLoaders())
		return false;

	if (!DigitanksGame()->GetUpdateGrid())
		return true;

	return m_bCanBuildArtilleryLoaders;
}

CDigitank* CDigitanksPlayer::GetTank(size_t i) const
{
	if (!m_ahTanks.size())
		return NULL;

	return m_ahTanks[i];
}

void CDigitanksPlayer::AddUnit(CDigitanksEntity* pEntity)
{
	if (!pEntity)
		return;

	for (size_t i = 0; i < m_ahUnits.size(); i++)
	{
		// If we're already on this team, forget it.
		// Calling the OnTeamChange() hooks just to stay on this team can be dangerous.
		if (pEntity == m_ahUnits[i])
			return;
	}

	if (pEntity->GetPlayerOwner())
		RemoveUnit(pEntity);

	pEntity->SetOwner(this);
	m_ahUnits.push_back(pEntity);

	OnAddUnit(pEntity);
}

void CDigitanksPlayer::OnAddUnit(CDigitanksEntity* pEntity)
{
	CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
	if (pTank)
		m_ahTanks.push_back(pTank);

	m_aflVisibilities[pEntity->GetHandle()] = 1;

	const CCPU* pCPU = dynamic_cast<const CCPU*>(pEntity);
	if (!m_hPrimaryCPU && pCPU)
		m_hPrimaryCPU = pCPU;

	DigitanksWindow()->GetHUD()->OnAddUnitToTeam(this, pEntity);
}

void CDigitanksPlayer::RemoveUnit(CDigitanksEntity* pEntity)
{
	for (size_t i = 0; i < m_ahUnits.size(); i++)
	{
		if (pEntity == m_ahUnits[i])
		{
			m_ahUnits.erase(i);
			pEntity->SetOwner(NULL);
			OnRemoveUnit(pEntity);
			return;
		}
	}
}

void CDigitanksPlayer::OnRemoveUnit(CDigitanksEntity* pEntity)
{
	DigitanksWindow()->GetHUD()->OnRemoveUnitFromTeam(this, pEntity);
}

CDigitanksEntity* CDigitanksPlayer::GetUnit(size_t i) const
{
	if (i >= m_ahUnits.size())
		return NULL;

	return m_ahUnits[i];
}

void CDigitanksPlayer::MouseMotion(int dx, int dy)
{
	if (DigitanksWindow()->IsMouseRightDown())
		m_iMouseMoved += (int)(fabs((float)dx) + fabs((float)dy));
}

void CDigitanksPlayer::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	if (!DigitanksGame())
		return;

	if (DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	Vector vecMousePosition;
	CBaseEntity* pClickedEntity = NULL;
	DigitanksWindow()->GetMouseGridPosition(vecMousePosition, &pClickedEntity);
	DigitanksWindow()->GetMouseGridPosition(vecMousePosition, NULL, CG_TERRAIN);

	if (iButton == TINKER_KEY_MOUSE_RIGHT && iState == 0 && m_iMouseMoved < 30)
	{
		DigitanksGame()->SetControlMode(MODE_NONE);
	}

	TStubbed("MouseInput Player controls");

#if 0
	if (DigitanksGame()->GetControlMode() != MODE_NONE && iButton == TINKER_KEY_MOUSE_LEFT && iState == 1)
	{
		// While aiming moving turning or building, either mouse button can be used and selections are disabled.

		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
			DigitanksGame()->MoveTanks();
		else if (DigitanksGame()->GetControlMode() == MODE_TURN)
			DigitanksGame()->TurnTanks(vecMousePosition);
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
		{
			DigitanksGame()->FireTanks();
			GameWindow()->GetInstructor()->FinishedLesson("mission-1-fire-away");
		}
		else if (DigitanksGame()->GetControlMode() == MODE_BUILD)
		{
			if (iButton == TINKER_KEY_MOUSE_RIGHT)
			{
				DigitanksGame()->SetControlMode(MODE_NONE);
			}
			else if (DigitanksGame()->GetCurrentLocalDigitanksPlayer())
			{
				CCPU* pCPU = DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetPrimaryCPU();
				if (pCPU && pCPU->IsPreviewBuildValid())
				{
					pCPU->BeginConstruction();
					DigitanksGame()->SetControlMode(MODE_NONE);
				}
			}
		}

		GetHUD()->SetupMenu();

		if (iState == 1)
			// Don't allow the release to take any action either.
			m_bMouseDownInGUI = true;

		return;
	}

	if (iButton == TINKER_KEY_MOUSE_RIGHT)
	{
		if (iState == 1)
			m_iMouseMoved = 0;
		else
		{
			if (m_iMouseMoved > 30)
				GameWindow()->GetInstructor()->FinishedLesson("mission-1-rotateview");
		}
	}

	if (iButton == TINKER_KEY_MOUSE_LEFT)
	{
		if (iState == 1 && !DigitanksGame()->IsFeatureDisabled(DISABLE_SELECT))
		{
			// Prevent UI interactions from affecting the camera target.
			// If the mouse was used no the UI, this will remain false.
			m_bBoxSelect = true;
			m_iMouseInitialX = m_iMouseCurrentX = mx;
			m_iMouseInitialY = m_iMouseCurrentY = my;
		}
	}

	if (bDoubleClick && pClickedEntity && DigitanksGame()->GetCurrentLocalDigitanksPlayer() && !DigitanksGame()->IsFeatureDisabled(DISABLE_SELECT))
	{
		CSelectable* pClickedSelectable = dynamic_cast<CSelectable*>(pClickedEntity);

		if (pClickedSelectable)
		{
			DigitanksGame()->GetCurrentLocalDigitanksPlayer()->SetPrimarySelection(pClickedSelectable);

			for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
			{
				CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
				if (!pEntity)
					continue;

				CSelectable* pSelectable = dynamic_cast<CSelectable*>(pEntity);
				if (!pSelectable)
					continue;

				if (pSelectable->GetVisibility() == 0)
					continue;

				if (pSelectable->GetPlayerOwner() != pClickedSelectable->GetPlayerOwner())
					continue;

				if (pSelectable->GetUnitType() != pClickedSelectable->GetUnitType())
					continue;

				if (pSelectable->Distance(pClickedEntity->GetGlobalOrigin()) > 25)
					continue;

				Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(pSelectable->GetGlobalOrigin());

				if (vecScreen.x < 0 || vecScreen.y < 0 || vecScreen.x > GetWindowWidth() || vecScreen.y > GetWindowHeight())
					continue;

				if (DigitanksGame()->GetCurrentLocalDigitanksPlayer())
					DigitanksGame()->GetCurrentLocalDigitanksPlayer()->AddToSelection(pSelectable);
			}
		}
	}
	else if (iState == 0 && iButton == TINKER_KEY_MOUSE_LEFT && !GameWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_SELECT))
	{
		if (m_bBoxSelect && IsMouseDragging() && !bDoubleClick)
		{
			if (!IsShiftDown() && DigitanksGame()->GetCurrentLocalDigitanksPlayer())
				DigitanksGame()->GetCurrentLocalDigitanksPlayer()->SetPrimarySelection(NULL);

			size_t iLowerX = (m_iMouseInitialX < m_iMouseCurrentX) ? m_iMouseInitialX : m_iMouseCurrentX;
			size_t iLowerY = (m_iMouseInitialY < m_iMouseCurrentY) ? m_iMouseInitialY : m_iMouseCurrentY;
			size_t iHigherX = (m_iMouseInitialX > m_iMouseCurrentX) ? m_iMouseInitialX : m_iMouseCurrentX;
			size_t iHigherY = (m_iMouseInitialY > m_iMouseCurrentY) ? m_iMouseInitialY : m_iMouseCurrentY;

			for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
			{
				CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
				if (!pEntity)
					continue;

				CSelectable* pSelectable = dynamic_cast<CSelectable*>(pEntity);
				if (!pSelectable)
					continue;

				if (pSelectable->GetVisibility() == 0)
					continue;

				Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(pSelectable->GetGlobalOrigin());

				if (vecScreen.x < iLowerX || vecScreen.y < iLowerY || vecScreen.x > iHigherX || vecScreen.y > iHigherY)
					continue;

				if (DigitanksGame()->GetCurrentLocalDigitanksPlayer())
					DigitanksGame()->GetCurrentLocalDigitanksPlayer()->AddToSelection(pSelectable);
			}

//			if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() && DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumSelected() == 3)
//				GetInstructor()->FinishedLesson(CInstructor::TUTORIAL_BOXSELECT);
		}
		else if (pClickedEntity && DigitanksGame()->GetCurrentLocalDigitanksPlayer())
		{
			CSelectable* pSelectable = dynamic_cast<CSelectable*>(pClickedEntity);

			if (pSelectable)
			{
				if (IsShiftDown())
					DigitanksGame()->GetCurrentLocalDigitanksPlayer()->AddToSelection(pSelectable);
				else
					DigitanksGame()->GetCurrentLocalDigitanksPlayer()->SetPrimarySelection(pSelectable);
			}
			else if (!IsShiftDown())
			{
				DigitanksGame()->GetCurrentLocalDigitanksPlayer()->SetPrimarySelection(NULL);
				GetHUD()->CloseWeaponPanel();
			}

//			if (DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumSelected() == 3)
//				GetInstructor()->FinishedLesson(CInstructor::TUTORIAL_SHIFTSELECT);
		}

		m_bBoxSelect = false;
	}

	GetHUD()->SetupMenu();
#endif
}

void CDigitanksPlayer::MouseDoubleClick()
{
	m_bBoxSelect = false;
}

void CDigitanksPlayer::MouseWheel(int iState)
{
	if (DigitanksGame() && DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	static int iOldState = 0;

	TStubbed("MouseWheel controls");
#if 0
	if (GameServer() && GameServer()->GetCamera())
	{
		if (iState > iOldState)
			DigitanksGame()->GetOverheadCamera()->ZoomIn();
		else
			DigitanksGame()->GetOverheadCamera()->ZoomOut();
	}
#endif

	iOldState = iState;
}

void CDigitanksPlayer::KeyPress(int c)
{
	if (glgui::CRootPanel::Get()->KeyPressed(c, GameWindow()->IsCtrlDown()))
		return;

	if (DigitanksGame() && DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	if (c == TINKER_KEY_F4 && GameWindow()->IsAltDown())
		exit(0);

	if (DigitanksGame() && (c == TINKER_KEY_ENTER || c == TINKER_KEY_KP_ENTER))
	{
		if (!DigitanksGame()->IsFeatureDisabled(DISABLE_ENTER) && DigitanksGame()->GetCurrentLocalDigitanksPlayer() == DigitanksGame()->GetCurrentPlayer())
		{
			CSoundLibrary::PlaySound(NULL, "sound/turn.wav");
			DigitanksGame()->EndTurn();
		}
	}

	if (c == TINKER_KEY_ESCAPE)
	{
		if (DigitanksWindow()->GetMenu()->IsVisible())
			DigitanksWindow()->GetMenu()->SetVisible(false);
		else if (DigitanksGame() && (DigitanksGame()->GetControlMode() == MODE_NONE || DigitanksGame()->GetPrimarySelection() == NULL))
			DigitanksWindow()->GetMenu()->SetVisible(true);
		else if (DigitanksWindow()->GetHUD()->GetUpdatesPanel()->IsVisible())
			DigitanksWindow()->GetHUD()->GetUpdatesPanel()->CloseCallback("");
		else if (DigitanksGame())
			DigitanksGame()->SetControlMode(MODE_NONE);

		if (DigitanksGame())
			DigitanksWindow()->SetContextualCommandsOverride(true);
	}

	TStubbed("KeyPress controls");
#if 0
	if (GameServer() && GameServer()->GetCamera())
		GameServer()->GetCamera()->KeyDown(c);

	if (c == ' ')
	{
		// Use m_iMouseInitialX to start tracking where teh spacebar was pressed so we can tell when it's released if it was dragged or not.
		// Then we'll know whether to drag the camera or blow up projectiles.
		m_iMouseInitialX = m_iMouseCurrentX;
		m_iMouseInitialY = m_iMouseCurrentY;
	}

	if (c == 'H')
	{
		if (DigitanksGame()->GetCurrentLocalDigitanksPlayer())
		{
			for (size_t i = 0; i < DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumMembers(); i++)
			{
				const CBaseEntity* pMember = DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetMember(i);
				const CCPU* pCPU = dynamic_cast<const CCPU*>(pMember);
				if (pCPU)
				{
					DigitanksGame()->GetCurrentLocalDigitanksPlayer()->SetPrimarySelection(pCPU);
					break;
				}
			}
		}
	}

	if (GetHUD())
	{
		if (c == 'Q')
			GetHUD()->ButtonCallback(0);

		if (c == 'W')
			GetHUD()->ButtonCallback(1);

		if (c == 'E')
			GetHUD()->ButtonCallback(2);

		if (c == 'R')
			GetHUD()->ButtonCallback(3);

		if (c == 'T')
			GetHUD()->ButtonCallback(4);

		if (c == 'A')
			GetHUD()->ButtonCallback(5);

		if (c == 'S')
			GetHUD()->ButtonCallback(6);

		if (c == 'D')
			GetHUD()->ButtonCallback(7);

		if (c == 'F')
			GetHUD()->ButtonCallback(8);

		if (c == 'G')
			GetHUD()->ButtonCallback(9);
	}

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->AllowCheats())
		return;

	// Cheats from here on out
	if (c == 'X')
		DigitanksGame()->SetRenderFogOfWar(!DigitanksGame()->ShouldRenderFogOfWar());

	if (c == 'C')
		DigitanksGame()->CompleteProductions();

	if (c == 'V')
	{
		if (DigitanksGame()->GetPrimarySelection())
			DigitanksGame()->GetPrimarySelection()->Delete();
	}

	if (c == 'B')
	{
		CDigitanksPlayer* pTeam = DigitanksGame()->GetCurrentPlayer();
		for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
		{
			for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
			{
				if (DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y].m_eUpdateClass == UPDATECLASS_EMPTY)
					continue;

				pTeam->DownloadUpdate(x, y, false);
				pTeam->DownloadComplete();
			}
		}
	}

	if (c == 'N')
		GetHUD()->SetVisible(!GetHUD()->IsVisible());

	if (c == 'M')
	{
		if (DigitanksGame()->GetPrimarySelection())
			DigitanksGame()->TankSpeak(DigitanksGame()->GetPrimarySelectionTank(), ":D!");
	}
#endif
}

void CDigitanksPlayer::KeyRelease(int c)
{
	TStubbed("KeyRelease controls");
#if 0
	if (GameServer() && GameServer()->GetCamera())
		GameServer()->GetCamera()->KeyUp(c);

	if (c == ' ' && !IsMouseDragging())
		DigitanksGame()->WeaponSpecialCommand();
#endif
}

void CDigitanksPlayer::CharPress(int c)
{
	TStubbed("CharPress controls");
#if 0
	if (c == '`')
	{
		ToggleConsole();
		return;
	}

	if (c == 'y' && !DigitanksWindow()->IsChatOpen())
	{
		DigitanksWindow()->OpenChat();
		return;
	}

	if (glgui::CRootPanel::Get()->CharPressed(c))
		return;

	if (!GetHUD())
		return;
#endif
}

void CDigitanksPlayer::CharRelease(int c)
{
}
