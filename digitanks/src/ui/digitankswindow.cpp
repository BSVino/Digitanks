#include "digitankswindow.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <assert.h>
#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <time.h>
#include <vector.h>
#include <strutils.h>

#include <mtrand.h>
#include <configfile.h>
#include <platform.h>
#include <network/network.h>
#include <sound/sound.h>
#include "glgui/glgui.h"
#include "digitanks/digitanksgame.h"
#include "debugdraw.h"
#include "hud.h"
#include "instructor.h"
#include "game/camera.h"
#include "shaders/shaders.h"
#include "menu.h"
#include "ui.h"
#include "renderer/renderer.h"

ConfigFile c( GetAppDataDirectory(L"Digitanks", L"options.cfg") );

CDigitanksWindow::CDigitanksWindow(int argc, char** argv)
	: CApplication(argc, argv)
{
	m_pGameServer = NULL;
	m_pHUD = NULL;
	m_pInstructor = NULL;

	memset(&m_oGameSettings, 0, sizeof(m_oGameSettings));
	m_oGameSettings.iLevel = ~0;

	m_bBoxSelect = false;

	m_iMouseLastX = 0;
	m_iMouseLastY = 0;

	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	if (c.isFileValid())
	{
		m_iWindowWidth = c.read<int>("width", 1024);
		m_iWindowHeight = c.read<int>("height", 768);

		m_bCfgFullscreen = !c.read<bool>("windowed", false);
		m_bConstrainMouse = c.read<bool>("constrainmouse", true);

		m_bWantsFramebuffers = c.read<bool>("useframebuffers", true);
		m_bWantsShaders = c.read<bool>("useshaders", true);

		SetSoundVolume(c.read<float>("soundvolume", 0.8f));
		SetMusicVolume(c.read<float>("musicvolume", 0.8f));
	}
	else
	{
		m_iWindowWidth = iScreenWidth*2/3;
		m_iWindowHeight = iScreenHeight*2/3;

#ifdef _DEBUG
		m_bCfgFullscreen = false;
#else
		m_bCfgFullscreen = true;
#endif

		m_bConstrainMouse = true;

		m_bWantsFramebuffers = true;
		m_bWantsShaders = true;

		SetSoundVolume(0.8f);
		SetMusicVolume(0.8f);
	}

	if (m_iWindowWidth < 1024)
		m_iWindowWidth = 1024;

	if (m_iWindowHeight < 768)
		m_iWindowHeight = 768;
}

void CDigitanksWindow::OpenWindow()
{
	BaseClass::OpenWindow(m_iWindowWidth, m_iWindowHeight, m_bCfgFullscreen, false);

	ilInit();

	m_iCursors = CRenderer::LoadTextureIntoGL(L"textures/cursors.png");

	m_iLoading = CRenderer::LoadTextureIntoGL(L"textures/loading.png");
	RenderLoading();

	InitUI();

	CNetwork::Initialize();

	// Save out the configuration file now that we know this config loads properly.
	SetConfigWindowDimensions(m_iWindowWidth, m_iWindowHeight);
	SaveConfig();
}

CDigitanksWindow::~CDigitanksWindow()
{
	CNetwork::Deinitialize();

	delete m_pMenu;
	delete m_pMainMenu;

	DestroyGame();
}

void CDigitanksWindow::RenderLoading()
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, m_iWindowWidth, m_iWindowHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glgui::CRootPanel::PaintTexture(m_iLoading, m_iWindowWidth/2 - 150, m_iWindowHeight/2 - 150, 300, 300);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();

	SwapBuffers();
}

void CDigitanksWindow::RenderMouseCursor()
{
	if (m_eMouseCursor == MOUSECURSOR_NONE)
		return;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, (double)m_iWindowWidth, (double)m_iWindowHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glShadeModel(GL_SMOOTH);

	int mx, my;
	GetMousePosition(mx, my);

	if (m_eMouseCursor == MOUSECURSOR_SELECT)
		glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 160, 0, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_BUILD)
		glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 0, 0, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_BUILDINVALID)
		glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 80, 0, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_MOVE)
		glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 0, 40, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_MOVEAUTO)
		glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 80, 40, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_ROTATE)
		glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 0, 80, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_AIM)
		glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 160, 40, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_AIMENEMY)
		glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 80, 80, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_AIMINVALID)
		glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 160, 80, 80, 40, 256, 128);

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

