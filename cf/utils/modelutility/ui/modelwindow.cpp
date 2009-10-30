#include "modelwindow.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>
#include <IL/il.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

CModelWindow* CModelWindow::s_pModelWindow = NULL;

CModelWindow::CModelWindow()
{
	s_pModelWindow = this;

	int argc = 1;
	char* argv = "modelwindow";

	m_aiObjects.clear();
	m_iObjectsCreated = 0;

	m_flCameraDistance = 100;

	m_bCameraRotating = false;
	m_bCameraDollying = false;
	m_bLightRotating = false;

	m_iMouseStartX = 0;
	m_iMouseStartY = 0;

	m_flCameraYaw = 45;
	m_flCameraPitch = 45;

	m_flLightYaw = 100;
	m_flLightPitch = 45;

	int iScreenWidth = glutGet(GLUT_SCREEN_WIDTH);
	int iScreenHeight = glutGet(GLUT_SCREEN_HEIGHT);

	glutInit(&argc, &argv);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);

	glutInitWindowPosition(iScreenWidth/6, iScreenHeight/6);
	glutInitWindowSize(iScreenWidth*2/3, iScreenHeight*2/3);

	glutCreateWindow("Model Utility");

	glutDisplayFunc(&CModelWindow::RenderCallback);
	glutIdleFunc(&CModelWindow::IdleCallback);
	glutMotionFunc(&CModelWindow::MouseMotionCallback);
	glutMouseFunc(&CModelWindow::MouseInputCallback);
	glutReshapeFunc(&CModelWindow::WindowResizeCallback);
	glutVisibilityFunc(&CModelWindow::VisibleCallback);
	glutKeyboardFunc(&CModelWindow::KeyPressCallback);
	glutSpecialFunc(&CModelWindow::SpecialCallback);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glLineWidth(1.0);

	WindowResize(iScreenWidth*2/3, iScreenHeight*2/3);

	GLfloat flLightDiffuse[] = {0.9, 1.0, 0.9, 1.0};
	GLfloat flLightAmbient[] = {0.2, 0.2, 0.2, 1.0};
	GLfloat flLightSpecular[] = {1.0, 1.0, 1.0, 1.0};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, flLightDiffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, flLightAmbient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, flLightSpecular);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05);

	ilInit();
}

void CModelWindow::Run()
{
	glutMainLoop();
}

