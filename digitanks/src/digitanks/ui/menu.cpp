#include "menu.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include <tinker_platform.h>
#include <strutils.h>
#include <mtrand.h>

#include <tinker/cvar.h>
#include <datamanager/data.h>
#include <dt_version.h>
#include <textures/texturelibrary.h>
#include <renderer/game_renderer.h>
#include <renderer/renderingcontext.h>
#include <tengine/lobby/lobby_client.h>
#include <glgui/filedialog.h>
#include <glgui/textfield.h>
#include <glgui/checkbox.h>
#include <glgui/menu.h>
#include <ui/instructor.h>

#include <digitanksgame.h>
#include <digitankslevel.h>
#include <campaign/campaigndata.h>

#include "digitankswindow.h"
#include "hud.h"
#include "lobbyui.h"

#define _T(x) x

using namespace glgui;

CMainMenu::CMainMenu()
	: CPanel(0, 0, 310, 620)
{
	m_pCampaign = AddControl(new CButton(0, 0, 100, 100, _T("CAMPAIGN")));
	m_pCampaign->SetClickedListener(this, OpenCampaignPanel);
	m_pCampaign->SetFont(_T("header"), 28);
	m_pCampaign->SetButtonColor(Color(0,0,0));

	m_pPlay = AddControl(new CButton(0, 0, 100, 100, _T("SKIRMISH")));
	m_pPlay->SetClickedListener(this, OpenGamesPanel);
	m_pPlay->SetFont(_T("header"), 28);
	m_pPlay->SetButtonColor(Color(0,0,0));

	m_pMultiplayer = AddControl(new CButton(0, 0, 100, 100, _T("MULTIPLAYER")));
	m_pMultiplayer->SetClickedListener(this, OpenMultiplayerPanel);
	m_pMultiplayer->SetFont(_T("header"), 28);
	m_pMultiplayer->SetButtonColor(Color(0,0,0));

	m_pOptions = AddControl(new CButton(0, 0, 100, 100, _T("OPTIONS")));
	m_pOptions->SetClickedListener(this, OpenOptionsPanel);
	m_pOptions->SetFont(_T("header"), 28);
	m_pOptions->SetButtonColor(Color(0,0,0));

	m_pQuit = AddControl(new CButton(0, 0, 100, 100, _T("QUIT")));
	m_pQuit->SetClickedListener(this, Quit);
	m_pQuit->SetFont(_T("header"), 28);
	m_pQuit->SetButtonColor(Color(0,0,0));

	m_pHint = AddControl(new CLabel(0, 0, 100, 100, _T("")));
	m_pHint->SetFont(_T("text"));

	m_pShowCredits = AddControl(new CButton(0, 0, 100, 100, _T("CREDITS")));
	m_pShowCredits->SetClickedListener(this, Credits);
	m_pShowCredits->SetFont(_T("header"), 9);
	m_pShowCredits->SetButtonColor(Color(0,0,0));

	m_pCredits = AddControl(new CLabel(0, 0, 100, 100, _T("")));
	m_pCredits->SetFont(_T("text"), 18);
	m_pCredits->SetAlign(CLabel::TA_TOPCENTER);

	m_pVersion = AddControl(new CLabel(0, 0, 100, 100, _T("")));
	m_pVersion->SetFont(_T("text"), 11);
	m_pVersion->SetAlign(CLabel::TA_LEFTCENTER);

	m_pDockPanel = NULL;
}

void CMainMenu::Layout()
{
	SetPos(40, 130);

	m_pCampaign->SetPos(20, 120);
	m_pCampaign->SetSize(270, 80);

	m_pPlay->SetPos(20, 220);
	m_pPlay->SetSize(270, 80);

	m_pMultiplayer->SetPos(20, 320);
	m_pMultiplayer->SetSize(270, 80);

	m_pOptions->SetPos(20, 420);
	m_pOptions->SetSize(270, 80);

	m_pQuit->SetPos(20, 520);
	m_pQuit->SetSize(270, 80);

	if (m_pDockPanel)
	{
		m_pDockPanel->SetSize(570, 520);
		m_pDockPanel->SetPos(390, 30);
	}

	m_pHint->SetPos(375, 440);
	m_pHint->SetSize(350, 160);

	m_pShowCredits->SetPos(130, 80);
	m_pShowCredits->SetSize(50, 20);

	m_pCredits->SetVisible(false);

	m_pVersion->SetPos(-GetLeft()+5, -GetTop());
	m_pVersion->SetSize(120, 20);
	m_pVersion->SetText(DIGITANKS_VERSION);

	BaseClass::Layout();
}

void CMainMenu::Think()
{
	BaseClass::Think();

	m_flCreditsRoll += GameServer()->GetFrameTime()*30;
	float x, y;
	GetAbsPos(x, y);
	m_pCredits->SetPos(370 - x, (int)(DigitanksWindow()->GetWindowHeight() - y - m_flCreditsRoll));
}

void CMainMenu::Paint(float x, float y, float w, float h)
{
	CRootPanel::PaintRect(0, 0, m_pVersion->GetWidth(), m_pVersion->GetHeight(), Color(0, 0, 0, 100));

	float hx, hy;
	m_pHint->GetAbsPos(hx, hy);
	if (m_pHint->GetText().length() > 1)
		CRootPanel::PaintRect(hx-25, hy-3, m_pHint->GetWidth()+50, m_pHint->GetHeight()+6, Color(0, 0, 0, 255));

	if (m_pCredits->IsVisible())
	{
		float cx, cy;
		m_pCredits->GetAbsPos(cx, cy);

		CRootPanel::PaintRect(cx-5, 0, m_pCredits->GetWidth()+10, CRootPanel::Get()->GetHeight(), Color(0, 0, 0, 100));
	}

	BaseClass::Paint(x, y, w, h);

	if (DigitanksGame() && DigitanksGame()->GetGameType() == GAMETYPE_MENU)
	{
		CRenderingContext c(GameServer()->GetRenderer(), true);

		c.SetBlend(BLEND_ALPHA);
		CHUD::PaintHUDSheet("MainMenu", 20, 20, 350, 730);
		if (!m_pCredits->IsVisible())
			CRootPanel::PaintTexture(DigitanksWindow()->GetLunarWorkshopLogo(), CRootPanel::Get()->GetWidth()-200-20, CRootPanel::Get()->GetHeight()-200, 200, 200);
	}
}

void CMainMenu::SetVisible(bool bVisible)
{
	BaseClass::SetVisible(bVisible);

	if (!bVisible)
	{
		GetDockPanel()->SetVisible(bVisible);
		m_pHint->SetText("");
	}

	m_pCredits->SetVisible(false);
}

void CMainMenu::OpenCampaignPanelCallback(const tstring& sArgs)
{
	CDockPanel* pDock = GetDockPanel();
	pDock->SetDockedPanel(new CCampaignPanel());
	pDock->SetVisible(true);

	m_pCredits->SetVisible(false);
}

void CMainMenu::OpenGamesPanelCallback(const tstring& sArgs)
{
	CDockPanel* pDock = GetDockPanel();
	pDock->SetDockedPanel(new CGamesPanel());
	pDock->SetVisible(true);

	m_pCredits->SetVisible(false);
}

void CMainMenu::OpenMultiplayerPanelCallback(const tstring& sArgs)
{
	CDockPanel* pDock = GetDockPanel();
	pDock->SetDockedPanel(new CMultiplayerPanel());
	pDock->SetVisible(true);

	m_pCredits->SetVisible(false);
}