void CDigitanksWindow::CreateGame(gametype_t eGameType)
{
	RenderLoading();

	if (eGameType != GAMETYPE_MENU)
		CSoundLibrary::StopMusic();

	if (eGameType == GAMETYPE_MENU)
	{
		if (!CSoundLibrary::IsMusicPlaying() && !HasCommandLineSwitch("--no-music"))
			CSoundLibrary::PlayMusic(L"sound/assemble-for-victory.ogg");
	}
	else if (!HasCommandLineSwitch("--no-music"))
		CSoundLibrary::PlayMusic(L"sound/network-rise-network-fall.ogg", true);

	mtsrand((size_t)time(NULL));

	eastl::string sHost = convertstring<char16_t, char>(m_sConnectHost);
	const char* pszPort = GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	if (!m_pGameServer)
	{
		m_pHUD = new CHUD();
		glgui::CRootPanel::Get()->AddControl(m_pHUD);

		m_pGameServer = new CGameServer();

		if (!m_pInstructor)
			m_pInstructor = new CInstructor();
	}

	if (GameServer())
	{
		GameServer()->SetConnectHost(sHost);
		GameServer()->SetServerType(m_eServerType);
		if (eGameType == GAMETYPE_MENU)
			GameServer()->SetServerType(SERVER_LOCAL);
		GameServer()->SetServerPort(iPort);
		GameServer()->Initialize();

		CNetwork::SetCallbacks(m_pGameServer, CGameServer::ClientConnectCallback, CGameServer::ClientDisconnectCallback);
	}

	if (CNetwork::IsHost() && DigitanksGame())
	{
		DigitanksGame()->SetGameSettings(m_oGameSettings);
		DigitanksGame()->SetupGame(eGameType);
	}

	glgui::CRootPanel::Get()->Layout();

	m_pMainMenu->SetVisible(eGameType == GAMETYPE_MENU);

	// Since loading is done, remove old game settings.
	memset(&m_oGameSettings, 0, sizeof(m_oGameSettings));
	m_oGameSettings.iLevel = ~0;
}

void CDigitanksWindow::DestroyGame()
{
	TMsg(L"Destroying game.\n");

	RenderLoading();

	CNetwork::Disconnect();

	if (m_pGameServer)
		delete m_pGameServer;

	if (m_pHUD)
	{
		glgui::CRootPanel::Get()->RemoveControl(m_pHUD);
		delete m_pHUD;
	}

	if (m_pInstructor)
		delete m_pInstructor;

	m_pGameServer = NULL;
	m_pHUD = NULL;
	m_pInstructor = NULL;

	CSoundLibrary::StopMusic();
}

void CDigitanksWindow::Run()
{
	CreateGame(GAMETYPE_MENU);

/*	if (!IsRegistered())
	{
		m_pMainMenu->SetVisible(false);
		m_pPurchase->OpeningApplication();
	}*/

	while (IsOpen())
	{
		SetMouseCursor(MOUSECURSOR_NONE);

		ConstrainMouse();

		if (GameServer()->IsHalting())
		{
			DestroyGame();
			CreateGame(GAMETYPE_MENU);
		}

		float flTime = GetTime();
		if (GameServer())
		{
			if (GameServer()->IsLoading())
			{
				CNetwork::Think();
				RenderLoading();
			}
			else if (GameServer()->IsClient() && !CNetwork::IsConnected())
			{
				DestroyGame();
				CreateGame(GAMETYPE_MENU);
			}
			else
			{
				GameServer()->Think(flTime);
				Render();
			}
		}
		else
			// Clear the buffer for the gui.
			glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

		SwapBuffers();
	}
}

