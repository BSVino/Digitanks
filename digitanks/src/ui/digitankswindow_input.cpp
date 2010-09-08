#include "digitankswindow.h"

#include <GL/glew.h>
#include <GL/glfw.h>

#include <sound/sound.h>
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
	glgui::CRootPanel::Get()->CursorMoved(x, y);

	if (GetGame() && GetGame()->GetCamera())
		GetGame()->GetCamera()->MouseInput(x-m_iMouseStartX, y-m_iMouseStartY);

	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		m_iMouseMoved += (int)(fabs((float)x-m_iMouseStartX) + fabs((float)y-m_iMouseStartY));

	m_iMouseStartX = x;
	m_iMouseStartY = y;
}

void CDigitanksWindow::MouseInput(int iButton, int iState)
{
	if (GetGame() && GetGame()->GetCamera())
	{
		// MouseButton enables camera rotation, so don't send the signal if the feature is disabled.
		if (!m_pInstructor->IsFeatureDisabled(DISABLE_ROTATE))
			GetGame()->GetCamera()->MouseButton(iButton, iState);
	}

	int mx, my;
	glfwGetMousePos(&mx, &my);
	if (iState == GLFW_PRESS)
	{
		if (glgui::CRootPanel::Get()->MousePressed(iButton, mx, my))
			return;
	}
	else
	{
		if (glgui::CRootPanel::Get()->MouseReleased(iButton, mx, my))
			return;
	}

	if (!DigitanksGame())
		return;

	if (iState == GLFW_PRESS)
	{
		if ((iButton == GLFW_MOUSE_BUTTON_1 || iButton == GLFW_MOUSE_BUTTON_2) && DigitanksGame()->GetControlMode() == MODE_FIRE)
		{
			if (IsShiftDown())
			{
				DigitanksGame()->SetControlMode(MODE_NONE);
				m_pHUD->SetAutoProceed(false);
			}
			else if (!m_pHUD->ShouldAutoProceed())
				DigitanksGame()->SetControlMode(MODE_NONE);
			else
				DigitanksGame()->GetCurrentTeam()->NextTank();

			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_ENERGY);

			return;	// Don't center camera
		}
		else if (iButton == GLFW_MOUSE_BUTTON_2 && DigitanksGame()->GetControlMode() == MODE_BUILD)
		{
			CCPU* pCPU = dynamic_cast<CCPU*>(DigitanksGame()->GetCurrentStructure());
			if (pCPU && pCPU->IsPreviewBuildValid())
			{
				pCPU->BeginConstruction();
				DigitanksGame()->SetControlMode(MODE_NONE);
			}
		}
	}

	Vector vecMousePosition;
	CBaseEntity* pClickedEntity = NULL;
	bool bFound = GetMouseGridPosition(vecMousePosition, &pClickedEntity);

	if (iButton == GLFW_MOUSE_BUTTON_1)
	{
		if (iState == GLFW_PRESS)
		{
			// Prevent UI interactions from affecting the camera target.
			// If the mouse was used on the UI, m_bCameraMouseDown will
			// remain false.
			m_bCameraMouseDown = true;
			m_iMouseMoved = 0;
		}
		else
		{
			if (m_iMouseMoved < 30 && GetGame() && GetGame()->GetCamera())
			{
				if (m_bCameraMouseDown)
				{
					GetGame()->GetCamera()->SetTarget(vecMousePosition);
					m_bCameraMouseDown = false;
					CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_MOVECAMERA);
				}
			}
			else if (m_iMouseMoved > 30)
				CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_TURNCAMERA);
		}
	}

	if (iState == GLFW_RELEASE && iButton == GLFW_MOUSE_BUTTON_1 && m_iMouseMoved < 30)
	{
		if (pClickedEntity)
		{
			CSelectable* pSelectable = dynamic_cast<CSelectable*>(pClickedEntity);

			if (pSelectable)
				DigitanksGame()->GetLocalDigitanksTeam()->SetCurrentSelection(pSelectable);
		}
	}

	if (iState == GLFW_PRESS && iButton == GLFW_MOUSE_BUTTON_2)
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
			DigitanksGame()->SetDesiredMove(IsShiftDown());
		else if (DigitanksGame()->GetControlMode() == MODE_TURN)
			DigitanksGame()->SetDesiredTurn(bFound && IsShiftDown(), vecMousePosition);
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
			DigitanksGame()->SetDesiredAim(IsShiftDown());
	}

	GetHUD()->SetupMenu();
}

void CDigitanksWindow::MouseWheel(int iState)
{
	static int iOldState = 0;

	if (GetGame() && GetGame()->GetCamera())
	{
		if (iState > iOldState)
			GetGame()->GetCamera()->ZoomIn();
		else
			GetGame()->GetCamera()->ZoomOut();
	}

	iOldState = iState;
}

