#include "digitankswindow.h"

#include <assert.h>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <time.h>
#include <vector.h>

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
#include "register.h"

CDigitanksWindow* CDigitanksWindow::s_pDigitanksWindow = NULL;

ConfigFile c( "options.cfg" );

CDigitanksWindow::CDigitanksWindow(int argc, char** argv)
{
	s_pDigitanksWindow = this;

	m_pGameServer = NULL;
	m_pHUD = NULL;
	m_pInstructor = NULL;

	srand((unsigned int)time(NULL));

	for (int i = 0; i < argc; i++)
		m_apszCommandLine.push_back(argv[i]);

	m_bBoxSelect = false;
	m_bCheatsOn = false;

	m_iMouseStartX = 0;
	m_iMouseStartY = 0;

	glfwInit();

	bool bFullscreen;

	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	if (c.isFileValid())
	{
		m_iWindowWidth = c.read<int>("width", 1024);
		m_iWindowHeight = c.read<int>("height", 768);

		bFullscreen = m_bFullscreen = !c.read<bool>("windowed", false);

		SetSoundVolume(c.read<float>("soundvolume", 0.8f));
		SetMusicVolume(c.read<float>("musicvolume", 0.8f));
	}
	else
	{
		m_iWindowWidth = iScreenWidth*2/3;
		m_iWindowHeight = iScreenHeight*2/3;

#ifdef _DEBUG
		bFullscreen = false;
#else
		bFullscreen = true;
#endif
		m_bFullscreen = true;

		SetSoundVolume(0.8f);
		SetMusicVolume(0.8f);
	}

	if (m_iWindowWidth < 1024)
		m_iWindowWidth = 1024;

	if (m_iWindowHeight < 768)
		m_iWindowHeight = 768;

	if (HasCommandLineSwitch("--fullscreen"))
		bFullscreen = true;

	if (HasCommandLineSwitch("--windowed"))
		bFullscreen = false;

	glfwEnable( GLFW_MOUSE_CURSOR );

	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	if (!glfwOpenWindow(m_iWindowWidth, m_iWindowHeight, 0, 0, 0, 0, 16, 0, bFullscreen?GLFW_FULLSCREEN:GLFW_WINDOW))
	{
		glfwTerminate();
		return;
	}

	glfwSetWindowTitle( "Digitanks!" );
	glfwSetWindowPos((int)(iScreenWidth/2-m_iWindowWidth/2), (int)(iScreenHeight/2-m_iWindowHeight/2));

	glfwSetWindowSizeCallback(&CDigitanksWindow::WindowResizeCallback);
	glfwSetKeyCallback(&CDigitanksWindow::KeyEventCallback);
	glfwSetCharCallback(&CDigitanksWindow::CharEventCallback);
	glfwSetMousePosCallback(&CDigitanksWindow::MouseMotionCallback);
	glfwSetMouseButtonCallback(&CDigitanksWindow::MouseInputCallback);
	glfwSetMouseWheelCallback(&CDigitanksWindow::MouseWheelCallback);
	glfwSwapInterval( 1 );
	glfwSetTime( 0.0 );
	glfwEnable( GLFW_MOUSE_CURSOR );

	ilInit();

	GLenum err = glewInit();
	if (GLEW_OK != err)
		exit(0);

	CShaderLibrary::CompileShaders();

	InitUI();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glLineWidth(1.0);

	CNetwork::Initialize();

	// Save out the configuration file now that we know this config loads properly.
	SetConfigWindowDimensions(m_iWindowWidth, m_iWindowHeight);
	SaveConfig();

	ReadProductCode();
}

CDigitanksWindow::~CDigitanksWindow()
{
	CNetwork::Deinitialize();

	delete m_pMenu;
	delete m_pMainMenu;

	DestroyGame();

	glfwTerminate();
}

