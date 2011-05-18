#include "digitankswindow.h"

#include <glgui/glgui.h>
#include <platform.h>
#include <strutils.h>
#include <sockets/sockets.h>

#include <renderer/renderer.h>

#include "hud.h"
#include "menu.h"
#include "ui.h"
#include "instructor.h"
#include "lobbyui.h"

using namespace glgui;

void CDigitanksWindow::InitUI()
{
	m_pMainMenu = new CMainMenu();
	m_pMenu = new CDigitanksMenu();
	m_pVictory = new CVictoryPanel();
	m_pPurchase = new CPurchasePanel();
	m_pStory = new CStoryPanel();
	m_pLobby = new CLobbyPanel();

	CRootPanel::Get()->AddControl(m_pMainMenu);
	CRootPanel::Get()->AddControl(m_pMenu);
	CRootPanel::Get()->AddControl(m_pVictory);
	CRootPanel::Get()->AddControl(m_pPurchase);
	CRootPanel::Get()->AddControl(m_pStory);
	CRootPanel::Get()->AddControl(m_pLobby);

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

	m_pDigitanks = new CLabel(0, 0, 100, 100, L"DIGITANKS");
	m_pDigitanks->SetFont(L"header");
	AddControl(m_pDigitanks);

	m_pDifficulty = new CScrollSelector<int>(L"text");
	m_pDifficulty->AddSelection(CScrollSelection<int>(0, L"Easy"));
	m_pDifficulty->AddSelection(CScrollSelection<int>(1, L"Normal"));
	m_pDifficulty->SetSelection(1);
	AddControl(m_pDifficulty);

	m_pDifficultyLabel = new CLabel(0, 0, 32, 32, L"Difficulty");
	m_pDifficultyLabel->SetWrap(false);
	m_pDifficultyLabel->SetFont(L"text");
	AddControl(m_pDifficultyLabel);

	m_pReturnToMenu = new CButton(0, 0, 100, 100, L"EXIT TO MENU");
	m_pReturnToMenu->SetClickedListener(this, Exit);
	m_pReturnToMenu->SetFont(L"header");
	AddControl(m_pReturnToMenu);

	m_pReturnToGame = new CButton(0, 0, 100, 100, L"X");
	m_pReturnToGame->SetClickedListener(this, Close);
	AddControl(m_pReturnToGame);

	m_pSaveGame = new CButton(0, 0, 100, 100, L"SAVE GAME");
	m_pSaveGame->SetClickedListener(this, Save);
	m_pSaveGame->SetFont(L"header");
#if !defined(TINKER_UNLOCKED)
	m_pSaveGame->SetEnabled(false);
#endif
	AddControl(m_pSaveGame);

	m_pLoadGame = new CButton(0, 0, 100, 100, L"LOAD GAME");
	m_pLoadGame->SetClickedListener(this, Load);
	m_pLoadGame->SetFont(L"header");
#if !defined(TINKER_UNLOCKED)
	m_pLoadGame->SetEnabled(false);
#endif
	AddControl(m_pLoadGame);

	m_pOptions = new CButton(0, 0, 100, 100, L"OPTIONS");
	m_pOptions->SetClickedListener(this, Options);
	m_pOptions->SetFont(L"header");
	AddControl(m_pOptions);

	m_pExit = new CButton(0, 0, 100, 100, L"QUIT TO DESKTOP");
	m_pExit->SetClickedListener(this, Quit);
	m_pExit->SetFont(L"header");
	AddControl(m_pExit);

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
	m_pDifficultyLabel->SetVisible(!CNetwork::IsConnected());

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 60);
	m_pDifficulty->SetVisible(!CNetwork::IsConnected());

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

void CDigitanksMenu::Paint(int x, int y, int w, int h)
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
		m_pOptionsPanel->Destructor();
		m_pOptionsPanel->Delete();
		m_pOptionsPanel = NULL;
	}

	BaseClass::SetVisible(bVisible);
}

void CDigitanksMenu::CloseCallback()
{
	SetVisible(false);

	if (m_pOptionsPanel)
	{
		m_pOptionsPanel->Destructor();
		m_pOptionsPanel->Delete();
		m_pOptionsPanel = NULL;
	}
}

