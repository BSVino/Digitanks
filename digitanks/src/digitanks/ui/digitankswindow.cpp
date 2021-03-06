#include "digitankswindow.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <time.h>

#include <common.h>
#include <vector.h>
#include <strutils.h>

#include <mtrand.h>
#include <tinker_platform.h>
#include <network/network.h>
#include <network/commands.h>
#include <sound/sound.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <tinker/portals/portal.h>
#include <textures/texturelibrary.h>
#include <tengine/lobby/lobby_client.h>
#include <tengine/lobby/lobby_server.h>
#include <tinker/console.h>
#include <renderer/renderer.h>
#include <renderer/shaders.h>
#include <renderer/game_renderingcontext.h>
#include <glgui/rootpanel.h>

#include "digitanksgame.h"
#include "digitankslevel.h"
#include "hud.h"
#include "game/entities/camera.h"
#include "menu.h"
#include "ui.h"
#include "campaign/campaigndata.h"
#include "lobbyui.h"
#include "dt_renderer.h"

#if defined(__linux__) && !defined(__ANDROID__)
#define USE_X
#endif

#ifdef USE_X
// Put this last so it doesn't interfere with any other headers
#include <X11/Xlib.h>
#include "ext-deps/glfw-2.7.2/lib/internal.h"		// The GLFW internal header file
													// Needed to grab the window handle so we can restrict the mouse

static Display* g_pDisplay = NULL;
static Window g_iWindow = 0;
#endif

using namespace glgui;

CDigitanksWindow::CDigitanksWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
	m_pGameServer = NULL;
	m_pRenderer = NULL;
	m_pHUD = NULL;
	m_pInstructor = NULL;
	m_pChatBox = NULL;
	m_pCampaign = NULL;

	m_eRestartAction = GAMETYPE_MENU;

	m_iMouseLastX = 0;
	m_iMouseLastY = 0;

	m_flLastClick = 0;

	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	TStubbed("Config");
	/*if (c.isFileValid())
	{
		m_iWindowWidth = c.read<int>("width", 1024);
		m_iWindowHeight = c.read<int>("height", 768);

		m_bCfgFullscreen = !c.read<bool>("windowed", true);
		m_bConstrainMouse = c.read<bool>("constrainmouse", true);

		m_bContextualCommands = c.read<bool>("contextualcommands", false);
		m_bReverseSpacebar = c.read<bool>("reversespacebar", true);

		SetSoundVolume(c.read<float>("soundvolume", 0.8f));
		SetMusicVolume(c.read<float>("musicvolume", 0.8f));

		m_iInstallID = c.read<int>("installid", RandomInt(10000000, 99999999));

		m_sNickname = c.read<tstring>("nickname", "");
	}
	else
	{*/
		m_iWindowWidth = iScreenWidth*2/3;
		m_iWindowHeight = iScreenHeight*2/3;

		m_bCfgFullscreen = false;

		m_bConstrainMouse = true;

		m_bContextualCommands = false;
		m_bReverseSpacebar = true;

		SetSoundVolume(0.8f);
		SetMusicVolume(0.8f);

		m_iInstallID = RandomInt(10000000, 99999999);

		m_sNickname = "";
	//}

	CNetwork::SetClientInfo(m_iInstallID, m_sNickname);

	if (m_iWindowWidth < 1024)
		m_iWindowWidth = 1024;

	if (m_iWindowHeight < 768)
		m_iWindowHeight = 768;

	if (IsFile(GetAppDataDirectory("campaign.txt")))
	{
		m_pCampaign = new CCampaignData(CCampaignInfo::GetCampaignInfo());
		m_pCampaign->ReadData(GetAppDataDirectory("campaign.txt"));
	}

	m_iTotalProgress = 0;
}

void CDigitanksWindow::OpenWindow()
{
	glgui::RootPanel()->AddFont("header", "fonts/header.ttf");
	glgui::RootPanel()->AddFont("text", "fonts/text.ttf");
	glgui::RootPanel()->AddFont("smileys", "fonts/smileys.ttf");
	glgui::RootPanel()->AddFont("cameramissile", "fonts/cameramissile.ttf");

	SetMultisampling(true);

	CApplication::OpenWindow(m_iWindowWidth, m_iWindowHeight, m_bCfgFullscreen, false);

	RenderLoading();

	m_hCursors = CMaterialLibrary::AddMaterial("textures/cursors.mat");

	CVar::SetCVar("game_mode", GetInitialGameMode());

	m_pGameServer = new CGameServer();
	m_pInstructor = new CInstructor();

	GameServer()->SetWorkListener(this);

	InitUI();

	CNetwork::Initialize();

	// Save out the configuration file now that we know this config loads properly.
	SetConfigWindowDimensions(m_iWindowWidth, m_iWindowHeight);

	if (m_sNickname.length() == 0)
	{
		if (TPortal_IsAvailable())
		{
			m_sNickname = TPortal_GetPlayerNickname();
			TMsg(tstring("Retrieved player nickname from ") + TPortal_GetPortalIdentifier() + ": " + m_sNickname + "\n");
		}
		else
			m_sNickname = "Commander";
	}

	SaveConfig();

	mtsrand((size_t)time(NULL));

	glgui::CRootPanel::Get()->AddControl(m_pHUD = CreateHUD());

	GameServer()->SetLoading(false);

	CApplication::Get()->SetMouseCursorEnabled(true);

#ifdef USE_X
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

#ifdef USE_X
	XCloseDisplay(g_pDisplay);
#endif
}

