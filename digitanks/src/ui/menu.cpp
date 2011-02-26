#include "menu.h"

#include <GL/glfw.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <platform.h>
#include <strutils.h>

#include <dt_version.h>
#include <digitanks/digitanksgame.h>
#include <renderer/renderer.h>
#include <game/digitanks/digitankslevel.h>

#include "instructor.h"
#include "digitankswindow.h"
#include "hud.h"

using namespace glgui;

CMainMenu::CMainMenu()
	: CPanel(0, 0, 310, 620)
{
	m_pTutorial = new CButton(0, 0, 100, 100, L"TUTORIALS");
	m_pTutorial->SetClickedListener(this, OpenTutorialsPanel);
	m_pTutorial->SetFont(L"header", 28);
	m_pTutorial->SetButtonColor(Color(0,0,0));
	AddControl(m_pTutorial);

	m_pPlay = new CButton(0, 0, 100, 100, L"NEW GAME");
	m_pPlay->SetClickedListener(this, OpenGamesPanel);
	m_pPlay->SetFont(L"header", 28);
	m_pPlay->SetButtonColor(Color(0,0,0));
	AddControl(m_pPlay);

	m_pMultiplayer = new CButton(0, 0, 100, 100, L"MULTIPLAYER");
	m_pMultiplayer->SetClickedListener(this, OpenMultiplayerPanel);
	m_pMultiplayer->SetFont(L"header", 28);
	m_pMultiplayer->SetButtonColor(Color(0,0,0));
#if !defined(TINKER_UNLOCKED)
	m_pMultiplayer->SetEnabled(false);
#endif
	AddControl(m_pMultiplayer);

	m_pOptions = new CButton(0, 0, 100, 100, L"OPTIONS");
	m_pOptions->SetClickedListener(this, OpenOptionsPanel);
	m_pOptions->SetFont(L"header", 28);
	m_pOptions->SetButtonColor(Color(0,0,0));
	AddControl(m_pOptions);

	m_pQuit = new CButton(0, 0, 100, 100, L"QUIT");
	m_pQuit->SetClickedListener(this, Quit);
	m_pQuit->SetFont(L"header", 28);
	m_pQuit->SetButtonColor(Color(0,0,0));
	AddControl(m_pQuit);

	m_pHint = new CLabel(0, 0, 100, 100, L"");
	m_pHint->SetFont(L"text");
	AddControl(m_pHint);

	m_pShowCredits = new CButton(0, 0, 100, 100, L"CREDITS");
	m_pShowCredits->SetClickedListener(this, Credits);
	m_pShowCredits->SetFont(L"header", 9);
	m_pShowCredits->SetButtonColor(Color(0,0,0));
	AddControl(m_pShowCredits);

	m_pCredits = new CLabel(0, 0, 100, 100, L"");
	m_pCredits->SetFont(L"text", 18);
	m_pCredits->SetAlign(CLabel::TA_TOPCENTER);
	AddControl(m_pCredits);

	m_pVersion = new CLabel(0, 0, 100, 100, L"");
	m_pVersion->SetFont(L"text", 11);
	m_pVersion->SetAlign(CLabel::TA_LEFTCENTER);
	AddControl(m_pVersion);

	m_pDockPanel = NULL;

	m_iLunarWorkshop = CRenderer::LoadTextureIntoGL(L"textures/lunar-workshop.png");
}

void CMainMenu::Layout()
{
	SetPos(40, 130);

	m_pTutorial->SetPos(20, 120);
	m_pTutorial->SetSize(270, 80);

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
	int x, y;
	GetAbsPos(x, y);
	m_pCredits->SetPos(370 - x, (int)(DigitanksWindow()->GetWindowHeight() - y - m_flCreditsRoll));
}

void CMainMenu::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(0, 0, m_pVersion->GetWidth(), m_pVersion->GetHeight(), Color(0, 0, 0, 100));

	int hx, hy;
	m_pHint->GetAbsPos(hx, hy);
	if (m_pHint->GetText().length() > 1)
		CRootPanel::PaintRect(hx-25, hy-3, m_pHint->GetWidth()+50, m_pHint->GetHeight()+6, Color(0, 0, 0, 255));

	if (m_pCredits->IsVisible())
	{
		int cx, cy;
		m_pCredits->GetAbsPos(cx, cy);

		CRootPanel::PaintRect(cx-5, 0, m_pCredits->GetWidth()+10, CRootPanel::Get()->GetHeight(), Color(0, 0, 0, 100));
	}

	BaseClass::Paint(x, y, w, h);

	if (DigitanksGame() && DigitanksGame()->GetGameType() == GAMETYPE_MENU)
	{
		CRenderingContext c(GameServer()->GetRenderer());

		c.SetBlend(BLEND_ALPHA);
		CHUD::PaintHUDSheet(20, 20, 350, 730, 0, 0, 350, 730);
		if (!m_pCredits->IsVisible())
			CRootPanel::PaintTexture(m_iLunarWorkshop, CRootPanel::Get()->GetWidth()-200-20, CRootPanel::Get()->GetHeight()-200, 200, 200);
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

void CMainMenu::OpenTutorialsPanelCallback()
{
	CDockPanel* pDock = GetDockPanel();
	pDock->SetDockedPanel(new CTutorialsPanel());
	pDock->SetVisible(true);

	m_pCredits->SetVisible(false);
}

void CMainMenu::OpenGamesPanelCallback()
{
	CDockPanel* pDock = GetDockPanel();
	pDock->SetDockedPanel(new CGamesPanel());
	pDock->SetVisible(true);

	m_pCredits->SetVisible(false);
}

void CMainMenu::OpenMultiplayerPanelCallback()
{
#if !defined(TINKER_UNLOCKED)
	return;
#endif

	CDockPanel* pDock = GetDockPanel();
	pDock->SetDockedPanel(new CMultiplayerPanel());
	pDock->SetVisible(true);

	m_pCredits->SetVisible(false);
}

void CMainMenu::OpenOptionsPanelCallback()
{
	CDockPanel* pDock = GetDockPanel();
	pDock->SetDockedPanel(new COptionsPanel());
	pDock->SetVisible(true);

	m_pCredits->SetVisible(false);
}

void CMainMenu::QuitCallback()
{
	DigitanksWindow()->CloseApplication();
}

void CMainMenu::CreditsCallback()
{
	std::wifstream i;
	i.open("credits.txt");
	std::wstring sCredits;
	if (i.is_open())
	{
		while (i.good())
		{
			std::wstring sLine;
			getline(i, sLine);
			if (sLine.length())
				sCredits.append(sLine);
			else
				sCredits.append(L" ");	// The text renderer skips empty lines
			sCredits.append(L"\n");
		}
	}

	m_pCredits->SetText(sCredits.c_str());

	m_pCredits->SetSize(DigitanksWindow()->GetWindowWidth()-m_pCredits->GetLeft()-40, 9999);
	m_pCredits->SetVisible(true);
	m_flCreditsRoll = 0;

	GetDockPanel()->SetVisible(false);
	m_pHint->SetText("");
}

CDockPanel* CMainMenu::GetDockPanel()
{
	if (!m_pDockPanel)
	{
		m_pDockPanel = new CDockPanel();
		m_pDockPanel->SetBGColor(Color(0, 0, 0, 255));
		CRootPanel::Get()->AddControl(m_pDockPanel);

		m_pDockPanel->SetSize(570, 520);
		m_pDockPanel->SetPos(390, 30);
	}

	return m_pDockPanel;
}

void CMainMenu::SetHint(const eastl::string16& s)
{
	m_pHint->SetText(s);
}

CDockPanel::CDockPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pDockedPanel = NULL;
}

