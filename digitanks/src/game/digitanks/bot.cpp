#include "digitanksteam.h"

#include "digitanksgame.h"

void CDigitanksTeam::Bot_ExecuteTurn()
{
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
