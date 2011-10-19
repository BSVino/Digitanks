#include "digitankswindow.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <GL/glew.h>
#include <time.h>
#include <IL/il.h>

#include <common.h>
#include <vector.h>
#include <strutils.h>

#include <mtrand.h>
#include <configfile.h>
#include <tinker_platform.h>
#include <network/network.h>
#include <network/commands.h>
#include <sound/sound.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <tinker/portals/portal.h>
#include <models/texturelibrary.h>
#include <tinker/lobby/lobby_client.h>
#include <tinker/lobby/lobby_server.h>
#include <tinker/console.h>
#include <renderer/renderer.h>
#include <shaders/shaders.h>

#include "glgui/glgui.h"
#include "digitanksgame.h"
#include "digitankslevel.h"
#include "hud.h"
#include "instructor.h"
#include "game/camera.h"
#include "menu.h"
#include "ui.h"
#include "campaign/campaigndata.h"
#include "lobbyui.h"

#ifdef __linux__
// Put this last so it doesn't interfere with any other headers
#include <X11/Xlib.h>
#include "ext-deps/glfw-2.7.2/lib/internal.h"		// The GLFW internal header file
													// Needed to grab the window handle so we can restrict the mouse

static Display* g_pDisplay = NULL;
static Window g_iWindow = 0;
#endif

ConfigFile c( GetAppDataDirectory(_T("Digitanks"), _T("options.cfg")) );

CDigitanksWindow::CDigitanksWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
	ilInit();

	m_pGameServer = NULL;
	m_pRenderer = NULL;
	m_pHUD = NULL;
	m_pInstructor = NULL;
	m_pChatBox = NULL;
	m_pCampaign = NULL;

	m_eRestartAction = GAMETYPE_MENU;

	m_bBoxSelect = false;

	m_iMouseLastX = 0;
	m_iMouseLastY = 0;

	m_flLastClick = 0;

	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	if (c.isFileValid())
	{
		m_iWindowWidth = c.read<int>(_T("width"), 1024);
		m_iWindowHeight = c.read<int>(_T("height"), 768);

		m_bCfgFullscreen = !c.read<bool>(_T("windowed"), true);
		m_bConstrainMouse = c.read<bool>(_T("constrainmouse"), true);

		m_bContextualCommands = c.read<bool>(_T("contextualcommands"), false);
		m_bReverseSpacebar = c.read<bool>(_T("reversespacebar"), false);

		m_bWantsFramebuffers = c.read<bool>(_T("useframebuffers"), true);
		m_bWantsShaders = c.read<bool>(_T("useshaders"), true);

		SetSoundVolume(c.read<float>(_T("soundvolume"), 0.8f));
		SetMusicVolume(c.read<float>(_T("musicvolume"), 0.8f));

		m_iInstallID = c.read<int>(_T("installid"), RandomInt(10000000, 99999999));

		m_sNickname = c.read<tstring>(_T("nickname"), _T(""));
	}
	else
	{
		m_iWindowWidth = iScreenWidth*2/3;
		m_iWindowHeight = iScreenHeight*2/3;

		m_bCfgFullscreen = false;

		m_bConstrainMouse = true;

		m_bContextualCommands = false;
		m_bReverseSpacebar = false;

		m_bWantsFramebuffers = true;
		m_bWantsShaders = true;

		SetSoundVolume(0.8f);
		SetMusicVolume(0.8f);

		m_iInstallID = RandomInt(10000000, 99999999);

		m_sNickname = _T("");
	}

	CNetwork::SetClientInfo(m_iInstallID, m_sNickname);

	if (m_iWindowWidth < 1024)
		m_iWindowWidth = 1024;

	if (m_iWindowHeight < 768)
		m_iWindowHeight = 768;

	if (IsFile(GetAppDataDirectory(_T("Digitanks"), _T("campaign.txt"))))
	{
		m_pCampaign = new CCampaignData(CCampaignInfo::GetCampaignInfo());
		m_pCampaign->ReadData(GetAppDataDirectory(_T("Digitanks"), _T("campaign.txt")));
	}

	m_iTotalProgress = 0;
}

