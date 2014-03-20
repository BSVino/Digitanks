#include "digitankswindow.h"

#include <glgui/rootpanel.h>
#include <tinker_platform.h>
#include <strutils.h>
#include <sockets/sockets.h>
#include <glgui/filedialog.h>
#include <ui/instructor.h>

#include <tengine/lobby/lobby_client.h>
#include <tengine/lobby/lobby_server.h>

#include <renderer/renderer.h>

#include "hud.h"
#include "menu.h"
#include "ui.h"
#include "lobbyui.h"

#define _T(x) x

using namespace glgui;

void CDigitanksWindow::InitUI()
{
	m_pMainMenu = CRootPanel::Get()->AddControl(new CMainMenu());
	m_pMenu = CRootPanel::Get()->AddControl(new CDigitanksMenu());
	m_pVictory = CRootPanel::Get()->AddControl(new CVictoryPanel());
	m_pStory = CRootPanel::Get()->AddControl(new CStoryPanel());
	m_pLobby = CRootPanel::Get()->AddControl(new CLobbyPanel());

	CRootPanel::Get()->Layout();

	CRootPanel::Get()->SetLighting(false);
}

void CDigitanksWindow::Layout()
{
	CRootPanel::Get()->Layout();
}

CDigitanksMenu::CDigitanksMenu()
	: CPanel(0, 0, 200, 300)
{
	m_pOptionsPanel = NULL;

	m_pDigitanks = AddControl(new CLabel(0, 0, 100, 100, _T("DIGITANKS")));
	m_pDigitanks->SetFont(_T("header"));

	m_pDifficulty = AddControl(new CScrollSelector<int>(_T("text")));
	m_pDifficulty->AddSelection(CScrollSelection<int>(0, _T("Easy")));
	m_pDifficulty->AddSelection(CScrollSelection<int>(1, _T("Normal")));
	m_pDifficulty->SetSelection(1);

	m_pDifficultyLabel = AddControl(new CLabel(0, 0, 32, 32, _T("Difficulty")));
	m_pDifficultyLabel->SetWrap(false);
	m_pDifficultyLabel->SetFont(_T("text"));

	m_pReturnToMenu = AddControl(new CButton(0, 0, 100, 100, _T("EXIT TO MENU")));
	m_pReturnToMenu->SetClickedListener(this, Exit);
	m_pReturnToMenu->SetFont(_T("header"));

	m_pReturnToGame = AddControl(new CButton(0, 0, 100, 100, _T("X")));
	m_pReturnToGame->SetClickedListener(this, Close);

	m_pSaveGame = AddControl(new CButton(0, 0, 100, 100, _T("SAVE GAME")));
	m_pSaveGame->SetClickedListener(this, Save);
	m_pSaveGame->SetFont(_T("header"));

	m_pLoadGame = AddControl(new CButton(0, 0, 100, 100, _T("LOAD GAME")));
	m_pLoadGame->SetClickedListener(this, Load);
	m_pLoadGame->SetFont(_T("header"));

	m_pOptions = AddControl(new CButton(0, 0, 100, 100, _T("OPTIONS")));
	m_pOptions->SetClickedListener(this, Options);
	m_pOptions->SetFont(_T("header"));

	m_pExit = AddControl(new CButton(0, 0, 100, 100, _T("QUIT TO DESKTOP")));
	m_pExit->SetClickedListener(this, Quit);
	m_pExit->SetFont(_T("header"));

	Layout();

	SetVisible(false);
}

void CDigitanksMenu::Layout()
{
	size_t iWidth = DigitanksWindow()->GetWindowWidth();
	size_t iHeight = DigitanksWindow()->GetWindowHeight();

	SetPos(iWidth/2-GetWidth()/2, iHeight/2-GetHeight()/2);

	m_pDigitanks->SetPos(0, 20);
	m_pDigitanks->SetSize(GetWidth(), GetHeight());
	m_pDigitanks->SetAlign(CLabel::TA_TOPCENTER);

	int iSelectorSize = m_pDifficultyLabel->GetHeight() - 4;

	m_pDifficultyLabel->EnsureTextFits();
	m_pDifficultyLabel->SetPos(25, 60);
	m_pDifficultyLabel->SetVisible(!GameNetwork()->IsConnected());

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 60);
	m_pDifficulty->SetVisible(!GameNetwork()->IsConnected());

	m_pSaveGame->SetPos(25, 130);
	m_pSaveGame->SetSize(150, 20);

	m_pLoadGame->SetPos(25, 160);
	m_pLoadGame->SetSize(150, 20);

	m_pOptions->SetPos(25, 190);
	m_pOptions->SetSize(150, 20);

	m_pReturnToMenu->SetPos(25, 220);
	m_pReturnToMenu->SetSize(150, 20);

	m_pReturnToGame->SetPos(GetWidth()-20, 10);
	m_pReturnToGame->SetSize(10, 10);
	m_pReturnToGame->SetButtonColor(Color(255, 0, 0));

	m_pExit->SetPos(25, 250);
	m_pExit->SetSize(150, 20);

	BaseClass::Layout();
}

