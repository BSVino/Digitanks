#include "digitanksplayer.h"

#include <maths.h>
#include <geometry.h>
#include <mtrand.h>

#include "digitanksgame.h"
#include "updates.h"
#include "structures/resource.h"
#include "structures/loader.h"
#include "structures/buffer.h"

#include "units/artillery.h"
#include "units/maintank.h"
#include "units/mechinf.h"
#include "units/scout.h"

typedef struct
{
	size_t x;
	size_t y;
} update_coordinate_t;

void CDigitanksPlayer::Bot_DownloadUpdates()
{
	CUpdateGrid* pGrid = DigitanksGame()->GetUpdateGrid();
	if (!pGrid)
		return;

	if (GetUpdateDownloading())
		return;

	tvector<update_coordinate_t> aUpdatesAvailable;

	bool bGetTanks = CanBuildPSUs();
	bool bGetArtillery = CanBuildPSUs() && CanBuildTankLoaders() && CanBuildBuffers();
	bool bGetBuffers = CanBuildPSUs();
	bool bGetPSU = true;

	for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
	{
		for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
		{
			if (!CanDownloadUpdate(x, y))
				continue;

			if (pGrid->m_aUpdates[x][y].m_eUpdateClass == UPDATECLASS_STRUCTURE)
			{
				// If it's a structure we need then it's top priority, grab it NAOW.
				if (pGrid->m_aUpdates[x][y].m_eStructure == STRUCTURE_PSU)
				{
					DownloadUpdate(x, y);
					return;
				}
				else if (pGrid->m_aUpdates[x][y].m_eStructure == STRUCTURE_TANKLOADER && bGetTanks)
				{
					DownloadUpdate(x, y);
					return;
				}
				else if (pGrid->m_aUpdates[x][y].m_eStructure == STRUCTURE_ARTILLERYLOADER && bGetArtillery)
				{
					DownloadUpdate(x, y);
					return;
				}
				else if (pGrid->m_aUpdates[x][y].m_eStructure == STRUCTURE_BUFFER && bGetBuffers)
				{
					DownloadUpdate(x, y);
					return;
				}
				// Otherwise skip it for now. We don't want it downloading artilleries when we really need power supplies.
				else
					continue;
			}

			aUpdatesAvailable.push_back(update_coordinate_t());
			aUpdatesAvailable[aUpdatesAvailable.size()-1].x = x;
			aUpdatesAvailable[aUpdatesAvailable.size()-1].y = y;
		}
	}

	if (!aUpdatesAvailable.size())
		return;

	size_t iRandom = RandomInt(0, aUpdatesAvailable.size()-1);
	size_t x, y;
	x = aUpdatesAvailable[iRandom].x;
	y = aUpdatesAvailable[iRandom].y;
	DownloadUpdate(x, y);
}

