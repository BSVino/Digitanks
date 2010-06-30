#include "digitankswindow.h"

#include <GL/glut.h>

#include "glgui/glgui.h"
#include "digitanks/digitanksgame.h"
#include "instructor.h"
#include "game/camera.h"
#include "hud.h"
#include "menu.h"
#include <game/digitanks/cpu.h>

#include <renderer/dissolver.h>

#ifdef _DEBUG
#include <game/digitanks/maintank.h>
#endif

void CDigitanksWindow::MouseMotion(int x, int y)
{
	FakeCtrlAltShift();

	glgui::CRootPanel::Get()->CursorMoved(x, y);

	if (GetGame() && GetGame()->GetCamera())
		GetGame()->GetCamera()->MouseInput(x-m_iMouseStartX, y-m_iMouseStartY);

	m_iMouseStartX = x;
	m_iMouseStartY = y;
}

void CDigitanksWindow::MouseDragged(int x, int y)
{
	FakeCtrlAltShift();

	glgui::CRootPanel::Get()->CursorMoved(x, y);

	if (GetGame() && GetGame()->GetCamera())
		GetGame()->GetCamera()->MouseInput(x-m_iMouseStartX, y-m_iMouseStartY);

	m_iMouseStartX = x;
	m_iMouseStartY = y;
}

void CDigitanksWindow::MouseInput(int iButton, int iState, int x, int y)
{
	FakeCtrlAltShift();

	if (GetGame() && GetGame()->GetCamera())
		GetGame()->GetCamera()->MouseButton(iButton, iState);

	if (iState == GLUT_DOWN)
	{
		if (glgui::CRootPanel::Get()->MousePressed(iButton, x, y))
			return;
	}
	else
	{
		if (glgui::CRootPanel::Get()->MouseReleased(iButton, x, y))
			return;
	}

	if (!DigitanksGame())
		return;

	if (iState == GLUT_DOWN && iButton == 0)
	{
		Vector vecMousePosition;
		bool bFound = GetMouseGridPosition(vecMousePosition);

		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		{
			DigitanksGame()->SetDesiredMove(glutGetModifiers()&GLUT_ACTIVE_SHIFT);

			if (DigitanksGame()->GetCurrentTank()->HasDesiredMove())
			{
				if (glutGetModifiers()&GLUT_ACTIVE_SHIFT || !m_pHUD->ShouldAutoProceed())
					DigitanksGame()->SetControlMode(MODE_AIM);

				if (m_pHUD->ShouldAutoProceed())
					DigitanksGame()->NextTank();

				m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_MOVE);

				if (DigitanksGame()->GetCurrentTank())
					GetGame()->GetCamera()->SetTarget(DigitanksGame()->GetCurrentTank()->GetPreviewMove());
			}
		}
		else if (DigitanksGame()->GetControlMode() == MODE_TURN)
		{
			DigitanksGame()->SetDesiredTurn(bFound && glutGetModifiers()&GLUT_ACTIVE_SHIFT, vecMousePosition);

			if (glutGetModifiers()&GLUT_ACTIVE_SHIFT || !m_pHUD->ShouldAutoProceed())
				DigitanksGame()->SetControlMode(MODE_NONE);
			else
				DigitanksGame()->NextTank();

//			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_TURN);
		}
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
		{
			DigitanksGame()->SetDesiredAim(glutGetModifiers()&GLUT_ACTIVE_SHIFT);

			if (glutGetModifiers()&GLUT_ACTIVE_SHIFT)
				DigitanksGame()->SetControlMode(MODE_FIRE);
			else if (!m_pHUD->ShouldAutoProceed())
				DigitanksGame()->SetControlMode(MODE_NONE);
			else
				DigitanksGame()->NextTank();

			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_AIM);
		}
		else if (DigitanksGame()->GetControlMode() == MODE_FIRE)
		{
			if (glutGetModifiers()&GLUT_ACTIVE_SHIFT)
			{
				DigitanksGame()->SetControlMode(MODE_NONE);
				m_pHUD->SetAutoProceed(false);
			}
			else if (!m_pHUD->ShouldAutoProceed())
				DigitanksGame()->SetControlMode(MODE_NONE);
			else
				DigitanksGame()->NextTank();

			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_POWER);
		}
		else if (DigitanksGame()->GetControlMode() == MODE_BUILD)
		{
			CCPU* pCPU = dynamic_cast<CCPU*>(DigitanksGame()->GetCurrentStructure());
			if (pCPU && pCPU->IsPreviewBuildValid())
			{
				pCPU->BeginConstruction();
				DigitanksGame()->SetControlMode(MODE_NONE);
			}
		}
	}

	GetHUD()->SetupMenu();
}

