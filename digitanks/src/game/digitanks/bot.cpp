#include "digitanksteam.h"

#include <maths.h>

#include "digitanksgame.h"
#include "resource.h"

structure_t g_aeBuildOrder[] =
{
	STRUCTURE_BUFFER,
	STRUCTURE_BUFFER,
	STRUCTURE_INFANTRYLOADER,
	STRUCTURE_BUFFER,
	STRUCTURE_BUFFER,
	STRUCTURE_TANKLOADER,
	STRUCTURE_BUFFER,
	STRUCTURE_BUFFER,
	STRUCTURE_PSU,
};

void CDigitanksTeam::Bot_ExpandBase()
{
	// If currently building something, don't think about it this turn.
	if (m_hPrimaryCPU->HasConstruction())
		return;

	if (m_iBuildPosition >= sizeof(g_aeBuildOrder)/sizeof(structure_t))
		m_iBuildPosition = 0;

	if (m_iProduction == 0)
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

void CDigitanksTeam::Bot_ExecuteTurn()
{
	Bot_ExpandBase();

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

	Vector vecFortifyPoint;
	bool bShouldFortify;

	if (pTarget)
	{
		bShouldFortify = true;

		// Find the closest fortification point. Each enemy tank is examined to find the closest spot outside of all enemy tank ranges.
		for (size_t i = 0; i < pTarget->GetDigitanksTeam()->GetNumTanks(); i++)
		{
			CDigitank* pTank = pTarget->GetDigitanksTeam()->GetTank(i);

			// Artillery isn't a big deal, and they have huge ranges.
			if (pTank->IsArtillery())
				continue;

			if (GetEntityVisibility(pTank->GetHandle()) == 0)
				continue;

			Vector vecDirection = (pHeadTank->GetOrigin() - pTank->GetOrigin()).Normalized();
			Vector vecTryFortifyPoint = pTank->GetOrigin() + vecDirection * (pTank->GetMaxRange() + pTank->GetMaxMovementDistance());

			if (i == 0)
			{
				vecFortifyPoint = vecTryFortifyPoint;
				continue;
			}

			if ((pHeadTank->GetOrigin() - vecTryFortifyPoint).Length2DSqr() < (pHeadTank->GetOrigin() - vecFortifyPoint).Length2DSqr())
				vecFortifyPoint = vecTryFortifyPoint;
		}
	}
	else
		bShouldFortify = false;

	size_t iFortifies = 0;

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

		if (pTank->IsArtillery())
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
		else if (pTank->CanFortify() && bShouldFortify)
		{
			Vector vecTankFortifyPoint;
			if (iFortifies == 0)
				vecTankFortifyPoint = vecFortifyPoint;
			else if (iFortifies%2 == 0)
			{
				Vector vecSide = (vecTargetOrigin - vecFortifyPoint).Normalized().Cross(Vector(0,1,0)).Normalized();
				vecTankFortifyPoint = vecFortifyPoint + vecSide * (float)((iFortifies/2)*pTank->GetMaxRange()/2);
			}
			else
			{
				Vector vecSide = -(vecTargetOrigin - vecFortifyPoint).Normalized().Cross(Vector(0,1,0)).Normalized();
				vecTankFortifyPoint = vecFortifyPoint + vecSide * (float)((iFortifies/2+1)*pTank->GetMaxRange()/2);
			}

			DigitanksGame()->GetTerrain()->SetPointHeight(vecTankFortifyPoint);

			pTank->SetFortifyPoint(vecTankFortifyPoint);

			iFortifies++;

			//if (pTank->IsFortified() && (pTank->GetOrigin() - vecTankFortifyPoint).Length2D() > pTank->GetMaxMovementDistance()*2)
			//	pTank->Fortify();

			// If our target is behind us, we have to mobilize so we can turn around to face them.
			if (pTank->IsFortified() && (vecTargetOrigin - pTank->GetOrigin()).Normalized().Dot(AngleVector(pTank->GetAngles())) < 0)
				pTank->Fortify();

			if (!pTank->IsFortified() && (pTank->GetOrigin() - vecTankFortifyPoint).Length2D() < pTank->GetBoundingRadius()*2)
			{
				// Face the enemy and fortify before he gets here.
				pTank->SetPreviewTurn(VectorAngles(vecTargetOrigin - pTank->GetOrigin()).y);
				pTank->SetDesiredTurn();
				pTank->Fortify();
			}
			else
			{
				// Head to the fortify point
				float flMovementDistance = pTank->GetMaxMovementDistance();
				Vector vecDirection = vecTankFortifyPoint - pTank->GetOrigin();
				vecDirection = vecDirection.Normalized() * (flMovementDistance*2/3);

				Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
				vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->SetDesiredMove();

				pTank->SetPreviewTurn(VectorAngles(vecTargetOrigin - pTank->GetDesiredMove()).y);
				pTank->SetDesiredTurn();
			}
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
