#include "digitanksteam.h"

#include <maths.h>
#include <geometry.h>
#include <mtrand.h>

#include "digitanksgame.h"
#include "updates.h"
#include "structures/resource.h"
#include "structures/loader.h"

#include "units/artillery.h"
#include "units/maintank.h"
#include "units/mechinf.h"
#include "units/scout.h"

unittype_t g_aeBuildOrder[] =
{
	STRUCTURE_BATTERY,
	UNIT_SCOUT,
	STRUCTURE_INFANTRYLOADER,
	STRUCTURE_PSU,
	STRUCTURE_BUFFER,
	STRUCTURE_TANKLOADER,
	STRUCTURE_PSU,
	STRUCTURE_PSU,
	STRUCTURE_ARTILLERYLOADER,
	STRUCTURE_BUFFER,
};

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

	for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
	{
		for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
		{
			if (!CanDownloadUpdate(x, y))
				continue;

			if (pGrid->m_aUpdates[x][y].m_eUpdateClass == UPDATECLASS_STRUCTURE)
			{
				// If it's a structure we need then it's top priority, grab it NAOW.
				if (pGrid->m_aUpdates[x][y].m_eStructure == g_aeBuildOrder[m_iBuildPosition])
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

void CDigitanksTeam::Bot_ExpandBase()
{
	// Never install more than one thing at a time so we don't get bogged down in installations.
	if (GetNumProducers() < 1)
	{
		for (size_t i = 0; i < m_ahMembers.size(); i++)
		{
			if (GetNumProducers() >= 1)
				break;

			CBaseEntity* pEntity = m_ahMembers[i];
			if (!pEntity)
				continue;

			CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);
			if (!pStructure)
				continue;

			if (!pStructure->HasUpdatesAvailable() && !pStructure->CanStructureUpgrade())
				continue;

			if (pStructure->IsInstalling() || pStructure->IsUpgrading())
				continue;

			// Give the CPU a chance to build structures.
			if (mtrand()%2 == 0)
				continue;

			if (pStructure->CanStructureUpgrade())
			{
				pStructure->BeginUpgrade();
				break;
			}

			// PSU's are highest priority.
			if (m_iBuildPosition < sizeof(g_aeBuildOrder)/sizeof(unittype_t) && g_aeBuildOrder[m_iBuildPosition] == STRUCTURE_PSU)
				continue;

			for (size_t u = 0; u < UPDATETYPE_SIZE; u++)
			{
				int iUpdate = pStructure->GetFirstUninstalledUpdate((updatetype_t)u);
				if (iUpdate < 0)
					continue;

				pStructure->InstallUpdate((updatetype_t)u);
				break;
			}
		}
	}

	if (m_hPrimaryCPU == NULL)
		return;

	// If currently building something, don't think about it this turn.
	if (m_hPrimaryCPU->HasConstruction())
		return;

	if (m_hPrimaryCPU->IsInstalling())
		return;

	if (m_hPrimaryCPU->IsProducing())
		return;

	if (GetNumProducers() >= 2)
		return;

	unittype_t iNextBuild;
	if (m_iBuildPosition >= sizeof(g_aeBuildOrder)/sizeof(unittype_t))
		iNextBuild = STRUCTURE_PSU;
	else
		iNextBuild = g_aeBuildOrder[m_iBuildPosition];

	bool bBumpBuildPosition = true;

	if (iNextBuild == UNIT_SCOUT)
	{
		m_hPrimaryCPU->BeginRogueProduction();
		m_iBuildPosition++;
		return;
	}

	if (iNextBuild == STRUCTURE_PSU)
	{
		if (!CanBuildPSUs())
		{
			if (RandomInt(0, 1) == 0)
			{
				// Build buffers while we wait.
				iNextBuild = CanBuildBuffers()?STRUCTURE_BUFFER:STRUCTURE_MINIBUFFER;
				bBumpBuildPosition = false;
			}
			else
			{
				iNextBuild = STRUCTURE_BATTERY;
				bBumpBuildPosition = false;
			}
		}
	}

	CResource* pTargetResource = NULL;
	CSupplier* pClosestSupplier = NULL;
	if (iNextBuild == STRUCTURE_PSU || iNextBuild == STRUCTURE_BATTERY)
	{
		while (true)
		{
			pTargetResource = CBaseEntity::FindClosest<CResource>(m_hPrimaryCPU->GetOrigin(), pTargetResource);

			if (!pTargetResource)
				break;

			if (pTargetResource->HasCollector())
				continue;

			break;
		}

		if (!pTargetResource)
		{
			m_iBuildPosition++;
			return;
		}

		while (true)
		{
			pClosestSupplier = CBaseEntity::FindClosest<CSupplier>(pTargetResource->GetOrigin(), pClosestSupplier);

			if (!pClosestSupplier)
				break;

			if (pClosestSupplier->GetDigitanksTeam() != this)
				continue;

			break;
		}

		// Don't know how this is possible but watev.
		if (!pClosestSupplier)
		{
			m_iBuildPosition++;
			return;
		}

		// Too damn far? Do a random buffer instead.
		if ((pTargetResource->GetOrigin() - pClosestSupplier->GetOrigin()).Length() > 80)
			iNextBuild = STRUCTURE_BUFFER;
	}

	if (iNextBuild == STRUCTURE_PSU || iNextBuild == STRUCTURE_BATTERY)
	{
		if (CSupplier::GetDataFlow(pTargetResource->GetOrigin(), this) > 1)
		{
			BuildCollector(pClosestSupplier, pTargetResource);
			m_iBuildPosition++;
			return;
		}
		else
		{
			Vector vecStructureDirection = pTargetResource->GetOrigin() - pClosestSupplier->GetOrigin();
			Vector vecStructure = pClosestSupplier->GetOrigin();
			vecStructure += vecStructureDirection.Normalized() * pClosestSupplier->GetDataFlowRadius()*2/3;

			DigitanksGame()->GetTerrain()->SetPointHeight(vecStructure);

			m_hPrimaryCPU->SetPreviewStructure(CanBuildBuffers()?STRUCTURE_BUFFER:STRUCTURE_MINIBUFFER);
			m_hPrimaryCPU->SetPreviewBuild(vecStructure);

			// If we can't build this for some reason, build a random buffer instead.
			if (!m_hPrimaryCPU->IsPreviewBuildValid())
				iNextBuild = STRUCTURE_BUFFER;
			else
				m_hPrimaryCPU->BeginConstruction();
		}
	}

	if (iNextBuild == STRUCTURE_PSU || iNextBuild == STRUCTURE_BATTERY)
		return;

	if (iNextBuild == STRUCTURE_MINIBUFFER && CanBuildBuffers())
		iNextBuild = STRUCTURE_BUFFER;

	if (iNextBuild == STRUCTURE_BUFFER && !CanBuildBuffers())
	{
		iNextBuild = STRUCTURE_MINIBUFFER;
		bBumpBuildPosition = false;	// Must not bump build position or we might skip researching buffers.
	}

	// If we can't build this kind of structure then return without trying to do anything
	// and wait until we can.
	if (iNextBuild == STRUCTURE_BUFFER)
	{
		if (!CanBuildBuffers())
			return;
	}
	else if (iNextBuild == STRUCTURE_TANKLOADER)
	{
		if (!CanBuildTankLoaders())
		{
			iNextBuild = STRUCTURE_BUFFER;
			bBumpBuildPosition = false;
		}
	}
	else if (iNextBuild == STRUCTURE_INFANTRYLOADER)
	{
		if (!CanBuildInfantryLoaders())
		{
			iNextBuild = STRUCTURE_BUFFER;
			bBumpBuildPosition = false;
		}
	}
	else if (iNextBuild == STRUCTURE_ARTILLERYLOADER)
	{
		if (!CanBuildArtilleryLoaders())
		{
			iNextBuild = STRUCTURE_BUFFER;
			bBumpBuildPosition = false;
		}
	}

	if (iNextBuild == STRUCTURE_BUFFER && !CanBuildBuffers())
	{
		iNextBuild = STRUCTURE_MINIBUFFER;
		bBumpBuildPosition = false;	// Must not bump build position or we might skip researching buffers.
	}

	CSupplier* pUnused = NULL;

	if (iNextBuild == STRUCTURE_BUFFER || iNextBuild == STRUCTURE_MINIBUFFER)
		pUnused = FindUnusedSupplier(4, false);
	else
		pUnused = FindUnusedSupplier(2);

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
	Vector vecStructure = pUnused->GetOrigin();
	if (iNextBuild == STRUCTURE_BUFFER || iNextBuild == STRUCTURE_MINIBUFFER)
	{
		Vector vecPreview = vecStructure + vecStructureDirection.Normalized() * pUnused->GetDataFlowRadius()*9/10;
		if (CSupplier::GetDataFlow(vecPreview, this) <= 0)
			vecPreview = vecStructure + vecStructureDirection.Normalized() * pUnused->GetDataFlowRadius()*2/3;

		vecStructure = vecPreview;
	}
	else
		vecStructure += vecStructureDirection.Normalized() * 20;

	// Don't build structures too close to the map edges.
	if (vecStructure.x < -DigitanksGame()->GetTerrain()->GetMapSize()+15)
		return;
	if (vecStructure.z < -DigitanksGame()->GetTerrain()->GetMapSize()+15)
		return;
	if (vecStructure.x > DigitanksGame()->GetTerrain()->GetMapSize()-15)
		return;
	if (vecStructure.z > DigitanksGame()->GetTerrain()->GetMapSize()-15)
		return;

	DigitanksGame()->GetTerrain()->SetPointHeight(vecStructure);

	m_hPrimaryCPU->SetPreviewStructure(iNextBuild);
	m_hPrimaryCPU->SetPreviewBuild(vecStructure);

	// If we can't build for some reason, don't bump the build position and skip this structure.
	if (!m_hPrimaryCPU->IsPreviewBuildValid())
		return;

	m_hPrimaryCPU->BeginConstruction();

	if (bBumpBuildPosition)
		m_iBuildPosition++;
}

void CDigitanksTeam::Bot_BuildUnits()
{
	// Above code is capped at 2 producers, but we make this one more so we always have a guarantee to be creating units.
	if (GetNumProducers() >= 3)
		return;

	size_t iInfantry = 0;
	size_t iMainTanks = 0;
	size_t iArtillery = 0;

	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		CBaseEntity* pEntity = m_ahMembers[i];
		if (!pEntity)
			continue;

		CDigitank* pDigitank = dynamic_cast<CDigitank*>(pEntity);

		if (dynamic_cast<CMechInfantry*>(pEntity))
			iInfantry += pDigitank->FleetPoints();
		else if (dynamic_cast<CMainBattleTank*>(pEntity))
			iMainTanks += pDigitank->FleetPoints();
		else if (dynamic_cast<CArtillery*>(pEntity))
			iArtillery += pDigitank->FleetPoints();
	}

	size_t iRatioTotal = CMechInfantry::InfantryFleetPoints() + CMainBattleTank::MainTankFleetPoints() + CArtillery::ArtilleryFleetPoints();
	float flInfantryRatio = (float)iRatioTotal/CMechInfantry::InfantryFleetPoints();
	float flMainTankRatio = (float)iRatioTotal/CMainBattleTank::MainTankFleetPoints();
	float flArtilleryRatio = (float)iRatioTotal/CArtillery::ArtilleryFleetPoints();
	float flRatioTotal = flInfantryRatio + flMainTankRatio + flArtilleryRatio;

	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (GetNumProducers() >= 3)
			continue;

		CBaseEntity* pEntity = m_ahMembers[i];
		if (!pEntity)
			continue;

		CLoader* pLoader = dynamic_cast<CLoader*>(pEntity);
		if (!pLoader)
			continue;

		if (pLoader->IsConstructing())
			continue;

		// Don't just build infantry all the time.
		if (mtrand()%2 == 0)
			continue;

		if (pLoader->IsProducing())
			continue;

		size_t iTanks;
		float flValue;
		if (pLoader->GetBuildUnit() == UNIT_INFANTRY)
		{
			iTanks = iInfantry;
			flValue = flInfantryRatio;
		}
		else if (pLoader->GetBuildUnit() == UNIT_TANK)
		{
			iTanks = iMainTanks;
			flValue = flMainTankRatio;
		}
		else if (pLoader->GetBuildUnit() == UNIT_ARTILLERY)
		{
			iTanks = iArtillery;
			flValue = flArtilleryRatio;
		}

		// Build a ratio of tanks similar to the cost of constructing the tanks. This way we won't build a bajillion infantry and only one or two other tanks.
		float flTanksRatio = ((float)iTanks+1)/GetTotalFleetPoints();
		float flBuildRatio = flValue/flRatioTotal;
		if (flTanksRatio > flBuildRatio)
			continue;

		pLoader->BeginProduction();
	}
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
	if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY || m_bUseArtilleryAI)
	{
		Bot_ExecuteTurnArtillery();
		return;
	}

	if (m_hPrimaryCPU != NULL)
	{
		Bot_DownloadUpdates();
		Bot_ExpandBase();
		Bot_BuildUnits();
		Bot_AssignDefenders();
	}

	CDigitank* pHeadTank = GetTank(0);

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

		if (GetEntityVisibility(pEntity->GetHandle()) == 0)
			continue;

		CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);

		if (!pTarget)
		{
			pTarget = pDTEntity;
			continue;
		}

		float flTargetVisibility = pDTEntity->GetVisibility();
		if (flTargetVisibility < 1 && RandomFloat(0, 1) > flTargetVisibility)
			continue;

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
		m_vecExplore.x = RandomFloat(-flMapSize, flMapSize);
		m_vecExplore.z = RandomFloat(-flMapSize, flMapSize);
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
			pTank->Fortify();
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
				Vector vecDirection = m_vecExplore - pTank->GetOrigin();
				vecDirection = vecDirection.Normalized() * (flMovementDistance*flMovementPower);

				vecDesiredMove = pTank->GetOrigin() + vecDirection;
				vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);
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
				float flMovementDistance = pTank->GetRemainingMovementDistance();
				Vector vecDirection = pTank->GetFortifyPoint() - pTank->GetOrigin();
				vecDirection = vecDirection.Normalized() * (flMovementDistance*2/3);

				Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
				vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

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
			// If we are not within the effective range, use 1/3 of our available movement power to move towards our target.
			if ((vecTargetOrigin - pTank->GetOrigin()).LengthSqr() > pTank->GetEffRange()*pTank->GetEffRange())
			{
				float flMovementDistance = pTank->GetRemainingMovementDistance();
				Vector vecDirection = vecTargetOrigin - pTank->GetOrigin();
				vecDirection = vecDirection.Normalized() * (flMovementDistance/3);

				Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
				vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->Move();
			}

			// If we are within the max range, try to fire.
			if (pTarget && pTank->IsInsideMaxRange(vecTargetOrigin))
			{
				pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(vecTargetOrigin));
				pTank->Fire();
			}
		}

		if (pTank->IsInfantry())
		{
			if (pTarget && pTank->IsInsideMaxRange(vecTargetOrigin))
			{
				pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(vecTargetOrigin));
				pTank->Fire();
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

		CDigitank* pClosestEnemy = NULL;

		for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
			if (!pEntity)
				continue;

			if (!dynamic_cast<CDigitank*>(pEntity))
				continue;

			if (pEntity->GetTeam() == pTank->GetTeam())
				continue;

			// Don't fire on neutral structures.
			if (pEntity->GetTeam() == NULL)
				continue;

			if (!pClosestEnemy)
				pClosestEnemy = dynamic_cast<CDigitank*>(pEntity);
			else if (pClosestEnemy->Distance(pTank->GetOrigin()) > pEntity->Distance(pTank->GetOrigin()))
				pClosestEnemy = dynamic_cast<CDigitank*>(pEntity);

			if (!pTank->IsInsideMaxRange(pEntity->GetOrigin()))
				continue;

			float flTargetVisibility = pTank->GetVisibility();
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
				vecDirection = vecDirection.Normalized() * (flMovementDistance/3);

				Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
				vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->Move();
			}
			continue;
		}

		CDigitank* pTankTarget = dynamic_cast<CDigitank*>(pTarget);

		weapon_t eWeapon = WEAPON_NONE;
		while (eWeapon == WEAPON_NONE || eWeapon == PROJECTILE_CAMERAGUIDED)
			eWeapon = pTank->GetWeapon(RandomInt(0, pTank->GetNumWeapons()-1));

		pTank->SetCurrentWeapon(eWeapon);

		// If we are not within the effective range, use some of our available movement power to move towards our target.
		if ((pTarget->GetOrigin() - pTank->GetOrigin()).LengthSqr() > pTank->GetEffRange()*pTank->GetEffRange())
		{
			float flMovementDistance = pTank->GetRemainingMovementDistance();
			Vector vecDirection = pTarget->GetOrigin() - pTank->GetOrigin();
			vecDirection = vecDirection.Normalized() * (flMovementDistance/3);

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

CSupplier* CDigitanksTeam::FindUnusedSupplier(size_t iMaxDependents, bool bNoSuppliers)
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

	return apSuppliers[rand()%apSuppliers.size()];
}

