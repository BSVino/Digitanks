#include "digitanksgame.h"

#include <maths.h>
#include <mtrand.h>
#include <strutils.h>
#include <platform.h>

#include <datamanager/dataserializer.h>
#include <models/models.h>
#include <sound/sound.h>
#include <renderer/particles.h>
#include <tinker/portals/portal.h>
#include <tinker/cvar.h>
#include <tinker/lobby/lobby_client.h>
#include <game/gameserver.h>
#include <network/network.h>
#include <network/commands.h>

#include <ui/digitankswindow.h>
#include <ui/ui.h>
#include <ui/instructor.h>
#include <ui/hud.h>
#include "powerup.h"
#include "terrain.h"
#include "dt_camera.h"
#include <ui/chatbox.h>

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
#include "digitanks/campaign/userfile.h"
#include "digitanks/instructor_entity.h"

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
	NETVAR_DEFINE(eastl::string16, m_sObjective);
	NETVAR_DEFINE(CEntityHandle<CTerrain>, m_hTerrain);
	NETVAR_DEFINE(size_t, m_iDifficulty);
	NETVAR_DEFINE(bool, m_bRenderFogOfWar);
	NETVAR_DEFINE(gametype_t, m_eGameType);
	NETVAR_DEFINE(size_t, m_iTurn);
	NETVAR_DEFINE(CEntityHandle<CUpdateGrid>, m_hUpdates);
	NETVAR_DEFINE(bool, m_bPartyMode);
	NETVAR_DEFINE(float, m_aflConstructionCosts);
	NETVAR_DEFINE(float, m_aflUpgradeCosts);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitanksGame);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iCurrentTeam);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, controlmode_t, m_eControlMode);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, aimtype_t, m_eAimType);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, eastl::string16, m_sObjective);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CTerrain>, m_hTerrain);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CInstructorEntity>, m_hInstructor);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, IDigitanksGameListener*, m_pListener);	// Set by constructor
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bWaitingForMoving);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bWaitingForProjectiles);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iWaitingForProjectiles);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bTurnActive);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iPowerups);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, Vector, m_avecTankAims);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, float, m_aflTankAimRadius);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, size_t, m_iTankAimFocus);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iDifficulty);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bRenderFogOfWar);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, gametype_t, m_eGameType);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CUpdateGrid>, m_hUpdates);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bPartyMode);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flPartyModeStart);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLastFireworks);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bOverrideAllowLasers);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, airstrike_t, m_aAirstrikes);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_aflConstructionCosts);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_aflUpgradeCosts);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flShowFightSign);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flShowArtilleryTutorial);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLastHumanMove);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CDigitanksLevel*, m_pLevel);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CDigitanksGame);
	INPUT_DEFINE(Autosave);
	INPUT_DEFINE(PlayerVictory);
	INPUT_DEFINE(PlayerLoss);
	INPUT_DEFINE(TankSelectionMedal);
INPUTS_TABLE_END();

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

	SetListener(DigitanksWindow()->GetHUD());

	m_flLastFireworks = 0;
	m_bPartyMode = false;

	m_flShowFightSign = 0;
	m_flShowArtilleryTutorial = 0;

	m_bOverrideAllowLasers = false;

	m_pLevel = NULL;

	m_sObjective = L"Win the game";
}

void CDigitanksGame::RegisterNetworkFunctions()
{
	BaseClass::RegisterNetworkFunctions();

	CNetwork::RegisterFunction("SetupEntities", this, SetupEntitiesCallback, 0);
	CNetwork::RegisterFunction("EnterGame", this, EnterGameCallback, 0);
	CNetwork::RegisterFunction("EndTurn", this, EndTurnCallback, 0);
	CNetwork::RegisterFunction("StartTurn", this, StartTurnCallback, 0);
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

void CDigitanksGame::ClientUpdate(int iClient)
{
	BaseClass::ClientUpdate(iClient);

	GetTerrain()->ResyncClientTerrainData(iClient);
}

CVar game_difficulty("game_difficulty", "1");

void CDigitanksGame::SetupGame(gametype_t eGameType)
{
	GameServer()->SetLoading(true);

	DigitanksWindow()->RenderLoading();

	SetupEntities();

	if (!CNetwork::IsHost())
		return;

	m_eGameType = eGameType;
	m_iTurn = 0;

	m_pLevel = CDigitanksGame::GetLevel(CVar::GetCVarValue(L"game_level"));

	m_hInstructor = GameServer()->Create<CInstructorEntity>("CInstructorEntity");

	if (eGameType == GAMETYPE_STANDARD)
		SetupStrategy();
	else if (eGameType == GAMETYPE_ARTILLERY)
		SetupArtillery();
	else if (eGameType == GAMETYPE_MENU)
		SetupMenuMarch();
	else if (eGameType == GAMETYPE_CAMPAIGN)
		SetupCampaign();

	GameServer()->SetLoading(false);

	if (eGameType != GAMETYPE_EMPTY)
		StartGame();

	DigitanksGame()->SetDifficulty(game_difficulty.GetInt());
}

void CDigitanksGame::ReadGameScript(eastl::string16 sScript)
{
	for (size_t i = 0; i < m_aflConstructionCosts.size(); i++)
		m_aflConstructionCosts[i] = 0;
	for (size_t i = 0; i < m_aflUpgradeCosts.size(); i++)
		m_aflUpgradeCosts[i] = 0;

	std::ifstream f((eastl::string16(L"scripts/") + sScript).c_str());
	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	CData* pConstructionCosts = pData->FindChild("ConstructionCosts");

	if (pConstructionCosts)
	{
		CData* pChild;
		
		pChild = pConstructionCosts->FindChild("AutoTurret");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_AUTOTURRET] = pChild->GetValueFloat();

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

		pChild = pConstructionCosts->FindChild("ResistorLoader");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_INFANTRYLOADER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("DigitankLoader");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_TANKLOADER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("ArtilleryLoader");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_ARTILLERYLOADER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild("Resistor");
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
/*	// I'm changing the game settings code over to cvars and since this code is not used I'm just commenting it out.
	// I did try to change it to what it should be though.
	CDigitanksLevel* pLevel = CDigitanksGame::GetLevel(CVar::GetCVarValue(L"game_level"));

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
	}*/

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
	AddTeam(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));

	CDigitanksTeam* pTeam = GetDigitanksTeam(GetNumTeams()-1);
	pTeam->SetColor(Color(128, 128, 128));
	pTeam->SetTeamName(eastl::string16(L"Network Guardians"));
	pTeam->Bot_UseArtilleryAI();
	pTeam->SetNotHumanPlayable();
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

			CBugTurret* pTurret = GameServer()->Create<CBugTurret>("CBugTurret");
			pTurret->SetOrigin(m_hTerrain->SetPointHeight(Vector(x, 0, z)));
			pTeam->AddEntity(pTurret);
		}
	}
}