void CDockPanel::Destructor()
{
	if (m_pDockedPanel)
		delete m_pDockedPanel;
}

void CDockPanel::Layout()
{
	BaseClass::Layout();
}

void CDockPanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h, m_clrBackground);

	BaseClass::Paint(x, y, w, h);
}

void CDockPanel::SetDockedPanel(glgui::CPanel* pDock)
{
	if (m_pDockedPanel)
	{
		RemoveControl(m_pDockedPanel);
		m_pDockedPanel->Destructor();
		m_pDockedPanel->Delete();
	}

	if (pDock)
	{
		m_pDockedPanel = pDock;
		AddControl(m_pDockedPanel);
		m_pDockedPanel->SetPos(0, 0);
		m_pDockedPanel->SetSize(GetWidth(), GetHeight());
		m_pDockedPanel->Layout();
	}
}

CTutorialsPanel::CTutorialsPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pBasics = new CButton(0, 0, 100, 100, L"BASICS");
	m_pBasics->SetClickedListener(this, Basics);
	m_pBasics->SetCursorInListener(this, BasicsHint);
	m_pBasics->SetFont(L"header", 18);
	AddControl(m_pBasics);

	m_pBases = new CButton(0, 0, 100, 100, L"BASE BUILDING");
	m_pBases->SetClickedListener(this, Bases);
	m_pBases->SetCursorInListener(this, BasesHint);
	m_pBases->SetFont(L"header", 18);
	AddControl(m_pBases);

	m_pUnits = new CButton(0, 0, 100, 100, L"UNIT OVERVIEW");
	m_pUnits->SetClickedListener(this, Units);
	m_pUnits->SetCursorInListener(this, UnitsHint);
	m_pUnits->SetFont(L"header", 18);
	AddControl(m_pUnits);
}

void CTutorialsPanel::Layout()
{
	m_pBasics->SetSize(300, 40);
	m_pBasics->SetPos(GetWidth()/2-m_pBasics->GetWidth()/2, 60);

	m_pBases->SetSize(300, 40);
	m_pBases->SetPos(GetWidth()/2-m_pBasics->GetWidth()/2, 120);

	m_pUnits->SetSize(300, 40);
	m_pUnits->SetPos(GetWidth()/2-m_pBasics->GetWidth()/2, 180);

	BaseClass::Layout();
}

void CTutorialsPanel::BasicsCallback()
{
	CInstructor* pInstructor = DigitanksWindow()->GetInstructor();

	pInstructor->SetActive(true);
	pInstructor->Initialize();

	DigitanksWindow()->SetServerType(SERVER_LOCAL);
	DigitanksWindow()->CreateGame(GAMETYPE_TUTORIAL);
	DigitanksGame()->SetDifficulty(0);

	pInstructor->DisplayFirstBasicsTutorial();

	DigitanksWindow()->GetMainMenu()->SetVisible(false);
}

void CTutorialsPanel::BasesCallback()
{
	CInstructor* pInstructor = DigitanksWindow()->GetInstructor();

	pInstructor->SetActive(true);
	pInstructor->Initialize();

	DigitanksWindow()->SetServerType(SERVER_LOCAL);
	DigitanksWindow()->CreateGame(GAMETYPE_TUTORIAL);
	DigitanksGame()->SetDifficulty(0);

	pInstructor->DisplayFirstBasesTutorial();

	DigitanksWindow()->GetMainMenu()->SetVisible(false);
}

void CTutorialsPanel::UnitsCallback()
{
	CInstructor* pInstructor = DigitanksWindow()->GetInstructor();

	pInstructor->SetActive(true);
	pInstructor->Initialize();

	DigitanksWindow()->SetServerType(SERVER_LOCAL);
	DigitanksWindow()->CreateGame(GAMETYPE_TUTORIAL);
	DigitanksGame()->SetDifficulty(0);

	pInstructor->DisplayFirstUnitsTutorial();

	DigitanksWindow()->GetMainMenu()->SetVisible(false);
}

void CTutorialsPanel::BasicsHintCallback()
{
	DigitanksWindow()->GetMainMenu()->SetHint(L"Learn the basics of Digitanks. This tutorial includes view control and basic tank manipulation. After this tutorial you should know enough to play an Artillery game.");
}