void CMainMenu::OpenOptionsPanelCallback(const tstring& sArgs)
{
	CDockPanel* pDock = GetDockPanel();
	pDock->SetDockedPanel(new COptionsPanel());
	pDock->SetVisible(true);

	m_pCredits->SetVisible(false);
}

void CMainMenu::QuitCallback(const tstring& sArgs)
{
	DigitanksWindow()->CloseApplication();
}

void CMainMenu::CreditsCallback(const tstring& sArgs)
{
	std::basic_ifstream<tchar> i;
	i.open("credits.txt");
	std::basic_string<tchar> sCredits;
	if (i.is_open())
	{
		while (i.good())
		{
			std::basic_string<tchar> sLine;
			getline(i, sLine);
			if (sLine.length())
				sCredits.append(sLine);
			else
				sCredits.append(_T(" "));	// The text renderer skips empty lines
			sCredits.append(_T("\n"));
		}
	}

	m_pCredits->SetText(sCredits.c_str());

	m_pCredits->SetSize(DigitanksWindow()->GetWindowWidth()-m_pCredits->GetLeft()-40, 9999);
	m_pCredits->SetVisible(true);
	m_flCreditsRoll = 0;

	GetDockPanel()->SetVisible(false);
	m_pHint->SetText("");
}

CControl<CDockPanel> CMainMenu::GetDockPanel()
{
	if (!m_pDockPanel)
	{
		m_pDockPanel = CRootPanel::Get()->AddControl(new CDockPanel());
		m_pDockPanel->SetBGColor(Color(0, 0, 0, 255));

		m_pDockPanel->SetSize(570, 520);
		m_pDockPanel->SetPos(390, 30);
	}

	return m_pDockPanel;
}

void CMainMenu::SetHint(const tstring& s)
{
	m_pHint->SetText(s);
}

CDockPanel::CDockPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pDockedPanel = NULL;
}

void CDockPanel::Layout()
{
	BaseClass::Layout();
}

void CDockPanel::Paint(float x, float y, float w, float h)
{
	CRootPanel::PaintRect(x, y, w, h, m_clrBackground);

	BaseClass::Paint(x, y, w, h);
}

void CDockPanel::SetDockedPanel(glgui::CControl<glgui::CPanel> pDock)
{
	if (m_pDockedPanel)
		RemoveControl(m_pDockedPanel);

	if (pDock)
	{
		m_pDockedPanel = pDock;
		AddControl(m_pDockedPanel);
		m_pDockedPanel->SetPos(0, 0);
		m_pDockedPanel->SetSize(GetWidth(), GetHeight());
		m_pDockedPanel->Layout();
	}
}

CCampaignPanel::CCampaignPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pNewCampaign = AddControl(new CButton(0, 0, 100, 100, _T("BEGIN CAMPAIGN")));
	m_pNewCampaign->SetClickedListener(this, NewCampaign);
	m_pNewCampaign->SetCursorInListener(this, NewCampaignHint);
	m_pNewCampaign->SetFont(_T("header"), 18);

	m_pContinueCampaign = AddControl(new CButton(0, 0, 100, 100, _T("CONTINUE CAMPAIGN")));
	m_pContinueCampaign->SetClickedListener(this, ContinueCampaign);
	m_pContinueCampaign->SetCursorInListener(this, ContinueCampaignHint);
	m_pContinueCampaign->SetFont(_T("header"), 18);
}

void CCampaignPanel::Layout()
{
	CCampaignData* pData = DigitanksWindow()->GetCampaignData();

	m_pNewCampaign->SetSize(300, 40);
	m_pNewCampaign->SetPos(GetWidth()/2-m_pNewCampaign->GetWidth()/2, 60);

	m_pContinueCampaign->SetSize(300, 40);
	m_pContinueCampaign->SetPos(GetWidth()/2-m_pContinueCampaign->GetWidth()/2, 120);

	if (pData && pData->GetCurrentLevel() > 0 && pData->GetCurrentLevel() < CCampaignInfo::GetCampaignInfo()->m_asLevels.size())
		m_pContinueCampaign->SetVisible(true);
	else
		m_pContinueCampaign->SetVisible(false);

	BaseClass::Layout();
}

void CCampaignPanel::NewCampaignCallback(const tstring& sArgs)
{
	DigitanksWindow()->NewCampaign();

	DigitanksGame()->SetDifficulty(1);

	DigitanksWindow()->GetMainMenu()->SetVisible(false);
}

void CCampaignPanel::NewCampaignHintCallback(const tstring& sArgs)
{
	DigitanksWindow()->GetMainMenu()->SetHint(_T("The Digitanks have captured your files. You've got to rescue them and take your hard drive back!"));
}

void CCampaignPanel::ContinueCampaignCallback(const tstring& sArgs)
{
	DigitanksWindow()->ContinueCampaign();

	DigitanksGame()->SetDifficulty(1);

	DigitanksWindow()->GetMainMenu()->SetVisible(false);
}

void CCampaignPanel::ContinueCampaignHintCallback(const tstring& sArgs)
{
	DigitanksWindow()->GetMainMenu()->SetHint(_T("Resume a previous campaign where you left off."));
}

CGamesPanel::CGamesPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pArtillery = AddControl(new CButton(0, 0, 100, 100, _T("ARTILLERY")));
	m_pArtillery->SetClickedListener(this, Artillery);
	m_pArtillery->SetCursorInListener(this, ArtilleryHint);
	m_pArtillery->SetFont(_T("header"), 18);

	m_pStrategy = AddControl(new CButton(0, 0, 100, 100, _T("STRATEGY")));
	m_pStrategy->SetClickedListener(this, Strategy);
	m_pStrategy->SetCursorInListener(this, StrategyHint);
	m_pStrategy->SetFont(_T("header"), 18);

	m_pLoad = AddControl(new CButton(0, 0, 100, 100, _T("LOAD")));
	m_pLoad->SetClickedListener(this, Load);
	m_pLoad->SetFont(_T("header"), 18);

	m_pDockPanel = AddControl(new CDockPanel());
	m_pDockPanel->SetBGColor(Color(12, 13, 12, 255));
}

void CGamesPanel::Layout()
{
	m_pArtillery->SetPos(20, 20);
	m_pArtillery->SetSize(135, 40);

	m_pStrategy->SetPos(20, 80);
	m_pStrategy->SetSize(135, 40);

	m_pLoad->SetPos(20, GetHeight() - 60);
	m_pLoad->SetSize(135, 40);

	m_pDockPanel->SetSize(GetWidth() - 20 - 135 - 20 - 20, GetHeight() - 40);
	m_pDockPanel->SetPos(20 + 135 + 20, 20);

	BaseClass::Layout();
}

void CGamesPanel::ArtilleryCallback(const tstring& sArgs)
{
	m_pDockPanel->SetDockedPanel(new CArtilleryGamePanel());
}

void CGamesPanel::StrategyCallback(const tstring& sArgs)
{
	m_pDockPanel->SetDockedPanel(new CStrategyGamePanel());
}

void CGamesPanel::LoadCallback(const tstring& sArgs)
{
	glgui::CFileDialog::ShowOpenDialog(DigitanksWindow()->GetAppDataDirectory(), ".sav", this, Open);
}

void CGamesPanel::OpenCallback(const tstring& sArgs)
{
	tstring sFilename = glgui::CFileDialog::GetFile();
	if (!sFilename.length())
		return;

	DigitanksWindow()->RenderLoading();

	if (CGameServer::LoadFromFile(sFilename.c_str()))
		DigitanksWindow()->GetMainMenu()->SetVisible(false);
	else
	{
		DigitanksWindow()->DestroyGame();
		DigitanksWindow()->CreateGame("menu");
	}
}

