#include "hud.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "digitankswindow.h"
#include "game/digitanksgame.h"

using namespace glgui;

CPowerBar::CPowerBar(powerbar_type_t ePowerbarType)
	: CBaseControl(0, 0, 100, 100)
{
	m_ePowerbarType = ePowerbarType;
}

void CPowerBar::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	CRootPanel::PaintRect(x, y, w, h, Color(255, 255, 255, 128));

	if (m_ePowerbarType == POWERBAR_ATTACK)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetAttackPower(true))-2, h-2, Color(255, 0, 0));
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetDefensePower(true))-2, h-2, Color(255, 255, 0));
	else
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetMovementPower(true))-2, h-2, Color(0, 0, 255));
}

CHUD::CHUD()
	: CPanel(0, 0, 100, 100)
{
	m_pGame = NULL;

	m_pAttackPower = new CPowerBar(POWERBAR_ATTACK);
	m_pDefensePower = new CPowerBar(POWERBAR_DEFENSE);
	m_pMovementPower = new CPowerBar(POWERBAR_MOVEMENT);

	AddControl(m_pAttackPower);
	AddControl(m_pDefensePower);
	AddControl(m_pMovementPower);

	m_pMoveButton = new CButton(0, 0, 50, 50, "");
	m_pMoveButton->SetClickedListener(this, Move);
	AddControl(m_pMoveButton);

	m_pTurnButton = new CButton(0, 0, 50, 50, "");
	m_pTurnButton->SetClickedListener(this, Turn);
	AddControl(m_pTurnButton);

	m_pFireButton = new CButton(0, 0, 50, 50, "");
	m_pFireButton->SetClickedListener(this, Fire);
	AddControl(m_pFireButton);
}

void CHUD::Layout()
{
	SetSize(GetParent()->GetWidth(), GetParent()->GetHeight());

	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	m_pAttackPower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 90);
	m_pAttackPower->SetSize(200, 20);

	m_pDefensePower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 60);
	m_pDefensePower->SetSize(200, 20);

	m_pMovementPower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 30);
	m_pMovementPower->SetSize(200, 20);

	m_pMoveButton->SetPos(iWidth/2 - 1024/2 + 700, iHeight - 100);
	m_pTurnButton->SetPos(iWidth/2 - 1024/2 + 760, iHeight - 100);
	m_pFireButton->SetPos(iWidth/2 - 1024/2 + 820, iHeight - 100);
}

void CHUD::Think()
{
	BaseClass::Think();

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
		m_pMoveButton->SetButtonColor(Color(100, 0, 0));
	else
		m_pMoveButton->SetButtonColor(Color(0, 0, 100));

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
		m_pTurnButton->SetButtonColor(Color(100, 0, 0));
	else
		m_pTurnButton->SetButtonColor(Color(0, 0, 100));

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
		m_pFireButton->SetButtonColor(Color(100, 0, 0));
	else
		m_pFireButton->SetButtonColor(Color(60, 40, 0));
}

void CHUD::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

#ifdef _DEBUG
	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	// Nobody runs resolutions under 1024x768 anymore.
	// Show me my constraints!
	CRootPanel::PaintRect(iWidth/2 - 1024/2, iHeight - 150, 1024, 200, Color(0, 0, 0, 100));

	// This is where the minimap will be.
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 10, iHeight - 150 - 30, 170, 170, Color(255, 255, 255, 100));

	// Shield schematic
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 190, iHeight - 150 + 10, 130, 130, Color(255, 255, 255, 100));

	// Tank data
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 330, iHeight - 150 + 10, 100, 130, Color(255, 255, 255, 100));
#endif

	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
	{
		CTeam* pTeam = DigitanksGame()->GetTeam(i);
		for (size_t j = 0; j < pTeam->GetNumTanks(); j++)
		{
			CDigitank* pTank = pTeam->GetTank(j);

			Vector vecOrigin;
			if (pTank->HasDesiredMove())
				vecOrigin = pTank->GetDesiredMove();
			else
				vecOrigin = pTank->GetOrigin();

			Vector vecScreen = CDigitanksWindow::Get()->ScreenPosition(vecOrigin);

			CRootPanel::PaintRect((int)vecScreen.x - 51, (int)vecScreen.y - 51, 102, 5, Color(255, 255, 255, 128));
			CRootPanel::PaintRect((int)vecScreen.x - 50, (int)vecScreen.y - 50, (int)(100.0f*pTank->GetHealth()/pTank->GetTotalHealth()), 3, Color(100, 255, 100));

			if (pTank == DigitanksGame()->GetCurrentTank() && CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
			{
				int iTop = (int)vecScreen.y - 100;
				int iBottom = (int)vecScreen.y + 100;
				CRootPanel::PaintRect((int)vecScreen.x + 60, iTop, 20, 200, Color(255, 255, 255, 128));

				int mx, my;
				glgui::CRootPanel::GetFullscreenMousePos(mx, my);

				float flAttackPercentage = RemapValClamped((float)my, (float)iTop, (float)iBottom, 0, 1);

				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + 1, 18, (int)(flAttackPercentage*198), Color(255, 0, 0, 255));
				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + 1 + (int)(flAttackPercentage*198), 18, (int)((1-flAttackPercentage)*198), Color(255, 255, 0, 255));
				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + (int)(flAttackPercentage*198) - 2, 18, 6, Color(128, 128, 128, 255));

				DigitanksGame()->GetCurrentTank()->SetAttackPower(flAttackPercentage * (pTank->GetTotalPower()*(1-pTank->GetMovementPower())));
			}
		}
	}

	CPanel::Paint(x, y, w, h);
}

void CHUD::SetGame(CDigitanksGame *pGame)
{
	m_pGame = pGame;
	m_pGame->SetListener(this);
}

void CHUD::GameStart()
{
	CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
}

void CHUD::GameOver()
{
}

void CHUD::NewCurrentTeam()
{
}

void CHUD::NewCurrentTank()
{
	if (!DigitanksGame()->GetCurrentTank()->HasDesiredMove() && !DigitanksGame()->GetCurrentTank()->HasDesiredTurn())
		CDigitanksWindow::Get()->SetControlMode(MODE_MOVE, true);
}

void CHUD::MoveCallback()
{
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_MOVE);
}

void CHUD::TurnCallback()
{
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_TURN);
}

void CHUD::FireCallback()
{
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_FIRE);
}