void CTutorialsPanel::BasesHintCallback()
{
	DigitanksWindow()->GetMainMenu()->SetHint(L"Learn how to set up a base. In this tutorial you'll learn how to construct buildings and produce units. It's a good idea to play through this tutorial before beginning Strategy mode.");
}

void CTutorialsPanel::UnitsHintCallback()
{
	DigitanksWindow()->GetMainMenu()->SetHint(L"Learn the special units. In this tutorial you'll learn how to use the Resistors, Artillery and Rogues to their highest effectiveness. It's a good idea to play through this tutorial before beginning Strategy mode.");
}

CGamesPanel::CGamesPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pArtillery = new CButton(0, 0, 100, 100, L"ARTILLERY");
	m_pArtillery->SetClickedListener(this, Artillery);
	m_pArtillery->SetCursorInListener(this, ArtilleryHint);
	m_pArtillery->SetFont(L"header", 18);
	AddControl(m_pArtillery);

	m_pStrategy = new CButton(0, 0, 100, 100, L"STRATEGY");
	m_pStrategy->SetClickedListener(this, Strategy);
	m_pStrategy->SetCursorInListener(this, StrategyHint);
	m_pStrategy->SetFont(L"header", 18);
	AddControl(m_pStrategy);

	m_pLoad = new CButton(0, 0, 100, 100, L"LOAD");
	m_pLoad->SetClickedListener(this, Load);
	m_pLoad->SetFont(L"header", 18);

#if !defined(TINKER_UNLOCKED)
	m_pLoad->SetEnabled(false);
#endif

	AddControl(m_pLoad);

	m_pDockPanel = new CDockPanel();
	m_pDockPanel->SetBGColor(Color(12, 13, 12, 255));
	AddControl(m_pDockPanel);
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

void CGamesPanel::ArtilleryCallback()
{
	m_pDockPanel->SetDockedPanel(new CArtilleryGamePanel());
}

void CGamesPanel::StrategyCallback()
{
	m_pDockPanel->SetDockedPanel(new CStrategyGamePanel());
}

void CGamesPanel::LoadCallback()
{
	wchar_t* pszFilename = OpenFileDialog(L"Save Games *.sav\0*.sav\0");
	if (!pszFilename)
		return;

#if !defined(TINKER_UNLOCKED)
	return;
#endif

	DigitanksWindow()->RenderLoading();

	if (CGameServer::LoadFromFile(pszFilename))
		DigitanksWindow()->GetMainMenu()->SetVisible(false);
	else
	{
		DigitanksWindow()->DestroyGame();
		DigitanksWindow()->CreateGame(GAMETYPE_MENU);
	}
}

void CGamesPanel::ArtilleryHintCallback()
{
	DigitanksWindow()->GetMainMenu()->SetHint(L"Artillery mode is a quick no-holds-barred fight to the death. You control 1 to 4 tanks in a head-on deathmatch against your enemies. The last team standing wins. Not much strategy here, just make sure you bring the biggest guns!");
}

void CGamesPanel::StrategyHintCallback()
{
	DigitanksWindow()->GetMainMenu()->SetHint(L"Strap in and grab a cup of coffee! Strategy mode takes a couple hours to play. You control a CPU, build a base, and produce units. You'll have to control and harvest the valuable Electronode resources to win. The objective is to destroy all enemy CPUs.");
}

CMultiplayerPanel::CMultiplayerPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pConnect = new CButton(0, 0, 100, 100, L"CONNECT");
	m_pConnect->SetClickedListener(this, Connect);
	m_pConnect->SetCursorInListener(this, ClientHint);
	m_pConnect->SetFont(L"header", 18);
	AddControl(m_pConnect);

	m_pArtillery = new CButton(0, 0, 100, 100, L"HOST ARTILLERY");
	m_pArtillery->SetClickedListener(this, Artillery);
	m_pArtillery->SetCursorInListener(this, HostHint);
	m_pArtillery->SetFont(L"header", 18);
	AddControl(m_pArtillery);

	if (DigitanksWindow()->IsRegistered())
	{
		m_pStrategy = new CButton(0, 0, 100, 100, L"HOST STRATEGY");
		m_pStrategy->SetClickedListener(this, Strategy);
		m_pStrategy->SetCursorInListener(this, HostHint);
		m_pStrategy->SetFont(L"header", 18);
		AddControl(m_pStrategy);
	}

	m_pLoad = new CButton(0, 0, 100, 100, L"LOAD");
	m_pLoad->SetClickedListener(this, Load);
	m_pLoad->SetCursorInListener(this, LoadHint);
	m_pLoad->SetFont(L"header", 18);

#if !defined(TINKER_UNLOCKED)
	m_pLoad->SetEnabled(false);
#endif

	AddControl(m_pLoad);

	m_pDockPanel = new CDockPanel();
	m_pDockPanel->SetBGColor(Color(12, 13, 12, 255));
	AddControl(m_pDockPanel);
}

void CMultiplayerPanel::Layout()
{
	m_pConnect->SetPos(20, 20);
	m_pConnect->SetSize(135, 40);

	m_pArtillery->SetPos(20, 100);
	m_pArtillery->SetSize(135, 40);

	if (DigitanksWindow()->IsRegistered())
	{
		m_pStrategy->SetPos(20, 160);
		m_pStrategy->SetSize(135, 40);
	}

	m_pLoad->SetPos(20, GetHeight() - 60);
	m_pLoad->SetSize(135, 40);

	m_pDockPanel->SetSize(GetWidth() - 20 - 135 - 20 - 20, GetHeight() - 40);
	m_pDockPanel->SetPos(20 + 135 + 20, 20);

	BaseClass::Layout();
}

void CMultiplayerPanel::ConnectCallback()
{
	m_pDockPanel->SetDockedPanel(new CConnectPanel());
}

void CMultiplayerPanel::ArtilleryCallback()
{
	m_pDockPanel->SetDockedPanel(new CArtilleryGamePanel(true));
}