void CDigitanksWindow::OpenWindow()
{
	glgui::CLabel::AddFont(_T("header"), _T("fonts/header.ttf"));
	glgui::CLabel::AddFont(_T("text"), _T("fonts/text.ttf"));
	glgui::CLabel::AddFont(_T("smileys"), _T("fonts/smileys.ttf"));
	glgui::CLabel::AddFont(_T("cameramissile"), _T("fonts/cameramissile.ttf"));

	CApplication::OpenWindow(m_iWindowWidth, m_iWindowHeight, m_bCfgFullscreen, false);

	m_iCursors = CTextureLibrary::AddTextureID(_T("textures/cursors.png"));
	m_iLoading = CTextureLibrary::AddTextureID(_T("textures/loading.png"));
	m_iLunarWorkshop = CTextureLibrary::AddTextureID(_T("textures/lunar-workshop.png"));

	RenderLoading();

	InitUI();

	CNetwork::Initialize();

	// Save out the configuration file now that we know this config loads properly.
	SetConfigWindowDimensions(m_iWindowWidth, m_iWindowHeight);

	if (m_sNickname.length() == 0)
	{
		if (TPortal_IsAvailable())
		{
			m_sNickname = TPortal_GetPlayerNickname();
			TMsg(tstring(_T("Retrieved player nickname from ")) + TPortal_GetPortalIdentifier() + _T(": ") + m_sNickname + _T("\n"));
		}
		else
			m_sNickname = _T("Noobie");
	}

	SaveConfig();

#ifdef __linux__
	g_pDisplay = XOpenDisplay(NULL);
	g_iWindow = _glfwWin.window;
#endif
}

CDigitanksWindow::~CDigitanksWindow()
{
	CNetwork::Deinitialize();

	delete m_pMenu;
	delete m_pMainMenu;

	DestroyGame();

#ifdef __linux__
	XCloseDisplay(g_pDisplay);
#endif
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
	glgui::CRootPanel::PaintTexture(GetLunarWorkshopLogo(), m_iWindowWidth-200-20, m_iWindowHeight - 200, 200, 200);

	float flWidth = glgui::CLabel::GetTextWidth(m_sAction, m_sAction.length(), _T("text"), 12);
	glgui::CLabel::PaintText(m_sAction, m_sAction.length(), _T("text"), 12, (float)m_iWindowWidth/2 - flWidth/2, (float)m_iWindowHeight/2 + 170);

	if (m_iTotalProgress)
	{
		glDisable(GL_TEXTURE_2D);
		float flProgress = (float)m_iProgress/(float)m_iTotalProgress;
		glgui::CBaseControl::PaintRect(m_iWindowWidth/2 - 200, m_iWindowHeight/2 + 190, (int)(400*flProgress), 10, Color(255, 255, 255));
	}

	CApplication::Get()->GetConsole()->Paint();

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

#ifdef TINKER_OPTIMIZE_SOFTWARE
	glShadeModel(GL_FLAT);
#else
	glShadeModel(GL_SMOOTH);
#endif

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
	{
		if (Oscillate(GameServer()->GetGameTime(), 0.4f) > 0.3f)
			glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 80, 80, 80, 40, 256, 128);
		else
			glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 160, 40, 80, 40, 256, 128);
	}
	else if (m_eMouseCursor == MOUSECURSOR_AIMINVALID)
		glgui::CBaseControl::PaintSheet(m_iCursors, mx-20, my-20, 80, 40, 160, 80, 80, 40, 256, 128);

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

