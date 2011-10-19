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
#include <tinker/lobby/lobby_client.h>
#include <game/gameserver.h>
#include <network/network.h>
#include <network/commands.h>
#include <tinker/chatbox.h>

#include <ui/digitankswindow.h>
#include <ui/ui.h>
#include <ui/instructor.h>
#include <ui/hud.h>
#include "powerup.h"
#include "terrain.h"
#include "dt_camera.h"
#include <ui/weaponpanel.h>

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
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iCurrentTeam);
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

void CDigitanksGame::Precache()
{
	BaseClass::Precache();

	// We precache this for the hud since it's not an entity
	PrecacheSound(_T("sound/actionsign.wav"));
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

	m_sObjective = _T("Win the game");
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

	// CDigitanksTeam
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

void CDigitanksGame::SetupGame(gametype_t eGameType)
{
	GameServer()->SetLoading(true);

	DigitanksWindow()->RenderLoading();

	SetupEntities();

	if (!GameNetwork()->IsHost())
		return;

	m_eGameType = eGameType;
	m_iTurn = 0;

	SetCurrentLevel(CVar::GetCVarValue("game_level"));

	m_hInstructor = GameServer()->Create<CInstructorEntity>("CInstructorEntity");

	if (eGameType == GAMETYPE_STANDARD)
		SetupStrategy();
	else if (eGameType == GAMETYPE_ARTILLERY)
		SetupArtillery();
	else if (eGameType == GAMETYPE_MENU)
		SetupMenuMarch();
	else if (eGameType == GAMETYPE_CAMPAIGN)
		SetupCampaign();

	if (eGameType != GAMETYPE_EMPTY)
		StartGame();

	DigitanksGame()->SetDifficulty(game_difficulty.GetInt());

	GameServer()->SetLoading(false);
}

void CDigitanksGame::ReadGameScript(tstring sScript)
{
	for (size_t i = 0; i < m_aflConstructionCosts.size(); i++)
		m_aflConstructionCosts[i] = 0;
	for (size_t i = 0; i < m_aflUpgradeCosts.size(); i++)
		m_aflUpgradeCosts[i] = 0;

	std::basic_ifstream<tchar> f((eastl::string("scripts/") + convertstring<tchar, char>(sScript)).c_str());
	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	CData* pConstructionCosts = pData->FindChild(_T("ConstructionCosts"));

	if (pConstructionCosts)
	{
		CData* pChild;
		
		pChild = pConstructionCosts->FindChild(_T("AutoTurret"));
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_FIREWALL] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("Minibuffer"));
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_MINIBUFFER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("Buffer"));
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_BUFFER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("BufferUpgrade"));
		if (pChild)
			m_aflUpgradeCosts[STRUCTURE_BUFFER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("Battery"));
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_BATTERY] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("PSU"));
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_PSU] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("PSUUpgrade"));
		if (pChild)
			m_aflUpgradeCosts[STRUCTURE_PSU] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("ResistorLoader"));
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_INFANTRYLOADER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("DigitankLoader"));
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_TANKLOADER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("ArtilleryLoader"));
		if (pChild)
			m_aflConstructionCosts[STRUCTURE_ARTILLERYLOADER] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("Resistor"));
		if (pChild)
			m_aflConstructionCosts[UNIT_INFANTRY] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("Tank"));
		if (pChild)
			m_aflConstructionCosts[UNIT_TANK] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("Artillery"));
		if (pChild)
			m_aflConstructionCosts[UNIT_ARTILLERY] = pChild->GetValueFloat();

		pChild = pConstructionCosts->FindChild(_T("Scout"));
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
			pResource->SetOrigin(m_hTerrain->GetPointHeight(Vector(x, 0, z)));
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
		pProp->SetOrigin(m_hTerrain->GetPointHeight(Vector(pLevelProp->m_vecPosition.x, 0, pLevelProp->m_vecPosition.y)));
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
			pProp->SetOrigin(m_hTerrain->GetPointHeight(Vector(x, 0, z)));
			pProp->SetAngles(EAngle(0, RandomFloat(0, 360), 0));
			pProp->SetColorSwap(m_hTerrain->GetPrimaryTerrainColor());

			if (GetGameType() == GAMETYPE_ARTILLERY)
			{
				pProp->SetModel(_T("models/props/prop05.obj"));
			}
			else
			{
				switch (RandomInt(0, 3))
				{
				case 0:
					pProp->SetModel(_T("models/props/prop01.obj"));
					break;

				case 1:
					pProp->SetModel(_T("models/props/prop02.obj"));
					break;

				case 2:
					pProp->SetModel(_T("models/props/prop03.obj"));
					break;

				case 3:
					pProp->SetModel(_T("models/props/prop04.obj"));
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
	pTeam->SetTeamName(tstring(_T("Network Guardians")));
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
			pTurret->SetOrigin(m_hTerrain->GetPointHeight(Vector(x, 0, z)));
			pTeam->AddEntity(pTurret);
		}
	}
}

