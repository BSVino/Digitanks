#include "digitanksgame.h"

#include <assert.h>
#include <maths.h>
#include <mtrand.h>
#include <strutils.h>
#include <datamanager/dataserializer.h>
#include <models/models.h>
#include <sound/sound.h>
#include <renderer/particles.h>
#include <tinker/portals/portal.h>

#include <game/gameserver.h>
#include <network/network.h>
#include <ui/digitankswindow.h>
#include <ui/ui.h>
#include <ui/instructor.h>
#include <ui/hud.h>
#include "powerup.h"
#include "terrain.h"
#include "dt_camera.h"

#include "digitanks/menumarcher.h"
#include "digitanks/units/standardtank.h"
#include "digitanks/units/mechinf.h"
#include "digitanks/units/maintank.h"
#include "digitanks/units/artillery.h"
#include "digitanks/units/scout.h"
#include "digitanks/units/mobilecpu.h"
#include "digitanks/units/barbarians.h"
#include "digitanks/structures/cpu.h"
#include "digitanks/structures/buffer.h"
#include "digitanks/weapons/projectile.h"
#include "digitanks/weapons/specialshells.h"
#include "digitanks/dt_renderer.h"
#include "digitanks/structures/resource.h"
#include "digitanks/structures/loader.h"
#include "digitanks/structures/props.h"
#include "digitanks/digitankslevel.h"

CGame* CreateGame()
{
	return GameServer()->Create<CDigitanksGame>("CDigitanksGame");
}

CRenderer* CreateRenderer()
{
	return new CDigitanksRenderer();
}

CCamera* CreateCamera()
{
	CDigitanksCamera* pCamera = new CDigitanksCamera();
	pCamera->SnapDistance(120);
	return pCamera;
}

CLevel* CreateLevel()
{
	return new CDigitanksLevel();
}

REGISTER_ENTITY(CDigitanksGame);

NETVAR_TABLE_BEGIN(CDigitanksGame);
	NETVAR_DEFINE(size_t, m_iCurrentTeam);
	NETVAR_DEFINE(CEntityHandle<CTerrain>, m_hTerrain);
	NETVAR_DEFINE(size_t, m_iDifficulty);
	NETVAR_DEFINE(bool, m_bRenderFogOfWar);
	NETVAR_DEFINE(gametype_t, m_eGameType);
	NETVAR_DEFINE(size_t, m_iTurn);
	NETVAR_DEFINE(CEntityHandle<CUpdateGrid>, m_hUpdates);
	NETVAR_DEFINE(bool, m_bPartyMode);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitanksGame);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iCurrentTeam);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, controlmode_t, m_eControlMode);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CTerrain>, m_hTerrain);
//	IDigitanksGameListener*	m_pListener;	// Set by constructor
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bWaitingForMoving);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bWaitingForProjectiles);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iWaitingForProjectiles);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bTurnActive);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iPowerups);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, Vector, m_avecTankAims);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, float, m_aflTankAimRadius);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iTankAimFocus);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iDifficulty);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bRenderFogOfWar);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, gamesettings_t, m_oGameSettings);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, gametype_t, m_eGameType);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CUpdateGrid>, m_hUpdates);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, actionitem_t, m_aActionItems);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bAllowActionItems);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bPartyMode);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flPartyModeStart);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLastFireworks);
SAVEDATA_TABLE_END();

void CDigitanksGame::Precache()
{
	BaseClass::Precache();

	// We precache this for the hud since it's not an entity
	PrecacheSound(L"sound/actionsign.wav");
}

void CDigitanksGame::Spawn()
{
	BaseClass::Spawn();

	m_iCurrentTeam = 0;
	m_pListener = NULL;
	m_bWaitingForMoving = false;
	m_bWaitingForProjectiles = false;
	m_iWaitingForProjectiles = 0;
	m_bTurnActive = true;
	m_iPowerups = 0;
	m_iDifficulty = 1;
	m_bRenderFogOfWar = true;
	m_bAllowActionItems = false;

	SetListener(DigitanksWindow()->GetHUD());

	m_flLastFireworks = 0;

	m_flShowFightSign = 0;
}

