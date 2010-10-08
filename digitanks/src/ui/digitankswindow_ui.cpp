#include "digitankswindow.h"

#include <glgui/glgui.h>
#include <digitanks/digitanksgame.h>
#include <platform.h>
#include <renderer/renderer.h>

#include "hud.h"
#include "menu.h"
#include "instructor.h"

using namespace glgui;

void CDigitanksWindow::InitUI()
{
	m_pMenu = new CDigitanksMenu();
	m_pVictory = new CVictoryPanel();
	m_pDonate = new CDonatePanel();
	m_pStory = new CStoryPanel();

	CRootPanel::Get()->AddControl(m_pMenu);
	CRootPanel::Get()->AddControl(m_pVictory);
	CRootPanel::Get()->AddControl(m_pDonate);
	CRootPanel::Get()->AddControl(m_pStory);

	CRootPanel::Get()->Layout();

	CRootPanel::Get()->SetLighting(false);
}

void CDigitanksWindow::Layout()
{
	CRootPanel::Get()->Layout();
}

CDigitanksMenu::CDigitanksMenu()
	: CPanel(0, 0, 300, 400)
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

	m_pStartTutorialBasics = new CButton(0, 0, 100, 100, "Tutorial - Basics");
	m_pStartTutorialBasics->SetClickedListener(this, StartTutorialBasics);
	AddControl(m_pStartTutorialBasics);

	m_pStartTutorialBases = new CButton(0, 0, 100, 100, "Tutorial - Bases");
	m_pStartTutorialBases->SetClickedListener(this, StartTutorialBases);
	AddControl(m_pStartTutorialBases);

	m_pStartArtilleryGame = new CButton(0, 0, 100, 100, "New Artillery Game");
	m_pStartArtilleryGame->SetClickedListener(this, StartArtilleryGame);
	AddControl(m_pStartArtilleryGame);

	m_pStartBasesGame = new CButton(0, 0, 100, 100, "New Strategy Game");
	m_pStartBasesGame->SetClickedListener(this, StartBasesGame);
	AddControl(m_pStartBasesGame);

	m_pReturnToGame = new CButton(0, 0, 100, 100, "Return To Game");
	m_pReturnToGame->SetClickedListener(this, Close);
	AddControl(m_pReturnToGame);

	m_pExit = new CButton(0, 0, 100, 100, "Exit To Desktop");
	m_pExit->SetClickedListener(this, Exit);
	AddControl(m_pExit);

	m_iLunarWorkshop = CRenderer::LoadTextureIntoGL(L"textures/lunar-workshop.png");
	m_iDigitanks = CRenderer::LoadTextureIntoGL(L"textures/digitanks.png");

	Layout();
}

void CDigitanksMenu::Layout()
{
	SetPos(50, 250);

	m_pDigitanks->SetPos(0, 20);
	m_pDigitanks->SetSize(GetWidth(), GetHeight());
	m_pDigitanks->SetAlign(CLabel::TA_TOPCENTER);

	int iSelectorSize = m_pDifficultyLabel->GetHeight() - 4;

	m_pStartTutorialBasics->SetPos(100, 140);
	m_pStartTutorialBasics->SetSize(100, 20);

	m_pStartTutorialBases->SetPos(100, 170);
	m_pStartTutorialBases->SetSize(100, 20);

	m_pDifficultyLabel->EnsureTextFits();
	m_pDifficultyLabel->SetPos(75, 220);

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 220);

	m_pStartArtilleryGame->SetPos(75, 270);
	m_pStartArtilleryGame->SetSize(150, 20);

	m_pStartBasesGame->SetPos(75, 300);
	m_pStartBasesGame->SetSize(150, 20);

	m_pReturnToGame->SetPos(100, 330);
	m_pReturnToGame->SetSize(100, 20);
	m_pReturnToGame->SetVisible(!!GameServer());

	m_pExit->SetPos(100, 360);
	m_pExit->SetSize(100, 20);

	BaseClass::Layout();
}