void CGamesPanel::ArtilleryHintCallback(const tstring& sArgs)
{
	DigitanksWindow()->GetMainMenu()->SetHint(_T("Artillery mode is a quick no-holds-barred fight to the death. You control 1 to 4 tanks in a head-on deathmatch against your enemies. The last team standing wins. Not much strategy here, just make sure you bring the biggest guns!"));
}

void CGamesPanel::StrategyHintCallback(const tstring& sArgs)
{
	DigitanksWindow()->GetMainMenu()->SetHint(_T("Strap in and grab a cup of coffee! Strategy mode takes a couple hours to play. You control a CPU, build a base, and produce units. You'll have to control and harvest the valuable Electronode resources to win. The objective is to destroy all enemy CPUs."));
}

CMultiplayerPanel::CMultiplayerPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pConnect = AddControl(new CButton(0, 0, 100, 100, _T("CONNECT")));
	m_pConnect->SetClickedListener(this, Connect);
	m_pConnect->SetCursorInListener(this, ClientHint);
	m_pConnect->SetFont(_T("header"), 18);

	m_pCreateArtilleryLobby = AddControl(new CButton(0, 0, 100, 100, _T("CREATE ARTILLERY LOBBY")));
	m_pCreateArtilleryLobby->SetClickedListener(this, CreateArtilleryLobby);
	m_pCreateArtilleryLobby->SetCursorInListener(this, CreateArtilleryHint);
	m_pCreateArtilleryLobby->SetFont(_T("header"), 18);

	m_pCreateStrategyLobby = AddControl(new CButton(0, 0, 100, 100, _T("CREATE STRATEGY LOBBY")));
	m_pCreateStrategyLobby->SetClickedListener(this, CreateStrategyLobby);
	m_pCreateStrategyLobby->SetCursorInListener(this, CreateStrategyHint);
	m_pCreateStrategyLobby->SetFont(_T("header"), 18);

	m_pDockPanel = AddControl(new CDockPanel());
	m_pDockPanel->SetBGColor(Color(12, 13, 12, 255));
}

void CMultiplayerPanel::Layout()
{
	m_pConnect->SetPos(20, 20);
	m_pConnect->SetSize(135, 40);

	m_pCreateArtilleryLobby->SetPos(20, 100);
	m_pCreateArtilleryLobby->SetSize(135, 80);

	m_pCreateStrategyLobby->SetPos(20, 200);
	m_pCreateStrategyLobby->SetSize(135, 80);

	m_pDockPanel->SetSize(GetWidth() - 20 - 135 - 20 - 20, GetHeight() - 40);
	m_pDockPanel->SetPos(20 + 135 + 20, 20);

	BaseClass::Layout();
}

void CMultiplayerPanel::ConnectCallback(const tstring& sArgs)
{
	m_pDockPanel->SetDockedPanel(new CConnectPanel());
}

void CMultiplayerPanel::CreateArtilleryLobbyCallback(const tstring& sArgs)
{
	m_pDockPanel->SetDockedPanel(new CCreateLobbyPanel(GAMETYPE_ARTILLERY));
}

void CMultiplayerPanel::CreateStrategyLobbyCallback(const tstring& sArgs)
{
	m_pDockPanel->SetDockedPanel(new CCreateLobbyPanel(GAMETYPE_STANDARD));
}

void CMultiplayerPanel::LoadCallback(const tstring& sArgs)
{
	glgui::CFileDialog::ShowOpenDialog(DigitanksWindow()->GetAppDataDirectory(""), ".sav", this, Open);
}

void CMultiplayerPanel::OpenCallback(const tstring& sArgs)
{
	tstring sFilename = glgui::CFileDialog::GetFile();
	if (!sFilename.length())
		return;

	GameServer()->SetServerType(SERVER_HOST);

	DigitanksWindow()->RenderLoading();

	if (CGameServer::LoadFromFile(sFilename.c_str()))
		DigitanksWindow()->GetMainMenu()->SetVisible(false);
	else
	{
		DigitanksWindow()->DestroyGame();
		DigitanksWindow()->CreateGame("menu");
	}
}

void CMultiplayerPanel::ClientHintCallback(const tstring& sArgs)
{
	DigitanksWindow()->GetMainMenu()->SetHint(_T("Enter a hostname and port to connect to a remote host and play."));
}

void CMultiplayerPanel::CreateArtilleryHintCallback(const tstring& sArgs)
{
	DigitanksWindow()->GetMainMenu()->SetHint(_T("Artillery mode is a quick no-holds-barred fight to the death. You control 1 to 4 tanks in a head-on deathmatch against your enemies. The last team standing wins. Not much strategy here, just make sure you bring the biggest guns!"));
}

void CMultiplayerPanel::CreateStrategyHintCallback(const tstring& sArgs)
{
	DigitanksWindow()->GetMainMenu()->SetHint(_T("Strap in and grab a cup of coffee! Strategy mode takes a couple hours to play. You control a CPU, build a base, and produce units. You'll have to control and harvest the valuable Electronode resources to win. The objective is to destroy all enemy CPUs."));
}

CCreateLobbyPanel::CCreateLobbyPanel(gametype_t eGameType)
	: CPanel(0, 0, 570, 520)
{
	m_eGameType = eGameType;

	if (m_eGameType == GAMETYPE_ARTILLERY)
	{
		m_pCreateHotseatLobby = AddControl(new CButton(0, 0, 100, 100, _T("Create Hotseat Lobby")));
		m_pCreateHotseatLobby->SetClickedListener(this, CreateHotseatLobby);
		m_pCreateHotseatLobby->SetCursorInListener(this, CreateHotseatHint);
		m_pCreateHotseatLobby->SetFont(_T("header"), 12);
	}
	else
		m_pCreateHotseatLobby = NULL;

	m_pCreateOnlineLobby = AddControl(new CButton(0, 0, 100, 100, _T("Create Online Lobby")));
	m_pCreateOnlineLobby->SetClickedListener(this, CreateOnlineLobby);
	m_pCreateOnlineLobby->SetCursorInListener(this, CreateOnlineHint);
	m_pCreateOnlineLobby->SetFont(_T("header"), 12);
}

void CCreateLobbyPanel::Layout()
{
	if (m_pCreateHotseatLobby)
	{
		m_pCreateHotseatLobby->SetSize(135, 40);
		m_pCreateHotseatLobby->SetPos(GetWidth()/2-135/2, 160);
	}

	m_pCreateOnlineLobby->SetSize(135, 40);
	m_pCreateOnlineLobby->SetPos(GetWidth()/2-135/2, 220);

	BaseClass::Layout();
}

void CCreateLobbyPanel::CreateHotseatLobbyCallback(const tstring& sArgs)
{
	CVar::SetCVar("lobby_gametype", (int)m_eGameType);

	DigitanksWindow()->GetLobbyPanel()->CreateLobby(false);
}

void CCreateLobbyPanel::CreateOnlineLobbyCallback(const tstring& sArgs)
{
	CVar::SetCVar("lobby_gametype", (int)m_eGameType);

	DigitanksWindow()->GetLobbyPanel()->CreateLobby(true);
}

void CCreateLobbyPanel::CreateHotseatHintCallback(const tstring& sArgs)
{
	DigitanksWindow()->GetMainMenu()->SetHint(_T("Hotseat games are played with you and a few friends on this computer. You and your friends will take turns at the computer."));
}