void CDigitanksGame::RegisterNetworkFunctions()
{
	BaseClass::RegisterNetworkFunctions();

	CNetwork::RegisterFunction("SetupEntities", this, SetupEntitiesCallback, 0);
	CNetwork::RegisterFunction("EnterGame", this, EnterGameCallback, 0);
	CNetwork::RegisterFunction("EndTurn", this, EndTurnCallback, 0);
	CNetwork::RegisterFunction("StartTurn", this, StartTurnCallback, 0);
	CNetwork::RegisterFunction("ManageSupplyLine", this, ManageSupplyLineCallback, 3, NET_HANDLE, NET_HANDLE, NET_HANDLE);
	CNetwork::RegisterFunction("Move", this, MoveCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("Turn", this, TurnCallback, 2, NET_HANDLE, NET_FLOAT);
	CNetwork::RegisterFunction("Fire", this, FireCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("FireWeapon", this, FireWeaponCallback, 5, NET_HANDLE, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("SetBonusPoints", this, SetBonusPointsCallback, 5, NET_HANDLE, NET_INT, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("TankPromoted", this, TankPromotedCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("PromoteAttack", this, PromoteAttackCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("PromoteDefense", this, PromoteDefenseCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("PromoteMovement", this, PromoteMovementCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("TankSpeak", this, SpeakCallback, 2, NET_HANDLE, NET_INT);
	CNetwork::RegisterFunction("Fortify", this, FortifyCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("Sentry", this, SentryCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("SetGoalMovePosition", this, SetGoalMovePositionCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("CancelGoalMovePosition", this, CancelGoalMovePositionCallback, 1, NET_HANDLE);

	CNetwork::RegisterFunction("TerrainData", this, TerrainDataCallback, 0);

	// CUpdateGrid
	CNetwork::RegisterFunction("UpdatesData", this, UpdatesDataCallback, 0);

	// CDigitanksTeam
	CNetwork::RegisterFunction("TeamUpdatesData", this, TeamUpdatesDataCallback, 0);
	CNetwork::RegisterFunction("DownloadUpdate", this, DownloadUpdateCallback, 0);
	CNetwork::RegisterFunction("DownloadComplete", this, DownloadCompleteCallback, 0);

	// CPU
	CNetwork::RegisterFunction("BeginConstruction", this, BeginConstructionCallback, 0);
	CNetwork::RegisterFunction("BeginRogueProduction", this, BeginRogueProductionCallback, 0);

	// CStructure
	CNetwork::RegisterFunction("BeginStructureConstruction", this, BeginStructureConstructionCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("InstallUpdate", this, InstallUpdateCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("BeginUpgrade", this, BeginUpgradeCallback, 1, NET_HANDLE);

	// CSupplier
	CNetwork::RegisterFunction("AddChild", this, AddChildCallback, 2, NET_HANDLE, NET_HANDLE);
	CNetwork::RegisterFunction("RemoveChild", this, RemoveChildCallback, 2, NET_HANDLE, NET_HANDLE);

	// CLoader
	CNetwork::RegisterFunction("BeginProduction", this, BeginProductionCallback, 1, NET_HANDLE);
}

void CDigitanksGame::OnClientConnect(CNetworkParameters* p)
{
	BaseClass::OnClientConnect(p);

	GetTerrain()->ResyncClientTerrainData(p->i2);
}

void CDigitanksGame::OnClientDisconnect(CNetworkParameters* p)
{
	BaseClass::OnClientDisconnect(p);
}

void CDigitanksGame::SetupGame(gametype_t eGameType)
{
	GameServer()->SetLoading(true);

	DigitanksWindow()->RenderLoading();

	SetupEntities();

	if (!CNetwork::IsHost())
		return;

	m_eGameType = eGameType;
	m_iTurn = 0;

	if (eGameType == GAMETYPE_STANDARD)
		SetupStrategy();
	else if (eGameType == GAMETYPE_ARTILLERY)
		SetupArtillery();
	else if (eGameType == GAMETYPE_TUTORIAL)
		SetupTutorial();
	else if (eGameType == GAMETYPE_MENU)
		SetupMenuMarch();

	GameServer()->SetLoading(false);

	if (eGameType != GAMETYPE_EMPTY)
		StartGame();
}

void CDigitanksGame::ReadGameScript(eastl::string16 sScript)
{
	memset(&m_aflConstructionCosts[0], 0, sizeof(m_aflConstructionCosts));
	memset(&m_aflUpgradeCosts[0], 0, sizeof(m_aflUpgradeCosts));

	std::ifstream f((eastl::string16(L"scripts/") + sScript).c_str());
	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	CData* pConstructionCosts = pData->FindChild("ConstructionCosts");

	if (pConstructionCosts)
	{
		CData* pChild;
		
		pChild = pConstructionCosts->FindChild("Minibuffer");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_MINIBUFFER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("Buffer");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_BUFFER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("BufferUpgrade");
		if (pChild)
			m_aflUpgradeCosts[STRUCTURE_BUFFER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("Battery");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_BATTERY] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("PSU");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_PSU] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("PSUUpgrade");
		if (pChild)
			m_aflUpgradeCosts[STRUCTURE_PSU] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("InfantryLoader");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_INFANTRYLOADER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("TankLoader");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_TANKLOADER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("ArtilleryLoader");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_ARTILLERYLOADER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("Infantry");
		if (pChild)
			m_aflConstructionCosts[UNIT_INFANTRY] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("Tank");
		if (pChild)
			m_aflConstructionCosts[UNIT_TANK] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("Artillery");
		if (pChild)
			m_aflConstructionCosts[UNIT_ARTILLERY] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("Scout");
		if (pChild)
			m_aflConstructionCosts[UNIT_SCOUT] = pChild->GetValueFloat();
	}

	delete pData;
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

			if (GetTerrain()->IsPointOverHole(Vector(x, 0, z)))
				continue;

			CResource* pResource = GameServer()->Create<CResource>("CResource");
			pResource->SetOrigin(m_hTerrain->SetPointHeight(Vector(x, 0, z)));
			pResource->FindGround();
		}
	}
}

void CDigitanksGame::SetupProps()
{
	gamesettings_t* pGameSettings = DigitanksGame()->GetGameSettings();

	CDigitanksLevel* pLevel = NULL;
	if (pGameSettings->iLevel < CDigitanksGame::GetNumLevels(GetGameType()))
		pLevel = dynamic_cast<CDigitanksLevel*>(CDigitanksGame::GetLevel(GetGameType(), pGameSettings->iLevel));

	if (!pLevel)
		return;

	for (size_t iProps = 0; iProps < pLevel->GetNumProps(); iProps++)
	{
		CLevelProp* pLevelProp = pLevel->GetProp(iProps);
		CStaticProp* pProp = GameServer()->Create<CStaticProp>("CStaticProp");
		pProp->SetOrigin(m_hTerrain->SetPointHeight(Vector(pLevelProp->m_vecPosition.x, 0, pLevelProp->m_vecPosition.y)));
		pProp->SetAngles(EAngle(0, pLevelProp->m_angOrientation.y, 0));
		pProp->SetColorSwap(m_hTerrain->GetPrimaryTerrainColor());
		pProp->SetModel(convertstring<char, char16_t>(pLevelProp->m_sModel));
	}

	for (size_t iProps = 0; iProps < 3; iProps++)
	{
		if (m_hTerrain->IsPointOverHole(Vector(m_hTerrain->GetMapSize(), 0, -100 + (float)iProps*100)))
			continue;

		CStaticProp* pProp = GameServer()->Create<CStaticProp>("CStaticProp");
		pProp->SetOrigin(m_hTerrain->SetPointHeight(Vector(m_hTerrain->GetMapSize(), 0, -100 + (float)iProps*100)));
		pProp->SetAngles(EAngle(0, 0, 0));
		pProp->SetColorSwap(m_hTerrain->GetPrimaryTerrainColor());
		pProp->SetUsesRaytracedCollision(false);
		pProp->SetModel(L"models/props/support01.obj");
	}

/*	for (int i = (int)-m_hTerrain->GetMapSize(); i < (int)m_hTerrain->GetMapSize(); i += 100)
	{
		for (int j = (int)-m_hTerrain->GetMapSize(); j < (int)m_hTerrain->GetMapSize(); j += 100)
		{
			float x = RandomFloat((float)i, (float)i+100);
			float z = RandomFloat((float)j, (float)j+100);

			if (x < -m_hTerrain->GetMapSize()+10 || z < -m_hTerrain->GetMapSize()+10)
				continue;

			if (x > m_hTerrain->GetMapSize()-10 || z > m_hTerrain->GetMapSize()-10)
				continue;

			if (GetGameType() == GAMETYPE_ARTILLERY && RandomInt(0, 2) != 0)
				continue;

			CStaticProp* pProp = GameServer()->Create<CStaticProp>("CStaticProp");
			pProp->SetOrigin(m_hTerrain->SetPointHeight(Vector(x, 0, z)));
			pProp->SetAngles(EAngle(0, RandomFloat(0, 360), 0));
			pProp->SetColorSwap(m_hTerrain->GetPrimaryTerrainColor());

			if (GetGameType() == GAMETYPE_ARTILLERY)
			{
				pProp->SetModel(L"models/props/prop05.obj");
			}
			else
			{
				switch (RandomInt(0, 3))
				{
				case 0:
					pProp->SetModel(L"models/props/prop01.obj");
					break;

				case 1:
					pProp->SetModel(L"models/props/prop02.obj");
					break;

				case 2:
					pProp->SetModel(L"models/props/prop03.obj");
					break;

				case 3:
					pProp->SetModel(L"models/props/prop04.obj");
					break;
				}
			}
		}
	}*/
}

void CDigitanksGame::ScatterNeutralUnits()
{
	AddTeamToList(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));

	CDigitanksTeam* pTeam = GetDigitanksTeam(GetNumTeams()-1);
	pTeam->SetColor(Color(128, 128, 128));
	pTeam->SetName(eastl::string16(L"Network Guardians"));
	pTeam->Bot_UseArtilleryAI();
	pTeam->DontIncludeInScoreboard();

	for (int i = (int)-m_hTerrain->GetMapSize(); i < (int)m_hTerrain->GetMapSize(); i += 100)
	{
		for (int j = (int)-m_hTerrain->GetMapSize(); j < (int)m_hTerrain->GetMapSize(); j += 100)
		{
			if (rand()%4 > 0)
				continue;

			float x = RandomFloat((float)i, (float)i+100);
			float z = RandomFloat((float)j, (float)j+100);

			if (x < -m_hTerrain->GetMapSize()+10 || z < -m_hTerrain->GetMapSize()+10)
				continue;

			if (x > m_hTerrain->GetMapSize()-10 || z > m_hTerrain->GetMapSize()-10)
				continue;

			CAutoTurret* pTurret = GameServer()->Create<CAutoTurret>("CAutoTurret");
			pTurret->SetOrigin(m_hTerrain->SetPointHeight(Vector(x, 0, z)));
			pTeam->AddEntity(pTurret);
		}
	}
}

void CDigitanksGame::SetupArtillery()
{
	TMsg(L"Setting up artillery game.\n");

	int iPlayers = m_oGameSettings.iHumanPlayers + m_oGameSettings.iBotPlayers;

	if (iPlayers > 8)
	{
		iPlayers = 8;
		if (m_oGameSettings.iHumanPlayers > 8)
		{
			m_oGameSettings.iHumanPlayers = 8;
			m_oGameSettings.iBotPlayers = 0;
		}
		else
			m_oGameSettings.iBotPlayers = 8-m_oGameSettings.iHumanPlayers;
	}

	if (iPlayers < 2)
	{
		iPlayers = 2;
		m_oGameSettings.iHumanPlayers = 2;
		m_oGameSettings.iBotPlayers = 0;
	}

	int iTanks = m_oGameSettings.iTanksPerPlayer;
	if (iTanks > 4)
		iTanks = 4;
	if (iTanks < 1)
		iTanks = 1;
	m_oGameSettings.iTanksPerPlayer = iTanks;

	Color aclrTeamColors[] =
	{
		Color(0, 0, 255),		// Blue
		Color(255, 0, 0),		// Red
		Color(42, 255, 0),		// Green
		Color(255, 200, 0),		// Yellow
		Color(0, 255, 221),		// Teal
		Color(200, 50, 150),	// Pink
		Color(100, 0, 255),		// Purple
		Color(255, 255, 255),	// White
	};

	eastl::string16 aszTeamNames[] =
	{
		L"Blue",
		L"Red",
		L"Green",
		L"Yellow",
		L"Teal",
		L"Pink",
		L"Purple",
		L"White",
	};

	for (int i = 0; i < iPlayers; i++)
	{
		AddTeamToList(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));

		CDigitanksTeam* pTeam = GetDigitanksTeam(GetNumTeams()-1);

		pTeam->SetColor(aclrTeamColors[i]);
		pTeam->SetName(aszTeamNames[i]);

		if (m_oGameSettings.iHumanPlayers == 1 && i == 0)
		{
			eastl::string16 sPlayerNickname = TPortal_GetPlayerNickname();
			if (sPlayerNickname.length())
				pTeam->SetName(sPlayerNickname);
		}
	}
}

void CDigitanksGame::SetupStrategy()
{
	TMsg(L"Setting up strategy game.\n");

	ReadGameScript(L"strategy.txt");

	m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
	m_hTerrain->GenerateTerrain();

	ScatterResources();
	SetupProps();
	ScatterNeutralUnits();

	int iPlayers = m_oGameSettings.iHumanPlayers + m_oGameSettings.iBotPlayers;

	if (iPlayers > 8)
	{
		iPlayers = 8;
		if (m_oGameSettings.iHumanPlayers > 8)
		{
			m_oGameSettings.iHumanPlayers = 8;
			m_oGameSettings.iBotPlayers = 0;
		}
		else
			m_oGameSettings.iBotPlayers = 8-m_oGameSettings.iHumanPlayers;
	}

	if (iPlayers < 2)
	{
		iPlayers = 2;
		m_oGameSettings.iHumanPlayers = 2;
		m_oGameSettings.iBotPlayers = 0;
	}

	Color aclrTeamColors[] =
	{
		Color(0, 64, 255),		// Blue
		Color(255, 0, 0),		// Red
		Color(42, 255, 0),		// Green
		Color(255, 200, 0),		// Yellow
	};

	eastl::string16 aszTeamNames[] =
	{
		L"Blue",
		L"Red",
		L"Green",
		L"Yellow",
	};

	Vector avecStartingPositions[] =
	{
		Vector(180, 0, 180),
		Vector(180, 0, -180),
		Vector(-180, 0, 180),
		Vector(-180, 0, -180),
	};

	eastl::vector<Vector> avecRandomStartingPositions;
	for (int i = 0; i < 4; i++)
		avecRandomStartingPositions.insert(avecRandomStartingPositions.begin()+RandomInt(0, i), avecStartingPositions[i]);

	for (int i = 0; i < iPlayers; i++)
	{
		AddTeamToList(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));

		CDigitanksTeam* pTeam = GetDigitanksTeam(GetNumTeams()-1);

		pTeam->SetColor(aclrTeamColors[i]);
		pTeam->SetName(aszTeamNames[i]);
		pTeam->SetLoseCondition(LOSE_NOCPU);

		if (m_oGameSettings.iHumanPlayers == 1 && i == 0)
		{
			eastl::string16 sPlayerNickname = TPortal_GetPlayerNickname();
			if (sPlayerNickname.length())
				pTeam->SetName(sPlayerNickname);
		}

		GetTerrain()->ClearArea(avecRandomStartingPositions[i], 40);

		CMobileCPU* pMobileCPU = GameServer()->Create<CMobileCPU>("CMobileCPU");
		pTeam->AddEntity(pMobileCPU);
		pMobileCPU->SetOrigin(m_hTerrain->SetPointHeight(avecRandomStartingPositions[i]));
		pMobileCPU->SetAngles(VectorAngles(-avecRandomStartingPositions[i].Normalized()));


		for (size_t j = 0; j < GameServer()->GetMaxEntities(); j++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(j);
			if (!pEntity)
				continue;

			CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);
			if (!pDTEntity)
				continue;

			if (pMobileCPU->GetTeam() == pDTEntity->GetTeam())
				continue;

			// Remove nearby stuff so our spawn point can be clear
			if ((pDTEntity->GetOrigin() - pMobileCPU->GetOrigin()).Length2D() < 30)
				pEntity->Delete();

			CAutoTurret* pAutoTurret = dynamic_cast<CAutoTurret*>(pEntity);
			if (pAutoTurret && (pAutoTurret->GetOrigin() - pMobileCPU->GetOrigin()).Length2D() < pAutoTurret->GetMaxRange())
				pAutoTurret->Delete();
		}

		CDigitank* pTank;
		Vector vecTank;
		EAngle angTank;

		Vector vecForward = (Vector(0,0,0) - avecRandomStartingPositions[i]).Normalized();
		Vector vecRight = vecForward.Cross(Vector(0,1,0)).Normalized();

		pTank = GameServer()->Create<CScout>("CScout");
		pTeam->AddEntity(pTank);

		vecTank = avecRandomStartingPositions[i] + vecForward * 20 + vecRight * 20;
		angTank = VectorAngles(-vecTank.Normalized());

		pTank->SetOrigin(GetTerrain()->SetPointHeight(vecTank));
		pTank->SetAngles(angTank);
		pTank->GiveBonusPoints(1, false);
	}

	for (size_t i = 0; i < m_oGameSettings.iHumanPlayers; i++)
		// There's one neutral team at the front so skip it.
		m_ahTeams[i+1]->SetClient(-1);

	CPowerup* pPowerup = GameServer()->Create<CPowerup>("CPowerup");
	pPowerup->SetOrigin(Vector(70, m_hTerrain->GetHeight(70, 70), 70));
	pPowerup = GameServer()->Create<CPowerup>("CPowerup");
	pPowerup->SetOrigin(Vector(70, m_hTerrain->GetHeight(70, -70), -70));
	pPowerup = GameServer()->Create<CPowerup>("CPowerup");
	pPowerup->SetOrigin(Vector(-70, m_hTerrain->GetHeight(-70, 70), 70));
	pPowerup = GameServer()->Create<CPowerup>("CPowerup");
	pPowerup->SetOrigin(Vector(-70, m_hTerrain->GetHeight(-70, -70), -70));

	m_iPowerups = 4;

	m_hUpdates = GameServer()->Create<CUpdateGrid>("CUpdateGrid");
	m_hUpdates->SetupStandardUpdates();
}

void CDigitanksGame::SetupTutorial()
{
	TMsg(L"Setting up tutorial game.\n");

	m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
	m_hTerrain->GenerateTerrain();

	AddTeamToList(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));
	m_ahTeams[0]->SetColor(Color(0, 0, 255));

	AddTeamToList(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));
	m_ahTeams[1]->SetColor(Color(255, 0, 0));

	m_ahTeams[0]->SetClient(-1);

	GetDigitanksTeam(0)->AddPower(50);

	m_iPowerups = 0;
}

void CDigitanksGame::SetupMenuMarch()
{
	TMsg(L"Setting up menu march.\n");

	m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
	m_hTerrain->GenerateTerrain();

	AddTeamToList(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));
	m_ahTeams[0]->SetColor(Color(0, 0, 255));

#ifndef _DEBUG
	CMenuMarcher* pMarcher;

	for (size_t i = 0; i < 4; i++)
	{
		float flZ = RemapVal((float)i, 0, 4, -79, 79);

		for (size_t j = 0; j < 5; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				pMarcher = GameServer()->Create<CMenuMarcher>("CMenuMarcher");
				m_ahTeams[0]->AddEntity(pMarcher);

				pMarcher->SetOrigin(GetTerrain()->SetPointHeight(Vector(RemapVal((float)j, 0, 5, -15, 15), 0, flZ + RemapVal((float)k, 0, 4, -15, 15))));
				pMarcher->SetAngles(EAngle(0,90,0));
			}
		}
	}
#endif

	m_ahTeams[0]->SetClient(-2);

	m_iPowerups = 0;
}

void CDigitanksGame::SetupEntities()
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "SetupEntities");

	CNetworkParameters p;
	SetupEntities(&p);
}

