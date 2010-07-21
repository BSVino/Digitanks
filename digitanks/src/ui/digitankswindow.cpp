#include "digitankswindow.h"

#include <assert.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <time.h>
#include <vector.h>

#include <network/network.h>
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

	srand((unsigned int)time(NULL));

	for (int i = 0; i < argc; i++)
		m_apszCommandLine.push_back(argv[i]);

	m_iMouseStartX = 0;
	m_iMouseStartY = 0;
	m_bCameraMouseDown = false;

	m_bCtrl = m_bAlt = m_bShift = false;

	glutInit(&argc, argv);

	int iScreenWidth = glutGet(GLUT_SCREEN_WIDTH);
	int iScreenHeight = glutGet(GLUT_SCREEN_HEIGHT);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA | GLUT_MULTISAMPLE);

	m_iWindowWidth = iScreenWidth*2/3;
	m_iWindowHeight = iScreenHeight*2/3;

	if (m_iWindowWidth < 1024)
		m_iWindowWidth = 1024;

	if (m_iWindowHeight < 768)
		m_iWindowHeight = 768;

	glutInitWindowPosition((int)(iScreenWidth/2-m_iWindowWidth/2), (int)(iScreenHeight/2-m_iWindowHeight/2));
	glutInitWindowSize((int)m_iWindowWidth, (int)m_iWindowHeight);

	glutCreateWindow("Digitanks!");

	ilInit();

	GLenum err = glewInit();
	if (GLEW_OK != err)
		exit(0);

	CShaderLibrary::CompileShaders();

	InitUI();

	m_pDigitanksGame = NULL;
	m_pHUD = NULL;
	m_pInstructor = NULL;

	glutPassiveMotionFunc(&CDigitanksWindow::MouseMotionCallback);
	glutMotionFunc(&CDigitanksWindow::MouseDraggedCallback);
	glutMouseFunc(&CDigitanksWindow::MouseInputCallback);
	glutReshapeFunc(&CDigitanksWindow::WindowResizeCallback);
	glutDisplayFunc(&CDigitanksWindow::DisplayCallback);
	glutVisibilityFunc(&CDigitanksWindow::VisibleCallback);
	glutKeyboardFunc(&CDigitanksWindow::KeyPressCallback);
	glutKeyboardUpFunc(&CDigitanksWindow::KeyReleaseCallback);
	glutSpecialFunc(&CDigitanksWindow::SpecialCallback);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glLineWidth(1.0);

	WindowResize(iScreenWidth*2/3, iScreenHeight*2/3);

	CNetwork::Initialize();
}

CDigitanksWindow::~CDigitanksWindow()
{
	CNetwork::Deinitialize();

	delete m_pMenu;

	DestroyGame();
}

void CDigitanksWindow::CreateGame(gametype_t eGameType)
{
	if (!m_pDigitanksGame)
	{
		m_pDigitanksGame = new CDigitanksGame();
		m_pDigitanksGame->CreateRenderer();

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
	while (true)
	{
		glutMainLoopEvent();
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
				Game()->Think((float)(glutGet(GLUT_ELAPSED_TIME))/1000.0f);
				Render();
			}
		}
		else
			// Clear the buffer for the gui.
			glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

		glgui::CRootPanel::Get()->Think();
		glgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

		glutSwapBuffers();
	}
}

void CDigitanksWindow::Render()
{
	Game()->Render();
}

void CDigitanksWindow::WindowResize(int w, int h)
{
	if (GetGame())
		GetGame()->GetRenderer()->SetSize(w, h);

	m_iWindowWidth = w;
	m_iWindowHeight = h;

	if (GetGame())
		Render();

	glgui::CRootPanel::Get()->Layout();
	glgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

	glutSwapBuffers();
}

void CDigitanksWindow::Display()
{
}

void CDigitanksWindow::Visible(int vis)
{
}

bool CDigitanksWindow::GetMouseGridPosition(Vector& vecPoint)
{
	if (!DigitanksGame()->GetTerrain())
		return false;

	int x, y;
	glgui::CRootPanel::Get()->GetFullscreenMousePos(x, y);

	Vector vecWorld = GetGame()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));

	Vector vecCameraVector = GetGame()->GetCamera()->GetCameraPosition();

	Vector vecRay = (vecWorld - vecCameraVector).Normalized();

	return DigitanksGame()->GetTerrain()->Collide(Ray(vecCameraVector, vecRay), vecPoint);
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
