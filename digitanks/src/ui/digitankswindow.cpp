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
#include "camera.h"
#include "shaders/shaders.h"
#include "menu.h"

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
	m_pCamera = NULL;
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
	if (m_pCamera)
		delete m_pCamera;
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

	m_pCamera = new CCamera();
	m_pCamera->SetDistance(120);
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
	m_pCamera->Think();

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);
	glViewport(0, 0, (GLsizei)m_iWindowWidth, (GLsizei)m_iWindowHeight);

	glClear(GL_DEPTH_BUFFER_BIT);

	// First draw a nice faded gray background.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	glShadeModel(GL_SMOOTH);

	glBegin(GL_QUADS);
		glColor3ub(20, 20, 20);
		glVertex2f(-1.0f, 1.0f);
		glColor3ub(10, 10, 10);
		glVertex2f(-1.0f, -1.0f);
		glColor3ub(20, 20, 20);
		glVertex2f(1.0f, -1.0f);
		glColor3ub(10, 10, 10);
		glVertex2f(1.0f, 1.0f);
	glEnd();

	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(
			44.0,
			(float)m_iWindowWidth/(float)m_iWindowHeight,
			1,
			10000.0
		);

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();

	Vector vecSceneCenter = m_pCamera->GetCameraTarget();
	Vector vecCameraVector = m_pCamera->GetCameraPosition();

	gluLookAt(vecCameraVector.x, vecCameraVector.y, vecCameraVector.z,
		vecSceneCenter.x, vecSceneCenter.y, vecSceneCenter.z,
		0.0, 1.0, 0.0);

	glGetDoublev( GL_MODELVIEW_MATRIX, m_aiModelView );
	glGetDoublev( GL_PROJECTION_MATRIX, m_aiProjection );
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );

	RenderObjects();

	glPopMatrix();
	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void CDigitanksWindow::RenderObjects()
{
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	RenderGame(m_pDigitanksGame);

	glPopAttrib();
}

void CDigitanksWindow::RenderGame(CDigitanksGame* pGame)
{
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i))->Render();

	RenderMovementSelection();
}

void CDigitanksWindow::RenderMovementSelection()
{
	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	if (!pCurrentTank)
		return;

	Vector vecOrigin = pCurrentTank->GetDesiredMove();

	Vector vecPoint;
	bool bMouseOnGrid = GetMouseGridPosition(vecPoint);

	if (GetControlMode() == MODE_MOVE && bMouseOnGrid)
	{
		pCurrentTank->SetPreviewMove(vecPoint);

		m_pHUD->UpdateAttackInfo();
	}

	if (GetControlMode() == MODE_TURN && bMouseOnGrid)
	{
		if ((vecPoint - vecOrigin).LengthSqr() > 3*3)
		{
			Vector vecTurn = vecPoint - pCurrentTank->GetDesiredMove();

			vecTurn.Normalize();

			float flTurn = atan2(vecTurn.z, vecTurn.x) * 180/M_PI;

			pCurrentTank->SetPreviewTurn(flTurn);
			m_pHUD->UpdateAttackInfo();
		}
		else
		{
			pCurrentTank->SetPreviewTurn(pCurrentTank->GetAngles().y);
			m_pHUD->UpdateAttackInfo();
		}
	}

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (GetControlMode() == MODE_AIM && bMouseOnGrid)
	{
		pCurrentTank->SetPreviewAim(vecPoint);

		m_pHUD->UpdateAttackInfo();

		CTeam* pTeam = DigitanksGame()->GetCurrentTeam();
		for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
		{
			CDigitank* pTank = pTeam->GetTank(i);

			if (pTank != pCurrentTank && !IsShiftDown())
				continue;

			int iAlpha = 255;
			if (pTank != pCurrentTank)
				iAlpha = 150;

			Vector vecTankAim = vecPoint;
			if ((vecTankAim - pTank->GetDesiredMove()).Length() > pTank->GetMaxRange())
			{
				vecTankAim = pTank->GetDesiredMove() + (vecTankAim - pTank->GetDesiredMove()).Normalized() * pTank->GetMaxRange() * 0.99f;
				vecTankAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecTankAim.x, vecTankAim.z);
			}

			Vector vecTankOrigin = pTank->GetDesiredMove();
			float flDistance = (vecTankAim - vecTankOrigin).Length();

			DebugCircle(vecTankAim, RemapValClamped(flDistance, pTank->GetMinRange(), pTank->GetMaxRange(), 2, TANK_MAX_RANGE_RADIUS), Color(255, 0, 0, iAlpha));

			float flGravity = DigitanksGame()->GetGravity();
			float flTime;
			Vector vecForce;
			FindLaunchVelocity(vecTankOrigin, vecTankAim, flGravity, vecForce, flTime);

			Vector vecProjectile = vecTankOrigin;
			for (size_t i = 0; i < 20; i++)
			{
				DebugLine(vecProjectile, vecProjectile + vecForce*flTime/20, Color(255, 0, 0, iAlpha));
				vecProjectile += vecForce*flTime/20;
				vecForce.y += flGravity*flTime/20;
			}
		}
	}

	if (!IsShiftDown())
	{
		for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
			if (!pEntity)
				continue;

			CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
			if (!pTank)
				continue;

			if (pTank->GetTeam() != DigitanksGame()->GetCurrentTeam())
				continue;

			if (!pTank->HasDesiredAim())
				continue;

			Vector vecTankOrigin = pTank->GetDesiredMove();
			Vector vecDesiredAim = pTank->GetDesiredAim();
			float flDistance = (vecDesiredAim - vecTankOrigin).Length();

			if (flDistance < pTank->GetMaxRange())
			{
				DebugCircle(vecDesiredAim, RemapValClamped(flDistance, pTank->GetMinRange(), pTank->GetMaxRange(), 2, TANK_MAX_RANGE_RADIUS), Color(255, 0, 0, 150));

				float flGravity = DigitanksGame()->GetGravity();
				float flTime;
				Vector vecForce;
				FindLaunchVelocity(vecTankOrigin, vecDesiredAim, flGravity, vecForce, flTime);

				Vector vecProjectile = vecTankOrigin;
				for (size_t i = 0; i < 20; i++)
				{
					DebugLine(vecProjectile, vecProjectile + vecForce*flTime/20, Color(255, 0, 0, 150));
					vecProjectile += vecForce*flTime/20;
					vecForce.y += flGravity*flTime/20;
				}
			}
		}
	}

	glPopAttrib();
}