void CMultiplayerPanel::StrategyCallback()
{
	m_pDockPanel->SetDockedPanel(new CStrategyGamePanel(true));
}

void CMultiplayerPanel::LoadCallback()
{
	wchar_t* pszFilename = OpenFileDialog(L"Save Games *.sav\0*.sav\0");
	if (!pszFilename)
		return;

#if !defined(TINKER_UNLOCKED)
	return;
#endif

	GameServer()->SetServerType(SERVER_HOST);

	DigitanksWindow()->RenderLoading();

	if (CGameServer::LoadFromFile(pszFilename))
		DigitanksWindow()->GetMainMenu()->SetVisible(false);
	else
	{
		DigitanksWindow()->DestroyGame();
		DigitanksWindow()->CreateGame(GAMETYPE_MENU);
	}
}

void CMultiplayerPanel::ClientHintCallback()
{
	DigitanksWindow()->GetMainMenu()->SetHint(L"Enter a hostname and port to connect to a remote host and play.");
}

void CMultiplayerPanel::HostHintCallback()
{
	DigitanksWindow()->GetMainMenu()->SetHint(L"Start a game here to set up your own host in this game mode.");
}

void CMultiplayerPanel::LoadHintCallback()
{
#if !defined(TINKER_UNLOCKED)
	DigitanksWindow()->GetMainMenu()->SetHint(L"The load feature is not available in this demo.");
	return;
#endif
	DigitanksWindow()->GetMainMenu()->SetHint(L"You can load any saved game here to host it in multiplayer.");
}

CConnectPanel::CConnectPanel()
	: CPanel(0, 0, 570, 520)
{
	DigitanksWindow()->SetServerType(SERVER_CLIENT);

	m_pHostnameLabel = new CLabel(0, 0, 32, 32, L"Host:");
	m_pHostnameLabel->SetWrap(false);
	m_pHostnameLabel->SetFont(L"text");
	AddControl(m_pHostnameLabel);

	m_pHostname = new CTextField();
	AddControl(m_pHostname);

	m_pConnect = new CButton(0, 0, 100, 100, L"Connect");
	m_pConnect->SetClickedListener(this, Connect);
	m_pConnect->SetFont(L"header", 12);
	AddControl(m_pConnect);
}

void CConnectPanel::Layout()
{
	m_pHostnameLabel->SetPos(GetWidth()/2-m_pHostnameLabel->GetWidth()/2, GetHeight()-330);
	m_pHostname->SetPos(GetWidth()/2-m_pHostname->GetWidth()/2, GetHeight()-300);

	m_pConnect->SetSize(135, 40);
	m_pConnect->SetPos(GetWidth()/2-135/2, GetHeight()-160);

	BaseClass::Layout();
}

void CConnectPanel::ConnectCallback()
{
	DigitanksWindow()->SetConnectHost(m_pHostname->GetText());
	DigitanksWindow()->CreateGame(GAMETYPE_EMPTY);
}

CArtilleryGamePanel::CArtilleryGamePanel(bool bMultiplayer)
	: CPanel(0, 0, 570, 520)
{
	if (bMultiplayer)
		DigitanksWindow()->SetServerType(SERVER_HOST);
	else
		DigitanksWindow()->SetServerType(SERVER_LOCAL);

	m_pLevels = new CMenu(L"Choose Level");
	m_pLevels->SetFont(L"header");
	AddControl(m_pLevels);

	for (size_t i = 0; i < CDigitanksGame::GetNumLevels(GAMETYPE_ARTILLERY); i++)
	{
		CLevel* pLevel = CDigitanksGame::GetLevel(GAMETYPE_ARTILLERY, i);
		m_pLevels->AddSubmenu(convertstring<char, char16_t>(pLevel->GetName()), this, LevelChosen);
	}
	m_iLevelSelected = 0;

	m_pDifficulty = new CScrollSelector<int>(L"text");
	m_pDifficulty->AddSelection(CScrollSelection<int>(0, L"Easy"));
	m_pDifficulty->AddSelection(CScrollSelection<int>(1, L"Normal"));
	m_pDifficulty->SetSelection(1);
	AddControl(m_pDifficulty);

	m_pDifficultyLabel = new CLabel(0, 0, 32, 32, L"Difficulty");
	m_pDifficultyLabel->SetWrap(false);
	m_pDifficultyLabel->SetFont(L"text");
	AddControl(m_pDifficultyLabel);

	if (bMultiplayer)
	{
		m_pDifficulty->SetVisible(false);
		m_pDifficultyLabel->SetVisible(false);
	}

	m_pHumanPlayers = new CScrollSelector<int>(L"text");
	m_pHumanPlayers->AddSelection(CScrollSelection<int>(1, L"1"));
	if (DigitanksWindow()->IsRegistered())
	{
		m_pHumanPlayers->AddSelection(CScrollSelection<int>(2, L"2"));
		m_pHumanPlayers->AddSelection(CScrollSelection<int>(3, L"3"));
		m_pHumanPlayers->AddSelection(CScrollSelection<int>(4, L"4"));
		m_pHumanPlayers->AddSelection(CScrollSelection<int>(5, L"5"));
		m_pHumanPlayers->AddSelection(CScrollSelection<int>(6, L"6"));
		m_pHumanPlayers->AddSelection(CScrollSelection<int>(7, L"7"));
		m_pHumanPlayers->AddSelection(CScrollSelection<int>(8, L"8"));
	}
	m_pHumanPlayers->SetSelection(0);
	m_pHumanPlayers->SetSelectedListener(this, UpdateLayout);
	AddControl(m_pHumanPlayers);

	m_pHumanPlayersLabel = new CLabel(0, 0, 32, 32, L"Human Players");
	m_pHumanPlayersLabel->SetWrap(false);
	m_pHumanPlayersLabel->SetFont(L"text");
	AddControl(m_pHumanPlayersLabel);

	m_pBotPlayers = new CScrollSelector<int>(L"text");
	AddControl(m_pBotPlayers);

	m_pBotPlayersLabel = new CLabel(0, 0, 32, 32, L"Bot Players");
	m_pBotPlayersLabel->SetWrap(false);
	m_pBotPlayersLabel->SetFont(L"text");
	AddControl(m_pBotPlayersLabel);

	m_pTanks = new CScrollSelector<int>(L"text");
	m_pTanks->AddSelection(CScrollSelection<int>(1, L"1"));
	m_pTanks->AddSelection(CScrollSelection<int>(2, L"2"));
	m_pTanks->AddSelection(CScrollSelection<int>(3, L"3"));
	m_pTanks->AddSelection(CScrollSelection<int>(4, L"4"));
	m_pTanks->SetSelection(2);
	AddControl(m_pTanks);

	m_pTanksLabel = new CLabel(0, 0, 32, 32, L"Tanks Per Player");
	m_pTanksLabel->SetWrap(false);
	m_pTanksLabel->SetFont(L"text");
	AddControl(m_pTanksLabel);

	m_pTerrain = new CScrollSelector<float>(L"text");
	m_pTerrain->AddSelection(CScrollSelection<float>(10, L"Flatty"));
	m_pTerrain->AddSelection(CScrollSelection<float>(50, L"Hilly"));
	m_pTerrain->AddSelection(CScrollSelection<float>(80, L"Mountainy"));
	m_pTerrain->AddSelection(CScrollSelection<float>(120, L"Everesty"));
	m_pTerrain->SetSelection(2);
	AddControl(m_pTerrain);

	m_pTerrainLabel = new CLabel(0, 0, 32, 32, L"Terrain");
	m_pTerrainLabel->SetWrap(false);
	m_pTerrainLabel->SetFont(L"text");
	AddControl(m_pTerrainLabel);

	m_pBeginGame = new CButton(0, 0, 100, 100, L"BEGIN!");
	m_pBeginGame->SetClickedListener(this, BeginGame);
	m_pBeginGame->SetFont(L"header", 12);
	AddControl(m_pBeginGame);
}

