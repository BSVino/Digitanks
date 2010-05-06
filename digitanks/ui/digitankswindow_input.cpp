#include "digitankswindow.h"

#include <GL/glut.h>

#include "glgui/glgui.h"
#include "game/digitanksgame.h"

void CDigitanksWindow::MouseMotion(int x, int y)
{
	glgui::CRootPanel::Get()->CursorMoved(x, y);
}

void CDigitanksWindow::MouseDragged(int x, int y)
{
	glgui::CRootPanel::Get()->CursorMoved(x, y);
}

void CDigitanksWindow::MouseInput(int iButton, int iState, int x, int y)
{
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

	if (DigitanksGame() && iState == GLUT_DOWN && GetControlMode() == MODE_MOVE)
	{
		DigitanksGame()->SetDesiredMove();
		SetControlMode(MODE_TURN, m_bAutoProceed);
	}
	else if (DigitanksGame() && iState == GLUT_DOWN && GetControlMode() == MODE_TURN)
	{
		DigitanksGame()->SetDesiredTurn();
		SetControlMode(MODE_AIM, m_bAutoProceed);
	}
	else if (DigitanksGame() && iState == GLUT_DOWN && GetControlMode() == MODE_AIM)
	{
		DigitanksGame()->SetDesiredAim();
		if (m_bAutoProceed)
			SetControlMode(MODE_FIRE, true);
		else
			SetControlMode(MODE_NONE, false);
	}
	else if (DigitanksGame() && iState == GLUT_DOWN && GetControlMode() == MODE_FIRE)
	{
		SetControlMode(MODE_NONE);
	}
}

void CDigitanksWindow::KeyPress(unsigned char c, int x, int y)
{
	if (glgui::CRootPanel::Get()->KeyPressed(c))
	{
		glutPostRedisplay();
		return;
	}

	if (DigitanksGame() && c == 13)
		DigitanksGame()->Turn();

	if (c == 27)
		exit(0);
}

void CDigitanksWindow::Special(int k, int x, int y)
{
	if (k == GLUT_KEY_F4 && (glutGetModifiers()&GLUT_ACTIVE_ALT))
		exit(0);

	glutPostRedisplay();
}
