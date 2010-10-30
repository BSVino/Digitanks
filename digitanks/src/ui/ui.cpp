#include "digitankswindow.h"

#include <glgui/glgui.h>
#include <platform.h>

#include "hud.h"
#include "menu.h"
#include "ui.h"
#include "instructor.h"
#include "register.h"

using namespace glgui;

void CDigitanksWindow::InitUI()
{
	m_pMainMenu = new CMainMenu();
	m_pMenu = new CDigitanksMenu();
	m_pVictory = new CVictoryPanel();
	m_pPurchase = new CPurchasePanel();
	m_pStory = new CStoryPanel();

	CRootPanel::Get()->AddControl(m_pMainMenu);
	CRootPanel::Get()->AddControl(m_pMenu);
	CRootPanel::Get()->AddControl(m_pVictory);
	CRootPanel::Get()->AddControl(m_pPurchase);
	CRootPanel::Get()->AddControl(m_pStory);

	CRootPanel::Get()->Layout();

	CRootPanel::Get()->SetLighting(false);
}

void CDigitanksWindow::Layout()
{
	CRootPanel::Get()->Layout();
}

CDigitanksMenu::CDigitanksMenu()
	: CPanel(0, 0, 300, 460)
{
	m_pDigitanks = new CLabel(0, 0, 100, 100, "Copyright © 2010, Lunar Workshop\nJorge Rodriguez <jorge@lunarworkshop.net>\n \nhttp://digitanks.com\nhttp://lunarworkshop.net");
	AddControl(m_pDigitanks);

	m_pDifficulty = new CScrollSelector<int>();
	m_pDifficulty->AddSelection(CScrollSelection<int>(0, L"Easy"));
	m_pDifficulty->AddSelection(CScrollSelection<int>(1, L"Normal"));
	m_pDifficulty->SetSelection(1);
	AddControl(m_pDifficulty);

	m_pDifficultyLabel = new CLabel(0, 0, 32, 32, "Difficulty");
	m_pDifficultyLabel->SetWrap(false);
	AddControl(m_pDifficultyLabel);

	m_pReturnToMenu = new CButton(0, 0, 100, 100, "Exit Game");
	m_pReturnToMenu->SetClickedListener(this, Exit);
	AddControl(m_pReturnToMenu);

	m_pReturnToGame = new CButton(0, 0, 100, 100, "Return To Game");
	m_pReturnToGame->SetClickedListener(this, Close);
	AddControl(m_pReturnToGame);

	m_pSaveGame = new CButton(0, 0, 100, 100, "Save Game");
	m_pSaveGame->SetClickedListener(this, Save);
	AddControl(m_pSaveGame);

	m_pLoadGame = new CButton(0, 0, 100, 100, "Load Game");
	m_pLoadGame->SetClickedListener(this, Load);
	AddControl(m_pLoadGame);

	m_pExit = new CButton(0, 0, 100, 100, "Quit To Desktop");
	m_pExit->SetClickedListener(this, Quit);
	AddControl(m_pExit);

	Layout();

	SetVisible(false);
}

void CDigitanksMenu::Layout()
{
	size_t iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	size_t iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	SetPos(iWidth/2-GetWidth()/2, iHeight/2-GetHeight()/2);

	m_pDigitanks->SetPos(0, 20);
	m_pDigitanks->SetSize(GetWidth(), GetHeight());
	m_pDigitanks->SetAlign(CLabel::TA_TOPCENTER);

	int iSelectorSize = m_pDifficultyLabel->GetHeight() - 4;

	m_pDifficultyLabel->EnsureTextFits();
	m_pDifficultyLabel->SetPos(75, 220);

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 220);

	m_pSaveGame->SetPos(100, 300);
	m_pSaveGame->SetSize(100, 20);

	m_pLoadGame->SetPos(100, 330);
	m_pLoadGame->SetSize(100, 20);

	m_pReturnToMenu->SetPos(100, 360);
	m_pReturnToMenu->SetSize(100, 20);
	m_pReturnToMenu->SetVisible(!!GameServer());

	m_pReturnToGame->SetPos(100, 390);
	m_pReturnToGame->SetSize(100, 20);
	m_pReturnToGame->SetVisible(!!GameServer());

	m_pExit->SetPos(100, 420);
	m_pExit->SetSize(100, 20);

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
	if (CDigitanksWindow::Get()->GetInstructor())
	{
		if (bVisible)
			CDigitanksWindow::Get()->GetInstructor()->HideTutorial();
		else
			CDigitanksWindow::Get()->GetInstructor()->ShowTutorial();
	}

	m_pReturnToGame->SetVisible(!!GameServer());

	BaseClass::SetVisible(bVisible);
}