void CDigitanksMenu::Paint(float x, float y, float w, float h)
{
	if (GameServer())
		CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 235));

	BaseClass::Paint(x, y, w, h);
}

void CDigitanksMenu::SetVisible(bool bVisible)
{
	m_pReturnToGame->SetVisible(!!GameServer());

	if (m_pOptionsPanel)
	{
		RemoveControl(m_pOptionsPanel);
		m_pOptionsPanel = NULL;
	}

	BaseClass::SetVisible(bVisible);
}

void CDigitanksMenu::CloseCallback(const tstring& sArgs)
{
	SetVisible(false);

	if (m_pOptionsPanel)
	{
		RemoveControl(m_pOptionsPanel);
		m_pOptionsPanel = NULL;
	}
}

void CDigitanksMenu::SaveCallback(const tstring& sArgs)
{
	glgui::CFileDialog::ShowSaveDialog(DigitanksWindow()->GetAppDataDirectory(), ".sav", this, SaveFile);
}

void CDigitanksMenu::SaveFileCallback(const tstring& sArgs)
{
	tstring sFilename = glgui::CFileDialog::GetFile();
	if (!sFilename.length())
		return;

	CGameServer::SaveToFile(sFilename.c_str());
}

void CDigitanksMenu::LoadCallback(const tstring& sArgs)
{
	if (!GameServer())
		DigitanksWindow()->CreateGame("empty");

	glgui::CFileDialog::ShowOpenDialog(DigitanksWindow()->GetAppDataDirectory(), ".sav", this, Open);
}

void CDigitanksMenu::OpenCallback(const tstring& sArgs)
{
	tstring sFilename = glgui::CFileDialog::GetFile();
	if (!sFilename.length())
		return;

	DigitanksWindow()->RenderLoading();

	if (CGameServer::LoadFromFile(sFilename.c_str()))
		SetVisible(false);
	else
	{
		DigitanksWindow()->DestroyGame();
		DigitanksWindow()->CreateGame("menu");
	}
}

void CDigitanksMenu::OptionsCallback(const tstring& sArgs)
{
	if (m_pOptionsPanel)
		RemoveControl(m_pOptionsPanel);

	m_pOptionsPanel = CRootPanel::Get()->AddControl(new COptionsPanel(), true);
	m_pOptionsPanel->SetStandalone(true);
	m_pOptionsPanel->Layout();
}

void CDigitanksMenu::ExitCallback(const tstring& sArgs)
{
	CGameLobbyClient::S_LeaveLobby();
	CGameLobbyServer::DestroyLobby(0);

	GameNetwork()->Disconnect();
	LobbyNetwork()->Disconnect();

	GameServer()->SetLoading(true);
	DigitanksWindow()->DestroyGame();
	DigitanksWindow()->CreateGame("menu");
	SetVisible(false);
	DigitanksWindow()->GetMainMenu()->SetVisible(true);
	GameServer()->SetLoading(false);
}

void CDigitanksMenu::QuitCallback(const tstring& sArgs)
{
	DigitanksWindow()->CloseApplication();
}

CVictoryPanel::CVictoryPanel()
	: CPanel(0, 0, 400, 300)
{
	m_pVictory = AddControl(new CLabel(0, 0, 100, 100, _T("")));
	m_pVictory->SetFont(_T("text"));

	m_pRestart = AddControl(new CButton(0, 0, 100, 100, _T("Restart")));
	m_pRestart->SetFont(_T("header"));

	SetVisible(false);

	Layout();
}

