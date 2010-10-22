#include "menu.h"

#include <platform.h>

#include <digitanks/digitanksgame.h>
#include <renderer/renderer.h>

#include "instructor.h"
#include "digitankswindow.h"

using namespace glgui;

CMainMenu::CMainMenu()
	: CPanel(0, 0, 310, 520)
{
	m_pTutorial = new CButton(0, 0, 100, 100, "Tutorials");
	m_pTutorial->SetClickedListener(this, OpenTutorialsPanel);
	m_pTutorial->SetFontFaceSize(36);
	AddControl(m_pTutorial);

	m_pPlay = new CButton(0, 0, 100, 100, "Play Digitanks!");
	m_pPlay->SetClickedListener(this, OpenGamesPanel);
	m_pPlay->SetFontFaceSize(36);
	AddControl(m_pPlay);

	m_pMultiplayer = new CButton(0, 0, 100, 100, "Multiplayer");
	m_pMultiplayer->SetClickedListener(this, OpenMultiplayerPanel);
	m_pMultiplayer->SetFontFaceSize(36);
	AddControl(m_pMultiplayer);

	m_pOptions = new CButton(0, 0, 100, 100, "Options");
	m_pOptions->SetClickedListener(this, OpenTutorialsPanel);
	m_pOptions->SetFontFaceSize(36);
	AddControl(m_pOptions);

	m_pQuit = new CButton(0, 0, 100, 100, "Quit");
	m_pQuit->SetClickedListener(this, Quit);
	m_pQuit->SetFontFaceSize(36);
	AddControl(m_pQuit);

	m_pHint = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pHint);

	m_pDockPanel = NULL;

	m_iLunarWorkshop = CRenderer::LoadTextureIntoGL(L"textures/lunar-workshop.png");
	m_iDigitanks = CRenderer::LoadTextureIntoGL(L"textures/digitanks.png");
}

void CMainMenu::Layout()
{
	BaseClass::Layout();

	SetPos(40, 230);

	m_pTutorial->SetPos(20, 20);
	m_pTutorial->SetSize(270, 80);

	m_pPlay->SetPos(20, 120);
	m_pPlay->SetSize(270, 80);

	m_pMultiplayer->SetPos(20, 220);
	m_pMultiplayer->SetSize(270, 80);

	m_pOptions->SetPos(20, 320);
	m_pOptions->SetSize(270, 80);

	m_pQuit->SetPos(20, 420);
	m_pQuit->SetSize(270, 80);

	if (m_pDockPanel)
	{
		m_pDockPanel->SetSize(570, 520);
		m_pDockPanel->SetPos(390, 30);
	}

	m_pHint->SetPos(375, 340);
	m_pHint->SetSize(350, 160);
}

void CMainMenu::Paint(int x, int y, int w, int h)
{
	if (DigitanksGame() && DigitanksGame()->GetGameType() == GAMETYPE_MENU)
	{
		CRenderingContext c(GameServer()->GetRenderer());

		c.SetBlend(BLEND_ALPHA);
		CRootPanel::PaintTexture(m_iLunarWorkshop, CRootPanel::Get()->GetWidth()-200-20, CRootPanel::Get()->GetHeight()-200, 200, 200);
		CRootPanel::PaintTexture(m_iDigitanks, 20, 20, 350, 175);
	}

	CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, 255));

	int hx, hy;
	m_pHint->GetAbsPos(hx, hy);
	if (wcslen(m_pHint->GetText()))
		CRootPanel::PaintRect(hx-25, hy-3, m_pHint->GetWidth()+50, m_pHint->GetHeight()+6, Color(0, 0, 0, 255));

	BaseClass::Paint(x, y, w, h);
}

void CMainMenu::SetVisible(bool bVisible)
{
	BaseClass::SetVisible(bVisible);

	if (!bVisible)
	{
		m_pDockPanel->SetVisible(bVisible);
		m_pHint->SetText("");
	}
}

void CMainMenu::OpenTutorialsPanelCallback()
{
	CDockPanel* pDock = GetDockPanel();
	pDock->SetDockedPanel(new CTutorialsPanel());
	pDock->SetVisible(true);
}

