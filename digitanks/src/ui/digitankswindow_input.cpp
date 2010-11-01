#include "digitankswindow.h"

#include <GL/glew.h>
#include <GL/glfw.h>

#include <sound/sound.h>
#include "glgui/glgui.h"
#include "digitanks/digitanksgame.h"
#include "instructor.h"
#include "hud.h"
#include "ui.h"
#include <game/digitanks/cpu.h>
#include <game/digitanks/dt_camera.h>

#include <renderer/renderer.h>
#include <renderer/dissolver.h>

#ifdef _DEBUG
#include <game/digitanks/maintank.h>
#endif

void CDigitanksWindow::MouseMotion(int x, int y)
{
	glgui::CRootPanel::Get()->CursorMoved(x, y);

	if (DigitanksGame() && DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	if (GameServer() && GameServer()->GetCamera())
		GameServer()->GetCamera()->MouseInput(x-m_iMouseStartX, y-m_iMouseStartY);

	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		m_iMouseMoved += (int)(fabs((float)x-m_iMouseStartX) + fabs((float)y-m_iMouseStartY));

	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		m_iMouseCurrentX = x;
		m_iMouseCurrentY = y;
	}

	m_iMouseStartX = x;
	m_iMouseStartY = y;
}

void CDigitanksWindow::MouseInput(int iButton, int iState)
{
	if (GameServer() && GameServer()->GetCamera())
	{
		// MouseButton enables camera rotation, so don't send the signal if the feature is disabled.
		if (!m_pInstructor->IsFeatureDisabled(DISABLE_ROTATE))
			GameServer()->GetCamera()->MouseButton(iButton, iState);
	}

	int mx, my;
	glfwGetMousePos(&mx, &my);
	if (iState == GLFW_PRESS)
	{
		if (glgui::CRootPanel::Get()->MousePressed(iButton, mx, my))
		{
			m_bMouseDownInGUI = true;
			return;
		}
		else
			m_bMouseDownInGUI = false;
	}
	else
	{
		if (glgui::CRootPanel::Get()->MouseReleased(iButton, mx, my))
			return;

		if (m_bMouseDownInGUI)
			return;
	}

	if (!DigitanksGame())
		return;

	if (DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	if (iState == GLFW_PRESS)
	{
		if ((iButton == GLFW_MOUSE_BUTTON_1 || iButton == GLFW_MOUSE_BUTTON_2) && DigitanksGame()->GetControlMode() == MODE_FIRE)
		{
			DigitanksGame()->SetControlMode(MODE_NONE);

			m_pInstructor->FinishedTutorial(CInstructor::TUTORIAL_ENERGY);

			return;	// Don't center camera
		}
	}
	else if (iButton == GLFW_MOUSE_BUTTON_2 && DigitanksGame()->GetControlMode() == MODE_BUILD && iState == GLFW_RELEASE && m_iMouseMoved < 30)
	{
		CCPU* pCPU = dynamic_cast<CCPU*>(DigitanksGame()->GetPrimarySelectionStructure());
		if (pCPU && pCPU->IsPreviewBuildValid())
		{
			pCPU->BeginConstruction();
			DigitanksGame()->SetControlMode(MODE_NONE);
		}
	}

	Vector vecMousePosition;
	CBaseEntity* pClickedEntity = NULL;
	GetMouseGridPosition(vecMousePosition, &pClickedEntity);
	GetMouseGridPosition(vecMousePosition, NULL, CG_TERRAIN);

	if (iButton == GLFW_MOUSE_BUTTON_2)
	{
		if (iState == GLFW_PRESS)
			m_iMouseMoved = 0;
		else
		{
			if (m_iMouseMoved > 30)
				CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_TURNCAMERA);
		}
	}

	size_t iDifference = abs((int)m_iMouseCurrentX - (int)m_iMouseInitialX) + abs((int)m_iMouseCurrentY - (int)m_iMouseInitialY);

	if (iButton == GLFW_MOUSE_BUTTON_1)
	{
		if (iState == GLFW_PRESS)
		{
			// Prevent UI interactions from affecting the camera target.
			// If the mouse was used no the UI, this will remain false.
			m_bBoxSelect = true;
			m_iMouseInitialX = m_iMouseCurrentX = mx;
			m_iMouseInitialY = m_iMouseCurrentY = my;
		}
		else if (GameServer() && GameServer()->GetCamera() && iDifference < 30)
		{
			DigitanksGame()->GetDigitanksCamera()->SetTarget(vecMousePosition);
			CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_MOVECAMERA);
		}
	}

	if (iState == GLFW_RELEASE && iButton == GLFW_MOUSE_BUTTON_1)
	{
		if (m_bBoxSelect && iDifference > 30)
		{
			if (!IsShiftDown())
				DigitanksGame()->GetLocalDigitanksTeam()->SetPrimarySelection(NULL);

			size_t iLowerX = (m_iMouseInitialX < m_iMouseCurrentX) ? m_iMouseInitialX : m_iMouseCurrentX;
			size_t iLowerY = (m_iMouseInitialY < m_iMouseCurrentY) ? m_iMouseInitialY : m_iMouseCurrentY;
			size_t iHigherX = (m_iMouseInitialX > m_iMouseCurrentX) ? m_iMouseInitialX : m_iMouseCurrentX;
			size_t iHigherY = (m_iMouseInitialY > m_iMouseCurrentY) ? m_iMouseInitialY : m_iMouseCurrentY;

			for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
			{
				CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
				if (!pEntity)
					continue;

				CSelectable* pSelectable = dynamic_cast<CSelectable*>(pEntity);
				if (!pSelectable)
					continue;

				if (pSelectable->GetVisibility() == 0)
					continue;

				Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(pSelectable->GetOrigin());

				if (vecScreen.x < iLowerX || vecScreen.y < iLowerY || vecScreen.x > iHigherX || vecScreen.y > iHigherY)
					continue;

				DigitanksGame()->GetLocalDigitanksTeam()->AddToSelection(pSelectable);
			}

			if (DigitanksGame()->GetLocalDigitanksTeam()->GetNumSelected() == 3)
				CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_BOXSELECT);
		}
		else if (pClickedEntity)
		{
			CSelectable* pSelectable = dynamic_cast<CSelectable*>(pClickedEntity);

			if (pSelectable)
			{
				if (IsShiftDown())
					DigitanksGame()->GetLocalDigitanksTeam()->AddToSelection(pSelectable);
				else
					DigitanksGame()->GetLocalDigitanksTeam()->SetPrimarySelection(pSelectable);
			}

			if (DigitanksGame()->GetLocalDigitanksTeam()->GetNumSelected() == 3)
				CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_SHIFTSELECT);
		}

		m_bBoxSelect = false;
	}

	if (iButton == GLFW_MOUSE_BUTTON_2 && iState == GLFW_RELEASE && m_iMouseMoved < 30)
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
			DigitanksGame()->MoveTanks();
		else if (DigitanksGame()->GetControlMode() == MODE_TURN)
			DigitanksGame()->TurnTanks(vecMousePosition);
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
		{
			DigitanksGame()->FireTanks();
			CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_ATTACK);
		}
	}

	GetHUD()->SetupMenu();
}

