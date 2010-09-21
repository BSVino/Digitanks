#include "digitanksgame.h"

#include <assert.h>
#include <maths.h>
#include <mtrand.h>
#include <models/models.h>
#include <sound/sound.h>
#include <renderer/particles.h>

#include <network/network.h>
#include <ui/digitankswindow.h>
#include <ui/menu.h>
#include <ui/instructor.h>
#include <ui/hud.h>
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
#include "digitanks/loader.h"

CDigitanksGame::CDigitanksGame()
{
	m_iCurrentTeam = 0;

	m_pListener = NULL;

	m_bWaitingForMoving = false;

	m_bWaitingForProjectiles = false;
	m_iWaitingForProjectiles = 0;

	m_bTurnActive = true;

	m_iPowerups = 0;

	m_iDifficulty = 1;

	m_bRenderFogOfWar = true;
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

void CDigitanksGame::SetupGame(gametype_t eGameType)
{
	m_bLoading = true;

	SetupEntities();

	if (!CNetwork::IsHost())
		return;

	m_hTerrain = Game()->Create<CTerrain>("CTerrain");

	m_eGameType = eGameType;
	m_iTurn = 0;

	if (eGameType == GAMETYPE_STANDARD)
		SetupStandard();
	else if (eGameType == GAMETYPE_ARTILLERY)
		SetupArtillery();
	else if (eGameType == GAMETYPE_TUTORIAL)
		SetupTutorial();

	StartGame();
}

void CDigitanksGame::ScatterResources()
{
	for (int i = (int)-m_hTerrain->GetMapSize(); i < (int)m_hTerrain->GetMapSize(); i += 50)
	{
		for (int j = (int)-m_hTerrain->GetMapSize(); j < (int)m_hTerrain->GetMapSize(); j += 50)
		{
			if (rand()%2 == 0)
				continue;

			float x = RandomFloat((float)i, (float)i+50);
			float z = RandomFloat((float)j, (float)j+50);

			if (x < -m_hTerrain->GetMapSize()+10 || z < -m_hTerrain->GetMapSize()+10)
				continue;

			if (x > m_hTerrain->GetMapSize()-10 || z > m_hTerrain->GetMapSize()-10)
				continue;

			CResource* pResource = Game()->Create<CResource>("CResource");
			pResource->SetOrigin(m_hTerrain->SetPointHeight(Vector(x, 0, z)));
		}
	}
}

void CDigitanksGame::SetupArtillery()
{
	int iPlayers = 8;
	if (iPlayers > 8)
		iPlayers = 8;
	if (iPlayers < 2)
		iPlayers = 2;

	int iTanks = 5;
	if (iTanks > 5)
		iTanks = 5;
	if (iTanks < 1)
		iTanks = 1;

	Vector avecStartingPositions[] =
	{
		Vector(45, 0, 110),
		Vector(-45, 0, 110),
		Vector(-110, 0, 45),
		Vector(-110, 0, -45),
		Vector(-45, 0, -110),
		Vector(45, 0, -110),
		Vector(110, 0, -45),
		Vector(110, 0, 45),
	};

	Vector avecTankPositions[] =
	{
		Vector(0, 0, 0),
		Vector(15, 0, 15),
		Vector(-15, 0, -15),
		Vector(15, 0, -15),
		Vector(-15, 0, 15),
	};

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

	std::vector<Vector> avecRandomStartingPositions;
	for (int i = 0; i < iPlayers; i++)
	{
		// 8 random starting positions.
		if (rand()%2)
			avecRandomStartingPositions.push_back(avecStartingPositions[i]);
		else
			avecRandomStartingPositions.insert(avecRandomStartingPositions.begin(), avecStartingPositions[i]);
	}

	for (int i = 0; i < iPlayers; i++)
	{
		m_ahTeams.push_back(Game()->Create<CDigitanksTeam>("CDigitanksTeam"));

		m_ahTeams[i]->SetColor(aclrTeamColors[i]);

		for (int j = 0; j < iTanks; j++)
		{
			Vector vecTank = avecRandomStartingPositions[i] + avecTankPositions[j];
			EAngle angTank = VectorAngles(-vecTank.Normalized());

			CDigitank* pTank = Game()->Create<CMainBattleTank>("CMainBattleTank");
			m_ahTeams[i]->AddEntity(pTank);

			vecTank.y = pTank->FindHoverHeight(vecTank);

			pTank->SetOrigin(vecTank);
			pTank->SetAngles(angTank);
			pTank->GiveBonusPoints(1, false);
		}
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
}

void CDigitanksGame::SetupStandard()
{
	ScatterResources();

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

	std::wstring aszTeamNames[] =
	{
		L"Blue",
		L"Yellow",
		L"Pink",
		L"Red",
	};

	Vector avecStartingPositions[] =
	{
		Vector(130, 0, 130),
		Vector(130, 0, -130),
		Vector(-130, 0, 130),
		Vector(-130, 0, -130),
	};

	for (int i = 0; i < 4; i++)
	{
		m_ahTeams.push_back(Game()->Create<CDigitanksTeam>("CDigitanksTeam"));

		m_ahTeams[i]->SetColor(aclrTeamColors[i]);
		m_ahTeams[i]->SetName(aszTeamNames[i]);

		CCPU* pCPU = Game()->Create<CCPU>("CCPU");
		pCPU->SetOrigin(GetTerrain()->SetPointHeight(avecStartingPositions[i]));
		m_ahTeams[i]->AddEntity(pCPU);
		pCPU->UpdateTendrils();

		for (size_t j = 0; j < CBaseEntity::GetNumEntities(); j++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(j);
			if (!pEntity)
				continue;

			CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);
			if (!pStructure)
				continue;

			if (pCPU->GetTeam() == pStructure->GetTeam())
				continue;

			// 80 units because the default CPU has a network radius of 40 units,
			// then add 20 because collectors can be built within 20 units and another 20 as a buffer.
			// The idea is, players have to grow their base to find more resources.
			if ((pStructure->GetOrigin() - pCPU->GetOrigin()).Length2D() < 80)
				pEntity->Delete();
		}

		CResource* pResource = Game()->Create<CResource>("CResource");
		float y = RandomFloat(0, 360);
		pResource->SetOrigin(m_hTerrain->SetPointHeight(pCPU->GetOrigin() + AngleVector(EAngle(0, y, 0)) * 20));

		CDigitank* pTank;
		Vector vecTank;
		EAngle angTank;

		Vector vecForward = (Vector(0,0,0) - avecStartingPositions[i]).Normalized();
		Vector vecRight = vecForward.Cross(Vector(0,1,0)).Normalized();

		pTank = Game()->Create<CMechInfantry>("CMechInfantry");
		m_ahTeams[i]->AddEntity(pTank);

		vecTank = avecStartingPositions[i] + vecForward * 20 + vecRight * 20;
		angTank = VectorAngles(-vecTank.Normalized());

		pTank->SetOrigin(GetTerrain()->SetPointHeight(vecTank));
		pTank->SetAngles(angTank);
		pTank->GiveBonusPoints(1, false);

		pTank = Game()->Create<CMechInfantry>("CMechInfantry");
		m_ahTeams[i]->AddEntity(pTank);

		vecTank = avecStartingPositions[i] + vecForward * 20 - vecRight * 20;
		angTank = VectorAngles(-vecTank.Normalized());

		pTank->SetOrigin(GetTerrain()->SetPointHeight(vecTank));
		pTank->SetAngles(angTank);
		pTank->GiveBonusPoints(1, false);
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

	m_hUpdates = Game()->Create<CUpdateGrid>("CUpdateGrid");
	m_hUpdates->SetupStandardUpdates();
}

void CDigitanksGame::SetupTutorial()
{
	m_ahTeams.push_back(Game()->Create<CDigitanksTeam>("CDigitanksTeam"));
	m_ahTeams[0]->SetColor(Color(0, 0, 255));

	m_ahTeams.push_back(Game()->Create<CDigitanksTeam>("CDigitanksTeam"));
	m_ahTeams[1]->SetColor(Color(255, 0, 0));

	m_ahTeams[0]->SetClient(-1);

	m_iPowerups = 0;
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

	m_iWaitingForProjectiles = 0;
	m_bWaitingForProjectiles = true;

	GetCurrentTeam()->StartTurn();

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

	for (size_t i = 0; i < GetNumTeams(); i++)
		GetDigitanksTeam(i)->CountScore();

	m_bWaitingForMoving = false;
	m_bWaitingForProjectiles = false;

	if (m_pListener)
	{
		m_pListener->GameStart();

		m_pListener->SetHUDActive(true);
		m_pListener->NewCurrentTeam();

		m_pListener->NewCurrentSelection();
	}

	if (m_eGameType == GAMETYPE_TUTORIAL)
		GetCamera()->SnapAngle(EAngle(45, 0, 0));
	else if (GetPrimarySelection())
	{
		// Point the camera in to the center
		EAngle angCamera = VectorAngles(GetPrimarySelection()->GetOrigin().Normalized());
		angCamera.p = 45;
		GetCamera()->SnapAngle(angCamera);
	}

	if (GetLocalDigitanksTeam()->GetMember(0))
		GetCamera()->SnapTarget(GetLocalDigitanksTeam()->GetMember(0)->GetOrigin());

	if (m_eGameType == GAMETYPE_STANDARD)
		CDigitanksWindow::Get()->GetStoryPanel()->SetVisible(true);
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
			m_bWaitingForMoving = false;
	}

	if (m_bWaitingForProjectiles)
	{
		if (m_iWaitingForProjectiles == 0)
			m_bWaitingForProjectiles = false;
	}

	if (!m_bTurnActive && !m_bWaitingForMoving && !m_bWaitingForProjectiles)
		StartTurn();
}

void CDigitanksGame::SetDesiredMove()
{
	if (!GetPrimarySelection())
		return;

	CDigitank* pCurrentTank = GetPrimarySelectionTank();
	if (!pCurrentTank)
		return;

	if (pCurrentTank->GetTeam() != GetLocalTeam())
		return;

	bool bMoved = false;

	Vector vecPreview = pCurrentTank->GetPreviewMove();
	Vector vecOrigin = pCurrentTank->GetOrigin();
	Vector vecMove = vecPreview - vecOrigin;

	CDigitanksTeam* pTeam = GetCurrentTeam();
	for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
	{
		CDigitank* pTank = pTeam->GetTank(i);

		if (!pTank->MovesWith(pCurrentTank))
			continue;

		Vector vecTankMove = vecMove;

		Vector vecNewPosition = pTank->GetOrigin() + vecTankMove;
		vecNewPosition.y = pTank->FindHoverHeight(vecNewPosition);

		pTank->SetPreviewMove(vecNewPosition);

		if (!pTank->IsPreviewMoveValid())
		{
			pTank->SetGoalMovePosition(vecNewPosition);
		}
		else
		{
			pTank->SetDesiredMove();

			if (pTank->HasDesiredMove())
			{
				if (pTank == pCurrentTank)
					bMoved = true;
				pTank->Move();
			}
		}
	}

	if (bMoved)
	{
		GetGame()->GetCamera()->SetTarget(pCurrentTank->GetPreviewMove());

		CDigitanksEntity* pClosestEnemy = NULL;
		while (true)
		{
			pClosestEnemy = CBaseEntity::FindClosest<CDigitanksEntity>(pCurrentTank->GetOrigin(), pClosestEnemy);

			if (pClosestEnemy)
			{
				if (pClosestEnemy->GetTeam() == pCurrentTank->GetTeam())
					continue;

				if (!pClosestEnemy->GetTeam())
					continue;

				if ((pClosestEnemy->GetOrigin() - pCurrentTank->GetOrigin()).Length() > pCurrentTank->VisibleRange())
				{
					pClosestEnemy = NULL;
					break;
				}
			}

			break;
		}

		// Only go to aim mode if there is an enemy in range.
		if (pClosestEnemy && GetPrimarySelectionTank()->CanAim())
			SetControlMode(MODE_AIM);
		else
			SetControlMode(MODE_NONE);

		CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_MOVE);
	}
}