void CCreateLobbyPanel::CreateOnlineHintCallback(const tstring& sArgs)
{
	DigitanksWindow()->GetMainMenu()->SetHint(_T("Online multiplayer games take place over the internet. You need a LAN or internet connection and a few friends to play online."));
}

CConnectPanel::CConnectPanel()
	: CPanel(0, 0, 570, 520)
{
	DigitanksWindow()->SetServerType(SERVER_CLIENT);

	m_pHostnameLabel = AddControl(new CLabel(0, 0, 32, 32, _T("Lobby Host:")));
	m_pHostnameLabel->SetWrap(false);
	m_pHostnameLabel->SetFont(_T("text"));

	m_pHostname = AddControl(new CTextField());

	m_pConnect = AddControl(new CButton(0, 0, 100, 100, _T("Connect")));
	m_pConnect->SetClickedListener(this, Connect);
	m_pConnect->SetFont(_T("header"), 12);
}

void CConnectPanel::Layout()
{
	m_pHostnameLabel->SetPos(GetWidth()/2-m_pHostnameLabel->GetWidth()/2, GetHeight()-330);
	m_pHostname->SetPos(GetWidth()/2-m_pHostname->GetWidth()/2, GetHeight()-300);

	m_pConnect->SetSize(135, 40);
	m_pConnect->SetPos(GetWidth()/2-135/2, GetHeight()-160);

	BaseClass::Layout();
}

void CConnectPanel::ConnectCallback(const tstring& sArgs)
{
	DigitanksWindow()->GetLobbyPanel()->ConnectToLocalLobby(m_pHostname->GetText());
}

CArtilleryGamePanel::CArtilleryGamePanel(bool bMultiplayer)
	: CPanel(0, 0, 570, 520)
{
	m_pLevels = AddControl(new CTree());
	m_pLevels->SetSelectedListener(this, LevelChosen);

	for (size_t i = 0; i < CDigitanksGame::GetNumLevels(GAMETYPE_ARTILLERY); i++)
	{
		CLevel* pLevel = CDigitanksGame::GetLevel(GAMETYPE_ARTILLERY, i);
		m_pLevels->AddNode(pLevel->GetName());
		m_pLevels->GetNode(i)->SetCursorInListener(this, LevelPreview);
		m_pLevels->GetNode(i)->SetCursorOutListener(this, LevelRevertPreview);
	}
	m_iLevelSelected = RandomInt(0, CDigitanksGame::GetNumLevels(GAMETYPE_ARTILLERY)-1);

	m_pLevelDescription = AddControl(new CLabel(0, 0, 32, 32, _T("")));
	m_pLevelDescription->SetWrap(true);
	m_pLevelDescription->SetFont(_T("text"));
	m_pLevelDescription->SetAlign(CLabel::TA_TOPLEFT);

	m_pDifficulty = AddControl(new CScrollSelector<int>(_T("text")));
	m_pDifficulty->AddSelection(CScrollSelection<int>(0, _T("Easy")));
	m_pDifficulty->AddSelection(CScrollSelection<int>(1, _T("Normal")));
	m_pDifficulty->SetSelection(1);

	m_pDifficultyLabel = AddControl(new CLabel(0, 0, 32, 32, _T("Difficulty")));
	m_pDifficultyLabel->SetWrap(false);
	m_pDifficultyLabel->SetFont(_T("text"));

	if (bMultiplayer)
	{
		m_pDifficulty->SetVisible(false);
		m_pDifficultyLabel->SetVisible(false);
	}

	if (bMultiplayer)
	{
		m_pBotPlayersLabel = NULL;
		m_pBotPlayers = NULL;
	}
	else
	{
		m_pBotPlayers = AddControl(new CScrollSelector<int>(_T("text")));

		m_pBotPlayersLabel = AddControl(new CLabel(0, 0, 32, 32, _T("Bot Players")));
		m_pBotPlayersLabel->SetWrap(false);
		m_pBotPlayersLabel->SetFont(_T("text"));
	}

	m_pTanks = AddControl(new CScrollSelector<int>(_T("text")));
	m_pTanks->AddSelection(CScrollSelection<int>(1, _T("1")));
	m_pTanks->AddSelection(CScrollSelection<int>(2, _T("2")));
	m_pTanks->AddSelection(CScrollSelection<int>(3, _T("3")));
	m_pTanks->SetSelection(2);
	m_pTanks->SetSelectedListener(this, TanksSelected);

	if (CGameLobbyClient::L_IsInLobby())
		CGameLobbyClient::S_UpdateLobby(_T("tanks"), tsprintf(tstring("%d"), m_pTanks->GetSelectionValue()));

	m_pTanksLabel = AddControl(new CLabel(0, 0, 32, 32, _T("Tanks Per Player")));
	m_pTanksLabel->SetWrap(false);
	m_pTanksLabel->SetFont(_T("text"));

	m_pTerrain = AddControl(new CScrollSelector<float>(_T("text")));
	m_pTerrain->AddSelection(CScrollSelection<float>(10, _T("Flatty")));
	m_pTerrain->AddSelection(CScrollSelection<float>(50, _T("Hilly")));
	m_pTerrain->AddSelection(CScrollSelection<float>(80, _T("Mountainy")));
	m_pTerrain->AddSelection(CScrollSelection<float>(120, _T("Everesty")));
	m_pTerrain->SetSelection(2);
	m_pTerrain->SetSelectedListener(this, TerrainSelected);

	if (CGameLobbyClient::L_IsInLobby())
		CGameLobbyClient::S_UpdateLobby(_T("terrain"), tsprintf(tstring("%.1f"), m_pTerrain->GetSelectionValue()));

	m_pTerrainLabel = AddControl(new CLabel(0, 0, 32, 32, _T("Terrain")));
	m_pTerrainLabel->SetWrap(false);
	m_pTerrainLabel->SetFont(_T("text"));

	if (bMultiplayer)
		m_pBeginGame = NULL;
	else
	{
		m_pBeginGame = AddControl(new CButton(0, 0, 100, 100, _T("BEGIN!")));
		m_pBeginGame->SetClickedListener(this, BeginGame);
		m_pBeginGame->SetFont(_T("header"), 12);
	}
}

CArtilleryGamePanel::~CArtilleryGamePanel()
{
	m_hLevelPreview.Reset();
}

void CArtilleryGamePanel::Layout()
{
	int iSelectorSize = m_pDifficultyLabel->GetHeight() - 4;

	m_pLevels->SetSize(200, 150);
	m_pLevels->SetPos(10, 10);

	m_pLevelDescription->SetSize(GetWidth()-40, 80);
	m_pLevelDescription->SetPos(20, 170);

	m_pDifficultyLabel->EnsureTextFits();
	m_pDifficultyLabel->SetPos(75, 250);

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 250);

	if (m_pBotPlayers)
	{
		m_pBotPlayersLabel->EnsureTextFits();
		m_pBotPlayersLabel->SetPos(75, 310);

		m_pBotPlayers->SetSize(GetWidth() - m_pBotPlayersLabel->GetLeft()*2 - m_pBotPlayersLabel->GetWidth(), iSelectorSize);
		m_pBotPlayers->SetPos(m_pBotPlayersLabel->GetRight(), 310);

		m_pBotPlayers->RemoveAllSelections();
		m_pBotPlayers->AddSelection(CScrollSelection<int>(1, _T("1")));
		m_pBotPlayers->AddSelection(CScrollSelection<int>(2, _T("2")));
		m_pBotPlayers->AddSelection(CScrollSelection<int>(3, _T("3")));
		m_pBotPlayers->AddSelection(CScrollSelection<int>(4, _T("4")));
		m_pBotPlayers->AddSelection(CScrollSelection<int>(5, _T("5")));
		m_pBotPlayers->AddSelection(CScrollSelection<int>(6, _T("6")));
		m_pBotPlayers->AddSelection(CScrollSelection<int>(7, _T("7")));
		m_pBotPlayers->SetSelection(m_pBotPlayers->GetNumSelections()/2);
	}

	m_pTanksLabel->EnsureTextFits();
	m_pTanksLabel->SetPos(75, 340);

	m_pTanks->SetSize(GetWidth() - m_pTanksLabel->GetLeft()*2 - m_pTanksLabel->GetWidth(), iSelectorSize);
	m_pTanks->SetPos(m_pTanksLabel->GetRight(), 340);

	m_pTerrainLabel->EnsureTextFits();
	m_pTerrainLabel->SetPos(75, 370);

	m_pTerrain->SetSize(GetWidth() - m_pTerrainLabel->GetLeft()*2 - m_pTerrainLabel->GetWidth(), iSelectorSize);
	m_pTerrain->SetPos(m_pTerrainLabel->GetRight(), 370);

	if (m_pBeginGame)
	{
		m_pBeginGame->SetSize(135, 40);
		m_pBeginGame->SetPos(GetWidth()/2-135/2, GetHeight()-60);
	}

	BaseClass::Layout();
}

