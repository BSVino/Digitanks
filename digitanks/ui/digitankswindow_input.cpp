#include "digitankswindow.h"

#include <GL/glut.h>

#include "glgui/glgui.h"
#include "game/digitanksgame.h"
#include "instructor.h"

void CDigitanksWindow::MouseMotion(int x, int y)
{
	FakeCtrlAltShift();

	glgui::CRootPanel::Get()->CursorMoved(x, y);

	if (m_bFPSMode)
	{
		m_angFPSCamera.y += ((x-m_iMouseStartX)/5.0f);
		m_angFPSCamera.p += ((m_iMouseStartY-y)/5.0f);
	}

	m_iMouseStartX = x;
	m_iMouseStartY = y;
}

void CDigitanksWindow::MouseDragged(int x, int y)
{
	FakeCtrlAltShift();

	glgui::CRootPanel::Get()->CursorMoved(x, y);
}

void CDigitanksWindow::MouseInput(int iButton, int iState, int x, int y)
{
	FakeCtrlAltShift();

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

	if (iState == GLUT_DOWN)
	{
		Vector vecMousePosition;
		bool bFound = GetMouseGridPosition(vecMousePosition);

		if (GetControlMode() == MODE_MOVE)
		{
			DigitanksGame()->SetDesiredMove(glutGetModifiers()&GLUT_ACTIVE_SHIFT);
			DigitanksGame()->NextTank();
			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_MOVE);
		}
		else if (GetControlMode() == MODE_TURN)
		{
			DigitanksGame()->SetDesiredTurn(bFound && glutGetModifiers()&GLUT_ACTIVE_SHIFT, vecMousePosition);
			DigitanksGame()->NextTank();
			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_TURN);
		}
		else if (GetControlMode() == MODE_AIM)
		{
			DigitanksGame()->SetDesiredAim(glutGetModifiers()&GLUT_ACTIVE_SHIFT);
			DigitanksGame()->NextTank();
			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_AIM);
		}
		else if (GetControlMode() == MODE_FIRE)
		{
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
		exit(0);

#ifdef _DEBUG
	if (c == 'z')
	{
		m_bFPSMode = !m_bFPSMode;
		//glutSetCursor(m_bFPSMode?GLUT_CURSOR_NONE:GLUT_CURSOR_LEFT_ARROW);
	}
#endif

	if (m_bFPSMode)
	{
		if (c == 'w')
			m_vecFPSVelocity.x = 1.0f;
		if (c == 's')
			m_vecFPSVelocity.x = -1.0f;
		if (c == 'd')
			m_vecFPSVelocity.z = 1.0f;
		if (c == 'a')
			m_vecFPSVelocity.z = -1.0f;
	}
}

void CDigitanksWindow::KeyRelease(unsigned char c, int x, int y)
{
	if (m_bFPSMode)
	{
		if (c == 'w')
			m_vecFPSVelocity.x = 0.0f;
		if (c == 's')
			m_vecFPSVelocity.x = 0.0f;
		if (c == 'd')
			m_vecFPSVelocity.z = 0.0f;
		if (c == 'a')
			m_vecFPSVelocity.z = 0.0f;
	}
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