void CDigitanksWindow::KeyPress(unsigned char c, int x, int y)
{
	FakeCtrlAltShift();

	if (glgui::CRootPanel::Get()->KeyPressed(c))
	{
		glutPostRedisplay();
		return;
	}

	if (DigitanksGame() && c == 13)
	{
		DigitanksGame()->EndTurn();
	}

	if (DigitanksGame() && c == 32)
	{
		// Don't clobber existing commands when scrolling through tanks.
		if (m_pHUD->ShouldAutoProceed())
		{
			// Set desired move so that the tank knows the player selected something.
			if (DigitanksGame()->GetControlMode() == MODE_MOVE)
			{
				if (DigitanksGame()->GetCurrentTank())
					DigitanksGame()->GetCurrentTank()->SetPreviewMove(Vector(9999, 9999, 9999));	// Make sure the mouse isn't hovering a legal move.
				DigitanksGame()->SetDesiredMove(glutGetModifiers()&GLUT_ACTIVE_SHIFT);
			}
		}
		else
			DigitanksGame()->SetControlMode(MODE_NONE);
		DigitanksGame()->NextTank();
	}

	if (DigitanksGame() && c == 9)
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
			DigitanksGame()->SetControlMode(MODE_AIM);
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
		{
			if (DigitanksGame()->GetCurrentTank() && DigitanksGame()->GetCurrentTank()->HasDesiredAim())
				DigitanksGame()->SetControlMode(MODE_FIRE);
			else
				DigitanksGame()->SetControlMode(MODE_MOVE);
		}
		else if (DigitanksGame()->GetControlMode() == MODE_FIRE)
			DigitanksGame()->SetControlMode(MODE_MOVE);
		else
			DigitanksGame()->SetControlMode(MODE_MOVE);
	}

	if (c == 27)
	{
		if (GetMenu()->IsVisible())
			GetMenu()->SetVisible(false);
		else if (DigitanksGame()->GetControlMode() == MODE_NONE)
			GetMenu()->SetVisible(true);
		else
			DigitanksGame()->SetControlMode(MODE_NONE);
	}

	if (c == 't')
		GetInstructor()->SetActive(true);

#ifdef _DEBUG
	if (c == 'x')
	{
		// Make a supertank at the given location.
		Vector vecPoint;
		if (GetMouseGridPosition(vecPoint))
		{
			CMainBattleTank* pTank = Game()->Create<CMainBattleTank>("CMainBattleTank");
			DigitanksGame()->GetCurrentTeam()->AddEntity(pTank);
			pTank->SetOrigin(DigitanksGame()->GetTerrain()->SetPointHeight(vecPoint));
			pTank->GiveBonusPoints(10, false);
		}
	}
#endif

	if (GetGame() && GetGame()->GetCamera())
		GetGame()->GetCamera()->KeyDown(c);
}

void CDigitanksWindow::KeyRelease(unsigned char c, int x, int y)
{
	if (GetGame() && GetGame()->GetCamera())
		GetGame()->GetCamera()->KeyUp(c);
}

void CDigitanksWindow::Special(int k, int x, int y)
{
	FakeCtrlAltShift();

	if (k == GLUT_KEY_F4 && (glutGetModifiers()&GLUT_ACTIVE_ALT))
		CloseApplication();

	glutPostRedisplay();
}

void CDigitanksWindow::FakeCtrlAltShift()
{
	int iModifiers = glutGetModifiers();

	if ((iModifiers&GLUT_ACTIVE_CTRL) && !m_bCtrl)
		m_bCtrl = true;
	else if (!(iModifiers&GLUT_ACTIVE_CTRL) && m_bCtrl)
		m_bCtrl = false;
 
	if ((iModifiers&GLUT_ACTIVE_ALT) && !m_bAlt)
		m_bAlt = true;
	else if (!(iModifiers&GLUT_ACTIVE_ALT) && m_bAlt)
		m_bAlt = false;

	if ((iModifiers&GLUT_ACTIVE_SHIFT) && !m_bShift)
		m_bShift = true;
	else if (!(iModifiers&GLUT_ACTIVE_SHIFT) && m_bShift)
		m_bShift = false;
}