void CArtilleryGamePanel::Paint(float x, float y, float w, float h)
{
	if (true)
	{
		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);

		float ax, ay;
		GetAbsPos(ax, ay);

		if (m_hLevelPreview.IsValid())
			CBaseControl::PaintTexture(m_hLevelPreview, ax + w - 160, ay + 10, 150, 150);
	}

	BaseClass::Paint(x, y, w, h);
}

void CArtilleryGamePanel::BeginGameCallback(const tstring& sArgs)
{
	CVar::SetCVar("game_players", 1);
	CVar::SetCVar("game_bots", m_pBotPlayers->GetSelectionValue());
	CVar::SetCVar("game_tanks", m_pTanks->GetSelectionValue());
	CVar::SetCVar("game_terrainheight", m_pTerrain->GetSelectionValue());
	CVar::SetCVar("game_difficulty", m_pDifficulty->GetSelectionValue());
	CVar::SetCVar(_T("game_level"), CDigitanksGame::GetLevel(GAMETYPE_ARTILLERY, m_iLevelSelected)->GetFile());

	DigitanksWindow()->CreateGame("artillery");

	if (!GameServer())
		return;

	CInstructor* pInstructor = DigitanksWindow()->GetInstructor();

	pInstructor->SetActive(true);
	pInstructor->Initialize();

	DigitanksWindow()->GetMainMenu()->SetVisible(false);
}

void CArtilleryGamePanel::UpdateLayoutCallback(const tstring& sArgs)
{
	Layout();
}

void CArtilleryGamePanel::LevelChosenCallback(const tstring& sArgs)
{
	size_t iMode = m_pLevels->GetSelectedNodeId();

	if (iMode >= CDigitanksGame::GetNumLevels(GAMETYPE_ARTILLERY))
		return;

	m_iLevelSelected = iMode;

	PreviewLevel(iMode);

	if (CGameLobbyClient::L_IsInLobby())
	{
		CGameLobbyClient::S_UpdateLobby(_T("level"), CDigitanksGame::GetLevel(GAMETYPE_ARTILLERY, m_iLevelSelected)->GetName());
		CGameLobbyClient::S_UpdateLobby(_T("level_file"), CDigitanksGame::GetLevel(GAMETYPE_ARTILLERY, m_iLevelSelected)->GetFile());
	}
}

void CArtilleryGamePanel::LevelPreviewCallback(const tstring& sArgs)
{
	size_t iMode = ~0;

	for (size_t i = 0; i < m_pLevels->GetControls().size(); i++)
	{
		float cx, cy, cw, ch;
		int mx, my;
		m_pLevels->GetControls()[i]->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::GetFullscreenMousePos(mx, my);
		if (mx >= cx &&
			my >= cy &&
			mx < cx + cw &&
			my < cy + ch)
		{
			iMode = i;
			break;
		}
	}

	if (iMode >= CDigitanksGame::GetNumLevels(GAMETYPE_ARTILLERY))
		return;

	PreviewLevel(iMode);
}

void CArtilleryGamePanel::LevelRevertPreviewCallback(const tstring& sArgs)
{
	PreviewLevel(m_iLevelSelected);
}

void CArtilleryGamePanel::TanksSelectedCallback(const tstring& sArgs)
{
	if (CGameLobbyClient::L_IsInLobby())
		CGameLobbyClient::S_UpdateLobby(_T("tanks"), tsprintf(tstring("%d"), m_pTanks->GetSelectionValue()));
}

void CArtilleryGamePanel::TerrainSelectedCallback(const tstring& sArgs)
{
	if (CGameLobbyClient::L_IsInLobby())
		CGameLobbyClient::S_UpdateLobby(_T("terrain"), tsprintf(tstring("%.1f"), m_pTerrain->GetSelectionValue()));
}

void CArtilleryGamePanel::PreviewLevel(size_t iLevel)
{
	if (m_hLevelPreview.IsValid())
		m_hLevelPreview.Reset();

	if (iLevel == ~0)
	{
		m_pLevelDescription->SetText("");
		return;
	}

	CDigitanksLevel* pLevel = CDigitanksGame::GetLevel(GAMETYPE_ARTILLERY, iLevel);

	Color clrPreview[256*256];

	int w, h;

	Color* pclrHeight = CRenderer::LoadTextureData(pLevel->GetTerrainHeightImage().GetName(), w, h);

	TAssert(w == TERRAIN_SIZE);
	TAssert(h == TERRAIN_SIZE);

	Color* pclrData = CRenderer::LoadTextureData(pLevel->GetTerrainDataImage().GetName(), w, h);

	TAssert(w == TERRAIN_SIZE);
	TAssert(h == TERRAIN_SIZE);

	for (size_t i = 0; i < 256; i++)
	{
		for (size_t j = 0; j < 256; j++)
		{
			size_t c = i*256+j;

			if (pclrHeight && pclrData)
			{
				Vector vecData = pclrData[c];
				Vector vecHeight = pclrHeight[c];
				if (vecData.x > 0.4f || vecData.y > 0.4f || vecData.z > 0.4f)
					clrPreview[c] = vecData * (vecHeight/2 + Vector(0.5f, 0.5f, 0.5));
				else
					clrPreview[c] = (vecHeight/2 + Vector(0.5f, 0.5f, 0.5));
				clrPreview[c].SetAlpha(pclrData[c].a() > 128?255:0);
			}
			else if (pclrHeight)
				clrPreview[c] = (Vector(pclrHeight[c])/2 + Vector(0.5f, 0.5f, 0.5));
			else if (pclrData)
			{
				clrPreview[c] = pclrData[c];
				clrPreview[c].SetAlpha(pclrData[c].a() > 128?255:0);
			}
			else
				clrPreview[c] = Color(0,0,0);
		}
	}

	std::shared_ptr<CData> pMaterial(new CData("Shader", "gui"));
	m_hLevelPreview = CMaterialLibrary::AddMaterial(pMaterial.get());

	m_pLevelDescription->SetText(tstring(_T("Author: ")) + pLevel->GetAuthor() + _T("\n \nDescription: ") + pLevel->GetDescription());

	m_pTerrain->SetVisible(pLevel->GetTerrainHeight().length() == 0);
	m_pTerrainLabel->SetVisible(pLevel->GetTerrainHeight().length() == 0);

	Layout();
}

