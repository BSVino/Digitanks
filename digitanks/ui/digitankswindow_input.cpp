#include "digitankswindow.h"

#include <GL/glut.h>

#include "glgui/glgui.h"
#include "game/digitanksgame.h"
#include "instructor.h"
#include "camera.h"
#include "hud.h"

void CDigitanksWindow::MouseMotion(int x, int y)
{
	FakeCtrlAltShift();

	glgui::CRootPanel::Get()->CursorMoved(x, y);

	m_pCamera->MouseInput(x-m_iMouseStartX, y-m_iMouseStartY);

	m_iMouseStartX = x;
	m_iMouseStartY = y;
}

void CDigitanksWindow::MouseDragged(int x, int y)
{
	FakeCtrlAltShift();

	glgui::CRootPanel::Get()->CursorMoved(x, y);

	m_pCamera->MouseInput(x-m_iMouseStartX, y-m_iMouseStartY);

	m_iMouseStartX = x;
	m_iMouseStartY = y;
}

void CDigitanksWindow::MouseInput(int iButton, int iState, int x, int y)
{
	FakeCtrlAltShift();

	m_pCamera->MouseButton(iButton, iState);

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

		if (GetControlMode() == MODE_MOVE)
		{
			DigitanksGame()->SetDesiredMove(glutGetModifiers()&GLUT_ACTIVE_SHIFT);

			if (glutGetModifiers()&GLUT_ACTIVE_SHIFT || !m_pHUD->ShouldAutoProceed())
				SetControlMode(MODE_TURN);
			else
				DigitanksGame()->NextTank();

			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_MOVE);

			GetCamera()->SetTarget(DigitanksGame()->GetCurrentTank()->GetDesiredMove());
		}
		else if (GetControlMode() == MODE_TURN)
		{
			DigitanksGame()->SetDesiredTurn(bFound && glutGetModifiers()&GLUT_ACTIVE_SHIFT, vecMousePosition);

			if (glutGetModifiers()&GLUT_ACTIVE_SHIFT || !m_pHUD->ShouldAutoProceed())
				SetControlMode(MODE_AIM);
			else
				DigitanksGame()->NextTank();

			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_TURN);
		}
		else if (GetControlMode() == MODE_AIM)
		{
			DigitanksGame()->SetDesiredAim(glutGetModifiers()&GLUT_ACTIVE_SHIFT);

			if (glutGetModifiers()&GLUT_ACTIVE_SHIFT)
				SetControlMode(MODE_FIRE);
			else if (!m_pHUD->ShouldAutoProceed())
				SetControlMode(MODE_NONE);
			else
				DigitanksGame()->NextTank();

			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_AIM);
		}
		else if (GetControlMode() == MODE_FIRE)
		{
			if (glutGetModifiers()&GLUT_ACTIVE_SHIFT)
				SetControlMode(MODE_NONE);
			else if (!m_pHUD->ShouldAutoProceed())
				SetControlMode(MODE_NONE);
			else
				DigitanksGame()->NextTank();

			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_POWER);
		}
	}
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
		m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_KEYS);
		DigitanksGame()->EndTurn();
	}

	if (DigitanksGame() && c == 32)
	{
		// Don't clobber existing commands when scrolling through tanks.
		SetControlMode(MODE_NONE);
		DigitanksGame()->NextTank();
	}

	if (c == 27)
	{
		if (GetControlMode() == MODE_NONE)
			exit(0);
		else
			SetControlMode(MODE_NONE);
	}

	m_pCamera->KeyDown(c);
}

void CDigitanksWindow::KeyRelease(unsigned char c, int x, int y)
{
	m_pCamera->KeyUp(c);
}

void CDigitanksWindow::Special(int k, int x, int y)
{
	FakeCtrlAltShift();

	if (k == GLUT_KEY_F4 && (glutGetModifiers()&GLUT_ACTIVE_ALT))
		exit(0);

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