CVar game_players("game_players", "1");
CVar game_bots("game_bots", "3");
CVar game_tanks("game_tanks", "3");

void CDigitanksGame::SetupArtillery()
{
	TMsg(_T("Setting up artillery game.\n"));

	m_sObjective = _T("Destroy all enemy tanks");

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
		iTanks = stoi(CGameLobbyClient::L_GetInfoValue(_T("tanks")).c_str());

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
			tstring sColor = pPlayer->GetInfoValue(_T("color"));
			if (sColor == _T("random") || sColor == _T(""))
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
		AddTeam(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));

		CDigitanksTeam* pTeam = GetDigitanksTeam(GetNumTeams()-1);

		if (GameServer()->ShouldSetupFromLobby())
		{
			CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
			if (pPlayer)
			{
				tstring sColor = pPlayer->GetInfoValue(_T("color"));
				if (sColor == _T("random") || sColor == _T(""))
				{
					size_t iColor = RandomInt(0, aiAvailableColors.size()-1);
					pTeam->SetColor(g_aclrTeamColors[aiAvailableColors[iColor]]);
					pTeam->SetTeamName(g_aszTeamNames[aiAvailableColors[iColor]]);
					aiAvailableColors.erase(aiAvailableColors.begin()+iColor);
				}
				else
				{
					size_t iColor = stoi(sColor.c_str());
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

			if (pPlayer && pPlayer->GetInfoValue(_T("bot")) != _T("1"))
				pTeam->SetTeamName(pPlayer->GetInfoValue(_T("name")));

			if (pPlayer->GetInfoValue(_T("bot")) == _T("1"))
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
			pTeam->SetTeamName(g_aszTeamNames[i]);

			if (i < game_players.GetInt())
			{
				if (i == 0)
				{
					tstring sPlayerNickname = TPortal_GetPlayerNickname();
					if (sPlayerNickname.length())
						pTeam->SetTeamName(sPlayerNickname);
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
	TMsg(_T("Setting up strategy game.\n"));

	m_sObjective = _T("Destroy all enemy CPUs");

	ReadGameScript(_T("strategy.txt"));

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
			tstring sColor = pPlayer->GetInfoValue(_T("color"));
			if (sColor == _T("random") || sColor == _T(""))
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
		AddTeam(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));

		CDigitanksTeam* pTeam = GetDigitanksTeam(GetNumTeams()-1);

		if (GameServer()->ShouldSetupFromLobby())
		{
			CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
			if (pPlayer)
			{
				tstring sColor = pPlayer->GetInfoValue(_T("color"));
				if (sColor == _T("random") || sColor == _T(""))
				{
					size_t iColor = RandomInt(0, aiAvailableColors.size()-1);
					pTeam->SetColor(g_aclrTeamColors[aiAvailableColors[iColor]]);
					pTeam->SetTeamName(g_aszTeamNames[aiAvailableColors[iColor]]);
					aiAvailableColors.erase(aiAvailableColors.begin()+iColor);
				}
				else
				{
					size_t iColor = stoi(sColor.c_str());
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

			if (pPlayer && pPlayer->GetInfoValue(_T("bot")) != _T("1"))
				pTeam->SetTeamName(pPlayer->GetInfoValue(_T("name")));

			if (pPlayer->GetInfoValue(_T("bot")) == _T("1"))
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
			pTeam->SetTeamName(g_aszTeamNames[i]);

			if (i == 0)
			{
				tstring sPlayerNickname = TPortal_GetPlayerNickname();
				if (sPlayerNickname.length())
					pTeam->SetTeamName(sPlayerNickname);

				pTeam->SetClient(NETWORK_LOCAL);
				pTeam->SetInstallID(CNetwork::GetInstallID());
			}
			else
				pTeam->SetClient(NETWORK_BOT);
		}

		pTeam->SetLoseCondition(LOSE_NOCPU);

		GetTerrain()->ClearArea(avecRandomStartingPositions[i], 40);

		CMobileCPU* pMobileCPU = GameServer()->Create<CMobileCPU>("CMobileCPU");
		pTeam->AddEntity(pMobileCPU);
		pMobileCPU->SetOrigin(m_hTerrain->GetPointHeight(avecRandomStartingPositions[i]));
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

		pTank->SetOrigin(GetTerrain()->GetPointHeight(vecTank));
		pTank->SetAngles(angTank);
		pTank->GiveBonusPoints(1, false);
	}

	if (GameServer()->ShouldSetupFromLobby())
	{
		for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
		{
			if (CGameLobbyClient::L_GetPlayer(i)->GetInfoValue(_T("bot")) == _T("1"))
				continue;

			// There's one neutral team at the front so skip it.
			m_ahTeams[i+1]->SetClient(CGameLobbyClient::L_GetPlayer(i)->iClient);
			m_ahTeams[i+1]->SetInstallID(LobbyNetwork()->GetClientInstallID(CGameLobbyClient::L_GetPlayer(i)->iClient));
		}
	}
	else
	{
		for (int i = 0; i < game_players.GetInt(); i++)
		{
			// There's one neutral team at the front so skip it.
			m_ahTeams[i+1]->SetClient(NETWORK_LOCAL);
			m_ahTeams[i+1]->SetInstallID(CNetwork::GetInstallID());
		}
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
	TMsg(_T("Setting up menu march.\n"));

	m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
	m_hTerrain->GenerateTerrain();

	AddTeam(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));
	m_ahTeams[0]->SetColor(Color(0, 0, 255));

#if !defined(_DEBUG) && !defined(TINKER_OPTIMIZE_SOFTWARE)
	CMenuMarcher* pMarcher;

	if (GameServer()->GetWorkListener())
		GameServer()->GetWorkListener()->SetAction(_T("Specifying marchers"), 4*5*4);

	for (size_t i = 0; i < 4; i++)
	{
		float flZ = RemapVal((float)i, 0, 4, -79, 79);

		for (size_t j = 0; j < 5; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				pMarcher = GameServer()->Create<CMenuMarcher>("CMenuMarcher");
				m_ahTeams[0]->AddEntity(pMarcher);

				pMarcher->SetOrigin(GetTerrain()->GetPointHeight(Vector(RemapVal((float)j, 0, 5, -15, 15), 0, flZ + RemapVal((float)k, 0, 4, -15, 15))));
				pMarcher->SetAngles(EAngle(0,90,0));

				if (GameServer()->GetWorkListener())
					GameServer()->GetWorkListener()->WorkProgress(i*4*5 + j*4 + k);
			}
		}
	}
#endif

	m_ahTeams[0]->SetClient(NETWORK_BOT);

	m_iPowerups = 0;
}

void MissionReload(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
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

void MissionWin(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	if (!CVar::GetCVarBool("cheats"))
		return;

	DigitanksGame()->PlayerVictory(asTokens);
}

CCommand mission_win("mission_win", ::MissionWin);

void MissionLose(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	DigitanksGame()->PlayerLoss(asTokens);
}

CCommand mission_lose("mission_lose", ::MissionLose);

void CDigitanksGame::SetupCampaign(bool bReload)
{
	TMsg(sprintf(tstring("Setting up campaign %s.\n"), CVar::GetCVarValue(_T("game_level")).c_str()));

	SetCurrentLevel(CVar::GetCVarValue("game_level"));

	m_sObjective = m_pLevel->GetObjective();

	GameServer()->Create<CCampaignEntity>("CCampaignEntity");

	if (!bReload)
	{
		m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
		m_hTerrain->GenerateTerrain();
	}

	AddTeam(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));
	m_ahTeams[0]->SetColor(Color(0, 0, 255));

	m_ahTeams[0]->SetClient(NETWORK_LOCAL);
	m_ahTeams[0]->SetInstallID(CNetwork::GetInstallID());

	tstring sPlayerNickname = TPortal_GetPlayerNickname();
	if (sPlayerNickname.length())
		m_ahTeams[0]->SetTeamName(sPlayerNickname);

	GetDigitanksTeam(0)->SetLoseCondition(LOSE_NOTANKS);

	AddTeam(GameServer()->Create<CDigitanksTeam>("CDigitanksTeam"));
	m_ahTeams[1]->SetColor(Color(255, 0, 0));

	m_ahTeams[1]->SetClient(NETWORK_BOT);

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

		pUnit->SetOrigin(m_hTerrain->GetPointHeight(Vector(pLevelUnit->m_vecPosition.x, 0, pLevelUnit->m_vecPosition.y)));
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
	eastl::vector<CLevel*> ahLevels = GetLevels(eGameType);
	if (i >= ahLevels.size())
		return NULL;

	return dynamic_cast<CDigitanksLevel*>(ahLevels[i]);
}

CDigitanksLevel* CDigitanksGame::GetLevel(tstring sFile)
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

	if (m_eGameType == GAMETYPE_STANDARD && !GameNetwork()->IsConnected())
		DigitanksWindow()->GetStoryPanel()->SetVisible(true);

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		pEntity->ClientEnterGame();
	}

	if (GameNetwork()->IsConnected() && !DigitanksWindow()->IsRegistered() && GetGameType() == GAMETYPE_STANDARD)
		DigitanksWindow()->Restart(GAMETYPE_MENU);

	if (m_eGameType == GAMETYPE_STANDARD || m_eGameType == GAMETYPE_ARTILLERY || m_eGameType == GAMETYPE_CAMPAIGN)
		DigitanksGame()->GetDigitanksCamera()->EnterGame();
}

void CDigitanksGame::StartNewRound()
{
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
		flTerrainHeight = (float)stof(CGameLobbyClient::L_GetInfoValue(_T("terrain")).c_str());

	m_hTerrain = GameServer()->Create<CTerrain>("CTerrain");
	m_hTerrain->GenerateTerrain(flTerrainHeight);

	float flMapBuffer = GetTerrain()->GetMapSize()*0.1f;
	float flMapSize = GetTerrain()->GetMapSize() - flMapBuffer*2;

	size_t iPlayers = (game_players.GetInt() + game_bots.GetInt());
	if (GameServer()->ShouldSetupFromLobby())
		iPlayers = CGameLobbyClient::L_GetNumPlayers();

	size_t iTanksPerPlayer = game_tanks.GetInt();
	if (GameServer()->ShouldSetupFromLobby())
		iTanksPerPlayer = stoi(CGameLobbyClient::L_GetInfoValue(_T("tanks")).c_str());

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

	if (GameServer()->GetWorkListener())
		GameServer()->GetWorkListener()->SetAction(_T("Randomizing tank locations"), iTotalTanks);

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

		if (GameServer()->GetWorkListener())
			GameServer()->GetWorkListener()->WorkProgress(iTanksPlaced);
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
}

bool CDigitanksGame::HasRounds()
{
	return m_eGameType == GAMETYPE_ARTILLERY;
}

void CDigitanksGame::Autosave(const eastl::vector<tstring>& sArgs)
{
	Autosave();
}

void CDigitanksGame::Autosave()
{
	GameServer()->SaveToFile(GetAppDataDirectory(DigitanksWindow()->AppDirectory(), _T("autosave.sav")).c_str());

	DigitanksWindow()->GetChatBox()->PrintChat(_T("Autosave...\n"));
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

	if (DigitanksWindow()->ShouldUseContextualCommands() && GetCurrentLocalDigitanksTeam())
	{
		CDigitank* pSelection = GetCurrentLocalDigitanksTeam()->GetPrimarySelectionTank();

		if (DigitanksWindow()->GetHUD()->GetWeaponPanel() && DigitanksWindow()->GetHUD()->GetWeaponPanel()->IsVisible())
			SetControlMode(MODE_NONE);
		else if (pSelection && pSelection->GetDigitanksTeam() == GetCurrentLocalDigitanksTeam())
		{
			CBaseEntity* pHit = NULL;
			Vector vecEntityPoint;

			bool bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecEntityPoint, &pHit);

			if (pHit)
			{
				CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pHit);
				if (pDTEntity)
				{
					if (pDTEntity->GetDigitanksTeam() != pSelection->GetDigitanksTeam())
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

	if (m_bTurnActive && GetCurrentTeam() && !GetCurrentTeam()->IsPlayerControlled() && GameNetwork()->IsHost())
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
				const CDigitank* pTank = GetCurrentTeam()->GetTank(i);
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
		const CTeam* pNextTeam = GetTeam((m_iCurrentTeam+(size_t)1)%GetNumTeams());
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

		if (GameNetwork()->IsHost() && GameServer()->GetGameTime() > m_flLastFireworks + RandomFloat(0.5f, 3.0f))
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
			vecTankAim = DigitanksGame()->GetTerrain()->GetPointHeight(pTank->GetOrigin() + vecDirection.Normalized() * vecDirection.Length2D() * 0.99f);
		}

		pTank->SetPreviewAim(vecTankAim);
		pTank->Fire();

		GetCurrentLocalDigitanksTeam()->HandledActionItem(pTank);
	}

	DigitanksWindow()->GetInstructor()->FinishedTutorial("artillery-command", true);
	SetControlMode(MODE_NONE);
}

void CDigitanksGame::CancelAutoMoves(const eastl::vector<tstring>& sArgs)
{
	CDigitanksTeam* pTeam = GetCurrentTeam();
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

	DigitanksWindow()->GetInstructor()->FinishedTutorial("artillery-endturn");
//	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_POWER);
}

void CDigitanksGame::EndTurn(int iConnection, CNetworkParameters* p)
{
	if (!GameNetwork()->ShouldRunClientFunction())
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
	if (!GameNetwork()->IsHost())
		return;

#ifndef DT_COMPETITION
	if (GetGameType() == GAMETYPE_STANDARD && !DigitanksWindow()->IsRegistered() && GetTurn() > GetDemoTurns())
	{
		DigitanksWindow()->Restart(GAMETYPE_MENU);
		return;
	}
#endif

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

				vecPoint = GetTerrain()->GetPointHeight(Vector(flX, 0, flZ));

				bIsVisible = false;

				// Skip the first team, that's the barbarians.
				for (size_t i = 1; i < GetNumTeams(); i++)
				{
					const CDigitanksTeam* pTeam = GetDigitanksTeam(i);
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

	GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "StartTurn");

	StartTurn(CONNECTION_GAME, NULL);

	if (GetCurrentTeam()->HasLost())
		EndTurn();
}

void CDigitanksGame::StartTurn(int iConnection, CNetworkParameters* p)
{
	if (!GameNetwork()->ShouldRunClientFunction())
		return;

	DigitanksGame()->GetDigitanksCamera()->ClearFollowTarget();

	if (m_iCurrentTeam == (size_t)0)
		m_iTurn++;

	if (GetGameType() != GAMETYPE_CAMPAIGN && m_iCurrentTeam == (size_t)0 && m_iTurn > 10 && m_iTurn%5 == 0)
		Autosave(eastl::vector<tstring>());

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

void CDigitanksGame::OnClientEnterGame(int iClient)
{
	size_t iInstallID = GameNetwork()->GetClientInstallID(iClient);

	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (m_ahTeams[i]->GetInstallID() == iInstallID)
		{
			m_ahTeams[i]->SetClient(iClient);
			m_ahTeams[i]->SetTeamName(GameNetwork()->GetClientNickname(iClient));
			return;
		}
	}

	// Couldn't find any spot for the guy? Take over a bot.
	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (!m_ahTeams[i]->IsPlayerControlled() && m_ahTeams[i]->IsHumanPlayable())
		{
			m_ahTeams[i]->SetInstallID(iInstallID);
			m_ahTeams[i]->SetClient(iClient);
			m_ahTeams[i]->SetTeamName(GameNetwork()->GetClientNickname(iClient));
			return;
		}
	}

	// No spots? Boot him.
	GameNetwork()->DisconnectClient(iClient);
}