bool CDigitanksPlayer::Bot_BuildFirstPriority()
{
	CTerrain* pTerrain = DigitanksGame()->GetTerrain();

	if (!m_hPrimaryCPU)
		return false;

	if (m_aeBuildPriorities.size() == m_iBuildPrioritiesHead)
		return false;

	builditem_t* pNextBuild = &m_aeBuildPriorities[m_iBuildPrioritiesHead];

	if (DigitanksGame()->GetConstructionCost(pNextBuild->m_eUnit) > GetPower())
		return false;

	if (pNextBuild->m_eUnit == UNIT_SCOUT)
	{
		m_hPrimaryCPU->BeginRogueProduction();
		m_iLastScoutBuilt = DigitanksGame()->GetTurn();
		return m_hPrimaryCPU->IsProducing();
	}

	if (pNextBuild->m_eUnit == UNIT_INFANTRY)
	{
		if (!m_hInfantryLoader)
			return false;

		m_hInfantryLoader->BeginProduction();
		return m_hInfantryLoader->IsProducing();
	}

	if (pNextBuild->m_eUnit == UNIT_TANK)
	{
		if (!m_hTankLoader)
			return false;

		m_hTankLoader->BeginProduction();
		return m_hTankLoader->IsProducing();
	}

	if (pNextBuild->m_eUnit == UNIT_ARTILLERY)
	{
		if (!m_hArtilleryLoader)
			return false;

		m_hArtilleryLoader->BeginProduction();
		return m_hArtilleryLoader->IsProducing();
	}

	if (pNextBuild->m_eUnit == STRUCTURE_PSU && pNextBuild->m_hTarget != NULL && dynamic_cast<CBattery*>(pNextBuild->m_hTarget.GetPointer()))
	{
		CBattery* pBattery = static_cast<CBattery*>(pNextBuild->m_hTarget.GetPointer());
		pBattery->BeginUpgrade();

		// Don't upgrade two turns in a row.
		if (pBattery->IsUpgrading())
			m_bCanUpgrade = false;

		return pBattery->IsUpgrading();
	}

	if (pNextBuild->m_eUnit == STRUCTURE_BUFFER && pNextBuild->m_hTarget != NULL)
	{
		CMiniBuffer* pBuffer = static_cast<CMiniBuffer*>(pNextBuild->m_hTarget.GetPointer());
		pBuffer->BeginUpgrade();

		// Don't upgrade two turns in a row.
		if (pBuffer->IsUpgrading())
			m_bCanUpgrade = false;

		return pBuffer->IsUpgrading();
	}

	if (pNextBuild->m_eUnit == STRUCTURE_PSU || pNextBuild->m_eUnit == STRUCTURE_BATTERY)
	{
		return Bot_BuildCollector(dynamic_cast<CResourceNode*>(pNextBuild->m_hTarget.GetPointer()));
	}

	if (pNextBuild->m_eUnit == STRUCTURE_BUFFER || pNextBuild->m_eUnit == STRUCTURE_MINIBUFFER)
	{
		CResourceNode* pTargetResource = NULL;
		CSupplier* pClosestSupplier = NULL;

		while (true)
		{
			pTargetResource = CBaseEntity::FindClosest<CResourceNode>(m_hPrimaryCPU->GetGlobalOrigin(), pTargetResource);

			if (!pTargetResource)
				break;

			if (pTargetResource->HasCollector())
				continue;

			if (GetVisibilityAtPoint(pTargetResource->GetGlobalOrigin()) < 0.2f)
				continue;

			break;
		}

		if (pTargetResource)
		{
			while (true)
			{
				pClosestSupplier = CBaseEntity::FindClosest<CSupplier>(pTargetResource->GetGlobalOrigin(), pClosestSupplier);

				if (!pClosestSupplier)
					break;

				if (pClosestSupplier->GetDigitanksPlayer() != this)
					continue;

				break;
			}
		}

		if (pTargetResource && CSupplier::GetDataFlow(pTargetResource->GetGlobalOrigin(), this) > 0)
		{
			return Bot_BuildCollector(pTargetResource);
		}
		else if (pTargetResource && pClosestSupplier)
		{
			Vector vecStructurePath = pTerrain->FindPath(pClosestSupplier->GetGlobalOrigin(), pTargetResource->GetGlobalOrigin(), NULL);

			size_t iTries = 0;
			do
			{
				// Try slightly different stuff to get around structures that may be in the way.
				Vector vecWobble(RandomFloat(-5, 5), 0, RandomFloat(-5, 5));

				Vector vecStructureDirection = (vecStructurePath + vecWobble - pClosestSupplier->GetGlobalOrigin()).Normalized();
				Vector vecStructure = pClosestSupplier->GetGlobalOrigin() + vecStructureDirection * pClosestSupplier->GetDataFlowRadius()*9/10;
				if (CSupplier::GetDataFlow(vecStructure, this) <= 0)
					vecStructure = pClosestSupplier->GetGlobalOrigin() + vecStructureDirection * pClosestSupplier->GetDataFlowRadius()*2/3;

				vecStructure = pTerrain->GetPointHeight(vecStructure);

				m_hPrimaryCPU->SetPreviewStructure(CanBuildBuffers()?STRUCTURE_BUFFER:STRUCTURE_MINIBUFFER);
				m_hPrimaryCPU->SetPreviewBuild(vecStructure);

				if (m_hPrimaryCPU->IsPreviewBuildValid())
					break;

			} while (iTries++ < 5);

			// If we can't build this for some reason, build a random buffer instead.
			if (m_hPrimaryCPU->BeginConstruction())
				return true;
		}

		// Couldn't build it? Fall through and build it randomly as part of the code below.
	}

	CSupplier* pUnused = NULL;
	Vector vecStructure;

	unittype_t eBuild = pNextBuild->m_eUnit;

	size_t iTries = 0;
	do
	{
		if (eBuild == STRUCTURE_BUFFER || eBuild == STRUCTURE_MINIBUFFER)
			pUnused = Bot_FindUnusedSupplier(4, false);
		else
			pUnused = Bot_FindUnusedSupplier(2);

		// Not enough buffers? Build another one.
		if (pUnused == NULL)
		{
			eBuild = CanBuildBuffers()?STRUCTURE_BUFFER:STRUCTURE_MINIBUFFER;

			pUnused = Bot_FindUnusedSupplier(4, false);

			if (!pUnused)
				pUnused = Bot_FindUnusedSupplier(6, false);

			if (!pUnused)
				pUnused = Bot_FindUnusedSupplier(9999, false);
		}

		float flYaw;
		if (pUnused == m_hPrimaryCPU)
			flYaw = RandomFloat(0, 360);
		else
		{
			flYaw = VectorAngles(pUnused->GetGlobalOrigin() - m_hPrimaryCPU->GetGlobalOrigin()).y;
			flYaw = RandomFloat(flYaw-90, flYaw+90);
		}

		// Pick a random direction facing more or less away from the CPU so that we spread outwards.
		Vector vecStructureDirection = AngleVector(EAngle(0, flYaw, 0));
		vecStructure = pUnused->GetGlobalOrigin();
		if (eBuild == STRUCTURE_BUFFER || eBuild == STRUCTURE_MINIBUFFER)
		{
			Vector vecPreview = vecStructure + vecStructureDirection.Normalized() * pUnused->GetDataFlowRadius()*9/10;
			if (CSupplier::GetDataFlow(vecPreview, this) <= 0)
				vecPreview = vecStructure + vecStructureDirection.Normalized() * pUnused->GetDataFlowRadius()*2/3;

			vecStructure = vecPreview;
		}
		else
			vecStructure += vecStructureDirection.Normalized() * 20;

		// Don't build structures too close to the map edges.
		if (vecStructure.x < -pTerrain->GetMapSize()+15)
			continue;
		if (vecStructure.z < -pTerrain->GetMapSize()+15)
			continue;
		if (vecStructure.x > pTerrain->GetMapSize()-15)
			continue;
		if (vecStructure.z > pTerrain->GetMapSize()-15)
			continue;

		vecStructure = pTerrain->GetPointHeight(vecStructure);

		if (eBuild == STRUCTURE_INFANTRYLOADER || eBuild == STRUCTURE_TANKLOADER || eBuild == STRUCTURE_ARTILLERYLOADER)
		{
			if (pTerrain->IsPointOverWater(vecStructure) || pTerrain->IsPointOverLava(vecStructure))
				continue;

			Vector vecUnloadPoint = vecStructure + AngleVector(EAngle(0, 0, 0)) * 9;
			if (pTerrain->IsPointOverWater(vecUnloadPoint) || pTerrain->IsPointOverLava(vecUnloadPoint))
				continue;

			vecUnloadPoint = vecStructure + AngleVector(EAngle(0, 90, 0)) * 9;
			if (pTerrain->IsPointOverWater(vecUnloadPoint) || pTerrain->IsPointOverLava(vecUnloadPoint))
				continue;

			vecUnloadPoint = vecStructure + AngleVector(EAngle(0, 180, 0)) * 9;
			if (pTerrain->IsPointOverWater(vecUnloadPoint) || pTerrain->IsPointOverLava(vecUnloadPoint))
				continue;

			vecUnloadPoint = vecStructure + AngleVector(EAngle(0, -90, 0)) * 9;
			if (pTerrain->IsPointOverWater(vecUnloadPoint) || pTerrain->IsPointOverLava(vecUnloadPoint))
				continue;
		}

		if (pTerrain->IsPointOverHole(vecStructure))
			continue;

		m_hPrimaryCPU->SetPreviewStructure(eBuild);
		m_hPrimaryCPU->SetPreviewBuild(vecStructure);

		if (m_hPrimaryCPU->IsPreviewBuildValid())
			break;

	} while (iTries++ < 5);

	m_hPrimaryCPU->SetPreviewStructure(eBuild);
	m_hPrimaryCPU->SetPreviewBuild(vecStructure);

	if (!m_hPrimaryCPU->IsPreviewBuildValid())
		return false;

	return m_hPrimaryCPU->BeginConstruction();
}

void CDigitanksPlayer::Bot_AssignDefenders()
{
	tvector<CStructure*> apDefend;
	for (size_t i = 0; i < m_ahUnits.size(); i++)
	{
		CBaseEntity* pEntity = m_ahUnits[i];
		if (!pEntity)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);
		if (!pStructure)
			continue;

		if (pStructure != m_hPrimaryCPU)
		{
			// Don't defend structures that face the back wall.
			Vector vecStructure = pStructure->GetGlobalOrigin() - m_hPrimaryCPU->GetGlobalOrigin();
			float flDot = -m_hPrimaryCPU->GetGlobalOrigin().Normalized().Dot(vecStructure.Normalized());
			if (flDot < 0)
				continue;
		}

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);

		apDefend.push_back(pStructure);

		// Non-suppliers and suppliers with no children should be defended with higher priority, to avoid defending structures in the center of the base.
		if (!pSupplier || pSupplier->GetNumChildren() == 0)
			apDefend.push_back(pStructure);
	}

	if (apDefend.size() == 0)
		return;

	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		CDigitank* pTank = m_ahTanks[i];
		if (!pTank)
			continue;

		if (!pTank->CanFortify())
			continue;

		if (pTank->IsArtillery())
			continue;

		if (pTank->HasFortifyPoint())
			continue;

		if (pTank->IsInAttackTeam())
			continue;

		if (pTank->ShouldStayPut())
			continue;

		size_t iFirst = rand()%apDefend.size();
		size_t iStructure = iFirst;
		CStructure* pDefendStructure;
		do
		{
			pDefendStructure = apDefend[iStructure++];
			iStructure = iStructure%apDefend.size();
		} while (pDefendStructure->GetNumLivingDefenders() > 2 && iStructure != iFirst);

		pDefendStructure->AddDefender(pTank);
	}
}

