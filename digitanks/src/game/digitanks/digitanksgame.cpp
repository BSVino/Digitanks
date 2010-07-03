#include "digitanksgame.h"

#include <assert.h>
#include <maths.h>
#include <models/models.h>
#include <sound/sound.h>
#include <renderer/particles.h>

#include <network/network.h>
#include <ui/digitankswindow.h>
#include <ui/menu.h>
#include <ui/instructor.h>
#include "powerup.h"
#include "terrain.h"
#include "camera.h"

#include "digitanks/mechinf.h"
#include "digitanks/maintank.h"
#include "digitanks/artillery.h"
#include "digitanks/cpu.h"
#include "digitanks/projectile.h"
#include "digitanks/dt_renderer.h"
#include "digitanks/resource.h"

CDigitanksGame::CDigitanksGame()
{
	m_iCurrentTeam = 0;
	m_iCurrentSelection = 0;

	m_pListener = NULL;

	m_bWaitingForMoving = false;

	m_bWaitingForProjectiles = false;
	m_iWaitingForProjectiles = 0;

	m_iPowerups = 0;

	m_iDifficulty = 1;
}

CDigitanksGame::~CDigitanksGame()
{
}

void CDigitanksGame::RegisterNetworkFunctions()
{
	BaseClass::RegisterNetworkFunctions();

	CNetwork::RegisterFunction("SetupEntities", this, SetupEntitiesCallback, 0);
	CNetwork::RegisterFunction("EnterGame", this, EnterGameCallback, 0);
	CNetwork::RegisterFunction("EndTurn", this, EndTurnCallback, 0);
	CNetwork::RegisterFunction("StartTurn", this, StartTurnCallback, 0);
	CNetwork::RegisterFunction("SetCurrentTeam", this, SetCurrentTeamCallback, NET_INT);
	CNetwork::RegisterFunction("SetTerrain", this, SetTerrainCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("SetDesiredMove", this, SetDesiredMoveCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("CancelDesiredMove", this, CancelDesiredMoveCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("SetDesiredTurn", this, SetDesiredTurnCallback, 2, NET_HANDLE, NET_FLOAT);
	CNetwork::RegisterFunction("CancelDesiredTurn", this, CancelDesiredTurnCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("SetDesiredAim", this, SetDesiredAimCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("CancelDesiredAim", this, CancelDesiredAimCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("SetAttackPower", this, SetAttackPowerCallback, 2, NET_HANDLE, NET_FLOAT);
	CNetwork::RegisterFunction("FireProjectile", this, FireProjectileCallback, 5, NET_HANDLE, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("SetBonusPoints", this, SetBonusPointsCallback, 5, NET_HANDLE, NET_INT, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("TankPromoted", this, TankPromotedCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("PromoteAttack", this, PromoteAttackCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("PromoteDefense", this, PromoteDefenseCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("PromoteMovement", this, PromoteMovementCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("TankSpeak", this, SpeakCallback, 2, NET_HANDLE, NET_INT);
}

void CDigitanksGame::OnClientConnect(CNetworkParameters* p)
{
	CNetwork::CallFunction(p->i2, "SetTerrain", GetTerrain()->GetHandle());

	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		CNetwork::CallFunction(p->i2, "AddTeam", GetTeam(i)->GetHandle());
	}

	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (!m_ahTeams[i]->IsPlayerControlled())
		{
			p->p1 = (void*)i;
			m_ahTeams[i]->SetClient(p->i2);
			break;
		}
	}
}

void CDigitanksGame::OnClientUpdate(CNetworkParameters* p)
{
	CNetwork::CallFunction(p->i2, "EnterGame");
}

void CDigitanksGame::OnClientDisconnect(CNetworkParameters* p)
{
	m_ahTeams[p->i1]->SetClient(-1);
}

void CDigitanksGame::SetupGame()
{
	m_bLoading = true;

	SetupEntities();

	if (!CNetwork::IsHost())
		return;

	m_hTerrain = Game()->Create<CTerrain>("CTerrain");

	for (int i = 0; i < 8; i++)
	{
		float x = RemapVal((float)(rand()%1000), 0, 1000, -m_hTerrain->GetMapSize(), m_hTerrain->GetMapSize());
		float z = RemapVal((float)(rand()%1000), 0, 1000, -m_hTerrain->GetMapSize(), m_hTerrain->GetMapSize());

		CResource* pResource = Game()->Create<CResource>("CResource");
		pResource->SetOrigin(m_hTerrain->SetPointHeight(Vector(x, 0, z)));
	}

	float flReflection = 1;

	Color aclrTeamColors[] =
	{
		Color(0, 0, 255),
		Color(255, 255, 0),
		Color(255, 0, 255),
		Color(255, 0, 0),
		Color(0, 255, 0),
		Color(0, 255, 255),
		Color(0, 0, 0),
		Color(255, 255, 255),
	};

	for (int i = 0; i < 2; i++)
	{
		m_ahTeams.push_back(Game()->Create<CDigitanksTeam>("CDigitanksTeam"));

		m_ahTeams[i]->SetColor(aclrTeamColors[i]);

		CCPU* pCPU = Game()->Create<CCPU>("CCPU");
		pCPU->SetOrigin(GetTerrain()->SetPointHeight(Vector(100, 0, 100) * flReflection));
		m_ahTeams[i]->AddEntity(pCPU);
		pCPU->UpdateTendrils();

		CResource* pResource = Game()->Create<CResource>("CResource");
		float y = RemapVal((float)(rand()%1000), 0, 1000, 0, 360);
		pResource->SetOrigin(m_hTerrain->SetPointHeight(pCPU->GetOrigin() + AngleVector(EAngle(0, y, 0)) * 20));

		CDigitank* pTank;
		Vector vecTank;
		EAngle angTank;

		pTank = Game()->Create<CMechInfantry>("CMechInfantry");
		m_ahTeams[i]->AddEntity(pTank);

		vecTank = Vector(60, 0, 60)* flReflection;
		angTank = VectorAngles(-vecTank.Normalized());

		pTank->SetOrigin(GetTerrain()->SetPointHeight(vecTank ));
		pTank->SetAngles(angTank);
		pTank->GiveBonusPoints(1, false);

		pTank = Game()->Create<CMechInfantry>("CMechInfantry");
		m_ahTeams[i]->AddEntity(pTank);

		vecTank = Vector(80, 0, 60)* flReflection;
		angTank = VectorAngles(-vecTank.Normalized());

		pTank->SetOrigin(GetTerrain()->SetPointHeight(vecTank));
		pTank->SetAngles(angTank);
		pTank->GiveBonusPoints(1, false);

		pTank = Game()->Create<CMechInfantry>("CMechInfantry");
		m_ahTeams[i]->AddEntity(pTank);

		vecTank = Vector(60, 0, 80) * flReflection;
		angTank = VectorAngles(-vecTank.Normalized());

		pTank->SetOrigin(GetTerrain()->SetPointHeight(vecTank));
		pTank->SetAngles(angTank);
		pTank->GiveBonusPoints(1, false);

		pTank = Game()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[i]->AddEntity(pTank);

		vecTank = Vector(80, 0, 80) * flReflection;
		angTank = VectorAngles(-vecTank.Normalized());

		pTank->SetOrigin(GetTerrain()->SetPointHeight(vecTank));
		pTank->SetAngles(angTank);
		pTank->GiveBonusPoints(1, false);

		pTank = Game()->Create<CArtillery>("CArtillery");
		m_ahTeams[i]->AddEntity(pTank);

		vecTank = Vector(60, 0, 100) * flReflection;
		angTank = VectorAngles(-vecTank.Normalized());

		pTank->SetOrigin(GetTerrain()->SetPointHeight(vecTank));
		pTank->SetAngles(angTank);
		pTank->GiveBonusPoints(1, false);

		pTank = Game()->Create<CArtillery>("CArtillery");
		m_ahTeams[i]->AddEntity(pTank);

		vecTank = Vector(100, 0, 60) * flReflection;
		angTank = VectorAngles(-vecTank.Normalized());

		pTank->SetOrigin(GetTerrain()->SetPointHeight(vecTank));
		pTank->SetAngles(angTank);
		pTank->GiveBonusPoints(1, false);

		flReflection = -flReflection;
	}

	m_ahTeams[0]->SetClient(-1);

	CPowerup* pPowerup = Game()->Create<CPowerup>("CPowerup");
	pPowerup->SetOrigin(Vector(70, m_hTerrain->GetHeight(70, 70), 70));
	pPowerup = Game()->Create<CPowerup>("CPowerup");
	pPowerup->SetOrigin(Vector(70, m_hTerrain->GetHeight(70, -70), -70));
	pPowerup = Game()->Create<CPowerup>("CPowerup");
	pPowerup->SetOrigin(Vector(-70, m_hTerrain->GetHeight(-70, 70), 70));
	pPowerup = Game()->Create<CPowerup>("CPowerup");
	pPowerup->SetOrigin(Vector(-70, m_hTerrain->GetHeight(-70, -70), -70));

	m_iPowerups = 4;

	StartGame();
}

void CDigitanksGame::SetupEntities()
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	CNetwork::CallFunction(-1, "SetupEntities");

	CNetworkParameters p;
	SetupEntities(&p);
}

void CDigitanksGame::SetupEntities(CNetworkParameters* p)
{
	CSoundLibrary::StopSound();
	CParticleSystemLibrary::ClearInstances();

	for (size_t i = 0; i < m_ahTeams.size(); i++)
		m_ahTeams[i]->Delete();

	m_ahTeams.clear();

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i))->Delete();
}

void CDigitanksGame::StartGame()
{
	m_iCurrentTeam = 0;

	GetCurrentTeam()->PreStartTurn();
	GetCurrentTeam()->StartTurn();
	GetCurrentTeam()->PostStartTurn();

	CNetwork::CallFunction(-1, "SetCurrentTeam", 0);

	EnterGame(NULL);

	m_bLoading = false;
}

void CDigitanksGame::EnterGame(CNetworkParameters* p)
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	if (CNetwork::IsHost())
		CNetwork::CallFunction(-1, "EnterGame");

	m_iCurrentSelection = 0;
	m_bWaitingForMoving = false;
	m_bWaitingForProjectiles = false;

	if (m_pListener)
	{
		m_pListener->GameStart();

		m_pListener->SetHUDActive(true);
		m_pListener->NewCurrentTeam();

		m_pListener->NewCurrentSelection();
	}

	// Point the camera in to the center
	EAngle angCamera = VectorAngles(GetCurrentSelection()->GetOrigin().Normalized());
	angCamera.p = 45;
	GetCamera()->SnapAngle(angCamera);
}

void CDigitanksGame::Think()
{
	BaseClass::Think();

	if (m_bWaitingForMoving)
	{
		bool bMoving = false;
		for (size_t i = 0; i < GetCurrentTeam()->GetNumTanks(); i++)
		{
			if (GetCurrentTeam()->GetTank(i)->IsMoving())
			{
				bMoving = true;
				break;
			}
		}

		if (!bMoving)
		{
			m_bWaitingForMoving = false;
			m_iWaitingForProjectiles = 0;
			GetCurrentTeam()->FireTanks();
			m_bWaitingForProjectiles = true;

			size_t iSum = 0;
			Vector vecSum;
			for (size_t i = 0; i < GetCurrentTeam()->GetNumTanks(); i++)
			{
				CDigitank* pTank = GetCurrentTeam()->GetTank(i);
				if (!pTank)
					continue;
				if (!pTank->HasDesiredAim())
					continue;
				vecSum += pTank->GetDesiredMove() + pTank->GetDesiredAim();
				iSum += 2;
			}

			if (iSum)
				GetCamera()->SetTarget(vecSum/(float)iSum);
		}
	}

	if (m_bWaitingForProjectiles)
	{
		if (m_iWaitingForProjectiles == 0)
		{
			m_bWaitingForProjectiles = false;
			StartTurn();
		}
	}
}

void CDigitanksGame::SetDesiredMove(bool bAllTanks)
{
	if (!GetCurrentSelection())
		return;

	CDigitank* pCurrentTank = dynamic_cast<CDigitank*>(GetCurrentSelection());
	if (!pCurrentTank)
		return;

	if (bAllTanks)
	{
		if (!pCurrentTank->IsPreviewMoveValid())
			return;

		Vector vecPreview = pCurrentTank->GetPreviewMove();
		Vector vecOrigin = pCurrentTank->GetOrigin();
		Vector vecMove = vecPreview - vecOrigin;

		CDigitanksTeam* pTeam = GetCurrentTeam();
		for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
		{
			CDigitank* pTank = pTeam->GetTank(i);

			Vector vecTankMove = vecMove;

			Vector vecNewPosition = pTank->GetOrigin() + vecTankMove;
			vecNewPosition.y = pTank->FindHoverHeight(vecNewPosition);

			pTank->SetPreviewMove(vecNewPosition);

			do
			{
				if (pTank->GetPreviewMovePower() > pTank->GetMaxMovementDistance())
					vecTankMove = vecTankMove * 0.95f;

				if (vecTankMove.Length() < 1)
					break;

				vecNewPosition = pTank->GetOrigin() + vecTankMove;
				vecNewPosition.y = pTank->FindHoverHeight(vecNewPosition);

				pTank->SetPreviewMove(vecNewPosition);
			}
			while (pTank->GetPreviewMovePower() > pTank->GetMaxMovementDistance());

			pTank->SetDesiredMove();
		}
	}
	else
		pCurrentTank->SetDesiredMove();
}

void CDigitanksGame::SetDesiredTurn(bool bAllTanks, Vector vecLookAt)
{
	if (!GetCurrentSelection())
		return;

	CDigitank* pCurrentTank = dynamic_cast<CDigitank*>(GetCurrentSelection());
	if (!pCurrentTank)
		return;

	if (bAllTanks)
	{
		bool bNoTurn = (vecLookAt - pCurrentTank->GetDesiredMove()).LengthSqr() < 4*4;

		CDigitanksTeam* pTeam = GetCurrentTeam();
		for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
		{
			CDigitank* pTank = pTeam->GetTank(i);

			if (bNoTurn)
				pTank->SetPreviewTurn(pTank->GetAngles().y);
			else
			{
				Vector vecDirection = (vecLookAt - pTank->GetDesiredMove()).Normalized();
				float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

				float flTankTurn = AngleDifference(flYaw, pTank->GetAngles().y);
				if (pTank->GetPreviewMovePower() + fabs(flTankTurn)/pTank->TurnPerPower() > pTank->GetTotalMovementPower())
					flTankTurn = (flTankTurn / fabs(flTankTurn)) * (pTank->GetTotalMovementPower() - pTank->GetPreviewMovePower()) * pTank->TurnPerPower() * 0.95f;

				pTank->SetPreviewTurn(pTank->GetAngles().y + flTankTurn);
			}

			pTank->SetDesiredTurn();
		}
	}
	else
		pCurrentTank->SetDesiredTurn();
}

void CDigitanksGame::SetDesiredAim(bool bAllTanks)
{
	if (!GetCurrentSelection())
		return;

	CDigitank* pCurrentTank = dynamic_cast<CDigitank*>(GetCurrentSelection());
	if (!pCurrentTank)
		return;

	if (bAllTanks)
	{
		Vector vecPreviewAim = pCurrentTank->GetPreviewAim();

		CDigitanksTeam* pTeam = GetCurrentTeam();
		for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
		{
			CDigitank* pTank = pTeam->GetTank(i);

			Vector vecTankAim = vecPreviewAim;
			if ((vecTankAim - pTank->GetDesiredMove()).Length() > pTank->GetMaxRange())
			{
				vecTankAim = pTank->GetDesiredMove() + (vecTankAim - pTank->GetDesiredMove()).Normalized() * pTank->GetMaxRange() * 0.99f;
				vecTankAim.y = pTank->FindHoverHeight(vecTankAim);
			}

			pTank->SetPreviewAim(vecTankAim);
			pTank->SetDesiredAim();
		}
	}
	else
		pCurrentTank->SetDesiredAim();
}

void CDigitanksGame::NextTank()
{
	assert(GetCurrentSelection());
	if (!GetCurrentSelection())
		return;

	do
	{
		if (++m_iCurrentSelection >= GetCurrentTeam()->GetNumMembers())
			m_iCurrentSelection = 0;
	}
	while (!GetCurrentSelection());

	if (GetCurrentSelection())
		GetCurrentSelection()->OnCurrentSelection();

	if (m_pListener)
		m_pListener->NewCurrentSelection();
}

void CDigitanksGame::EndTurn()
{
	if (m_bWaitingForProjectiles || m_bWaitingForMoving)
		return;

	CNetwork::CallFunction(-1, "EndTurn");

	EndTurn(NULL);
}

void CDigitanksGame::EndTurn(CNetworkParameters* p)
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	if (m_pListener)
		m_pListener->SetHUDActive(false);

	GetCurrentTeam()->MoveTanks();
	m_bWaitingForMoving = true;

	size_t iSum = 0;
	Vector vecSum;
	for (size_t i = 0; i < GetCurrentTeam()->GetNumTanks(); i++)
	{
		CDigitank* pTank = GetCurrentTeam()->GetTank(i);
		if (!pTank)
			continue;
		vecSum += pTank->GetDesiredMove();
		iSum += 1;
	}

	if (iSum)
		GetCamera()->SetTarget(vecSum/(float)iSum);
}

void CDigitanksGame::StartTurn()
{
	if (!CNetwork::IsHost())
		return;

	if (m_iPowerups < 10 && rand()%6 == 0)
	{
		float flX = RemapVal((float)(rand()%1000), 0, 1000, -GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize());
		float flZ = RemapVal((float)(rand()%1000), 0, 1000, -GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize());

		CPowerup* pPowerup = Game()->Create<CPowerup>("CPowerup");
		pPowerup->SetOrigin(Vector(flX, m_hTerrain->GetHeight(flX, flZ), flZ));

		m_iPowerups++;
	}

	CNetwork::CallFunction(-1, "StartTurn");

	StartTurn(NULL);
}

void CDigitanksGame::StartTurn(CNetworkParameters* p)
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	m_iCurrentSelection = 0;

	if (++m_iCurrentTeam >= GetNumTeams())
		m_iCurrentTeam = 0;

	if (m_pListener)
	{
		m_pListener->SetHUDActive(true);
		m_pListener->NewCurrentTeam();
	}

	GetCurrentTeam()->PreStartTurn();
	GetCurrentTeam()->StartTurn();
	GetCurrentTeam()->PostStartTurn();

	if (GetCurrentSelection())
		GetCurrentSelection()->OnCurrentSelection();

	if (GetCurrentTeam()->IsPlayerControlled())
	{
		if (m_pListener)
			m_pListener->NewCurrentSelection();
	}
	else if (CNetwork::IsHost())
		Bot_ExecuteTurn();
}

void CDigitanksGame::Bot_ExecuteTurn()
{
	CDigitank* pHeadTank = GetCurrentTeam()->GetTank(0);

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

		if (GetCurrentTeam()->GetEntityVisibility(pTank->GetHandle()) == 0)
			continue;

		if (!pTarget)
		{
			pTarget = pTank;
			continue;
		}

		if ((pHeadTank->GetOrigin() - pTank->GetOrigin()).Length2DSqr() < (pHeadTank->GetOrigin() - pTarget->GetOrigin()).Length2DSqr())
			pTarget = pTank;
	}

	if (!pTarget)
	{
		EndTurn();
		return;
	}

	Vector vecFortifyPoint;

	// Find the closest fortification point. Each enemy tank is examined to find the closest spot outside of all enemy tank ranges.
	for (size_t i = 0; i < pTarget->GetDigitanksTeam()->GetNumTanks(); i++)
	{
		CDigitank* pTank = pTarget->GetDigitanksTeam()->GetTank(i);

		// Artillery isn't a big deal, and they have huge ranges.
		if (pTank->IsArtillery())
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

	size_t iFortifies = 0;

	for (size_t i = 0; i < GetCurrentTeam()->GetNumTanks(); i++)
	{
		CDigitank* pTank = GetCurrentTeam()->GetTank(i);

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
			if (pTank->IsFortified() && (pTank->GetOrigin() - pTarget->GetOrigin()).Length() < pTank->GetMinRange())
				pTank->Fortify();

			if (!pTank->IsFortified())
			{
				if ((pTank->GetOrigin() - pTarget->GetOrigin()).Length() > pTank->GetMinRange())
				{
					// Deploy so we can rain some hell down.
					pTank->SetPreviewTurn(VectorAngles(pTarget->GetOrigin() - pTank->GetOrigin()).y);
					pTank->SetDesiredTurn();
					pTank->Fortify();
				}

				// Head away from enemies at full speed
				float flMovementDistance = pTank->GetMaxMovementDistance();
				Vector vecDirection = pTarget->GetOrigin() - pTank->GetOrigin();
				vecDirection = -vecDirection.Normalized() * (flMovementDistance*0.90f);

				Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
				vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->SetDesiredMove();

				pTank->SetPreviewTurn(VectorAngles(pTarget->GetOrigin() - pTank->GetDesiredMove()).y);
				pTank->SetDesiredTurn();
			}

			// If we are within the max range, try to fire.
			if ((pTarget->GetOrigin() - pTank->GetPreviewMove()).LengthSqr() < pTank->GetMaxRange()*pTank->GetMaxRange())
			{
				pTank->SetPreviewAim(GetTerrain()->SetPointHeight(pTarget->GetOrigin()));
				pTank->SetDesiredAim();
			}
		}
		else if (pTank->CanFortify())
		{
			Vector vecTankFortifyPoint;
			if (iFortifies == 0)
				vecTankFortifyPoint = vecFortifyPoint;
			else if (iFortifies%2 == 0)
			{
				Vector vecSide = (pTarget->GetOrigin() - vecFortifyPoint).Normalized().Cross(Vector(0,1,0)).Normalized();
				vecTankFortifyPoint = vecFortifyPoint + vecSide * (float)((iFortifies/2)*pTank->GetMaxRange()/2);
			}
			else
			{
				Vector vecSide = -(pTarget->GetOrigin() - vecFortifyPoint).Normalized().Cross(Vector(0,1,0)).Normalized();
				vecTankFortifyPoint = vecFortifyPoint + vecSide * (float)((iFortifies/2+1)*pTank->GetMaxRange()/2);
			}

			GetTerrain()->SetPointHeight(vecTankFortifyPoint);

			pTank->SetFortifyPoint(vecTankFortifyPoint);

			iFortifies++;

			//if (pTank->IsFortified() && (pTank->GetOrigin() - vecTankFortifyPoint).Length2D() > pTank->GetMaxMovementDistance()*2)
			//	pTank->Fortify();

			// If our target is behind us, we have to mobilize so we can turn around to face them.
			if (pTank->IsFortified() && (pTarget->GetOrigin() - pTank->GetOrigin()).Normalized().Dot(AngleVector(pTank->GetAngles())) < 0)
				pTank->Fortify();

			if (!pTank->IsFortified() && (pTank->GetOrigin() - vecTankFortifyPoint).Length2D() < pTank->GetBoundingRadius()*2)
			{
				// Face the enemy and fortify before he gets here.
				pTank->SetPreviewTurn(VectorAngles(pTarget->GetOrigin() - pTank->GetOrigin()).y);
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

				pTank->SetPreviewTurn(VectorAngles(pTarget->GetOrigin() - pTank->GetDesiredMove()).y);
				pTank->SetDesiredTurn();
			}
		}
		else
		{
			// If we are not within the effective range, use 1/3 of our available movement power to move towards our target.
			if ((pTarget->GetOrigin() - pTank->GetOrigin()).LengthSqr() > pTank->GetEffRange()*pTank->GetEffRange())
			{
				float flMovementDistance = pTank->GetMaxMovementDistance();
				Vector vecDirection = pTarget->GetOrigin() - pTank->GetOrigin();
				vecDirection = vecDirection.Normalized() * (flMovementDistance/3);

				Vector vecDesiredMove = pTank->GetOrigin() + vecDirection;
				vecDesiredMove.y = pTank->FindHoverHeight(vecDesiredMove);

				pTank->SetPreviewMove(vecDesiredMove);
				pTank->SetDesiredMove();
			}

			// If we are within the max range, try to fire.
			if ((pTarget->GetOrigin() - pTank->GetPreviewMove()).LengthSqr() < pTank->GetMaxRange()*pTank->GetMaxRange())
			{
				pTank->SetPreviewAim(GetTerrain()->SetPointHeight(pTarget->GetOrigin()));
				pTank->SetDesiredAim();
			}
		}

		if (pTarget->IsFortified())
			pTank->SetAttackPower((pTank->GetBasePower() - pTank->GetBaseMovementPower())*3/4);
		else
			pTank->SetAttackPower((pTank->GetBasePower() - pTank->GetBaseMovementPower())/2);
	}

	EndTurn();
}

bool CDigitanksGame::Explode(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flRadius, float flDamage, CBaseEntity* pIgnore, CTeam* pTeamIgnore)
{
	std::vector<CBaseEntity*> apHit;
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);

		if (!pEntity)
			continue;

		if (pEntity == pIgnore)
			continue;

		// Fire too close to yourself and the explosion can rock you.
		if (pEntity != pAttacker)
		{
			if (dynamic_cast<CDigitank*>(pEntity) && dynamic_cast<CDigitank*>(pEntity)->GetTeam() == pTeamIgnore)
				continue;

			if (!pInflictor->ShouldTouch(pEntity))
				continue;
		}

		float flDistanceSqr = (pInflictor->GetOrigin() - pEntity->GetOrigin()).LengthSqr();

		float flTotalRadius = flRadius + pEntity->GetBoundingRadius();
		if (flDistanceSqr < flTotalRadius*flTotalRadius)
			apHit.push_back(pEntity);
	}

	bool bHit = false;

	for (size_t i = 0; i < apHit.size(); i++)
	{
		float flDistance = (pInflictor->GetOrigin() - apHit[i]->GetOrigin()).Length();

		float flFalloffDamage = RemapVal(flDistance, 0, flRadius + apHit[i]->GetBoundingRadius(), flDamage, 0);
		if (flFalloffDamage <= 0)
			continue;

		bHit = true;

		apHit[i]->TakeDamage(pAttacker, pInflictor, flFalloffDamage, false);
	}

	return bHit;
}

void CDigitanksGame::OnTakeShieldDamage(CDigitank* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly)
{
	if (m_pListener)
		m_pListener->OnTakeShieldDamage(pVictim, pAttacker, pInflictor, flDamage, bDirectHit, bShieldOnly);
}

void CDigitanksGame::OnTakeDamage(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled)
{
	if (m_pListener)
		m_pListener->OnTakeDamage(pVictim, pAttacker, pInflictor, flDamage, bDirectHit, bKilled);
}

void CDigitanksGame::OnKilled(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
		m_ahTeams[i]->OnKilled(pEntity);

	CheckWinConditions();
}

void CDigitanksGame::CheckWinConditions()
{
	bool bPlayerLost = false;

	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (GetDigitanksTeam(i)->GetNumTanksAlive() == 0)
		{
			m_ahTeams[i]->Delete();
			m_ahTeams.erase(m_ahTeams.begin()+i);

			if (i == 0)
				bPlayerLost = true;
		}
	}

	if (bPlayerLost || m_ahTeams.size() <= 1)
	{
		if (m_pListener)
			m_pListener->GameOver(!bPlayerLost);
	}
}

