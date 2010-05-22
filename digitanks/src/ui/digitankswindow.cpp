#include "digitankswindow.h"

#include <assert.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <time.h>
#include <vector.h>

#include "glgui/glgui.h"
#include "game/digitanksgame.h"
#include "debugdraw.h"
#include "hud.h"
#include "instructor.h"
#include "game/camera.h"
#include "shaders/shaders.h"
#include "menu.h"
#include "renderer/renderer.h"

CDigitanksWindow* CDigitanksWindow::s_pDigitanksWindow = NULL;

CDigitanksWindow::CDigitanksWindow()
{
	s_pDigitanksWindow = this;

	srand((unsigned int)time(NULL));

	int argc = 1;
	char* argv = "digitanks";

	m_iMouseStartX = 0;
	m_iMouseStartY = 0;

	m_bCtrl = m_bAlt = m_bShift = false;

	glutInit(&argc, &argv);

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
}

CDigitanksWindow::~CDigitanksWindow()
{
	delete m_pMenu;

	if (m_pDigitanksGame)
		delete m_pDigitanksGame;
	if (m_pHUD)
		delete m_pHUD;
	if (m_pInstructor)
		delete m_pInstructor;
}

void CDigitanksWindow::CreateGame(int iPlayers, int iTanks)
{
	if (m_pDigitanksGame)
	{
		m_pDigitanksGame->SetupGame(iPlayers, iTanks);
		return;
	}

	m_pDigitanksGame = new CDigitanksGame();

	m_pHUD = new CHUD();
	m_pHUD->SetGame(m_pDigitanksGame);
	glgui::CRootPanel::Get()->AddControl(m_pHUD);

	m_pInstructor = new CInstructor();

	m_pDigitanksGame->SetupGame(iPlayers, iTanks);

	glgui::CRootPanel::Get()->Layout();
}

void CDigitanksWindow::Run()
{
	while (true)
	{
		glutMainLoopEvent();
		if (GetGame())
		{
			Game()->Think((float)(glutGet(GLUT_ELAPSED_TIME))/1000.0f);
			Render();
		}
		glgui::CRootPanel::Get()->Think();
		glgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);
		glutSwapBuffers();
	}
}

size_t CDigitanksWindow::LoadTextureIntoGL(std::wstring sFilename)
{
	if (!sFilename.length())
		return 0;

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilLoadImage(sFilename.c_str());

	if (!bSuccess)
		bSuccess = ilLoadImage(sFilename.c_str());

	ILenum iError = ilGetError();

	if (!bSuccess)
		return 0;

	bSuccess = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!bSuccess)
		return 0;

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);

	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		iluFlipImage();

	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D,
		ilGetInteger(IL_IMAGE_BPP),
		ilGetInteger(IL_IMAGE_WIDTH),
		ilGetInteger(IL_IMAGE_HEIGHT),
		ilGetInteger(IL_IMAGE_FORMAT),
		GL_UNSIGNED_BYTE,
		ilGetData());

	ilDeleteImages(1, &iDevILId);

	return iGLId;
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
	int x, y;
	glgui::CRootPanel::Get()->GetFullscreenMousePos(x, y);

	Vector vecWorld = GetGame()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1));

	Vector vecCameraVector = GetGame()->GetCamera()->GetCameraPosition();

	Vector vecRay = (vecWorld - vecCameraVector).Normalized();

	return DigitanksGame()->GetTerrain()->Collide(Ray(vecCameraVector, vecRay), vecPoint);
}

controlmode_t CDigitanksWindow::GetControlMode()
{
	if (DigitanksGame()->GetCurrentTeam() == DigitanksGame()->GetTeam(0))
		return m_eControlMode;

	return MODE_NONE;
}

void CDigitanksWindow::SetControlMode(controlmode_t eMode, bool bAutoProceed)
{
	if (!DigitanksGame()->GetCurrentTank())
		return;

	if (m_eControlMode == MODE_MOVE)
	{
		if (eMode == MODE_NONE)
			DigitanksGame()->GetCurrentTank()->CancelDesiredMove();
		else
			DigitanksGame()->GetCurrentTank()->ClearPreviewMove();
	}

	if (m_eControlMode == MODE_TURN)
		DigitanksGame()->GetCurrentTank()->ClearPreviewTurn();

	if (m_eControlMode == MODE_AIM)
		DigitanksGame()->GetCurrentTank()->ClearPreviewAim();

	if (eMode == MODE_MOVE)
	{
		DigitanksGame()->GetCurrentTank()->CancelDesiredMove();
		DigitanksGame()->GetCurrentTank()->CancelDesiredTurn();
		DigitanksGame()->GetCurrentTank()->CancelDesiredAim();

		GetGame()->GetCamera()->SetDistance(100);
		m_pInstructor->DisplayTutorial(CInstructor::TUTORIAL_MOVE);
	}

	if (eMode == MODE_TURN)
	{
		DigitanksGame()->GetCurrentTank()->CancelDesiredTurn();

		GetGame()->GetCamera()->SetDistance(80);
//		m_pInstructor->DisplayTutorial(CInstructor::TUTORIAL_TURN);
	}

	if (eMode == MODE_AIM)
	{
		DigitanksGame()->GetCurrentTank()->CancelDesiredAim();

		GetGame()->GetCamera()->SetDistance(120);
		m_pInstructor->DisplayTutorial(CInstructor::TUTORIAL_AIM);
	}

	if (eMode == MODE_FIRE)
	{
		GetGame()->GetCamera()->SetDistance(80);
		m_pInstructor->DisplayTutorial(CInstructor::TUTORIAL_POWER);
	}

	if (eMode == MODE_NONE)
		GetGame()->GetCamera()->SetDistance(100);

	m_eControlMode = eMode;
}