CVar game_players("game_players", "1");
CVar game_bots("game_bots", "3");
CVar game_tanks("game_tanks", "3");

void CDigitanksGame::SetupArtillery()
{
	TMsg(L"Setting up artillery game.\n");

	m_sObjective = L"Destroy all enemy tanks";

	int iPlayers = game_players.GetInt() + game_bots.GetInt();

	if (GameServer()->ShouldSetupFromLobby())
		iPlayers = CGameLobbyClient::L_GetNumPlayers();

	if (iPlayers > 8)
	{
		TAssert(!GameServer()->ShouldSetupFromLobby());

		iPlayers = 8;
		if (game_players.GetInt() > 8)
		{
			game_players.SetValue(8);
			game_bots.SetValue(0);
		}
		else
			game_bots.SetValue(8-game_players.GetInt());
	}

	if (iPlayers < 2)
	{
		iPlayers = 2;
		game_players.SetValue(1);
		game_bots.SetValue(1);
	}

	int iTanks = game_tanks.GetInt();

	if (GameServer()->ShouldSetupFromLobby())
		iTanks = _wtoi(CGameLobbyClient::L_GetInfoValue(L"tanks").c_str());

	if (iTanks > 4)
		iTanks = 4;
	if (iTanks < 1)
		iTanks = 1;
	game_tanks.SetValue(iTanks);

	eastl::vector<size_t> aiAvailableColors;
	if (GameServer()->ShouldSetupFromLobby())
	{
		for (int i = 0; i < 8; i++)
			aiAvailableColors.push_back(i);

		for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
		{
			CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
			eastl::string16 sColor = pPlayer->GetInfoValue(L"color");
			if (sColor == L"random" || sColor == L"")
				continue;

			size_t iColor = _wtoi(sColor.c_str());
			for (size_t j = 0; j < aiAvailableColors.size(); j++)
			{
				if (aiAvailableColors[j] == iColor)
				{
					aiAvailableColors.erase(aiAvailableColors.begin() + j);
					break;
				}
			}
		}
	}

	for (int i = 0; i < iPlayers; i++)
	{
		AddTeam(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));

		CDigitanksTeam* pTeam = GetDigitanksTeam(GetNumTeams()-1);

		if (GameServer()->ShouldSetupFromLobby())
		{
			CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
			if (pPlayer)
			{
				eastl::string16 sColor = pPlayer->GetInfoValue(L"color");
				if (sColor == L"random" || sColor == L"")
				{
					size_t iColor = RandomInt(0, aiAvailableColors.size()-1);
					pTeam->SetColor(g_aclrTeamColors[aiAvailableColors[iColor]]);
					pTeam->SetTeamName(g_aszTeamNames[aiAvailableColors[iColor]]);
					aiAvailableColors.erase(aiAvailableColors.begin()+iColor);
				}
				else
				{
					size_t iColor = _wtoi(sColor.c_str());
					pTeam->SetColor(g_aclrTeamColors[iColor]);
					pTeam->SetTeamName(g_aszTeamNames[iColor]);
				}
			}
			else
			{
				size_t iColor = RandomInt(0, aiAvailableColors.size()-1);
				pTeam->SetColor(g_aclrTeamColors[iColor]);
				pTeam->SetTeamName(g_aszTeamNames[iColor]);
				aiAvailableColors.erase(aiAvailableColors.begin()+iColor);
			}

			if (pPlayer && pPlayer->GetInfoValue(L"bot") != L"1")
				pTeam->SetTeamName(pPlayer->GetInfoValue(L"name"));

			if (pPlayer->GetInfoValue(L"bot") == L"1")
				pTeam->SetClient(-2);
			else
				pTeam->SetClient(pPlayer->iClient);
		}
		else
		{
			pTeam->SetColor(g_aclrTeamColors[i]);
			pTeam->SetTeamName(g_aszTeamNames[i]);

			if (game_players.GetInt() == 1 && i == 0)
			{
				eastl::string16 sPlayerNickname = TPortal_GetPlayerNickname();
				if (sPlayerNickname.length())
					pTeam->SetTeamName(sPlayerNickname);
			}

			pTeam->SetClient(-1);
		}
	}
}