void CDigitanksGame::SetDesiredTurn(Vector vecLookAt)
{
	if (!GetPrimarySelection())
		return;

	CDigitank* pCurrentTank = GetPrimarySelectionTank();
	if (!pCurrentTank)
		return;

	if (pCurrentTank->GetTeam() != GetLocalTeam())
		return;

	bool bNoTurn = (vecLookAt - pCurrentTank->GetDesiredMove()).LengthSqr() < 4*4;

	CDigitanksTeam* pTeam = GetCurrentTeam();
	for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
	{
		CDigitank* pTank = pTeam->GetTank(i);

		if (!pTank->TurnsWith(pCurrentTank))
			continue;

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
		pTank->Turn();
	}

	SetControlMode(MODE_NONE);

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_TURN);
}

void CDigitanksGame::SetDesiredAim()
{
	if (!GetPrimarySelection())
		return;

	CDigitank* pCurrentTank = GetPrimarySelectionTank();
	if (!pCurrentTank)
		return;

	if (pCurrentTank->GetTeam() != GetLocalTeam())
		return;

	Vector vecPreviewAim = pCurrentTank->GetPreviewAim();

	CDigitanksTeam* pTeam = GetCurrentTeam();
	for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
	{
		CDigitank* pTank = pTeam->GetTank(i);

		if (!pTank->AimsWith(pCurrentTank))
			continue;

		Vector vecTankAim = vecPreviewAim;
		if ((vecTankAim - pTank->GetDesiredMove()).Length() > pTank->GetMaxRange())
		{
			vecTankAim = pTank->GetDesiredMove() + (vecTankAim - pTank->GetDesiredMove()).Normalized() * pTank->GetMaxRange() * 0.99f;
			vecTankAim.y = pTank->FindHoverHeight(vecTankAim);
		}

		pTank->SetPreviewAim(vecTankAim);
		pTank->SetDesiredAim();
		pTank->Fire();
	}

	SetControlMode(MODE_NONE);

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_AIM);
}

