#include "digitankswindow.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <maths.h>
#include <vector.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>
#include <IL/ilu.h>

extern "C" {
static void CALLBACK RenderTesselateBegin(GLenum ePrim);
static void CALLBACK RenderTesselateVertex(void* pVertexData, void* pPolygonData);
static void CALLBACK RenderTesselateEnd();
}

CDigitanksWindow* CDigitanksWindow::s_pDigitanksWindow = NULL;

CDigitanksWindow::CDigitanksWindow()
{
	s_pDigitanksWindow = this;

	int argc = 1;
	char* argv = "digitanks";

	m_iMouseStartX = 0;
	m_iMouseStartY = 0;

	glutInit(&argc, &argv);

	int iScreenWidth = glutGet(GLUT_SCREEN_WIDTH);
	int iScreenHeight = glutGet(GLUT_SCREEN_HEIGHT);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA | GLUT_MULTISAMPLE);

	m_iWindowWidth = iScreenWidth*2/3;
	m_iWindowHeight = iScreenHeight*2/3;

	glutInitWindowPosition(iScreenWidth/6, iScreenHeight/6);
	glutInitWindowSize((int)m_iWindowWidth, (int)m_iWindowHeight);

	glutCreateWindow("Digitanks!");

	ilInit();

	InitUI();

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
	glLineWidth(1.0);

	WindowResize(iScreenWidth*2/3, iScreenHeight*2/3);

	GLfloat flLightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat flLightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat flLightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, flLightDiffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, flLightAmbient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, flLightSpecular);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1f);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05f);
}

CDigitanksWindow::~CDigitanksWindow()
{
}

void CDigitanksWindow::CompileShaders()
{
}

void CDigitanksWindow::Run()
{
	while (true)
	{
		glutMainLoopEvent();
		Render();
		//modelgui::CRootPanel::Get()->Think();
		//modelgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);
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

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	glShadeModel(GL_SMOOTH);

	glBegin(GL_QUADS);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex2f(-1.0f, 1.0f);
		glColor3f(0.4f, 0.4f, 0.4f);
		glVertex2f(-1.0f, -1.0f);
		glColor3f(0.2f, 0.2f, 0.2f);
		glVertex2f(1.0f, -1.0f);
		glColor3f(0.4f, 0.4f, 0.4f);
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

	Vector vecCameraVector = AngleVector(EAngle(45, 0, 0)) * 100 + vecSceneCenter;

	gluLookAt(vecCameraVector.x, vecCameraVector.y, vecCameraVector.z,
		vecSceneCenter.x, vecSceneCenter.y, vecSceneCenter.z,
		0.0, 1.0, 0.0);

	// Reposition the light source.
	Vector vecLightDirection = AngleVector(EAngle(45, 0, 0));

	Vector vecLightPosition = vecLightDirection * 100/2;

	GLfloat flLightPosition[4];
	flLightPosition[0] = vecLightPosition.x;
	flLightPosition[1] = vecLightPosition.y;
	flLightPosition[2] = vecLightPosition.z;
	flLightPosition[3] = 0;

	// Tell GL new light source position.
    glLightfv(GL_LIGHT0, GL_POSITION, flLightPosition);

	RenderGround();

	RenderObjects();

	glPopMatrix();
	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void CDigitanksWindow::RenderGround(void)
{
	glDisable(GL_LIGHTING);
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
			GLfloat aflBorderLineBright[3] = { 0.7f, 0.7f, 0.7f };
			GLfloat aflBorderLineDarker[3] = { 0.6f, 0.6f, 0.6f };
			GLfloat aflInsideLineBright[3] = { 0.5f, 0.5f, 0.5f };
			GLfloat aflInsideLineDarker[3] = { 0.4f, 0.4f, 0.4f };

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

	glEnable(GL_LIGHTING);

	glEnable(GL_CULL_FACE);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);

	// Render objects here!

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
//	modelgui::CRootPanel::Get()->Layout();
//	modelgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

	glutSwapBuffers();
}

void CDigitanksWindow::Display()
{
}

void CDigitanksWindow::Visible(int vis)
{
}