void CDigitanksGame::SetupStrategy()
{
	TMsg(L"Setting up strategy game.\n");

	m_sObjective = L"Destroy all enemy CPUs";

	ReadGameScript(L"strategy.txt");

	m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
	m_hTerrain->GenerateTerrain();

	ScatterResources();
	SetupProps();
	ScatterNeutralUnits();

	int iPlayers = game_players.GetInt() + game_bots.GetInt();

	if (GameServer()->ShouldSetupFromLobby())
		iPlayers = CGameLobbyClient::L_GetNumPlayers();

	if (iPlayers > 4)
	{
		TAssert(!GameServer()->ShouldSetupFromLobby());

		iPlayers = 4;
		if (game_players.GetInt() > 4)
		{
			game_players.SetValue(1);
			game_bots.SetValue(3);
		}
		else
			game_bots.SetValue(4-game_players.GetInt());
	}

	if (iPlayers < 2)
	{
		iPlayers = 2;
		game_players.SetValue(1);
		game_bots.SetValue(1);
	}

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

	eastl::vector<size_t> aiAvailableColors;
	if (GameServer()->ShouldSetupFromLobby())
	{
		for (int i = 0; i < 8; i++)
			aiAvailableColors.push_back(i);

		for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
		{
			CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
			eastl::string16 sColor = pPlayer->GetInfoValue(L"color");
			if (sColor == L"random" || sColor == L"")
				continue;

			size_t iColor = _wtoi(sColor.c_str());
			for (size_t j = 0; j < aiAvailableColors.size(); j++)
			{
				if (aiAvailableColors[j] == iColor)
				{
					aiAvailableColors.erase(aiAvailableColors.begin() + j);
					break;
				}
			}
		}
	}

	for (int i = 0; i < iPlayers; i++)
	{
		AddTeam(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));

		CDigitanksTeam* pTeam = GetDigitanksTeam(GetNumTeams()-1);

		if (GameServer()->ShouldSetupFromLobby())
		{
			CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
			if (pPlayer)
			{
				eastl::string16 sColor = pPlayer->GetInfoValue(L"color");
				if (sColor == L"random" || sColor == L"")
				{
					size_t iColor = RandomInt(0, aiAvailableColors.size()-1);
					pTeam->SetColor(g_aclrTeamColors[aiAvailableColors[iColor]]);
					pTeam->SetTeamName(g_aszTeamNames[aiAvailableColors[iColor]]);
					aiAvailableColors.erase(aiAvailableColors.begin()+iColor);
				}
				else
				{
					size_t iColor = _wtoi(sColor.c_str());
					pTeam->SetColor(g_aclrTeamColors[iColor]);
					pTeam->SetTeamName(g_aszTeamNames[iColor]);
				}
			}
			else
			{
				size_t iColor = RandomInt(0, aiAvailableColors.size()-1);
				pTeam->SetColor(g_aclrTeamColors[iColor]);
				pTeam->SetTeamName(g_aszTeamNames[iColor]);
				aiAvailableColors.erase(aiAvailableColors.begin()+iColor);
			}

			if (pPlayer && pPlayer->GetInfoValue(L"bot") != L"1")
				pTeam->SetTeamName(pPlayer->GetInfoValue(L"name"));

			if (pPlayer->GetInfoValue(L"bot") == L"1")
				pTeam->SetClient(-2);
			else
				pTeam->SetClient(pPlayer->iClient);
		}
		else
		{
			pTeam->SetColor(g_aclrTeamColors[i]);
			pTeam->SetTeamName(g_aszTeamNames[i]);

			if (game_players.GetInt() == 1 && i == 0)
			{
				eastl::string16 sPlayerNickname = TPortal_GetPlayerNickname();
				if (sPlayerNickname.length())
					pTeam->SetTeamName(sPlayerNickname);
			}

			pTeam->SetClient(-1);
		}

		pTeam->SetLoseCondition(LOSE_NOCPU);

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

			CBugTurret* pBugTurret = dynamic_cast<CBugTurret*>(pEntity);
			if (pBugTurret && pTeam->GetVisibilityAtPoint(pBugTurret->GetOrigin()) > 0.5f)
				pBugTurret->Delete();
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

	if (GameServer()->ShouldSetupFromLobby())
	{
		for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
		{
			if (CGameLobbyClient::L_GetPlayer(i)->GetInfoValue(L"bot") == L"1")
				continue;

			// There's one neutral team at the front so skip it.
			m_ahTeams[i+1]->SetClient(CGameLobbyClient::L_GetPlayer(i)->iClient);
		}
	}
	else
	{
		for (int i = 0; i < game_players.GetInt(); i++)
			// There's one neutral team at the front so skip it.
			m_ahTeams[i+1]->SetClient(-1);
	}

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

void CDigitanksGame::SetupMenuMarch()
{
	TMsg(L"Setting up menu march.\n");

	m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
	m_hTerrain->GenerateTerrain();

	AddTeam(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));
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

void MissionReload(class CCommand* pCommand, eastl::vector<eastl::string16>& asTokens, const eastl::string16& sCommand)
{
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		if (pEntity == DigitanksGame())
			continue;

		if (dynamic_cast<CTerrain*>(pEntity))
			continue;

		if (dynamic_cast<CInstructorEntity*>(pEntity))
			continue;

		pEntity->Delete();
	}

	GameServer()->ReadLevels();
	DigitanksGame()->SetupCampaign(true);
}

CCommand mission_reload("mission_reload", ::MissionReload);

void MissionWin(class CCommand* pCommand, eastl::vector<eastl::string16>& asTokens, const eastl::string16& sCommand)
{
	if (!CVar::GetCVarBool("cheats"))
		return;

	DigitanksGame()->PlayerVictory(asTokens);
}

CCommand mission_win("mission_win", ::MissionWin);

void MissionLose(class CCommand* pCommand, eastl::vector<eastl::string16>& asTokens, const eastl::string16& sCommand)
{
	DigitanksGame()->PlayerLoss(asTokens);
}

CCommand mission_lose("mission_lose", ::MissionLose);