void CDigitanksWindow::WindowResize(int w, int h)
{
	m_iWindowWidth = w;
	m_iWindowHeight = h;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
			44.0,						// FOV
			(float)w/(float)h,			// Aspect ratio
			1.0,						// Z near
			10000.0						// Z far
		);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

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

	Vector vecWorld = WorldPosition(Vector((float)x, (float)y, 1));

	Vector vecCameraVector = m_pCamera->GetCameraPosition();

	Vector vecRay = (vecWorld - vecCameraVector).Normalized();

	return DigitanksGame()->GetTerrain()->Collide(Ray(vecCameraVector, vecRay), vecPoint);
}

Vector CDigitanksWindow::ScreenPosition(Vector vecWorld)
{
	GLdouble x, y, z;
	gluProject(
		vecWorld.x, vecWorld.y, vecWorld.z,
		(GLdouble*)m_aiModelView, (GLdouble*)m_aiProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)GetWindowHeight() - (float)y, (float)z);
}

Vector CDigitanksWindow::WorldPosition(Vector vecScreen)
{
	GLdouble x, y, z;
	gluUnProject(
		vecScreen.x, (float)GetWindowHeight() - vecScreen.y, vecScreen.z,
		(GLdouble*)m_aiModelView, (GLdouble*)m_aiProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)y, (float)z);
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

		m_pCamera->SetDistance(100);
		m_pInstructor->DisplayTutorial(CInstructor::TUTORIAL_MOVE);
	}

	if (eMode == MODE_TURN)
	{
		DigitanksGame()->GetCurrentTank()->CancelDesiredTurn();

		m_pCamera->SetDistance(80);
//		m_pInstructor->DisplayTutorial(CInstructor::TUTORIAL_TURN);
	}

	if (eMode == MODE_AIM)
	{
		DigitanksGame()->GetCurrentTank()->CancelDesiredAim();

		m_pCamera->SetDistance(120);
		m_pInstructor->DisplayTutorial(CInstructor::TUTORIAL_AIM);
	}

	if (eMode == MODE_FIRE)
	{
		m_pCamera->SetDistance(80);
		m_pInstructor->DisplayTutorial(CInstructor::TUTORIAL_POWER);
	}

	if (eMode == MODE_NONE)
		m_pCamera->SetDistance(100);

	m_eControlMode = eMode;
}