void CDigitanksGame::SetupEntities(CNetworkParameters* p)
{
	CSoundLibrary::StopSound();
	CParticleSystemLibrary::ClearInstances();

	while (m_ahTeams.size())
	{
		CTeam* pTeam = m_ahTeams[0];
		RemoveTeamFromList(pTeam);
		pTeam->Delete();
	}

	// Just in case!
	assert(m_ahTeams.size() == 0);
	m_ahTeams.clear();

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		if (pEntity == this)
			continue;

		pEntity->Delete();
	}
}

eastl::vector<CLevel*> CDigitanksGame::GetLevels(gametype_t eGameType)
{
	eastl::vector<CLevel*> ahReturn;
	for (size_t i = 0; i < GameServer()->GetNumLevels(); i++)
	{
		CDigitanksLevel* pLevel = dynamic_cast<CDigitanksLevel*>(GameServer()->GetLevel(i));
		if (!pLevel)
			continue;

		if (pLevel->GetGameType() == eGameType)
			ahReturn.push_back(pLevel);
	}

	return ahReturn;
}

size_t CDigitanksGame::GetNumLevels(gametype_t eGameType)
{
	return GetLevels(eGameType).size();
}

CLevel* CDigitanksGame::GetLevel(gametype_t eGameType, size_t i)
{
	return GetLevels(eGameType)[i];
}

void CDigitanksGame::StartGame()
{
	if (GetGameType() == GAMETYPE_STANDARD)
		// Start with the player's team so the neutral team doesn't get a chance to attack.
		m_iCurrentTeam = 1;
	else
		m_iCurrentTeam = 0;

	m_iWaitingForProjectiles = 0;
	m_bWaitingForProjectiles = true;

	if (HasRounds())
		StartNewRound();
	else
		GetCurrentTeam()->StartTurn();

	EnterGame(NULL);
}

void CDigitanksGame::EnterGame(CNetworkParameters* p)
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	if (CNetwork::IsHost())
		CNetwork::CallFunction(NETWORK_TOCLIENTS, "EnterGame");

	for (size_t i = 0; i < GetNumTeams(); i++)
		GetDigitanksTeam(i)->CountScore();

	m_aActionItems.clear();

	m_bWaitingForMoving = false;
	m_bWaitingForProjectiles = false;

	if (m_pListener)
	{
		m_pListener->GameStart();

		m_pListener->SetHUDActive(true);
		m_pListener->NewCurrentTeam();

		m_pListener->NewCurrentSelection();
	}

	if (m_eGameType == GAMETYPE_STANDARD && !CNetwork::IsConnected())
		DigitanksWindow()->GetStoryPanel()->SetVisible(true);

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		pEntity->ClientEnterGame();
	}

	if (CNetwork::IsConnected() && !DigitanksWindow()->IsRegistered() && GetGameType() == GAMETYPE_STANDARD)
		GameServer()->Halt();
}