void CMainMenu::OpenGamesPanelCallback()
{
	CDockPanel* pDock = GetDockPanel();
	pDock->SetDockedPanel(new CGamesPanel());
	pDock->SetVisible(true);
}

void CMainMenu::OpenMultiplayerPanelCallback()
{
	CDockPanel* pDock = GetDockPanel();
	pDock->SetDockedPanel(new CMultiplayerPanel());
	pDock->SetVisible(true);
}

void CMainMenu::QuitCallback()
{
	CDigitanksWindow::Get()->CloseApplication();
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

void CMainMenu::SetHint(const std::wstring& s)
{
	m_pHint->SetText(s.c_str());
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
	m_pBasics = new CButton(0, 0, 100, 100, "The Basics");
	m_pBasics->SetClickedListener(this, Basics);
	m_pBasics->SetCursorInListener(this, BasicsHint);
	m_pBasics->SetFontFaceSize(18);
	AddControl(m_pBasics);

	m_pBases = new CButton(0, 0, 100, 100, "Building a Base");
	m_pBases->SetClickedListener(this, Bases);
	m_pBases->SetCursorInListener(this, BasesHint);
	m_pBases->SetFontFaceSize(18);
	AddControl(m_pBases);
}

void CTutorialsPanel::Layout()
{
	BaseClass::Layout();

	m_pBasics->SetPos(100, 60);
	m_pBasics->SetSize(135, 40);

	m_pBases->SetPos(100, 120);
	m_pBases->SetSize(135, 40);
}

void CTutorialsPanel::BasicsCallback()
{
	CInstructor* pInstructor = CDigitanksWindow::Get()->GetInstructor();

	pInstructor->SetActive(true);
	pInstructor->Initialize();

	CDigitanksWindow::Get()->SetServerType(SERVER_LOCAL);
	CDigitanksWindow::Get()->CreateGame(GAMETYPE_TUTORIAL);
	DigitanksGame()->SetDifficulty(0);

	pInstructor->DisplayFirstBasicsTutorial();

	CDigitanksWindow::Get()->GetMainMenu()->SetVisible(false);
}

void CTutorialsPanel::BasesCallback()
{
	CInstructor* pInstructor = CDigitanksWindow::Get()->GetInstructor();

	pInstructor->SetActive(true);
	pInstructor->Initialize();

	CDigitanksWindow::Get()->SetServerType(SERVER_LOCAL);
	CDigitanksWindow::Get()->CreateGame(GAMETYPE_TUTORIAL);
	DigitanksGame()->SetDifficulty(0);

	pInstructor->DisplayFirstBasesTutorial();

	CDigitanksWindow::Get()->GetMainMenu()->SetVisible(false);
}

void CTutorialsPanel::BasicsHintCallback()
{
	CDigitanksWindow::Get()->GetMainMenu()->SetHint(L"Learn the basics of Digitanks. This tutorial includes view control and basic tank manipulation. After this tutorial you should know enough to play an Artillery game.");
}

void CTutorialsPanel::BasesHintCallback()
{
	CDigitanksWindow::Get()->GetMainMenu()->SetHint(L"Learn how to set up a base. In this tutorial you'll learn how to construct buildings and produce units. It's a good idea to play through this tutorial before beginning Strategy mode.");
}

CGamesPanel::CGamesPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pArtillery = new CButton(0, 0, 100, 100, "Artillery Mode");
	m_pArtillery->SetClickedListener(this, Artillery);
	m_pArtillery->SetCursorInListener(this, ArtilleryHint);
	m_pArtillery->SetFontFaceSize(18);
	AddControl(m_pArtillery);

	m_pStrategy = new CButton(0, 0, 100, 100, "Strategy Mode");
	m_pStrategy->SetClickedListener(this, Strategy);
	m_pStrategy->SetCursorInListener(this, StrategyHint);
	m_pStrategy->SetFontFaceSize(18);
	AddControl(m_pStrategy);

	m_pLoad = new CButton(0, 0, 100, 100, "Load");
	m_pLoad->SetClickedListener(this, Load);
	m_pLoad->SetFontFaceSize(18);
	AddControl(m_pLoad);

	m_pDockPanel = new CDockPanel();
	m_pDockPanel->SetBGColor(Color(12, 13, 12, 255));
	AddControl(m_pDockPanel);
}

