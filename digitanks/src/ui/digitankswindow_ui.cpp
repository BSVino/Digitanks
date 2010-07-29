#include "digitankswindow.h"

#include <glgui/glgui.h>
#include <digitanks/digitanksgame.h>
#include <platform.h>
#include <renderer/renderer.h>

#include "menu.h"
#include "instructor.h"

using namespace glgui;

void CDigitanksWindow::InitUI()
{
	m_pMenu = new CDigitanksMenu();
	m_pVictory = new CVictoryPanel();
	m_pDonate = new CDonatePanel();

	CRootPanel::Get()->AddControl(m_pMenu);
	CRootPanel::Get()->AddControl(m_pVictory);
	CRootPanel::Get()->AddControl(m_pDonate);

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
	m_pDigitanks = new CLabel(0, 0, 100, 100, "DIGITANKS\n \nCopyright © 2010, Jorge Rodriguez <bs.vino@gmail.com>\n \nhttp://digitanks.com");
	AddControl(m_pDigitanks);

	m_pDifficulty = new CScrollSelector<int>();
	m_pDifficulty->AddSelection(CScrollSelection<int>(0, L"Easy"));
	m_pDifficulty->AddSelection(CScrollSelection<int>(1, L"Normal"));
	m_pDifficulty->SetSelection(1);
	AddControl(m_pDifficulty);

	m_pDifficultyLabel = new CLabel(0, 0, 32, 32, "Difficulty");
	AddControl(m_pDifficultyLabel);

	m_pStartTutorialBasics = new CButton(0, 0, 100, 100, "Tutorial - Basics");
	m_pStartTutorialBasics->SetClickedListener(this, StartTutorialBasics);
	AddControl(m_pStartTutorialBasics);

	m_pStartTutorialBases = new CButton(0, 0, 100, 100, "Tutorial - Bases");
	m_pStartTutorialBases->SetClickedListener(this, StartTutorialBases);
	AddControl(m_pStartTutorialBases);

	m_pStartGame = new CButton(0, 0, 100, 100, "New Game");
	m_pStartGame->SetText("New Game");
	m_pStartGame->SetClickedListener(this, StartGame);
	AddControl(m_pStartGame);

	m_pExit = new CButton(0, 0, 100, 100, "Exit To Desktop");
	m_pExit->SetClickedListener(this, Exit);
	AddControl(m_pExit);

	m_iLunarWorkshop = CRenderer::LoadTextureIntoGL(L"textures/lunar-workshop.png");

	Layout();
}

void CDigitanksMenu::Layout()
{
	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_pDigitanks->SetPos(0, 20);
	m_pDigitanks->SetSize(GetWidth(), GetHeight());
	m_pDigitanks->SetAlign(CLabel::TA_TOPCENTER);

	int iSelectorSize = m_pDifficultyLabel->GetHeight() - 4;

	m_pStartTutorialBasics->SetPos(100, 200);
	m_pStartTutorialBasics->SetSize(100, 20);

	m_pStartTutorialBases->SetPos(100, 230);
	m_pStartTutorialBases->SetSize(100, 20);

	m_pDifficultyLabel->EnsureTextFits();
	m_pDifficultyLabel->SetPos(75, 280);

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 280);

	m_pStartGame->SetPos(100, 330);
	m_pStartGame->SetSize(100, 20);

	m_pExit->SetPos(100, 360);
	m_pExit->SetSize(100, 20);

	BaseClass::Layout();
}

void CDigitanksMenu::Paint(int x, int y, int w, int h)
{
	if (!CDigitanksWindow::Get()->GetGame())
		CRootPanel::PaintTexture(m_iLunarWorkshop, 20, CRootPanel::Get()->GetHeight()-200, 200, 200);

	if (CDigitanksWindow::Get()->GetGame())
		CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 235));
	else
		CRootPanel::PaintRect(x, y, w, h, Color(120, 130, 120, 255));

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

void CDigitanksMenu::StartGameCallback()
{
	CDigitanksWindow::Get()->CreateGame(GAMETYPE_STANDARD);

	if (!Game())
		return;

	CDigitanksWindow::Get()->GetInstructor()->SetActive(false);
	DigitanksGame()->SetDifficulty(m_pDifficulty->GetSelectionValue());
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
	Layout();

	if (bPlayerWon)
		m_pVictory->SetText(L"VICTORY!\n \nYou have crushed the weak and foolish under your merciless, unwavering treads. Your enemies bow before you as you stand - ruler of the Digiverse!\n \n");
	else
		m_pVictory->SetText(L"DEFEAT!\n \nYour ravenous enemies have destroyed your feeble tank armies. Database memories will recall the day when your once-glorious digital empire crumbled!\n \n");

	m_pVictory->AppendText(L"Thanks for playing Digitanks. You can close this window to continue playing, or you can press escape to start a new game.");

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
	CRootPanel::PaintRect(x, y, w, h, Color(120, 130, 120, 255));

	BaseClass::Paint(x, y, w, h);
}

void CDonatePanel::ClosingApplication()
{
	Layout();

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