void CDigitanksGame::StartNewRound()
{
	DigitanksWindow()->GetVictoryPanel()->SetVisible(false);

	m_iCurrentTeam = 0;

	m_iWaitingForProjectiles = 0;
	m_bWaitingForProjectiles = true;

	m_iTurn = 0;

	m_flPartyModeStart = 0;
	m_bPartyMode = false;

	for (size_t i = 0; i < GetNumTeams(); i++)
		GetDigitanksTeam(i)->StartNewRound();

	if (m_eGameType == GAMETYPE_ARTILLERY)
		SetupArtilleryRound();

	GetCurrentTeam()->StartTurn();
}

void CDigitanksGame::SetupArtilleryRound()
{
	GameServer()->SetLoading(true);

	DigitanksWindow()->RenderLoading();

	eastl::vector<eastl::string> asSpare;
	asSpare.push_back("CDigitanksGame");
	asSpare.push_back("CDigitanksTeam");

	GameServer()->DestroyAllEntities(asSpare);

	m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
	m_hTerrain->GenerateTerrain(m_oGameSettings.flTerrainHeight);

	float flMapBuffer = GetTerrain()->GetMapSize()*0.1f;
	float flMapSize = GetTerrain()->GetMapSize() - flMapBuffer*2;

	size_t iTotalTanks = m_oGameSettings.iTanksPerPlayer * (m_oGameSettings.iHumanPlayers + m_oGameSettings.iBotPlayers);
	size_t iSections = (int)sqrt((float)iTotalTanks);
	float flSectionSize = flMapSize*2/iSections;

	eastl::vector<size_t> aiRandomTeamPositions;
	// 8 random starting positions.
	for (size_t i = 0; i < (m_oGameSettings.iHumanPlayers + m_oGameSettings.iBotPlayers); i++)
	{
		for (size_t j = 0; j < m_oGameSettings.iTanksPerPlayer; j++)
			aiRandomTeamPositions.insert(aiRandomTeamPositions.begin()+RandomInt(0, aiRandomTeamPositions.size()-1), i);
	}

	size_t iPosition = 0;
	size_t iTanksPlaced = 0;
	while (iTanksPlaced < iTotalTanks)
	{
		for (size_t x = 0; x < iSections; x++)
		{
			for (size_t y = 0; y < iSections; y++)
			{
				if (iTanksPlaced >= iTotalTanks)
					break;

				CTeam* pTeam = m_ahTeams[aiRandomTeamPositions[iPosition]];

				float flSectionPositionX = -GetTerrain()->GetMapSize() + flMapBuffer + flSectionSize*x;
				float flSectionPositionY = -GetTerrain()->GetMapSize() + flMapBuffer + flSectionSize*y;

				Vector vecSectionPosition(flSectionPositionX, 0, flSectionPositionY);
				Vector vecSectionRandomize(RandomFloat(0, flSectionSize), 0, RandomFloat(0, flSectionSize));

				Vector vecTank = vecSectionPosition + vecSectionRandomize;

				if (GetTerrain()->IsPointOverLava(vecTank))
					continue;

				if (GetTerrain()->IsPointOverHole(vecTank))
					continue;

				if (GetTerrain()->IsPointOverWater(vecTank))
					continue;

				EAngle angTank = VectorAngles(-vecTank.Normalized());

				CDigitank* pTank = GameServer()->Create<CStandardTank>("CStandardTank");
				pTeam->AddEntity(pTank);

				vecTank.y = pTank->FindHoverHeight(vecTank);

				pTank->SetOrigin(vecTank);
				pTank->SetAngles(angTank);
				pTank->GiveBonusPoints(1, false);

				iPosition = (iPosition+1)%aiRandomTeamPositions.size();
				iTanksPlaced++;
			}
		}
	}

	for (size_t i = 0; i < m_oGameSettings.iHumanPlayers; i++)
		m_ahTeams[i]->SetClient(-1);

	CPowerup* pPowerup;
	
	Vector vecPowerup = Vector(70, m_hTerrain->GetHeight(70, 70), 70);
	if (!GetTerrain()->IsPointOverHole(vecPowerup))
	{
		pPowerup = GameServer()->Create<CPowerup>("CPowerup");
		pPowerup->SetOrigin(vecPowerup);
		m_iPowerups++;
	}

	vecPowerup = Vector(70, m_hTerrain->GetHeight(70, -70), -70);
	if (!GetTerrain()->IsPointOverHole(vecPowerup))
	{
		pPowerup = GameServer()->Create<CPowerup>("CPowerup");
		pPowerup->SetOrigin(vecPowerup);
		m_iPowerups++;
	}

	vecPowerup = Vector(-70, m_hTerrain->GetHeight(-70, 70), 70);
	if (!GetTerrain()->IsPointOverHole(vecPowerup))
	{
		pPowerup = GameServer()->Create<CPowerup>("CPowerup");
		pPowerup->SetOrigin(vecPowerup);
		m_iPowerups++;
	}

	vecPowerup = Vector(-70, m_hTerrain->GetHeight(-70, -70), -70);
	if (!GetTerrain()->IsPointOverHole(vecPowerup))
	{
		pPowerup = GameServer()->Create<CPowerup>("CPowerup");
		pPowerup->SetOrigin(vecPowerup);
		m_iPowerups++;
	}

	SetupProps();

	GameServer()->SetLoading(false);
}

bool CDigitanksGame::HasRounds()
{
	return m_eGameType == GAMETYPE_ARTILLERY;
}

void CDigitanksGame::Think()
{
	BaseClass::Think();

	if (m_flShowFightSign > 0 && m_flShowFightSign < GameServer()->GetGameTime())
	{
		DigitanksWindow()->GetHUD()->ShowFightSign();
		m_flShowFightSign = 0;
	}

	if (GetGameType() == GAMETYPE_MENU)
		return;

	if (HasRounds() && m_bPartyMode && GameServer()->GetGameTime() - m_flPartyModeStart > 10.0f)
	{
		StartNewRound();
		return;
	}

	if (m_bTurnActive && GetCurrentTeam() && !GetCurrentTeam()->IsPlayerControlled() && CNetwork::IsHost())
		GetCurrentTeam()->Bot_ExecuteTurn();

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
		{
			bool bTanksWaiting = false;
			for (size_t i = 0; i < GetCurrentTeam()->GetNumTanks(); i++)
			{
				CDigitank* pTank = GetCurrentTeam()->GetTank(i);
				if (pTank->IsWaitingToFire())
				{
					bTanksWaiting = true;
					break;
				}
			}

			if (!bTanksWaiting)
				m_bWaitingForProjectiles = false;
		}
	}

	bool bWaitingASecond = false;
	if (!GetCurrentTeam()->IsPlayerControlled())
	{
		CTeam* pNextTeam = GetTeam((m_iCurrentTeam+1)%GetNumTeams());
		if (pNextTeam && pNextTeam->IsPlayerControlled())
		{
			if (GameServer()->GetGameTime() - m_flLastHumanMove < 2.0f)
				bWaitingASecond = true;
		}
	}

	if (!m_bTurnActive && !m_bWaitingForMoving && !m_bWaitingForProjectiles && !bWaitingASecond)
		StartTurn();

	if (m_bPartyMode)
	{
		EAngle angCamera = GetDigitanksCamera()->GetAngles();
		angCamera.y += GameServer()->GetFrameTime()*2;
		GetDigitanksCamera()->SnapAngle(angCamera);

		if (CNetwork::IsHost() && GameServer()->GetGameTime() > m_flLastFireworks + RandomFloat(0.5f, 3.0f))
		{
			eastl::vector<CEntityHandle<CDigitanksEntity> > ahEntities;
			for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
			{
				if (!CBaseEntity::GetEntity(i))
					continue;

				CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(CBaseEntity::GetEntity(i));
				if (!pEntity)
					continue;
				
				if (dynamic_cast<CPowerup*>(pEntity))
					continue;

				if (dynamic_cast<CResource*>(pEntity))
					continue;

				if (dynamic_cast<CStaticProp*>(pEntity))
					continue;

				ahEntities.push_back(pEntity);
			}

			CDigitanksEntity* pEntity = ahEntities[RandomInt(0, ahEntities.size()-1)];

			CFireworks* pFireworks = GameServer()->Create<CFireworks>("CFireworks");
			pFireworks->SetOrigin(pEntity->GetOrigin());
			pFireworks->SetOwner(NULL);
			pFireworks->SetVelocity(Vector(RandomFloat(-8, 8), 45, RandomFloat(-8, 8)));
			pFireworks->SetGravity(Vector(0, DigitanksGame()->GetGravity(), 0));

			m_flLastFireworks = GameServer()->GetGameTime();
		}
	}

	for (size_t i = 0; i < m_aAirstrikes.size(); i++)
	{
		airstrike_t* pAirstrike = &m_aAirstrikes[i];
		if (pAirstrike->flNextShell < GameServer()->GetGameTime())
		{
			Vector vecLandingSpot = pAirstrike->vecLocation;

			float flYaw = RandomFloat(0, 360);
			float flRadius = RandomFloat(0, AirstrikeSize());

			// Don't use uniform distribution, I like how it's clustered on the target.
			vecLandingSpot += Vector(flRadius*cos(flYaw), 0, flRadius*sin(flYaw));

			CAirstrikeShell* pShell = GameServer()->Create<CAirstrikeShell>("CAirstrikeShell");
			pShell->SetOrigin(vecLandingSpot + Vector(30, 100, 30));
			pShell->SetVelocity(Vector(-30, -100, -30));
			pShell->SetGravity(Vector());
			pShell->SetOwner(NULL);

			pAirstrike->iShells--;
			if (pAirstrike->iShells == 0)
			{
				m_aAirstrikes.erase(m_aAirstrikes.begin()+i);
				// Prevent problems with the array resizing. Other airstrikes can be figured out next frame.
				break;
			}

			pAirstrike->flNextShell = GameServer()->GetGameTime() + RandomFloat(0.1f, 0.5f);
		}
	}
}

