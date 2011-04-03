#include "digitanksteam.h"

#include <mtrand.h>

#include "digitanksgame.h"

#include "units/artillery.h"
#include "units/maintank.h"
#include "units/mechinf.h"
#include "units/scout.h"

void CDigitanksTeam::Bot_ExecuteTurnCampaign()
{
	m_aeBuildPriorities.clear();

	if (m_hPrimaryCPU != NULL && m_hPrimaryCPU->IsActive())
	{
		m_hInfantryLoader = NULL;
		m_hTankLoader = NULL;
		m_hArtilleryLoader = NULL;

		size_t iInfantry = 0;
		size_t iMainTanks = 0;
		size_t iArtillery = 0;
		size_t iScouts = 0;

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

		if (iScouts < 2 && DigitanksGame()->GetTurn() > m_iLastScoutBuilt + 5)
			Bot_AddBuildPriority(UNIT_SCOUT);

		if (m_hInfantryLoader != NULL && !m_hInfantryLoader->IsProducing() && flInfantryFleetRatio < flBuildInfantryRatio && GetUnusedFleetPoints() >= CMechInfantry::InfantryFleetPoints())
			Bot_AddBuildPriority(UNIT_INFANTRY);

		if (m_hTankLoader != NULL && !m_hTankLoader->IsProducing() && flTankFleetRatio < flBuildTankRatio && GetUnusedFleetPoints() >= CMainBattleTank::MainTankFleetPoints())
			Bot_AddBuildPriority(UNIT_TANK);

		if (m_hArtilleryLoader != NULL && !m_hArtilleryLoader->IsProducing() && flArtilleryFleetRatio < flBuildArtilleryRatio && GetUnusedFleetPoints() >= CArtillery::ArtilleryFleetPoints())
			Bot_AddBuildPriority(UNIT_ARTILLERY);

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

				if (pTank->ShouldStayPut())
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

		if (pTank && pTank->IsImprisoned())
			continue;

		if (pEntity->GetTeam() == pHeadTank->GetTeam())
			continue;

		// Don't fire on neutral structures.
		if (pEntity->GetTeam() == NULL)
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

		if (pTank->IsScout())
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

				if (pClosestInfantry->IsImprisoned())
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

					if (pClosestEnemy->IsImprisoned())
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

				if (pTank->GetUnitType() == UNIT_TANK)
				{
					Vector vecMove = vecDesiredMove - pTank->GetOrigin();

					// Don't let tanks get ahead of their infantry complements.
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

					if (pClosestEnemy->IsImprisoned())
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
		else if (pTank->CanFortify() && !pTank->ShouldStayPut())
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

				if (pClosestEnemy->IsImprisoned())
					continue;

				float flTargetVisibility = pClosestEnemy->GetVisibility(this);
				if (flTargetVisibility < 0.4f)
					continue;

				if (flTargetVisibility < 1 && RandomFloat(0, 1) > flTargetVisibility)
					continue;

				if (!pTank->IsInsideMaxRange(pClosestEnemy->GetOrigin()))
				{
					if (pClosestEnemy->Distance(pTank->GetOrigin()) < pTank->VisibleRange()*1.5f)
						break;

					pClosestEnemy = NULL;
					break;
				}

				break;
			}

			if (pClosestEnemy)
			{
				// Jesus we were just hanging out at the base and this asshole came up and started shooting us!
				if ((pClosestEnemy->GetOrigin() - pTank->GetOrigin()).LengthSqr() > pTank->GetEffRange()*pTank->GetEffRange())
				{
					Vector vecDesiredMove = pTank->GetOrigin() + (pClosestEnemy->GetOrigin() - pTank->GetOrigin()).Normalized() * (pTank->GetRemainingMovementDistance() * 0.6f);

					pTank->SetPreviewMove(vecDesiredMove);
					pTank->Move();
				}

				if (pTank->IsInfantry())
				{
					if (pClosestEnemy->GetUnitType() == UNIT_SCOUT)
						pTank->SetCurrentWeapon(WEAPON_INFANTRYLASER, false);
					else
						pTank->SetCurrentWeapon(PROJECTILE_FLAK, false);
				}

				// If we are within the max range, try to fire.
				if (pTank->IsInsideMaxRange(pClosestEnemy->GetOrigin()))
				{
					pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(pClosestEnemy->GetOrigin()));
					pTank->Fire();
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

					if (pClosestEnemy->IsImprisoned())
						continue;

					float flTargetVisibility = pClosestEnemy->GetVisibility(this);
					if (flTargetVisibility < 0.4f)
						continue;

					if (flTargetVisibility < 1 && RandomFloat(0, 1) > flTargetVisibility)
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
