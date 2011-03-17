#include "digitankswindow.h"

#include <sound/sound.h>
#include "glgui/glgui.h"
#include "digitanks/digitanksgame.h"
#include "instructor.h"
#include "hud.h"
#include "ui.h"
#include <game/digitanks/structures/cpu.h>
#include <game/digitanks/dt_camera.h>
#include <renderer/renderer.h>
#include <renderer/dissolver.h>
#include <tinker/keys.h>

void CDigitanksWindow::MouseMotion(int x, int y)
{
	glgui::CRootPanel::Get()->CursorMoved(x, y);

	if (DigitanksGame() && DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	if (GameServer() && GameServer()->GetCamera())
		GameServer()->GetCamera()->MouseInput(x, y);

	if (IsMouseRightDown())
		m_iMouseMoved += (int)(fabs((float)x-m_iMouseLastX) + fabs((float)y-m_iMouseLastY));

	if (IsMouseLeftDown())
	{
		m_iMouseCurrentX = x;
		m_iMouseCurrentY = y;
	}

	m_iMouseLastX = x;
	m_iMouseLastY = y;
}

void CDigitanksWindow::MouseInput(int iButton, int iState)
{
	bool bDoubleClick = false;
	if (iButton == TINKER_KEY_MOUSE_LEFT && iState == 0)
	{
		if (GameServer()->GetGameTime() < m_flLastClick)
			m_flLastClick = 0;

		if (GameServer()->GetGameTime() - m_flLastClick < 0.2f)
			bDoubleClick = true;
		else
			m_flLastClick = GameServer()->GetGameTime();
	}

	if (GameServer() && GameServer()->GetCamera())
	{
		// MouseButton enables camera rotation, so don't send the signal if the feature is disabled.
		if (!m_pInstructor->IsFeatureDisabled(DISABLE_ROTATE))
			GameServer()->GetCamera()->MouseButton(iButton, iState);
	}

	int mx, my;
	GetMousePosition(mx, my);
	if (iState == 1)
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
		{
			m_bMouseDownInGUI = false;
			return;
		}
	}

	if (!DigitanksGame())
		return;

	if (DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	Vector vecMousePosition;
	CBaseEntity* pClickedEntity = NULL;
	GetMouseGridPosition(vecMousePosition, &pClickedEntity);
	GetMouseGridPosition(vecMousePosition, NULL, CG_TERRAIN);

	bool bMouseAction = false;
	if (iButton == TINKER_KEY_MOUSE_RIGHT && iState == 0 && m_iMouseMoved < 30)
		bMouseAction = true;
	if (iButton == TINKER_KEY_MOUSE_LEFT && iState == 1)
		bMouseAction = true;

	if (DigitanksGame()->GetControlMode() != MODE_NONE && bMouseAction)
	{
		// While aiming moving turning or building, either mouse button can be used and selections are disabled.

		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
			DigitanksGame()->MoveTanks();
		else if (DigitanksGame()->GetControlMode() == MODE_TURN)
			DigitanksGame()->TurnTanks(vecMousePosition);
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
		{
			DigitanksGame()->FireTanks();
			GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_ATTACK);
		}
		else if (DigitanksGame()->GetControlMode() == MODE_BUILD)
		{
			if (iButton == TINKER_KEY_MOUSE_RIGHT)
			{
				DigitanksGame()->SetControlMode(MODE_NONE);
			}
			else if (DigitanksGame()->GetCurrentLocalDigitanksTeam())
			{
				CCPU* pCPU = DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetPrimaryCPU();
				if (pCPU && pCPU->IsPreviewBuildValid())
				{
					pCPU->BeginConstruction();
					DigitanksGame()->SetControlMode(MODE_NONE);
				}
			}
		}

		GetHUD()->SetupMenu();

		if (iState == 1)
			// Don't allow the release to take any action either.
			m_bMouseDownInGUI = true;

		return;
	}

	if (iButton == TINKER_KEY_MOUSE_RIGHT)
	{
		if (iState == 1)
			m_iMouseMoved = 0;
		else
		{
			if (m_iMouseMoved > 30)
				GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_TURNCAMERA);
		}
	}

	if (iButton == TINKER_KEY_MOUSE_LEFT)
	{
		if (iState == 1)
		{
			// Prevent UI interactions from affecting the camera target.
			// If the mouse was used no the UI, this will remain false.
			m_bBoxSelect = true;
			m_iMouseInitialX = m_iMouseCurrentX = mx;
			m_iMouseInitialY = m_iMouseCurrentY = my;
		}
	}

	if (bDoubleClick && pClickedEntity && DigitanksGame()->GetCurrentLocalDigitanksTeam())
	{
		CSelectable* pClickedSelectable = dynamic_cast<CSelectable*>(pClickedEntity);

		if (pClickedSelectable)
		{
			DigitanksGame()->GetCurrentLocalDigitanksTeam()->SetPrimarySelection(pClickedSelectable);

			for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
			{
				CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
				if (!pEntity)
					continue;

				CSelectable* pSelectable = dynamic_cast<CSelectable*>(pEntity);
				if (!pSelectable)
					continue;

				if (pSelectable->GetVisibility() == 0)
					continue;

				if (pSelectable->GetTeam() != pClickedSelectable->GetTeam())
					continue;

				if (pSelectable->GetUnitType() != pClickedSelectable->GetUnitType())
					continue;

				if (pSelectable->Distance(pClickedEntity->GetOrigin()) > 25)
					continue;

				Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(pSelectable->GetOrigin());

				if (vecScreen.x < 0 || vecScreen.y < 0 || vecScreen.x > GetWindowWidth() || vecScreen.y > GetWindowHeight())
					continue;

				if (DigitanksGame()->GetCurrentLocalDigitanksTeam())
					DigitanksGame()->GetCurrentLocalDigitanksTeam()->AddToSelection(pSelectable);
			}
		}
	}
	else if (iState == 0 && iButton == TINKER_KEY_MOUSE_LEFT)
	{
		if (m_bBoxSelect && IsMouseDragging())
		{
			if (!IsShiftDown() && DigitanksGame()->GetCurrentLocalDigitanksTeam())
				DigitanksGame()->GetCurrentLocalDigitanksTeam()->SetPrimarySelection(NULL);

			size_t iLowerX = (m_iMouseInitialX < m_iMouseCurrentX) ? m_iMouseInitialX : m_iMouseCurrentX;
			size_t iLowerY = (m_iMouseInitialY < m_iMouseCurrentY) ? m_iMouseInitialY : m_iMouseCurrentY;
			size_t iHigherX = (m_iMouseInitialX > m_iMouseCurrentX) ? m_iMouseInitialX : m_iMouseCurrentX;
			size_t iHigherY = (m_iMouseInitialY > m_iMouseCurrentY) ? m_iMouseInitialY : m_iMouseCurrentY;

			for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
			{
				CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
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

				if (DigitanksGame()->GetCurrentLocalDigitanksTeam())
					DigitanksGame()->GetCurrentLocalDigitanksTeam()->AddToSelection(pSelectable);
			}

			if (DigitanksGame()->GetCurrentLocalDigitanksTeam() && DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetNumSelected() == 3)
				GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_BOXSELECT);
		}
		else if (pClickedEntity && DigitanksGame()->GetCurrentLocalDigitanksTeam())
		{
			CSelectable* pSelectable = dynamic_cast<CSelectable*>(pClickedEntity);

			if (pSelectable)
			{
				if (IsShiftDown())
					DigitanksGame()->GetCurrentLocalDigitanksTeam()->AddToSelection(pSelectable);
				else
					DigitanksGame()->GetCurrentLocalDigitanksTeam()->SetPrimarySelection(pSelectable);
			}
			else if (!IsShiftDown())
				DigitanksGame()->GetCurrentLocalDigitanksTeam()->SetPrimarySelection(NULL);

			if (DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetNumSelected() == 3)
				GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_SHIFTSELECT);
		}

		m_bBoxSelect = false;
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