void CDigitanksGame::MoveTanks()
{
	if (!GetPrimarySelection())
		return;

	CDigitank* pCurrentTank = GetPrimarySelectionTank();
	if (!pCurrentTank)
		return;

	if (pCurrentTank->GetTeam() != GetCurrentLocalDigitanksTeam())
		return;

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
			pTank->SetGoalMovePosition(vecNewPosition);
		else
			pTank->Move();

		HandledActionItem(pTank);

		if (pTank->GetUnitType() == UNIT_MOBILECPU)
			DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_STRATEGY_COMMAND, true);
	}

	SetControlMode(MODE_NONE);

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_MOVE);
	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_ARTILLERY_COMMAND);
}

void CDigitanksGame::TurnTanks(Vector vecLookAt)
{
	if (!GetPrimarySelection())
		return;

	CDigitank* pCurrentTank = GetPrimarySelectionTank();
	if (!pCurrentTank)
		return;

	if (pCurrentTank->GetTeam() != GetCurrentLocalDigitanksTeam())
		return;

	bool bNoTurn = (vecLookAt - pCurrentTank->GetOrigin()).LengthSqr() < 4*4;

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
			Vector vecDirection = (vecLookAt - pTank->GetOrigin()).Normalized();
			float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

			float flTankTurn = AngleDifference(flYaw, pTank->GetAngles().y);
			if (fabs(flTankTurn)/pTank->TurnPerPower() > pTank->GetRemainingMovementEnergy())
				flTankTurn = (flTankTurn / fabs(flTankTurn)) * pTank->GetRemainingTurningDistance() * 0.95f;

			pTank->SetPreviewTurn(pTank->GetAngles().y + flTankTurn);
		}

		pTank->Turn();

		HandledActionItem(pTank);
	}

	SetControlMode(MODE_NONE);

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_ARTILLERY_COMMAND);
}

void CDigitanksGame::FireTanks()
{
	if (!GetPrimarySelection())
		return;

	CDigitank* pCurrentTank = GetPrimarySelectionTank();
	if (!pCurrentTank)
		return;

	if (pCurrentTank->GetTeam() != GetCurrentLocalDigitanksTeam())
		return;

	Vector vecPreviewAim = pCurrentTank->GetPreviewAim();

	CDigitanksTeam* pTeam = GetCurrentTeam();
	for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
	{
		CDigitank* pTank = pTeam->GetTank(i);

		if (CBaseWeapon::IsWeaponPrimarySelectionOnly(pTank->GetCurrentWeapon()) && GetPrimarySelectionTank() != pTank)
			continue;

		if (pTank->GetCurrentWeapon() == WEAPON_CHARGERAM)
		{
			pTank->Charge();
			continue;
		}

		if (pTank->GetCurrentWeapon() == PROJECTILE_AIRSTRIKE)
		{
			pTank->FireSpecial();
			continue;
		}

		if (!pTank->AimsWith(pCurrentTank))
			continue;

		Vector vecTankAim = vecPreviewAim;
		while (!pTank->IsInsideMaxRange(vecTankAim))
		{
			Vector vecDirection = vecTankAim - pTank->GetOrigin();
			vecDirection.y = 0;
			vecTankAim = DigitanksGame()->GetTerrain()->SetPointHeight(pTank->GetOrigin() + vecDirection.Normalized() * vecDirection.Length2D() * 0.99f);
		}

		pTank->SetPreviewAim(vecTankAim);
		pTank->Fire();

		HandledActionItem(pTank);
	}

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_ARTILLERY_COMMAND);
	SetControlMode(MODE_NONE);
}

void CDigitanksGame::EndTurn()
{
	CNetwork::CallFunction(NETWORK_TOEVERYONE, "EndTurn");

	EndTurn(NULL);

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_ENTERKEY);
	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_POWER);
	if (GetCurrentTeam() == GetCurrentLocalDigitanksTeam())
		DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_DEPLOYING2, true);
}

void CDigitanksGame::EndTurn(CNetworkParameters* p)
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	if (!GetCurrentTeam()->IsPlayerControlled())
		DigitanksGame()->GetDigitanksCamera()->ShowEnemyMoves();

	if (GetCurrentTeam()->IsPlayerControlled())
		m_flLastHumanMove = GameServer()->GetGameTime();

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

	if (GetGameType() == GAMETYPE_STANDARD && !DigitanksWindow()->IsRegistered() && GetTurn() > GetDemoTurns())
	{
		GameServer()->Halt();
		return;
	}

	int iPowerupChance;
	if (GetGameType() == GAMETYPE_STANDARD)
		iPowerupChance = RandomInt(0, 6);
	else
		iPowerupChance = RandomInt(0, 3);

	if (GetGameType() != GAMETYPE_MENU && !DigitanksWindow()->GetInstructor()->GetActive() && m_iPowerups < 10 && iPowerupChance == 0)
	{
		float flX = RandomFloat(-GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize());
		float flZ = RandomFloat(-GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize());
		Vector vecPowerup = Vector(flX, m_hTerrain->GetHeight(flX, flZ), flZ);

		if (!GetTerrain()->IsPointOverHole(vecPowerup))
		{
			CPowerup* pPowerup = GameServer()->Create<CPowerup>("CPowerup");
			pPowerup->SetOrigin(vecPowerup);

			m_iPowerups++;
		}
	}

	if (GetGameType() == GAMETYPE_STANDARD && GetTurn() > 3 && GetTurn() < 50)
	{
		if (RandomInt(0, GetNumTeams()-1) == 0)
		{
			Vector vecPoint;

			bool bIsVisible = false;

			size_t iTries = 0;
			while (iTries++ < 5)
			{
				float flX = RandomFloat(-GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize()) * 0.95f;
				float flZ = RandomFloat(-GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize()) * 0.95f;

				vecPoint = Vector(flX, 0, flZ);
				GetTerrain()->SetPointHeight(vecPoint);

				bIsVisible = false;

				// Skip the first team, that's the barbarians.
				for (size_t i = 1; i < GetNumTeams(); i++)
				{
					CDigitanksTeam* pTeam = GetDigitanksTeam(i);
					if (pTeam->GetVisibilityAtPoint(vecPoint) > 0)
					{
						bIsVisible = true;
						break;
					}
				}

				if (!bIsVisible)
					break;
			}

			if (!bIsVisible)
			{
				size_t iGridBugs = RandomInt(1, (int)RemapVal((float)GetTurn(), 6, 50, 2, 4));
				for (size_t i = 0; i < iGridBugs; i++)
				{
					vecPoint.x += 8;
					vecPoint.z += 8;

					Vector vecGridBug = vecPoint;

					if (!GetTerrain()->IsPointOverHole(vecGridBug) && GetTerrain()->IsPointOnMap(vecGridBug))
					{
						CGridBug* pGridBug = GameServer()->Create<CGridBug>("CGridBug");
						pGridBug->SetOrigin(vecGridBug);
						GetTeam(0)->AddEntity(pGridBug);
					}
				}
			}
		}
	}

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "StartTurn");

	StartTurn(NULL);

	if (GetCurrentTeam()->HasLost())
		EndTurn();
}

