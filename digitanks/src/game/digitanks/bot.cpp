#include "digitanksteam.h"

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

void CDigitanksTeam::Bot_DownloadUpdates()
{
	CUpdateGrid* pGrid = DigitanksGame()->GetUpdateGrid();
	if (!pGrid)
		return;

	if (GetUpdateDownloading())
		return;

	eastl::vector<update_coordinate_t> aUpdatesAvailable;

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

bool CDigitanksTeam::Bot_BuildFirstPriority()
{
	CTerrain* pTerrain = DigitanksGame()->GetTerrain();

	if (m_hPrimaryCPU == NULL)
		return false;

	if (m_aeBuildPriorities.size() == 0)
		return false;

	builditem_t* pNextBuild = &m_aeBuildPriorities.front();

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
		if (m_hInfantryLoader == NULL)
			return false;

		m_hInfantryLoader->BeginProduction();
		return m_hInfantryLoader->IsProducing();
	}

	if (pNextBuild->m_eUnit == UNIT_TANK)
	{
		if (m_hTankLoader == NULL)
			return false;

		m_hTankLoader->BeginProduction();
		return m_hTankLoader->IsProducing();
	}

	if (pNextBuild->m_eUnit == UNIT_ARTILLERY)
	{
		if (m_hArtilleryLoader == NULL)
			return false;

		m_hArtilleryLoader->BeginProduction();
		return m_hArtilleryLoader->IsProducing();
	}

	if (pNextBuild->m_eUnit == STRUCTURE_PSU && pNextBuild->m_hTarget != NULL)
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
		return Bot_BuildCollector(dynamic_cast<CResource*>(pNextBuild->m_hTarget.GetPointer()));
	}

	if (pNextBuild->m_eUnit == STRUCTURE_BUFFER || pNextBuild->m_eUnit == STRUCTURE_MINIBUFFER)
	{
		CResource* pTargetResource = NULL;
		CSupplier* pClosestSupplier = NULL;

		while (true)
		{
			pTargetResource = CBaseEntity::FindClosest<CResource>(m_hPrimaryCPU->GetOrigin(), pTargetResource);

			if (!pTargetResource)
				break;

			if (pTargetResource->HasCollector())
				continue;

			if (GetVisibilityAtPoint(pTargetResource->GetOrigin()) < 0.2f)
				continue;

			break;
		}

		if (pTargetResource)
		{
			while (true)
			{
				pClosestSupplier = CBaseEntity::FindClosest<CSupplier>(pTargetResource->GetOrigin(), pClosestSupplier);

				if (!pClosestSupplier)
					break;

				if (pClosestSupplier->GetDigitanksTeam() != this)
					continue;

				break;
			}
		}

		if (pTargetResource && CSupplier::GetDataFlow(pTargetResource->GetOrigin(), this) > 0)
		{
			return Bot_BuildCollector(pTargetResource);
		}
		else if (pTargetResource && pClosestSupplier)
		{
			Vector vecStructurePath = pTerrain->FindPath(pClosestSupplier->GetOrigin(), pTargetResource->GetOrigin(), NULL);

			size_t iTries = 0;
			do
			{
				// Try slightly different stuff to get around structures that may be in the way.
				Vector vecWobble(RandomFloat(-5, 5), 0, RandomFloat(-5, 5));

				Vector vecStructureDirection = (vecStructurePath + vecWobble - pClosestSupplier->GetOrigin()).Normalized();
				Vector vecStructure = pClosestSupplier->GetOrigin() + vecStructureDirection * pClosestSupplier->GetDataFlowRadius()*9/10;
				if (CSupplier::GetDataFlow(vecStructure, this) <= 0)
					vecStructure = pClosestSupplier->GetOrigin() + vecStructureDirection * pClosestSupplier->GetDataFlowRadius()*2/3;

				pTerrain->SetPointHeight(vecStructure);

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
			continue;
		}

		float flYaw;
		if (pUnused == m_hPrimaryCPU)
			flYaw = RandomFloat(0, 360);
		else
		{
			flYaw = VectorAngles(pUnused->GetOrigin() - m_hPrimaryCPU->GetOrigin()).y;
			flYaw = RandomFloat(flYaw-90, flYaw+90);
		}

		// Pick a random direction facing more or less away from the CPU so that we spread outwards.
		Vector vecStructureDirection = AngleVector(EAngle(0, flYaw, 0));
		vecStructure = pUnused->GetOrigin();
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

		pTerrain->SetPointHeight(vecStructure);

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

	m_hPrimaryCPU->SetPreviewStructure(pNextBuild->m_eUnit);
	m_hPrimaryCPU->SetPreviewBuild(vecStructure);

	if (!m_hPrimaryCPU->IsPreviewBuildValid())
		return false;

	return m_hPrimaryCPU->BeginConstruction();
}

void CDigitanksTeam::Bot_AssignDefenders()
{
	eastl::vector<CStructure*> apDefend;
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		CBaseEntity* pEntity = m_ahMembers[i];
		if (!pEntity)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);
		if (!pStructure)
			continue;

		if (pStructure != m_hPrimaryCPU)
		{
			// Don't defend structures that face the back wall.
			Vector vecStructure = pStructure->GetOrigin() - m_hPrimaryCPU->GetOrigin();
			float flDot = -m_hPrimaryCPU->GetOrigin().Normalized().Dot(vecStructure.Normalized());
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

void CDigitanksTeam::Bot_ExecuteTurn()
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

	m_aeBuildPriorities.clear();

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

		eastl::vector<CEntityHandle<CMiniBuffer> > ahMinibufferUpgrades;
		eastl::vector<CEntityHandle<CBattery> > ahBatteryUpgrades;

		for (size_t i = 0; i < GetNumMembers(); i++)
		{
			CBaseEntity* pEntity = GetMember(i);
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

			if (pDTEnt->GetUnitType() == STRUCTURE_AUTOTURRET)
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
				CResource* pElectronode = static_cast<CResource*>(pDTEnt);
				if (pElectronode->HasCollector())
					continue;

				if (CSupplier::GetDataFlow(pElectronode->GetOrigin(), this) <= 0)
					continue;

				Bot_AddBuildPriority(CanBuildPSUs()?STRUCTURE_PSU:STRUCTURE_BATTERY, pElectronode);
			}
		}

		if (m_hInfantryLoader == NULL)
			Bot_AddBuildPriority(STRUCTURE_INFANTRYLOADER);

		if (m_hTankLoader == NULL && CanBuildTankLoaders())
			Bot_AddBuildPriority(STRUCTURE_TANKLOADER);

		if (m_hArtilleryLoader == NULL && CanBuildArtilleryLoaders())
			Bot_AddBuildPriority(STRUCTURE_ARTILLERYLOADER);

		if (((float)iAutoTurrets/(float)iStructures) < 0.6f)
			Bot_AddBuildPriority(STRUCTURE_AUTOTURRET);

		if (m_bCanUpgrade)
		{
			for (size_t i = 0; i < ahBatteryUpgrades.size(); i++)
				Bot_AddBuildPriority(STRUCTURE_PSU, ahBatteryUpgrades[i]);

			for (size_t i = 0; i < ahMinibufferUpgrades.size(); i++)
				Bot_AddBuildPriority(STRUCTURE_BUFFER, ahMinibufferUpgrades[i]);
		}
		m_bCanUpgrade = true;

		if (iScouts < 2 && DigitanksGame()->GetTurn() > m_iLastScoutBuilt + 5)
			Bot_AddBuildPriority(UNIT_SCOUT);

		if (m_hInfantryLoader != NULL && !m_hInfantryLoader->IsProducing() && flInfantryFleetRatio < flBuildInfantryRatio && GetUnusedFleetPoints() >= CMechInfantry::InfantryFleetPoints())
			Bot_AddBuildPriority(UNIT_INFANTRY);

		if (m_hTankLoader != NULL && !m_hTankLoader->IsProducing() && flTankFleetRatio < flBuildTankRatio && GetUnusedFleetPoints() >= CMainBattleTank::MainTankFleetPoints())
			Bot_AddBuildPriority(UNIT_TANK);

		if (m_hArtilleryLoader != NULL && !m_hArtilleryLoader->IsProducing() && flArtilleryFleetRatio < flBuildArtilleryRatio && GetUnusedFleetPoints() >= CArtillery::ArtilleryFleetPoints())
			Bot_AddBuildPriority(UNIT_ARTILLERY);

		Bot_AddBuildPriority(CanBuildBuffers()?STRUCTURE_BUFFER:STRUCTURE_MINIBUFFER);

		Bot_DownloadUpdates();

		bool bSuccess;
		do
		{
			bSuccess = Bot_BuildFirstPriority();
			if (bSuccess)
				m_aeBuildPriorities.pop_front();
		} while (bSuccess);

		Bot_AssignDefenders();
	}

	CDigitank* pHeadTank = NULL;

	if (m_ahAttackTeam.size())
	{
		for (size_t i = m_ahAttackTeam.size()-1; i < m_ahAttackTeam.size(); i--)
		{
			if (m_ahAttackTeam[i] == NULL)
				m_ahAttackTeam.erase(m_ahAttackTeam.begin()+i);
		}
	}

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

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);
		if (!pTank && !pStructure)
			continue;

		if (pEntity->GetTeam() == pHeadTank->GetTeam())
			continue;

		// Don't fire on neutral structures.
		if (pEntity->GetTeam() == NULL)
			continue;

		// Don't make targets out of barbarians or we'll spend the whole game fighting them.
		if (pEntity->GetTeam() == Game()->GetTeam(0))
			continue;

		if (GetEntityVisibility(pEntity->GetHandle()) == 0)
			continue;

		CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);

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

		if ((pHeadTank->GetOrigin() - pDTEntity->GetOrigin()).Length2DSqr() < (pHeadTank->GetOrigin() - pTarget->GetOrigin()).Length2DSqr())
			pTarget = pDTEntity;
	}

	Vector vecTargetOrigin;
	if (pTarget)
	{
		vecTargetOrigin = pTarget->GetOrigin();
		m_bLKV = true;
		m_vecLKV = vecTargetOrigin;
	}
	else if (m_bLKV)
	{
		bool bCloseToLKV = false;
		for (size_t i = 0; i < GetNumTanks(); i++)
		{
			if ((GetTank(i)->GetOrigin() - m_vecLKV).Length() < (GetTank(i)->GetEffRange()+GetTank(i)->GetMaxRange())/2)
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
		if ((GetTank(i)->GetOrigin() - m_vecExplore).Length() < (GetTank(i)->GetEffRange()+GetTank(i)->GetMaxRange())/2)
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

		DigitanksGame()->GetTerrain()->SetPointHeight(m_vecExplore);
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
			CResource* pResource = CBaseEntity::FindClosest<CResource>(pTank->GetOrigin(), NULL);

			if (pResource)
			{
				float flDistance = pResource->Distance(pTank->GetOrigin())/2;

				float flMovementDistance = pTank->GetRemainingMovementDistance() * 0.9f;

				if (flMovementDistance < flDistance)
					flDistance = flMovementDistance;

				Vector vecDirection = pResource->GetOrigin() - pTank->GetOrigin();
				vecDirection = vecDirection.Normalized() * flDistance;

				Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
				vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

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
				pClosestInfantry = CBaseEntity::FindClosest<CMechInfantry>(pTank->GetOrigin(), pClosestInfantry);

				if (!pClosestInfantry)
					break;

				if (pClosestInfantry->GetTeam() == pTank->GetTeam())
					continue;

				if (!pClosestInfantry->IsInsideMaxRange(pTank->GetOrigin()))
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
				pClosestSupply = CBaseEntity::FindClosest<CSupplyLine>(pTank->GetOrigin(), pClosestSupply);

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

				if (pClosestSupply->Distance(pTank->GetOrigin()) > pTank->VisibleRange())
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
				DistanceToLineSegment(pTank->GetOrigin(), pClosestSupply->GetEntity()->GetOrigin(), pClosestSupply->GetSupplier()->GetOrigin(), &vecPoint);

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
					pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(pTank->GetOrigin(), pClosestEnemy);

					if (!pClosestEnemy)
						break;

					if (pClosestEnemy->IsScout())
						continue;

					if (pClosestEnemy->GetTeam() == pTank->GetTeam())
						continue;

					if (!pTank->IsInsideMaxRange(pClosestEnemy->GetOrigin()))
					{
						pClosestEnemy = NULL;
						break;
					}

					break;
				}

				if (pClosestEnemy)
				{
					// If we are within the max range, try to fire.
					if (pTank->IsInsideMaxRange(pClosestEnemy->GetOrigin()))
					{
						pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(pClosestEnemy->GetOrigin()));
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
				Vector vecDirection = pTank->GetOrigin() - pClosestInfantry->GetOrigin();
				vecDirection = vecDirection.Normalized() * (flMovementDistance*flMovementPower);

				vecDesiredMove = pTank->GetOrigin() + vecDirection;
				vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);
				bMove = true;
			}
			else if (pClosestSupply)
			{
				if (!pTank->IsInsideMaxRange(pClosestSupply->GetOrigin()))
				{
					if (!pTank->HasFiredWeapon())
						flMovementPower = 0.69f;

					float flMovementDistance = pTank->GetRemainingMovementDistance();
					Vector vecDirection = vecPoint - pTank->GetOrigin();
					vecDirection = vecDirection.Normalized() * (flMovementDistance*flMovementPower);

					vecDesiredMove = pTank->GetOrigin() + vecDirection;
					vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);
					bMove = true;
				}

				// Otherwise just wait for it to get bombed enough.
			}
			else
			{
				float flMovementDistance = pTank->GetRemainingMovementDistance();

				vecDesiredMove = DigitanksGame()->GetTerrain()->FindPath(pTank->GetOrigin(), m_vecExplore, pTank);
				bMove = true;
			}

			if (bMove)
			{
				pTank->SetPreviewMove(vecDesiredMove);
				pTank->Move();
			}

			if (pClosestSupply)
				DistanceToLineSegment(pTank->GetOrigin(), pClosestSupply->GetEntity()->GetOrigin(), pClosestSupply->GetSupplier()->GetOrigin(), &vecPoint);

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
				if (pTank->IsInsideMaxRange(pClosestEnemy->GetOrigin()))
				{
					pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(pClosestEnemy->GetOrigin()));
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
			if ((vecTargetOrigin - pTank->GetOrigin()).LengthSqr() > pTank->GetEffRange()*pTank->GetEffRange())
			{
				Vector vecDesiredMove = DigitanksGame()->GetTerrain()->FindPath(pTank->GetOrigin(), vecTargetOrigin, pTank);

				if (pTank->GetUnitType() == UNIT_INFANTRY)
				{
					Vector vecMove = vecDesiredMove - pTank->GetOrigin();

					// Don't let resistors get ahead of their tank complements.
					vecMove *= 0.7f;

					vecDesiredMove = pTank->GetOrigin() + vecMove;
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

				pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(vecTargetOrigin));
				pTank->Fire();
			}
			else
			{
				// Otherwise look for the closest enemy and fire on them.
				CDigitank* pClosestEnemy = NULL;
				while (true)
				{
					pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(pTank->GetOrigin(), pClosestEnemy);

					if (!pClosestEnemy)
						break;

					if (pClosestEnemy->GetTeam() == pTank->GetTeam())
						continue;

					if (!pTank->IsInsideMaxRange(pClosestEnemy->GetOrigin()))
					{
						pClosestEnemy = NULL;
						break;
					}

					break;
				}

				if (pClosestEnemy)
				{
					// If we are within the max range, try to fire.
					if (pTank->IsInsideMaxRange(pClosestEnemy->GetOrigin()))
					{
						if (pTank->IsInfantry())
						{
							if (pClosestEnemy->GetUnitType() == UNIT_SCOUT)
								pTank->SetCurrentWeapon(WEAPON_INFANTRYLASER, false);
							else
								pTank->SetCurrentWeapon(PROJECTILE_FLAK, false);
						}

						pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(pClosestEnemy->GetOrigin()));
						pTank->Fire();
					}
				}
			}
		}
		else if (pTank->HasFortifyPoint() && !pTank->IsFortified())
		{
			if (!pTank->IsFortified() && (pTank->GetOrigin() - pTank->GetFortifyPoint()).Length2D() < pTank->GetBoundingRadius()*2)
			{
				CCPU* pDefend = m_hPrimaryCPU;
				if (pDefend)
				{
					pTank->SetPreviewTurn(VectorAngles(pTank->GetOrigin() - pDefend->GetOrigin()).y);
					pTank->Turn();
				}

				pTank->Fortify();
			}
			else
			{
				// Head to the fortify point
				Vector vecDesiredMove = DigitanksGame()->GetTerrain()->FindPath(pTank->GetOrigin(), pTank->GetFortifyPoint(), pTank);

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->Move();

				pTank->SetPreviewTurn(VectorAngles(vecTargetOrigin - pTank->GetOrigin()).y);
				pTank->Turn();
			}
		}
		else if (pTank->IsArtillery())
		{
			// If we're fortified and our target is too close, get the fuck outta there!
			if (pTank->IsFortified() && (pTank->GetOrigin() - vecTargetOrigin).Length() < pTank->GetMinRange())
				pTank->Fortify();

			if (!pTank->IsFortified())
			{
				if ((pTank->GetOrigin() - vecTargetOrigin).Length() > pTank->GetMinRange())
				{
					// Deploy so we can rain some hell down.
					pTank->SetPreviewTurn(VectorAngles(vecTargetOrigin - pTank->GetOrigin()).y);
					pTank->Turn();
					pTank->Fortify();
				}
				else
				{
					// Head away from enemies at full speed
					float flMovementDistance = pTank->GetRemainingMovementDistance();
					Vector vecDirection = vecTargetOrigin - pTank->GetOrigin();
					vecDirection = -vecDirection.Normalized() * (flMovementDistance*0.90f);

					Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
					vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

					pTank->SetPreviewMove(vecDesiredMove);
					pTank->Move();

					pTank->SetPreviewTurn(VectorAngles(vecTargetOrigin - pTank->GetOrigin()).y);
					pTank->Turn();
				}
			}

			// If we are within the max range, try to fire.
			if (pTank->IsInsideMaxRange(vecTargetOrigin))
			{
				pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(vecTargetOrigin));
				pTank->Fire();
			}
		}
		else if (pTank->CanFortify())
		{
			// If the fortify point has moved, we must move.
			if (pTank->IsFortified() && (pTank->GetOrigin() - pTank->GetFortifyPoint()).Length2D() > pTank->GetRemainingMovementDistance()*2)
				pTank->Fortify();
		}
		else
		{
			// Hang out at spawn. We're probably waiting to be put in an attack team.

			CDigitank* pClosestEnemy = NULL;
			while (true)
			{
				pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(pTank->GetOrigin(), pClosestEnemy);

				if (!pClosestEnemy)
					break;

				if (pClosestEnemy->GetTeam() == pTank->GetTeam())
					continue;

				if (!pTank->IsInsideMaxRange(pClosestEnemy->GetOrigin()))
				{
					pClosestEnemy = NULL;
					break;
				}

				break;
			}

			if (pClosestEnemy)
			{
				if (pClosestEnemy->Distance(pTank->GetOrigin()) < pTank->VisibleRange())
				{
					// Jesus we were just hanging out at the base and this asshole came up and started shooting us!
					if ((pClosestEnemy->GetOrigin() - pTank->GetOrigin()).LengthSqr() > pTank->GetEffRange()*pTank->GetEffRange())
					{
						Vector vecDesiredMove = pTank->GetOrigin() + (pClosestEnemy->GetOrigin() - pTank->GetOrigin()).Normalized() * (pTank->GetRemainingMovementDistance() * 0.6f);

						pTank->SetPreviewMove(vecDesiredMove);
						pTank->Move();
					}

					// If we are within the max range, try to fire.
					if (pTank->IsInsideMaxRange(pClosestEnemy->GetOrigin()))
					{
						pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(pClosestEnemy->GetOrigin()));
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

				pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(vecTargetOrigin));
				pTank->Fire();
			}
			else
			{
				CDigitank* pClosestEnemy = NULL;

				// Otherwise look for the closest enemy and fire on them.
				while (true)
				{
					pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(pTank->GetOrigin(), pClosestEnemy);

					if (!pClosestEnemy)
						break;

					if (pClosestEnemy->GetTeam() == pTank->GetTeam())
						continue;

					if (!pTank->IsInsideMaxRange(pClosestEnemy->GetOrigin()))
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

					pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(pClosestEnemy->GetOrigin()));
					pTank->Fire();
				}
			}
		}
	}

	DigitanksGame()->EndTurn();
}