void CDigitanksMenu::CloseCallback()
{
	SetVisible(false);
}

void CDigitanksMenu::SaveCallback()
{
	CGameServer::SaveToFile(L"digitanks.sav");
}

void CDigitanksMenu::LoadCallback()
{
	if (!GameServer())
		CDigitanksWindow::Get()->CreateGame(GAMETYPE_EMPTY);

	if (CGameServer::LoadFromFile(L"digitanks.sav"))
		SetVisible(false);
	else
	{
		CDigitanksWindow::Get()->DestroyGame();
		CDigitanksWindow::Get()->CreateGame(GAMETYPE_MENU);
	}
}

void CDigitanksMenu::ExitCallback()
{
	CDigitanksWindow::Get()->DestroyGame();
	CDigitanksWindow::Get()->CreateGame(GAMETYPE_MENU);
	SetVisible(false);
	CDigitanksWindow::Get()->GetMainMenu()->SetVisible(true);
}

void CDigitanksMenu::QuitCallback()
{
	CDigitanksWindow::Get()->CloseApplication();
}

CVictoryPanel::CVictoryPanel()
	: CPanel(0, 0, 400, 300)
{
	m_pVictory = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pVictory);

	SetVisible(false);

	Layout();
}

void CVictoryPanel::Layout()
{
	SetSize(250, 250);
	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_pVictory->SetPos(10, 20);
	m_pVictory->SetSize(GetWidth()-20, GetHeight());
	m_pVictory->SetAlign(CLabel::TA_TOPCENTER);
}

void CVictoryPanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 235));

	BaseClass::Paint(x, y, w, h);
}

bool CVictoryPanel::MousePressed(int code, int mx, int my)
{
	if (BaseClass::MousePressed(code, mx, my))
		return true;

	SetVisible(false);
	return true;
}

bool CVictoryPanel::KeyPressed(int iKey)
{
	SetVisible(false);
	// Pass the keypress through so that the menu opens.
	return false;
}

void CVictoryPanel::GameOver(bool bPlayerWon)
{
	if (bPlayerWon)
		m_pVictory->SetText(L"VICTORY!\n \nYou have crushed the weak and foolish under your merciless, unwavering treads. Your enemies bow before you as you stand - ruler of the Digiverse!\n \n");
	else
		m_pVictory->SetText(L"DEFEAT!\n \nYour ravenous enemies have destroyed your feeble tank armies. Database memories will recall the day when your once-glorious digital empire crumbled!\n \n");

	if (CNetwork::IsConnected())
		m_pVictory->AppendText(L"Thanks for playing Digitanks. You can now spectate while the remaining players finish their game.");
	else
		m_pVictory->AppendText(L"Thanks for playing Digitanks. Return to the main menu to start a new game.");

	SetVisible(true);
}

CPurchasePanel::CPurchasePanel()
	: CPanel(0, 0, 400, 300)
{
	m_pPurchase = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pPurchase);

	m_pRegistrationKey = new CTextField();
	AddControl(m_pRegistrationKey);

	m_pRegister = new CButton(0, 0, 100, 100, "Register");
	m_pRegister->SetClickedListener(this, Register);
	AddControl(m_pRegister);

	m_pRegisterResult = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pRegisterResult);

	m_pRegisterOffline = new CButton(0, 0, 100, 100, "Register Offline");
	m_pRegisterOffline->SetFontFaceSize(11);
	m_pRegisterOffline->SetClickedListener(this, RegisterOffline);
	AddControl(m_pRegisterOffline);

	m_pPurchaseButton = new CButton(0, 0, 100, 100, "Website!");
	m_pPurchaseButton->SetClickedListener(this, Purchase);
	AddControl(m_pPurchaseButton);

	m_pExitButton = new CButton(0, 0, 100, 100, "Maybe later");
	AddControl(m_pExitButton);

	SetVisible(false);

	Layout();
}