void CDigitanksGame::StartTurn(CNetworkParameters* p)
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	DigitanksGame()->GetDigitanksCamera()->ClearFollowTarget();

	if (m_iCurrentTeam == (size_t)0)
		m_iTurn++;

	if (++m_iCurrentTeam >= GetNumTeams())
		m_iCurrentTeam = 0;

	if (m_pListener)
		m_pListener->ClearTurnInfo();

	m_iWaitingForProjectiles = 0;

	m_bTurnActive = true;

	m_aActionItems.clear();
	m_bAllowActionItems = true;

	GetCurrentTeam()->StartTurn();

	m_bAllowActionItems = false;

	if (m_pListener)
	{
		m_pListener->SetHUDActive(true);
		m_pListener->NewCurrentTeam();
	}

	if (GetPrimarySelection())
		GetPrimarySelection()->OnCurrentSelection();

	GetTerrain()->CalculateVisibility();
}

bool CDigitanksGame::Explode(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flRadius, float flDamage, CBaseEntity* pIgnore, CTeam* pTeamIgnore)
{
	CBaseWeapon* pWeapon = dynamic_cast<CBaseWeapon*>(pInflictor);

	Vector vecExplosionOrigin;
	if (pInflictor)
		vecExplosionOrigin = pInflictor->GetOrigin();
	else
		vecExplosionOrigin = pAttacker->GetOrigin();

	eastl::vector<CBaseEntity*> apHit;
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CDigitank* pDigitank = dynamic_cast<CDigitank*>(pEntity);

		float flDistanceSqr = (pInflictor->GetOrigin() - pEntity->GetOrigin()).LengthSqr();
		float flTotalRadius = flRadius + pEntity->GetBoundingRadius();
		float flPushRadius = pWeapon?pWeapon->PushRadius():20;
		float flTotalRadius2 = flRadius + pEntity->GetBoundingRadius() + flPushRadius;

		if (pDigitank && flDistanceSqr < flTotalRadius2*flTotalRadius2 && !pDigitank->IsFortified() && !pDigitank->IsFortifying())
		{
			float flRockIntensity = pWeapon?pWeapon->RockIntensity():0.5f;
			Vector vecExplosion = (pDigitank->GetOrigin() - vecExplosionOrigin).Normalized();
			pDigitank->RockTheBoat(RemapValClamped(flDistanceSqr, flTotalRadius*flTotalRadius, flTotalRadius2*flTotalRadius2, flRockIntensity, flRockIntensity/5), vecExplosion);

			if (flRadius < 1 || flDistanceSqr > flTotalRadius*flTotalRadius)
			{
				float flPushDistance = pWeapon?pWeapon->PushDistance():flRadius/2;

				Vector vecPushDirection = vecExplosion;
				if (vecPushDirection.y < 0)
				{
					vecPushDirection.y = 0;
					vecPushDirection.Normalize();
				}

				// If we have a direct hit (the ignored is a direct hit, see CProjectile::Touching) exaggerate it.
				if (pEntity == pIgnore)
					flPushDistance *= 1.5f;

				pDigitank->Move(pDigitank->GetOrigin() + vecPushDirection * RemapValClamped(flDistanceSqr, flTotalRadius*flTotalRadius, flTotalRadius2*flTotalRadius2, flPushDistance, flPushDistance/2), 2);
			}

			pDigitank->SetGoalTurretYaw(atan2(-vecExplosion.z, -vecExplosion.x) * 180/M_PI - pDigitank->GetRenderAngles().y);
		}

		if (pEntity == pIgnore)
			continue;

		// Fire too close to yourself and the explosion can rock you.
		if (pEntity != pAttacker)
		{
			if (!pInflictor->ShouldTouch(pEntity))
				continue;

			// We can still push teammates around (above code) but we can't damage them.
			if (pDigitank && pDigitank->GetTeam() == pTeamIgnore)
				continue;
		}

		if (flDistanceSqr < flTotalRadius*flTotalRadius)
			apHit.push_back(pEntity);
		else
		{
			if (pDigitank && dynamic_cast<CProjectile*>(pInflictor))
			{
				if (pDigitank->IsScout() && (pEntity->GetOrigin() - vecExplosionOrigin).Length2DSqr() < flTotalRadius*flTotalRadius && pEntity->GetOrigin().y > vecExplosionOrigin.y)
					OnMiss(pEntity, pAttacker, pInflictor);
			}
		}
	}

	bool bHit = false;

	for (size_t i = 0; i < apHit.size(); i++)
	{
		float flDistance = (pInflictor->GetOrigin() - apHit[i]->GetOrigin()).Length();

		if (!pWeapon || pWeapon->HasDamageFalloff())
		{
			flDamage = RemapVal(flDistance, 0, flRadius + apHit[i]->GetBoundingRadius(), flDamage, flDamage/2);
			if (flDamage <= 0)
				continue;
		}

		bHit = true;

		apHit[i]->TakeDamage(pAttacker, pInflictor, DAMAGE_EXPLOSION, flDamage, false);
	}

	int iRunners = RandomInt(15, 10);
	for (int i = 0; i < iRunners; i++)
		GetTerrain()->AddRunner(vecExplosionOrigin, Color(255, 255, 255));

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

void CDigitanksGame::OnDisabled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	if (m_pListener)
		m_pListener->OnDisabled(pVictim, pAttacker, pInflictor);

	CDigitank* pTank = dynamic_cast<CDigitank*>(pVictim);
	if (pTank)
		pTank->Speak(TANKSPEECH_DISABLED);
}

void CDigitanksGame::OnMiss(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	if (m_pListener)
		m_pListener->OnMiss(pVictim, pAttacker, pInflictor);

	CDigitank* pTank = dynamic_cast<CDigitank*>(pVictim);
	if (pTank)
		pTank->Speak(TANKSPEECH_TAUNT);
}

void CDigitanksGame::OnKilled(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
		m_ahTeams[i]->OnKilled(pEntity);

	CheckWinConditions();
}

void CDigitanksGame::CheckWinConditions()
{
	if (m_eGameType == GAMETYPE_TUTORIAL || m_eGameType == GAMETYPE_MENU)
		return;

	if (m_bPartyMode)
		return;

	bool bSomeoneLost = false;
	size_t iTeamsLeft = 0;

	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (GetDigitanksTeam(i)->HasLost())
			continue;

		switch (GetDigitanksTeam(i)->GetLoseCondition())
		{
		case LOSE_NOCPU:
		{
			bool bHasCPU = false;
			for (size_t j = 0; j < m_ahTeams[i]->GetNumMembers(); j++)
			{
				CBaseEntity* pEntity = m_ahTeams[i]->GetMember(j);
				if (dynamic_cast<CCPU*>(pEntity) || dynamic_cast<CMobileCPU*>(pEntity))
				{
					bHasCPU = true;
					iTeamsLeft++;
					break;
				}
			}

			if (!bHasCPU)
			{
				GetDigitanksTeam(i)->YouLoseSirGoodDay();
				bSomeoneLost = true;
			}

			break;
		}

		case LOSE_NOTANKS:
		{
			if (GetDigitanksTeam(i)->GetNumTanksAlive() == 0)
			{
				GetDigitanksTeam(i)->YouLoseSirGoodDay();
				bSomeoneLost = true;
			}
			else
				iTeamsLeft++;
			break;
		}
		}
	}

	if (bSomeoneLost && iTeamsLeft == 2)
		DigitanksWindow()->GetHUD()->ShowShowdownSign();

	if (iTeamsLeft <= 1)
		GameOver();
}

void CDigitanksGame::GameOver()
{
	if (GameServer()->IsLoading())
		return;

	if (m_pListener && GetCurrentLocalDigitanksTeam() && !GetCurrentLocalDigitanksTeam()->HasLost())
		m_pListener->GameOver(!GetCurrentLocalDigitanksTeam()->HasLost());

	m_bPartyMode = true;
	m_flPartyModeStart = GameServer()->GetGameTime();

	GetDigitanksCamera()->SetDistance(250);
	GetDigitanksCamera()->SetTarget(Vector(0,0,0));
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

void CDigitanksGame::TankSpeak(class CBaseEntity* pTank, const eastl::string& sSpeech)
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
	if (GameServer()->IsLoading())
		return MODE_NONE;

	if (IsTeamControlledByMe(GetCurrentTeam()))
		return m_eControlMode;

	return MODE_NONE;
}

void CDigitanksGame::SetControlMode(controlmode_t eMode)
{
	if (!GetPrimarySelection())
	{
		if (eMode == MODE_NONE)
			m_eControlMode = eMode;

		return;
	}

	if (DigitanksWindow()->GetVictoryPanel()->IsVisible())
		return;

	if (!GetPrimarySelection()->AllowControlMode(eMode))
		return;

	GetPrimarySelection()->OnControlModeChange(m_eControlMode, eMode);

	m_eControlMode = eMode;

	if (eMode == MODE_AIM)
		DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_AIM);
}

aimtype_t CDigitanksGame::GetAimType()
{
	if (GameServer()->IsLoading())
		return AIM_NONE;

	if (IsTeamControlledByMe(GetCurrentTeam()))
		return m_eAimType;

	return AIM_NONE;
}

void CDigitanksGame::SetAimType(aimtype_t eAimType)
{
	if (!GetPrimarySelection())
	{
		if (eAimType == AIM_NONE)
			m_eAimType = eAimType;
		return;
	}

	if (DigitanksWindow()->GetVictoryPanel()->IsVisible())
		return;

	m_eAimType = eAimType;
}

