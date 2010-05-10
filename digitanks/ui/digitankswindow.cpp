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

CDigitanksWindow* CDigitanksWindow::s_pDigitanksWindow = NULL;

#define CAMERA_DISTANCE 80

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

	m_pDigitanksGame = new CDigitanksGame();

	InitUI();

	m_pDigitanksGame->SetupDefaultGame();

	GLenum err = glewInit();
	if (GLEW_OK != err)
		exit(0);

	CompileShaders();

	glutPassiveMotionFunc(&CDigitanksWindow::MouseMotionCallback);
	glutMotionFunc(&CDigitanksWindow::MouseDraggedCallback);
	glutMouseFunc(&CDigitanksWindow::MouseInputCallback);
	glutReshapeFunc(&CDigitanksWindow::WindowResizeCallback);
	glutDisplayFunc(&CDigitanksWindow::DisplayCallback);
	glutVisibilityFunc(&CDigitanksWindow::VisibleCallback);
	glutKeyboardFunc(&CDigitanksWindow::KeyPressCallback);
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
	delete m_pDigitanksGame;
}

void CDigitanksWindow::CompileShaders()
{
}

void CDigitanksWindow::Run()
{
	while (true)
	{
		glutMainLoopEvent();
		Game()->Think((float)(glutGet(GLUT_ELAPSED_TIME))/1000.0f);
		Render();
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

	Vector vecSceneCenter = Vector(0,0,0);

	Vector vecCameraVector = AngleVector(EAngle(45, 0, 0)) * CAMERA_DISTANCE + vecSceneCenter;

	gluLookAt(vecCameraVector.x, vecCameraVector.y, vecCameraVector.z,
		vecSceneCenter.x, vecSceneCenter.y, vecSceneCenter.z,
		0.0, 1.0, 0.0);

	glGetDoublev( GL_MODELVIEW_MATRIX, m_aiModelView );
	glGetDoublev( GL_PROJECTION_MATRIX, m_aiProjection );
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );

	RenderGround();

	RenderObjects();

	glPopMatrix();
	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void CDigitanksWindow::RenderGround(void)
{
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();

	glTranslatef(0, 0, 0);

	int i;

	for (i = 0; i < 20; i++)
	{
		Vector vecStartX(-100, 0, -100);
		Vector vecEndX(-100, 0, 100);
		Vector vecStartZ(-100, 0, -100);
		Vector vecEndZ(100, 0, -100);

		for (int j = 0; j <= 20; j++)
		{
			GLfloat aflBorderLineBright[3] = { 0.5f, 0.6f, 0.5f };
			GLfloat aflBorderLineDarker[3] = { 0.4f, 0.5f, 0.4f };
			GLfloat aflInsideLineBright[3] = { 0.3f, 0.3f, 0.3f };
			GLfloat aflInsideLineDarker[3] = { 0.2f, 0.2f, 0.2f };

			glBegin(GL_LINES);

				if (j == 0 || j == 20 || j == 10)
					glColor3fv(aflBorderLineBright);
				else
					glColor3fv(aflInsideLineBright);

				glVertex3fv(vecStartX);

				if (j == 0 || j == 20 || j == 10)
					glColor3fv(aflBorderLineDarker);
				else
					glColor3fv(aflInsideLineDarker);

				glVertex3fv(vecEndX);

			glEnd();

			glBegin(GL_LINES);

				if (j == 0 || j == 20 || j == 10)
					glColor3fv(aflBorderLineBright);
				else
					glColor3fv(aflInsideLineBright);

				glVertex3fv(vecStartZ);

				if (j == 0 || j == 20 || j == 10)
					glColor3fv(aflBorderLineDarker);
				else
					glColor3fv(aflInsideLineDarker);

				glVertex3fv(vecEndZ);

			glEnd();

			vecStartX.x += 10;
			vecEndX.x += 10;
			vecStartZ.z += 10;
			vecEndZ.z += 10;
		}
	}

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

void CDigitanksWindow::RenderTank(class CDigitank* pTank, Vector vecOrigin, EAngle angDirection, Color clrTank)
{
	glPushAttrib(GL_ENABLE_BIT);

	glPushMatrix();

	if (clrTank.a() < 255)
	{
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glTranslatef(vecOrigin.x, vecOrigin.y, vecOrigin.z);
	glScalef(0.5f, 0.5f, 0.5f);

	// Turret
	glPushMatrix();
	glColor4ubv(Color(100, 100, 100, clrTank.a()));

	if ((pTank == DigitanksGame()->GetCurrentTank() && GetControlMode() == MODE_AIM) || pTank->HasDesiredAim())
	{
		Vector vecAimTarget;
		if (pTank == DigitanksGame()->GetCurrentTank() && GetControlMode() == MODE_AIM)
			vecAimTarget = pTank->GetPreviewAim();
		else
			vecAimTarget = pTank->GetDesiredAim();
		Vector vecTarget = (vecAimTarget - vecOrigin).Normalized();
		float flAngle = atan2(vecTarget.z, vecTarget.x) * 180/M_PI;
		glRotatef(-flAngle, 0.0f, 1.0f, 0.0f);
	}
	else
		glRotatef(-angDirection.y, 0.0f, 1.0f, 0.0f);

	float flScale = RemapVal(pTank->GetAttackPower(true), 0, 10, 1, 2);
	glScalef(flScale, flScale, flScale);

	glPushMatrix();
	glTranslatef(0, 2, 0);
	glScalef(1, 0.5f, 1);
	glutSolidCube(2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(2, 2, 0);
	glScalef(2, 1, 1);
	glutSolidCube(1);
	glPopMatrix();
	glPopMatrix();

	glRotatef(-angDirection.y, 0.0f, 1.0f, 0.0f);

	glColor4ubv(clrTank);

	glPushMatrix();
	glScalef(1, 0.5f, 1);

	glutSolidCube(4);
	glPopMatrix();

	// Tracks
	glPushMatrix();
	glTranslatef(0, -0.5f, 2);
	glColor4ubv(Color(100, 100, 100, clrTank.a()));
	glScalef(2, 0.5f, 0.5f);
	glutSolidCube(2.1f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -0.5f, -2);
	glColor4ubv(Color(100, 100, 100, clrTank.a()));
	glScalef(2, 0.5f, 0.5f);
	glutSolidCube(2.1f);
	glPopMatrix();

	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int iShieldStrength = (int)(pTank->GetFrontShieldStrength() * clrTank.a());
	if (iShieldStrength > 255)
		iShieldStrength = 255;

	glPushMatrix();
	glColor4ubv(Color(255, 255, 255, iShieldStrength));
	glTranslatef(4, 0, 0);
	glScalef(0.1f, 0.5f, 1);
	glutSolidCube(4);
	glPopMatrix();

	iShieldStrength = (int)(pTank->GetLeftShieldStrength() * clrTank.a());
	if (iShieldStrength > 255)
		iShieldStrength = 255;

	glPushMatrix();
	glColor4ubv(Color(255, 255, 255, iShieldStrength));
	glTranslatef(0, 0, -4);
	glScalef(1, 0.5f, 0.1f);
	glutSolidCube(4);
	glPopMatrix();

	iShieldStrength = (int)(pTank->GetRightShieldStrength() * clrTank.a());
	if (iShieldStrength > 255)
		iShieldStrength = 255;

	glPushMatrix();
	glColor4ubv(Color(255, 255, 255, iShieldStrength));
	glTranslatef(0, 0, 4);
	glScalef(1, 0.5f, 0.1f);
	glutSolidCube(4);
	glPopMatrix();

	iShieldStrength = (int)(pTank->GetRearShieldStrength() * clrTank.a());
	if (iShieldStrength > 255)
		iShieldStrength = 255;

	glPushMatrix();
	glColor4ubv(Color(255, 255, 255, iShieldStrength));
	glTranslatef(-4, 0, 0);
	glScalef(0.1f, 0.5f, 1);
	glutSolidCube(4);
	glPopMatrix();

	glPopMatrix();

	glPopAttrib();
}

void CDigitanksWindow::RenderMovementSelection()
{
	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	if (!pCurrentTank)
		return;

	// Movement
	if (GetControlMode() == MODE_MOVE)
		DebugCircle(pCurrentTank->GetOrigin(), pCurrentTank->GetTotalMovementPower(), Color(255, 255, 0));

	Vector vecOrigin = pCurrentTank->GetDesiredMove();

	if (GetControlMode() == MODE_TURN)
	{
		float flMaxTurnWithLeftoverPower = (pCurrentTank->GetTotalMovementPower() - pCurrentTank->GetMovementPower()) * pCurrentTank->TurnPerPower();
		RenderTurnIndicator(pCurrentTank->GetDesiredMove(), pCurrentTank->GetAngles(), flMaxTurnWithLeftoverPower);
	}

	Vector vecRangeOrigin = vecOrigin;

	Vector vecPoint;
	bool bMouseOnGrid = GetMouseGridPosition(vecPoint);

	if (GetControlMode() == MODE_MOVE && bMouseOnGrid)
	{
		pCurrentTank->SetPreviewMove(vecPoint);

		if (pCurrentTank->GetPreviewMovePower() <= pCurrentTank->GetBasePower())
		{
			vecRangeOrigin = vecPoint;

			Color clrTeam = pCurrentTank->GetTeam()->GetColor();
			clrTeam.SetAlpha(50);

			RenderTank(pCurrentTank, vecPoint, pCurrentTank->GetAngles(), clrTeam);

			float flMaxTurnWithLeftoverPower = (pCurrentTank->GetTotalMovementPower() - pCurrentTank->GetMovementPower(true)) * pCurrentTank->TurnPerPower();
			if (flMaxTurnWithLeftoverPower < 180)
				RenderTurnIndicator(vecPoint, pCurrentTank->GetAngles(), flMaxTurnWithLeftoverPower, 0.3f);

			if (IsShiftDown())
			{
				Vector vecMove = pCurrentTank->GetPreviewMove() - pCurrentTank->GetOrigin();

				CTeam* pTeam = DigitanksGame()->GetCurrentTeam();
				for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
				{
					CDigitank* pTank = pTeam->GetTank(i);

					if (pTank == pCurrentTank)
						continue;

					Vector vecTankMove = vecMove;
					if (vecMove.Length() > pTank->GetTotalMovementPower())
						vecTankMove = vecMove.Normalized() * pTank->GetTotalMovementPower() * 0.95f;

					RenderTank(pTank, pTank->GetOrigin() + vecTankMove, pCurrentTank->GetAngles(), clrTeam);
				}
			}
		}

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

			Vector vecTankAim = vecPoint;
			if ((vecTankAim - pTank->GetDesiredMove()).Length() > pTank->GetMaxRange())
				vecTankAim = pTank->GetDesiredMove() + (vecTankAim - pTank->GetDesiredMove()).Normalized() * pTank->GetMaxRange() * 0.99f;

			Vector vecTankOrigin = pTank->GetDesiredMove();
			float flDistance = (vecTankAim - vecTankOrigin).Length();

			DebugCircle(vecTankAim, RemapValClamped(flDistance, pTank->GetMinRange(), pTank->GetMaxRange(), 2, TANK_MAX_RANGE_RADIUS), Color(255, 0, 0));

			float flGravity = -flDistance*2;

			Vector vecForce = vecTankAim - vecTankOrigin;
			vecForce.y = -flGravity * 0.45f;	// Not quite sure how this works, but it does

			Vector vecProjectile = vecTankOrigin;
			for (size_t i = 0; i < 10; i++)
			{
				DebugLine(vecProjectile, vecProjectile + vecForce/10, Color(255, 0, 0));
				vecProjectile += vecForce/10;
				vecForce.y += flGravity/10;
			}
		}
	}
	else
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

				float flGravity = -flDistance*2;

				Vector vecForce = vecDesiredAim - vecTankOrigin;
				vecForce.y = -flGravity * 0.45f;	// Not quite sure how this works, but it does

				Vector vecProjectile = vecTankOrigin;
				for (size_t i = 0; i < 10; i++)
				{
					DebugLine(vecProjectile, vecProjectile + vecForce/10, Color(255, 0, 0, 150));
					vecProjectile += vecForce/10;
					vecForce.y += flGravity/10;
				}
			}
		}
	}

	// Range
	DebugCircle(vecRangeOrigin, pCurrentTank->GetMinRange(), Color(0, 255, 0, (GetControlMode() == MODE_AIM)?255:150));
	DebugCircle(vecRangeOrigin, pCurrentTank->GetMaxRange(), Color(255, 0, 0, (GetControlMode() == MODE_AIM)?255:150));

	glPopAttrib();
}