void CDigitanksTeam::Bot_ExecuteTurnArtillery()
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

		eastl::vector<CBaseEntity*> apTargets;

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
			else if (pClosestEnemy->Distance(pTank->GetOrigin()) > pEntity->Distance(pTank->GetOrigin()))
				pClosestEnemy = dynamic_cast<CDigitanksEntity*>(pEntity);

			if (!pTank->IsInsideMaxRange(pEntity->GetOrigin()))
				continue;

			float flTargetVisibility = pTank->GetVisibility(this);
			if (flTargetVisibility < 1 && RandomFloat(0, 1) > flTargetVisibility)
				continue;

			apTargets.push_back(pEntity);
		}

		if (pClosestEnemy && pTank->CanCharge() && pClosestEnemy->Distance(pTank->GetOrigin()) < pTank->ChargeRadius() && RandomInt(0, 3) == 0)
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
				Vector vecDirection = pClosestEnemy->GetOrigin() - pTank->GetOrigin();

				int iTries = 0;
				do
				{
					vecDirection = vecDirection.Normalized() * (flMovementDistance*4/5);

					Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
					vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

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
		if ((pTarget->GetOrigin() - pTank->GetOrigin()).LengthSqr() > pTank->GetEffRange()*pTank->GetEffRange())
		{
			float flMovementDistance = pTank->GetRemainingMovementDistance();
			Vector vecDirection = pTarget->GetOrigin() - pTank->GetOrigin();
			vecDirection = vecDirection.Normalized() * (flMovementDistance*2/3);

			Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
			vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

			pTank->SetPreviewMove(vecDesiredMove);
			pTank->Move();
		}

		// If we are within the max range, try to fire.
		if (pTarget && pTank->IsInsideMaxRange(pTarget->GetOrigin()))
		{
			pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(pTarget->GetOrigin()));
			pTank->Fire();
		}
	}

	DigitanksGame()->EndTurn();
}

