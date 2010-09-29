#include "digitankswindow.h"

#include <assert.h>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <time.h>
#include <vector.h>

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
#include "renderer/renderer.h"

CDigitanksWindow* CDigitanksWindow::s_pDigitanksWindow = NULL;

CDigitanksWindow::CDigitanksWindow(int argc, char** argv)
{
	s_pDigitanksWindow = this;

	m_pDigitanksGame = NULL;
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

	int iScreenWidth = 1024;
	int iScreenHeight = 768;

	GetScreenSize(iScreenWidth, iScreenHeight);

	m_iWindowWidth = iScreenWidth*2/3;
	m_iWindowHeight = iScreenHeight*2/3;

	if (m_iWindowWidth < 1024)
		m_iWindowWidth = 1024;

	if (m_iWindowHeight < 768)
		m_iWindowHeight = 768;

#ifdef _DEBUG
	int iMode = GLFW_WINDOW;
#else
	int iMode = GLFW_FULLSCREEN;
#endif

	if (HasCommandLineSwitch("--fullscreen"))
		iMode = GLFW_FULLSCREEN;

	if (HasCommandLineSwitch("--windowed"))
		iMode = GLFW_WINDOW;

	glfwEnable( GLFW_MOUSE_CURSOR );

	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	if (!glfwOpenWindow(m_iWindowWidth, m_iWindowHeight, 0, 0, 0, 0, 16, 0, GLFW_WINDOW))
	{
		glfwTerminate();
		return;
	}

	glfwSetWindowTitle( "Digitanks!" );
	glfwSetWindowPos((int)(iScreenWidth/2-m_iWindowWidth/2), (int)(iScreenHeight/2-m_iWindowHeight/2));

	glfwSetWindowSizeCallback(&CDigitanksWindow::WindowResizeCallback);
	glfwSetKeyCallback(&CDigitanksWindow::KeyEventCallback);
	glfwSetMousePosCallback(&CDigitanksWindow::MouseMotionCallback);
	glfwSetMouseButtonCallback(&CDigitanksWindow::MouseInputCallback);
	glfwSetMouseWheelCallback(&CDigitanksWindow::MouseWheelCallback);
	glfwSwapInterval( 1 );
	glfwSetTime( 0.0 );

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

//	if (!HasCommandLineSwitch("--nomusic"))
//		CSoundLibrary::PlayMusic("sound/digitanks-march.ogg");
}

CDigitanksWindow::~CDigitanksWindow()
{
	CNetwork::Deinitialize();

	delete m_pMenu;

	DestroyGame();

	glfwTerminate();
}

void CDigitanksWindow::CreateGame(gametype_t eGameType)
{
	CSoundLibrary::StopMusic();

	if (!m_pDigitanksGame)
	{
		m_pDigitanksGame = new CDigitanksGame();
		m_pDigitanksGame->CreateRenderer();
		m_pDigitanksGame->CreateCamera();

		m_pHUD = new CHUD();
		m_pHUD->SetGame(m_pDigitanksGame);
		glgui::CRootPanel::Get()->AddControl(m_pHUD);

		if (!m_pInstructor)
			m_pInstructor = new CInstructor();
	}

	m_pDigitanksGame->RegisterNetworkFunctions();

	const char* pszPort = GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	if (HasCommandLineSwitch("--host"))
		CNetwork::CreateHost(iPort, m_pDigitanksGame, CGame::ClientConnectCallback, CGame::ClientDisconnectCallback);

	if (HasCommandLineSwitch("--connect"))
	{
		CNetwork::ConnectToHost(GetCommandLineSwitchValue("--connect"), iPort);
		if (!CNetwork::IsConnected())
		{
			DestroyGame();
			return;
		}
	}

	m_pDigitanksGame->SetupGame(eGameType);

	glgui::CRootPanel::Get()->Layout();
}

void CDigitanksWindow::DestroyGame()
{
	CNetwork::Disconnect();

	if (m_pDigitanksGame)
		delete m_pDigitanksGame;

	if (m_pHUD)
	{
		glgui::CRootPanel::Get()->RemoveControl(m_pHUD);
		delete m_pHUD;
	}

	if (m_pInstructor)
		delete m_pInstructor;

	m_pDigitanksGame = NULL;
	m_pHUD = NULL;
	m_pInstructor = NULL;
}

void CDigitanksWindow::Run()
{
	while (glfwGetWindowParam( GLFW_OPENED ))
	{
		float flTime = (float)glfwGetTime();
		if (m_pDonate->IsVisible())
		{
			// Clear the buffer for the gui.
			glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
		}
		else if (Game())
		{
			if (Game()->IsLoading())
				CNetwork::Think();
			else if (Game()->IsClient() && !CNetwork::IsConnected())
			{
				DestroyGame();
				m_pMenu->SetVisible(true);
			}
			else
			{
				Game()->Think(flTime);
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
	Game()->Render();
}

void CDigitanksWindow::WindowResize(int w, int h)
{
	if (GetGame() && GetGame()->GetRenderer())
		GetGame()->GetRenderer()->SetSize(w, h);

	m_iWindowWidth = w;
	m_iWindowHeight = h;

	if (GetGame())
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

	Vector vecWorld = GetGame()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));

	Vector vecCameraVector = GetGame()->GetCamera()->GetCameraPosition();

	Vector vecRay = (vecWorld - vecCameraVector).Normalized();

	return Game()->TraceLine(vecCameraVector, vecCameraVector+vecRay*1000, vecPoint, pHit);
}

void CDigitanksWindow::GameOver(bool bPlayerWon)
{
	DigitanksGame()->SetControlMode(MODE_NONE);
	GetInstructor()->SetActive(false);
	m_pVictory->GameOver(bPlayerWon);
}

void CDigitanksWindow::CloseApplication()
{
	if (m_pDonate->IsVisible())
		exit(0);

	DestroyGame();

	m_pMenu->SetVisible(false);
	m_pDonate->ClosingApplication();
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