CStrategyGamePanel::CStrategyGamePanel(bool bMultiplayer)
	: CPanel(0, 0, 570, 520)
{
	m_pLevels = AddControl(new CTree());
	m_pLevels->SetSelectedListener(this, LevelChosen);

	for (size_t i = 0; i < CDigitanksGame::GetNumLevels(GAMETYPE_STANDARD); i++)
	{
		CLevel* pLevel = CDigitanksGame::GetLevel(GAMETYPE_STANDARD, i);
		m_pLevels->AddNode(pLevel->GetName());
		m_pLevels->GetNode(i)->SetCursorInListener(this, LevelPreview);
		m_pLevels->GetNode(i)->SetCursorOutListener(this, LevelRevertPreview);
	}
	m_iLevelSelected = RandomInt(0, CDigitanksGame::GetNumLevels(GAMETYPE_STANDARD)-1);

	m_pLevelDescription = AddControl(new CLabel(0, 0, 32, 32, _T("")));
	m_pLevelDescription->SetWrap(true);
	m_pLevelDescription->SetFont(_T("text"));
	m_pLevelDescription->SetAlign(CLabel::TA_TOPLEFT);

	m_pDifficulty = AddControl(new CScrollSelector<int>(_T("text")));
	m_pDifficulty->AddSelection(CScrollSelection<int>(0, _T("Easy")));
	m_pDifficulty->AddSelection(CScrollSelection<int>(1, _T("Normal")));
	m_pDifficulty->SetSelection(1);

	m_pDifficultyLabel = AddControl(new CLabel(0, 0, 32, 32, _T("Difficulty")));
	m_pDifficultyLabel->SetWrap(false);
	m_pDifficultyLabel->SetFont(_T("text"));

	if (bMultiplayer)
	{
		m_pDifficulty->SetVisible(false);
		m_pDifficultyLabel->SetVisible(false);
	}

	if (bMultiplayer)
	{
		m_pBotPlayersLabel = NULL;
		m_pBotPlayers = NULL;
	}
	else
	{
		m_pBotPlayers = AddControl(new CScrollSelector<int>(_T("text")));

		m_pBotPlayersLabel = AddControl(new CLabel(0, 0, 32, 32, _T("Bot Players")));
		m_pBotPlayersLabel->SetWrap(false);
		m_pBotPlayersLabel->SetFont(_T("text"));
	}

	if (bMultiplayer)
		m_pBeginGame = NULL;
	else
	{
		m_pBeginGame = AddControl(new CButton(0, 0, 100, 100, _T("BEGIN!")));
		m_pBeginGame->SetClickedListener(this, BeginGame);
		m_pBeginGame->SetFont(_T("header"), 12);
	}
}

CStrategyGamePanel::~CStrategyGamePanel()
{
	m_hLevelPreview.Reset();
}

void CStrategyGamePanel::Layout()
{
	int iSelectorSize = m_pDifficultyLabel->GetHeight() - 4;

	m_pLevels->SetSize(200, 150);
	m_pLevels->SetPos(10, 10);

	m_pLevelDescription->SetSize(GetWidth()-40, 80);
	m_pLevelDescription->SetPos(20, 170);

	m_pDifficultyLabel->EnsureTextFits();
	m_pDifficultyLabel->SetPos(75, 250);

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 250);

	if (m_pBotPlayers)
	{
		m_pBotPlayersLabel->EnsureTextFits();
		m_pBotPlayersLabel->SetPos(75, 280);
		m_pBotPlayersLabel->SetVisible(true);

		m_pBotPlayers->SetSize(GetWidth() - m_pBotPlayersLabel->GetLeft()*2 - m_pBotPlayersLabel->GetWidth(), iSelectorSize);
		m_pBotPlayers->SetPos(m_pBotPlayersLabel->GetRight(), 280);
		m_pBotPlayers->SetVisible(true);

		m_pBotPlayers->RemoveAllSelections();
		m_pBotPlayers->AddSelection(CScrollSelection<int>(1, _T("1")));
		m_pBotPlayers->AddSelection(CScrollSelection<int>(2, _T("2")));
		m_pBotPlayers->AddSelection(CScrollSelection<int>(3, _T("3")));
		m_pBotPlayers->SetSelection(m_pBotPlayers->GetNumSelections()-1);
	}

	if (m_pBeginGame)
	{
		m_pBeginGame->SetSize(135, 40);
		m_pBeginGame->SetPos(GetWidth()/2-135/2, GetHeight()-60);
	}

	BaseClass::Layout();
}

void CStrategyGamePanel::Paint(float x, float y, float w, float h)
{
	if (true)
	{
		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);

		float ax, ay;
		GetAbsPos(ax, ay);

		if (m_hLevelPreview.IsValid())
			CBaseControl::PaintTexture(m_hLevelPreview, ax + w - 160, ay + 10, 150, 150);
	}

	BaseClass::Paint(x, y, w, h);
}

void CStrategyGamePanel::BeginGameCallback(const tstring& sArgs)
{
	CVar::SetCVar("game_players", 1);
	CVar::SetCVar("game_bots", m_pBotPlayers->GetSelectionValue());
	CVar::SetCVar("game_difficulty", m_pDifficulty->GetSelectionValue());
	CVar::SetCVar(_T("game_level"), CDigitanksGame::GetLevel(GAMETYPE_STANDARD, m_iLevelSelected)->GetFile());

	DigitanksWindow()->CreateGame("standard");

	if (!GameServer())
		return;

	CInstructor* pInstructor = DigitanksWindow()->GetInstructor();

	pInstructor->SetActive(false);

	DigitanksWindow()->GetMainMenu()->SetVisible(false);
}

void CStrategyGamePanel::UpdateLayoutCallback(const tstring& sArgs)
{
	Layout();
}

void CStrategyGamePanel::LevelChosenCallback(const tstring& sArgs)
{
	size_t iMode = m_pLevels->GetSelectedNodeId();

	if (iMode >= CDigitanksGame::GetNumLevels(GAMETYPE_STANDARD))
		return;

	m_iLevelSelected = iMode;

	PreviewLevel(iMode);

	if (CGameLobbyClient::L_IsInLobby())
	{
		CGameLobbyClient::S_UpdateLobby(_T("level"), CDigitanksGame::GetLevel(GAMETYPE_STANDARD, m_iLevelSelected)->GetName());
		CGameLobbyClient::S_UpdateLobby(_T("level_file"), CDigitanksGame::GetLevel(GAMETYPE_STANDARD, m_iLevelSelected)->GetFile());
	}
}

void CStrategyGamePanel::LevelPreviewCallback(const tstring& sArgs)
{
	size_t iMode = ~0;

	for (size_t i = 0; i < m_pLevels->GetControls().size(); i++)
	{
		float cx, cy, cw, ch;
		int mx, my;
		m_pLevels->GetControls()[i]->GetAbsDimensions(cx, cy, cw, ch);
		CRootPanel::GetFullscreenMousePos(mx, my);
		if (mx >= cx &&
			my >= cy &&
			mx < cx + cw &&
			my < cy + ch)
		{
			iMode = i;
			break;
		}
	}

	if (iMode >= CDigitanksGame::GetNumLevels(GAMETYPE_STANDARD))
		return;

	PreviewLevel(iMode);
}

void CStrategyGamePanel::LevelRevertPreviewCallback(const tstring& sArgs)
{
	PreviewLevel(m_iLevelSelected);
}