void CArtilleryGamePanel::Layout()
{
	int iSelectorSize = m_pDifficultyLabel->GetHeight() - 4;

	m_pLevels->SetSize(135, 40);
	m_pLevels->SetPos(GetWidth()/2-135/2, 60);
	m_pLevels->SetText(CDigitanksGame::GetLevel(GAMETYPE_ARTILLERY, m_iLevelSelected)->GetName());

	m_pDifficultyLabel->EnsureTextFits();
	m_pDifficultyLabel->SetPos(75, 120);

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 120);

	m_pHumanPlayersLabel->EnsureTextFits();
	m_pHumanPlayersLabel->SetPos(75, 160);

	m_pHumanPlayers->SetSize(GetWidth() - m_pHumanPlayersLabel->GetLeft()*2 - m_pHumanPlayersLabel->GetWidth(), iSelectorSize);
	m_pHumanPlayers->SetPos(m_pHumanPlayersLabel->GetRight(), 160);

	m_pBotPlayersLabel->EnsureTextFits();
	m_pBotPlayersLabel->SetPos(75, 200);
	m_pBotPlayersLabel->SetVisible(m_pHumanPlayers->GetSelectionValue() < 8);

	m_pBotPlayers->SetSize(GetWidth() - m_pBotPlayersLabel->GetLeft()*2 - m_pBotPlayersLabel->GetWidth(), iSelectorSize);
	m_pBotPlayers->SetPos(m_pBotPlayersLabel->GetRight(), 200);
	m_pBotPlayers->SetVisible(m_pHumanPlayers->GetSelectionValue() < 8);

	m_pBotPlayers->RemoveAllSelections();
	if (m_pHumanPlayers->GetSelectionValue() > 1)
		m_pBotPlayers->AddSelection(CScrollSelection<int>(0, L"0"));
	if (m_pHumanPlayers->GetSelectionValue() <= 7)
		m_pBotPlayers->AddSelection(CScrollSelection<int>(1, L"1"));
	if (m_pHumanPlayers->GetSelectionValue() <= 6)
		m_pBotPlayers->AddSelection(CScrollSelection<int>(2, L"2"));
	if (m_pHumanPlayers->GetSelectionValue() <= 5)
		m_pBotPlayers->AddSelection(CScrollSelection<int>(3, L"3"));
	if (m_pHumanPlayers->GetSelectionValue() <= 4)
		m_pBotPlayers->AddSelection(CScrollSelection<int>(4, L"4"));
	if (m_pHumanPlayers->GetSelectionValue() <= 3)
		m_pBotPlayers->AddSelection(CScrollSelection<int>(5, L"5"));
	if (m_pHumanPlayers->GetSelectionValue() <= 2)
		m_pBotPlayers->AddSelection(CScrollSelection<int>(6, L"6"));
	if (m_pHumanPlayers->GetSelectionValue() <= 1)
		m_pBotPlayers->AddSelection(CScrollSelection<int>(7, L"7"));
	m_pBotPlayers->SetSelection(m_pBotPlayers->GetNumSelections()/2);

	m_pTanksLabel->EnsureTextFits();
	m_pTanksLabel->SetPos(75, 240);

	m_pTanks->SetSize(GetWidth() - m_pTanksLabel->GetLeft()*2 - m_pTanksLabel->GetWidth(), iSelectorSize);
	m_pTanks->SetPos(m_pTanksLabel->GetRight(), 240);

	m_pTerrainLabel->EnsureTextFits();
	m_pTerrainLabel->SetPos(75, 280);

	m_pTerrain->SetSize(GetWidth() - m_pTerrainLabel->GetLeft()*2 - m_pTerrainLabel->GetWidth(), iSelectorSize);
	m_pTerrain->SetPos(m_pTerrainLabel->GetRight(), 280);

	m_pBeginGame->SetSize(135, 40);
	m_pBeginGame->SetPos(GetWidth()/2-135/2, GetHeight()-160);

	BaseClass::Layout();
}

