#include "digitankswindow.h"

#include <glgui/glgui.h>
#include <game/digitanksgame.h>

#include "menu.h"
#include "instructor.h"

using namespace glgui;

void CDigitanksWindow::InitUI()
{
	m_pMenu = new CDigitanksMenu();
	m_pVictory = new CVictoryPanel();

	CRootPanel::Get()->AddControl(m_pMenu);
	CRootPanel::Get()->AddControl(m_pVictory);

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

	m_pNumberOfPlayers = new CScrollSelector<int>();
	m_pNumberOfPlayers->AddSelection(CScrollSelection<int>(2, L"2"));
	m_pNumberOfPlayers->AddSelection(CScrollSelection<int>(3, L"3"));
	m_pNumberOfPlayers->AddSelection(CScrollSelection<int>(4, L"4"));
	m_pNumberOfPlayers->AddSelection(CScrollSelection<int>(5, L"5"));
	m_pNumberOfPlayers->AddSelection(CScrollSelection<int>(6, L"6"));
	m_pNumberOfPlayers->AddSelection(CScrollSelection<int>(7, L"7"));
	m_pNumberOfPlayers->AddSelection(CScrollSelection<int>(8, L"8"));
	m_pNumberOfPlayers->SetSelection(2);
	AddControl(m_pNumberOfPlayers);

	m_pPlayersLabel = new CLabel(0, 0, 32, 32, "Number of teams");
	AddControl(m_pPlayersLabel);

	m_pNumberOfTanks = new CScrollSelector<int>();
	m_pNumberOfTanks->AddSelection(CScrollSelection<int>(1, L"1"));
	m_pNumberOfTanks->AddSelection(CScrollSelection<int>(2, L"2"));
	m_pNumberOfTanks->AddSelection(CScrollSelection<int>(3, L"3"));
	m_pNumberOfTanks->AddSelection(CScrollSelection<int>(4, L"4"));
	m_pNumberOfTanks->AddSelection(CScrollSelection<int>(5, L"5"));
	m_pNumberOfTanks->SetSelection(2);
	AddControl(m_pNumberOfTanks);

	m_pTanksLabel = new CLabel(0, 0, 32, 32, "Tanks per team");
	AddControl(m_pTanksLabel);

	m_pDifficulty = new CScrollSelector<int>();
	m_pDifficulty->AddSelection(CScrollSelection<int>(0, L"Easy"));
	m_pDifficulty->AddSelection(CScrollSelection<int>(1, L"Normal"));
	m_pDifficulty->SetSelection(1);
	AddControl(m_pDifficulty);

	m_pDifficultyLabel = new CLabel(0, 0, 32, 32, "Difficulty");
	AddControl(m_pDifficultyLabel);

	m_pTutorialBox = new CCheckBox();
	m_pTutorialBox->SetClickedListener(this, Tutorial);
	m_pTutorialBox->SetUnclickedListener(this, Tutorial);
	m_pTutorialBox->SetState(true, false);
	AddControl(m_pTutorialBox);

	m_pTutorialLabel = new CLabel(0, 0, 100, 100, "Display tutorial");
	AddControl(m_pTutorialLabel);

	m_pStartGame = new CButton(0, 0, 100, 100, "Start Game");
	m_pStartGame->SetClickedListener(this, StartGame);
	AddControl(m_pStartGame);

	m_pExit = new CButton(0, 0, 100, 100, "Exit");
	m_pExit->SetClickedListener(this, Exit);
	AddControl(m_pExit);

	Layout();
}