void CDigitanksGame::SetAimTypeByWeapon(weapon_t eWeapon)
{
	if (eWeapon == WEAPON_CHARGERAM)
		SetAimType(AIM_MOVEMENT);
	else if (eWeapon == WEAPON_LASER)
		SetAimType(AIM_NORANGE);
	else if (eWeapon == PROJECTILE_CAMERAGUIDED)
		SetAimType(AIM_NORANGE);
	else if (eWeapon == PROJECTILE_AIRSTRIKE)
		SetAimType(AIM_NORANGE);
	else
		SetAimType(AIM_NORMAL);
}

void CDigitanksGame::TerrainData(class CNetworkParameters* p)
{
	if (!GetTerrain())
		return;

	GetTerrain()->TerrainData(p);
}

CDigitanksRenderer*	CDigitanksGame::GetDigitanksRenderer()
{
	return dynamic_cast<CDigitanksRenderer*>(GameServer()->GetRenderer());
}

CDigitanksCamera* CDigitanksGame::GetDigitanksCamera()
{
	CCamera* pCamera = GameServer()->GetCamera();
	return dynamic_cast<CDigitanksCamera*>(pCamera);
}

float CDigitanksGame::GetGravity()
{
	return -20;
}

void CDigitanksGame::WeaponSpecialCommand()
{
	if (DigitanksGame()->GetGameType() != GAMETYPE_ARTILLERY)
		return;

	eastl::vector<CEntityHandle<CBaseWeapon> > ahWeapons;

	// Form a list of weapons to send the message to since sometimes it creates new projectiles,
	// and we don't want to send the message to those new ones.
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CBaseWeapon* pWeapon = dynamic_cast<CBaseWeapon*>(pEntity);
		if (!pWeapon)
			continue;

		if (!pWeapon->GetOwner())
			continue;

		// If it's not my grenade don't blow it up.
		if (GetCurrentLocalDigitanksTeam() != pWeapon->GetOwner()->GetTeam())
			continue;

		// If it's not my turn, don't blow it up.
		if (GetCurrentTeam() != pWeapon->GetOwner()->GetTeam())
			continue;

		ahWeapons.push_back(pWeapon);
	}

	for (size_t i = 0; i < ahWeapons.size(); i++)
	{
		CBaseWeapon* pWeapon = ahWeapons[i];
		if (!pWeapon)
			continue;

		pWeapon->SpecialCommand();
	}
}

void CDigitanksGame::AddTankAim(Vector vecAim, float flRadius, bool bFocus)
{
	vecAim.y = 0;
	m_avecTankAims.push_back(vecAim);
	m_aflTankAimRadius.push_back(flRadius);
	if (bFocus)
		m_iTankAimFocus = m_avecTankAims.size()-1;
}

void CDigitanksGame::GetTankAims(eastl::vector<Vector>& avecAims, eastl::vector<float>& aflAimRadius, size_t& iFocus)
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

void CDigitanksGame::AppendTurnInfo(const eastl::string16& pszTurnInfo)
{
	if (m_pListener)
		m_pListener->AppendTurnInfo(pszTurnInfo);
}

void CDigitanksGame::OnDisplayTutorial(size_t iTutorial)
{
	if (iTutorial == CInstructor::TUTORIAL_INTRO_BASICS)
	{
		CDigitank* pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[0]->AddEntity(pTank);
		pTank->StartTurn();

		pTank->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, 0)));

		GetDigitanksCamera()->SnapTarget(pTank->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, 0, 0));
	}
	else if (iTutorial == CInstructor::TUTORIAL_SELECTION)
		GetDigitanksCamera()->SetTarget(GetDigitanksTeam(0)->GetTank(0)->GetOrigin());
	else if (iTutorial == CInstructor::TUTORIAL_MOVE)
	{
		// Make an enemy for us to clobber. Close enough that moving out of the way won't move us out of range
		CDigitank* pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[1]->AddEntity(pTank);

		pTank->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, -50)));

		GetDigitanksCamera()->SetTarget(GetDigitanksTeam(0)->GetTank(0)->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, 0, 0));
	}
	else if (iTutorial == CInstructor::TUTORIAL_POWERUP)
	{
		CPowerup* pPowerup = GameServer()->Create<CPowerup>("CPowerup");
		pPowerup->SetOrigin(GetTerrain()->SetPointHeight(GetDigitanksTeam(0)->GetTank(0)->GetOrigin() + Vector(0, 0, -10)));
	}
	else if (iTutorial == CInstructor::TUTORIAL_SHIFTSELECT)
	{
		CDigitank* pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[0]->AddEntity(pTank);
		pTank->StartTurn();
		pTank->SetOrigin(GetTerrain()->SetPointHeight(m_ahTeams[0]->GetMember(0)->GetOrigin() + Vector(-15, 0, 15)));

		pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[0]->AddEntity(pTank);
		pTank->StartTurn();
		pTank->SetOrigin(GetTerrain()->SetPointHeight(m_ahTeams[0]->GetMember(0)->GetOrigin() + Vector(15, 0, -15)));
	}
	else if (iTutorial == CInstructor::TUTORIAL_THEEND_BASICS)
	{
		// So that pressing the escape key works the first time.
		SetControlMode(MODE_NONE);
	}
	else if (iTutorial == CInstructor::TUTORIAL_INTRO_BASES)
	{
		CCPU* pCPU = GameServer()->Create<CCPU>("CCPU");
		pCPU->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, 0)));
		m_ahTeams[0]->AddEntity(pCPU);
		pCPU->UpdateTendrils();

		CResource* pResource = GameServer()->Create<CResource>("CResource");
		pResource->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, 20)));

		GetDigitanksCamera()->SnapTarget(pCPU->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, 0, 0));

		EndTurn();	// Force structure height and power updates.

		GetDigitanksTeam(0)->SetPrimarySelection(pCPU);
	}
	else if (iTutorial == CInstructor::TUTORIAL_THEEND_BASES)
	{
		// So that pressing the escape key works the first time.
		SetControlMode(MODE_NONE);
	}
	else if (iTutorial == CInstructor::TUTORIAL_INTRO_UNITS)
	{
		CDigitank* pTank = GameServer()->Create<CMechInfantry>("CMechInfantry");
		m_ahTeams[0]->AddEntity(pTank);
		pTank->StartTurn();

		pTank->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, 0)));

		GetDigitanksCamera()->SnapTarget(pTank->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, 0, 0));
	}
	else if (iTutorial == CInstructor::TUTORIAL_ARTILLERY)
	{
		// Kill the infantry, spawn an artillery.
		m_ahTeams[0]->GetMember(0)->Delete();

		CDigitank* pTank = GameServer()->Create<CArtillery>("CArtillery");
		m_ahTeams[0]->AddEntity(pTank);
		pTank->StartTurn();

		pTank->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, 0)));

		GetDigitanksCamera()->SetTarget(pTank->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, 0, 0));
	}
	else if (iTutorial == CInstructor::TUTORIAL_FIRE_ARTILLERY)
	{
		// Kill the infantry, spawn an artillery.
		CDigitank* pArtillery = GetDigitanksTeam(0)->GetTank(0);

		CDigitank* pTarget = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[1]->AddEntity(pTarget);
		pTarget->StartTurn();

		Vector vecOrigin = pArtillery->GetOrigin() + AngleVector(pTarget->GetAngles()) * pArtillery->GetEffRange();
		pTarget->SetOrigin(GetTerrain()->SetPointHeight(vecOrigin));

		CDigitank* pSpotter = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[0]->AddEntity(pSpotter);
		pSpotter->StartTurn();

		vecOrigin = pArtillery->GetOrigin() + AngleVector(pTarget->GetAngles()) * (pArtillery->GetMinRange() + pArtillery->GetEffRange())/2;
		pSpotter->SetOrigin(GetTerrain()->SetPointHeight(vecOrigin));

		GetDigitanksCamera()->SetTarget(pTarget->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, pArtillery->GetAngles().y-45, 0));

		// So we can see the new guy
		GetDigitanksTeam(0)->CalculateVisibility();
	}
	else if (iTutorial == CInstructor::TUTORIAL_ROGUE)
	{
		if (m_ahTeams[1]->GetMember(0))
			m_ahTeams[1]->GetMember(0)->Delete();
		if (m_ahTeams[0]->GetMember(1))
			m_ahTeams[0]->GetMember(1)->Delete();
		if (m_ahTeams[0]->GetMember(0))
			m_ahTeams[0]->GetMember(0)->Delete();

		CDigitank* pTank = GameServer()->Create<CScout>("CScout");
		m_ahTeams[0]->AddEntity(pTank);
		pTank->StartTurn();

		pTank->SetOrigin(GetTerrain()->SetPointHeight(Vector(0.0f, 0, 0.0f)));

		GetDigitanksCamera()->SetTarget(pTank->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, 0, 0));
	}
	else if (iTutorial == CInstructor::TUTORIAL_TORPEDO)
	{
		CCPU* pCPU = GameServer()->Create<CCPU>("CCPU");
		pCPU->SetOrigin(GetTerrain()->SetPointHeight(Vector(20, 0, 20)));
		m_ahTeams[1]->AddEntity(pCPU);

		CBuffer* pBuffer = GameServer()->Create<CBuffer>("CBuffer");
		pBuffer->CompleteConstruction();
		pBuffer->SetOrigin(GetTerrain()->SetPointHeight(Vector(-20, 0, 20)));
		m_ahTeams[1]->AddEntity(pBuffer);
		pBuffer->SetSupplier(pCPU);

		GetDigitanksTeam(0)->CalculateVisibility();
	}
	else if (iTutorial == CInstructor::TUTORIAL_INGAME_STRATEGY_COMMAND)
	{
		SetControlMode(MODE_MOVE);
	}

	// Make sure that features now enabled are turned on.
	DigitanksWindow()->GetHUD()->SetupMenu();
}

