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

	if (!DigitanksGame())
		return;

	if (iState == GLUT_DOWN)
	{
		Vector vecPoint;
		bool bMouseOnGrid = GetMouseGridPosition(vecPoint);

		bool bSelected = false;

		if (bMouseOnGrid)
		{
			for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
			{
				CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
				if (!pEntity)
					continue;

				CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
				if (!pTank)
					continue;

				if (DigitanksGame()->GetCurrentTeam() != pTank->GetTeam())
					continue;

				if (DigitanksGame()->GetCurrentTank() == pTank)
					continue;

				if ((vecPoint - pTank->GetOrigin()).LengthSqr() < 2*2)
				{
					DigitanksGame()->SetCurrentTank(pTank);
					bSelected = true;
					break;
				}

				if (pTank->HasDesiredMove() && (vecPoint - pTank->GetDesiredMove()).LengthSqr() < 2*2)
				{
					DigitanksGame()->SetCurrentTank(pTank);
					bSelected = true;
					break;
				}
			}
		}

		if (bSelected)
		{
		}
		else if (GetControlMode() == MODE_MOVE)
		{
			DigitanksGame()->SetDesiredMove();
			SetControlMode(MODE_TURN, m_bAutoProceed);
		}
		else if (GetControlMode() == MODE_TURN)
		{
			DigitanksGame()->SetDesiredTurn();
			if (m_bAutoProceed)
				SetControlMode(MODE_AIM, true);
			else
				SetControlMode(MODE_NONE);
		}
		else if (GetControlMode() == MODE_AIM)
		{
			DigitanksGame()->SetDesiredAim();
			if (m_bAutoProceed)
				SetControlMode(MODE_FIRE, true);
			else
				SetControlMode(MODE_NONE);
		}
		else if (GetControlMode() == MODE_FIRE)
		{
			if (m_bAutoProceed)
				DigitanksGame()->NextTank();
			else
				SetControlMode(MODE_NONE);
		}
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
		DigitanksGame()->EndTurn();

	if (DigitanksGame() && c == 32)
		DigitanksGame()->NextTank();

	if (c == 27)
		exit(0);
}

void CDigitanksWindow::Special(int k, int x, int y)
{
	if (k == GLUT_KEY_F4 && (glutGetModifiers()&GLUT_ACTIVE_ALT))
		exit(0);

	glutPostRedisplay();
}