void CDigitanksMenu::SaveCallback()
{
#if !defined(TINKER_UNLOCKED)
	return;
#endif

	wchar_t* pszFilename = SaveFileDialog(L"Save Games *.sav\0*.sav\0");
	if (!pszFilename)
		return;

	CGameServer::SaveToFile(pszFilename);
}

void CDigitanksMenu::LoadCallback()
{
#if !defined(TINKER_UNLOCKED)
	return;
#endif

	if (!GameServer())
		DigitanksWindow()->CreateGame(GAMETYPE_EMPTY);

	wchar_t* pszFilename = OpenFileDialog(L"Save Games *.sav\0*.sav\0");
	if (!pszFilename)
		return;

	DigitanksWindow()->RenderLoading();

	if (CGameServer::LoadFromFile(pszFilename))
		SetVisible(false);
	else
	{
		DigitanksWindow()->DestroyGame();
		DigitanksWindow()->CreateGame(GAMETYPE_MENU);
	}
}

void CDigitanksMenu::OptionsCallback()
{
	if (m_pOptionsPanel)
	{
		m_pOptionsPanel->Destructor();
		m_pOptionsPanel->Delete();
	}

	m_pOptionsPanel = new COptionsPanel();
	CRootPanel::Get()->AddControl(m_pOptionsPanel, true);
	m_pOptionsPanel->SetStandalone(true);
	m_pOptionsPanel->Layout();
}

void CDigitanksMenu::ExitCallback()
{
	DigitanksWindow()->DestroyGame();
	DigitanksWindow()->CreateGame(GAMETYPE_MENU);
	SetVisible(false);
	DigitanksWindow()->GetMainMenu()->SetVisible(true);
}

void CDigitanksMenu::QuitCallback()
{
	DigitanksWindow()->CloseApplication();
}

CVictoryPanel::CVictoryPanel()
	: CPanel(0, 0, 400, 300)
{
	m_pVictory = new CLabel(0, 0, 100, 100, L"");
	m_pVictory->SetFont(L"text");
	AddControl(m_pVictory);

	m_pRestart = new CButton(0, 0, 100, 100, L"Restart");
	m_pRestart->SetFont(L"header");
	AddControl(m_pRestart);

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

void CVictoryPanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 235));

	BaseClass::Paint(x, y, w, h);
}

void CVictoryPanel::GameOver(bool bPlayerWon)
{
	if (bPlayerWon)
		m_pVictory->SetText(L"VICTORY!\n \nYou have crushed the weak and foolish under your merciless, unwavering treads. Your enemies bow before you as you stand - ruler of the Digiverse!\n \n");
	else
		m_pVictory->SetText(L"DEFEAT!\n \nYour ravenous enemies have destroyed your feeble tank armies. Database memories will recall the day when your once-glorious digital empire crumbled!\n \n");

	if (DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN && bPlayerWon)
		m_pRestart->SetVisible(false);
	else
		m_pRestart->SetVisible(true);

	SetVisible(true);
}

void CVictoryPanel::RestartCallback()
{
	DigitanksWindow()->Restart(GAMETYPE_FROM_CVAR);

	SetVisible(false);
}

CPurchasePanel::CPurchasePanel()
	: CPanel(0, 0, 400, 300)
{
	m_pPurchase = new CLabel(0, 0, 100, 100, L"");
	m_pPurchase->SetFont(L"text", 18);
	AddControl(m_pPurchase);

	m_pPurchaseButton = new CButton(0, 0, 100, 100, L"Buy now!");
	m_pPurchaseButton->SetClickedListener(this, Purchase);
	m_pPurchaseButton->SetFont(L"header");
	AddControl(m_pPurchaseButton);

	m_pEnterEmail = new CLabel(0, 0, 100, 100, L"");
	m_pEnterEmail->SetFont(L"text", 18);
	AddControl(m_pEnterEmail);

	m_pEmail = new CTextField();
	AddControl(m_pEmail);

	m_pContinueButton = new CButton(0, 0, 100, 100, L"Maybe later");
	m_pContinueButton->SetFont(L"header");
	AddControl(m_pContinueButton);

	SetVisible(false);

	Layout();
}