void CDigitanksGame::OnDeleted(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
		m_ahTeams[i]->OnDeleted(pEntity);

	if (dynamic_cast<class CProjectile*>(pEntity) != NULL)
	{
		if (m_iWaitingForProjectiles > 0)
			m_iWaitingForProjectiles--;
	}

	if (dynamic_cast<class CPowerup*>(pEntity) != NULL)
		m_iPowerups--;
}

void CDigitanksGame::TankSpeak(class CDigitank* pTank, const std::string& sSpeech)
{
	if (m_pListener)
		m_pListener->TankSpeak(pTank, sSpeech);
}

CDigitanksTeam* CDigitanksGame::GetDigitanksTeam(size_t i)
{
	return dynamic_cast<CDigitanksTeam*>(BaseClass::GetTeam(i));
}

CDigitanksTeam* CDigitanksGame::GetCurrentTeam()
{
	if (m_iCurrentTeam >= m_ahTeams.size())
		return NULL;

	return dynamic_cast<CDigitanksTeam*>(m_ahTeams[m_iCurrentTeam].GetPointer());
}

CSelectable* CDigitanksGame::GetCurrentSelection()
{
	if (!GetCurrentTeam())
		return NULL;

	if (m_iCurrentSelection >= GetCurrentTeam()->GetNumMembers())
		return NULL;

	CBaseEntity* pEntity = GetCurrentTeam()->GetMember(m_iCurrentSelection);

	return dynamic_cast<CSelectable*>(pEntity);
}