void CDigitanksMenu::Layout()
{
	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_pDigitanks->SetPos(0, 20);
	m_pDigitanks->SetSize(GetWidth(), GetHeight());
	m_pDigitanks->SetAlign(CLabel::TA_TOPCENTER);

	m_pPlayersLabel->EnsureTextFits();
	m_pPlayersLabel->SetPos(5, 180);

	int iSelectorSize = m_pPlayersLabel->GetHeight() - 4;

	m_pNumberOfPlayers->SetSize(GetWidth() - m_pPlayersLabel->GetWidth() - 20, iSelectorSize);
	m_pNumberOfPlayers->SetPos(GetWidth() - m_pNumberOfPlayers->GetWidth() - 20/2, 180);

	m_pTanksLabel->EnsureTextFits();
	m_pTanksLabel->SetPos(5, 210);

	m_pNumberOfTanks->SetSize(GetWidth() - m_pTanksLabel->GetWidth() - 20, iSelectorSize);
	m_pNumberOfTanks->SetPos(GetWidth() - m_pNumberOfTanks->GetWidth() - 20/2, 210);

	m_pDifficultyLabel->EnsureTextFits();
	m_pDifficultyLabel->SetPos(75, 240);

	m_pDifficulty->SetSize(GetWidth() - m_pDifficultyLabel->GetLeft()*2 - m_pDifficultyLabel->GetWidth(), iSelectorSize);
	m_pDifficulty->SetPos(m_pDifficultyLabel->GetRight(), 240);

	m_pTutorialLabel->SetPos(110, 300);
	m_pTutorialLabel->SetSize(100, 20);
	m_pTutorialBox->SetPos(100, 300 + m_pTutorialLabel->GetHeight()/2 - m_pTutorialBox->GetHeight()/2);

	m_pStartGame->SetPos(100, 330);
	m_pStartGame->SetSize(100, 20);

	m_pExit->SetPos(100, 360);
	m_pExit->SetSize(100, 20);

	BaseClass::Layout();
}

void CDigitanksMenu::Paint(int x, int y, int w, int h)
{
	if (CDigitanksWindow::Get()->GetGame())
		CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 235));
	else
		CRootPanel::PaintRect(x, y, w, h, Color(120, 130, 120, 255));

	BaseClass::Paint(x, y, w, h);
}

void CDigitanksMenu::SetVisible(bool bVisible)
{
	if (bVisible)
	{
		if (CDigitanksWindow::Get()->GetGame())
			m_pStartGame->SetText("Restart Game");
		else
			m_pStartGame->SetText("Start Game");
	}

	if (CDigitanksWindow::Get()->GetInstructor())
	{
		if (bVisible)
			CDigitanksWindow::Get()->GetInstructor()->HideTutorial();
		else
			CDigitanksWindow::Get()->GetInstructor()->ShowTutorial();
	}

	BaseClass::SetVisible(bVisible);
}

void CDigitanksMenu::TutorialCallback()
{
	CInstructor* pInstructor = CDigitanksWindow::Get()->GetInstructor();

	if (!pInstructor)
		return;

	pInstructor->SetActive(m_pTutorialBox->GetState());
	pInstructor->Initialize();
}

void CDigitanksMenu::StartGameCallback()
{
	CDigitanksWindow::Get()->CreateGame(m_pNumberOfPlayers->GetSelectionValue(), m_pNumberOfTanks->GetSelectionValue());
	CDigitanksWindow::Get()->GetInstructor()->SetActive(m_pTutorialBox->GetState());
	DigitanksGame()->SetDifficulty(m_pDifficulty->GetSelectionValue());
	SetVisible(false);
}

void CDigitanksMenu::ExitCallback()
{
	exit(0);
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
	SetSize(500, 400);
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
		m_pVictory->SetText(L"VICTORY!\n \nYou have crushed the weak and foolish under your merciless, unwavering treads. Your enemies bow before you as you stand - ruler of the digital universe!\n \n \n");
	else
		m_pVictory->SetText(L"DEFEAT!\n \nYour ravenous enemies have destroyed your feeble tank armies. Database memories will recall the day when your once-glorious digital empire crumbled!\n \n \n");

	m_pVictory->AppendText(L"Thanks for playing Digitanks. You can close this window to continue playing, or you can press escape to start a new game.\n \n"
		L"This is just a demo of the full game, which will have many more features:\n"
		L"* 4X-style gameplay - Explore, expand, exploit, exterminate\n"
		L"* Fleet construction - Design and build a fleet of tanks\n"
		L"* Base building - Find resources and expand your territory\n"
		L"* More weapons - Big bombs, small bombs, short bombs, tall bombs\n"
		L"* More tank classes - Artillery, light tanks, and more\n"

		L" \nIf you would like to see this game completed and you want to help out, please visit our website. There you can read more about Digitanks development and donate to the cause.\n \nhttp://digitanks.com");

	SetVisible(true);
}