void CDigitanksWindow::KeyPress(int c)
{
	if (glgui::CRootPanel::Get()->KeyPressed(c, IsCtrlDown()))
		return;

	if (DigitanksGame() && DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	if (c == TINKER_KEY_F4 && IsAltDown())
		exit(0);

	if (DigitanksGame() && (c == TINKER_KEY_ENTER || c == TINKER_KEY_KP_ENTER))
	{
		if (!m_pInstructor->IsFeatureDisabled(DISABLE_ENTER) && DigitanksGame()->GetCurrentLocalDigitanksTeam() == DigitanksGame()->GetCurrentTeam())
		{
			CSoundLibrary::PlaySound(NULL, L"sound/turn.wav");
			DigitanksGame()->EndTurn();
		}
	}

	if (c == TINKER_KEY_ESCAPE)
	{
		if (GetMenu()->IsVisible())
			GetMenu()->SetVisible(false);
		else if (DigitanksGame() && (DigitanksGame()->GetControlMode() == MODE_NONE || DigitanksGame()->GetPrimarySelection() == NULL))
			GetMenu()->SetVisible(true);
		else
			DigitanksGame()->SetControlMode(MODE_NONE);
	}

	if (GameServer() && GameServer()->GetCamera())
		GameServer()->GetCamera()->KeyDown(c);
}

void CDigitanksWindow::KeyRelease(int c)
{
	if (GameServer() && GameServer()->GetCamera())
		GameServer()->GetCamera()->KeyUp(c);
}

void CDigitanksWindow::CharPress(int c)
{
	if (c == '`')
	{
		ToggleConsole();
		return;
	}

	if (glgui::CRootPanel::Get()->CharPressed(c))
		return;

	if (!GetHUD())
		return;

	if (c == ' ')
		DigitanksGame()->WeaponSpecialCommand();

	if (c == 'h')
	{
		if (DigitanksGame()->GetCurrentLocalDigitanksTeam())
		{
			for (size_t i = 0; i < DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetNumMembers(); i++)
			{
				CBaseEntity* pMember = DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetMember(i);
				CCPU* pCPU = dynamic_cast<CCPU*>(pMember);
				if (pCPU)
				{
					DigitanksGame()->GetCurrentLocalDigitanksTeam()->SetPrimarySelection(pCPU);
					break;
				}
			}
		}
	}

	if (c == 'q')
		GetHUD()->ButtonCallback(0);

	if (c == 'w')
		GetHUD()->ButtonCallback(1);

	if (c == 'e')
		GetHUD()->ButtonCallback(2);

	if (c == 'r')
		GetHUD()->ButtonCallback(3);

	if (c == 't')
		GetHUD()->ButtonCallback(4);

	if (c == 'a')
		GetHUD()->ButtonCallback(5);

	if (c == 's')
		GetHUD()->ButtonCallback(6);

	if (c == 'd')
		GetHUD()->ButtonCallback(7);

	if (c == 'f')
		GetHUD()->ButtonCallback(8);

	if (c == 'g')
		GetHUD()->ButtonCallback(9);

	if (!DigitanksGame()->AllowCheats())
		return;

	// Cheats from here on out
	if (c == 'x')
		DigitanksGame()->SetRenderFogOfWar(!DigitanksGame()->ShouldRenderFogOfWar());

	if (c == 'c')
		DigitanksGame()->CompleteProductions();

	if (c == 'v')
	{
		if (DigitanksGame()->GetPrimarySelection())
			DigitanksGame()->GetPrimarySelection()->Delete();
	}

	if (c == 'b')
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

	if (c == 'n')
		GetHUD()->SetVisible(!GetHUD()->IsVisible());

	if (c == 'm')
	{
		if (DigitanksGame()->GetPrimarySelection())
			DigitanksGame()->TankSpeak(DigitanksGame()->GetPrimarySelectionTank(), ":D!");
	}
}

void CDigitanksWindow::CharRelease(int c)
{
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

bool CDigitanksWindow::IsMouseDragging()
{
	size_t iDifference = abs((int)m_iMouseCurrentX - (int)m_iMouseInitialX) + abs((int)m_iMouseCurrentY - (int)m_iMouseInitialY);

	return iDifference > 30;
}