void CDigitanksGame::EndTurn()
{
	CNetwork::CallFunction(-1, "EndTurn");

	EndTurn(NULL);
}

void CDigitanksGame::EndTurn(CNetworkParameters* p)
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	GetCurrentTeam()->EndTurn();

	m_bTurnActive = false;
	m_bWaitingForProjectiles = true;
	m_bWaitingForMoving = true;

	if (m_pListener)
		m_pListener->SetHUDActive(false);

	CheckWinConditions();
}

void CDigitanksGame::StartTurn()
{
	if (!CNetwork::IsHost())
		return;

	if (!CDigitanksWindow::Get()->GetInstructor()->GetActive() && m_iPowerups < 10 && rand()%6 == 0)
	{
		float flX = RandomFloat(-GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize());
		float flZ = RandomFloat(-GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize());

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

	if (m_iCurrentTeam == 0)
		m_iTurn++;

	if (m_pListener)
		m_pListener->ClearTurnInfo();

	if (++m_iCurrentTeam >= GetNumTeams())
		m_iCurrentTeam = 0;

	m_iWaitingForProjectiles = 0;

	m_bTurnActive = true;

	m_aActionItems.clear();

	GetCurrentTeam()->StartTurn();

	if (m_pListener)
	{
		m_pListener->SetHUDActive(true);
		m_pListener->NewCurrentTeam();
	}

	if (GetPrimarySelection())
		GetPrimarySelection()->OnCurrentSelection();

	if (!GetCurrentTeam()->IsPlayerControlled() && CNetwork::IsHost())
		GetCurrentTeam()->Bot_ExecuteTurn();
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
	if (m_eGameType == GAMETYPE_TUTORIAL)
		return;

	bool bPlayerLost = false;
	size_t iTeamsLeft = 0;

	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (m_eGameType == GAMETYPE_STANDARD)
		{
			bool bHasCPU = false;
			for (size_t j = 0; j < m_ahTeams[i]->GetNumMembers(); j++)
			{
				CBaseEntity* pEntity = m_ahTeams[i]->GetMember(j);
				if (dynamic_cast<CCPU*>(pEntity))
				{
					bHasCPU = true;
					iTeamsLeft++;
					break;
				}
			}

			if (!bHasCPU && i == 0)
				bPlayerLost = true;
		}
		else	// Artillery mode
		{
			if (GetDigitanksTeam(i)->GetNumTanksAlive() == 0)
			{
				if (i == 0)
					bPlayerLost = true;
			}
			else
			{
				iTeamsLeft++;
			}
		}
	}

	if (bPlayerLost || iTeamsLeft <= 1)
	{
		if (m_pListener)
			m_pListener->GameOver(!bPlayerLost);
	}
}