void CGamesPanel::Layout()
{
	BaseClass::Layout();

	m_pArtillery->SetPos(20, 20);
	m_pArtillery->SetSize(135, 40);

	m_pStrategy->SetPos(20, 80);
	m_pStrategy->SetSize(135, 40);

	m_pLoad->SetPos(20, GetHeight() - 60);
	m_pLoad->SetSize(135, 40);

	m_pDockPanel->SetSize(GetWidth() - 20 - 135 - 20 - 20, GetHeight() - 40);
	m_pDockPanel->SetPos(20 + 135 + 20, 20);
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

	if (CGameServer::LoadFromFile(pszFilename))
		CDigitanksWindow::Get()->GetMainMenu()->SetVisible(false);
	else
	{
		CDigitanksWindow::Get()->DestroyGame();
		CDigitanksWindow::Get()->CreateGame(GAMETYPE_MENU);
	}
}

void CGamesPanel::ArtilleryHintCallback()
{
	CDigitanksWindow::Get()->GetMainMenu()->SetHint(L"Artillery mode is a quick game mode. You control 2 to 5 tanks in a heads-on deathmatch against your enemies. The last team standing wins. Not much strategy here, just make sure you bring the biggest guns!");
}

void CGamesPanel::StrategyHintCallback()
{
	CDigitanksWindow::Get()->GetMainMenu()->SetHint(L"Strap in and grab a cup of coffee! Strategy mode takes a couple hours to play. You control a CPU, build a base, and produce units. You'll have to control and harvest the valuable Electronode resources to win. The objective is to destroy all enemy CPUs.");
}

CMultiplayerPanel::CMultiplayerPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pConnect = new CButton(0, 0, 100, 100, "Connect");
	m_pConnect->SetClickedListener(this, Connect);
	m_pConnect->SetCursorInListener(this, ClientHint);
	m_pConnect->SetFontFaceSize(18);
	AddControl(m_pConnect);

	m_pArtillery = new CButton(0, 0, 100, 100, "Host Artillery");
	m_pArtillery->SetClickedListener(this, Artillery);
	m_pArtillery->SetCursorInListener(this, HostHint);
	m_pArtillery->SetFontFaceSize(18);
	AddControl(m_pArtillery);

	m_pStrategy = new CButton(0, 0, 100, 100, "Host Strategy");
	m_pStrategy->SetClickedListener(this, Strategy);
	m_pStrategy->SetCursorInListener(this, HostHint);
	m_pStrategy->SetFontFaceSize(18);
	AddControl(m_pStrategy);

//	m_pLoad = new CButton(0, 0, 100, 100, "Load");
//	m_pLoad->SetClickedListener(this, Load);
//	m_pLoad->SetCursorInListener(this, LoadHint);
//	m_pLoad->SetFontFaceSize(18);
//	AddControl(m_pLoad);

	m_pDockPanel = new CDockPanel();
	m_pDockPanel->SetBGColor(Color(12, 13, 12, 255));
	AddControl(m_pDockPanel);
}

void CMultiplayerPanel::Layout()
{
	BaseClass::Layout();

	m_pConnect->SetPos(20, 20);
	m_pConnect->SetSize(135, 40);

	m_pArtillery->SetPos(20, 100);
	m_pArtillery->SetSize(135, 40);

	m_pStrategy->SetPos(20, 160);
	m_pStrategy->SetSize(135, 40);

//	m_pLoad->SetPos(20, GetHeight() - 60);
//	m_pLoad->SetSize(135, 40);

	m_pDockPanel->SetSize(GetWidth() - 20 - 135 - 20 - 20, GetHeight() - 40);
	m_pDockPanel->SetPos(20 + 135 + 20, 20);
}

void CMultiplayerPanel::ConnectCallback()
{
	CDigitanksWindow::Get()->SetServerType(SERVER_CLIENT);
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

	if (CGameServer::LoadFromFile(pszFilename))
		CDigitanksWindow::Get()->GetMainMenu()->SetVisible(false);
	else
	{
		CDigitanksWindow::Get()->DestroyGame();
		CDigitanksWindow::Get()->CreateGame(GAMETYPE_MENU);
	}
}