void CDigitanksGame::SetupCampaign(bool bReload)
{
	TMsg(sprintf(L"Setting up campaign %s.\n", CVar::GetCVarValue(L"game_level")));

	m_pLevel = CDigitanksGame::GetLevel(CVar::GetCVarValue(L"game_level"));

	m_sObjective = convertstring<char, char16_t>(m_pLevel->GetObjective());

	if (!bReload)
	{
		m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
		m_hTerrain->GenerateTerrain();
	}

	AddTeam(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));
	m_ahTeams[0]->SetColor(Color(0, 0, 255));

	m_ahTeams[0]->SetClient(-1);

	eastl::string16 sPlayerNickname = TPortal_GetPlayerNickname();
	if (sPlayerNickname.length())
		m_ahTeams[0]->SetTeamName(sPlayerNickname);

	GetDigitanksTeam(0)->SetLoseCondition(LOSE_NOTANKS);

	AddTeam(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));
	m_ahTeams[1]->SetColor(Color(255, 0, 0));

	m_ahTeams[1]->SetClient(-2);

	GetDigitanksTeam(1)->SetLoseCondition(LOSE_NONE);

	m_iPowerups = 0;

	if (!m_pLevel)
		return;

	for (size_t iUnits = 0; iUnits < m_pLevel->GetNumUnits(); iUnits++)
	{
		CLevelUnit* pLevelUnit = m_pLevel->GetUnit(iUnits);
		CBaseEntity* pEntity = NULL;
		
		if (pLevelUnit->m_sClassName == "Rogue")
			pEntity = GameServer()->Create<CScout>("CScout");
		else if (pLevelUnit->m_sClassName == "Resistor")
			pEntity = GameServer()->Create<CMechInfantry>("CMechInfantry");
		else if (pLevelUnit->m_sClassName == "BugTurret")
			pEntity = GameServer()->Create<CBugTurret>("CBugTurret");
		else if (pLevelUnit->m_sClassName == "GridBug")
			pEntity = GameServer()->Create<CGridBug>("CGridBug");
		else if (pLevelUnit->m_sClassName == "UserFile")
		{
			CUserFile* pUserFile = GameServer()->Create<CUserFile>("CUserFile");
			pUserFile->SetFile(pLevelUnit->m_sFile);
			pEntity = pUserFile;
		}
		else if (pLevelUnit->m_sClassName == "Powerup")
			pEntity = GameServer()->Create<CPowerup>("CPowerup");
		else if (pLevelUnit->m_sClassName == "CPU")
		{
			CCPU* pCPU = GameServer()->Create<CCPU>("CCPU");
			pCPU->AddFleetPoints(m_pLevel->GetBonusCPUFleetPoints());
			pEntity = pCPU;
		}
		else if (pLevelUnit->m_sClassName == "Counter")
			pEntity = GameServer()->Create<CBaseEntity>("CCounter");
		else if (pLevelUnit->m_sClassName == "Electronode")
			pEntity = GameServer()->Create<CResource>("CResource");
		else if (pLevelUnit->m_sClassName == "Capacitor")
			pEntity = GameServer()->Create<CBattery>("CBattery");
		else if (pLevelUnit->m_sClassName == "Buffer")
			pEntity = GameServer()->Create<CMiniBuffer>("CMiniBuffer");
		else if (pLevelUnit->m_sClassName == "AutoTurret")
			pEntity = GameServer()->Create<CStructure>("CAutoTurret");
		else if (pLevelUnit->m_sClassName == "ResistorFactory")
		{
			CLoader* pLoader = GameServer()->Create<CLoader>("CLoader");
			pLoader->SetBuildUnit(UNIT_INFANTRY);
			pEntity = pLoader;
		}
		else
		{
			TAssert(!"Invalid unit");
			continue;
		}

		pEntity->SetName(pLevelUnit->m_sName);
		pEntity->SetActive(pLevelUnit->m_bActive);

		for (size_t iOutputs = 0; iOutputs < pLevelUnit->m_aOutputs.size(); iOutputs++)
		{
			CLevelUnitOutput* pOutput = &pLevelUnit->m_aOutputs[iOutputs];
			pEntity->AddOutputTarget(pOutput->m_sOutput, pOutput->m_sTarget, pOutput->m_sInput, pOutput->m_sArgs, pOutput->m_bKill);
		}

		CDigitanksEntity* pUnit = dynamic_cast<CDigitanksEntity*>(pEntity);
		if (!pUnit)
			continue;

		pUnit->SetOrigin(m_hTerrain->SetPointHeight(Vector(pLevelUnit->m_vecPosition.x, 0, pLevelUnit->m_vecPosition.y)));
		pUnit->SetAngles(EAngle(0, pLevelUnit->m_angOrientation.y, 0));
		pUnit->SetObjective(pLevelUnit->m_bObjective);

		if (!pLevelUnit->m_bImprisoned)
		{
			if (pLevelUnit->m_sTeamName == "Player")
				m_ahTeams[0]->AddEntity(pUnit);
			else if (pLevelUnit->m_sTeamName == "Hackers")
				m_ahTeams[1]->AddEntity(pUnit);
		}

 		CDigitank* pTank = dynamic_cast<CDigitank*>(pUnit);

		if (pLevelUnit->m_bFortified && pTank)
			pTank->Fortify();

		if (pLevelUnit->m_bImprisoned)
			pUnit->Imprison();

		CPowerup* pPowerup = dynamic_cast<CPowerup*>(pUnit);
		if (pPowerup)
		{
			if (pLevelUnit->m_sType == "Airstrike")
				pPowerup->SetPowerupType(POWERUP_AIRSTRIKE);
			else if (pLevelUnit->m_sType == "Tank")
				pPowerup->SetPowerupType(POWERUP_TANK);
			else if (pLevelUnit->m_sType == "MissileDefense")
				pPowerup->SetPowerupType(POWERUP_MISSILEDEFENSE);
			else if (pLevelUnit->m_sType == "Bonus")
				pPowerup->SetPowerupType(POWERUP_BONUS);
			else if (pLevelUnit->m_sType == "Weapon")
				pPowerup->SetPowerupType(POWERUP_WEAPON);
		}

		// All starting tanks should stay put.
		// This means if they are fortified they should always stay fortified and not join an attack team.
		// If they are not fortified they should hang around to protect the base in case it is attacked.
		if (pTank)
			pTank->StayPut();

 		CStructure* pStructure = dynamic_cast<CStructure*>(pUnit);
		if (pStructure)
			pStructure->SetConstructing(false);

		pUnit->StartTurn();
	}

	if (m_pLevel->GetStartingLesson().length())
		DigitanksWindow()->GetInstructor()->DisplayFirstTutorial(m_pLevel->GetStartingLesson());
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

	if (!CNetwork::IsHost())
		return;

	while (m_ahTeams.size())
	{
		CTeam* pTeam = m_ahTeams[0];
		RemoveTeam(pTeam);
		pTeam->Delete();
	}

	// Just in case!
	TAssert(m_ahTeams.size() == 0);
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