void CDigitanksTeam::BuildCollector(CSupplier* pSupplier, CResource* pResource)
{
	if (CSupplier::GetDataFlow(pResource->GetOrigin(), this) < 1)
		return;

	Vector vecPSU = pResource->GetOrigin();

	DigitanksGame()->GetTerrain()->SetPointHeight(vecPSU);

	m_hPrimaryCPU->SetPreviewStructure(CanBuildPSUs()?STRUCTURE_PSU:STRUCTURE_BATTERY);
	m_hPrimaryCPU->SetPreviewBuild(vecPSU);
	m_hPrimaryCPU->BeginConstruction();
}

void CStructure::AddDefender(CDigitank* pTank)
{
	for (size_t i = 0; i < m_aoDefenders.size(); i++)
	{
		if (m_aoDefenders[i].m_hDefender == NULL)
		{
			m_aoDefenders[i].m_hDefender = pTank;
			Vector vecFortify = GetOrigin() + AngleVector(EAngle(0, m_aoDefenders[i].m_flPosition, 0)) * 20;
			DigitanksGame()->GetTerrain()->SetPointHeight(vecFortify);
			pTank->SetFortifyPoint(vecFortify);
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
	if (iFortifies == 0)
	{
		// Default value
	}
	else if (iFortifies%2 == 0)
		flYaw += 45*iFortifies/2;
	else
		flYaw -= 45*(iFortifies/2+1);

	Vector vecFortify = GetOrigin() + AngleVector(EAngle(0, flYaw, 0)) * 20;

	DigitanksGame()->GetTerrain()->SetPointHeight(vecFortify);

	m_aoDefenders.push_back(defender_t());
	defender_t* pDefender = &m_aoDefenders[m_aoDefenders.size()-1];
	pDefender->m_flPosition = flYaw;
	pDefender->m_hDefender = pTank;
	pTank->SetFortifyPoint(vecFortify);
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