void LoadLevel(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	if (asTokens.size() == 1)
	{
		if (!GameServer())
		{
			TMsg("Use load_level 'levelpath' to specify the level.\n");
			return;
		}

		CDigitanksLevel* pLevel = CDigitanksGame::GetLevel(CVar::GetCVarValue(_T("game_level")));

		if (!pLevel)
		{
			TMsg(tstring(_T("Can't find file '")) + CVar::GetCVarValue(_T("game_level")) + _T("'.\n"));
			return;
		}

		DigitanksWindow()->CreateGame(pLevel->GetGameType());
		return;
	}

	CDigitanksLevel* pLevel = CDigitanksGame::GetLevel(asTokens[1]);

	if (!pLevel)
	{
		TMsg(tstring(_T("Can't find file '")) + asTokens[1] + _T("'.\n"));
		return;
	}

	CVar::SetCVar(_T("game_level"), pLevel->GetFile());

	DigitanksWindow()->CreateGame(pLevel->GetGameType());

	CApplication::CloseConsole();

	DigitanksWindow()->GetInstructor()->SetActive(false);
}

CCommand load_level("load_level", ::LoadLevel);
CVar game_type("game_type", "");

void CDigitanksWindow::CreateGame(gametype_t eRequestedGameType)
{
	gametype_t eGameType = GAMETYPE_MENU;
	if (eRequestedGameType == GAMETYPE_FROM_CVAR)
	{
		if (game_type.GetValue() == _T("menu"))
			eGameType = GAMETYPE_MENU;
		else if (game_type.GetValue() == _T("artillery"))
			eGameType = GAMETYPE_ARTILLERY;
		else if (game_type.GetValue() == _T("strategy"))
			eGameType = GAMETYPE_STANDARD;
		else if (game_type.GetValue() == _T("campaign"))
			eGameType = GAMETYPE_CAMPAIGN;
		else
			eGameType = GAMETYPE_EMPTY;
	}
	else if (eRequestedGameType == GAMETYPE_FROM_LOBBY)
		eGameType = (gametype_t)stoi(CGameLobbyClient::L_GetInfoValue(_T("gametype")).c_str());
	else
		eGameType = eRequestedGameType;

	if (eGameType == GAMETYPE_MENU)
		game_type.SetValue(_T("menu"));
	else if (eGameType == GAMETYPE_ARTILLERY)
		game_type.SetValue(_T("artillery"));
	else if (eGameType == GAMETYPE_STANDARD)
		game_type.SetValue(_T("strategy"));
	else if (eGameType == GAMETYPE_CAMPAIGN)
		game_type.SetValue(_T("campaign"));
	else
		game_type.SetValue(_T("empty"));

	// Suppress all network commands until the game is done loading.
	GameNetwork()->SetLoading(true);

	RenderLoading();

	if (eGameType != GAMETYPE_MENU)
		CSoundLibrary::StopMusic();

	if (eGameType == GAMETYPE_MENU)
	{
		if (!CSoundLibrary::IsMusicPlaying() && !HasCommandLineSwitch("--no-music"))
			CSoundLibrary::PlayMusic(_T("sound/assemble-for-victory.ogg"), true);
	}
	else if (!HasCommandLineSwitch("--no-music"))
		CSoundLibrary::PlayMusic(_T("sound/network-rise-network-fall.ogg"), true);

	mtsrand((size_t)time(NULL));

	const char* pszPort = GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	if (!m_pGameServer)
	{
		m_pHUD = new CHUD();
		glgui::CRootPanel::Get()->AddControl(m_pHUD);

		m_pGameServer = new CGameServer(this);

		if (!m_pRenderer)
		{
			m_pRenderer = CreateRenderer();
			m_pRenderer->Initialize();
		}

		if (!m_pInstructor)
			m_pInstructor = new CInstructor();
	}

	if (GameServer())
	{
		GameServer()->SetServerType(m_eServerType);
		if (eGameType == GAMETYPE_MENU)
			GameServer()->SetServerType(SERVER_LOCAL);
		GameServer()->SetServerPort(iPort);
		GameServer()->Initialize();

		GameNetwork()->SetCallbacks(m_pGameServer, CGameServer::ClientConnectCallback, CGameServer::ClientEnterGameCallback, CGameServer::ClientDisconnectCallback);
	}

	if (GameNetwork()->IsHost() && DigitanksGame())
	{
		GameServer()->SetupFromLobby(eRequestedGameType == GAMETYPE_FROM_LOBBY);
		DigitanksGame()->SetupGame(eGameType);
	}

	// Now turn the network on and connect all clients.
	GameNetwork()->SetLoading(false);

	// Must set player nickname after teams have been set up or it won't stick.
	if (GameServer())
		GameServer()->SetPlayerNickname(GetPlayerNickname());

	glgui::CRootPanel::Get()->Layout();

	m_pMainMenu->SetVisible(eGameType == GAMETYPE_MENU);
	m_pVictory->SetVisible(false);
}