void CDigitanksWindow::CreateGame(gametype_t eGameType)
{
	CSoundLibrary::StopMusic();

	mtsrand((size_t)glfwGetTime());

	if (GameServer())
		GameServer()->Initialize();

	const char* pszPort = GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	if (eGameType != GAMETYPE_MENU)
	{
		if (m_eServerType == SERVER_HOST)
			CNetwork::CreateHost(iPort);

		if (m_eServerType == SERVER_CLIENT)
		{
			std::string sHost;
			sHost.assign(m_sConnectHost.begin(), m_sConnectHost.end());
			CNetwork::ConnectToHost(sHost.c_str(), iPort);
			if (!CNetwork::IsConnected())
				return;
		}
	}

	if (!m_pGameServer)
	{
		m_pHUD = new CHUD();

		m_pGameServer = new CGameServer();
		m_pGameServer->Initialize();
		CNetwork::SetCallbacks(m_pGameServer, CGameServer::ClientConnectCallback, CGameServer::ClientDisconnectCallback);

		glgui::CRootPanel::Get()->AddControl(m_pHUD);

		if (!m_pInstructor)
			m_pInstructor = new CInstructor();
	}

	if (CNetwork::IsHost() && DigitanksGame())
	{
		DigitanksGame()->SetPlayers(m_iPlayers);
		DigitanksGame()->SetTanks(m_iTanks);
		DigitanksGame()->SetupGame(eGameType);
	}

	glgui::CRootPanel::Get()->Layout();

	m_pMainMenu->SetVisible(eGameType == GAMETYPE_MENU);
}

void CDigitanksWindow::DestroyGame()
{
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
}

void CDigitanksWindow::Run()
{
	CreateGame(GAMETYPE_MENU);

	if (!IsRegistered())
	{
		m_pMainMenu->SetVisible(false);
		m_pPurchase->OpeningApplication();
	}

	if (!HasCommandLineSwitch("--nomusic"))
		CSoundLibrary::PlayMusic("sound/assemble-for-victory.ogg");

	while (glfwGetWindowParam( GLFW_OPENED ))
	{
		if (GameServer()->IsHalting())
		{
			DestroyGame();
			CreateGame(GAMETYPE_MENU);
		}

		float flTime = (float)glfwGetTime();
		if (GameServer())
		{
			if (GameServer()->IsLoading())
				CNetwork::Think();
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

		glgui::CRootPanel::Get()->Think(flTime);
		glgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

		glfwSwapBuffers();
	}
}

void CDigitanksWindow::Render()
{
	if (GameServer())
		GameServer()->Render();
}

void CDigitanksWindow::WindowResize(int w, int h)
{
	if (GameServer() && GameServer()->GetRenderer())
		GameServer()->GetRenderer()->SetSize(w, h);

	m_iWindowWidth = w;
	m_iWindowHeight = h;

	if (GameServer())
		Render();

	glgui::CRootPanel::Get()->Layout();
	glgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

	glfwSwapBuffers();
}

bool CDigitanksWindow::GetMouseGridPosition(Vector& vecPoint, CBaseEntity** pHit)
{
	if (!DigitanksGame()->GetTerrain())
		return false;

	int x, y;
	glgui::CRootPanel::Get()->GetFullscreenMousePos(x, y);

	Vector vecWorld = GameServer()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));

	Vector vecCameraVector = GameServer()->GetCamera()->GetCameraPosition();

	Vector vecRay = (vecWorld - vecCameraVector).Normalized();

	return GameServer()->GetGame()->TraceLine(vecCameraVector, vecCameraVector+vecRay*1000, vecPoint, pHit);
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
	c.add<bool>("windowed", !m_bFullscreen);
	c.add<int>("width", m_iCfgWidth);
	c.add<int>("height", m_iCfgHeight);
	std::ofstream o;
	o.open("options.cfg", std::ios_base::out);
	o << c;
}

CInstructor* CDigitanksWindow::GetInstructor()
{
	if (!m_pInstructor)
		m_pInstructor = new CInstructor();

	return m_pInstructor;
}

bool CDigitanksWindow::HasCommandLineSwitch(const char* pszSwitch)
{
	for (size_t i = 0; i < m_apszCommandLine.size(); i++)
	{
		if (strcmp(m_apszCommandLine[i], pszSwitch) == 0)
			return true;
	}

	return false;
}

const char* CDigitanksWindow::GetCommandLineSwitchValue(const char* pszSwitch)
{
	// -1 to prevent buffer overrun
	for (size_t i = 0; i < m_apszCommandLine.size()-1; i++)
	{
		if (strcmp(m_apszCommandLine[i], pszSwitch) == 0)
			return m_apszCommandLine[i+1];
	}

	return NULL;
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