void CDigitanksGame::OnDeleted(CBaseEntity* pEntity)
{
	BaseClass::OnDeleted(pEntity);

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

CSelectable* CDigitanksGame::GetPrimarySelection()
{
	if (!GetCurrentTeam())
		return NULL;

	return GetCurrentTeam()->GetPrimarySelection();
}

CDigitank* CDigitanksGame::GetPrimarySelectionTank()
{
	return dynamic_cast<CDigitank*>(GetPrimarySelection());
}

CStructure* CDigitanksGame::GetPrimarySelectionStructure()
{
	return dynamic_cast<CStructure*>(GetPrimarySelection());
}

controlmode_t CDigitanksGame::GetControlMode()
{
	if (IsLoading())
		return MODE_NONE;

	if (IsTeamControlledByMe(GetCurrentTeam()))
		return m_eControlMode;

	return MODE_NONE;
}

void CDigitanksGame::SetControlMode(controlmode_t eMode)
{
	if (!GetPrimarySelection())
		return;

	if (CDigitanksWindow::Get()->GetVictoryPanel()->IsVisible())
		return;

	if (!GetPrimarySelection()->AllowControlMode(eMode))
		return;

	GetPrimarySelection()->OnControlModeChange(m_eControlMode, eMode);

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

void CDigitanksGame::AppendTurnInfo(const wchar_t* pszTurnInfo)
{
	if (m_pListener)
		m_pListener->AppendTurnInfo(pszTurnInfo);
}

void CDigitanksGame::OnDisplayTutorial(size_t iTutorial)
{
	if (iTutorial == CInstructor::TUTORIAL_INTRO_BASICS)
	{
		CDigitank* pTank = Game()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[0]->AddEntity(pTank);

		pTank->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, 0)));

		GetCamera()->SnapTarget(pTank->GetOrigin());
		GetCamera()->SetDistance(100);
		GetCamera()->SetAngle(EAngle(45, 0, 0));
	}
	else if (iTutorial == CInstructor::TUTORIAL_MOVE)
	{
		// Make an enemy for us to clobber. Close enough that moving out of the way won't move us out of range
		CDigitank* pTank = Game()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[1]->AddEntity(pTank);

		pTank->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, -50)));

		GetCamera()->SetTarget(GetDigitanksTeam(0)->GetTank(0)->GetOrigin());
		GetCamera()->SetDistance(100);
		GetCamera()->SetAngle(EAngle(45, 0, 0));
	}
	else if (iTutorial == CInstructor::TUTORIAL_POWERUP)
	{
		CPowerup* pPowerup = Game()->Create<CPowerup>("CPowerup");
		pPowerup->SetOrigin(GetTerrain()->SetPointHeight(GetDigitanksTeam(0)->GetTank(0)->GetOrigin() + Vector(0, 0, -10)));
	}
	else if (iTutorial == CInstructor::TUTORIAL_THEEND_BASICS)
	{
		// So that pressing the escape key works the first time.
		SetControlMode(MODE_NONE);
	}
	else if (iTutorial == CInstructor::TUTORIAL_INTRO_BASES)
	{
		CCPU* pCPU = Game()->Create<CCPU>("CCPU");
		pCPU->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, 0)));
		m_ahTeams[0]->AddEntity(pCPU);
		pCPU->UpdateTendrils();

		CResource* pResource = Game()->Create<CResource>("CResource");
		pResource->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, 20)));

		GetCamera()->SnapTarget(pCPU->GetOrigin());
		GetCamera()->SetDistance(100);
		GetCamera()->SetAngle(EAngle(45, 0, 0));

		EndTurn();	// Force structure height and power updates.

		GetDigitanksTeam(0)->SetPrimarySelection(pCPU);
	}
	else if (iTutorial == CInstructor::TUTORIAL_THEEND_BASES)
	{
		// So that pressing the escape key works the first time.
		SetControlMode(MODE_NONE);
	}

	// Make sure that features now enabled are turned on.
	CDigitanksWindow::Get()->GetHUD()->SetupMenu();
}