void CStrategyGamePanel::PreviewLevel(size_t iLevel)
{
	m_hLevelPreview.Reset();

	if (iLevel == ~0)
	{
		m_pLevelDescription->SetText("");
		return;
	}

	CDigitanksLevel* pLevel = CDigitanksGame::GetLevel(GAMETYPE_STANDARD, iLevel);

	Color clrPreview[256*256];

	int w, h;

	Color* pclrHeight = CRenderer::LoadTextureData(pLevel->GetTerrainHeightImage().GetName(), w, h);

	if (pclrHeight)
	{
		TAssert(w == TERRAIN_SIZE);
		TAssert(h == TERRAIN_SIZE);
	}

	Color* pclrData = CRenderer::LoadTextureData(pLevel->GetTerrainDataImage().GetName(), w, h);

	if (pclrData)
	{
		TAssert(w == TERRAIN_SIZE);
		TAssert(h == TERRAIN_SIZE);
	}

	for (size_t i = 0; i < 256; i++)
	{
		for (size_t j = 0; j < 256; j++)
		{
			size_t c = i*256+j;

			if (pclrHeight && pclrData)
			{
				Vector vecData = pclrData[c];
				Vector vecHeight = pclrHeight[c];
				if (vecData.x > 0.5f || vecData.y > 0.5f || vecData.z > 0.5f)
					clrPreview[c] = vecData * (vecHeight/2 + Vector(0.5f, 0.5f, 0.5));
				else
					clrPreview[c] = (vecHeight/2 + Vector(0.5f, 0.5f, 0.5));
				clrPreview[c].SetAlpha(pclrData[c].a() > 128?255:0);
			}
			else if (pclrHeight)
				clrPreview[c] = (Vector(pclrHeight[c])/2 + Vector(0.5f, 0.5f, 0.5));
			else if (pclrData)
			{
				clrPreview[c] = pclrData[c];
				clrPreview[c].SetAlpha(pclrData[c].a() > 128?255:0);
			}
			else
				clrPreview[c] = Color(0,0,0);
		}
	}

	std::shared_ptr<CData> pMaterial(new CData("Shader", "gui"));
	m_hLevelPreview = CMaterialLibrary::AddMaterial(pMaterial.get());

	m_pLevelDescription->SetText(tstring(_T("Author: ")) + pLevel->GetAuthor() + _T("\n \nDescription: ") + pLevel->GetDescription());

	Layout();
}

#if 0
// HOLY CRAP A GLOBAL! Yeah it's bad. Sue me.
tvector<GLFWvidmode> g_aVideoModes;
GLFWvidmode g_aModes[ 100 ];
#endif

COptionsPanel::COptionsPanel()
	: CPanel(0, 0, 570, 520)
{
	m_bStandalone = false;

	m_pNicknameLabel = AddControl(new CLabel(0, 0, 32, 32, _T("Nickname:")));
	m_pNicknameLabel->SetWrap(false);
	m_pNicknameLabel->SetFont(_T("text"));

	m_pNickname = AddControl(new CTextField());
	m_pNickname->SetContentsChangedListener(this, NewNickname);

	m_pSoundVolume = AddControl(new CScrollSelector<float>(_T("text")));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0, _T("Off")));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.1f, _T("10%")));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.2f, _T("20%")));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.3f, _T("30%")));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.4f, _T("40%")));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.5f, _T("50%")));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.6f, _T("60%")));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.7f, _T("70%")));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.8f, _T("80%")));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.9f, _T("90%")));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(1.0f, _T("100%")));
	m_pSoundVolume->SetSelection(1);
	m_pSoundVolume->SetSelectedListener(this, SoundVolumeChanged);

	m_pSoundVolumeLabel = AddControl(new CLabel(0, 0, 32, 32, _T("Sound Volume")));
	m_pSoundVolumeLabel->SetWrap(false);
	m_pSoundVolumeLabel->SetFont(_T("text"));

	m_pMusicVolume = AddControl(new CScrollSelector<float>(_T("text")));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0, _T("Off")));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.1f, _T("10%")));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.2f, _T("20%")));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.3f, _T("30%")));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.4f, _T("40%")));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.5f, _T("50%")));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.6f, _T("60%")));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.7f, _T("70%")));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.8f, _T("80%")));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.9f, _T("90%")));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(1.0f, _T("100%")));
	m_pMusicVolume->SetSelection(1);
	m_pMusicVolume->SetSelectedListener(this, MusicVolumeChanged);

	m_pMusicVolumeLabel = AddControl(new CLabel(0, 0, 32, 32, _T("Music Volume")));
	m_pMusicVolumeLabel->SetWrap(false);
	m_pMusicVolumeLabel->SetFont(_T("text"));

	m_pVideoChangedNotice = AddControl(new CLabel(0, 0, 32, 32, _T("Changes to the video settings will take effect after the game has been restarted.")));
	m_pVideoChangedNotice->SetVisible(false);
	m_pVideoChangedNotice->SetFont(_T("text"));

	m_pWindowed = AddControl(new CCheckBox());
	m_pWindowed->SetClickedListener(this, WindowedChanged);
	m_pWindowed->SetUnclickedListener(this, WindowedChanged);

	m_pWindowedLabel = AddControl(new CLabel(0, 0, 100, 100, _T("Run in a window")));
	m_pWindowedLabel->SetFont(_T("text"));

	m_pVideoModes = AddControl(new CMenu(_T("Change Resolution")));
	m_pVideoModes->SetFont(_T("text"));

    int iModes;

	TStubbed("Video modes");
#if 0
	g_aVideoModes.clear();

	iModes = glfwGetVideoModes( g_aModes, 100 );
    for( int i = 0; i < iModes; i ++ )
    {
		if (g_aModes[i].Width < 1024)
			continue;

		if (g_aModes[i].Height < 768)
			continue;

		if (g_aModes[i].BlueBits < 8)
			continue;

		tstring sMode;
		sMode = tsprintf(tstring("%dx%d"), g_aModes[i].Width, g_aModes[i].Height);
		m_pVideoModes->AddSubmenu(sMode, this, VideoModeChosen);
		g_aVideoModes.push_back(g_aModes[i]);
	}
#endif

	m_pConstrain = AddControl(new CCheckBox());
	m_pConstrain->SetClickedListener(this, ConstrainChanged);
	m_pConstrain->SetUnclickedListener(this, ConstrainChanged);

	m_pConstrainLabel = AddControl(new CLabel(0, 0, 100, 100, _T("Constrain mouse to screen edges")));
	m_pConstrainLabel->SetFont(_T("text"));

	m_pContextual = AddControl(new CCheckBox());
	m_pContextual->SetClickedListener(this, ContextualChanged);
	m_pContextual->SetUnclickedListener(this, ContextualChanged);

	m_pContextualLabel = AddControl(new CLabel(0, 0, 100, 100, _T("Contextual mouse commands")));
	m_pContextualLabel->SetFont(_T("text"));

	m_pReverseSpacebar = AddControl(new CCheckBox());
	m_pReverseSpacebar->SetClickedListener(this, ReverseSpacebarChanged);
	m_pReverseSpacebar->SetUnclickedListener(this, ReverseSpacebarChanged);

	m_pReverseSpacebarLabel = AddControl(new CLabel(0, 0, 100, 100, _T("Reverse spacebar drag")));
	m_pReverseSpacebarLabel->SetFont(_T("text"));

	m_pClose = AddControl(new CButton(0, 0, 100, 100, _T("X")));
	m_pClose->SetClickedListener(this, Close);
}