void CVictoryPanel::Layout()
{
	SetSize(550, 200);
	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_pVictory->SetPos(10, 20);
	m_pVictory->SetSize(GetWidth()-20, GetHeight());
	m_pVictory->SetAlign(CLabel::TA_TOPCENTER);

	m_pRestart->SetSize(80, 35);
	m_pRestart->SetPos(GetWidth()/2-m_pRestart->GetWidth()/2, GetHeight() - m_pRestart->GetHeight() - 20);
	m_pRestart->SetClickedListener(this, Restart);
}

void CVictoryPanel::Paint(float x, float y, float w, float h)
{
	CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 235));

	BaseClass::Paint(x, y, w, h);
}

void CVictoryPanel::GameOver(bool bPlayerWon)
{
	if (bPlayerWon)
		m_pVictory->SetText(_T("VICTORY!\n \nYou have crushed the weak and foolish under your merciless, unwavering treads. Your enemies bow before you as you stand - ruler of the Digiverse!\n \n"));
	else
		m_pVictory->SetText(_T("DEFEAT!\n \nYour ravenous enemies have destroyed your feeble tank armies. Database memories will recall the day when your once-glorious digital empire crumbled!\n \n"));

	if (DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN && bPlayerWon)
		m_pRestart->SetVisible(false);
	else
		m_pRestart->SetVisible(true);

	if (!GameNetwork()->IsHost())
		m_pRestart->SetVisible(false);

	SetVisible(true);
}

void CVictoryPanel::RestartCallback(const tstring& sArgs)
{
	DigitanksWindow()->RestartLevel();

	SetVisible(false);
}

CStoryPanel::CStoryPanel()
	: CPanel(0, 0, 400, 300)
{
	m_pStory = AddControl(new CLabel(0, 0, 100, 100,
		_T("THE STORY OF DIGIVILLE\n \n")

		_T("The Digizens of Digiville were happy and content.\n")
		_T("They ate in tiny bits and bytes and always paid their rent.\n")
		_T("They shared every Electronode and Data Wells were free,\n")
		_T("But that's not very interesting, as you're about to see.\n \n ")

		_T("One day the shortest Digizen in all the Digiverse\n")
		_T("He cried, \"U nubs OLOL i h4x j0ur m3g4hu|2tz!\"\n")
		_T("The Digizens of Digiville just laughed and said, \"That's great!\"\n")
		_T("\"You're way too short and you're just trying to overcompensate!\"\n \n")

		_T("Our little man was not so pleased, retreating to his lair\n")
		_T("He powered up his Trolling Rage Machine with utmost flair.\n")
		_T("It sputtered up to life with a cacophony of clanks\n")
		_T("And shortly then thereafter it began to spit out tanks.\n \n")

		_T("The Digizens were sleeping when there came a sudden chill\n")
		_T("And when they woke there was no longer any Digiville.\n")
		_T("It's up to you now! You must act before it gets much worse,\n")
		_T("And while you're there, why not conquer the whole damn Digiverse?\n \n")

		_T("Click here to begin.")
		));
	m_pStory->SetFont(_T("text"));

	SetVisible(false);

	Layout();
}

void CStoryPanel::Layout()
{
	SetSize(500, 400);
	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_pStory->SetPos(10, 20);
	m_pStory->SetSize(GetWidth()-20, GetHeight());
	m_pStory->SetAlign(CLabel::TA_TOPCENTER);
}

void CStoryPanel::Paint(float x, float y, float w, float h)
{
	CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 235));

	BaseClass::Paint(x, y, w, h);
}

bool CStoryPanel::MousePressed(int code, int mx, int my)
{
	if (BaseClass::MousePressed(code, mx, my))
		return true;

	SetVisible(false);

	// Now that it's closed, run the tutorial!
	CInstructor* pInstructor = DigitanksWindow()->GetInstructor();
	pInstructor->SetActive(true);
	pInstructor->Initialize();
	pInstructor->DisplayFirstLesson("strategy-select");

	return true;
}

bool CStoryPanel::KeyPressed(int iKey, bool bCtrlDown)
{
	SetVisible(false);

	// Now that it's closed, run the tutorial!
	CInstructor* pInstructor = DigitanksWindow()->GetInstructor();
	pInstructor->SetActive(true);
	pInstructor->Initialize();
	pInstructor->DisplayFirstLesson("strategy-select");

	// Pass the keypress through so that the menu opens.
	return false;
}