CRenderer* CDigitanksWindow::CreateRenderer()
{
	return new CDigitanksRenderer();
}

void CDigitanksWindow::RenderLoading()
{
	if (!m_hLoading)
		m_hLoading = CMaterialLibrary::AddMaterial("textures/loading.mat");

	if (!m_hLunarWorkshop)
		m_hLunarWorkshop = CMaterialLibrary::AddMaterial("textures/lunar-workshop.mat");

	CRenderingContext c(GetRenderer(), true);

	c.ClearColor();
	c.ClearDepth();

	c.SetProjection(Matrix4x4::ProjectOrthographic(0, RootPanel()->GetWidth(), RootPanel()->GetHeight(), 0, -1, 1));

	c.SetDepthTest(false);
	c.SetBlend(BLEND_ALPHA);

	glgui::CRootPanel::PaintTexture(m_hLoading, RootPanel()->GetWidth() / 2 - 150, RootPanel()->GetHeight() / 2 - 150, 300, 300);
	glgui::CRootPanel::PaintTexture(GetLunarWorkshopLogo(), RootPanel()->GetWidth() - 200 - 20, RootPanel()->GetHeight() - 200, 200, 200);

	float flWidth = glgui::RootPanel()->GetTextWidth(m_sAction, m_sAction.length(), "text", 12);
	glgui::CLabel::PaintText(m_sAction, m_sAction.length(), "text", 12, (float)RootPanel()->GetWidth() / 2 - flWidth / 2, (float)RootPanel()->GetHeight() / 2 + 170);

	if (m_iTotalProgress)
	{
		float flProgress = (float)m_iProgress/(float)m_iTotalProgress;
		glgui::CBaseControl::PaintRect(RootPanel()->GetWidth() / 2 - 200, RootPanel()->GetHeight() / 2 + 190, (int)(400 * flProgress), 10, Color(255, 255, 255));
	}

	CApplication::Get()->GetConsole()->Paint();

	SwapBuffers();
}

void CDigitanksWindow::RenderMouseCursor()
{
	if (m_eMouseCursor == MOUSECURSOR_NONE)
		return;

	TPROF("CDigitanksWindow::RenderMouseCursor");

	CRenderingContext c(GetRenderer(), true);

	c.ClearDepth();

	c.SetProjection(Matrix4x4::ProjectOrthographic(0, m_iWindowWidth, m_iWindowHeight, 0, -1, 1));

	c.SetDepthTest(false);
	c.SetBlend(BLEND_ALPHA);

	int mx, my;
	GetMousePosition(mx, my);

	if (m_eMouseCursor == MOUSECURSOR_SELECT)
		glgui::CBaseControl::PaintSheet(m_hCursors, mx-20, my-20, 80, 40, 160, 0, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_BUILD)
		glgui::CBaseControl::PaintSheet(m_hCursors, mx-20, my-20, 80, 40, 0, 0, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_BUILDINVALID)
		glgui::CBaseControl::PaintSheet(m_hCursors, mx-20, my-20, 80, 40, 80, 0, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_MOVE)
		glgui::CBaseControl::PaintSheet(m_hCursors, mx-20, my-20, 80, 40, 0, 40, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_MOVEAUTO)
		glgui::CBaseControl::PaintSheet(m_hCursors, mx-20, my-20, 80, 40, 80, 40, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_ROTATE)
		glgui::CBaseControl::PaintSheet(m_hCursors, mx-20, my-20, 80, 40, 0, 80, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_AIM)
		glgui::CBaseControl::PaintSheet(m_hCursors, mx-20, my-20, 80, 40, 160, 40, 80, 40, 256, 128);
	else if (m_eMouseCursor == MOUSECURSOR_AIMENEMY)
	{
		if (Oscillate(GameServer()->GetGameTime(), 0.4f) > 0.3f)
			glgui::CBaseControl::PaintSheet(m_hCursors, mx-20, my-20, 80, 40, 80, 80, 80, 40, 256, 128);
		else
			glgui::CBaseControl::PaintSheet(m_hCursors, mx-20, my-20, 80, 40, 160, 40, 80, 40, 256, 128);
	}
	else if (m_eMouseCursor == MOUSECURSOR_AIMINVALID)
		glgui::CBaseControl::PaintSheet(m_hCursors, mx-20, my-20, 80, 40, 160, 80, 80, 40, 256, 128);

	CRenderingContext::DebugFinish();
}