void CDigitanksGame::ClientEnterGame()
{
	BaseClass::ClientEnterGame();

	if (m_eGameType == GAMETYPE_MENU)
	{
		GetDigitanksCamera()->SnapTarget(Vector(0,0,0));
		GetDigitanksCamera()->SnapAngle(EAngle(55,20,0));
		GetDigitanksCamera()->SnapDistance(60);
	}
	else if (m_eGameType == GAMETYPE_TUTORIAL)
	{
		GetDigitanksCamera()->SnapTarget(Vector(0,0,0));
		GetDigitanksCamera()->SnapAngle(EAngle(45, 0, 0));
		GetDigitanksCamera()->SnapDistance(80);
	}
	else
	{
		if (GetCurrentLocalDigitanksTeam() && GetCurrentLocalDigitanksTeam()->GetMember(0))
			GetDigitanksCamera()->SnapTarget(GetCurrentLocalDigitanksTeam()->GetMember(0)->GetOrigin());
		else
			GetDigitanksCamera()->SnapTarget(Vector(0,0,0));
		GetDigitanksCamera()->SnapAngle(EAngle(45,0,0));

		if (m_eGameType == GAMETYPE_ARTILLERY)
			GetDigitanksCamera()->SnapDistance(220);
		else
			GetDigitanksCamera()->SnapDistance(120);
	}

	// Give the game a second to load up before showing the fight sign.
	// Otherwise the sound sometimes plays while the game is still loading.
	if (m_eGameType == GAMETYPE_ARTILLERY)
		m_flShowFightSign = GameServer()->GetGameTime() + 1.0f;

	DigitanksWindow()->GetHUD()->ClientEnterGame();
	glgui::CRootPanel::Get()->Layout();
}

void CDigitanksGame::SetRenderFogOfWar(bool bRenderFogOfWar)
{
	m_bRenderFogOfWar = bRenderFogOfWar;
	GetTerrain()->CalculateVisibility();
}

bool CDigitanksGame::ShouldRenderFogOfWar()
{
	if (m_eGameType == GAMETYPE_ARTILLERY)
		return false;

	if (m_eGameType == GAMETYPE_MENU)
		return false;

	if (m_eGameType == GAMETYPE_TUTORIAL)
	{
		size_t iTutorial = DigitanksWindow()->GetInstructor()->GetCurrentTutorial();
		if (iTutorial >= CInstructor::TUTORIAL_INTRO_BASES)
			return true;
		else
			return false;
	}

	if (IsPartyMode())
		return false;
	else
		return m_bRenderFogOfWar;
}

float CDigitanksGame::GetVisibilityAtPoint(CDigitanksTeam* pViewingTeam, Vector vecPoint)
{
	if (!ShouldRenderFogOfWar())
		return 1.0f;

	if (!pViewingTeam)
		return 0.0f;

	return pViewingTeam->GetVisibilityAtPoint(vecPoint);
}

bool CDigitanksGame::ShouldShowScores()
{
	return m_eGameType == GAMETYPE_STANDARD;
}

float CDigitanksGame::GetConstructionCost(unittype_t eUnit)
{
	if (GetGameType() == GAMETYPE_STANDARD)
		return m_aflConstructionCosts[eUnit];
	else
		return 5.0f;
}

float CDigitanksGame::GetUpgradeCost(unittype_t eUnit)
{
	if (GetGameType() == GAMETYPE_STANDARD)
		return m_aflUpgradeCosts[eUnit];
	else
		return 5.0f;
}

bool CDigitanksGame::CanBuildMiniBuffers()
{
	if (m_eGameType == GAMETYPE_TUTORIAL)
		return false;

	return true;
}

bool CDigitanksGame::CanBuildBuffers()
{
	bool bDisableBuffer = DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_BUFFER);
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
	bool bDisablePSU = DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_PSU);
	return !bDisablePSU;
}

bool CDigitanksGame::CanBuildInfantryLoaders()
{
	bool bDisableLoaders = DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

bool CDigitanksGame::CanBuildTankLoaders()
{
	bool bDisableLoaders = DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

bool CDigitanksGame::CanBuildArtilleryLoaders()
{
	bool bDisableLoaders = DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

void CDigitanksGame::AddActionItem(CSelectable* pUnit, actiontype_t eActionType)
{
	if (pUnit && !IsTeamControlledByMe(pUnit->GetTeam()))
		return;

	if (!m_bAllowActionItems)
		return;

	if (GetGameType() != GAMETYPE_STANDARD)
		return;

	// Prevent duplicates
	for (size_t i = 0; i < m_aActionItems.size(); i++)
	{
		if (!pUnit && m_aActionItems[i].iUnit == ~0 && eActionType == m_aActionItems[i].eActionType)
			return;

		if (pUnit && m_aActionItems[i].iUnit == pUnit->GetHandle())
		{
			// Use the lowest value, that list is sorted that way.
			if (eActionType < m_aActionItems[i].eActionType)
				m_aActionItems[i].eActionType = eActionType;

			return;
		}
	}

	m_aActionItems.push_back(actionitem_t());
	actionitem_t* pActionItem = &m_aActionItems[m_aActionItems.size()-1];
	pActionItem->iUnit = pUnit?pUnit->GetHandle():~0;
	pActionItem->eActionType = eActionType;
	pActionItem->bHandled = false;
	DigitanksWindow()->GetHUD()->OnAddNewActionItem();
}

void CDigitanksGame::HandledActionItem(CSelectable* pUnit)
{
	if (!pUnit)
		return;

	size_t iItem = ~0;
	for (size_t i = 0; i < m_aActionItems.size(); i++)
	{
		if (m_aActionItems[i].iUnit == pUnit->GetHandle())
		{
			iItem = i;
			break;
		}
	}

	if (iItem == ~0)
		return;

	if (!pUnit->NeedsOrders())
	{
		m_aActionItems[iItem].bHandled = true;
		DigitanksWindow()->GetHUD()->Layout();
	}
}

void CDigitanksGame::HandledActionItem(actiontype_t eItem)
{
	for (size_t i = 0; i < m_aActionItems.size(); i++)
	{
		if (m_aActionItems[i].eActionType == eItem)
		{
			m_aActionItems[i].bHandled = true;
			DigitanksWindow()->GetHUD()->Layout();
			return;
		}
	}
}

void CDigitanksGame::BeginAirstrike(Vector vecLocation)
{
	m_aAirstrikes.push_back();
	size_t iAirstrike = m_aAirstrikes.size()-1;
	airstrike_t* pAirstrike = &m_aAirstrikes[iAirstrike];
	pAirstrike->iShells = 20;
	pAirstrike->flNextShell = 0;
	pAirstrike->vecLocation = vecLocation;
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
				pStructure->CompleteConstruction();

			if (pStructure->IsUpgrading())
				pStructure->CompleteConstruction();
		}

		CLoader* pLoader = dynamic_cast<CLoader*>(pMember);
		if (pLoader)
		{
			if (pLoader->IsProducing())
				pLoader->CompleteProduction();
		}
	}
}

CDigitanksTeam* CDigitanksGame::GetCurrentLocalDigitanksTeam()
{
	eastl::vector<CEntityHandle<CTeam> > hLocalTeams = GetLocalTeams();

	if (!hLocalTeams.size())
		return NULL;

	if (hLocalTeams.size() == 1)
		return dynamic_cast<CDigitanksTeam*>(hLocalTeams[0].GetPointer());

	for (size_t i = 0; i < hLocalTeams.size(); i++)
	{
		if (GetCurrentTeam() == hLocalTeams[i])
			return dynamic_cast<CDigitanksTeam*>(hLocalTeams[i].GetPointer());
	}

	return NULL;
}

bool CDigitanksGame::SoftCraters()
{
	return m_eGameType == GAMETYPE_STANDARD;
}

void CDigitanksGame::UpdateHUD(CNetworkedVariableBase* pVariable)
{
	CHUD::SetNeedsUpdate();
}