bool CDigitanksGame::Explode(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flRadius, float flDamage, CBaseEntity* pIgnore, const CTeam* pTeamIgnore)
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

SERVER_GAME_COMMAND(HitIndicator)
{
	if (pCmd->GetNumArguments() == 0)
	{
		TMsg(_T("HitIndicator with 0 arguments.\n"));
		return;
	}

	if (pCmd->Arg(0) == _T("sdmg"))
	{
		if (pCmd->GetNumArguments() < 7)
		{
			TMsg(_T("HitIndicator sdmg with not enough arguments.\n"));
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

	if (pCmd->Arg(0) == _T("dmg"))
	{
		if (pCmd->GetNumArguments() < 7)
		{
			TMsg(_T("HitIndicator dmg with not enough arguments.\n"));
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

	if (pCmd->Arg(0) == _T("disable"))
	{
		if (pCmd->GetNumArguments() < 4)
		{
			TMsg(_T("HitIndicator disable with not enough arguments.\n"));
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

	if (pCmd->Arg(0) == _T("miss"))
	{
		if (pCmd->GetNumArguments() < 4)
		{
			TMsg(_T("HitIndicator miss with not enough arguments.\n"));
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

	if (pCmd->Arg(0) == _T("crit"))
	{
		if (pCmd->GetNumArguments() < 4)
		{
			TMsg(_T("HitIndicator crit with not enough arguments.\n"));
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

		// The barbarian team doesn't count towards teams left.
		if (GetGameType() == GAMETYPE_STANDARD && i == 0)
			continue;

		switch (GetDigitanksTeam(i)->GetLoseCondition())
		{
		case LOSE_NOCPU:
		{
			bool bHasCPU = false;
			for (size_t j = 0; j < m_ahTeams[i]->GetNumMembers(); j++)
			{
				const CBaseEntity* pEntity = m_ahTeams[i]->GetMember(j);
				if (dynamic_cast<const CCPU*>(pEntity) || dynamic_cast<const CMobileCPU*>(pEntity))
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

SERVER_GAME_COMMAND(GameOver)
{
	if (DigitanksGame()->GetListener() && DigitanksGame()->GetCurrentLocalDigitanksTeam() && !DigitanksGame()->GetCurrentLocalDigitanksTeam()->HasLost())
		DigitanksGame()->GetListener()->GameOver(!DigitanksGame()->GetCurrentLocalDigitanksTeam()->HasLost());

	DigitanksGame()->GetDigitanksCamera()->SetDistance(250);
	DigitanksGame()->GetDigitanksCamera()->SetTarget(Vector(0,0,0));
}

void CDigitanksGame::GameOver()
{
	if (GameServer()->IsLoading())
		return;

	m_bPartyMode = true;
	m_flPartyModeStart = GameServer()->GetGameTime();

	::GameOver.RunCommand(_T(""));
}

void CDigitanksGame::PlayerVictory(const eastl::vector<tstring>& sArgs)
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

void CDigitanksGame::PlayerLoss(const eastl::vector<tstring>& sArgs)
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

void CDigitanksGame::TankSelectionMedal(const eastl::vector<tstring>& sArgs)
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

CDigitanksTeam* CDigitanksGame::GetDigitanksTeam(size_t i) const
{
	return static_cast<CDigitanksTeam*>(BaseClass::GetTeam(i));
}

CDigitanksTeam* CDigitanksGame::GetCurrentTeam() const
{
	if (m_iCurrentTeam >= m_ahTeams.size())
		return NULL;

	return static_cast<CDigitanksTeam*>(m_ahTeams[m_iCurrentTeam].GetPointer());
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

	if (m_eControlMode == eMode)
		return;

	GetPrimarySelection()->OnControlModeChange(m_eControlMode, eMode);

	m_eControlMode = eMode;

	if (eMode == MODE_AIM)
		DigitanksWindow()->GetInstructor()->FinishedTutorial("mission-1-turret-spotted");

	DigitanksWindow()->GetHUD()->SetupMenu();
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

CDigitanksCamera* CDigitanksGame::GetDigitanksCamera()
{
	CCamera* pCamera = GameServer()->GetCamera();
	return dynamic_cast<CDigitanksCamera*>(pCamera);
}

float CDigitanksGame::GetGravity()
{
	return -20;
}

CLIENT_GAME_COMMAND(WeaponSpecial)
{
	if (GameNetwork()->IsRunningClientFunctions() && (DigitanksGame()->GetCurrentTeam()->GetClient() != (int)iClient))
		return;

	DigitanksGame()->WeaponSpecialCommand(DigitanksGame()->GetCurrentTeam());
}

void CDigitanksGame::WeaponSpecialCommand(CDigitanksTeam* pTeam)
{
	if (DigitanksGame()->GetGameType() != GAMETYPE_ARTILLERY)
		return;

	if (!pTeam)
	{
		::WeaponSpecial.RunCommand(_T(""));
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

		pTank->SetOrigin(GetTerrain()->GetPointHeight(Vector(0, 0, 0)));

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

		pTank->SetOrigin(GetTerrain()->GetPointHeight(Vector(0, 0, -50)));

		GetDigitanksCamera()->SetTarget(GetDigitanksTeam(0)->GetTank(0)->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, 0, 0));
	}
	else if (sTutorial == CInstructor::TUTORIAL_POWERUP)
	{
		CPowerup* pPowerup = GameServer()->Create<CPowerup>("CPowerup");
		pPowerup->SetOrigin(GetTerrain()->GetPointHeight(GetDigitanksTeam(0)->GetTank(0)->GetOrigin() + Vector(0, 0, -10)));
	}
	else if (sTutorial == CInstructor::TUTORIAL_SHIFTSELECT)
	{
		CDigitank* pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[0]->AddEntity(pTank);
		pTank->StartTurn();
		pTank->SetOrigin(GetTerrain()->GetPointHeight(m_ahTeams[0]->GetMember(0)->GetOrigin() + Vector(-15, 0, 15)));

		pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[0]->AddEntity(pTank);
		pTank->StartTurn();
		pTank->SetOrigin(GetTerrain()->GetPointHeight(m_ahTeams[0]->GetMember(0)->GetOrigin() + Vector(15, 0, -15)));
	}
	else if (sTutorial == CInstructor::TUTORIAL_THEEND_BASICS)
	{
		// So that pressing the escape key works the first time.
		SetControlMode(MODE_NONE);
	}
	else if (sTutorial == CInstructor::TUTORIAL_INTRO_BASES)
	{
		CCPU* pCPU = GameServer()->Create<CCPU>("CCPU");
		pCPU->SetOrigin(GetTerrain()->GetPointHeight(Vector(0, 0, 0)));
		m_ahTeams[0]->AddEntity(pCPU);
		pCPU->UpdateTendrils();

		CResource* pResource = GameServer()->Create<CResource>("CResource");
		pResource->SetOrigin(GetTerrain()->GetPointHeight(Vector(0, 0, 20)));

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

		pTank->SetOrigin(GetTerrain()->GetPointHeight(Vector(0, 0, 0)));

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

		pTank->SetOrigin(GetTerrain()->GetPointHeight(Vector(0, 0, 0)));

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
		pTarget->SetOrigin(GetTerrain()->GetPointHeight(vecOrigin));

		CDigitank* pSpotter = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		m_ahTeams[0]->AddEntity(pSpotter);
		pSpotter->StartTurn();

		vecOrigin = pArtillery->GetOrigin() + AngleVector(pTarget->GetAngles()) * (pArtillery->GetMinRange() + pArtillery->GetEffRange())/2;
		pSpotter->SetOrigin(GetTerrain()->GetPointHeight(vecOrigin));

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

		pTank->SetOrigin(GetTerrain()->GetPointHeight(Vector(0.0f, 0, 0.0f)));

		GetDigitanksCamera()->SetTarget(pTank->GetOrigin());
		GetDigitanksCamera()->SetDistance(100);
		GetDigitanksCamera()->SetAngle(EAngle(45, 0, 0));

		DigitanksGame()->GetTerrain()->CalculateVisibility();
	}
	else if (sTutorial == CInstructor::TUTORIAL_TORPEDO)
	{
		CCPU* pCPU = GameServer()->Create<CCPU>("CCPU");
		pCPU->SetOrigin(GetTerrain()->GetPointHeight(Vector(20, 0, 20)));
		m_ahTeams[1]->AddEntity(pCPU);

		CBuffer* pBuffer = GameServer()->Create<CBuffer>("CBuffer");
		pBuffer->CompleteConstruction();
		pBuffer->SetOrigin(GetTerrain()->GetPointHeight(Vector(-20, 0, 20)));
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
		const CBaseEntity* pMember = GetCurrentLocalDigitanksTeam()->GetMember(0);
		if (GetCurrentLocalDigitanksTeam() && pMember)
			GetDigitanksCamera()->SnapTarget(pMember->GetOrigin());
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
	if (pViewingTeam && pViewingTeam->IsPlayerControlled() && !ShouldRenderFogOfWar())
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
	if (!m_bLevelAllowsTankLoaders)
		return false;

	bool bDisableLoaders = DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

bool CDigitanksGame::CanBuildArtilleryLoaders()
{
	if (!m_bLevelAllowsArtilleryLoaders)
		return false;

	bool bDisableLoaders = DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_LOADERS);
	return !bDisableLoaders;
}

bool CDigitanksGame::IsWeaponAllowed(weapon_t eWeapon, const CDigitank* pTank)
{
	if (eWeapon == WEAPON_INFANTRYLASER)
	{
		if (m_bOverrideAllowLasers)
			return true;

		// Enemy tanks have access to this weapon from the first mission.
		if (DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN && pTank && pTank->GetTeam() && !pTank->GetTeam()->IsPlayerControlled())
		{
			TAssert(!GameNetwork()->IsConnected());
			return m_pLevel->AllowEnemyInfantryLasers();
		}

		return m_bLevelAllowsInfantryLasers;
	}

	if (eWeapon == PROJECTILE_TREECUTTER)
	{
		// Enemy tanks have access to this weapon from the first mission.
		if (DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN && pTank && pTank->GetTeam() && !pTank->GetTeam()->IsPlayerControlled())
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

void CDigitanksGame::SetCurrentLevel(eastl::string sLevel)
{
	CVar::SetCVar("game_level", sLevel);
	m_pLevel = CDigitanksGame::GetLevel(CVar::GetCVarValue(_T("game_level")));

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
