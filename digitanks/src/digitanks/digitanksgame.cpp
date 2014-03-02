#include "digitanksgame.h"

#include <maths.h>
#include <mtrand.h>
#include <strutils.h>
#include <tinker_platform.h>

#include <datamanager/dataserializer.h>
#include <models/models.h>
#include <sound/sound.h>
#include <renderer/particles.h>
#include <tinker/portals/portal.h>
#include <tinker/cvar.h>
#include <tengine/lobby/lobby_client.h>
#include <game/gameserver.h>
#include <network/network.h>
#include <network/commands.h>
#include <tengine/ui/chatbox.h>
#include <game/cameramanager.h>

#include <ui/digitankswindow.h>
#include <ui/ui.h>
#include <ui/instructor.h>
#include <ui/hud.h>
#include "powerup.h"
#include "terrain.h"
#include "dt_camera.h"
#include <ui/weaponpanel.h>
#include "ui/menu.h"

#include "menumarcher.h"
#include "units/standardtank.h"
#include "units/mechinf.h"
#include "units/maintank.h"
#include "units/artillery.h"
#include "units/scout.h"
#include "units/mobilecpu.h"
#include "units/barbarians.h"
#include "structures/cpu.h"
#include "structures/buffer.h"
#include "weapons/projectile.h"
#include "weapons/specialshells.h"
#include "dt_renderer.h"
#include "structures/resource.h"
#include "structures/loader.h"
#include "structures/props.h"
#include "digitankslevel.h"
#include "campaign/userfile.h"
#include "instructor_entity.h"
#include "campaign/campaignentity.h"

CGame* CreateGame()
{
	return GameServer()->Create<CDigitanksGame>("CDigitanksGame");
}

CResource<CLevel> CreateLevel()
{
	return CResource<CLevel>(new CDigitanksLevel());
}

CHUDViewport* CreateHUD()
{
	return new CHUD();
}

tstring GetInitialGameMode()
{
	return "menu";
}

pfnConditionsMet Game_GetInstructorConditions(const tstring& sConditions)
{
	return false;
}

REGISTER_ENTITY(CDigitanksGame);

NETVAR_TABLE_BEGIN(CDigitanksGame);
	NETVAR_DEFINE(size_t, m_iCurrentPlayer);
	NETVAR_DEFINE(tstring, m_sObjective);
	NETVAR_DEFINE(CEntityHandle<CTerrain>, m_hTerrain);
	NETVAR_DEFINE(size_t, m_iDifficulty);
	NETVAR_DEFINE(bool, m_bRenderFogOfWar);
	NETVAR_DEFINE(gametype_t, m_eGameType);
	NETVAR_DEFINE(size_t, m_iTurn);
	NETVAR_DEFINE(CEntityHandle<CUpdateGrid>, m_hUpdates);
	NETVAR_DEFINE(bool, m_bPartyMode);
	NETVAR_DEFINE(float, m_aflConstructionCosts);
	NETVAR_DEFINE(float, m_aflUpgradeCosts);
	NETVAR_DEFINE(bool, m_bLevelAllowsBuffers);
	NETVAR_DEFINE(bool, m_bLevelAllowsPSUs);
	NETVAR_DEFINE(bool, m_bLevelAllowsTankLoaders);
	NETVAR_DEFINE(bool, m_bLevelAllowsArtilleryLoaders);
	NETVAR_DEFINE(bool, m_bLevelAllowsInfantryLasers);
	NETVAR_DEFINE(bool, m_bLevelAllowsInfantryTreeCutters);
	NETVAR_DEFINE(bool, m_bLevelAllowsInfantryFortify);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitanksGame);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iCurrentPlayer);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, controlmode_t, m_eControlMode);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, aimtype_t, m_eAimType);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, tstring, m_sObjective);
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
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bLevelAllowsBuffers);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bLevelAllowsPSUs);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bLevelAllowsTankLoaders);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bLevelAllowsArtilleryLoaders);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bLevelAllowsInfantryLasers);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bLevelAllowsInfantryTreeCutters);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bLevelAllowsInfantryFortify);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CDigitanksGame);
	INPUT_DEFINE(Autosave);
	INPUT_DEFINE(CancelAutoMoves);
	INPUT_DEFINE(PlayerVictory);
	INPUT_DEFINE(PlayerLoss);
	INPUT_DEFINE(TankSelectionMedal);
INPUTS_TABLE_END();

CDigitanksGame::CDigitanksGame()
{
	m_eGameType = GAMETYPE_EMPTY;
}

void CDigitanksGame::Precache()
{
	BaseClass::Precache();

	// We precache this for the hud since it's not an entity
	PrecacheSound("sound/actionsign.wav");

	PrecacheModel("models/skybox/ring1.toy");
	PrecacheModel("models/skybox/ring2.toy");
	PrecacheModel("models/skybox/ring3.toy");
	PrecacheModel("models/skybox/vortex.toy");
	PrecacheModel("models/skybox/digiverse.toy");
	PrecacheModel("models/skybox/floaters/float01.toy");
	PrecacheModel("models/skybox/floaters/float02.toy");
	PrecacheModel("models/skybox/floaters/float03.toy");
	PrecacheModel("models/skybox/floaters/float04.toy");
	PrecacheModel("models/skybox/floaters/float05.toy");
	PrecacheModel("models/skybox/floaters/float06.toy");
	PrecacheModel("models/skybox/floaters/float07.toy");
	PrecacheModel("models/skybox/floaters/float08.toy");
	PrecacheModel("models/skybox/floaters/float09.toy");
	PrecacheModel("models/skybox/floaters/float10.toy");
	PrecacheModel("models/skybox/floaters/float11.toy");
	PrecacheModel("models/skybox/floaters/float12.toy");
	PrecacheModel("models/skybox/floaters/float13.toy");
	PrecacheModel("models/skybox/floaters/float14.toy");
	PrecacheModel("models/skybox/floaters/float15.toy");
}

void CDigitanksGame::Spawn()
{
	BaseClass::Spawn();

	m_iCurrentPlayer = 0;
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

	m_sObjective = "Win the game";
}