void CArtilleryGamePanel::BeginGameCallback()
{
	gamesettings_t oSettings;
	oSettings.iHumanPlayers = m_pHumanPlayers->GetSelectionValue();
	oSettings.iBotPlayers = m_pBotPlayers->GetSelectionValue();
	oSettings.iTanksPerPlayer = m_pTanks->GetSelectionValue();
	oSettings.flTerrainHeight = m_pTerrain->GetSelectionValue();
	oSettings.iLevel = m_iLevelSelected;

	DigitanksWindow()->SetGameSettings(oSettings);
	DigitanksWindow()->CreateGame(GAMETYPE_ARTILLERY);

	if (!GameServer())
		return;

	if (CNetwork::IsHost() && DigitanksGame())
		DigitanksGame()->SetDifficulty(m_pDifficulty->GetSelectionValue());

	CInstructor* pInstructor = DigitanksWindow()->GetInstructor();

	pInstructor->SetActive(true);
	pInstructor->Initialize();
	pInstructor->DisplayIngameArtilleryTutorial();

	DigitanksWindow()->GetMainMenu()->SetVisible(false);
}

void CArtilleryGamePanel::UpdateLayoutCallback()
{
	Layout();
}

void CArtilleryGamePanel::LevelChosenCallback()
{
	size_t iMode = m_pLevels->GetSelectedMenu();

	if (iMode >= CDigitanksGame::GetNumLevels(GAMETYPE_ARTILLERY))
		return;

	m_iLevelSelected = iMode;

	m_pLevels->Pop(true, true);

	Layout();
}

CStrategyGamePanel::CStrategyGamePanel(bool bMultiplayer)
	: CPanel(0, 0, 570, 520)
{
	if (bMultiplayer)
		DigitanksWindow()->SetServerType(SERVER_HOST);
	else
		DigitanksWindow()->SetServerType(SERVER_LOCAL);

	m_pLevels = new CMenu(L"Choose Level");
	m_pLevels->SetFont(L"header");
	AddControl(m_pLevels);

	for (size_t i = 0; i < CDigitanksGame::GetNumLevels(GAMETYPE_STANDARD); i++)
	{
		CLevel* pLevel = CDigitanksGame::GetLevel(GAMETYPE_STANDARD, i);
		m_pLevels->AddSubmenu(convertstring<char, char16_t>(pLevel->GetName()), this, LevelChosen);
	}
	m_iLevelSelected = 0;

	m_pDifficulty = new CScrollSelector<int>(L"text");
	m_pDifficulty->AddSelection(CScrollSelection<int>(0, L"Easy"));
	m_pDifficulty->AddSelection(CScrollSelection<int>(1, L"Normal"));
	m_pDifficulty->SetSelection(1);
	AddControl(m_pDifficulty);

	m_pDifficultyLabel = new CLabel(0, 0, 32, 32, L"Difficulty");
	m_pDifficultyLabel->SetWrap(false);
	m_pDifficultyLabel->SetFont(L"text");
	AddControl(m_pDifficultyLabel);

	if (bMultiplayer)
	{
		m_pDifficulty->SetVisible(false);
		m_pDifficultyLabel->SetVisible(false);
	}

	m_pBotPlayers = new CScrollSelector<int>(L"text");
	AddControl(m_pBotPlayers);

	m_pBotPlayersLabel = new CLabel(0, 0, 32, 32, L"Bot Players");
	m_pBotPlayersLabel->SetWrap(false);
	m_pBotPlayersLabel->SetFont(L"text");
	AddControl(m_pBotPlayersLabel);

	m_pBeginGame = new CButton(0, 0, 100, 100, L"BEGIN!");
	m_pBeginGame->SetClickedListener(this, BeginGame);
	m_pBeginGame->SetFont(L"header", 12);
	AddControl(m_pBeginGame);
}

void CStrategyGamePanel::Layout()
{
	int iSelectorSize = m_pDifficultyLabel->GetHeight() - 4;

	m_pLevels->SetSize(135, 40);
	m_pLevels->SetPos(GetWidth()/2-135/2, 60);
	m_pLevels->SetText(CDigitanksGame::GetLevel(GAMETYPE_STANDARD, m_iLevelSelected)->GetName());

	m_pDifficultyLabel->EnsureTextFits();
	m_pDifficultyLabel->SetPos(75, 120);

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 120);

	m_pBotPlayersLabel->EnsureTextFits();
	m_pBotPlayersLabel->SetPos(75, 240);
	m_pBotPlayersLabel->SetVisible(true);

	m_pBotPlayers->SetSize(GetWidth() - m_pBotPlayersLabel->GetLeft()*2 - m_pBotPlayersLabel->GetWidth(), iSelectorSize);
	m_pBotPlayers->SetPos(m_pBotPlayersLabel->GetRight(), 240);
	m_pBotPlayers->SetVisible(true);

	m_pBotPlayers->RemoveAllSelections();
	m_pBotPlayers->AddSelection(CScrollSelection<int>(1, L"1"));
	m_pBotPlayers->AddSelection(CScrollSelection<int>(2, L"2"));
	m_pBotPlayers->AddSelection(CScrollSelection<int>(3, L"3"));
	m_pBotPlayers->SetSelection(m_pBotPlayers->GetNumSelections()-1);

	m_pBeginGame->SetSize(135, 40);
	m_pBeginGame->SetPos(GetWidth()/2-135/2, GetHeight()-160);

	BaseClass::Layout();
}

void CStrategyGamePanel::BeginGameCallback()
{
	gamesettings_t oSettings;
	oSettings.iHumanPlayers = 1;
	oSettings.iBotPlayers = m_pBotPlayers->GetSelectionValue();
	oSettings.iLevel = m_iLevelSelected;
	DigitanksWindow()->SetGameSettings(oSettings);
	DigitanksWindow()->CreateGame(GAMETYPE_STANDARD);

	if (!GameServer())
		return;

	if (CNetwork::IsHost() && DigitanksGame())
		DigitanksGame()->SetDifficulty(m_pDifficulty->GetSelectionValue());

	CInstructor* pInstructor = DigitanksWindow()->GetInstructor();

	pInstructor->SetActive(false);

	DigitanksWindow()->GetMainMenu()->SetVisible(false);
}