void CModelWindow::LoadFromScene(CConversionScene* pScene)
{
	float flFarthest = 0;
	size_t i;

	for (i = 0; i < pScene->GetNumMaterials(); i++)
	{
		CConversionMaterial* pMaterial = pScene->GetMaterial(i);

		ILuint iDevILId;
		ilGenImages(1, &iDevILId);
		ilBindImage(iDevILId);

		m_aiMaterials.push_back(0);

		assert(m_aiMaterials.size()-1 == i);

		const char* pszTexture = pMaterial->GetTexture();

		if (!pszTexture || !*pszTexture)
			continue;

		wchar_t szTexture[1024];
		mbstowcs(szTexture, pszTexture, 1024);

		ILboolean bSuccess = ilLoadImage(szTexture);

		if (!bSuccess)
			bSuccess = ilLoadImage(szTexture);

		ILenum iError = ilGetError();

		if (bSuccess)
		{
			bSuccess = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
			if (bSuccess)
			{
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

				m_aiMaterials[i] = iGLId;
			}
		}
	}

	for (i = 0; i < pScene->GetNumMeshes(); i++)
	{
		CConversionMesh* pMesh = pScene->GetMesh(i);

		GLuint iObject = (GLuint)GetNextObjectId();
		m_aiObjects.push_back((size_t)iObject);

		glNewList(iObject, GL_COMPILE);

		for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
		{
			size_t k;
			CConversionFace* pFace = pMesh->GetFace(j);

			if (pFace->m == ~0)
				glBindTexture(GL_TEXTURE_2D, 0);
			else
				glBindTexture(GL_TEXTURE_2D, (GLuint)m_aiMaterials[pFace->m]);

			glBegin(GL_POLYGON);

			for (k = 0; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(k);

				Vector vecVertex = pMesh->GetVertex(pVertex->v);
				Vector vecNormal = pMesh->GetNormal(pVertex->vn);
				Vector vecUV = pMesh->GetUV(pVertex->vt);

				// Why? I dunno.
				vecUV.y = -vecUV.y;

				if (pFace->m != ~0 && pScene->GetMaterial(pFace->m))
				{
					CConversionMaterial* pMaterial = pScene->GetMaterial(pFace->m);
					glMaterialfv(GL_FRONT, GL_AMBIENT, pMaterial->m_vecAmbient);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, pMaterial->m_vecDiffuse);
					glMaterialfv(GL_FRONT, GL_SPECULAR, pMaterial->m_vecSpecular);
					glMaterialfv(GL_FRONT, GL_EMISSION, pMaterial->m_vecEmissive);
					glMaterialf(GL_FRONT, GL_SHININESS, pMaterial->m_flShininess);
				}

				glTexCoord2fv(vecUV);
				glNormal3fv(vecNormal);
				glVertex3fv(vecVertex);

				if (vecVertex.LengthSqr() > flFarthest)
					flFarthest = vecVertex.LengthSqr();
			}

			glEnd();

#if 0
			for (k = 0; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(k);

				glBindTexture(GL_TEXTURE_2D, (GLuint)0);
				glBegin(GL_LINES);

				glColor3f(0.8f, 0.8f, 0.8f);

				Vector vecVertex = pMesh->GetVertex(pVertex->v);
				Vector vecNormal = pMesh->GetNormal(pVertex->vn);

				glVertex3fv(vecVertex);
				glVertex3fv(vecVertex + vecNormal);

				glEnd();
			}
#endif
		}

		glEndList();
	}

	m_flCameraDistance = sqrt(flFarthest);
	if (m_flCameraDistance < 100)
		m_flCameraDistance = 100;

	gluLookAt(0.0, 0.0, m_flCameraDistance,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0);
}

void CModelWindow::Render()
{
	glClear(GL_DEPTH_BUFFER_BIT);

	// First draw a nice faded gray background.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
		glColor3f(0.6,0.6,0.6);
		glVertex2f(-1.0, 1.0);
		glColor3f(0.4,0.4,0.4);
		glVertex2f(-1.0,-1.0);
		glColor3f(0.2,0.2,0.2);
		glVertex2f(1.0,-1.0);
		glColor3f(0.4,0.4,0.4);
		glVertex2f(1.0, 1.0);
	glEnd();

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPushMatrix();
 
	gluLookAt(0.0, 0.0, m_flCameraDistance,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0);

    glRotatef(m_flCameraYaw, 1.0, 0.0, 0.0);
    glRotatef(m_flCameraPitch, 0.0, 1.0, 0.0);

	RenderLightSource();

    RenderGround();

	RenderObjects();

	glPopMatrix();

	glutSwapBuffers();
}

void CModelWindow::RenderGround(void)
{
	glDisable(GL_LIGHTING);

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
}

void CModelWindow::RenderLightSource()
{
	GLfloat flLightPosition[4];
	GLfloat lightColor[] = {0.9, 1.0, 0.9, 1.0};

	// Reposition the light source.
	flLightPosition[0] = cos(m_flLightPitch * (M_PI*2 / 360)) * cos(m_flLightYaw * (M_PI*2 / 360)) * m_flCameraDistance/4;
	flLightPosition[1] = sin(m_flLightPitch * (M_PI*2 / 360)) * m_flCameraDistance/4;
	flLightPosition[2] = cos(m_flLightPitch * (M_PI*2 / 360)) * sin(m_flLightYaw * (M_PI*2 / 360)) * m_flCameraDistance/4;
	flLightPosition[3] = 0.0;

	// Tell GL new light source position.
    glLightfv(GL_LIGHT0, GL_POSITION, flLightPosition);

	float flScale = m_flCameraDistance/60;

	glPushMatrix();
		glDisable(GL_LIGHTING);
		glColor3f(1.0, 1.0, 0.0);

		// Draw an arrowhead.
		glDisable(GL_CULL_FACE);
		glTranslatef(flLightPosition[0], flLightPosition[1], flLightPosition[2]);
		glRotatef(-m_flLightYaw, 0, 1, 0);
		glRotatef(m_flLightPitch, 0, 0, 1);
		glScalef(flScale, flScale, flScale);
		glBegin(GL_TRIANGLE_FAN);
			glVertex3f(0, 0, 0);
			glVertex3f(2, 1, 1);
			glVertex3f(2, -1, 1);
			glVertex3f(2, -1, -1);
			glVertex3f(2, 1, -1);
			glVertex3f(2, 1, 1);
		glEnd();

		// Draw a white line from light direction.
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			glVertex3f(5, 0, 0);
		glEnd();
	glPopMatrix();
}