void CDigitanksPlayer::Bot_ExecuteTurn()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN)
	{
		Bot_ExecuteTurnCampaign();
		return;
	}

	if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY || m_bUseArtilleryAI)
	{
		Bot_ExecuteTurnArtillery();
		return;
	}

	// Do this first so we have the info while we're deciding what things to build.
	if (m_ahAttackTeam.size())
	{
		for (size_t i = m_ahAttackTeam.size()-1; i < m_ahAttackTeam.size(); i--)
		{
			if (!m_ahAttackTeam[i])
				m_ahAttackTeam.erase(m_ahAttackTeam.begin()+i);
		}
	}

	m_aeBuildPriorities.clear();
	m_iBuildPrioritiesHead = 0;

	if (m_hPrimaryCPU != NULL)
	{
		m_hInfantryLoader = NULL;
		m_hTankLoader = NULL;
		m_hArtilleryLoader = NULL;

		size_t iInfantry = 0;
		size_t iMainTanks = 0;
		size_t iArtillery = 0;
		size_t iScouts = 0;
		size_t iStructures = 0;
		size_t iAutoTurrets = 0;

		tvector<CEntityHandle<CMiniBuffer> > ahMinibufferUpgrades;
		tvector<CEntityHandle<CBattery> > ahBatteryUpgrades;

		for (size_t i = 0; i < GetNumUnits(); i++)
		{
			CBaseEntity* pEntity = GetUnit(i);
			if (!pEntity)
				continue;

			CDigitanksEntity* pDTEnt = dynamic_cast<CDigitanksEntity*>(pEntity);
			if (!pDTEnt)
				continue;

			if (pDTEnt->GetUnitType() == STRUCTURE_INFANTRYLOADER)
				m_hInfantryLoader = static_cast<CLoader*>(pDTEnt);
			else if (pDTEnt->GetUnitType() == STRUCTURE_TANKLOADER)
				m_hTankLoader = static_cast<CLoader*>(pDTEnt);
			else if (pDTEnt->GetUnitType() == STRUCTURE_ARTILLERYLOADER)
				m_hArtilleryLoader = static_cast<CLoader*>(pDTEnt);

			else if (pDTEnt->GetUnitType() == UNIT_INFANTRY)
				iInfantry += static_cast<CDigitank*>(pDTEnt)->FleetPoints();
			else if (pDTEnt->GetUnitType() == UNIT_TANK)
				iMainTanks += static_cast<CDigitank*>(pDTEnt)->FleetPoints();
			else if (pDTEnt->GetUnitType() == UNIT_ARTILLERY)
				iArtillery += static_cast<CDigitank*>(pDTEnt)->FleetPoints();
			else if (pDTEnt->GetUnitType() == UNIT_SCOUT)
				iScouts++;

			else if (pDTEnt->GetUnitType() == STRUCTURE_MINIBUFFER && CanBuildBuffers())
				ahMinibufferUpgrades.push_back(static_cast<CMiniBuffer*>(pDTEnt));
			else if (pDTEnt->GetUnitType() == STRUCTURE_BATTERY && CanBuildPSUs())
				ahBatteryUpgrades.push_back(static_cast<CBattery*>(pDTEnt));

			if (pDTEnt->GetUnitType() == STRUCTURE_FIREWALL)
				iAutoTurrets++;
			else if (dynamic_cast<CStructure*>(pDTEnt))
				iStructures++;
		}

		// Build a ratio of tanks similar to the cost of constructing the tanks. This way we won't build a bajillion infantry and only one or two other tanks.
		size_t iRatioTotal = CMechInfantry::InfantryFleetPoints();
		if (CanBuildTankLoaders())
			iRatioTotal += CMainBattleTank::MainTankFleetPoints();
		if (CanBuildArtilleryLoaders())
			iRatioTotal += CArtillery::ArtilleryFleetPoints();

		float flInfantryRatio = (float)iRatioTotal/CMechInfantry::InfantryFleetPoints();
		float flMainTankRatio = (float)iRatioTotal/CMainBattleTank::MainTankFleetPoints();
		float flArtilleryRatio = (float)iRatioTotal/CArtillery::ArtilleryFleetPoints();
		float flRatioTotal = flInfantryRatio;
		if (CanBuildTankLoaders())
			flRatioTotal += flMainTankRatio;
		if (CanBuildArtilleryLoaders())
			flRatioTotal += flArtilleryRatio;

		size_t iTotalFleetPoints = GetTotalFleetPoints();
		if (!iTotalFleetPoints)
			iTotalFleetPoints++;

		float flInfantryFleetRatio = ((float)iInfantry+1)/iTotalFleetPoints;
		float flBuildInfantryRatio = flInfantryRatio/flRatioTotal;

		float flTankFleetRatio = ((float)iMainTanks+1)/iTotalFleetPoints;
		float flBuildTankRatio = flMainTankRatio/flRatioTotal;

		float flArtilleryFleetRatio = ((float)iArtillery+1)/iTotalFleetPoints;
		float flBuildArtilleryRatio = flArtilleryRatio/flRatioTotal;

		// Collectors are first priority
		for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
			if (!pEntity)
				continue;

			CDigitanksEntity* pDTEnt = dynamic_cast<CDigitanksEntity*>(pEntity);
			if (!pDTEnt)
				continue;

			if (pDTEnt->GetUnitType() == STRUCTURE_ELECTRONODE)
			{
				CResourceNode* pElectronode = static_cast<CResourceNode*>(pDTEnt);
				if (pElectronode->HasCollector())
					continue;

				if (CSupplier::GetDataFlow(pElectronode->GetGlobalOrigin(), this) <= 0)
					continue;

				Bot_AddBuildPriority(CanBuildPSUs()?STRUCTURE_PSU:STRUCTURE_BATTERY, pElectronode);
			}
		}

		if (!m_hInfantryLoader)
			Bot_AddBuildPriority(STRUCTURE_INFANTRYLOADER);

		if (!m_hTankLoader && CanBuildTankLoaders())
			Bot_AddBuildPriority(STRUCTURE_TANKLOADER);

		if (!m_hArtilleryLoader && CanBuildArtilleryLoaders())
			Bot_AddBuildPriority(STRUCTURE_ARTILLERYLOADER);

		if (m_ahAttackTeam.size() && ((float)iAutoTurrets/(float)iStructures) < 0.6f)
			Bot_AddBuildPriority(STRUCTURE_FIREWALL);

		if (m_ahAttackTeam.size() && m_bCanUpgrade && RandomFloat(0, 1) > 0.7f)
		{
			for (size_t i = 0; i < ahBatteryUpgrades.size(); i++)
			{
				Bot_AddBuildPriority(STRUCTURE_PSU, ahBatteryUpgrades[i]);
				break;
			}

			for (size_t i = 0; i < ahMinibufferUpgrades.size(); i++)
			{
				Bot_AddBuildPriority(STRUCTURE_BUFFER, ahMinibufferUpgrades[i]);
				break;
			}
		}
		m_bCanUpgrade = true;

		if (m_ahAttackTeam.size() && iScouts < 2 && DigitanksGame()->GetTurn() > m_iLastScoutBuilt + 5)
			Bot_AddBuildPriority(UNIT_SCOUT);

		bool bBuildInfantry = m_hInfantryLoader != NULL && !m_hInfantryLoader->IsProducing() && !m_hInfantryLoader->IsConstructing() && flInfantryFleetRatio < flBuildInfantryRatio && GetUnusedFleetPoints() >= CMechInfantry::InfantryFleetPoints();
		bool bBuildTank = m_hTankLoader != NULL && !m_hTankLoader->IsProducing() && !m_hTankLoader->IsConstructing() && flTankFleetRatio < flBuildTankRatio && GetUnusedFleetPoints() >= CMainBattleTank::MainTankFleetPoints();
		bool bBuildArtillery = m_hArtilleryLoader != NULL && !m_hArtilleryLoader->IsProducing() && !m_hArtilleryLoader->IsConstructing() && flArtilleryFleetRatio < flBuildArtilleryRatio && GetUnusedFleetPoints() >= CArtillery::ArtilleryFleetPoints();

		// If we have no attack team don't worry about artillery.
		if (!m_ahAttackTeam.size())
			bBuildArtillery = false;

		// If we are on the warpath then reduce unit priority in favor of falling through to buffers, which means expansion.
		if (m_ahAttackTeam.size() && RandomFloat(0, 1) < 0.5f)
			bBuildInfantry = bBuildTank = false;

		if (bBuildInfantry)
		{
			if (bBuildTank || bBuildArtillery)
			{
				if (flTankFleetRatio < flInfantryFleetRatio || flArtilleryFleetRatio < flInfantryFleetRatio)
				{
					// If other tanks need to be built more than infantry, then don't be a hog.
					if (RandomFloat(0, 1) > 0.7f)
						Bot_AddBuildPriority(UNIT_INFANTRY);
				}
				// If infantry is needed more than other tanks, still don't be a hog sometimes.
				else if (RandomFloat(0, 1) > 0.3f)
					Bot_AddBuildPriority(UNIT_INFANTRY);
			}
			else
				Bot_AddBuildPriority(UNIT_INFANTRY);
		}

		if (bBuildTank)
		{
			if (bBuildArtillery)
			{
				if (flArtilleryFleetRatio < flTankFleetRatio)
				{
					if (RandomFloat(0, 1) > 0.7f)
						Bot_AddBuildPriority(UNIT_TANK);
				}
				else if (RandomFloat(0, 1) > 0.5f)
					Bot_AddBuildPriority(UNIT_TANK);
			}
			else
				Bot_AddBuildPriority(UNIT_TANK);
		}

		if (bBuildArtillery)
			Bot_AddBuildPriority(UNIT_ARTILLERY);

		Bot_AddBuildPriority(CanBuildBuffers()?STRUCTURE_BUFFER:STRUCTURE_MINIBUFFER);

		Bot_DownloadUpdates();

		bool bSuccess;
		do
		{
			bSuccess = Bot_BuildFirstPriority();
			if (bSuccess)
				m_iBuildPrioritiesHead++;
		} while (bSuccess);

		Bot_AssignDefenders();
	}

	CDigitank* pHeadTank = NULL;

	if (!m_ahAttackTeam.size())
	{
		// Examine whether it's time to attack.

		CountFleetPoints();

		if (GetTotalFleetPoints() > 0 && (m_iFleetPointAttackQuota == ~0 || GetTotalFleetPoints() <= m_iFleetPointAttackQuota))
		{
			// If our total fleet points is lower than the quota than perhaps we lost some buffers, let's make a new quota.
			m_iFleetPointAttackQuota = GetTotalFleetPoints()*2/3;
		}

		// We have enough tanks made that we should attack now.
		if (GetUsedFleetPoints() >= m_iFleetPointAttackQuota)
		{
			for (size_t i = 0; i < GetNumTanks(); i++)
			{
				CDigitank* pTank = GetTank(i);
				if (!pTank)
					continue;

				// Scouts are just for exploring and hassling.
				if (pTank->GetUnitType() == UNIT_SCOUT)
					continue;

				// Artillery is just for barrages
				if (pTank->GetUnitType() == UNIT_ARTILLERY)
					continue;

				// Just in case.
				if (pTank->GetUnitType() == UNIT_MOBILECPU || pTank->GetUnitType() == UNIT_BUGTURRET)
					continue;

				// Gotta leave some infantry behind to support the cause
				if (pTank->GetUnitType() == UNIT_INFANTRY && RandomInt(0, 1) == 0)
					continue;

				m_ahAttackTeam.push_back(pTank);
				pTank->SetInAttackTeam(true);
			}

			m_iFleetPointAttackQuota = ~0;
		}
	}

	if (m_ahAttackTeam.size())
		pHeadTank = m_ahAttackTeam[0];
	else
	{
		for (size_t i = 0; i < GetNumTanks(); i++)
		{
			CDigitank* pTank = GetTank(i);
			if (!pTank)
				continue;

			if (pTank->IsScout())
				continue;

			if (pTank->IsArtillery())
				continue;

			pHeadTank = pTank;
			break;
		}
	}

	// If there's no regular tanks we'll accept an artillery or scout
	if (!pHeadTank)
		pHeadTank = GetTank(0);

	if (!pHeadTank)
	{
		DigitanksGame()->EndTurn();
		return;
	}

	CDigitanksEntity* pTarget = NULL;

	// Find the nearest enemy to the head tank, he's our target.
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);
		if (!pDTEntity)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(pDTEntity);
		CStructure* pStructure = dynamic_cast<CStructure*>(pDTEntity);
		if (!pTank && !pStructure)
			continue;

		if (pEntity->GetTeam() == pHeadTank->GetTeam())
			continue;

		// Don't fire on neutral structures.
		if (pEntity->GetTeam() == NULL)
			continue;

		// Don't make targets out of barbarians or we'll spend the whole game fighting them.
		if (pDTEntity->GetOwner() == Game()->GetPlayer(0))
			continue;

		if (GetEntityVisibility(pEntity->GetHandle()) == 0)
			continue;

		if (pDTEntity->GetUnitType() == UNIT_SCOUT)
			continue;

		float flTargetVisibility = pDTEntity->GetVisibility(this);
		if (flTargetVisibility < 1 && RandomFloat(0, 1) > flTargetVisibility)
			continue;

		if (!pTarget)
		{
			pTarget = pDTEntity;
			continue;
		}

		if ((pHeadTank->GetGlobalOrigin() - pDTEntity->GetGlobalOrigin()).Length2DSqr() < (pHeadTank->GetGlobalOrigin() - pTarget->GetGlobalOrigin()).Length2DSqr())
			pTarget = pDTEntity;
	}

	Vector vecTargetOrigin;
	if (pTarget)
	{
		vecTargetOrigin = pTarget->GetGlobalOrigin();
		m_bLKV = true;
		m_vecLKV = vecTargetOrigin;
	}
	else if (m_bLKV)
	{
		bool bCloseToLKV = false;
		for (size_t i = 0; i < GetNumTanks(); i++)
		{
			if (GetTank(i)->GetUnitType() == UNIT_ARTILLERY)
				continue;

			if ((GetTank(i)->GetGlobalOrigin() - m_vecLKV).Length() < (GetTank(i)->GetEffRange()+GetTank(i)->GetMaxRange())/2)
			{
				bCloseToLKV = true;
				break;
			}
		}

		// Close to the LKV? Lose it, fall back to explore mode.
		if (bCloseToLKV)
			m_bLKV = false;

		vecTargetOrigin = m_vecLKV;
	}

	bool bCloseToExplorePoint = false;
	for (size_t i = 0; i < GetNumTanks(); i++)
	{
		if ((GetTank(i)->GetGlobalOrigin() - m_vecExplore).Length() < (GetTank(i)->GetEffRange()+GetTank(i)->GetMaxRange())/2)
		{
			bCloseToExplorePoint = true;
			break;
		}
	}

	if (m_vecExplore.Length() < 1 || bCloseToExplorePoint)
	{
		float flMapSize = DigitanksGame()->GetTerrain()->GetMapSize();

		do
		{
			m_vecExplore.x = RandomFloat(-flMapSize, flMapSize);
			m_vecExplore.z = RandomFloat(-flMapSize, flMapSize);
		}
		while (DigitanksGame()->GetTerrain()->IsPointOverHole(m_vecExplore));

		m_vecExplore = DigitanksGame()->GetTerrain()->GetPointHeight(m_vecExplore);
	}

	if (!pTarget && !m_bLKV)
		vecTargetOrigin = m_vecExplore;

	for (size_t i = 0; i < GetNumTanks(); i++)
	{
		CDigitank* pTank = GetTank(i);

		// Use any promotion points.
		while (pTank->HasBonusPoints())
		{
			switch (rand()%3)
			{
			case 0:
				pTank->PromoteAttack();
				break;

			case 1:
				pTank->PromoteDefense();
				break;

			case 2:
				pTank->PromoteMovement();
				break;
			}
		}

		CDigitank* pTankTarget = dynamic_cast<CDigitank*>(pTarget);

		if (pTank->IsMobileCPU())
		{
			// Move halfway to the closest available resource and put down.
			CResourceNode* pResource = CBaseEntity::FindClosest<CResourceNode>(pTank->GetGlobalOrigin(), NULL);

			if (pResource)
			{
				float flDistance = pResource->Distance(pTank->GetGlobalOrigin())/2;

				float flMovementDistance = pTank->GetRemainingMovementDistance() * 0.9f;

				if (flMovementDistance < flDistance)
					flDistance = flMovementDistance;

				Vector vecDirection = pResource->GetGlobalOrigin() - pTank->GetGlobalOrigin();
				vecDirection = vecDirection.Normalized() * flDistance;

				Vector vecDesiredMove = pTank->GetGlobalOrigin() + vecDirection;
				vecDesiredMove.z = pTank->FindHoverHeight(vecDesiredMove);

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->Move();
			}

			pTank->Fortify();
		}
		else if (pTank->IsScout())
		{
			// We HATE infantry, so always know where the closest one is.
			CMechInfantry* pClosestInfantry = NULL;
			while (true)
			{
				pClosestInfantry = CBaseEntity::FindClosest<CMechInfantry>(pTank->GetGlobalOrigin(), pClosestInfantry);

				if (!pClosestInfantry)
					break;

				if (pClosestInfantry->GetTeam() == pTank->GetTeam())
					continue;

				if (!pClosestInfantry->IsInsideMaxRange(pTank->GetGlobalOrigin()))
				{
					pClosestInfantry = NULL;
					break;
				}

				break;
			}

			CDigitank* pClosestEnemy = NULL;

			CSupplyLine* pClosestSupply = NULL;
			while (true)
			{
				pClosestSupply = CBaseEntity::FindClosest<CSupplyLine>(pTank->GetGlobalOrigin(), pClosestSupply);

				if (!pClosestSupply)
					break;

				if (pClosestSupply->GetTeam() == pTank->GetTeam())
					continue;

				if (!pClosestSupply->GetTeam())
					continue;

				if (!pClosestSupply->GetSupplier() || !pClosestSupply->GetEntity())
					continue;

				// Who cares about scouts
				bool bSupplyIsForScout = dynamic_cast<CDigitank*>(pClosestSupply->GetEntity()) && dynamic_cast<CDigitank*>(pClosestSupply->GetEntity())->IsScout();
				if (bSupplyIsForScout)
					continue;

				// Don't fuck with infantry
				bool bSupplyIsForInfantry = dynamic_cast<CDigitank*>(pClosestSupply->GetEntity()) && dynamic_cast<CDigitank*>(pClosestSupply->GetEntity())->IsInfantry();
				if (bSupplyIsForInfantry)
					continue;

				if (pClosestSupply->GetIntegrity() < 0.3f)
					continue;

				if (pClosestSupply->Distance(pTank->GetGlobalOrigin()) > pTank->VisibleRange())
				{
					pClosestSupply = NULL;
					break;
				}

				// This one will do.
				break;
			}

			float flMovementPower = 0.69f;

			Vector vecPoint;
			if (pClosestSupply)
				DistanceToLineSegment(pTank->GetGlobalOrigin(), pClosestSupply->GetEntity()->GetGlobalOrigin(), pClosestSupply->GetSupplier()->GetGlobalOrigin(), &vecPoint);

			// Bomb it until it's below 1/3 and then our job is done, move to the next one.
			if (pClosestSupply && pTank->IsInsideMaxRange(vecPoint))
			{
				// FIRE ZE MISSILES
				pTank->SetPreviewAim(vecPoint);
				pTank->Fire();
				flMovementPower = 0.95f;
			}
			else
			{
				// Otherwise look for the closest enemy and fire on them.
				while (true)
				{
					pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(pTank->GetGlobalOrigin(), pClosestEnemy);

					if (!pClosestEnemy)
						break;

					if (pClosestEnemy->IsScout())
						continue;

					if (pClosestEnemy->GetTeam() == pTank->GetTeam())
						continue;

					if (!pTank->IsInsideMaxRange(pClosestEnemy->GetGlobalOrigin()))
					{
						pClosestEnemy = NULL;
						break;
					}

					break;
				}

				if (pClosestEnemy)
				{
					// If we are within the max range, try to fire.
					if (pTank->IsInsideMaxRange(pClosestEnemy->GetGlobalOrigin()))
					{
						pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->GetPointHeight(pClosestEnemy->GetGlobalOrigin()));
						pTank->Fire();
					}
				}
			}

			Vector vecDesiredMove;
			bool bMove = false;
			if (pClosestInfantry)
			{
				// Scouts hate infantry! Move directly away from them.
				float flMovementDistance = pTank->GetRemainingMovementDistance();
				Vector vecDirection = pTank->GetGlobalOrigin() - pClosestInfantry->GetGlobalOrigin();
				vecDirection = vecDirection.Normalized() * (flMovementDistance*flMovementPower);

				vecDesiredMove = pTank->GetGlobalOrigin() + vecDirection;
				vecDesiredMove.z = pTank->FindHoverHeight(vecDesiredMove);
				bMove = true;
			}
			else if (pClosestSupply)
			{
				if (!pTank->IsInsideMaxRange(pClosestSupply->GetGlobalOrigin()))
				{
					if (!pTank->HasFiredWeapon())
						flMovementPower = 0.69f;

					float flMovementDistance = pTank->GetRemainingMovementDistance();
					Vector vecDirection = vecPoint - pTank->GetGlobalOrigin();
					vecDirection = vecDirection.Normalized() * (flMovementDistance*flMovementPower);

					vecDesiredMove = pTank->GetGlobalOrigin() + vecDirection;
					vecDesiredMove.z = pTank->FindHoverHeight(vecDesiredMove);
					bMove = true;
				}

				// Otherwise just wait for it to get bombed enough.
			}
			else
			{
				float flMovementDistance = pTank->GetRemainingMovementDistance();

				vecDesiredMove = DigitanksGame()->GetTerrain()->FindPath(pTank->GetGlobalOrigin(), m_vecExplore, pTank);
				bMove = true;
			}

			if (bMove)
			{
				pTank->SetPreviewMove(vecDesiredMove);
				pTank->Move();
			}

			if (pClosestSupply)
				DistanceToLineSegment(pTank->GetGlobalOrigin(), pClosestSupply->GetEntity()->GetGlobalOrigin(), pClosestSupply->GetSupplier()->GetGlobalOrigin(), &vecPoint);

			// Maybe now that we've moved closer we can try to fire again.
			if (pClosestSupply && pClosestSupply->GetIntegrity() > 0.3f && pTank->IsInsideMaxRange(vecPoint))
			{
				// FIRE ZE MISSILES
				pTank->SetPreviewAim(vecPoint);
				pTank->Fire();
			}
			else if (pClosestEnemy)
			{
				// If we are within the max range, try to fire.
				if (pTank->IsInsideMaxRange(pClosestEnemy->GetGlobalOrigin()))
				{
					pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->GetPointHeight(pClosestEnemy->GetGlobalOrigin()));
					pTank->Fire();
				}
			}
		}
		else if (pTank->IsInAttackTeam())
		{
			if (pTank->IsFortified() || pTank->IsFortifying())
			{
				// We were defending but now we're attacking. Un-fortify!
				pTank->Fortify();
				pTank->RemoveFortifyPoint();
			}

			// WE WILL ATTACK ALL NIGHT, AND WE WILL ATTACK THROUGH THE MORNING, AND IF WE ARE NOT VICTORIOUS THEN LET NO MAN COME BACK ALIVE
			// I've been watching too much of the movie Patton. It's research!
			if ((vecTargetOrigin - pTank->GetGlobalOrigin()).LengthSqr() > pTank->GetEffRange()*pTank->GetEffRange())
			{
				Vector vecDesiredMove = DigitanksGame()->GetTerrain()->FindPath(pTank->GetGlobalOrigin(), vecTargetOrigin + Vector(RandomFloat(-4, 4), 0, RandomFloat(-4, 4)), pTank);

				if (pTank->GetUnitType() == UNIT_INFANTRY)
				{
					Vector vecMove = vecDesiredMove - pTank->GetGlobalOrigin();

					// Don't let resistors get ahead of their tank complements.
					vecMove *= 0.7f;

					vecDesiredMove = pTank->GetGlobalOrigin() + vecMove;
				}

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->Move();
			}

			// If we are within the max range, try to fire.
			if (pTarget && pTank->IsInsideMaxRange(vecTargetOrigin))
			{
				if (pTank->IsInfantry())
				{
					if (pTarget->GetUnitType() == UNIT_SCOUT)
						pTank->SetCurrentWeapon(WEAPON_INFANTRYLASER, false);
					else
						pTank->SetCurrentWeapon(PROJECTILE_FLAK, false);
				}

				pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->GetPointHeight(vecTargetOrigin));
				pTank->Fire();
			}
			else
			{
				// Otherwise look for the closest enemy and fire on them.
				CDigitank* pClosestEnemy = NULL;
				while (true)
				{
					pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(pTank->GetGlobalOrigin(), pClosestEnemy);

					if (!pClosestEnemy)
						break;

					if (pClosestEnemy->GetTeam() == pTank->GetTeam())
						continue;

					if (!pTank->IsInsideMaxRange(pClosestEnemy->GetGlobalOrigin()))
					{
						pClosestEnemy = NULL;
						break;
					}

					break;
				}

				if (pClosestEnemy)
				{
					// If we are within the max range, try to fire.
					if (pTank->IsInsideMaxRange(pClosestEnemy->GetGlobalOrigin()))
					{
						if (pTank->IsInfantry())
						{
							if (pClosestEnemy->GetUnitType() == UNIT_SCOUT)
								pTank->SetCurrentWeapon(WEAPON_INFANTRYLASER, false);
							else
								pTank->SetCurrentWeapon(PROJECTILE_FLAK, false);
						}

						pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->GetPointHeight(pClosestEnemy->GetGlobalOrigin()));
						pTank->Fire();
					}
				}
			}
		}
		else if (pTank->HasFortifyPoint() && !pTank->IsFortified())
		{
			if (!pTank->IsFortified() && (pTank->GetGlobalOrigin() - pTank->GetFortifyPoint()).Length2D() < pTank->GetBoundingRadius()*2)
			{
				CCPU* pDefend = m_hPrimaryCPU;
				if (pDefend)
				{
					pTank->SetPreviewTurn(VectorAngles(pTank->GetGlobalOrigin() - pDefend->GetGlobalOrigin()).y);
					pTank->Turn();
				}

				pTank->Fortify();
			}
			else
			{
				// Head to the fortify point
				Vector vecDesiredMove = DigitanksGame()->GetTerrain()->FindPath(pTank->GetGlobalOrigin(), pTank->GetFortifyPoint(), pTank);

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->Move();

				pTank->SetPreviewTurn(VectorAngles(vecTargetOrigin - pTank->GetGlobalOrigin()).y);
				pTank->Turn();
			}
		}
		else if (pTank->IsArtillery())
		{
			// We HATE infantry, so always know where the closest one is.
			CDigitank* pClosestEnemy = NULL;
			while (true)
			{
				pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(pTank->GetGlobalOrigin(), pClosestEnemy);

				if (!pClosestEnemy)
					break;

				if (pClosestEnemy->GetTeam() == pTank->GetTeam())
					continue;

				if (pClosestEnemy->GetUnitType() == UNIT_ARTILLERY)
					continue;

				float flTargetVisibility = pClosestEnemy->GetVisibility(this);
				if (flTargetVisibility < 0.4f)
					continue;

				if (flTargetVisibility < 1 && RandomFloat(0, 1) > flTargetVisibility)
					continue;

				if (!pClosestEnemy->IsInsideMaxRange(pTank->GetGlobalOrigin()))
				{
					pClosestEnemy = NULL;
					break;
				}

				break;
			}

			if (pClosestEnemy)
			{
				if (pTank->IsFortified())
					pTank->Fortify();

				// Head away from enemies at full speed
				float flMovementDistance = pTank->GetRemainingMovementDistance();
				Vector vecDirection = vecTargetOrigin - pTank->GetGlobalOrigin();
				vecDirection = -vecDirection.Normalized() * (flMovementDistance*0.90f);

				Vector vecDesiredMove = pTank->GetGlobalOrigin() + vecDirection;
				vecDesiredMove.z = pTank->FindHoverHeight(vecDesiredMove);

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->Move();
			}
			else
			{
				if (pTarget)
				{
					float flDistanceToTarget = (pTank->GetGlobalOrigin() - pTarget->GetGlobalOrigin()).Length();

					if (flDistanceToTarget > pTank->GetMinRange() && flDistanceToTarget < pTank->GetMaxRange())
					{
						if (pTank->IsFortified())
						{
							pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->GetPointHeight(vecTargetOrigin));
							pTank->Fire();
						}
						else
						{
							// Deploy so we can rain some hell down.
							pTank->SetPreviewTurn(VectorAngles(vecTargetOrigin - pTank->GetGlobalOrigin()).y);
							pTank->Turn();
							pTank->Fortify();
						}
					}
					else if (flDistanceToTarget > pTank->GetMaxRange())
					{
						if (pTank->IsFortified())
							pTank->Fortify();

						// Head towards the target
						float flMovementDistance = pTank->GetRemainingMovementDistance();
						Vector vecDirection = pTarget->GetGlobalOrigin() - pTank->GetGlobalOrigin();
						vecDirection = vecDirection.Normalized() * (flMovementDistance*0.90f);

						Vector vecDesiredMove = pTank->GetGlobalOrigin() + vecDirection;
						vecDesiredMove.z = pTank->FindHoverHeight(vecDesiredMove);

						pTank->SetPreviewMove(vecDesiredMove);
						pTank->Move();
					}
				}
				// Otherwise hang around and wait for a target.
			}
		}
		else if (pTank->CanFortify())
		{
			// If the fortify point has moved, we must move.
			if (pTank->IsFortified() && (pTank->GetGlobalOrigin() - pTank->GetFortifyPoint()).Length2D() > pTank->GetRemainingMovementDistance()*2)
				pTank->Fortify();
		}
		else
		{
			// Hang out at spawn. We're probably waiting to be put in an attack team.

			CDigitank* pClosestEnemy = NULL;
			while (true)
			{
				pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(pTank->GetGlobalOrigin(), pClosestEnemy);

				if (!pClosestEnemy)
					break;

				if (pClosestEnemy->GetTeam() == pTank->GetTeam())
					continue;

				if (pClosestEnemy->IsImprisoned())
					continue;

				float flTargetVisibility = pClosestEnemy->GetVisibility(this);
				if (flTargetVisibility < 0.4f)
					continue;

				if (flTargetVisibility < 1 && RandomFloat(0, 1) > flTargetVisibility)
					continue;

				if (!pTank->IsInsideMaxRange(pClosestEnemy->GetGlobalOrigin()))
				{
					if (pClosestEnemy->Distance(pTank->GetGlobalOrigin()) < pTank->VisibleRange()*1.5f)
						break;

					pClosestEnemy = NULL;
					break;
				}

				break;
			}

			if (pClosestEnemy)
			{
				if (pClosestEnemy->Distance(pTank->GetGlobalOrigin()) < pTank->VisibleRange())
				{
					// Jesus we were just hanging out at the base and this asshole came up and started shooting us!
					if ((pClosestEnemy->GetGlobalOrigin() - pTank->GetGlobalOrigin()).LengthSqr() > pTank->GetEffRange()*pTank->GetEffRange())
					{
						Vector vecDesiredMove = pTank->GetGlobalOrigin() + (pClosestEnemy->GetGlobalOrigin() - pTank->GetGlobalOrigin()).Normalized() * (pTank->GetRemainingMovementDistance() * 0.6f);

						pTank->SetPreviewMove(vecDesiredMove);
						pTank->Move();
					}

					// If we are within the max range, try to fire.
					if (pTank->IsInsideMaxRange(pClosestEnemy->GetGlobalOrigin()))
					{
						pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->GetPointHeight(pClosestEnemy->GetGlobalOrigin()));
						pTank->Fire();
					}
				}
			}
		}

		if (pTank->IsInfantry())
		{
			if (pTarget && pTank->IsInsideMaxRange(vecTargetOrigin))
			{
				if (pTarget->GetUnitType() == UNIT_SCOUT)
					pTank->SetCurrentWeapon(WEAPON_INFANTRYLASER, false);
				else
					pTank->SetCurrentWeapon(PROJECTILE_FLAK, false);

				pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->GetPointHeight(vecTargetOrigin));
				pTank->Fire();
			}
			else
			{
				CDigitank* pClosestEnemy = NULL;

				// Otherwise look for the closest enemy and fire on them.
				while (true)
				{
					pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(pTank->GetGlobalOrigin(), pClosestEnemy);

					if (!pClosestEnemy)
						break;

					if (pClosestEnemy->GetTeam() == pTank->GetTeam())
						continue;

					if (pClosestEnemy->IsImprisoned())
						continue;

					float flTargetVisibility = pClosestEnemy->GetVisibility(this);
					if (flTargetVisibility < 0.4f)
						continue;

					if (flTargetVisibility < 1 && RandomFloat(0, 1) > flTargetVisibility)
						continue;

					if (!pTank->IsInsideMaxRange(pClosestEnemy->GetGlobalOrigin()))
					{
						pClosestEnemy = NULL;
						break;
					}

					break;
				}

				if (pClosestEnemy)
				{
					if (pClosestEnemy->GetUnitType() == UNIT_SCOUT)
						pTank->SetCurrentWeapon(WEAPON_INFANTRYLASER, false);
					else
						pTank->SetCurrentWeapon(PROJECTILE_FLAK, false);

					pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->GetPointHeight(pClosestEnemy->GetGlobalOrigin()));
					pTank->Fire();
				}
			}
		}
	}

	DigitanksGame()->EndTurn();
}