CDigitanksLevel* CDigitanksGame::GetLevel(gametype_t eGameType, size_t i)
{
	return dynamic_cast<CDigitanksLevel*>(GetLevels(eGameType)[i]);
}

CDigitanksLevel* CDigitanksGame::GetLevel(eastl::string16 sFile)
{
	return dynamic_cast<CDigitanksLevel*>(GameServer()->GetLevel(sFile));
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

void CDigitanksGame::EnterGame()
{
	BaseClass::EnterGame();

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "EnterGame");
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

	m_bWaitingForMoving = false;
	m_bWaitingForProjectiles = false;

	if (m_pListener)
	{
		m_pListener->GameStart();

		m_pListener->SetHUDActive(GetCurrentTeam() == GetCurrentLocalDigitanksTeam());
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
		DigitanksWindow()->Restart(GAMETYPE_MENU);

	if (m_eGameType == GAMETYPE_STANDARD || m_eGameType == GAMETYPE_ARTILLERY || m_eGameType == GAMETYPE_CAMPAIGN)
		DigitanksGame()->GetDigitanksCamera()->EnterGame();
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

CVar game_terrainheight("game_terrainheight", "60");

void CDigitanksGame::SetupArtilleryRound()
{
	GameServer()->SetLoading(true);

	DigitanksWindow()->RenderLoading();

	eastl::vector<eastl::string> asSpare;
	asSpare.push_back("CDigitanksGame");
	asSpare.push_back("CDigitanksTeam");

	GameServer()->DestroyAllEntities(asSpare);

	float flTerrainHeight = game_terrainheight.GetFloat();
	if (GameServer()->ShouldSetupFromLobby())
		flTerrainHeight = (float)_wtof(CGameLobbyClient::L_GetInfoValue(L"terrain").c_str());

	m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
	m_hTerrain->GenerateTerrain(flTerrainHeight);

	float flMapBuffer = GetTerrain()->GetMapSize()*0.1f;
	float flMapSize = GetTerrain()->GetMapSize() - flMapBuffer*2;

	size_t iPlayers = (game_players.GetInt() + game_bots.GetInt());
	if (GameServer()->ShouldSetupFromLobby())
		iPlayers = CGameLobbyClient::L_GetNumPlayers();

	size_t iTanksPerPlayer = game_tanks.GetInt();
	if (GameServer()->ShouldSetupFromLobby())
		iTanksPerPlayer = _wtoi(CGameLobbyClient::L_GetInfoValue(L"tanks").c_str());

	size_t iTotalTanks = iTanksPerPlayer * iPlayers;

	size_t iSections = (int)sqrt((float)iTotalTanks);
	float flSectionSize = flMapSize*2/iSections;

	eastl::vector<size_t> aiRandomTeamPositions;
	// 8 random starting positions.
	for (size_t i = 0; i < iPlayers; i++)
	{
		for (size_t j = 0; j < iTanksPerPlayer; j++)
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

void CDigitanksGame::Autosave(const eastl::vector<eastl::string16>& sArgs)
{
	Autosave();
}

void CDigitanksGame::Autosave()
{
	GameServer()->SaveToFile(GetAppDataDirectory(DigitanksWindow()->AppDirectory(), L"autosave.sav").c_str());

	DigitanksWindow()->GetChatBox()->PrintChat(L"Autosave...\n");
}

void CDigitanksGame::Think()
{
	BaseClass::Think();

	if (m_flShowFightSign > 0 && m_flShowFightSign < GameServer()->GetGameTime())
	{
		DigitanksWindow()->GetHUD()->ShowFightSign();
		m_flShowFightSign = 0;
	}

	if (m_flShowArtilleryTutorial > 0 && m_flShowArtilleryTutorial < GameServer()->GetGameTime())
	{
		DigitanksWindow()->GetInstructor()->DisplayFirstTutorial("artillery-select");
		m_flShowArtilleryTutorial = 0;
	}

	if (GetGameType() == GAMETYPE_MENU)
		return;

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
	if (GetCurrentTeam() && !GetCurrentTeam()->IsPlayerControlled())
	{
		CTeam* pNextTeam = GetTeam((m_iCurrentTeam+(size_t)1)%GetNumTeams());
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
		if (GetGameType() == GAMETYPE_CAMPAIGN)
		{
			if (GameServer()->GetGameTime() - m_flPartyModeStart > 5)
			{
				if (!DigitanksGame()->GetCurrentLocalDigitanksTeam()->HasLost())
					DigitanksWindow()->NextCampaignLevel();
				return;
			}
		}

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

			if (ahEntities.size())
			{
				CDigitanksEntity* pEntity = ahEntities[RandomInt(0, ahEntities.size()-1)];

				CFireworks* pFireworks = GameServer()->Create<CFireworks>("CFireworks");
				pFireworks->SetOrigin(pEntity->GetOrigin());
				pFireworks->SetOwner(NULL);
				pFireworks->SetVelocity(Vector(RandomFloat(-8, 8), 45, RandomFloat(-8, 8)));
				pFireworks->SetGravity(Vector(0, DigitanksGame()->GetGravity(), 0));
			}

			m_flLastFireworks = GameServer()->GetGameTime();
		}
	}

	if (CNetwork::IsHost())
	{
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

	size_t iFormation = 0;

	CDigitanksTeam* pTeam = GetCurrentTeam();
	for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
	{
		CDigitank* pTank = pTeam->GetTank(i);

		if (!pTank->MovesWith(pCurrentTank))
			continue;

		Vector vecNewPosition = GetFormationPosition(vecPreview, vecMove, pTeam->GetNumTanks(), iFormation++);

		vecNewPosition.y = pTank->FindHoverHeight(vecNewPosition);

		pTank->SetPreviewMove(vecNewPosition);

		if (!pTank->IsPreviewMoveValid())
			pTank->SetGoalMovePosition(vecNewPosition);
		else
			pTank->Move();

		GetCurrentLocalDigitanksTeam()->HandledActionItem(pTank);

		if (pTank->GetUnitType() == UNIT_MOBILECPU)
			DigitanksWindow()->GetInstructor()->FinishedTutorial("strategy-command", true);
	}

	SetControlMode(MODE_NONE);

	DigitanksWindow()->GetInstructor()->FinishedTutorial("mission-1-move");
	DigitanksWindow()->GetInstructor()->FinishedTutorial("strategy-command");
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

		GetCurrentLocalDigitanksTeam()->HandledActionItem(pTank);
	}

	SetControlMode(MODE_NONE);

	DigitanksWindow()->GetInstructor()->FinishedTutorial("artillery-command", true);
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

		GetCurrentLocalDigitanksTeam()->HandledActionItem(pTank);
	}

	DigitanksWindow()->GetInstructor()->FinishedTutorial("artillery-command", true);
	SetControlMode(MODE_NONE);
}

void CDigitanksGame::EndTurn()
{
	CNetwork::CallFunction(NETWORK_TOEVERYONE, "EndTurn");

	EndTurn(NULL);

	DigitanksWindow()->GetInstructor()->FinishedTutorial("artillery-endturn");
//	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_POWER);
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
		DigitanksWindow()->Restart(GAMETYPE_MENU);
		return;
	}

	int iPowerupChance;
	if (GetGameType() == GAMETYPE_STANDARD)
		iPowerupChance = RandomInt(0, 6);
	else
		iPowerupChance = RandomInt(0, 3);

	if (GetGameType() != GAMETYPE_MENU && GetGameType() != GAMETYPE_CAMPAIGN && m_iPowerups < 10 && iPowerupChance == 0)
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
				if (GetDifficulty() == 0)
					iGridBugs /= 2;

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

	if (GetGameType() != GAMETYPE_CAMPAIGN && m_iCurrentTeam == (size_t)0 && m_iTurn > 10 && m_iTurn%5 == 0)
		Autosave(eastl::vector<eastl::string16>());

	if (++m_iCurrentTeam >= GetNumTeams())
		m_iCurrentTeam = 0;

	m_iWaitingForProjectiles = 0;

	m_bTurnActive = true;

	GetCurrentTeam()->StartTurn();

	if (m_pListener)
	{
		m_pListener->SetHUDActive(GetCurrentTeam() == GetCurrentLocalDigitanksTeam());
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

	if (pWeapon && (pWeapon->GetWeaponType() == PROJECTILE_FLAK || pWeapon->GetWeaponType() == PROJECTILE_SPLOOGE))
		iRunners = RandomInt(2, 4);

	for (int i = 0; i < iRunners; i++)
		GetTerrain()->AddRunner(vecExplosionOrigin, Color(255, 255, 255), 1);

	return bHit;
}

SERVER_COMMAND(HitIndicator)
{
	if (pCmd->GetNumArguments() == 0)
	{
		TMsg(L"HitIndicator with 0 arguments.\n");
		return;
	}

	if (pCmd->Arg(0) == L"sdmg")
	{
		if (pCmd->GetNumArguments() < 7)
		{
			TMsg(L"HitIndicator sdmg with not enough arguments.\n");
			return;
		}

		if (DigitanksGame()->GetListener())
		{
			DigitanksGame()->GetListener()->OnTakeShieldDamage(
					CEntityHandle<CDigitank>(pCmd->ArgAsUInt(1)),
					CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(2)),
					CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(3)),
					pCmd->ArgAsFloat(4), !!pCmd->ArgAsInt(5), !!pCmd->ArgAsInt(6));
		}

		return;
	}

	if (pCmd->Arg(0) == L"dmg")
	{
		if (pCmd->GetNumArguments() < 7)
		{
			TMsg(L"HitIndicator dmg with not enough arguments.\n");
			return;
		}

		if (DigitanksGame()->GetListener())
		{
			DigitanksGame()->GetListener()->OnTakeDamage(
					CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(1)),
					CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(2)),
					CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(3)),
					pCmd->ArgAsFloat(4), !!pCmd->ArgAsInt(5), !!pCmd->ArgAsInt(6));
		}

		return;
	}

	if (pCmd->Arg(0) == L"disable")
	{
		if (pCmd->GetNumArguments() < 4)
		{
			TMsg(L"HitIndicator disable with not enough arguments.\n");
			return;
		}

		if (DigitanksGame()->GetListener())
		{
			DigitanksGame()->GetListener()->OnDisabled(
					CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(1)),
					CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(2)),
					CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(3))
				);
		}

		return;
	}

	if (pCmd->Arg(0) == L"miss")
	{
		if (pCmd->GetNumArguments() < 4)
		{
			TMsg(L"HitIndicator miss with not enough arguments.\n");
			return;
		}

		if (DigitanksGame()->GetListener())
		{
			DigitanksGame()->GetListener()->OnMiss(
					CEntityHandle<CDigitank>(pCmd->ArgAsUInt(1)),
					CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(2)),
					CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(3))
				);
		}

		return;
	}
}