void CDigitanksGame::RegisterNetworkFunctions()
{
	BaseClass::RegisterNetworkFunctions();

	GameNetwork()->RegisterFunction("SetupEntities", this, SetupEntitiesCallback, 0);
	GameNetwork()->RegisterFunction("EnterGame", this, EnterGameCallback, 0);
	GameNetwork()->RegisterFunction("EndTurn", this, EndTurnCallback, 0);
	GameNetwork()->RegisterFunction("StartTurn", this, StartTurnCallback, 0);
	GameNetwork()->RegisterFunction("Move", this, MoveCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	GameNetwork()->RegisterFunction("Turn", this, TurnCallback, 2, NET_HANDLE, NET_FLOAT);
	GameNetwork()->RegisterFunction("Fire", this, FireCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	GameNetwork()->RegisterFunction("FireWeapon", this, FireWeaponCallback, 5, NET_HANDLE, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	GameNetwork()->RegisterFunction("SetBonusPoints", this, SetBonusPointsCallback, 5, NET_HANDLE, NET_INT, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	GameNetwork()->RegisterFunction("TankPromoted", this, TankPromotedCallback, 1, NET_HANDLE);
	GameNetwork()->RegisterFunction("PromoteAttack", this, PromoteAttackCallback, 1, NET_HANDLE);
	GameNetwork()->RegisterFunction("PromoteDefense", this, PromoteDefenseCallback, 1, NET_HANDLE);
	GameNetwork()->RegisterFunction("PromoteMovement", this, PromoteMovementCallback, 1, NET_HANDLE);
	GameNetwork()->RegisterFunction("TankSpeak", this, SpeakCallback, 2, NET_HANDLE, NET_INT);
	GameNetwork()->RegisterFunction("Fortify", this, FortifyCallback, 1, NET_HANDLE);
	GameNetwork()->RegisterFunction("Sentry", this, SentryCallback, 1, NET_HANDLE);
	GameNetwork()->RegisterFunction("SetGoalMovePosition", this, SetGoalMovePositionCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	GameNetwork()->RegisterFunction("CancelGoalMovePosition", this, CancelGoalMovePositionCallback, 1, NET_HANDLE);

	GameNetwork()->RegisterFunction("TerrainData", this, TerrainDataCallback, 0);

	// CUpdateGrid
	GameNetwork()->RegisterFunction("UpdatesData", this, UpdatesDataCallback, 0);

	// CDigitanksPlayer
	GameNetwork()->RegisterFunction("DownloadUpdate", this, DownloadUpdateCallback, 0);
	GameNetwork()->RegisterFunction("DownloadComplete", this, DownloadCompleteCallback, 0);

	// CPU
	GameNetwork()->RegisterFunction("BeginConstruction", this, BeginConstructionCallback, 0);
	GameNetwork()->RegisterFunction("BeginRogueProduction", this, BeginRogueProductionCallback, 0);

	// CStructure
	GameNetwork()->RegisterFunction("BeginStructureConstruction", this, BeginStructureConstructionCallback, 1, NET_HANDLE);
	GameNetwork()->RegisterFunction("InstallUpdate", this, InstallUpdateCallback, 1, NET_HANDLE);
	GameNetwork()->RegisterFunction("BeginUpgrade", this, BeginUpgradeCallback, 1, NET_HANDLE);

	// CSupplier
	GameNetwork()->RegisterFunction("AddChild", this, AddChildCallback, 2, NET_HANDLE, NET_HANDLE);
	GameNetwork()->RegisterFunction("RemoveChild", this, RemoveChildCallback, 2, NET_HANDLE, NET_HANDLE);

	// CLoader
	GameNetwork()->RegisterFunction("BeginProduction", this, BeginProductionCallback, 1, NET_HANDLE);
}

void CDigitanksGame::ClientUpdate(int iClient)
{
	BaseClass::ClientUpdate(iClient);

	GetTerrain()->ResyncClientTerrainData(iClient);
}

CVar game_difficulty("game_difficulty", "1");

void CDigitanksGame::SetupGame(tstring sGameType)
{
	if (sGameType == "from_cvar")
		sGameType = CVar::GetCVarValue("game_type");
	else if (sGameType == "from_lobby")
	{
		TUnimplemented(); // This info value will be an integer string, eg "1". Will have to be changed.
		sGameType = CGameLobbyClient::L_GetInfoValue("gametype");
	}

	GameServer()->SetLoading(true);

	DigitanksWindow()->RenderLoading();

	if (sGameType != "menu")
		CSoundLibrary::StopMusic();

	if (sGameType == "menu")
	{
		if (!CSoundLibrary::IsMusicPlaying() && !Application()->HasCommandLineSwitch("--no-music"))
			CSoundLibrary::PlayMusic("sound/assemble-for-victory.ogg", true);
	}
	else if (!Application()->HasCommandLineSwitch("--no-music"))
		CSoundLibrary::PlayMusic("sound/network-rise-network-fall.ogg", true);

	SetupEntities();

	if (!GameNetwork()->IsHost())
		return;

	if (sGameType == "menu")
		m_eGameType = GAMETYPE_MENU;
	else if (sGameType == "artillery")
		m_eGameType = GAMETYPE_ARTILLERY;
	else if (sGameType == "standard")
		m_eGameType = GAMETYPE_STANDARD;
	else if (sGameType == "campaign")
		m_eGameType = GAMETYPE_CAMPAIGN;
	else
		m_eGameType = GAMETYPE_EMPTY;

	m_iTurn = 0;

	SetCurrentLevel(CVar::GetCVarValue("game_level"));

	m_hInstructor = GameServer()->Create<CInstructorEntity>("CInstructorEntity");

	if (m_eGameType == GAMETYPE_STANDARD)
		SetupStrategy();
	else if (m_eGameType == GAMETYPE_ARTILLERY)
		SetupArtillery();
	else if (m_eGameType == GAMETYPE_MENU)
		SetupMenuMarch();
	else if (m_eGameType == GAMETYPE_CAMPAIGN)
		SetupCampaign();

	if (m_eGameType != GAMETYPE_EMPTY && m_eGameType != GAMETYPE_MENU)
		StartGame();

	DigitanksGame()->SetDifficulty(game_difficulty.GetInt());

	GameServer()->SetLoading(false);

	DigitanksWindow()->GetMainMenu()->SetVisible(m_eGameType == GAMETYPE_MENU);
	DigitanksWindow()->GetVictoryPanel()->SetVisible(false);
}

void CDigitanksGame::ReadGameScript(tstring sScript)
{
	for (size_t i = 0; i < m_aflConstructionCosts.size(); i++)
		m_aflConstructionCosts[i] = 0;
	for (size_t i = 0; i < m_aflUpgradeCosts.size(); i++)
		m_aflUpgradeCosts[i] = 0;

	std::basic_ifstream<tchar> f((tstring("scripts/") + convertstring<tchar, char>(sScript)).c_str());
	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	CData* pConstructionCosts = pData->FindChild("ConstructionCosts");

	if (pConstructionCosts)
	{
		CData* pChild;
		
		pChild = pConstructionCosts->FindChild("AutoTurret");
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_FIREWALL] = pChild->GetValueFloat();

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
			float y = RandomFloat((float)j, (float)j+50);

			if (x < -m_hTerrain->GetMapSize()+10 || y < -m_hTerrain->GetMapSize()+10)
				continue;

			if (x > m_hTerrain->GetMapSize()-10 || y > m_hTerrain->GetMapSize()-10)
				continue;

			if (GetTerrain()->IsPointOverHole(Vector(x, y, 0)))
				continue;

			CResourceNode* pResource = GameServer()->Create<CResourceNode>("CResourceNode");
			pResource->SetGlobalOrigin(m_hTerrain->GetPointHeight(Vector(x, y, 0)));
			pResource->FindGround();
		}
	}
}

void CDigitanksGame::SetupProps()
{
/*	// I'm changing the game settings code over to cvars and since this code is not used I'm just commenting it out.
	// I did try to change it to what it should be though.
	CDigitanksLevel* pLevel = CDigitanksGame::GetLevel(CVar::GetCVarValue(_T("game_level")));

	if (!pLevel)
		return;

	for (size_t iProps = 0; iProps < pLevel->GetNumProps(); iProps++)
	{
		CLevelProp* pLevelProp = pLevel->GetProp(iProps);
		CStaticProp* pProp = GameServer()->Create<CStaticProp>("CStaticProp");
		pProp->SetGlobalOrigin(m_hTerrain->GetPointHeight(Vector(pLevelProp->m_vecPosition.x, 0, pLevelProp->m_vecPosition.y)));
		pProp->SetAngles(EAngle(0, pLevelProp->m_angOrientation.y, 0));
		pProp->SetColorSwap(m_hTerrain->GetPrimaryTerrainColor());
		pProp->SetModel(convertstring<char, tchar>(pLevelProp->m_sModel));
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
			pProp->SetGlobalOrigin(m_hTerrain->GetPointHeight(Vector(x, y, 0)));
			pProp->SetAngles(EAngle(0, RandomFloat(0, 360), 0));
			pProp->SetColorSwap(m_hTerrain->GetPrimaryTerrainColor());

			if (GetGameType() == GAMETYPE_ARTILLERY)
			{
				pProp->SetModel(_T("models/props/prop05.toy"));
			}
			else
			{
				switch (RandomInt(0, 3))
				{
				case 0:
					pProp->SetModel(_T("models/props/prop01.toy"));
					break;

				case 1:
					pProp->SetModel(_T("models/props/prop02.toy"));
					break;

				case 2:
					pProp->SetModel(_T("models/props/prop03.toy"));
					break;

				case 3:
					pProp->SetModel(_T("models/props/prop04.toy"));
					break;
				}
			}
		}
	}*/
}

void CDigitanksGame::ScatterNeutralUnits()
{
	AddPlayer(GameServer()->Create<CDigitanksPlayer>("CDigitanksPlayer"));

	CDigitanksPlayer* pPlayer = GetDigitanksPlayer(GetNumPlayers()-1);
	pPlayer->SetColor(Color(128, 128, 128));
	pPlayer->SetPlayerName(tstring("Network Guardians"));
	pPlayer->Bot_UseArtilleryAI();
	pPlayer->SetNotHumanPlayable();
	pPlayer->DontIncludeInScoreboard();

	for (int i = (int)-m_hTerrain->GetMapSize(); i < (int)m_hTerrain->GetMapSize(); i += 100)
	{
		for (int j = (int)-m_hTerrain->GetMapSize(); j < (int)m_hTerrain->GetMapSize(); j += 100)
		{
			if (rand()%4 > 0)
				continue;

			float x = RandomFloat((float)i, (float)i+100);
			float y = RandomFloat((float)j, (float)j+100);

			if (x < -m_hTerrain->GetMapSize()+10 || y < -m_hTerrain->GetMapSize()+10)
				continue;

			if (x > m_hTerrain->GetMapSize()-10 || y > m_hTerrain->GetMapSize()-10)
				continue;

			CBugTurret* pTurret = GameServer()->Create<CBugTurret>("CBugTurret");
			pTurret->SetGlobalOrigin(m_hTerrain->GetPointHeight(Vector(x, y, 0)));
			pPlayer->AddUnit(pTurret);
		}
	}
}

CVar game_players("game_players", "1");
CVar game_bots("game_bots", "3");
CVar game_tanks("game_tanks", "3");

void CDigitanksGame::SetupArtillery()
{
	TMsg("Setting up artillery game.\n");

	m_sObjective = "Destroy all enemy tanks";

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
		iTanks = stoi(CGameLobbyClient::L_GetInfoValue("tanks").c_str());

	if (iTanks > 4)
		iTanks = 4;
	if (iTanks < 1)
		iTanks = 1;
	game_tanks.SetValue(iTanks);

	tvector<size_t> aiAvailableColors;
	if (GameServer()->ShouldSetupFromLobby())
	{
		for (int i = 0; i < 8; i++)
			aiAvailableColors.push_back(i);

		for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
		{
			CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
			tstring sColor = pPlayer->GetInfoValue("color");
			if (sColor == "random" || sColor == "")
				continue;

			size_t iColor = stoi(sColor.c_str());
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
		AddPlayer(GameServer()->Create<CDigitanksPlayer>("CDigitanksPlayer"));

		CDigitanksPlayer* pTeam = GetDigitanksPlayer(GetNumPlayers()-1);

		if (GameServer()->ShouldSetupFromLobby())
		{
			CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
			if (pPlayer)
			{
				tstring sColor = pPlayer->GetInfoValue("color");
				if (sColor == "random" || sColor == "")
				{
					size_t iColor = RandomInt(0, aiAvailableColors.size()-1);
					pTeam->SetColor(g_aclrTeamColors[aiAvailableColors[iColor]]);
					pTeam->SetPlayerName(g_aszTeamNames[aiAvailableColors[iColor]]);
					aiAvailableColors.erase(aiAvailableColors.begin()+iColor);
				}
				else
				{
					size_t iColor = stoi(sColor.c_str());
					pTeam->SetColor(g_aclrTeamColors[iColor]);
					pTeam->SetPlayerName(g_aszTeamNames[iColor]);
				}
			}
			else
			{
				size_t iColor = RandomInt(0, aiAvailableColors.size()-1);
				pTeam->SetColor(g_aclrTeamColors[iColor]);
				pTeam->SetPlayerName(g_aszTeamNames[iColor]);
				aiAvailableColors.erase(aiAvailableColors.begin()+iColor);
			}

			if (pPlayer && pPlayer->GetInfoValue("bot") != "1")
				pTeam->SetPlayerName(pPlayer->GetInfoValue("name"));

			if (pPlayer->GetInfoValue("bot") == "1")
				pTeam->SetClient(NETWORK_BOT);
			else
			{
				pTeam->SetClient(pPlayer->iClient);
				pTeam->SetInstallID(LobbyNetwork()->GetClientInstallID(pPlayer->iClient));
			}
		}
		else
		{
			pTeam->SetColor(g_aclrTeamColors[i]);
			pTeam->SetPlayerName(g_aszTeamNames[i]);

			if (i < game_players.GetInt())
			{
				if (i == 0)
				{
					tstring sPlayerNickname = TPortal_GetPlayerNickname();
					if (sPlayerNickname.length())
						pTeam->SetPlayerName(sPlayerNickname);
				}

				pTeam->SetClient(NETWORK_LOCAL);
				pTeam->SetInstallID(CNetwork::GetInstallID());
			}
			else
				pTeam->SetClient(NETWORK_BOT);
		}
	}
}

void CDigitanksGame::SetupStrategy()
{
	TMsg("Setting up strategy game.\n");

	m_sObjective = "Destroy all enemy CPUs";

	ReadGameScript("strategy.txt");

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

	tvector<Vector> avecRandomStartingPositions;
	for (int i = 0; i < 4; i++)
		avecRandomStartingPositions.insert(avecRandomStartingPositions.begin()+RandomInt(0, i), avecStartingPositions[i]);

	tvector<size_t> aiAvailableColors;
	if (GameServer()->ShouldSetupFromLobby())
	{
		for (int i = 0; i < 8; i++)
			aiAvailableColors.push_back(i);

		for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
		{
			CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
			tstring sColor = pPlayer->GetInfoValue("color");
			if (sColor == "random" || sColor == "")
				continue;

			size_t iColor = stoi(sColor.c_str());
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
		AddPlayer(GameServer()->Create<CDigitanksPlayer>("CDigitanksPlayer"));

		CDigitanksPlayer* pTeam = GetDigitanksPlayer(GetNumPlayers()-1);

		if (GameServer()->ShouldSetupFromLobby())
		{
			CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
			if (pPlayer)
			{
				tstring sColor = pPlayer->GetInfoValue("color");
				if (sColor == "random" || sColor == "")
				{
					size_t iColor = RandomInt(0, aiAvailableColors.size()-1);
					pTeam->SetColor(g_aclrTeamColors[aiAvailableColors[iColor]]);
					pTeam->SetPlayerName(g_aszTeamNames[aiAvailableColors[iColor]]);
					aiAvailableColors.erase(aiAvailableColors.begin()+iColor);
				}
				else
				{
					size_t iColor = stoi(sColor.c_str());
					pTeam->SetColor(g_aclrTeamColors[iColor]);
					pTeam->SetPlayerName(g_aszTeamNames[iColor]);
				}
			}
			else
			{
				size_t iColor = RandomInt(0, aiAvailableColors.size()-1);
				pTeam->SetColor(g_aclrTeamColors[iColor]);
				pTeam->SetPlayerName(g_aszTeamNames[iColor]);
				aiAvailableColors.erase(aiAvailableColors.begin()+iColor);
			}

			if (pPlayer && pPlayer->GetInfoValue("bot") != "1")
				pTeam->SetPlayerName(pPlayer->GetInfoValue("name"));

			if (pPlayer->GetInfoValue("bot") == "1")
				pTeam->SetClient(NETWORK_BOT);
			else
			{
				pTeam->SetClient(pPlayer->iClient);
				pTeam->SetInstallID(LobbyNetwork()->GetClientInstallID(pPlayer->iClient));
			}
		}
		else
		{
			pTeam->SetColor(g_aclrTeamColors[i]);
			pTeam->SetPlayerName(g_aszTeamNames[i]);

			if (i == 0)
			{
				tstring sPlayerNickname = TPortal_GetPlayerNickname();
				if (sPlayerNickname.length())
					pTeam->SetPlayerName(sPlayerNickname);

				pTeam->SetClient(NETWORK_LOCAL);
				pTeam->SetInstallID(CNetwork::GetInstallID());
			}
			else
				pTeam->SetClient(NETWORK_BOT);
		}

		pTeam->SetLoseCondition(LOSE_NOCPU);

		GetTerrain()->ClearArea(avecRandomStartingPositions[i], 40);

		CMobileCPU* pMobileCPU = GameServer()->Create<CMobileCPU>("CMobileCPU");
		pTeam->AddUnit(pMobileCPU);
		pMobileCPU->SetGlobalOrigin(m_hTerrain->GetPointHeight(avecRandomStartingPositions[i]));
		pMobileCPU->SetGlobalAngles(VectorAngles(-avecRandomStartingPositions[i].Normalized()));

		for (size_t j = 0; j < GameServer()->GetMaxEntities(); j++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(j);
			if (!pEntity)
				continue;

			CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);
			if (!pDTEntity)
				continue;

			if (pMobileCPU->GetPlayerOwner() == pDTEntity->GetPlayerOwner())
				continue;

			// Remove nearby stuff so our spawn point can be clear
			if ((pDTEntity->GetGlobalOrigin() - pMobileCPU->GetGlobalOrigin()).Length2D() < 30)
				pEntity->Delete();

			CBugTurret* pBugTurret = dynamic_cast<CBugTurret*>(pEntity);
			if (pBugTurret && pTeam->GetVisibilityAtPoint(pBugTurret->GetGlobalOrigin()) > 0.5f)
				pBugTurret->Delete();
		}

		CDigitank* pTank;
		Vector vecTank;
		EAngle angTank;

		Vector vecForward = (Vector(0,0,0) - avecRandomStartingPositions[i]).Normalized();
		Vector vecRight = vecForward.Cross(Vector(0,1,0)).Normalized();

		pTank = GameServer()->Create<CScout>("CScout");
		pTeam->AddUnit(pTank);

		vecTank = avecRandomStartingPositions[i] + vecForward * 20 + vecRight * 20;
		angTank = VectorAngles(-vecTank.Normalized());

		pTank->SetGlobalOrigin(GetTerrain()->GetPointHeight(vecTank));
		pTank->SetGlobalAngles(angTank);
		pTank->GiveBonusPoints(1, false);
	}

	if (GameServer()->ShouldSetupFromLobby())
	{
		for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
		{
			if (CGameLobbyClient::L_GetPlayer(i)->GetInfoValue("bot") == "1")
				continue;

			// There's one neutral team at the front so skip it.
			GetPlayer(i+1)->SetClient(CGameLobbyClient::L_GetPlayer(i)->iClient);
			GetPlayer(i+1)->SetInstallID(LobbyNetwork()->GetClientInstallID(CGameLobbyClient::L_GetPlayer(i)->iClient));
		}
	}
	else
	{
		for (int i = 0; i < game_players.GetInt(); i++)
		{
			// There's one neutral team at the front so skip it.
			GetPlayer(i+1)->SetClient(NETWORK_LOCAL);
			GetPlayer(i+1)->SetInstallID(CNetwork::GetInstallID());
		}
	}

	CPowerup* pPowerup = GameServer()->Create<CPowerup>("CPowerup");
	pPowerup->SetGlobalOrigin(Vector(70, 70, m_hTerrain->GetHeight(70, 70)));
	pPowerup = GameServer()->Create<CPowerup>("CPowerup");
	pPowerup->SetGlobalOrigin(Vector(70, -70, m_hTerrain->GetHeight(70, -70)));
	pPowerup = GameServer()->Create<CPowerup>("CPowerup");
	pPowerup->SetGlobalOrigin(Vector(-70, 70, m_hTerrain->GetHeight(-70, 70)));
	pPowerup = GameServer()->Create<CPowerup>("CPowerup");
	pPowerup->SetGlobalOrigin(Vector(-70, -70, m_hTerrain->GetHeight(-70, -70)));

	m_iPowerups = 4;

	m_hUpdates = GameServer()->Create<CUpdateGrid>("CUpdateGrid");
	m_hUpdates->SetupStandardUpdates();
}

void CDigitanksGame::SetupMenuMarch()
{
	TMsg("Setting up menu march.\n");

	m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
	m_hTerrain->GenerateTerrain();

	CTeam* pTeam = GameServer()->Create<CTeam>("CTeam");
	pTeam->SetColor(Color(0, 0, 255));

	GameServer()->Create<COverheadCamera>("COverheadCamera");

	GetOverheadCamera()->SnapTarget(Vector(0,0,0));
	GetOverheadCamera()->SnapAngle(EAngle(55,40,0));
	GetOverheadCamera()->SnapDistance(60);

#if !defined(TINKER_OPTIMIZE_SOFTWARE)
	CMenuMarcher* pMarcher;

	if (GameServer()->GetWorkListener())
		GameServer()->GetWorkListener()->SetAction("Specifying marchers", 4*5*4);

	for (size_t i = 0; i < 4; i++)
	{
		float flY = RemapVal((float)i, 0, 4, -79, 79);

		for (size_t j = 0; j < 5; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				pMarcher = GameServer()->Create<CMenuMarcher>("CMenuMarcher");
				pTeam->AddEntity(pMarcher);

				pMarcher->SetGlobalOrigin(GetTerrain()->GetPointHeight(Vector(RemapVal((float)j, 0, 5, -15, 15), flY + RemapVal((float)k, 0, 4, -15, 15), 0)));
				pMarcher->SetGlobalAngles(EAngle(0,90,0));

				if (GameServer()->GetWorkListener())
					GameServer()->GetWorkListener()->WorkProgress(i*4*5 + j*4 + k);
			}
		}
	}
#endif

	m_iPowerups = 0;
}

void MissionReload(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
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

void MissionWin(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	if (!CVar::GetCVarBool("cheats"))
		return;

	DigitanksGame()->PlayerVictory(asTokens);
}

CCommand mission_win("mission_win", ::MissionWin);

void MissionLose(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	DigitanksGame()->PlayerLoss(asTokens);
}

CCommand mission_lose("mission_lose", ::MissionLose);

void CDigitanksGame::SetupCampaign(bool bReload)
{
	TMsg(sprintf(tstring("Setting up campaign %s.\n"), CVar::GetCVarValue("game_level").c_str()));

	SetCurrentLevel(CVar::GetCVarValue("game_level"));

	m_sObjective = m_pLevel->GetObjective();

	GameServer()->Create<CCampaignEntity>("CCampaignEntity");

	if (!bReload)
	{
		m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
		m_hTerrain->GenerateTerrain();
	}

	AddPlayer(GameServer()->Create<CDigitanksPlayer>("CDigitanksPlayer"));
	GetPlayer(0)->SetColor(Color(0, 0, 255));

	GetPlayer(0)->SetClient(NETWORK_LOCAL);
	GetPlayer(0)->SetInstallID(CNetwork::GetInstallID());

	tstring sPlayerNickname = TPortal_GetPlayerNickname();
	if (sPlayerNickname.length())
		GetPlayer(0)->SetPlayerName(sPlayerNickname);

	GetDigitanksPlayer(0)->SetLoseCondition(LOSE_NOTANKS);

	AddPlayer(GameServer()->Create<CDigitanksPlayer>("CDigitanksPlayer"));
	GetPlayer(1)->SetColor(Color(255, 0, 0));

	GetPlayer(1)->SetClient(NETWORK_BOT);

	GetDigitanksPlayer(1)->SetLoseCondition(LOSE_NONE);

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
			pEntity = GameServer()->Create<CResourceNode>("CResourceNode");
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

		pUnit->SetGlobalOrigin(m_hTerrain->GetPointHeight(Vector(pLevelUnit->m_vecPosition.x, pLevelUnit->m_vecPosition.y, 0)));
		pUnit->SetGlobalAngles(EAngle(0, pLevelUnit->m_angOrientation.y, 0));
		pUnit->SetObjective(pLevelUnit->m_bObjective);

		if (!pLevelUnit->m_bImprisoned)
		{
			if (pLevelUnit->m_sTeamName == "Player")
				ToDigitanksPlayer(GetPlayer(0))->AddUnit(pUnit);
			else if (pLevelUnit->m_sTeamName == "Hackers")
				ToDigitanksPlayer(GetPlayer(1))->AddUnit(pUnit);
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
		DigitanksWindow()->GetInstructor()->DisplayFirstLesson(m_pLevel->GetStartingLesson());
}

void CDigitanksGame::SetupEntities()
{
	if (!GameNetwork()->ShouldRunClientFunction())
		return;

	GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "SetupEntities");

	CNetworkParameters p;
	SetupEntities(CONNECTION_GAME, &p);
}

void CDigitanksGame::SetupEntities(int iConnection, CNetworkParameters* p)
{
	CSoundLibrary::StopSound();
	CParticleSystemLibrary::ClearInstances();

	if (!GameNetwork()->IsHost())
		return;

	while (GetNumPlayers())
	{
		CPlayer* pPlayer = GetPlayer(0);
		RemovePlayer(pPlayer);
		pPlayer->Delete();
	}

	// Just in case!
	TAssert(GetNumPlayers() == 0);
	m_ahPlayers.clear();

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

tvector<CLevel*> CDigitanksGame::GetLevels(gametype_t eGameType)
{
	tvector<CLevel*> ahReturn;
	for (size_t i = 0; i < GameServer()->GetNumLevels(); i++)
	{
		CDigitanksLevel* pLevel = GameServer()->GetLevel(i).DowncastStatic<CDigitanksLevel>();
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
	tvector<CLevel*> ahLevels = GetLevels(eGameType);
	if (i >= ahLevels.size())
		return NULL;

	return dynamic_cast<CDigitanksLevel*>(ahLevels[i]);
}

CDigitanksLevel* CDigitanksGame::GetLevel(tstring sFile)
{
	return GameServer()->GetLevel(sFile).DowncastStatic<CDigitanksLevel>();
}

void CDigitanksGame::StartGame()
{
	if (GetGameType() == GAMETYPE_STANDARD)
		// Start with the player's team so the neutral team doesn't get a chance to attack.
		m_iCurrentPlayer = 1;
	else
		m_iCurrentPlayer = 0;

	m_iWaitingForProjectiles = 0;
	m_bWaitingForProjectiles = true;

	if (HasRounds())
		StartNewRound();
	else
		GetCurrentPlayer()->StartTurn();

	EnterGame(CONNECTION_GAME, NULL);
}

void CDigitanksGame::EnterGame()
{
	BaseClass::EnterGame();

	GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "EnterGame");
	EnterGame(CONNECTION_GAME, NULL);
}

void CDigitanksGame::EnterGame(int iConnection, CNetworkParameters* p)
{
	if (!GameNetwork()->ShouldRunClientFunction())
		return;

	if (GameNetwork()->IsHost())
		GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "EnterGame");

	for (size_t i = 0; i < GetNumPlayers(); i++)
		GetDigitanksPlayer(i)->CountScore();

	m_bWaitingForMoving = false;
	m_bWaitingForProjectiles = false;

	if (m_pListener)
	{
		m_pListener->GameStart();

		m_pListener->SetHUDActive(GetCurrentPlayer() == GetCurrentLocalDigitanksPlayer());
		m_pListener->NewCurrentTeam();

		m_pListener->NewCurrentSelection();
	}

	if (m_eGameType == GAMETYPE_STANDARD && !GameNetwork()->IsConnected())
		DigitanksWindow()->GetStoryPanel()->SetVisible(true);

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		pEntity->ClientEnterGame();
	}

	if (m_eGameType == GAMETYPE_STANDARD || m_eGameType == GAMETYPE_ARTILLERY || m_eGameType == GAMETYPE_CAMPAIGN)
		DigitanksGame()->GetOverheadCamera()->EnterGame();
}

void CDigitanksGame::StartNewRound()
{
	m_iCurrentPlayer = 0;

	m_iWaitingForProjectiles = 0;
	m_bWaitingForProjectiles = true;

	m_iTurn = 0;

	m_flPartyModeStart = 0;
	m_bPartyMode = false;

	for (size_t i = 0; i < GetNumPlayers(); i++)
		GetDigitanksPlayer(i)->StartNewRound();

	if (m_eGameType == GAMETYPE_ARTILLERY)
		SetupArtilleryRound();

	GetCurrentPlayer()->StartTurn();
}

CVar game_terrainheight("game_terrainheight", "60");

void CDigitanksGame::SetupArtilleryRound()
{
	GameServer()->SetLoading(true);

	DigitanksWindow()->RenderLoading();

	tvector<tstring> asSpare;
	asSpare.push_back("CDigitanksGame");
	asSpare.push_back("CDigitanksPlayer");

	GameServer()->DestroyAllEntities(asSpare);

	float flTerrainHeight = game_terrainheight.GetFloat();
	if (GameServer()->ShouldSetupFromLobby())
		flTerrainHeight = (float)stof(CGameLobbyClient::L_GetInfoValue("terrain").c_str());

	m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
	m_hTerrain->GenerateTerrain(flTerrainHeight);

	float flMapBuffer = GetTerrain()->GetMapSize()*0.1f;
	float flMapSize = GetTerrain()->GetMapSize() - flMapBuffer*2;

	size_t iPlayers = (game_players.GetInt() + game_bots.GetInt());
	if (GameServer()->ShouldSetupFromLobby())
		iPlayers = CGameLobbyClient::L_GetNumPlayers();

	size_t iTanksPerPlayer = game_tanks.GetInt();
	if (GameServer()->ShouldSetupFromLobby())
		iTanksPerPlayer = stoi(CGameLobbyClient::L_GetInfoValue("tanks").c_str());

	size_t iTotalTanks = iTanksPerPlayer * iPlayers;

	size_t iSections = (int)sqrt((float)iTotalTanks);
	float flSectionSize = flMapSize*2/iSections;

	tvector<size_t> aiRandomTeamPositions;
	// 8 random starting positions.
	for (size_t i = 0; i < iPlayers; i++)
	{
		for (size_t j = 0; j < iTanksPerPlayer; j++)
			aiRandomTeamPositions.insert(aiRandomTeamPositions.begin()+RandomInt(0, aiRandomTeamPositions.size()-1), i);
	}

	if (GameServer()->GetWorkListener())
		GameServer()->GetWorkListener()->SetAction("Randomizing tank locations", iTotalTanks);

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

				CDigitanksPlayer* pTeam = ToDigitanksPlayer(GetPlayer(aiRandomTeamPositions[iPosition]));

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
				pTeam->AddUnit(pTank);

				vecTank.z = pTank->FindHoverHeight(vecTank);

				pTank->SetGlobalOrigin(vecTank);
				pTank->SetGlobalAngles(angTank);
				pTank->GiveBonusPoints(1, false);

				iPosition = (iPosition+1)%aiRandomTeamPositions.size();
				iTanksPlaced++;
			}
		}

		if (GameServer()->GetWorkListener())
			GameServer()->GetWorkListener()->WorkProgress(iTanksPlaced);
	}

	CPowerup* pPowerup;
	
	Vector vecPowerup = Vector(70, 70, m_hTerrain->GetHeight(70, 70));
	if (!GetTerrain()->IsPointOverHole(vecPowerup))
	{
		pPowerup = GameServer()->Create<CPowerup>("CPowerup");
		pPowerup->SetGlobalOrigin(vecPowerup);
		m_iPowerups++;
	}

	vecPowerup = Vector(70, -70, m_hTerrain->GetHeight(70, -70));
	if (!GetTerrain()->IsPointOverHole(vecPowerup))
	{
		pPowerup = GameServer()->Create<CPowerup>("CPowerup");
		pPowerup->SetGlobalOrigin(vecPowerup);
		m_iPowerups++;
	}

	vecPowerup = Vector(-70, 70, m_hTerrain->GetHeight(-70, 70));
	if (!GetTerrain()->IsPointOverHole(vecPowerup))
	{
		pPowerup = GameServer()->Create<CPowerup>("CPowerup");
		pPowerup->SetGlobalOrigin(vecPowerup);
		m_iPowerups++;
	}

	vecPowerup = Vector(-70, -70, m_hTerrain->GetHeight(-70, -70));
	if (!GetTerrain()->IsPointOverHole(vecPowerup))
	{
		pPowerup = GameServer()->Create<CPowerup>("CPowerup");
		pPowerup->SetGlobalOrigin(vecPowerup);
		m_iPowerups++;
	}

	SetupProps();
}