void CDigitanksPlayer::Bot_ExecuteTurnArtillery()
{
	// Find the nearest enemy to the head tank, he's our target.
	for (size_t i = 0; i < GetNumTanks(); i++)
	{
		CDigitank* pTank = GetTank(i);

		// Use any promotion points.
		while (pTank->HasBonusPoints())
		{
			switch (rand()%3)
			{
			case 0:
				pTank->PromoteAttack();
				break;

			case 1:
				pTank->PromoteDefense();
				break;

			case 2:
				pTank->PromoteMovement();
				break;
			}
		}

		tvector<CBaseEntity*> apTargets;

		CDigitanksEntity* pClosestEnemy = NULL;

		for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
			if (!pEntity)
				continue;

			if (!dynamic_cast<CDigitank*>(pEntity) && !dynamic_cast<CStructure*>(pEntity))
				continue;

			if (pEntity->GetTeam() == pTank->GetTeam())
				continue;

			// Don't fire on neutral structures.
			if (pEntity->GetTeam() == NULL)
				continue;

			if (!pClosestEnemy)
				pClosestEnemy = dynamic_cast<CDigitanksEntity*>(pEntity);
			else if (pClosestEnemy->Distance(pTank->GetGlobalOrigin()) > pEntity->Distance(pTank->GetGlobalOrigin()))
				pClosestEnemy = dynamic_cast<CDigitanksEntity*>(pEntity);

			if (!pTank->IsInsideMaxRange(pEntity->GetGlobalOrigin()))
				continue;

			float flTargetVisibility = pTank->GetVisibility(this);
			if (flTargetVisibility < 1 && RandomFloat(0, 1) > flTargetVisibility)
				continue;

			apTargets.push_back(pEntity);
		}

		if (pClosestEnemy && pTank->CanCharge() && pClosestEnemy->Distance(pTank->GetGlobalOrigin()) < pTank->ChargeRadius() && RandomInt(0, 3) == 0)
		{
			pTank->SetPreviewCharge(pClosestEnemy);
			pTank->Charge();
			continue;
		}

		CBaseEntity* pTarget = NULL;
		if (apTargets.size())
			pTarget = apTargets[RandomInt(0, apTargets.size()-1)];

		if (!pTarget)
		{
			if (pClosestEnemy)
			{
				float flMovementDistance = pTank->GetRemainingMovementDistance();
				Vector vecDirection = pClosestEnemy->GetGlobalOrigin() - pTank->GetGlobalOrigin();

				int iTries = 0;
				do
				{
					vecDirection = vecDirection.Normalized() * (flMovementDistance*4/5);

					Vector vecDesiredMove = pTank->GetGlobalOrigin() + vecDirection;
					vecDesiredMove.z = pTank->FindHoverHeight(vecDesiredMove);

					pTank->SetPreviewMove(vecDesiredMove);

					flMovementDistance *= 0.95f;

					if (pTank->IsPreviewMoveValid())
						break;
				} while (iTries++ < 50);

				pTank->Move();
			}
			continue;
		}

		CDigitank* pTankTarget = dynamic_cast<CDigitank*>(pTarget);

		weapon_t eWeapon = WEAPON_NONE;
		while (eWeapon == WEAPON_NONE || eWeapon == PROJECTILE_CAMERAGUIDED || eWeapon == WEAPON_CHARGERAM)
			eWeapon = pTank->GetWeapon(RandomInt(0, pTank->GetNumWeapons()-1));

		pTank->SetCurrentWeapon(eWeapon, false);

		// If we are not within the effective range, use some of our available movement power to move towards our target.
		if ((pTarget->GetGlobalOrigin() - pTank->GetGlobalOrigin()).LengthSqr() > pTank->GetEffRange()*pTank->GetEffRange())
		{
			float flMovementDistance = pTank->GetRemainingMovementDistance();
			Vector vecDirection = pTarget->GetGlobalOrigin() - pTank->GetGlobalOrigin();
			vecDirection = vecDirection.Normalized() * (flMovementDistance*2/3);

			Vector vecDesiredMove = pTank->GetGlobalOrigin() + vecDirection;
			vecDesiredMove.z = pTank->FindHoverHeight(vecDesiredMove);

			pTank->SetPreviewMove(vecDesiredMove);
			pTank->Move();
		}

		// If we are within the max range, try to fire.
		if (pTarget && pTank->IsInsideMaxRange(pTarget->GetGlobalOrigin()))
		{
			pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->GetPointHeight(pTarget->GetGlobalOrigin()));
			pTank->Fire();
		}
	}

	DigitanksGame()->EndTurn();
}

