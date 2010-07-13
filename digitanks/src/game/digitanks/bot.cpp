#include "digitanksteam.h"

#include <maths.h>

#include "digitanksgame.h"
#include "resource.h"
#include "loader.h"

structure_t g_aeBuildOrder[] =
{
	STRUCTURE_BUFFER,
	STRUCTURE_INFANTRYLOADER,
	STRUCTURE_BUFFER,
	STRUCTURE_PSU,
	STRUCTURE_BUFFER,
	STRUCTURE_TANKLOADER,
};

void CDigitanksTeam::Bot_ExpandBase()
{
	// If currently building something, don't think about it this turn.
	if (m_hPrimaryCPU->HasConstruction())
		return;

	if (m_iBuildPosition >= sizeof(g_aeBuildOrder)/sizeof(structure_t))
		m_iBuildPosition = 0;

	if (m_iProduction < 8)
	{
		// Find the closest electronode and build a collector.
		CResource* pClosest = CBaseEntity::FindClosest<CResource>(m_hPrimaryCPU->GetOrigin());
		if (pClosest)
			BuildCollector(m_hPrimaryCPU, pClosest);

		return;
	}

	if (g_aeBuildOrder[m_iBuildPosition] == STRUCTURE_PSU)
	{
		CResource* pTargetResource = NULL;
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

		CSupplier* pClosestSupplier = NULL;
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

		// Too damn far? Don't bother.
		if ((pTargetResource->GetOrigin() - pClosestSupplier->GetOrigin()).Length() > 80)
		{
			m_iBuildPosition++;
			return;
		}

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

			m_hPrimaryCPU->SetPreviewStructure(STRUCTURE_BUFFER);
			m_hPrimaryCPU->SetPreviewBuild(vecStructure);
			m_hPrimaryCPU->BeginConstruction();
		}

		return;
	}

	CSupplier* pUnused = NULL;

	if (g_aeBuildOrder[m_iBuildPosition] == STRUCTURE_BUFFER)
		pUnused = FindUnusedSupplier(4, false);
	else
		pUnused = FindUnusedSupplier(2);

	float flYaw;
	if (pUnused == m_hPrimaryCPU)
		flYaw = RemapVal((float)(rand()%1000), 0, 1000, 0, 360);
	else
	{
		flYaw = VectorAngles(pUnused->GetOrigin() - m_hPrimaryCPU->GetOrigin()).y;
		flYaw = RemapVal((float)(rand()%1000), 0, 1000, flYaw-90, flYaw+90);
	}

	// Pick a random direction facing more or less away from the CPU so that we spread outwards.
	Vector vecStructureDirection = AngleVector(EAngle(0, flYaw, 0));
	Vector vecStructure = pUnused->GetOrigin();
	if (g_aeBuildOrder[m_iBuildPosition] == STRUCTURE_BUFFER)
		vecStructure += vecStructureDirection.Normalized() * pUnused->GetDataFlowRadius()*2/3;
	else
		vecStructure += vecStructureDirection.Normalized() * 20;

	DigitanksGame()->GetTerrain()->SetPointHeight(vecStructure);

	m_hPrimaryCPU->SetPreviewStructure(g_aeBuildOrder[m_iBuildPosition]);
	m_hPrimaryCPU->SetPreviewBuild(vecStructure);
	m_hPrimaryCPU->BeginConstruction();

	m_iBuildPosition++;
}

void CDigitanksTeam::Bot_BuildUnits()
{
	// Find the nearest enemy to the head tank, he's our target.
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		CBaseEntity* pEntity = m_ahMembers[i];
		if (!pEntity)
			continue;

		CLoader* pLoader = dynamic_cast<CLoader*>(pEntity);
		if (!pLoader)
			continue;

		if (pLoader->IsConstructing())
			continue;

		if (!pLoader->IsProducing())
			pLoader->BeginProduction();
	}
}