bool CDigitanksGame::HasRounds()
{
	return m_eGameType == GAMETYPE_ARTILLERY;
}

void CDigitanksGame::Autosave(const tvector<tstring>& sArgs)
{
	Autosave();
}

void CDigitanksGame::Autosave()
{
	GameServer()->SaveToFile(GetAppDataDirectory(DigitanksWindow()->AppDirectory(), "autosave.sav").c_str());

	DigitanksWindow()->GetChatBox()->PrintChat("Autosave...\n");
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
		DigitanksWindow()->GetInstructor()->DisplayFirstLesson("artillery-select");
		m_flShowArtilleryTutorial = 0;
	}

	if (DigitanksWindow()->ShouldUseContextualCommands() && GetCurrentLocalDigitanksPlayer())
	{
		CDigitank* pSelection = GetCurrentLocalDigitanksPlayer()->GetPrimarySelectionTank();

		if (DigitanksWindow()->GetHUD()->GetWeaponPanel() && DigitanksWindow()->GetHUD()->GetWeaponPanel()->IsVisible())
			SetControlMode(MODE_NONE);
		else if (pSelection && pSelection->GetDigitanksPlayer() == GetCurrentLocalDigitanksPlayer())
		{
			CBaseEntity* pHit = NULL;
			Vector vecEntityPoint;

			bool bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecEntityPoint, &pHit);

			if (pHit)
			{
				CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pHit);
				if (pDTEntity)
				{
					if (pDTEntity->GetDigitanksPlayer() != pSelection->GetDigitanksPlayer())
					{
						SetControlMode(MODE_AIM);
						SetAimTypeByWeapon(pSelection->GetCurrentWeapon());
						if (pSelection->GetCurrentWeapon() == WEAPON_CHARGERAM)
							pSelection->SetPreviewCharge(pHit);
						else
							pSelection->SetPreviewAim(vecEntityPoint);
					}
					else
						SetControlMode(MODE_NONE);
				}
				else
				{
					SetControlMode(MODE_MOVE);
					pSelection->SetPreviewMove(vecEntityPoint);
				}
			}
			else
				SetControlMode(MODE_NONE);
		}
	}

	if (GetGameType() == GAMETYPE_MENU)
		return;

	if (m_bTurnActive && GetCurrentPlayer() && !GetCurrentPlayer()->IsHumanControlled() && GameNetwork()->IsHost())
		GetCurrentPlayer()->Bot_ExecuteTurn();

	if (m_bWaitingForMoving)
	{
		bool bMoving = false;
		for (size_t i = 0; i < GetCurrentPlayer()->GetNumTanks(); i++)
		{
			if (GetCurrentPlayer()->GetTank(i)->IsMoving())
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
			for (size_t i = 0; i < GetCurrentPlayer()->GetNumTanks(); i++)
			{
				const CDigitank* pTank = GetCurrentPlayer()->GetTank(i);
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
	if (GetCurrentPlayer() && !GetCurrentPlayer()->IsHumanControlled())
	{
		const CDigitanksPlayer* pNextPlayer = GetDigitanksPlayer((m_iCurrentPlayer+(size_t)1)%GetNumPlayers());
		if (pNextPlayer && pNextPlayer->IsHumanControlled())
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
				if (!DigitanksGame()->GetCurrentLocalDigitanksPlayer()->HasLost())
					DigitanksWindow()->NextCampaignLevel();
				return;
			}
		}

		EAngle angCamera = GetOverheadCamera()->GetAngles();
		angCamera.y += (float)GameServer()->GetFrameTime()*2;
		GetOverheadCamera()->SnapAngle(angCamera);

		if (GameNetwork()->IsHost() && GameServer()->GetGameTime() > m_flLastFireworks + RandomFloat(0.5f, 3.0f))
		{
			tvector<CEntityHandle<CDigitanksEntity> > ahEntities;
			for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
			{
				if (!CBaseEntity::GetEntity(i))
					continue;

				CDigitanksEntity* pEntity = dynamic_cast<CDigitanksEntity*>(CBaseEntity::GetEntity(i));
				if (!pEntity)
					continue;
				
				if (dynamic_cast<CPowerup*>(pEntity))
					continue;

				if (dynamic_cast<CResourceNode*>(pEntity))
					continue;

				if (dynamic_cast<CStaticProp*>(pEntity))
					continue;

				ahEntities.push_back(pEntity);
			}

			if (ahEntities.size())
			{
				CDigitanksEntity* pEntity = ahEntities[RandomInt(0, ahEntities.size()-1)];

				CFireworks* pFireworks = GameServer()->Create<CFireworks>("CFireworks");
				pFireworks->SetGlobalOrigin(pEntity->GetGlobalOrigin());
				pFireworks->SetOwner(NULL);
				pFireworks->SetGlobalVelocity(Vector(RandomFloat(-8, 8), RandomFloat(-8, 8), 45));
				pFireworks->SetGlobalGravity(Vector(0, 0, DigitanksGame()->GetGravity()));
			}

			m_flLastFireworks = GameServer()->GetGameTime();
		}
	}

	if (GameNetwork()->IsHost())
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
				pShell->SetGlobalOrigin(vecLandingSpot + Vector(30, 100, 30));
				pShell->SetGlobalVelocity(Vector(-30, -30, -100));
				pShell->SetGlobalGravity(Vector());
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

	if (pCurrentTank->GetPlayerOwner() != GetCurrentLocalDigitanksPlayer())
		return;

	Vector vecPreview = pCurrentTank->GetPreviewMove();
	Vector vecOrigin = pCurrentTank->GetGlobalOrigin();

	Vector vecMove = vecPreview - vecOrigin;

	size_t iFormation = 0;

	CDigitanksPlayer* pTeam = GetCurrentPlayer();
	for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
	{
		CDigitank* pTank = pTeam->GetTank(i);

		if (!pTank->MovesWith(pCurrentTank))
			continue;

		Vector vecNewPosition = GetFormationPosition(vecPreview, vecMove, pTeam->GetNumTanks(), iFormation++);

		vecNewPosition.z = pTank->FindHoverHeight(vecNewPosition);

		pTank->SetPreviewMove(vecNewPosition);

		if (!pTank->IsPreviewMoveValid())
			pTank->SetGoalMovePosition(vecNewPosition);
		else
			pTank->Move();

		GetCurrentLocalDigitanksPlayer()->HandledActionItem(pTank);

		if (pTank->GetUnitType() == UNIT_MOBILECPU)
			DigitanksWindow()->GetInstructor()->FinishedLesson("strategy-command", true);
	}

	SetControlMode(MODE_NONE);

	DigitanksWindow()->GetInstructor()->FinishedLesson("mission-1-move");
	DigitanksWindow()->GetInstructor()->FinishedLesson("strategy-command");
}