CSupplier* CDigitanksPlayer::Bot_FindUnusedSupplier(size_t iMaxDependents, bool bNoSuppliers)
{
	tvector<CSupplier*> apSuppliers;

	// Find an appropriate supplier to build from.
	for (size_t i = 0; i < GetNumUnits(); i++)
	{
		CDigitanksEntity* pEntity = GetUnit(i);
		if (!pEntity)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (iMaxDependents != ~0)
		{
			size_t iDependents = 0;
			for (size_t j = 0; j < pSupplier->GetNumChildren(); j++)
			{
				CStructure* pChild = pSupplier->GetChild(j);
				if (!pChild)
					continue;

				if (bNoSuppliers && dynamic_cast<CSupplier*>(pChild))
					continue;

				iDependents++;
			}

			if (iDependents >= iMaxDependents)
				continue;
		}

		apSuppliers.push_back(pSupplier);
	}

	if (!apSuppliers.size())
		return NULL;

	return apSuppliers[rand()%apSuppliers.size()];
}

bool CDigitanksPlayer::Bot_BuildCollector(CResourceNode* pResource)
{
	if (!pResource)
		return false;

	if (CSupplier::GetDataFlow(pResource->GetGlobalOrigin(), this) < 1)
		return false;

	Vector vecPSU = DigitanksGame()->GetTerrain()->GetPointHeight(pResource->GetGlobalOrigin());

	m_hPrimaryCPU->SetPreviewStructure(CanBuildPSUs()?STRUCTURE_PSU:STRUCTURE_BATTERY);
	m_hPrimaryCPU->SetPreviewBuild(vecPSU);

	if (!m_hPrimaryCPU->IsPreviewBuildValid())
		return false;

	return m_hPrimaryCPU->BeginConstruction();
}