void CDigitanksTeam::Bot_AssignDefenders()
{
	std::vector<CStructure*> apDefend;
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		CBaseEntity* pEntity = m_ahMembers[i];
		if (!pEntity)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);
		if (!pStructure)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);

		// Only non-suppliers or suppliers with no children should be defended, so that structures in the center of the base aren't defended unnecessarily.
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
	Bot_ExpandBase();
	Bot_BuildUnits();
	Bot_AssignDefenders();

	CDigitank* pHeadTank = GetTank(0);

	CDigitank* pTarget = NULL;

	// Find the nearest enemy to the head tank, he's our target.
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		if (!pTank)
			continue;

		if (pTank->GetTeam() == pHeadTank->GetTeam())
			continue;

		if (GetEntityVisibility(pTank->GetHandle()) == 0)
			continue;

		if (!pTarget)
		{
			pTarget = pTank;
			continue;
		}

		if ((pHeadTank->GetOrigin() - pTank->GetOrigin()).Length2DSqr() < (pHeadTank->GetOrigin() - pTarget->GetOrigin()).Length2DSqr())
			pTarget = pTank;
	}

	Vector vecTargetOrigin;
	if (pTarget)
		vecTargetOrigin = pTarget->GetOrigin();
	else
		vecTargetOrigin = DigitanksGame()->GetDigitanksTeam(0)->GetMember(0)->GetOrigin();	// Should be the CPU

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

		if (pTank->HasFortifyPoint() && !pTank->IsFortified())
		{
			if (!pTank->IsFortified() && (pTank->GetOrigin() - pTank->GetFortifyPoint()).Length2D() < pTank->GetBoundingRadius()*2)
			{
				CCPU* pDefend = m_hPrimaryCPU;
				pTank->SetPreviewTurn(VectorAngles(pTank->GetOrigin() - pDefend->GetOrigin()).y);
				pTank->SetDesiredTurn();
				pTank->Fortify();
			}
			else
			{
				// Head to the fortify point
				float flMovementDistance = pTank->GetMaxMovementDistance();
				Vector vecDirection = pTank->GetFortifyPoint() - pTank->GetOrigin();
				vecDirection = vecDirection.Normalized() * (flMovementDistance*2/3);

				Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
				vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->SetDesiredMove();

				pTank->SetPreviewTurn(VectorAngles(vecTargetOrigin - pTank->GetDesiredMove()).y);
				pTank->SetDesiredTurn();
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
					pTank->SetDesiredTurn();
					pTank->Fortify();
				}
				else
				{
					// Head away from enemies at full speed
					float flMovementDistance = pTank->GetMaxMovementDistance();
					Vector vecDirection = vecTargetOrigin - pTank->GetOrigin();
					vecDirection = -vecDirection.Normalized() * (flMovementDistance*0.90f);

					Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
					vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

					pTank->SetPreviewMove(vecDesiredMove);
					pTank->SetDesiredMove();

					pTank->SetPreviewTurn(VectorAngles(vecTargetOrigin - pTank->GetDesiredMove()).y);
					pTank->SetDesiredTurn();
				}
			}

			// If we are within the max range, try to fire.
			if ((vecTargetOrigin - pTank->GetPreviewMove()).LengthSqr() < pTank->GetMaxRange()*pTank->GetMaxRange())
			{
				pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(vecTargetOrigin));
				pTank->SetDesiredAim();
			}
		}
		else if (pTank->CanFortify())
		{
			// If the fortify point has moved, we must move.
			if (pTank->IsFortified() && (pTank->GetOrigin() - pTank->GetFortifyPoint()).Length2D() > pTank->GetMaxMovementDistance()*2)
				pTank->Fortify();
		}
		else
		{
			// If we are not within the effective range, use 1/3 of our available movement power to move towards our target.
			if ((vecTargetOrigin - pTank->GetOrigin()).LengthSqr() > pTank->GetEffRange()*pTank->GetEffRange())
			{
				float flMovementDistance = pTank->GetMaxMovementDistance();
				Vector vecDirection = vecTargetOrigin - pTank->GetOrigin();
				vecDirection = vecDirection.Normalized() * (flMovementDistance/3);

				Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
				vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->SetDesiredMove();
			}

			// If we are within the max range, try to fire.
			if ((vecTargetOrigin - pTank->GetPreviewMove()).LengthSqr() < pTank->GetMaxRange()*pTank->GetMaxRange())
			{
				pTank->SetPreviewAim(DigitanksGame()->GetTerrain()->SetPointHeight(vecTargetOrigin));
				pTank->SetDesiredAim();
			}
		}

		if (pTarget && pTarget->IsFortified())
			pTank->SetAttackPower((pTank->GetBasePower() - pTank->GetBaseMovementPower())*3/4);
		else
			pTank->SetAttackPower((pTank->GetBasePower() - pTank->GetBaseMovementPower())/2);
	}

	DigitanksGame()->EndTurn();
}

CSupplier* CDigitanksTeam::FindUnusedSupplier(size_t iMaxDependents, bool bNoSuppliers)
{
	std::vector<CSupplier*> apSuppliers;

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

	Vector vecForward = pResource->GetOrigin() - pSupplier->GetOrigin();
	Vector vecRight;
	AngleVectors(VectorAngles(vecForward.Normalized()), NULL, &vecRight, NULL);

	// A pretty spot midway between the supplier and the resource.
	Vector vecPSU = (pResource->GetOrigin() + pSupplier->GetOrigin())/2 + vecRight*vecForward.Length2D();

	// Move that spot near to the resource so we know we can get it built.
	Vector vecPSUDirection = vecPSU - pResource->GetOrigin();
	vecPSU = pResource->GetOrigin() + vecPSUDirection.Normalized() * 10;

	DigitanksGame()->GetTerrain()->SetPointHeight(vecPSU);

	m_hPrimaryCPU->SetPreviewStructure(STRUCTURE_PSU);
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
	float flYaw = VectorAngles(GetOrigin() - pTank->GetDigitanksTeam()->GetMember(0)->GetOrigin()).y;

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