void CDigitanksGame::TurnTanks(Vector vecLookAt)
{
	if (!GetPrimarySelection())
		return;

	CDigitank* pCurrentTank = GetPrimarySelectionTank();
	if (!pCurrentTank)
		return;

	if (pCurrentTank->GetPlayerOwner() != GetCurrentLocalDigitanksPlayer())
		return;

	bool bNoTurn = (vecLookAt - pCurrentTank->GetGlobalOrigin()).LengthSqr() < 4*4;

	CDigitanksPlayer* pTeam = GetCurrentPlayer();
	for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
	{
		CDigitank* pTank = pTeam->GetTank(i);

		if (!pTank->TurnsWith(pCurrentTank))
			continue;

		if (bNoTurn)
			pTank->SetPreviewTurn(pTank->GetAngles().y);
		else
		{
			Vector vecDirection = (vecLookAt - pTank->GetGlobalOrigin()).Normalized();
			float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

			float flTankTurn = AngleDifference(flYaw, pTank->GetAngles().y);
			if (fabs(flTankTurn)/pTank->TurnPerPower() > pTank->GetRemainingMovementEnergy())
				flTankTurn = (flTankTurn / fabs(flTankTurn)) * pTank->GetRemainingTurningDistance() * 0.95f;

			pTank->SetPreviewTurn(pTank->GetAngles().y + flTankTurn);
		}

		pTank->Turn();

		GetCurrentLocalDigitanksPlayer()->HandledActionItem(pTank);
	}

	SetControlMode(MODE_NONE);

	DigitanksWindow()->GetInstructor()->FinishedLesson("artillery-command", true);
}