CSupplier* CDigitanksTeam::Bot_FindUnusedSupplier(size_t iMaxDependents, bool bNoSuppliers)
{
	eastl::vector<CSupplier*> apSuppliers;

	// Find an appropriate supplier to build from.
	for (size_t i = 0; i < GetNumMembers(); i++)
	{
		CBaseEntity* pEntity = GetMember(i);
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

bool CDigitanksTeam::Bot_BuildCollector(CResource* pResource)
{
	if (!pResource)
		return false;

	if (CSupplier::GetDataFlow(pResource->GetOrigin(), this) < 1)
		return false;

	Vector vecPSU = pResource->GetOrigin();

	DigitanksGame()->GetTerrain()->SetPointHeight(vecPSU);

	m_hPrimaryCPU->SetPreviewStructure(CanBuildPSUs()?STRUCTURE_PSU:STRUCTURE_BATTERY);
	m_hPrimaryCPU->SetPreviewBuild(vecPSU);

	if (!m_hPrimaryCPU->IsPreviewBuildValid())
		return false;

	return m_hPrimaryCPU->BeginConstruction();
}

void CDigitanksTeam::Bot_AddBuildPriority(unittype_t eUnit, CDigitanksEntity* pTarget)
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
		if (m_aoDefenders[i].m_hDefender == NULL)
		{
			m_aoDefenders[i].m_hDefender = pTank;

			Vector vecFortify = GetOrigin() + AngleVector(EAngle(0, m_aoDefenders[i].m_flPosition, 0)) * 20;
			pTerrain->SetPointHeight(vecFortify);

			if (pTerrain->IsPointOverHole(vecFortify) || pTerrain->IsPointOverWater(vecFortify) || pTerrain->IsPointOverLava(vecFortify))
				continue;

			pTank->SetFortifyPoint(this, vecFortify);
			return;
		}
	}

	// Member 0 is typically the CPU.
	float flYaw;
	if (dynamic_cast<CCPU*>(this))
		flYaw = VectorAngles(-GetOrigin()).y;
	else
		flYaw = VectorAngles(GetOrigin() - pTank->GetDigitanksTeam()->GetMember(0)->GetOrigin()).y;

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

		vecFortify = GetOrigin() + AngleVector(EAngle(0, flYaw, 0)) * 20;
		DigitanksGame()->GetTerrain()->SetPointHeight(vecFortify);

		iFortifies++;

		if (pTerrain->IsPointOverHole(vecFortify) || pTerrain->IsPointOverWater(vecFortify) || pTerrain->IsPointOverLava(vecFortify))
			continue;

		bFound = true;
		break;
	}

	if (!bFound)
		return;

	DigitanksGame()->GetTerrain()->SetPointHeight(vecFortify);

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
		if (m_aoDefenders[i].m_hDefender == pTank)
			m_aoDefenders[i].m_hDefender = NULL;
	}
}

size_t CStructure::GetNumLivingDefenders()
{
	size_t iDefenders = 0;
	for (size_t i = 0; i < m_aoDefenders.size(); i++)
	{
		if (m_aoDefenders[i].m_hDefender == NULL)
			continue;

		if (m_aoDefenders[i].m_hDefender->IsAlive())
			iDefenders++;
	}

	return iDefenders;
}