void CDigitanksWindow::NewCampaign()
{
	if (!m_pCampaign)
		m_pCampaign = new CCampaignData(CCampaignInfo::GetCampaignInfo());

	CVar::SetCVar("game_level", m_pCampaign->BeginCampaign());

	DigitanksWindow()->SetServerType(SERVER_LOCAL);
	GameWindow()->Restart("campaign");
}

void CDigitanksWindow::RestartCampaignLevel()
{
	Restart(GAMETYPE_CAMPAIGN);
}

void CDigitanksWindow::NextCampaignLevel()
{
	tstring sNextLevel = m_pCampaign->ProceedToNextLevel();
	if (sNextLevel.length() == 0)
		Restart(GAMETYPE_MENU);
	else
	{
		GetVictoryPanel()->SetVisible(false);
		CVar::SetCVar("game_level", sNextLevel);
		Restart(GAMETYPE_CAMPAIGN);
	}

	m_pCampaign->SaveData(GetAppDataDirectory("campaign.txt"));
}

void CDigitanksWindow::ContinueCampaign()
{
	tstring sLevel = m_pCampaign->GetCurrentLevelFile();
	if (sLevel.length() != 0)
	{
		GetVictoryPanel()->SetVisible(false);
		CVar::SetCVar("game_level", sLevel);
		Restart(GAMETYPE_CAMPAIGN);
	}

	m_pCampaign->SaveData(GetAppDataDirectory("campaign.txt"));
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

	::RestartLevel.RunCommand("");
}

void CDigitanksWindow::Restart(gametype_t eRestartAction)
{
	m_eRestartAction = eRestartAction;
	GameServer()->Halt();
}

void CDigitanksWindow::PreFrame()
{
	BaseClass::PreFrame();

	SetMouseCursor(MOUSECURSOR_NONE);

	ConstrainMouse();
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
#ifdef USE_X
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

	BaseClass::Render();

	RenderMouseCursor();
}

int CDigitanksWindow::WindowClose()
{
	CloseApplication();
	return 0;
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

bool CDigitanksWindow::GetMouseGridPosition(Vector& vecPoint, CBaseEntity** pHit, bool bTerrainOnly)
{
#ifdef T_PLATFROM_TOUCH
	// Mouse doesn't exist on this platform.
	TAssert(false);
#endif

	if (!DigitanksGame()->GetTerrain())
		return false;

	int x, y;
	glgui::CRootPanel::Get()->GetFullscreenMousePos(x, y);

	return GetGridPosition(Vector2D(x, y), vecPoint, pHit, bTerrainOnly);
}

bool CDigitanksWindow::GetGridPosition(const Vector2D& vecScreen, Vector& vecPoint, CBaseEntity** pHit, bool bTerrainOnly)
{
	Vector vecWorld = GameServer()->GetRenderer()->WorldPosition(Vector(vecScreen.x, vecScreen.y, 1));

	Vector vecCameraVector = GameServer()->GetRenderer()->GetCameraPosition();

	Vector vecRay = (vecWorld - vecCameraVector).Normalized();

	return DigitanksGame()->TraceLine(vecCameraVector, vecCameraVector + vecRay * 1000, vecPoint, pHit, bTerrainOnly);
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
	SaveConfig();

	exit(0);
}

void CDigitanksWindow::SaveConfig()
{
	TStubbed("SaveConfig()");

#if 0
	c.add<float>("soundvolume", GetSoundVolume());
	c.add<float>("musicvolume", GetMusicVolume());
	c.add<bool>("windowed", !m_bCfgFullscreen);
	c.add<bool>("constrainmouse", m_bConstrainMouse);
	c.add<bool>("contextualcommands", m_bContextualCommands);
	c.add<bool>("reversespacebar", m_bReverseSpacebar);
	c.add<int>("width", m_iCfgWidth);
	c.add<int>("height", m_iCfgHeight);
	c.add<int>("installid", m_iInstallID);
	c.add<tstring>("nickname", m_sNickname);
	std::basic_ofstream<tchar> o;
	o.open(convertstring<tchar, char>(GetAppDataDirectory("Digitanks", "options.cfg")).c_str(), std::ios_base::out);
	o << c;

	TMsg("Saved config.\n");
#endif
}

CHUD* CDigitanksWindow::GetHUD()
{
	return static_cast<CHUD*>(m_pHUD);
}

CInstructor* CDigitanksWindow::GetInstructor()
{
	return BaseClass::GetInstructor();
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