void CDigitanksGame::FireTanks()
{
	if (!GetPrimarySelection())
		return;

	CDigitank* pCurrentTank = GetPrimarySelectionTank();
	if (!pCurrentTank)
		return;

	if (pCurrentTank->GetPlayerOwner() != GetCurrentLocalDigitanksPlayer())
		return;

	Vector vecPreviewAim = pCurrentTank->GetPreviewAim();

	CDigitanksPlayer* pTeam = GetCurrentPlayer();
	for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
	{
		CDigitank* pTank = pTeam->GetTank(i);

		if (CDigitanksWeapon::IsWeaponPrimarySelectionOnly(pTank->GetCurrentWeapon()) && GetPrimarySelectionTank() != pTank)
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
			Vector vecDirection = vecTankAim - pTank->GetGlobalOrigin();
			vecDirection.z = 0;
			vecTankAim = DigitanksGame()->GetTerrain()->GetPointHeight(pTank->GetGlobalOrigin() + vecDirection.Normalized() * vecDirection.Length2D() * 0.99f);
		}

		pTank->SetPreviewAim(vecTankAim);
		pTank->Fire();

		GetCurrentLocalDigitanksPlayer()->HandledActionItem(pTank);
	}

	DigitanksWindow()->GetInstructor()->FinishedLesson("artillery-command", true);
	SetControlMode(MODE_NONE);
}