void CDigitanksWindow::ConstrainMouse()
{
#ifdef _WIN32
	if (IsFullscreen())
		return;

	HWND hWindow = FindWindow(NULL, L"Digitanks!");

	if (!hWindow)
		return;

	HWND hActiveWindow = GetActiveWindow();
	if (ShouldConstrainMouse())
	{
		RECT rc;
		GetClientRect(hWindow, &rc);

		// Convert the client area to screen coordinates.
		POINT pt = { rc.left, rc.top };
		POINT pt2 = { rc.right, rc.bottom };
		ClientToScreen(hWindow, &pt);
		ClientToScreen(hWindow, &pt2);
		SetRect(&rc, pt.x, pt.y, pt2.x, pt2.y);

		// Confine the cursor.
		ClipCursor(&rc);
	}
	else
		ClipCursor(NULL);
#endif
}

void CDigitanksWindow::Render()
{
	if (!GameServer())
		return;

	GameServer()->Render();

	glgui::CRootPanel::Get()->Think(GameServer()->GetGameTime());
	glgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

	RenderMouseCursor();
}

void CDigitanksWindow::WindowResize(int w, int h)
{
	if (GameServer() && GameServer()->GetRenderer())
		GameServer()->GetRenderer()->SetSize(w, h);

	BaseClass::WindowResize(w, h);
}

bool CDigitanksWindow::ShouldConstrainMouse()
{
	if (!m_bConstrainMouse)
		return false;

#ifdef _WIN32
	HWND hWindow = FindWindow(NULL, L"Digitanks!");
	HWND hActiveWindow = GetActiveWindow();

	if (hActiveWindow != hWindow)
		return false;
#endif

	if (GameServer() && GameServer()->IsLoading())
		return false;
	
	if (!DigitanksGame() || DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return false;
	
	if (GetMenu()->IsVisible())
		return false;
	
	if (IsConsoleOpen())
		return false;

	return true;
}

bool CDigitanksWindow::GetMouseGridPosition(Vector& vecPoint, CBaseEntity** pHit, int iCollisionGroup)
{
	if (!DigitanksGame()->GetTerrain())
		return false;

	int x, y;
	glgui::CRootPanel::Get()->GetFullscreenMousePos(x, y);

	Vector vecWorld = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));

	Vector vecCameraVector = GameServer()->GetCamera()->GetCameraPosition();

	Vector vecRay = (vecWorld - vecCameraVector).Normalized();

	return GameServer()->GetGame()->TraceLine(vecCameraVector, vecCameraVector+vecRay*1000, vecPoint, pHit, iCollisionGroup);
}

void CDigitanksWindow::GameOver(bool bPlayerWon)
{
	DigitanksGame()->SetControlMode(MODE_NONE);
	GetInstructor()->SetActive(false);
	m_pVictory->GameOver(bPlayerWon);
}

void CDigitanksWindow::CloseApplication()
{
	if (IsRegistered())
		exit(0);

	if (m_pPurchase->IsVisible())
		exit(0);

	m_pMenu->SetVisible(false);
	m_pMainMenu->SetVisible(false);
	m_pPurchase->ClosingApplication();

	SaveConfig();
}

void CDigitanksWindow::SaveConfig()
{
	c.add<float>("soundvolume", GetSoundVolume());
	c.add<float>("musicvolume", GetMusicVolume());
	c.add<bool>("windowed", !m_bCfgFullscreen);
	c.add<bool>("constrainmouse", m_bConstrainMouse);
	c.add<bool>("useframebuffers", m_bWantsFramebuffers);
	c.add<bool>("useshaders", m_bWantsShaders);
	c.add<int>("width", m_iCfgWidth);
	c.add<int>("height", m_iCfgHeight);
	std::ofstream o;
	o.open(GetAppDataDirectory(L"Digitanks", L"options.cfg").c_str(), std::ios_base::out);
	o << c;

	TMsg(L"Saved config.\n");
}

CInstructor* CDigitanksWindow::GetInstructor()
{
	if (!m_pInstructor)
		m_pInstructor = new CInstructor();

	return m_pInstructor;
}

void CDigitanksWindow::SetSoundVolume(float flSoundVolume)
{
	m_flSoundVolume = flSoundVolume;
	CSoundLibrary::SetSoundVolume(m_flSoundVolume);
}

void CDigitanksWindow::SetMusicVolume(float flMusicVolume)
{
	m_flMusicVolume = flMusicVolume;
	CSoundLibrary::SetMusicVolume(m_flMusicVolume);
}