#define SAFE_HANDLE(pEntity) pEntity?pEntity->GetHandle():~0

void CDigitanksGame::OnTakeShieldDamage(CDigitank* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly)
{
	if (CNetwork::IsHost())
		HitIndicator.RunCommand(sprintf(L"sdmg %d %d %d %f %d %d", SAFE_HANDLE(pVictim), SAFE_HANDLE(pAttacker), SAFE_HANDLE(pInflictor), flDamage, bDirectHit, bShieldOnly));
}

void CDigitanksGame::OnTakeDamage(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled)
{
	if (CNetwork::IsHost())
		HitIndicator.RunCommand(sprintf(L"dmg %d %d %d %f %d %d", SAFE_HANDLE(pVictim), SAFE_HANDLE(pAttacker), SAFE_HANDLE(pInflictor), flDamage, bDirectHit, bKilled));
}

void CDigitanksGame::OnDisabled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	if (CNetwork::IsHost())
	{
		HitIndicator.RunCommand(sprintf(L"disable %d %d %d", SAFE_HANDLE(pVictim), SAFE_HANDLE(pAttacker), SAFE_HANDLE(pInflictor)));

		CDigitank* pTank = dynamic_cast<CDigitank*>(pVictim);
		if (pTank)
			pTank->Speak(TANKSPEECH_DISABLED);
	}
}