bool CDigitanksGame::ShouldRenderFogOfWar()
{
	if (m_eGameType == GAMETYPE_ARTILLERY)
		return false;

	if (m_eGameType == GAMETYPE_TUTORIAL && CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial() <= CInstructor::TUTORIAL_THEEND_BASICS)
		return false;

	else
		return m_bRenderFogOfWar;
}

bool CDigitanksGame::ShouldShowScores()
{
	return m_eGameType == GAMETYPE_STANDARD;
}

bool CDigitanksGame::CanBuildMiniBuffers()
{
	if (m_eGameType == GAMETYPE_TUTORIAL)
		return false;

	return true;
}

bool CDigitanksGame::CanBuildBuffers()
{
	bool bDisableBuffer = CDigitanksWindow::Get()->GetInstructor()->IsFeatureDisabled(DISABLE_BUFFER);
	return !bDisableBuffer;
}

bool CDigitanksGame::CanBuildBatteries()
{
	if (m_eGameType == GAMETYPE_TUTORIAL)
		return false;

	return true;
}

bool CDigitanksGame::CanBuildPSUs()
{
	bool bDisablePSU = CDigitanksWindow::Get()->GetInstructor()->IsFeatureDisabled(DISABLE_PSU);
	return !bDisablePSU;
}