void CDigitanksWindow::MouseWheel(int iState)
{
	if (DigitanksGame() && DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	static int iOldState = 0;

	if (GameServer() && GameServer()->GetCamera())
	{
		if (iState > iOldState)
			DigitanksGame()->GetDigitanksCamera()->ZoomIn();
		else
			DigitanksGame()->GetDigitanksCamera()->ZoomOut();
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

void CDigitanksWindow::CharEvent(int c, int e)
{
	if (e == GLFW_PRESS)
		CharPress(c);
}

void CDigitanksWindow::KeyPress(int c)
{
	if (glgui::CRootPanel::Get()->KeyPressed(c))
		return;

	if (DigitanksGame() && DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	if (DigitanksGame() && (c == GLFW_KEY_ENTER || c == GLFW_KEY_KP_ENTER))
	{
		if (!m_pInstructor->IsFeatureDisabled(DISABLE_ENTER) && DigitanksGame()->GetLocalDigitanksTeam() == DigitanksGame()->GetCurrentTeam())
		{
			CSoundLibrary::PlaySound(NULL, "sound/turn.wav");
			DigitanksGame()->EndTurn();
		}
	}

	if (DigitanksGame() && c == GLFW_KEY_TAB)
	{
	}

	if (c == GLFW_KEY_ESC)
	{
		if (GetMenu()->IsVisible())
			GetMenu()->SetVisible(false);
		else if (DigitanksGame()->GetControlMode() == MODE_NONE || DigitanksGame()->GetPrimarySelection() == NULL)
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
					DigitanksGame()->GetLocalDigitanksTeam()->SetPrimarySelection(pCPU);
					break;
				}
			}
		}
	}

	if (GameServer() && GameServer()->GetCamera())
		GameServer()->GetCamera()->KeyDown(c);

	if (c == 'Q')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(0);

	if (c == 'W')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(1);

	if (c == 'E')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(2);

	if (c == 'R')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(3);

	if (c == 'T')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(4);

	if (c == 'A')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(5);

	if (c == 'S')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(6);

	if (c == 'D')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(7);

	if (c == 'F')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(8);

	if (c == 'G')
		CDigitanksWindow::Get()->GetHUD()->ButtonCallback(9);

	if (c == '`')
	{
		m_bCheatsOn = !m_bCheatsOn;
		return;
	}

	if (!m_bCheatsOn)
		return;

	// Cheats from here on out
	if (c == 'X')
		DigitanksGame()->SetRenderFogOfWar(!DigitanksGame()->ShouldRenderFogOfWar());

	if (c == 'C')
		DigitanksGame()->CompleteProductions();

	if (c == 'V')
	{
		if (DigitanksGame()->GetPrimarySelection())
			DigitanksGame()->GetPrimarySelection()->Delete();
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

	if (c == 'M')
	{
		if (DigitanksGame()->GetPrimarySelection())
			DigitanksGame()->TankSpeak(DigitanksGame()->GetPrimarySelectionTank(), ":D!");
	}
}

void CDigitanksWindow::KeyRelease(int c)
{
	if (GameServer() && GameServer()->GetCamera())
		GameServer()->GetCamera()->KeyUp(c);
}

void CDigitanksWindow::CharPress(int c)
{
	if (glgui::CRootPanel::Get()->CharPressed(c))
		return;
}

bool CDigitanksWindow::IsCtrlDown()
{
	return glfwGetKey(GLFW_KEY_LCTRL) || glfwGetKey(GLFW_KEY_RCTRL);
}

bool CDigitanksWindow::IsAltDown()
{
	return glfwGetKey(GLFW_KEY_LALT) || glfwGetKey(GLFW_KEY_RALT);
}

bool CDigitanksWindow::IsShiftDown()
{
	return glfwGetKey(GLFW_KEY_LSHIFT) || glfwGetKey(GLFW_KEY_RSHIFT);
}

bool CDigitanksWindow::GetBoxSelection(size_t& iX, size_t& iY, size_t& iX2, size_t& iY2)
{
	if (m_bBoxSelect)
	{
		iX = (m_iMouseInitialX < m_iMouseCurrentX) ? m_iMouseInitialX : m_iMouseCurrentX;
		iY = (m_iMouseInitialY < m_iMouseCurrentY) ? m_iMouseInitialY : m_iMouseCurrentY;
		iX2 = (m_iMouseInitialX > m_iMouseCurrentX) ? m_iMouseInitialX : m_iMouseCurrentX;
		iY2 = (m_iMouseInitialY > m_iMouseCurrentY) ? m_iMouseInitialY : m_iMouseCurrentY;
		return true;
	}

	return false;
}