void CMultiplayerPanel::ClientHintCallback()
{
	CDigitanksWindow::Get()->GetMainMenu()->SetHint(L"Enter a hostname and port to connect to a remote host and play.");
}

void CMultiplayerPanel::HostHintCallback()
{
	CDigitanksWindow::Get()->GetMainMenu()->SetHint(L"Start a game here to set up your own host in this game mode.");
}

void CMultiplayerPanel::LoadHintCallback()
{
	CDigitanksWindow::Get()->GetMainMenu()->SetHint(L"You can load any saved game here to host it in multiplayer.");
}

CArtilleryGamePanel::CArtilleryGamePanel(bool bMultiplayer)
	: CPanel(0, 0, 570, 520)
{
	if (bMultiplayer)
		CDigitanksWindow::Get()->SetServerType(SERVER_HOST);
	else
		CDigitanksWindow::Get()->SetServerType(SERVER_LOCAL);

	m_pDifficulty = new CScrollSelector<int>();
	m_pDifficulty->AddSelection(CScrollSelection<int>(0, L"Easy"));
	m_pDifficulty->AddSelection(CScrollSelection<int>(1, L"Normal"));
	m_pDifficulty->SetSelection(1);
	AddControl(m_pDifficulty);

	m_pDifficultyLabel = new CLabel(0, 0, 32, 32, "Difficulty");
	m_pDifficultyLabel->SetWrap(false);
	AddControl(m_pDifficultyLabel);

	m_pPlayers = new CScrollSelector<int>();
	m_pPlayers->AddSelection(CScrollSelection<int>(2, L"2"));
	m_pPlayers->AddSelection(CScrollSelection<int>(3, L"3"));
	m_pPlayers->AddSelection(CScrollSelection<int>(4, L"4"));
	m_pPlayers->AddSelection(CScrollSelection<int>(5, L"5"));
	m_pPlayers->AddSelection(CScrollSelection<int>(6, L"6"));
	m_pPlayers->AddSelection(CScrollSelection<int>(7, L"7"));
	m_pPlayers->AddSelection(CScrollSelection<int>(8, L"8"));
	m_pPlayers->SetSelection(2);
	AddControl(m_pPlayers);

	m_pPlayersLabel = new CLabel(0, 0, 32, 32, "Players");
	m_pPlayersLabel->SetWrap(false);
	AddControl(m_pPlayersLabel);

	m_pTanks = new CScrollSelector<int>();
	m_pTanks->AddSelection(CScrollSelection<int>(1, L"1"));
	m_pTanks->AddSelection(CScrollSelection<int>(2, L"2"));
	m_pTanks->AddSelection(CScrollSelection<int>(3, L"3"));
	m_pTanks->AddSelection(CScrollSelection<int>(4, L"4"));
	m_pTanks->AddSelection(CScrollSelection<int>(5, L"5"));
	m_pTanks->SetSelection(2);
	AddControl(m_pTanks);

	m_pTanksLabel = new CLabel(0, 0, 32, 32, "Tanks Per Player");
	m_pTanksLabel->SetWrap(false);
	AddControl(m_pTanksLabel);

	m_pBeginGame = new CButton(0, 0, 100, 100, "BEGIN!");
	m_pBeginGame->SetClickedListener(this, BeginGame);
	m_pBeginGame->SetFontFaceSize(12);
	AddControl(m_pBeginGame);
}

void CArtilleryGamePanel::Layout()
{
	BaseClass::Layout();

	int iSelectorSize = m_pDifficultyLabel->GetHeight() - 4;

	m_pDifficultyLabel->EnsureTextFits();
	m_pDifficultyLabel->SetPos(75, 120);

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 120);

	m_pPlayersLabel->EnsureTextFits();
	m_pPlayersLabel->SetPos(75, 180);

	m_pPlayers->SetSize(GetWidth() - m_pPlayersLabel->GetLeft()*2 - m_pPlayersLabel->GetWidth(), iSelectorSize);
	m_pPlayers->SetPos(m_pPlayersLabel->GetRight(), 180);

	m_pTanksLabel->EnsureTextFits();
	m_pTanksLabel->SetPos(75, 240);

	m_pTanks->SetSize(GetWidth() - m_pTanksLabel->GetLeft()*2 - m_pTanksLabel->GetWidth(), iSelectorSize);
	m_pTanks->SetPos(m_pTanksLabel->GetRight(), 240);

	m_pBeginGame->SetSize(135, 40);
	m_pBeginGame->SetPos(GetWidth()/2-135/2, GetHeight()-160);
}