void CDigitanksWindow::RenderTurnIndicator(Vector vecOrigin, EAngle angAngle, float flDegrees, float flAlpha)
{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (flDegrees < 180)
	{
		Vector vecTurnLeft, vecTurnRight;

		AngleVectors(angAngle + EAngle(0, flDegrees, 0), &vecTurnLeft, NULL, NULL);
		AngleVectors(angAngle - EAngle(0, flDegrees, 0), &vecTurnRight, NULL, NULL);

		DebugLine(vecOrigin, vecOrigin + vecTurnLeft*10, Color(255, 255, 0, (int)(255.0f*flAlpha)));
		DebugLine(vecOrigin, vecOrigin + vecTurnRight*10, Color(255, 255, 0, (int)(255.0f*flAlpha)));

		DebugArc(vecOrigin, 10, angAngle.y - flDegrees, angAngle.y + flDegrees, Color(255, 255, 0, (int)(255.0f*flAlpha)));
	}
	else
	{
		Vector vecForward;
		AngleVectors(angAngle, &vecForward, NULL, NULL);

		DebugLine(vecOrigin, vecOrigin + vecForward*10, Color(255, 255, 0, (int)(255.0f*flAlpha)));
		DebugArc(vecOrigin, 10, angAngle.y - 30, angAngle.y + 30, Color(255, 255, 0, (int)(255.0f*flAlpha)));
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

	Vector vecCameraVector = AngleVector(EAngle(45, 0, 0)) * CAMERA_DISTANCE;

	Vector vecRay = (vecWorld - vecCameraVector).Normalized();

	float a = -vecCameraVector.y;
	float b = vecRay.y;

	float ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Ray is parallel
			return false;	// Ray is inside plane
		else
			return false;	// Ray is somewhere else
	}

	float r = a/b;
	if (r < 0)
		return false;		// Ray goes away from the triangle

	vecPoint = vecCameraVector + vecRay*r;

	return true;
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

void CDigitanksWindow::SetControlMode(controlmode_t eMode, bool bAutoProceed)
{
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
	}

	if (eMode == MODE_TURN)
		DigitanksGame()->GetCurrentTank()->CancelDesiredTurn();

	if (eMode == MODE_AIM)
		DigitanksGame()->GetCurrentTank()->CancelDesiredAim();

	m_bAutoProceed = bAutoProceed;
	m_eControlMode = eMode;
}