void CPurchasePanel::Layout()
{
	SetSize(580, 580);
	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_pPurchase->SetPos(30, 50);
	m_pPurchase->SetSize(GetWidth()-60, 200);
	m_pPurchase->SetAlign(CLabel::TA_TOPCENTER);

	m_pPurchaseButton->SetSize(100, 30);
	m_pPurchaseButton->SetPos(385, 170);
	m_pPurchaseButton->SetButtonColor(Color(50, 200, 10));

	m_pEnterEmail->SetPos(30, 340);
	m_pEnterEmail->SetSize(GetWidth()-60, 200);
	m_pEnterEmail->SetAlign(CLabel::TA_TOPCENTER);
	m_pEnterEmail->SetText(L"NEWS FROM THE FRONT\n \n"
		L"Enter your email to get the\nLunar Workshop Newsletter!");

	m_pEmail->SetPos(GetWidth()/2 - m_pEmail->GetWidth()/2, 450);
	m_pEmail->SetSize(200, m_pEmail->GetHeight());

	m_pContinueButton->SetSize(100, 30);
	m_pContinueButton->SetPos(385, 500);
	m_pContinueButton->SetButtonColor(Color(50, 200, 10));

	BaseClass::Layout();

	m_flTimeOpened = 0;
}

void CPurchasePanel::Think()
{
	if (!IsVisible())
		return;

	if (m_flTimeOpened == 0 && GameServer())
		m_flTimeOpened = GameServer()->GetGameTime();

	BaseClass::Think();

	if (m_pEmail->GetText().length())
	{
		m_pContinueButton->SetText("Sign me up!");
		m_pContinueButton->SetClickedListener(this, Email);
	}
	else
	{
		m_pContinueButton->SetText("Maybe later");
		if (m_bClosing)
			m_pContinueButton->SetClickedListener(this, Exit);
		else
			m_pContinueButton->SetClickedListener(this, MainMenu);
	}
}

void CPurchasePanel::Paint(int x, int y, int w, int h)
{
	CRenderingContext c(GameServer()->GetRenderer());
	c.SetBlend(BLEND_ALPHA);

	size_t iPurchasePanel = DigitanksWindow()->GetHUD()->GetPurchasePanel();
	glgui::CRootPanel::PaintTexture(iPurchasePanel, glgui::CRootPanel::Get()->GetWidth()/2 - 512, glgui::CRootPanel::Get()->GetHeight()/2 - 512, 1024, 1024);

	float flTimeOpen = GameServer()->GetGameTime() - m_flTimeOpened;
	float flSlide = Lerp(RemapValClamped(flTimeOpen, 0, 3, 1, 0), 0.2f);
	float flAlpha = RemapValClamped(flTimeOpen, 0, 3, 0, 1);
	float flSlideDistance = 400;

	size_t iTotalWidth = DigitanksWindow()->GetWindowWidth();
	size_t iTotalHeight = DigitanksWindow()->GetWindowHeight();

	size_t iTankWidth = iTotalWidth/4;
	size_t iTankHeight = iTankWidth/2;

	PaintSheet(CHUD::GetActionTanksSheet(), x - iTankWidth + 40 - (int)(flSlide*flSlideDistance), y + 20, iTankWidth, iTankHeight, 0, 256, 512, 256, 512, 512, Color(255, 255, 255, (int)(flAlpha*255)));
	PaintSheet(CHUD::GetActionTanksSheet(), x + w - 50 + (int)(flSlide*flSlideDistance), y + 20, iTankWidth, iTankHeight, 0, 0, 512, 256, 512, 512, Color(255, 255, 255, (int)(flAlpha*255)));

	CRootPanel::PaintTexture(DigitanksWindow()->GetLunarWorkshopLogo(), glgui::CRootPanel::Get()->GetWidth()-200-20, glgui::CRootPanel::Get()->GetHeight()-200, 200, 200);

	BaseClass::Paint(x, y, w, h);
}

void CPurchasePanel::ClosingApplication()
{
	m_bClosing = true;

	m_pPurchase->SetText(L"LIKE WHAT YOU SEE?\n \n"
		L"Use the discount code DIGIDEMO to\nget 25% off the full price");

	m_pEmail->SetText(L"");

	SetVisible(true);

	m_pContinueButton->SetClickedListener(this, Exit);

	DigitanksWindow()->GetInstructor()->SetActive(false);
	DigitanksWindow()->GetVictoryPanel()->SetVisible(false);
}