CDigitank* CDigitanksGame::GetCurrentTank()
{
	return dynamic_cast<CDigitank*>(GetCurrentSelection());
}

CStructure* CDigitanksGame::GetCurrentStructure()
{
	return dynamic_cast<CStructure*>(GetCurrentSelection());
}

size_t CDigitanksGame::GetCurrentSelectionId()
{
	return m_iCurrentSelection;
}

bool CDigitanksGame::IsCurrentSelection(const CSelectable* pEntity)
{
	return GetCurrentSelection() == pEntity;
}

controlmode_t CDigitanksGame::GetControlMode()
{
	if (IsLoading())
		return MODE_NONE;

	if (IsTeamControlledByMe(GetCurrentTeam()))
		return m_eControlMode;

	return MODE_NONE;
}

void CDigitanksGame::SetControlMode(controlmode_t eMode, bool bAutoProceed)
{
	if (!GetCurrentSelection())
		return;

	if (CDigitanksWindow::Get()->GetVictoryPanel()->IsVisible())
		return;

	GetCurrentSelection()->OnControlModeChange(m_eControlMode, eMode);

	if (eMode == MODE_MOVE)
	{
		GetCamera()->SetDistance(100);
		CDigitanksWindow::Get()->GetInstructor()->DisplayTutorial(CInstructor::TUTORIAL_MOVE);
	}

	if (eMode == MODE_TURN)
	{
		GetCamera()->SetDistance(80);
//		CDigitanksWindow::Get()->GetInstructor()->DisplayTutorial(CInstructor::TUTORIAL_TURN);
	}

	if (eMode == MODE_AIM)
	{
		GetCamera()->SetDistance(140);
		CDigitanksWindow::Get()->GetInstructor()->DisplayTutorial(CInstructor::TUTORIAL_AIM);
	}

	if (eMode == MODE_FIRE)
	{
		GetCamera()->SetDistance(80);
		CDigitanksWindow::Get()->GetInstructor()->DisplayTutorial(CInstructor::TUTORIAL_POWER);
	}

	if (eMode == MODE_NONE)
		GetCamera()->SetDistance(100);

	m_eControlMode = eMode;
}