void CArtilleryGamePanel::BeginGameCallback()
{
	CDigitanksWindow::Get()->SetPlayers(m_pPlayers->GetSelectionValue());
	CDigitanksWindow::Get()->SetTanks(m_pTanks->GetSelectionValue());
	CDigitanksWindow::Get()->CreateGame(GAMETYPE_ARTILLERY);

	if (!GameServer())
		return;

	if (CNetwork::IsHost() && DigitanksGame())
		DigitanksGame()->SetDifficulty(m_pDifficulty->GetSelectionValue());

	CDigitanksWindow::Get()->GetInstructor()->SetActive(false);
	CDigitanksWindow::Get()->GetMainMenu()->SetVisible(false);
}

CStrategyGamePanel::CStrategyGamePanel(bool bMultiplayer)
	: CPanel(0, 0, 570, 520)
{
	if (bMultiplayer)
		CDigitanksWindow::Get()->SetServerType(SERVER_HOST);
	else
		CDigitanksWindow::Get()->SetServerType(SERVER_LOCAL);

	m_pDifficulty = new CScrollSelector<int>();
	m_pDifficulty->AddSelection(CScrollSelection<int>(0, L"Easy"));
	m_pDifficulty->AddSelection(CScrollSelection<int>(1, L"Normal"));
	m_pDifficulty->SetSelection(1);
	AddControl(m_pDifficulty);

	m_pDifficultyLabel = new CLabel(0, 0, 32, 32, "Difficulty");
	m_pDifficultyLabel->SetWrap(false);
	AddControl(m_pDifficultyLabel);

	m_pPlayers = new CScrollSelector<int>();
	m_pPlayers->AddSelection(CScrollSelection<int>(2, L"2"));
	m_pPlayers->AddSelection(CScrollSelection<int>(3, L"3"));
	m_pPlayers->AddSelection(CScrollSelection<int>(4, L"4"));
	m_pPlayers->SetSelection(2);
	AddControl(m_pPlayers);

	m_pPlayersLabel = new CLabel(0, 0, 32, 32, "Players");
	m_pPlayersLabel->SetWrap(false);
	AddControl(m_pPlayersLabel);

	m_pBeginGame = new CButton(0, 0, 100, 100, "BEGIN!");
	m_pBeginGame->SetClickedListener(this, BeginGame);
	m_pBeginGame->SetFontFaceSize(12);
	AddControl(m_pBeginGame);
}

void CStrategyGamePanel::Layout()
{
	BaseClass::Layout();

	int iSelectorSize = m_pDifficultyLabel->GetHeight() - 4;

	m_pDifficultyLabel->EnsureTextFits();
	m_pDifficultyLabel->SetPos(75, 120);

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 120);

	m_pPlayersLabel->EnsureTextFits();
	m_pPlayersLabel->SetPos(75, 180);

	m_pPlayers->SetSize(GetWidth() - m_pPlayersLabel->GetLeft()*2 - m_pPlayersLabel->GetWidth(), iSelectorSize);
	m_pPlayers->SetPos(m_pPlayersLabel->GetRight(), 180);

	m_pBeginGame->SetSize(135, 40);
	m_pBeginGame->SetPos(GetWidth()/2-135/2, GetHeight()-160);
}

void CStrategyGamePanel::BeginGameCallback()
{
	CDigitanksWindow::Get()->SetPlayers(m_pPlayers->GetSelectionValue());
	CDigitanksWindow::Get()->CreateGame(GAMETYPE_STANDARD);

	if (!GameServer())
		return;

	if (CNetwork::IsHost() && DigitanksGame())
		DigitanksGame()->SetDifficulty(m_pDifficulty->GetSelectionValue());

	CDigitanksWindow::Get()->GetInstructor()->SetActive(false);
	CDigitanksWindow::Get()->GetMainMenu()->SetVisible(false);
}