void CDigitanksGame::OnMiss(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	if (CNetwork::IsHost())
	{
		HitIndicator.RunCommand(sprintf(L"miss %d %d %d", SAFE_HANDLE(pVictim), SAFE_HANDLE(pAttacker), SAFE_HANDLE(pInflictor)));

		CDigitank* pTank = dynamic_cast<CDigitank*>(pVictim);
		if (pTank)
			pTank->Speak(TANKSPEECH_TAUNT);
	}
}

void CDigitanksGame::OnKilled(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
		m_ahTeams[i]->OnKilled(pEntity);

	CheckWinConditions();
}

void CDigitanksGame::CheckWinConditions()
{
	if (m_eGameType == GAMETYPE_MENU)
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

		default:
			iTeamsLeft++;
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

void CDigitanksGame::PlayerVictory(const eastl::vector<eastl::string16>& sArgs)
{
	for (size_t i = 0; i < GetNumTeams(); i++)
	{
		if (!GetTeam(i))
			continue;

		if (!GetTeam(i)->IsPlayerControlled())
			GetDigitanksTeam(i)->YouLoseSirGoodDay();
	}

	GameOver();
}

void CDigitanksGame::PlayerLoss(const eastl::vector<eastl::string16>& sArgs)
{
	for (size_t i = 0; i < GetNumTeams(); i++)
	{
		if (!GetTeam(i))
			continue;

		if (GetTeam(i)->IsPlayerControlled())
			GetDigitanksTeam(i)->YouLoseSirGoodDay();
	}

	GameOver();
}

void CDigitanksGame::TankSelectionMedal(const eastl::vector<eastl::string16>& sArgs)
{
	DigitanksWindow()->GetHUD()->ShowTankSelectionMedal();
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
	return static_cast<CDigitanksTeam*>(BaseClass::GetTeam(i));
}

CDigitanksTeam* CDigitanksGame::GetCurrentTeam()
{
	if (m_iCurrentTeam >= m_ahTeams.size())
		return NULL;

	// Force the const version so we don't send it over the wire.
	const CNetworkedSTLVector<CEntityHandle<CTeam> >& ahTeams = m_ahTeams;
	return static_cast<CDigitanksTeam*>(ahTeams[m_iCurrentTeam].GetPointer());
}

CSelectable* CDigitanksGame::GetPrimarySelection()
{
	if (!GetCurrentLocalDigitanksTeam())
		return NULL;

	return GetCurrentLocalDigitanksTeam()->GetPrimarySelection();
}

CDigitank* CDigitanksGame::GetPrimarySelectionTank()
{
	return dynamic_cast<CDigitank*>(GetPrimarySelection());
}

CStructure* CDigitanksGame::GetPrimarySelectionStructure()
{
	return dynamic_cast<CStructure*>(GetPrimarySelection());
}

Vector CDigitanksGame::GetFormationPosition(Vector vecPosition, Vector vecFacing, size_t iUnitsInFormation, size_t iPosition)
{
	vecFacing.y = 0;
	vecFacing.Normalize();

	if (iPosition == 0)
		return vecPosition;

	float flSeparation = 10;
	if (GetGameType() == GAMETYPE_ARTILLERY)
		flSeparation = 12;

	Vector vecRight = Vector(0,1,0).Cross(vecFacing).Normalized();
	if (iPosition%2 == 1)
		return vecPosition + vecRight*flSeparation*(float)((iPosition+1)/2);

	if (iPosition%2 == 0)
		return vecPosition - vecRight*flSeparation*(float)(iPosition/2);

	return vecPosition;
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
		DigitanksWindow()->GetInstructor()->FinishedTutorial("mission-1-turret-spotted");
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

CLIENT_COMMAND(WeaponSpecial)
{
	if (CNetwork::IsRunningClientFunctions() && (DigitanksGame()->GetCurrentTeam()->GetClient() != (int)iClient))
		return;

	DigitanksGame()->WeaponSpecialCommand(DigitanksGame()->GetCurrentTeam());
}

void CDigitanksGame::WeaponSpecialCommand(CDigitanksTeam* pTeam)
{
	if (DigitanksGame()->GetGameType() != GAMETYPE_ARTILLERY)
		return;

	if (!pTeam)
	{
		::WeaponSpecial.RunCommand(L"");
		return;
	}

	if (pTeam != GetCurrentTeam())
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
		if (pTeam != pWeapon->GetOwner()->GetTeam())
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

	DigitanksWindow()->GetHUD()->ClearHintWeapon();
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

void CDigitanksGame::OnDisplayTutorial(eastl::string sTutorial)
{
	/*
	if (sTutorial == CInstructor::TUTORIAL_INTRO_BASICS)
	{
		CDigitank* pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[0]->AddEntity(pTank);
		pTank->StartTurn();

		pTank->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, 0)));

		GetDigitanksCamera()->SnapTarget(pTank->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, 0, 0));
	}
	else if (sTutorial == CInstructor::TUTORIAL_SELECTION)
		GetDigitanksCamera()->SetTarget(GetDigitanksTeam(0)->GetTank(0)->GetOrigin());
	else if (sTutorial == CInstructor::TUTORIAL_MOVE_MODE)
	{
		// Make an enemy for us to clobber. Close enough that moving out of the way won't move us out of range
		CDigitank* pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[1]->AddEntity(pTank);

		pTank->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, -50)));

		GetDigitanksCamera()->SetTarget(GetDigitanksTeam(0)->GetTank(0)->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, 0, 0));
	}
	else if (sTutorial == CInstructor::TUTORIAL_POWERUP)
	{
		CPowerup* pPowerup = GameServer()->Create<CPowerup>("CPowerup");
		pPowerup->SetOrigin(GetTerrain()->SetPointHeight(GetDigitanksTeam(0)->GetTank(0)->GetOrigin() + Vector(0, 0, -10)));
	}
	else if (sTutorial == CInstructor::TUTORIAL_SHIFTSELECT)
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
	else if (sTutorial == CInstructor::TUTORIAL_THEEND_BASICS)
	{
		// So that pressing the escape key works the first time.
		SetControlMode(MODE_NONE);
	}
	else if (sTutorial == CInstructor::TUTORIAL_INTRO_BASES)
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
	else if (sTutorial == CInstructor::TUTORIAL_THEEND_BASES)
	{
		// So that pressing the escape key works the first time.
		SetControlMode(MODE_NONE);
	}
	else if (sTutorial == CInstructor::TUTORIAL_INTRO_UNITS)
	{
		CDigitank* pTank = GameServer()->Create<CMechInfantry>("CMechInfantry");
		m_ahTeams[0]->AddEntity(pTank);
		pTank->StartTurn();

		pTank->SetOrigin(GetTerrain()->SetPointHeight(Vector(0, 0, 0)));

		GetDigitanksCamera()->SnapTarget(pTank->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, 0, 0));

		GetDigitanksTeam(0)->CalculateVisibility();

		DigitanksGame()->GetTerrain()->CalculateVisibility();
	}
	else if (sTutorial == CInstructor::TUTORIAL_ARTILLERY)
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

		DigitanksGame()->GetTerrain()->CalculateVisibility();
	}
	else if (sTutorial == CInstructor::TUTORIAL_FIRE_ARTILLERY)
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
	else if (sTutorial == CInstructor::TUTORIAL_ROGUE)
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

		DigitanksGame()->GetTerrain()->CalculateVisibility();
	}
	else if (sTutorial == CInstructor::TUTORIAL_TORPEDO)
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
	}*/

	if (sTutorial == "strategy-command")
		SetControlMode(MODE_MOVE);

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
	else if (m_eGameType != GAMETYPE_CAMPAIGN)
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
	{
		m_flShowFightSign = GameServer()->GetGameTime() + 1.0f;

		// Wait until after the view dolly-in to show the tutorial, so the dolly-in doesn't distract
		m_flShowArtilleryTutorial = GameServer()->GetGameTime() + 3.0f;
	}

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
	bool bDisableBuffer = DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_BUFFER);
	return !bDisableBuffer;
}

bool CDigitanksGame::CanBuildBuffers()
{
	if (!m_pLevel)
		return true;

	return m_pLevel->AllowBuffers();
}

bool CDigitanksGame::CanBuildBatteries()
{
	return true;
}

bool CDigitanksGame::CanBuildPSUs()
{
	if (!m_pLevel)
		return true;

	if (!m_pLevel->AllowPSUs())
		return false;

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
	if (!m_pLevel)
		return true;

	if (!m_pLevel->AllowTankLoaders())
		return false;

	bool bDisableLoaders = DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

bool CDigitanksGame::CanBuildArtilleryLoaders()
{
	if (!m_pLevel)
		return true;

	if (!m_pLevel->AllowArtilleryLoaders())
		return false;

	bool bDisableLoaders = DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

bool CDigitanksGame::IsWeaponAllowed(weapon_t eWeapon, const CDigitank* pTank)
{
	if (!m_pLevel)
		return false;

	if (eWeapon == WEAPON_INFANTRYLASER)
	{
		if (m_bOverrideAllowLasers)
			return true;

		// Enemy tanks have access to this weapon from the first mission.
		if (DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN && pTank && pTank->GetTeam() && !pTank->GetTeam()->IsPlayerControlled())
			return m_pLevel->AllowEnemyInfantryLasers();

		return m_pLevel->AllowInfantryLasers();
	}

	if (eWeapon == PROJECTILE_TREECUTTER)
	{
		// Enemy tanks have access to this weapon from the first mission.
		if (DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN && pTank && pTank->GetTeam() && !pTank->GetTeam()->IsPlayerControlled())
			return true;

		return m_pLevel->AllowInfantryTreeCutters();
	}

	return true;
}

bool CDigitanksGame::IsInfantryFortifyAllowed()
{
	if (!m_pLevel)
		return false;

	return m_pLevel->AllowInfantryFortify();
}

void CDigitanksGame::AllowLaser()
{
	m_bOverrideAllowLasers = true;
}

void CDigitanksGame::BeginAirstrike(Vector vecLocation)
{
	if (!CNetwork::IsHost())
		return;

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
	size_t iLocalTeams = GetNumLocalTeams();

	if (!iLocalTeams)
		return NULL;

	if (iLocalTeams == 1)
		return static_cast<CDigitanksTeam*>(GetLocalTeam(0));

	for (size_t i = 0; i < iLocalTeams; i++)
	{
		CTeam* pTeam = GetLocalTeam(i);
		if (GetCurrentTeam() == pTeam)
			return static_cast<CDigitanksTeam*>(pTeam);
	}

	return NULL;
}

bool CDigitanksGame::SoftCraters()
{
	return m_eGameType == GAMETYPE_STANDARD || m_eGameType == GAMETYPE_CAMPAIGN;
}

void CDigitanksGame::UpdateHUD(CNetworkedVariableBase* pVariable)
{
	CHUD::SetNeedsUpdate();
}

void CDigitanksGame::UpdateTeamMembers(CNetworkedVariableBase* pVariable)
{
	CHUD::SetTeamMembersUpdated();
}