void CDigitanksGame::SetTerrain(CNetworkParameters* p)
{
	m_hTerrain = CEntityHandle<CTerrain>(p->ui1);
}

void CDigitanksGame::SetCurrentTeam(CNetworkParameters* p)
{
	m_iCurrentTeam = p->i1;
}

void CDigitanksGame::CreateRenderer()
{
	m_pRenderer = new CDigitanksRenderer();
}

CDigitanksRenderer*	CDigitanksGame::GetDigitanksRenderer()
{
	return dynamic_cast<CDigitanksRenderer*>(GetRenderer());
}

float CDigitanksGame::GetGravity()
{
	return -20;
}

void CDigitanksGame::AddTankAim(Vector vecAim, float flRadius, bool bFocus)
{
	vecAim.y = 0;
	m_avecTankAims.push_back(vecAim);
	m_aflTankAimRadius.push_back(flRadius);
	if (bFocus)
		m_iTankAimFocus = m_avecTankAims.size()-1;
}

void CDigitanksGame::GetTankAims(std::vector<Vector>& avecAims, std::vector<float>& aflAimRadius, size_t& iFocus)
{
	avecAims = m_avecTankAims;
	aflAimRadius = m_aflTankAimRadius;
	iFocus = m_iTankAimFocus;
}

void CDigitanksGame::ClearTankAims()
{
	m_avecTankAims.clear();
	m_aflTankAimRadius.clear();
}