void CPurchasePanel::Layout()
{
	SetSize(500, 300);
	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_pPurchase->SetPos(10, 20);
	m_pPurchase->SetSize(GetWidth()-20, GetHeight());
	m_pPurchase->SetAlign(CLabel::TA_TOPCENTER);

	m_pRegistrationKey->SetPos(GetWidth()/2 - m_pRegistrationKey->GetWidth()/2, 100);

	m_pRegister->SetSize(100, 30);
	m_pRegister->SetPos(GetWidth()/2 - m_pRegister->GetWidth()/2, 140);

	m_pRegisterResult->SetPos(10, 180);
	m_pRegisterResult->SetSize(GetWidth()-20, GetHeight());
	m_pRegisterResult->SetAlign(CLabel::TA_TOPCENTER);

	m_pRegisterOffline->SetSize(60, 40);
	m_pRegisterOffline->SetPos(GetWidth()-80, GetHeight()-100);

	m_pPurchaseButton->SetSize(100, 30);
	m_pPurchaseButton->SetPos(GetWidth()/2-150, GetHeight() - 50);

	m_pExitButton->SetSize(100, 30);
	m_pExitButton->SetPos(GetWidth()/2+50, GetHeight() - 50);
}

void CPurchasePanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 255));

	BaseClass::Paint(x, y, w, h);
}

void CPurchasePanel::ClosingApplication()
{
	m_pPurchase->SetText(L"SUPPORT THE WAR EFFORT!\n \n"
		L"What you've seen is just the beginning for Digitanks. Buying and registering your copy can help us add:\n \n"
		L" * More tanks\n"
		L" * More weapons\n"
		L" * More scenarios\n"
		L" * More structures to build and conquer\n"
		L" * And hopefully not more bugs!\n \n"
		L"All that stuff is really hard though, and we need your help to build it.");

	SetVisible(true);
	m_pRegistrationKey->SetVisible(false);
	m_pRegister->SetVisible(false);
	m_pRegisterOffline->SetVisible(false);
	m_pRegisterResult->SetVisible(false);

	m_pExitButton->SetClickedListener(this, Exit);

	CDigitanksWindow::Get()->GetInstructor()->SetActive(false);
	CDigitanksWindow::Get()->GetVictoryPanel()->SetVisible(false);
}

void CPurchasePanel::OpeningApplication()
{
	m_pPurchase->SetText(L"REGISTER DIGITANKS!\n \n"
		L"If you've purchased the game, please paste your registration key into the box below. Otherwise please enjoy this demo with a 50 move turn limit.");

	SetVisible(true);
	m_pRegistrationKey->SetVisible(true);
	m_pRegister->SetVisible(true);

	m_pExitButton->SetClickedListener(this, MainMenu);

	CDigitanksWindow::Get()->GetInstructor()->SetActive(false);
	CDigitanksWindow::Get()->GetVictoryPanel()->SetVisible(false);
}

void CPurchasePanel::PurchaseCallback()
{
	OpenBrowser(L"http://digitanks.com/gamelanding/");
	exit(0);
}

void CPurchasePanel::ExitCallback()
{
	exit(0);
}

void CPurchasePanel::MainMenuCallback()
{
	SetVisible(false);
	CDigitanksWindow::Get()->GetMainMenu()->SetVisible(true);
}

void CPurchasePanel::RegisterCallback()
{
	std::wstring sError;
	bool bSucceeded = QueryRegistrationKey(m_pRegistrationKey->GetText(), sError);
	m_pRegisterResult->SetText(sError.c_str());

	if (bSucceeded)
	{
		m_pRegister->SetVisible(false);
		m_pRegisterOffline->SetVisible(false);
		m_pRegistrationKey->SetVisible(false);
		m_pExitButton->SetText(L"Awesome!");
	}
}