void CModelWindow::RenderObjects()
{
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);

	// It uses this color if the texture is missing.
	GLfloat flMaterialColor[] = {0.7, 0.7, 0.7, 1.0};

	for (size_t i = 0; i < m_aiObjects.size(); i++)
	{
		glPushMatrix();
		glMaterialfv(GL_FRONT, GL_DIFFUSE, flMaterialColor);
		glColor4fv(flMaterialColor);
		glCallList((GLuint)m_aiObjects[i]);
		glPopMatrix();
	}
}

void CModelWindow::WindowResize(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
			44.0,						// FOV
			(float)w/(float)h,			// Aspect ratio
			1.0,						// Z near
			10000.0						// Z far
		);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void CModelWindow::Idle(void)
{
	glutPostRedisplay();
}

void CModelWindow::MouseMotion(int x, int y)
{
	if (m_bCameraRotating)
	{
		m_flCameraPitch += (x - m_iMouseStartX)/2;
		m_flCameraYaw += (y - m_iMouseStartY)/2;
		m_iMouseStartX = x;
		m_iMouseStartY = y;
		glutPostRedisplay();
	}

	if (m_bCameraDollying)
	{
		m_flCameraDistance += y - m_iMouseStartY;

		if (m_flCameraDistance < 1)
			m_flCameraDistance = 1;

		m_iMouseStartY = y;
		glutPostRedisplay();
	}

	if (m_bLightRotating)
	{
		m_flLightYaw += (m_iMouseStartX - x);
		m_flLightPitch += (m_iMouseStartY - y);

		while (m_flLightYaw >= 180)
			m_flLightYaw -= 360;
		while (m_flLightYaw < -180)
			m_flLightYaw += 360;
		while (m_flLightPitch > 89)
			m_flLightPitch = 89;
		while (m_flLightPitch < -89)
			m_flLightPitch = -89;

		m_iMouseStartX = x;
		m_iMouseStartY = y;
		glutPostRedisplay();
	}
}

void CModelWindow::Visible(int vis)
{
}

void CModelWindow::MouseInput(int iButton, int iState, int x, int y)
{
	if ((glutGetModifiers() & GLUT_ACTIVE_CTRL) && iButton == GLUT_LEFT_BUTTON)
	{
		if (iState == GLUT_DOWN)
		{
			m_bLightRotating = 1;
			m_iMouseStartX = x;
			m_iMouseStartY = y;
		}
		if (iState == GLUT_UP)
			m_bLightRotating = 0;
	}
	else if (iButton == GLUT_LEFT_BUTTON)
	{
		if (iState == GLUT_DOWN)
		{
			m_bCameraRotating = 1;
			m_iMouseStartX = x;
			m_iMouseStartY = y;
		}
		if (iState == GLUT_UP)
			m_bCameraRotating = 0;
	}
	else if (iButton == GLUT_RIGHT_BUTTON)
	{
		if (iState == GLUT_DOWN)
		{
			m_bCameraDollying = 1;
			m_iMouseStartX = x;
			m_iMouseStartY = y;
		}
		if (iState == GLUT_UP)
			m_bCameraDollying = 0;
	}
}

void CModelWindow::KeyPress(unsigned char c, int x, int y)
{
	if (c == 27)
		exit(0);

	glutPostRedisplay();
}

void CModelWindow::Special(int k, int x, int y)
{
	if (k == GLUT_KEY_F4 && (glutGetModifiers()&GLUT_ACTIVE_ALT))
		exit(0);

	glutPostRedisplay();
}

size_t CModelWindow::GetNextObjectId()
{
	return (m_iObjectsCreated++)+1;
}