void CDigitanksGame::CancelAutoMoves(const tvector<tstring>& sArgs)
{
	CDigitanksPlayer* pTeam = GetCurrentPlayer();
	for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
	{
		CDigitank* pTank = pTeam->GetTank(i);
		pTank->CancelGoalMovePosition();
	}
}

void CDigitanksGame::EndTurn()
{
	GameNetwork()->CallFunction(NETWORK_TOEVERYONE, "EndTurn");

	EndTurn(CONNECTION_GAME, NULL);

	DigitanksWindow()->GetInstructor()->FinishedLesson("artillery-endturn");
//	DigitanksWindow()->GetInstructor()->FinishedLesson(CInstructor::TUTORIAL_POWER);
}

void CDigitanksGame::EndTurn(int iConnection, CNetworkParameters* p)
{
	if (!GameNetwork()->ShouldRunClientFunction())
		return;

	if (!GetCurrentPlayer()->IsHumanControlled())
		DigitanksGame()->GetOverheadCamera()->ShowEnemyMoves();

	if (GetCurrentPlayer()->IsHumanControlled())
		m_flLastHumanMove = GameServer()->GetGameTime();

	GetCurrentPlayer()->EndTurn();

	m_bTurnActive = false;
	m_bWaitingForProjectiles = true;
	m_bWaitingForMoving = true;

	if (m_pListener)
		m_pListener->SetHUDActive(false);

	CheckWinConditions();
}

void CDigitanksGame::StartTurn()
{
	if (!GameNetwork()->IsHost())
		return;

	int iPowerupChance;
	if (GetGameType() == GAMETYPE_STANDARD)
		iPowerupChance = RandomInt(0, 6);
	else
		iPowerupChance = RandomInt(0, 3);

	if (GetGameType() != GAMETYPE_MENU && GetGameType() != GAMETYPE_CAMPAIGN && m_iPowerups < 10 && iPowerupChance == 0)
	{
		float flX = RandomFloat(-GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize());
		float flY = RandomFloat(-GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize());
		Vector vecPowerup = Vector(flX, flY, m_hTerrain->GetHeight(flX, flY));

		if (!GetTerrain()->IsPointOverHole(vecPowerup))
		{
			CPowerup* pPowerup = GameServer()->Create<CPowerup>("CPowerup");
			pPowerup->SetGlobalOrigin(vecPowerup);

			m_iPowerups++;
		}
	}

	if (GetGameType() == GAMETYPE_STANDARD && GetTurn() > 3 && GetTurn() < 50)
	{
		if (RandomInt(0, GetNumPlayers()-1) == 0)
		{
			Vector vecPoint;

			bool bIsVisible = false;

			size_t iTries = 0;
			while (iTries++ < 5)
			{
				float flX = RandomFloat(-GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize()) * 0.95f;
				float flY = RandomFloat(-GetTerrain()->GetMapSize(), GetTerrain()->GetMapSize()) * 0.95f;

				vecPoint = GetTerrain()->GetPointHeight(Vector(flX, flY, 0));

				bIsVisible = false;

				// Skip the first team, that's the barbarians.
				for (size_t i = 1; i < GetNumPlayers(); i++)
				{
					const CDigitanksPlayer* pTeam = GetDigitanksPlayer(i);
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
						pGridBug->SetGlobalOrigin(vecGridBug);
						GetDigitanksPlayer(0)->AddUnit(pGridBug);
					}
				}
			}
		}
	}

	GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "StartTurn");

	StartTurn(CONNECTION_GAME, NULL);

	if (GetCurrentPlayer()->HasLost())
		EndTurn();
}

void CDigitanksGame::StartTurn(int iConnection, CNetworkParameters* p)
{
	if (!GameNetwork()->ShouldRunClientFunction())
		return;

	DigitanksGame()->GetOverheadCamera()->ClearFollowTarget();

	if (m_iCurrentPlayer == (size_t)0)
		m_iTurn++;

	if (GetGameType() != GAMETYPE_CAMPAIGN && m_iCurrentPlayer == (size_t)0 && m_iTurn > 10 && m_iTurn%5 == 0)
		Autosave(tvector<tstring>());

	if (++m_iCurrentPlayer >= GetNumPlayers())
		m_iCurrentPlayer = 0;

	m_iWaitingForProjectiles = 0;

	m_bTurnActive = true;

	GetCurrentPlayer()->StartTurn();

	if (m_pListener)
	{
		m_pListener->SetHUDActive(GetCurrentPlayer() == GetCurrentLocalDigitanksPlayer());
		m_pListener->NewCurrentTeam();
	}

	if (GetPrimarySelection())
		GetPrimarySelection()->OnCurrentSelection();

	GetTerrain()->CalculateVisibility();
}

void CDigitanksGame::OnClientEnterGame(int iClient)
{
	size_t iInstallID = GameNetwork()->GetClientInstallID(iClient);

	for (size_t i = 0; i < GetNumPlayers(); i++)
	{
		if (GetPlayer(i)->GetInstallID() == iInstallID)
		{
			GetPlayer(i)->SetClient(iClient);
			GetPlayer(i)->SetPlayerName(GameNetwork()->GetClientNickname(iClient));
			return;
		}
	}

	// Couldn't find any spot for the guy? Take over a bot.
	for (size_t i = 0; i < GetNumPlayers(); i++)
	{
		if (!GetPlayer(i)->IsHumanControlled() && GetPlayer(i)->IsHumanPlayable())
		{
			GetPlayer(i)->SetInstallID(iInstallID);
			GetPlayer(i)->SetClient(iClient);
			GetPlayer(i)->SetPlayerName(GameNetwork()->GetClientNickname(iClient));
			return;
		}
	}

	// No spots? Boot him.
	GameNetwork()->DisconnectClient(iClient);
}

