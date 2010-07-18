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
#include "digitanks/loader.h"

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

void CDigitanksGame::SetupGame()
{
	m_bLoading = true;

	SetupEntities();

	if (!CNetwork::IsHost())
		return;

	m_hTerrain = Game()->Create<CTerrain>("CTerrain");

	for (int i = (int)-m_hTerrain->GetMapSize(); i < (int)m_hTerrain->GetMapSize(); i += 50)
	{
		for (int j = (int)-m_hTerrain->GetMapSize(); j < (int)m_hTerrain->GetMapSize(); j += 50)
		{
			if (rand()%2 == 0)
				continue;

			float x = RemapVal((float)(rand()%1000), 0, 1000, (float)i, (float)i+50);
			float z = RemapVal((float)(rand()%1000), 0, 1000, (float)j, (float)j+50);

			if (x < -m_hTerrain->GetMapSize()+10 || z < -m_hTerrain->GetMapSize()+10)
				continue;

			if (x > m_hTerrain->GetMapSize()-10 || z > m_hTerrain->GetMapSize()-10)
				continue;

			CResource* pResource = Game()->Create<CResource>("CResource");
			pResource->SetOrigin(m_hTerrain->SetPointHeight(Vector(x, 0, z)));
		}
	}

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
		float y = RemapVal((float)(rand()%1000), 0, 1000, 0, 360);
		pResource->SetOrigin(m_hTerrain->SetPointHeight(pCPU->GetOrigin() + AngleVector(EAngle(0, y, 0)) * 20));
		pResource->SetProduction(4);

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

		pTank = Game()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[i]->AddEntity(pTank);

		vecTank = avecStartingPositions[i] + vecForward * 30;
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
	{
		pCurrentTank->SetDesiredMove();

		if (!pCurrentTank->HasDesiredMove())
			pCurrentTank->SetGoalMovePosition(pCurrentTank->GetPreviewMove());
	}
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
	size_t iOriginal = m_iCurrentSelection;
	while ((m_iCurrentSelection = ++m_iCurrentSelection%GetCurrentTeam()->GetNumMembers()) != iOriginal)
	{
		if (!GetCurrentTank())
			continue;

		if (GetCurrentTank()->IsFortified())
			continue;

		if (GetCurrentTank()->HasGoalMovePosition())
			continue;

		break;
	}

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

	size_t iSum = 0;
	Vector vecSum;
	for (size_t i = 0; i < GetCurrentTeam()->GetNumTanks(); i++)
	{
		CDigitank* pTank = GetCurrentTeam()->GetTank(i);
		if (!pTank)
			continue;

		if (!pTank->HasDesiredMove())
			continue;

		if (pTank->GetVisibility() == 0)
			continue;

		vecSum += pTank->GetDesiredMove();
		iSum += 1;
	}

	if (iSum)
		GetCamera()->SetTarget(vecSum/(float)iSum);

	GetCurrentTeam()->MoveTanks();
	m_bWaitingForMoving = true;

	if (m_pListener)
		m_pListener->SetHUDActive(false);
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

	if (m_pListener)
		m_pListener->ClearTurnInfo();

	m_iCurrentSelection = 0;

	if (++m_iCurrentTeam >= GetNumTeams())
		m_iCurrentTeam = 0;

	GetCurrentTeam()->StartTurn();

	if (m_pListener)
	{
		m_pListener->SetHUDActive(true);
		m_pListener->NewCurrentTeam();
	}

	if (GetCurrentSelection())
		GetCurrentSelection()->OnCurrentSelection();

	if (GetCurrentTeam()->IsPlayerControlled())
	{
		// Find the first selectable tank.
		NextTank();
	}
	else if (CNetwork::IsHost())
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

void CDigitanksGame::SetCurrentSelection(CSelectable* pCurrent)
{
	for (size_t i = 0; i < GetCurrentTeam()->GetNumMembers(); i++)
	{
		CBaseEntity* pMember = GetCurrentTeam()->GetMember(i);
		if (!pMember)
			continue;

		CSelectable* pSelectable = dynamic_cast<CSelectable*>(pMember);
		if (!pSelectable)
			continue;

		if (pSelectable == pCurrent)
		{
			m_iCurrentSelection = i;
			break;
		}
	}

	if (GetCurrentSelection())
		GetCurrentSelection()->OnCurrentSelection();

	if (m_pListener)
		m_pListener->NewCurrentSelection();
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

	if (!GetCurrentSelection()->OnControlModeChange(m_eControlMode, eMode))
		return;

	if (eMode == MODE_MOVE)
	{
		CDigitanksWindow::Get()->GetInstructor()->DisplayTutorial(CInstructor::TUTORIAL_MOVE);
	}

	if (eMode == MODE_TURN)
	{
//		CDigitanksWindow::Get()->GetInstructor()->DisplayTutorial(CInstructor::TUTORIAL_TURN);
	}

	if (eMode == MODE_AIM)
	{
		CDigitanksWindow::Get()->GetInstructor()->DisplayTutorial(CInstructor::TUTORIAL_AIM);
	}

	if (eMode == MODE_FIRE)
	{
		CDigitanksWindow::Get()->GetInstructor()->DisplayTutorial(CInstructor::TUTORIAL_POWER);
	}

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

void CDigitanksGame::AppendTurnInfo(const char* pszTurnInfo)
{
	if (m_pListener)
		m_pListener->AppendTurnInfo(pszTurnInfo);
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