void CPurchasePanel::OpeningApplication()
{
	m_bClosing = false;

	m_pPurchase->SetText(L"AVAILABLE NOW\n \n"
		L"The best way to buy indie games\nis direct from the developer.\n \nIt's the best $12.99 you'll spend this week, I promise.");

	m_pEmail->SetText(L"");

	SetVisible(true);

	m_pContinueButton->SetClickedListener(this, MainMenu);

	DigitanksWindow()->GetInstructor()->SetActive(false);
	DigitanksWindow()->GetVictoryPanel()->SetVisible(false);
}

void CPurchasePanel::PurchaseCallback()
{
	OpenBrowser(sprintf(L"http://digitanks.com/gamelanding/?a=p&i=%d", DigitanksWindow()->GetInstallID()));
	exit(0);
}

void CPurchasePanel::ExitCallback()
{
	OpenBrowser(sprintf(L"http://digitanks.com/gamelanding/?a=e&i=%d", DigitanksWindow()->GetInstallID()));
	exit(0);
}

void CPurchasePanel::MainMenuCallback()
{
	SetVisible(false);
	DigitanksWindow()->GetMainMenu()->SetVisible(true);
}

void CPurchasePanel::EmailCallback()
{
	eastl::string16 sEmail = m_pEmail->GetText();
	sEmail = str_replace(sEmail, L"@", L"%40");
	sEmail = str_replace(sEmail, L"+", L"%2B");
	eastl::string16 sURI = sprintf(L"/reg/email.php?e=%s&i=%d", sEmail, DigitanksWindow()->GetInstallID());

	CHTTPPostSocket s("reg.lunarworkshop.com");
	s.SendHTTP11(convertstring<char16_t, char>(sURI).c_str());
	// Don't care about the output.
	s.Close();

	if (m_bClosing)
	{
		OpenBrowser(sprintf(L"http://digitanks.com/gamelanding/?a=e&i=%d", DigitanksWindow()->GetInstallID()));
		exit(0);
	}
	else
	{
		SetVisible(false);
		DigitanksWindow()->GetMainMenu()->SetVisible(true);
	}
}

CStoryPanel::CStoryPanel()
	: CPanel(0, 0, 400, 300)
{
	m_pStory = new CLabel(0, 0, 100, 100,
		L"THE STORY OF DIGIVILLE\n \n"

		L"The Digizens of Digiville were happy and content.\n"
		L"They ate in tiny bits and bytes and always paid their rent.\n"
		L"They shared every Electronode and Data Wells were free,\n"
		L"But that's not very interesting, as you're about to see.\n \n "

		L"One day the shortest Digizen in all the Digiverse\n"
		L"He cried, \"U nubs OLOL i h4x j0ur m3g4hu|2tz!\"\n"
		L"The Digizens of Digiville just laughed and said, \"That's great!\"\n"
		L"\"You're way too short and you're just trying to overcompensate!\"\n \n"

		L"Our little man was not so pleased, retreating to his lair\n"
		L"He powered up his Trolling Rage Machine with utmost flair.\n"
		L"It sputtered up to life with a cacophony of clanks\n"
		L"And shortly then thereafter it began to spit out tanks.\n \n"

		L"The Digizens were sleeping when there came a sudden chill\n"
		L"And when they woke there was no longer any Digiville.\n"
		L"It's up to you now! You must act before it gets much worse,\n"
		L"And while you're there, why not conquer the whole damn Digiverse?\n \n"

		L"Click here to begin."
		);
	m_pStory->SetFont(L"text");
	AddControl(m_pStory);

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

void CStoryPanel::Paint(int x, int y, int w, int h)
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
	pInstructor->DisplayFirstTutorial("strategy-select");

	return true;
}

bool CStoryPanel::KeyPressed(int iKey, bool bCtrlDown)
{
	SetVisible(false);

	// Now that it's closed, run the tutorial!
	CInstructor* pInstructor = DigitanksWindow()->GetInstructor();
	pInstructor->SetActive(true);
	pInstructor->Initialize();
	pInstructor->DisplayFirstTutorial("strategy-select");

	// Pass the keypress through so that the menu opens.
	return false;
}