void CStrategyGamePanel::UpdateLayoutCallback()
{
	Layout();
}

void CStrategyGamePanel::LevelChosenCallback()
{
	size_t iMode = m_pLevels->GetSelectedMenu();

	if (iMode >= CDigitanksGame::GetNumLevels(GAMETYPE_STANDARD))
		return;

	m_iLevelSelected = iMode;

	m_pLevels->Pop(true, true);

	Layout();
}

// HOLY CRAP A GLOBAL! Yeah it's bad. Sue me.
eastl::vector<GLFWvidmode> g_aVideoModes;

COptionsPanel::COptionsPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pSoundVolume = new CScrollSelector<float>(L"text");
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0, L"Off"));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.1f, L"10%"));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.2f, L"20%"));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.3f, L"30%"));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.4f, L"40%"));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.5f, L"50%"));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.6f, L"60%"));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.7f, L"70%"));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.8f, L"80%"));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(0.9f, L"90%"));
	m_pSoundVolume->AddSelection(CScrollSelection<float>(1.0f, L"100%"));
	m_pSoundVolume->SetSelection(1);
	m_pSoundVolume->SetSelectedListener(this, SoundVolumeChanged);
	AddControl(m_pSoundVolume);

	m_pSoundVolumeLabel = new CLabel(0, 0, 32, 32, L"Sound Volume");
	m_pSoundVolumeLabel->SetWrap(false);
	m_pSoundVolumeLabel->SetFont(L"text");
	AddControl(m_pSoundVolumeLabel);

	m_pMusicVolume = new CScrollSelector<float>(L"text");
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0, L"Off"));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.1f, L"10%"));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.2f, L"20%"));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.3f, L"30%"));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.4f, L"40%"));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.5f, L"50%"));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.6f, L"60%"));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.7f, L"70%"));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.8f, L"80%"));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(0.9f, L"90%"));
	m_pMusicVolume->AddSelection(CScrollSelection<float>(1.0f, L"100%"));
	m_pMusicVolume->SetSelection(1);
	m_pMusicVolume->SetSelectedListener(this, MusicVolumeChanged);
	AddControl(m_pMusicVolume);

	m_pMusicVolumeLabel = new CLabel(0, 0, 32, 32, L"Music Volume");
	m_pMusicVolumeLabel->SetWrap(false);
	m_pMusicVolumeLabel->SetFont(L"text");
	AddControl(m_pMusicVolumeLabel);

	m_pVideoChangedNotice = new CLabel(0, 0, 32, 32, L"Changes to the video settings will take effect after the game has been restarted.");
	m_pVideoChangedNotice->SetVisible(false);
	m_pVideoChangedNotice->SetFont(L"text");
	AddControl(m_pVideoChangedNotice);

	m_pWindowed = new CCheckBox();
	m_pWindowed->SetClickedListener(this, WindowedChanged);
	m_pWindowed->SetUnclickedListener(this, WindowedChanged);
	AddControl(m_pWindowed);

	m_pWindowedLabel = new CLabel(0, 0, 100, 100, L"Run in a window");
	m_pWindowedLabel->SetFont(L"text");
	AddControl(m_pWindowedLabel);

	m_pVideoModes = new CMenu(L"Change Resolution");
	m_pVideoModes->SetFont(L"text");
	AddControl(m_pVideoModes);

	GLFWvidmode aModes[ 20 ];
    int iModes;

	g_aVideoModes.clear();

	iModes = glfwGetVideoModes( aModes, 20 );
    for( int i = 0; i < iModes; i ++ )
    {
		if (aModes[i].Width < 1024)
			continue;

		if (aModes[i].Height < 768)
			continue;

		if (aModes[i].BlueBits < 8)
			continue;

		eastl::string16 sMode;
		sMode.sprintf(L"%dx%d", aModes[i].Width, aModes[i].Height);
		m_pVideoModes->AddSubmenu(sMode, this, VideoModeChosen);
		g_aVideoModes.push_back(aModes[i]);
	}

	m_pFramebuffers = new CCheckBox();
	m_pFramebuffers->SetClickedListener(this, FramebuffersChanged);
	m_pFramebuffers->SetUnclickedListener(this, FramebuffersChanged);
	AddControl(m_pFramebuffers);

	m_pFramebuffersLabel = new CLabel(0, 0, 100, 100, L"Use framebuffers");
	m_pFramebuffersLabel->SetFont(L"text");
	AddControl(m_pFramebuffersLabel);

	m_pShaders = new CCheckBox();
	m_pShaders->SetClickedListener(this, ShadersChanged);
	m_pShaders->SetUnclickedListener(this, ShadersChanged);
	AddControl(m_pShaders);

	m_pShadersLabel = new CLabel(0, 0, 100, 100, L"Use shaders");
	m_pShadersLabel->SetFont(L"text");
	AddControl(m_pShadersLabel);

	m_pConstrain = new CCheckBox();
	m_pConstrain->SetClickedListener(this, ConstrainChanged);
	m_pConstrain->SetUnclickedListener(this, ConstrainChanged);
	AddControl(m_pConstrain);

	m_pConstrainLabel = new CLabel(0, 0, 100, 100, L"Constrain mouse to screen edges");
	m_pConstrainLabel->SetFont(L"text");
	AddControl(m_pConstrainLabel);
}