void CDigitanksPlayer::Bot_AddBuildPriority(unittype_t eUnit, CDigitanksEntity* pTarget)
{
	builditem_t eBuildItem;
	eBuildItem.m_eUnit = eUnit;
	eBuildItem.m_hTarget = pTarget;

	m_aeBuildPriorities.push_back(eBuildItem);
}

void CStructure::AddDefender(CDigitank* pTank)
{
	CTerrain* pTerrain = DigitanksGame()->GetTerrain();

	for (size_t i = 0; i < m_aoDefenders.size(); i++)
	{
		if (!m_aoDefenders[i].m_hDefender)
		{
			m_aoDefenders[i].m_hDefender = pTank;

			Vector vecFortify = GetGlobalOrigin() + AngleVector(EAngle(0, m_aoDefenders[i].m_flPosition, 0)) * 20;
			vecFortify = pTerrain->GetPointHeight(vecFortify);

			if (pTerrain->IsPointOverHole(vecFortify) || pTerrain->IsPointOverWater(vecFortify) || pTerrain->IsPointOverLava(vecFortify))
				continue;

			pTank->SetFortifyPoint(this, vecFortify);
			return;
		}
	}

	// Member 0 is typically the CPU.
	float flYaw;
	if (dynamic_cast<CCPU*>(this))
		flYaw = VectorAngles(-GetGlobalOrigin()).y;
	else
		flYaw = VectorAngles(GetGlobalOrigin() - pTank->GetDigitanksPlayer()->GetUnit(0)->GetGlobalOrigin()).y;

	size_t iFortifies = m_aoDefenders.size();

	Vector vecFortify;
	int iTries = 0;
	bool bFound = false;
	while (iTries++ < 6)
	{
		if (iFortifies == 0)
		{
			// Default value
		}
		else if (iFortifies%2 == 0)
			flYaw += 45*iFortifies/2;
		else
			flYaw -= 45*(iFortifies/2+1);

		vecFortify = GetGlobalOrigin() + AngleVector(EAngle(0, flYaw, 0)) * 20;
		vecFortify = DigitanksGame()->GetTerrain()->GetPointHeight(vecFortify);

		iFortifies++;

		if (pTerrain->IsPointOverHole(vecFortify) || pTerrain->IsPointOverWater(vecFortify) || pTerrain->IsPointOverLava(vecFortify))
			continue;

		bFound = true;
		break;
	}

	if (!bFound)
		return;

	vecFortify = DigitanksGame()->GetTerrain()->GetPointHeight(vecFortify);

	m_aoDefenders.push_back(defender_t());
	defender_t* pDefender = &m_aoDefenders[m_aoDefenders.size()-1];
	pDefender->m_flPosition = flYaw;
	pDefender->m_hDefender = pTank;
	pTank->SetFortifyPoint(this, vecFortify);
}

void CStructure::RemoveDefender(CDigitank* pTank)
{
	for (size_t i = 0; i < m_aoDefenders.size(); i++)
	{
		if (pTank == m_aoDefenders[i].m_hDefender)
			m_aoDefenders[i].m_hDefender = NULL;
	}
}

size_t CStructure::GetNumLivingDefenders()
{
	size_t iDefenders = 0;
	for (size_t i = 0; i < m_aoDefenders.size(); i++)
	{
		if (!m_aoDefenders[i].m_hDefender)
			continue;

		if (m_aoDefenders[i].m_hDefender->IsAlive())
			iDefenders++;
	}

	return iDefenders;
}
