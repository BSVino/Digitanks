#include "digitankswindow.h"

#include <GL/glut.h>

void CDigitanksWindow::MouseMotion(int x, int y)
{
//	modelgui::CRootPanel::Get()->CursorMoved(x, y);
}

void CDigitanksWindow::MouseDragged(int x, int y)
{
//	modelgui::CRootPanel::Get()->CursorMoved(x, y);
}

void CDigitanksWindow::MouseInput(int iButton, int iState, int x, int y)
{
	if (iState == GLUT_DOWN)
	{
//		if (modelgui::CRootPanel::Get()->MousePressed(iButton, x, y))
//			return;
	}
	else
	{
//		if (modelgui::CRootPanel::Get()->MouseReleased(iButton, x, y))
//			return;
	}

}

void CDigitanksWindow::KeyPress(unsigned char c, int x, int y)
{
//	if (modelgui::CRootPanel::Get()->KeyPressed(c))
//	{
//		glutPostRedisplay();
//		return;
//	}

	if (c == 27)
		exit(0);
}

void CDigitanksWindow::Special(int k, int x, int y)
{
	if (k == GLUT_KEY_F4 && (glutGetModifiers()&GLUT_ACTIVE_ALT))
		exit(0);

	glutPostRedisplay();
}