void CPurchasePanel::RegisterOfflineCallback()
{
	m_pRegistrationKey->SetSize(400, m_pRegister->GetHeight());
	m_pRegistrationKey->SetPos(GetWidth()/2 - m_pRegistrationKey->GetWidth()/2, 140);
	m_pRegister->SetPos(GetWidth()/2 - m_pRegister->GetWidth()/2, 180);
	m_pRegister->SetClickedListener(this, SetKey);
	m_pRegisterResult->SetPos(10, 220);

	m_pRegisterOffline->SetSize(60, 20);
	m_pRegisterOffline->SetText(L"Copy");
	m_pRegisterOffline->SetPos(GetWidth()-80, 100);
	m_pRegisterOffline->SetClickedListener(this, CopyProductCode);

	m_pProductCode = new CLabel(0, 110, GetWidth(), GetHeight(), "");
	m_pProductCode->SetAlign(CLabel::TA_TOPCENTER);
	m_pProductCode->SetText(L"Product Code: ");
	m_pProductCode->AppendText(GetProductCode().c_str());
	AddControl(m_pProductCode);
}

void CPurchasePanel::CopyProductCodeCallback()
{
	SetClipboard(GetProductCode());
}

void CPurchasePanel::SetKeyCallback()
{
	std::wstring sText = m_pRegistrationKey->GetText();
	std::string sKey;
	sKey.assign(sText.begin(), sText.end());
	SetLicenseKey(sKey);

	if (IsRegistered())
	{
		m_pRegisterResult->SetText(L"Thank you for registering Digitanks!");
		m_pRegister->SetVisible(false);
		m_pRegisterOffline->SetVisible(false);
		m_pRegistrationKey->SetVisible(false);
		m_pProductCode->SetVisible(false);
		m_pExitButton->SetText(L"Awesome!");
	}
	else
		m_pRegisterResult->SetText(L"Sorry, that key didn't seem to work. Try again!");
}

CStoryPanel::CStoryPanel()
	: CPanel(0, 0, 400, 300)
{
	m_pStory = new CLabel(0, 0, 100, 100,
		"THE STORY OF DIGIVILLE\n \n"

		"The Digizens of Digiville were happy and content.\n"
		"They ate in tiny bits and bytes and always paid their rent.\n"
		"They shared every Electronode and Data Wells were free,\n"
		"But that's not very interesting, as you're about to see.\n \n "

		"One day the shortest Digizen in all the Digiverse\n"
		"He cried, \"U nubs OLOL i h4x j0ur m3g4hu|2tz!\"\n"
		"The Digizens of Digiville just laughed and said, \"That's great!\"\n"
		"\"You're way too short and you're just trying to overcompensate!\"\n \n"

		"Our little man was not so pleased, retreating to his lair\n"
		"He powered up his Trolling Rage Machine with utmost flair.\n"
		"It sputtered up to life with a cacophony of clanks\n"
		"And shortly then thereafter it began to spit out tanks.\n \n"

		"The Digizens were sleeping when there came a sudden chill\n"
		"And when they woke there was no longer any Digiville.\n"
		"It's up to you now! You must act before it gets much worse,\n"
		"And while you're there, why not conquer the whole damn Digiverse?\n \n"

		"Click here to begin."
		);
	AddControl(m_pStory);

	SetVisible(false);

	Layout();
}

void CStoryPanel::Layout()
{
	SetSize(450, 450);
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

	// Now that it's closed, open our first action item!
	DigitanksGame()->AllowActionItems(true);
	DigitanksGame()->AddActionItem(NULL, ACTIONTYPE_WELCOME);
	DigitanksGame()->AllowActionItems(false);
	CDigitanksWindow::Get()->GetHUD()->ShowFirstActionItem();

	return true;
}

bool CStoryPanel::KeyPressed(int iKey)
{
	SetVisible(false);
	// Pass the keypress through so that the menu opens.
	return false;
}