void CDigitanksWindow::KeyEvent(int c, int e)
{
	if (e == GLFW_PRESS)
		KeyPress(c);
	else
		KeyRelease(c);
}

void CDigitanksWindow::KeyPress(int c)
{
	if (glgui::CRootPanel::Get()->KeyPressed(c))
		return;

	if (DigitanksGame() && (c == GLFW_KEY_ENTER || c == GLFW_KEY_KP_ENTER))
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
			DigitanksGame()->SetDesiredMove();
		else if (DigitanksGame()->GetControlMode() == MODE_TURN)
			DigitanksGame()->SetDesiredTurn();
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
			DigitanksGame()->SetDesiredAim();
		else if (DigitanksGame()->GetControlMode() == MODE_FIRE)
			DigitanksGame()->SetControlMode(MODE_NONE);

		else if (!m_pInstructor->IsFeatureDisabled(DISABLE_ENTER))
		{
			CSoundLibrary::PlaySound(NULL, "sound/turn.wav");
			DigitanksGame()->EndTurn();
			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_ENTERKEY);
			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_POWER);
		}
	}

	if (DigitanksGame() && c == GLFW_KEY_SPACE)
	{
		// Don't clobber existing commands when scrolling through tanks.
		if (m_pHUD->ShouldAutoProceed())
		{
			// Set desired move so that the tank knows the player selected something.
			if (DigitanksGame()->GetControlMode() == MODE_MOVE)
			{
				if (DigitanksGame()->GetCurrentTank())
					DigitanksGame()->GetCurrentTank()->SetPreviewMove(Vector(9999, 9999, 9999));	// Make sure the mouse isn't hovering a legal move.
				DigitanksGame()->SetDesiredMove(IsShiftDown());
			}
		}

		DigitanksGame()->GetCurrentTeam()->NextTank();
	}

	if (DigitanksGame() && c == GLFW_KEY_TAB)
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

	if (c == GLFW_KEY_ESC)
	{
		if (GetMenu()->IsVisible())
			GetMenu()->SetVisible(false);
		else if (DigitanksGame()->GetControlMode() == MODE_NONE || DigitanksGame()->GetCurrentSelection() == NULL)
			GetMenu()->SetVisible(true);
		else
			DigitanksGame()->SetControlMode(MODE_NONE);
	}

	if (c == 'H')
	{
		if (DigitanksGame()->GetLocalDigitanksTeam())
		{
			for (size_t i = 0; i < DigitanksGame()->GetLocalDigitanksTeam()->GetNumMembers(); i++)
			{
				CBaseEntity* pMember = DigitanksGame()->GetLocalDigitanksTeam()->GetMember(i);
				CCPU* pCPU = dynamic_cast<CCPU*>(pMember);
				if (pCPU)
				{
					DigitanksGame()->GetLocalDigitanksTeam()->SetCurrentSelection(pCPU);
					break;
				}
			}
		}
	}

	if (GetGame() && GetGame()->GetCamera())
		GetGame()->GetCamera()->KeyDown(c);

	// Cheats from here on out
	if (c == 'X')
		DigitanksGame()->SetRenderFogOfWar(!DigitanksGame()->ShouldRenderFogOfWar());

	if (c == 'C')
		DigitanksGame()->CompleteProductions();

	if (c == 'V')
	{
		if (DigitanksGame()->GetCurrentSelection())
			DigitanksGame()->GetCurrentSelection()->Delete();
	}

	if (c == 'B')
	{
		CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentTeam();
		for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
		{
			for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
			{
				if (DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y].m_eUpdateClass == UPDATECLASS_EMPTY)
					continue;

				pTeam->DownloadUpdate(x, y, false);
				pTeam->DownloadComplete();
			}
		}
	}

	if (c == 'N')
		CDigitanksWindow::Get()->GetHUD()->SetVisible(!CDigitanksWindow::Get()->GetHUD()->IsVisible());
}

void CDigitanksWindow::KeyRelease(int c)
{
	if (GetGame() && GetGame()->GetCamera())
		GetGame()->GetCamera()->KeyUp(c);
}

bool CDigitanksWindow::IsCtrlDown()
{
	return glfwGetKey(GLFW_KEY_LCTRL) || glfwGetKey(GLFW_KEY_LCTRL);
}

bool CDigitanksWindow::IsAltDown()
{
	return glfwGetKey(GLFW_KEY_LALT) || glfwGetKey(GLFW_KEY_LALT);
}

bool CDigitanksWindow::IsShiftDown()
{
	return glfwGetKey(GLFW_KEY_LSHIFT) || glfwGetKey(GLFW_KEY_LSHIFT);
}