bool CDigitanksGame::Explode(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flRadius, float flDamage, CBaseEntity* pIgnore, const CDigitanksPlayer* pTeamIgnore)
{
	CDigitanksWeapon* pWeapon = dynamic_cast<CDigitanksWeapon*>(pInflictor);

	Vector vecExplosionOrigin;
	if (pInflictor)
		vecExplosionOrigin = pInflictor->GetGlobalOrigin();
	else
		vecExplosionOrigin = pAttacker->GetGlobalOrigin();

	tvector<CBaseEntity*> apHit;
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CDigitank* pDigitank = dynamic_cast<CDigitank*>(pEntity);

		float flDistanceSqr = (pInflictor->GetGlobalOrigin() - pEntity->GetGlobalOrigin()).LengthSqr();
		float flTotalRadius = flRadius + pEntity->GetBoundingRadius();
		float flPushRadius = pWeapon?pWeapon->PushRadius():20;
		float flTotalRadius2 = flRadius + pEntity->GetBoundingRadius() + flPushRadius;

		if (pDigitank && flDistanceSqr < flTotalRadius2*flTotalRadius2 && !pDigitank->IsFortified() && !pDigitank->IsFortifying())
		{
			float flRockIntensity = pWeapon?pWeapon->RockIntensity():0.5f;
			Vector vecExplosion = (pDigitank->GetGlobalOrigin() - vecExplosionOrigin).Normalized();
			pDigitank->RockTheBoat(RemapValClamped(flDistanceSqr, flTotalRadius*flTotalRadius, flTotalRadius2*flTotalRadius2, flRockIntensity, flRockIntensity/5), vecExplosion);

			if (flRadius < 1 || flDistanceSqr > flTotalRadius*flTotalRadius)
			{
				float flPushDistance = pWeapon?pWeapon->PushDistance():flRadius/2;

				Vector vecPushDirection = vecExplosion;
				if (vecPushDirection.z < 0)
				{
					vecPushDirection.z = 0;
					vecPushDirection.Normalize();
				}

				// If we have a direct hit (the ignored is a direct hit, see CProjectile::Touching) exaggerate it.
				if (pEntity == pIgnore)
					flPushDistance *= 1.5f;

				pDigitank->Move(pDigitank->GetGlobalOrigin() + vecPushDirection * RemapValClamped(flDistanceSqr, flTotalRadius*flTotalRadius, flTotalRadius2*flTotalRadius2, flPushDistance, flPushDistance/2), 2);
			}

			pDigitank->SetGoalTurretYaw(atan2(-vecExplosion.z, -vecExplosion.x) * 180/M_PI - pDigitank->GetRenderAngles().y);
		}

		if (pEntity == pIgnore)
			continue;

		// Fire too close to yourself and the explosion can rock you.
		if (pEntity != pAttacker)
		{
			// We can still push teammates around (above code) but we can't damage them.
			if (pDigitank && pDigitank->GetPlayerOwner() == pTeamIgnore)
				continue;
		}

		if (flDistanceSqr < flTotalRadius*flTotalRadius)
			apHit.push_back(pEntity);
		else
		{
			if (pDigitank && dynamic_cast<CProjectile*>(pInflictor))
			{
				if (pDigitank->IsScout() && (pEntity->GetGlobalOrigin() - vecExplosionOrigin).Length2DSqr() < flTotalRadius*flTotalRadius && pEntity->GetGlobalOrigin().y > vecExplosionOrigin.y)
					OnMiss(pEntity, pAttacker, pInflictor);
			}
		}
	}

	bool bHit = false;

	for (size_t i = 0; i < apHit.size(); i++)
	{
		float flDistance = (pInflictor->GetGlobalOrigin() - apHit[i]->GetGlobalOrigin()).Length();

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

SERVER_GAME_COMMAND(HitIndicator)
{
	if (pCmd->GetNumArguments() == 0)
	{
		TMsg("HitIndicator with 0 arguments.\n");
		return;
	}

	if (pCmd->Arg(0) == "sdmg")
	{
		if (pCmd->GetNumArguments() < 7)
		{
			TMsg("HitIndicator sdmg with not enough arguments.\n");
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

	if (pCmd->Arg(0) == "dmg")
	{
		if (pCmd->GetNumArguments() < 7)
		{
			TMsg("HitIndicator dmg with not enough arguments.\n");
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

	if (pCmd->Arg(0) == "disable")
	{
		if (pCmd->GetNumArguments() < 4)
		{
			TMsg("HitIndicator disable with not enough arguments.\n");
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

	if (pCmd->Arg(0) == "miss")
	{
		if (pCmd->GetNumArguments() < 4)
		{
			TMsg("HitIndicator miss with not enough arguments.\n");
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

	if (pCmd->Arg(0) == "crit")
	{
		if (pCmd->GetNumArguments() < 4)
		{
			TMsg("HitIndicator crit with not enough arguments.\n");
			return;
		}

		if (DigitanksGame()->GetListener())
		{
			DigitanksGame()->GetListener()->OnCritical(
					CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(1)),
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
	if (GameNetwork()->IsHost())
		HitIndicator.RunCommand(sprintf(tstring("sdmg %d %d %d %f %d %d"), SAFE_HANDLE(pVictim), SAFE_HANDLE(pAttacker), SAFE_HANDLE(pInflictor), flDamage, bDirectHit, bShieldOnly));
}

void CDigitanksGame::OnTakeDamage(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled)
{
	if (GameNetwork()->IsHost())
		HitIndicator.RunCommand(sprintf(tstring("dmg %d %d %d %f %d %d"), SAFE_HANDLE(pVictim), SAFE_HANDLE(pAttacker), SAFE_HANDLE(pInflictor), flDamage, bDirectHit, bKilled));
}

void CDigitanksGame::OnDisabled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	if (GameNetwork()->IsHost())
	{
		HitIndicator.RunCommand(sprintf(tstring("disable %d %d %d"), SAFE_HANDLE(pVictim), SAFE_HANDLE(pAttacker), SAFE_HANDLE(pInflictor)));

		CDigitank* pTank = dynamic_cast<CDigitank*>(pVictim);
		if (pTank)
			pTank->Speak(TANKSPEECH_DISABLED);
	}
}

void CDigitanksGame::OnMiss(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	if (GameNetwork()->IsHost())
	{
		HitIndicator.RunCommand(sprintf(tstring("miss %d %d %d"), SAFE_HANDLE(pVictim), SAFE_HANDLE(pAttacker), SAFE_HANDLE(pInflictor)));

		CDigitank* pTank = dynamic_cast<CDigitank*>(pVictim);
		if (pTank)
			pTank->Speak(TANKSPEECH_TAUNT);
	}
}

void CDigitanksGame::OnCritical(CDigitank* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	if (GameNetwork()->IsHost())
		HitIndicator.RunCommand(sprintf(tstring("crit %d %d %d"), SAFE_HANDLE(pVictim), SAFE_HANDLE(pAttacker), SAFE_HANDLE(pInflictor)));
}

void CDigitanksGame::OnKilled(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < GetNumPlayers(); i++)
		GetPlayer(i)->OnKilled(pEntity);

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

	for (size_t i = 0; i < GetNumPlayers(); i++)
	{
		if (GetDigitanksPlayer(i)->HasLost())
			continue;

		// The barbarian team doesn't count towards teams left.
		if (GetGameType() == GAMETYPE_STANDARD && i == 0)
			continue;

		switch (GetDigitanksPlayer(i)->GetLoseCondition())
		{
		case LOSE_NOCPU:
		{
			bool bHasCPU = false;
			for (size_t j = 0; j < GetDigitanksPlayer(i)->GetNumUnits(); j++)
			{
				const CBaseEntity* pEntity = GetDigitanksPlayer(i)->GetUnit(j);
				if (dynamic_cast<const CCPU*>(pEntity) || dynamic_cast<const CMobileCPU*>(pEntity))
				{
					bHasCPU = true;
					iTeamsLeft++;
					break;
				}
			}

			if (!bHasCPU)
			{
				GetDigitanksPlayer(i)->YouLoseSirGoodDay();
				bSomeoneLost = true;
			}

			break;
		}

		case LOSE_NOTANKS:
		{
			if (GetDigitanksPlayer(i)->GetNumTanksAlive() == 0)
			{
				GetDigitanksPlayer(i)->YouLoseSirGoodDay();
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

SERVER_GAME_COMMAND(GameOver)
{
	if (DigitanksGame()->GetListener() && DigitanksGame()->GetCurrentLocalDigitanksPlayer() && !DigitanksGame()->GetCurrentLocalDigitanksPlayer()->HasLost())
		DigitanksGame()->GetListener()->GameOver(!DigitanksGame()->GetCurrentLocalDigitanksPlayer()->HasLost());

	DigitanksGame()->GetOverheadCamera()->SetDistance(250);
	DigitanksGame()->GetOverheadCamera()->SetTarget(Vector(0,0,0));
}

void CDigitanksGame::GameOver()
{
	if (GameServer()->IsLoading())
		return;

	m_bPartyMode = true;
	m_flPartyModeStart = GameServer()->GetGameTime();

	::GameOver.RunCommand("");
}

void CDigitanksGame::PlayerVictory(const tvector<tstring>& sArgs)
{
	for (size_t i = 0; i < GetNumPlayers(); i++)
	{
		if (!GetDigitanksPlayer(i))
			continue;

		if (!GetDigitanksPlayer(i)->IsHumanControlled())
			GetDigitanksPlayer(i)->YouLoseSirGoodDay();
	}

	GameOver();
}

void CDigitanksGame::PlayerLoss(const tvector<tstring>& sArgs)
{
	for (size_t i = 0; i < GetNumPlayers(); i++)
	{
		if (!GetDigitanksPlayer(i))
			continue;

		if (GetDigitanksPlayer(i)->IsHumanControlled())
			GetDigitanksPlayer(i)->YouLoseSirGoodDay();
	}

	GameOver();
}

void CDigitanksGame::TankSelectionMedal(const tvector<tstring>& sArgs)
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

void CDigitanksGame::TankSpeak(class CBaseEntity* pTank, const tstring& sSpeech)
{
	if (m_pListener)
		m_pListener->TankSpeak(pTank, sSpeech);
}

CDigitanksPlayer* CDigitanksGame::GetDigitanksPlayer(size_t i) const
{
	return static_cast<CDigitanksPlayer*>(BaseClass::GetPlayer(i));
}

CDigitanksPlayer* CDigitanksGame::GetCurrentPlayer() const
{
	if (m_iCurrentPlayer >= GetNumPlayers())
		return NULL;

	return static_cast<CDigitanksPlayer*>(GetPlayer(m_iCurrentPlayer.Get()));
}

CSelectable* CDigitanksGame::GetPrimarySelection()
{
	if (!GetCurrentLocalDigitanksPlayer())
		return NULL;

	return GetCurrentLocalDigitanksPlayer()->GetPrimarySelection();
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
	vecFacing.z = 0;
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

	if (IsTeamControlledByMe(GetCurrentPlayer()))
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

	if (m_eControlMode == eMode)
		return;

	GetPrimarySelection()->OnControlModeChange(m_eControlMode, eMode);

	m_eControlMode = eMode;

	if (eMode == MODE_AIM)
		DigitanksWindow()->GetInstructor()->FinishedLesson("mission-1-turret-spotted");

	DigitanksWindow()->GetHUD()->SetupMenu();
}

aimtype_t CDigitanksGame::GetAimType()
{
	if (GameServer()->IsLoading())
		return AIM_NONE;

	if (IsTeamControlledByMe(GetCurrentPlayer()))
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

void CDigitanksGame::TerrainData(int iConnection, class CNetworkParameters* p)
{
	if (!GetTerrain())
		return;

	GetTerrain()->TerrainData(p);
}

CDigitanksRenderer*	CDigitanksGame::GetDigitanksRenderer()
{
	return dynamic_cast<CDigitanksRenderer*>(GameServer()->GetRenderer());
}

COverheadCamera* CDigitanksGame::GetOverheadCamera()
{
	return m_hOverheadCamera;
}

void CDigitanksGame::SetOverheadCamera(COverheadCamera* pCamera)
{
	m_hOverheadCamera = pCamera;
}

float CDigitanksGame::GetGravity()
{
	return -20;
}

CLIENT_GAME_COMMAND(WeaponSpecial)
{
	if (GameNetwork()->IsRunningClientFunctions() && (DigitanksGame()->GetCurrentPlayer()->GetClient() != (int)iClient))
		return;

	DigitanksGame()->WeaponSpecialCommand(DigitanksGame()->GetCurrentPlayer());
}

void CDigitanksGame::WeaponSpecialCommand(CDigitanksPlayer* pTeam)
{
	if (DigitanksGame()->GetGameType() != GAMETYPE_ARTILLERY)
		return;

	if (!pTeam)
	{
		::WeaponSpecial.RunCommand("");
		return;
	}

	if (pTeam != GetCurrentPlayer())
		return;

	tvector<CEntityHandle<CDigitanksWeapon> > ahWeapons;

	// Form a list of weapons to send the message to since sometimes it creates new projectiles,
	// and we don't want to send the message to those new ones.
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CDigitanksWeapon* pWeapon = dynamic_cast<CDigitanksWeapon*>(pEntity);
		if (!pWeapon)
			continue;

		if (!pWeapon->GetOwner())
			continue;

		// If it's not my grenade don't blow it up.
		if (pTeam != pWeapon->GetOwner()->GetPlayerOwner())
			continue;

		ahWeapons.push_back(pWeapon);
	}

	for (size_t i = 0; i < ahWeapons.size(); i++)
	{
		CDigitanksWeapon* pWeapon = ahWeapons[i];
		if (!pWeapon)
			continue;

		pWeapon->SpecialCommand();
	}

	DigitanksWindow()->GetHUD()->ClearHintWeapon();
}

void CDigitanksGame::AddTankAim(Vector vecAim, float flRadius, bool bFocus)
{
	vecAim.z = 0;
	m_avecTankAims.push_back(vecAim);
	m_aflTankAimRadius.push_back(flRadius);
	if (bFocus)
		m_iTankAimFocus = m_avecTankAims.size()-1;
}

void CDigitanksGame::GetTankAims(tvector<Vector>& avecAims, tvector<float>& aflAimRadius, size_t& iFocus)
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

void CDigitanksGame::OnDisplayLesson(tstring sLesson)
{
	if (sLesson == "strategy-command")
		SetControlMode(MODE_MOVE);

	// Make sure that features now enabled are turned on.
	DigitanksWindow()->GetHUD()->SetupMenu();
}

void CDigitanksGame::ClientEnterGame()
{
	BaseClass::ClientEnterGame();

	if (m_eGameType == GAMETYPE_MENU)
	{
		GetOverheadCamera()->SnapTarget(Vector(0,0,0));
		GetOverheadCamera()->SnapAngle(EAngle(55,20,0));
		GetOverheadCamera()->SnapDistance(60);
	}
	else if (m_eGameType != GAMETYPE_CAMPAIGN)
	{
		const CBaseEntity* pMember = GetCurrentLocalDigitanksPlayer()->GetUnit(0);
		if (GetCurrentLocalDigitanksPlayer() && pMember)
			GetOverheadCamera()->SnapTarget(pMember->GetGlobalOrigin());
		else
			GetOverheadCamera()->SnapTarget(Vector(0,0,0));
		GetOverheadCamera()->SnapAngle(EAngle(45,0,0));

		if (m_eGameType == GAMETYPE_ARTILLERY)
			GetOverheadCamera()->SnapDistance(220);
		else
			GetOverheadCamera()->SnapDistance(120);
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

float CDigitanksGame::GetVisibilityAtPoint(CDigitanksPlayer* pViewingTeam, Vector vecPoint)
{
	if (pViewingTeam && pViewingTeam->IsHumanControlled() && !ShouldRenderFogOfWar())
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
	bool bDisableBuffer = DigitanksGame()->IsFeatureDisabled(DISABLE_BUFFER);
	return !bDisableBuffer;
}

bool CDigitanksGame::CanBuildBuffers()
{
	return m_bLevelAllowsBuffers;
}

bool CDigitanksGame::CanBuildBatteries()
{
	return true;
}

bool CDigitanksGame::CanBuildPSUs()
{
	if (!m_bLevelAllowsPSUs)
		return false;

	bool bDisablePSU = DigitanksGame()->IsFeatureDisabled(DISABLE_PSU);
	return !bDisablePSU;
}

bool CDigitanksGame::CanBuildInfantryLoaders()
{
	bool bDisableLoaders = DigitanksGame()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

bool CDigitanksGame::CanBuildTankLoaders()
{
	if (!m_bLevelAllowsTankLoaders)
		return false;

	bool bDisableLoaders = DigitanksGame()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

bool CDigitanksGame::CanBuildArtilleryLoaders()
{
	if (!m_bLevelAllowsArtilleryLoaders)
		return false;

	bool bDisableLoaders = DigitanksGame()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

bool CDigitanksGame::IsWeaponAllowed(weapon_t eWeapon, const CDigitank* pTank)
{
	if (eWeapon == WEAPON_INFANTRYLASER)
	{
		if (m_bOverrideAllowLasers)
			return true;

		// Enemy tanks have access to this weapon from the first mission.
		if (DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN && pTank && pTank->GetPlayerOwner() && !pTank->GetPlayerOwner()->IsHumanControlled())
		{
			TAssert(!GameNetwork()->IsConnected());
			return m_pLevel->AllowEnemyInfantryLasers();
		}

		return m_bLevelAllowsInfantryLasers;
	}

	if (eWeapon == PROJECTILE_TREECUTTER)
	{
		// Enemy tanks have access to this weapon from the first mission.
		if (DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN && pTank && pTank->GetPlayerOwner() && !pTank->GetPlayerOwner()->IsHumanControlled())
			return true;

		return m_bLevelAllowsInfantryTreeCutters;
	}

	return true;
}

bool CDigitanksGame::IsInfantryFortifyAllowed()
{
	return m_bLevelAllowsInfantryFortify;
}

void CDigitanksGame::AllowLaser()
{
	m_bOverrideAllowLasers = true;
}

void CDigitanksGame::BeginAirstrike(Vector vecLocation)
{
	if (!GameNetwork()->IsHost())
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
	for (size_t i = 0; i < GetCurrentPlayer()->GetNumUnits(); i++)
	{
		CDigitanksEntity* pMember = GetCurrentPlayer()->GetUnit(i);
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

CDigitanksPlayer* CDigitanksGame::GetCurrentLocalDigitanksPlayer()
{
	size_t iLocalPlayers = GetNumLocalPlayers();

	if (!iLocalPlayers)
		return NULL;

	if (iLocalPlayers == 1)
		return static_cast<CDigitanksPlayer*>(GetLocalPlayer(0));

	for (size_t i = 0; i < iLocalPlayers; i++)
	{
		CPlayer* pTeam = GetLocalPlayer(i);
		if (GetCurrentPlayer() == pTeam)
			return static_cast<CDigitanksPlayer*>(pTeam);
	}

	return NULL;
}

void CDigitanksGame::SetCurrentLevel(tstring sLevel)
{
	CVar::SetCVar("game_level", sLevel);
	m_pLevel = CDigitanksGame::GetLevel(CVar::GetCVarValue("game_level"));

	if (!m_pLevel)
		return;

	m_bLevelAllowsBuffers = m_pLevel->AllowBuffers();
	m_bLevelAllowsPSUs = m_pLevel->AllowPSUs();
	m_bLevelAllowsTankLoaders = m_pLevel->AllowTankLoaders();
	m_bLevelAllowsArtilleryLoaders = m_pLevel->AllowArtilleryLoaders();
	m_bLevelAllowsInfantryLasers = m_pLevel->AllowInfantryLasers();
	m_bLevelAllowsInfantryTreeCutters = m_pLevel->AllowInfantryTreeCutters();
	m_bLevelAllowsInfantryFortify = m_pLevel->AllowInfantryFortify();
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

bool CDigitanksGame::IsFeatureDisabled(disable_t /*eDisabled*/)
{
	TStubbed("IsFeatureDisabled");
	return false;
}