void COptionsPanel::Layout()
{
	if (m_bStandalone)
	{
		SetSize(570, 520);
		SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, CRootPanel::Get()->GetHeight()/2 - GetHeight()/2);
	}

	int iSelectorSize = m_pMusicVolumeLabel->GetHeight() - 4;

	m_pNicknameLabel->SetPos(GetWidth()/2-m_pNicknameLabel->GetWidth()/2, 50);
	m_pNickname->SetPos(GetWidth()/2-m_pNickname->GetWidth()/2, 80);
	m_pNickname->SetText(DigitanksWindow()->GetPlayerNickname());

	m_pSoundVolumeLabel->EnsureTextFits();
	m_pSoundVolumeLabel->SetPos(75, 160);

	m_pSoundVolume->SetSize(GetWidth() - m_pSoundVolumeLabel->GetLeft()*2 - m_pSoundVolumeLabel->GetWidth(), iSelectorSize);
	m_pSoundVolume->SetPos(m_pSoundVolumeLabel->GetRight(), 160);

	m_pSoundVolume->SetSelection((size_t)(DigitanksWindow()->GetSoundVolume()*10));

	m_pMusicVolumeLabel->EnsureTextFits();
	m_pMusicVolumeLabel->SetPos(75, 200);

	m_pMusicVolume->SetSize(GetWidth() - m_pMusicVolumeLabel->GetLeft()*2 - m_pMusicVolumeLabel->GetWidth(), iSelectorSize);
	m_pMusicVolume->SetPos(m_pMusicVolumeLabel->GetRight(), 200);

	m_pMusicVolume->SetSelection((size_t)(DigitanksWindow()->GetMusicVolume()*10));

	m_pVideoChangedNotice->SetSize(GetWidth()-50, 30);
	m_pVideoChangedNotice->SetPos(25, GetHeight()-290);

	m_pVideoModes->SetSize(120, 30);
	m_pVideoModes->SetPos(GetWidth()/2 - m_pVideoModes->GetWidth() - 40, GetHeight()-230);

	tstring sVideoMode;
	sVideoMode = tsprintf(tstring("%dx%d"), DigitanksWindow()->GetWindowWidth(), DigitanksWindow()->GetWindowHeight());
	m_pVideoModes->SetText(sVideoMode);

	m_pWindowedLabel->SetWrap(false);
	m_pWindowedLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pWindowedLabel->SetSize(10, 10);
	m_pWindowedLabel->EnsureTextFits();
	m_pWindowedLabel->SetPos(GetWidth()/2 - m_pWindowedLabel->GetWidth()/2 + 10 + 40, GetHeight()-230);
	m_pWindowed->SetPos(m_pWindowedLabel->GetLeft() - 15, GetHeight()-230 + m_pWindowedLabel->GetHeight()/2 - m_pWindowed->GetHeight()/2);
	m_pWindowed->SetState(!DigitanksWindow()->IsFullscreen(), false);

	m_pConstrainLabel->SetWrap(false);
	m_pConstrainLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pConstrainLabel->SetSize(10, 10);
	m_pConstrainLabel->EnsureTextFits();
	m_pConstrainLabel->SetPos(GetWidth()/2 - m_pConstrainLabel->GetWidth()/2 + 10 + 40, GetHeight()-130);
	m_pConstrain->SetPos(m_pConstrainLabel->GetLeft() - 15, GetHeight()-130 + m_pConstrainLabel->GetHeight()/2 - m_pConstrain->GetHeight()/2);
	m_pConstrain->SetState(DigitanksWindow()->WantsConstrainMouse(), false);

	m_pContextualLabel->SetWrap(false);
	m_pContextualLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pContextualLabel->SetSize(10, 10);
	m_pContextualLabel->EnsureTextFits();
	m_pContextualLabel->SetPos(GetWidth()/2 - m_pContextualLabel->GetWidth()/2 + 10 + 40, GetHeight()-100);
	m_pContextual->SetPos(m_pContextualLabel->GetLeft() - 15, GetHeight()-100 + m_pContextualLabel->GetHeight()/2 - m_pContextual->GetHeight()/2);
	m_pContextual->SetState(DigitanksWindow()->WantsContextualCommands(), false);

	m_pReverseSpacebarLabel->SetWrap(false);
	m_pReverseSpacebarLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pReverseSpacebarLabel->SetSize(10, 10);
	m_pReverseSpacebarLabel->EnsureTextFits();
	m_pReverseSpacebarLabel->SetPos(GetWidth()/2 - m_pReverseSpacebarLabel->GetWidth()/2 + 10 + 40, GetHeight()-70);
	m_pReverseSpacebar->SetPos(m_pReverseSpacebarLabel->GetLeft() - 15, GetHeight()-70 + m_pReverseSpacebarLabel->GetHeight()/2 - m_pReverseSpacebar->GetHeight()/2);
	m_pReverseSpacebar->SetState(DigitanksWindow()->ShouldReverseSpacebar(), false);

	BaseClass::Layout();

	m_pClose->SetPos(GetWidth()-20, 10);
	m_pClose->SetSize(10, 10);
	m_pClose->SetButtonColor(Color(255, 0, 0));
	m_pClose->SetVisible(m_bStandalone);
}

void COptionsPanel::Paint(float x, float y, float w, float h)
{
	if (m_bStandalone)
		CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 255));

	BaseClass::Paint(x, y, w, h);
}

void COptionsPanel::NewNicknameCallback(const tstring& sArgs)
{
	DigitanksWindow()->SetPlayerNickname(m_pNickname->GetText());
}

void COptionsPanel::SoundVolumeChangedCallback(const tstring& sArgs)
{
	DigitanksWindow()->SetSoundVolume(m_pSoundVolume->GetSelectionValue());
	DigitanksWindow()->SaveConfig();
}

void COptionsPanel::MusicVolumeChangedCallback(const tstring& sArgs)
{
	DigitanksWindow()->SetMusicVolume(m_pMusicVolume->GetSelectionValue());
	DigitanksWindow()->SaveConfig();
}

void COptionsPanel::VideoModeChosenCallback(const tstring& sArgs)
{
#if 0
	size_t iMode = m_pVideoModes->GetSelectedMenu();

	if (iMode >= g_aVideoModes.size())
		return;

	int iHeight = g_aVideoModes[iMode].Height;
	int iWidth = g_aVideoModes[iMode].Width;
	DigitanksWindow()->SetConfigWindowDimensions(iWidth, iHeight);
	DigitanksWindow()->SaveConfig();

	m_pVideoChangedNotice->SetVisible(true);

	m_pVideoModes->Pop(true, true);
#endif
}

void COptionsPanel::WindowedChangedCallback(const tstring& sArgs)
{
	DigitanksWindow()->SetConfigFullscreen(!m_pWindowed->GetState());
	DigitanksWindow()->SaveConfig();

	m_pVideoChangedNotice->SetVisible(true);
}

void COptionsPanel::ConstrainChangedCallback(const tstring& sArgs)
{
	DigitanksWindow()->SetConstrainMouse(m_pConstrain->GetState());
	DigitanksWindow()->SaveConfig();
}

void COptionsPanel::ContextualChangedCallback(const tstring& sArgs)
{
	DigitanksWindow()->SetContextualCommands(m_pContextual->GetState());
	DigitanksWindow()->SaveConfig();
}

void COptionsPanel::ReverseSpacebarChangedCallback(const tstring& sArgs)
{
	DigitanksWindow()->SetReverseSpacebar(m_pReverseSpacebar->GetState());
	DigitanksWindow()->SaveConfig();
}

void COptionsPanel::CloseCallback(const tstring& sArgs)
{
	SetVisible(false);
}
