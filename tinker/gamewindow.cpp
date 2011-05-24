#include "gamewindow.h"

#include <platform.h>

#include <tinker/profiler.h>
#include <game/gameserver.h>
#include <glgui/glgui.h>
#include <game/camera.h>

CGameWindow::CGameWindow(int argc, char** argv)
	: CApplication(argc, argv)
{
}

void CGameWindow::OpenWindow()
{
	int iScreenWidth, iScreenHeight;
	GetScreenSize(iScreenWidth, iScreenHeight);

	BaseClass::OpenWindow(iScreenWidth, iScreenHeight, true, false);

	RenderLoading();

	m_pGameServer = new CGameServer();
}

void CGameWindow::Run()
{
	while (IsOpen())
	{
		CProfiler::BeginFrame();

		if (true)
		{
			TPROF("CGameWindow::Run");

			PreFrame();

			if (GameServer()->IsHalting())
			{
				//DestroyGame();
				//CreateGame(m_eRestartAction);
			}

			float flTime = GetTime();
			if (GameServer())
			{
				if (GameServer()->IsLoading())
				{
					// Pump the network
					CNetwork::Think();
					RenderLoading();
					continue;
				}
				else if (GameServer()->IsClient() && !CNetwork::IsConnected())
				{
					//DestroyGame();
					//CreateGame(GAMETYPE_MENU);
				}
				else
				{
					GameServer()->Think(flTime);
					Render();
				}
			}
		}

		CProfiler::Render();
		SwapBuffers();
	}
}

void CGameWindow::Render()
{
	if (!GameServer())
		return;

	TPROF("CGameWindow::Render");

	GameServer()->Render();

	if (true)
	{
		TPROF("GUI");
		glgui::CRootPanel::Get()->Think(GameServer()->GetGameTime());
		glgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);
	}
}

void CGameWindow::KeyPress(int c)
{
	BaseClass::KeyPress(c);

	if (GameServer() && GameServer()->GetCamera())
		GameServer()->GetCamera()->KeyDown(c);
}

void CGameWindow::KeyRelease(int c)
{
	BaseClass::KeyRelease(c);

	if (GameServer() && GameServer()->GetCamera())
		GameServer()->GetCamera()->KeyUp(c);
}