void CDigitanksWindow::DestroyGame()
{
	TMsg(_T("Destroying game.\n"));

	RenderLoading();

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

void CDigitanksWindow::NewCampaign()
{
	if (!m_pCampaign)
		m_pCampaign = new CCampaignData(CCampaignInfo::GetCampaignInfo());

	CVar::SetCVar("game_level", m_pCampaign->BeginCampaign());

	DigitanksWindow()->SetServerType(SERVER_LOCAL);
	DigitanksWindow()->CreateGame(GAMETYPE_CAMPAIGN);
}

void CDigitanksWindow::RestartCampaignLevel()
{
	Restart(GAMETYPE_CAMPAIGN);
}

void CDigitanksWindow::NextCampaignLevel()
{
	eastl::string sNextLevel = m_pCampaign->ProceedToNextLevel();
	if (sNextLevel.length() == 0)
		Restart(GAMETYPE_MENU);
	else
	{
		GetVictoryPanel()->SetVisible(false);
		CVar::SetCVar("game_level", sNextLevel);
		Restart(GAMETYPE_CAMPAIGN);
	}

	m_pCampaign->SaveData(GetAppDataDirectory(_T("Digitanks"), _T("campaign.txt")));
}

void CDigitanksWindow::ContinueCampaign()
{
	eastl::string sLevel = m_pCampaign->GetCurrentLevelFile();
	if (sLevel.length() != 0)
	{
		GetVictoryPanel()->SetVisible(false);
		CVar::SetCVar("game_level", sLevel);
		Restart(GAMETYPE_CAMPAIGN);
	}

	m_pCampaign->SaveData(GetAppDataDirectory(_T("Digitanks"), _T("campaign.txt")));
}

SERVER_GAME_COMMAND(RestartLevel)
{
	if (GameServer()->ShouldSetupFromLobby())
		DigitanksWindow()->Restart(GAMETYPE_FROM_LOBBY);
	else
		DigitanksWindow()->Restart(GAMETYPE_FROM_CVAR);
}

void CDigitanksWindow::RestartLevel()
{
	TAssert(GameNetwork()->IsHost());
	if (!GameNetwork()->IsHost())
		return;

	::RestartLevel.RunCommand(_T(""));
}

void CDigitanksWindow::Restart(gametype_t eRestartAction)
{
	m_eRestartAction = eRestartAction;
	GameServer()->Halt();
}

void CDigitanksWindow::Run()
{
	CreateGame(GAMETYPE_MENU);

#ifndef DT_COMPETITION
	if (!IsRegistered())
	{
		m_pMainMenu->SetVisible(false);
		m_pPurchase->OpeningApplication();
	}
#endif

	while (IsOpen())
	{
		CProfiler::BeginFrame();

		if (true)
		{
			TPROF("CDigitanksWindow::Run");

			SetMouseCursor(MOUSECURSOR_NONE);

			ConstrainMouse();

			if (GameServer()->IsHalting())
			{
				DestroyGame();
				CreateGame(m_eRestartAction);

				m_eRestartAction = GAMETYPE_MENU;
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
				else if (GameServer()->IsClient() && !GameNetwork()->IsConnected())
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
		}

		CProfiler::Render();
		SwapBuffers();
	}
}

void CDigitanksWindow::ConstrainMouse()
{
	if (IsFullscreen())
		return;

#ifdef _WIN32
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
#else
#ifdef __linux__
	Window iFocus;
	int iRevert;
	XGetInputFocus(g_pDisplay, &iFocus, &iRevert);

	if (iFocus != g_iWindow)
		return;

	if (ShouldConstrainMouse())
	{
		unsigned int mask;
		Window window, root;
		int windowX, windowY, rootX, rootY;

		XQueryPointer( g_pDisplay,
			g_iWindow,
			&root,
			&window,
			&rootX, &rootY,
			&windowX, &windowY,
			&mask );

		if (windowX < 1)
		{
			XWarpPointer(g_pDisplay, None, g_iWindow, 0, 0, 0, 0, 1, windowY);
			windowX = 1;
		}

		if (windowY < 1)
		{
			XWarpPointer(g_pDisplay, None, g_iWindow, 0, 0, 0, 0, windowX, 1);
			windowY = 1;
		}

		if (windowX >= GetWindowWidth()-1)
		{
			XWarpPointer(g_pDisplay, None, g_iWindow, 0, 0, 0, 0, GetWindowWidth()-2, windowY);
			windowX = GetWindowWidth()-2;
		}

		if (windowY >= GetWindowHeight()-1)
			XWarpPointer(g_pDisplay, None, g_iWindow, 0, 0, 0, 0, windowX, GetWindowHeight()-2);
	}
#endif
#endif
}

void CDigitanksWindow::Render()
{
	if (!GameServer())
		return;

	TPROF("CDigitanksWindow::Render");

	GameServer()->Render();

	if (true)
	{
		TPROF("GUI");
		glgui::CRootPanel::Get()->Think(GameServer()->GetGameTime());
		glgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);
	}

	RenderMouseCursor();
}

int CDigitanksWindow::WindowClose()
{
	if (m_pPurchase->IsVisible())
		return GL_TRUE;

	CloseApplication();
	return GL_FALSE;
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

void CDigitanksWindow::OnClientDisconnect(int iClient)
{
	if (iClient == GameNetwork()->GetClientID())
		Restart(GAMETYPE_MENU);
}

void CDigitanksWindow::CloseApplication()
{
#ifdef DT_COMPETITION
	exit(0);
#endif

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
	c.add<float>(_T("soundvolume"), GetSoundVolume());
	c.add<float>(_T("musicvolume"), GetMusicVolume());
	c.add<bool>(_T("windowed"), !m_bCfgFullscreen);
	c.add<bool>(_T("constrainmouse"), m_bConstrainMouse);
	c.add<bool>(_T("contextualcommands"), m_bContextualCommands);
	c.add<bool>(_T("reversespacebar"), m_bReverseSpacebar);
	c.add<bool>(_T("useframebuffers"), m_bWantsFramebuffers);
	c.add<bool>(_T("useshaders"), m_bWantsShaders);
	c.add<int>(_T("width"), m_iCfgWidth);
	c.add<int>(_T("height"), m_iCfgHeight);
	c.add<int>(_T("installid"), m_iInstallID);
	c.add<tstring>(_T("nickname"), m_sNickname);
	std::basic_ofstream<tchar> o;
	o.open(convertstring<tchar, char>(GetAppDataDirectory(_T("Digitanks"), _T("options.cfg"))).c_str(), std::ios_base::out);
	o << c;

	TMsg(_T("Saved config.\n"));
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

CChatBox* CDigitanksWindow::GetChatBox()
{
	if (GetLobbyPanel()->IsVisible())
		return GetLobbyPanel()->GetChat();

	return BaseClass::GetChatBox();
}

void CDigitanksWindow::BeginProgress()
{
}

void CDigitanksWindow::SetAction(const tstring& sAction, size_t iTotalProgress)
{
	m_sAction = sAction;
	m_iTotalProgress = iTotalProgress;

	WorkProgress(0, true);
}

void CDigitanksWindow::WorkProgress(size_t iProgress, bool bForceDraw)
{
	if (!GameServer()->IsLoading())
		return;

	m_iProgress = iProgress;

	static float flLastTime = 0;

	CNetwork::Think();

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && GetTime() - flLastTime < 0.5f)
		return;

	RenderLoading();

	flLastTime = GetTime();
}

void CDigitanksWindow::EndProgress()
{
}