bool CDigitanksGame::CanBuildInfantryLoaders()
{
	bool bDisableLoaders = CDigitanksWindow::Get()->GetInstructor()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

bool CDigitanksGame::CanBuildTankLoaders()
{
	bool bDisableLoaders = CDigitanksWindow::Get()->GetInstructor()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

bool CDigitanksGame::CanBuildArtilleryLoaders()
{
	bool bDisableLoaders = CDigitanksWindow::Get()->GetInstructor()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

void CDigitanksGame::AddActionItem(CSelectable* pUnit, actiontype_t eActionType)
{
	if (!IsTeamControlledByMe(GetCurrentTeam()))
		return;

	m_aActionItems.push_back(actionitem_t());
	actionitem_t* pActionItem = &m_aActionItems[m_aActionItems.size()-1];
	pActionItem->iUnit = pUnit->GetHandle();
	pActionItem->eActionType = eActionType;
}

void CDigitanksGame::CompleteProductions()
{
	for (size_t i = 0; i < GetCurrentTeam()->GetNumMembers(); i++)
	{
		CBaseEntity* pMember = GetCurrentTeam()->GetMember(i);
		if (!pMember)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(pMember);
		if (pStructure)
		{
			if (pStructure->IsConstructing())
				pStructure->AddProduction(pStructure->ConstructionCost());

			if (pStructure->IsInstalling())
				pStructure->AddProduction(pStructure->GetProductionToInstall()+10);

			if (pStructure->IsUpgrading())
				pStructure->AddProduction(pStructure->GetProductionToUpgrade()+10);
		}

		CLoader* pLoader = dynamic_cast<CLoader*>(pMember);
		if (pLoader)
		{
			if (pLoader->IsProducing())
				pLoader->AddProduction(99999);
		}
	}
}

CDigitanksTeam* CDigitanksGame::GetLocalDigitanksTeam()
{
	return dynamic_cast<CDigitanksTeam*>(GetLocalTeam());
}