void COptionsPanel::Layout()
{
	int iSelectorSize = m_pMusicVolumeLabel->GetHeight() - 4;

	m_pSoundVolumeLabel->EnsureTextFits();
	m_pSoundVolumeLabel->SetPos(75, 80);

	m_pSoundVolume->SetSize(GetWidth() - m_pSoundVolumeLabel->GetLeft()*2 - m_pSoundVolumeLabel->GetWidth(), iSelectorSize);
	m_pSoundVolume->SetPos(m_pSoundVolumeLabel->GetRight(), 80);

	m_pSoundVolume->SetSelection((size_t)(DigitanksWindow()->GetSoundVolume()*10));

	m_pMusicVolumeLabel->EnsureTextFits();
	m_pMusicVolumeLabel->SetPos(75, 160);

	m_pMusicVolume->SetSize(GetWidth() - m_pMusicVolumeLabel->GetLeft()*2 - m_pMusicVolumeLabel->GetWidth(), iSelectorSize);
	m_pMusicVolume->SetPos(m_pMusicVolumeLabel->GetRight(), 160);

	m_pMusicVolume->SetSelection((size_t)(DigitanksWindow()->GetMusicVolume()*10));

	m_pVideoChangedNotice->SetSize(GetWidth()-50, 30);
	m_pVideoChangedNotice->SetPos(25, GetHeight()-290);

	m_pVideoModes->SetSize(120, 30);
	m_pVideoModes->SetPos(GetWidth()/2 - m_pVideoModes->GetWidth() - 40, GetHeight()-230);

	eastl::string16 sVideoMode;
	sVideoMode.sprintf(L"%dx%d", DigitanksWindow()->GetWindowWidth(), DigitanksWindow()->GetWindowHeight());
	m_pVideoModes->SetText(sVideoMode);

	m_pWindowedLabel->SetWrap(false);
	m_pWindowedLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pWindowedLabel->SetSize(10, 10);
	m_pWindowedLabel->EnsureTextFits();
	m_pWindowedLabel->SetPos(GetWidth()/2 - m_pWindowedLabel->GetWidth()/2 + 10 + 40, GetHeight()-230);
	m_pWindowed->SetPos(m_pWindowedLabel->GetLeft() - 15, GetHeight()-230 + m_pWindowedLabel->GetHeight()/2 - m_pWindowed->GetHeight()/2);
	m_pWindowed->SetState(!DigitanksWindow()->IsFullscreen(), false);

	m_pFramebuffersLabel->SetWrap(false);
	m_pFramebuffersLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pFramebuffersLabel->SetSize(10, 10);
	m_pFramebuffersLabel->EnsureTextFits();
	m_pFramebuffersLabel->SetPos(GetWidth()/2 - m_pFramebuffersLabel->GetWidth()/2 + 10 + 40, GetHeight()-200);
	m_pFramebuffers->SetPos(m_pFramebuffersLabel->GetLeft() - 15, GetHeight()-200 + m_pFramebuffersLabel->GetHeight()/2 - m_pFramebuffers->GetHeight()/2);
	m_pFramebuffers->SetState(DigitanksWindow()->WantsFramebuffers(), false);

	m_pShadersLabel->SetWrap(false);
	m_pShadersLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pShadersLabel->SetSize(10, 10);
	m_pShadersLabel->EnsureTextFits();
	m_pShadersLabel->SetPos(GetWidth()/2 - m_pShadersLabel->GetWidth()/2 + 10 + 40, GetHeight()-180);
	m_pShaders->SetPos(m_pShadersLabel->GetLeft() - 15, GetHeight()-180 + m_pShadersLabel->GetHeight()/2 - m_pShaders->GetHeight()/2);
	m_pShaders->SetState(DigitanksWindow()->WantsShaders(), false);

	m_pConstrainLabel->SetWrap(false);
	m_pConstrainLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pConstrainLabel->SetSize(10, 10);
	m_pConstrainLabel->EnsureTextFits();
	m_pConstrainLabel->SetPos(GetWidth()/2 - m_pConstrainLabel->GetWidth()/2 + 10 + 40, GetHeight()-130);
	m_pConstrain->SetPos(m_pConstrainLabel->GetLeft() - 15, GetHeight()-130 + m_pConstrainLabel->GetHeight()/2 - m_pConstrain->GetHeight()/2);
	m_pConstrain->SetState(DigitanksWindow()->ShouldConstrainMouse(), false);

	BaseClass::Layout();
}

void COptionsPanel::SoundVolumeChangedCallback()
{
	DigitanksWindow()->SetSoundVolume(m_pSoundVolume->GetSelectionValue());
	DigitanksWindow()->SaveConfig();
}

void COptionsPanel::MusicVolumeChangedCallback()
{
	DigitanksWindow()->SetMusicVolume(m_pMusicVolume->GetSelectionValue());
	DigitanksWindow()->SaveConfig();
}

void COptionsPanel::VideoModeChosenCallback()
{
	size_t iMode = m_pVideoModes->GetSelectedMenu();

	if (iMode >= g_aVideoModes.size())
		return;

	int iHeight = g_aVideoModes[iMode].Height;
	int iWidth = g_aVideoModes[iMode].Width;
	DigitanksWindow()->SetConfigWindowDimensions(iWidth, iHeight);
	DigitanksWindow()->SaveConfig();

	m_pVideoChangedNotice->SetVisible(true);

	m_pVideoModes->Pop(true, true);
}

void COptionsPanel::WindowedChangedCallback()
{
	DigitanksWindow()->SetConfigFullscreen(!m_pWindowed->GetState());
	DigitanksWindow()->SaveConfig();

	m_pVideoChangedNotice->SetVisible(true);
}

void COptionsPanel::FramebuffersChangedCallback()
{
	DigitanksWindow()->SetWantsFramebuffers(m_pFramebuffers->GetState());
	DigitanksWindow()->SaveConfig();

	m_pVideoChangedNotice->SetVisible(true);
}

void COptionsPanel::ShadersChangedCallback()
{
	DigitanksWindow()->SetWantsShaders(m_pShaders->GetState());
	DigitanksWindow()->SaveConfig();

	m_pVideoChangedNotice->SetVisible(true);
}

void COptionsPanel::ConstrainChangedCallback()
{
	DigitanksWindow()->SetConstrainMouse(m_pConstrain->GetState());
	DigitanksWindow()->SaveConfig();
}