void CDigitanksMenu::Paint(int x, int y, int w, int h)
{
	if (!GameServer())
	{
		CRootPanel::PaintTexture(m_iLunarWorkshop, CRootPanel::Get()->GetWidth()-200-20, CRootPanel::Get()->GetHeight()-200, 200, 200);
		CRootPanel::PaintTexture(m_iDigitanks, 20, 20, 410, 205);
	}

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

void CDigitanksMenu::StartTutorialBasicsCallback()
{
	CInstructor* pInstructor = CDigitanksWindow::Get()->GetInstructor();

	pInstructor->SetActive(true);
	pInstructor->Initialize();

	CDigitanksWindow::Get()->CreateGame(GAMETYPE_TUTORIAL);
	DigitanksGame()->SetDifficulty(0);

	pInstructor->DisplayFirstBasicsTutorial();

	SetVisible(false);
}

void CDigitanksMenu::StartTutorialBasesCallback()
{
	CInstructor* pInstructor = CDigitanksWindow::Get()->GetInstructor();

	pInstructor->SetActive(true);
	pInstructor->Initialize();

	CDigitanksWindow::Get()->CreateGame(GAMETYPE_TUTORIAL);
	DigitanksGame()->SetDifficulty(0);

	pInstructor->DisplayFirstBasesTutorial();

	SetVisible(false);
}

void CDigitanksMenu::StartArtilleryGameCallback()
{
	CDigitanksWindow::Get()->CreateGame(GAMETYPE_ARTILLERY);

	if (!GameServer())
		return;

	CDigitanksWindow::Get()->GetInstructor()->SetActive(false);
	DigitanksGame()->SetDifficulty(m_pDifficulty->GetSelectionValue());
	SetVisible(false);
}

void CDigitanksMenu::StartBasesGameCallback()
{
	CDigitanksWindow::Get()->CreateGame(GAMETYPE_STANDARD);

	if (!GameServer())
		return;

	CDigitanksWindow::Get()->GetInstructor()->SetActive(false);
	DigitanksGame()->SetDifficulty(m_pDifficulty->GetSelectionValue());
	SetVisible(false);
}

void CDigitanksMenu::CloseCallback()
{
	SetVisible(false);
}

void CDigitanksMenu::ExitCallback()
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
	{
		m_pVictory->SetText(L"VICTORY!\n \nYou have crushed the weak and foolish under your merciless, unwavering treads. Your enemies bow before you as you stand - ruler of the Digiverse!\n \n");
		m_pVictory->AppendText(L"Thanks for playing Digitanks. Press escape to start a new game.");
	}
	else
	{
		m_pVictory->SetText(L"DEFEAT!\n \nYour ravenous enemies have destroyed your feeble tank armies. Database memories will recall the day when your once-glorious digital empire crumbled!\n \n");
		m_pVictory->AppendText(L"Thanks for playing Digitanks. You can close this window to continue playing, or you can press escape to start a new game.");
	}

	SetVisible(true);
}

CDonatePanel::CDonatePanel()
	: CPanel(0, 0, 400, 300)
{
	m_pDonate = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pDonate);

	m_pDonateButton = new CButton(0, 0, 100, 100, "Website!");
	m_pDonateButton->SetClickedListener(this, Donate);
	AddControl(m_pDonateButton);

	m_pExitButton = new CButton(0, 0, 100, 100, "Quit");
	m_pExitButton->SetClickedListener(this, Exit);
	AddControl(m_pExitButton);

	SetVisible(false);

	Layout();
}

void CDonatePanel::Layout()
{
	SetSize(500, 250);
	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_pDonate->SetPos(10, 20);
	m_pDonate->SetSize(GetWidth()-20, GetHeight());
	m_pDonate->SetAlign(CLabel::TA_TOPCENTER);

	m_pDonateButton->SetSize(100, 30);
	m_pDonateButton->SetPos(GetWidth()/2-150, GetHeight() - 50);

	m_pExitButton->SetSize(100, 30);
	m_pExitButton->SetPos(GetWidth()/2+50, GetHeight() - 50);
}

void CDonatePanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 255));

	BaseClass::Paint(x, y, w, h);
}

void CDonatePanel::ClosingApplication()
{
	m_pDonate->SetText(L"HELP ME FINISH DIGITANKS!\n \n"
		L"The full version of Digitanks will have more tanks, more weapons, bases to build and conquer, and even online multiplayer! All that stuff is really hard though, and I need your help to build it.\n \n"
		L"Please visit the website to contribute feedback from your playing experience by filling out a short survey. You can also donate to the Digitanks development effort, which will earn you a FREE copy of the game when it's released, your name in the credits, and the satisfaction of having helped out for a good cause!");

	SetVisible(true);

	CDigitanksWindow::Get()->GetInstructor()->SetActive(false);
}

void CDonatePanel::DonateCallback()
{
	OpenBrowser(L"http://digitanks.com/gamelanding/");
	exit(0);
}

void CDonatePanel::ExitCallback()
{
	exit(0);
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

