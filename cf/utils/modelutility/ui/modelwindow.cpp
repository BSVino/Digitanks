#include "modelwindow.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>

#include <modelconverter/modelconverter.h>
#include "modelgui.h"

#ifndef M_PI
#define M_PI 3.14159265
#endif

CModelWindow* CModelWindow::s_pModelWindow = NULL;

CModelWindow::CModelWindow()
{
	s_pModelWindow = this;

	int argc = 1;
	char* argv = "smak";

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

	glutInit(&argc, &argv);

	int iScreenWidth = glutGet(GLUT_SCREEN_WIDTH);
	int iScreenHeight = glutGet(GLUT_SCREEN_HEIGHT);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);

	m_iWindowWidth = iScreenWidth*2/3;
	m_iWindowHeight = iScreenHeight*2/3;

	glutInitWindowPosition(iScreenWidth/6, iScreenHeight/6);
	glutInitWindowSize((int)m_iWindowWidth, (int)m_iWindowHeight);

	glutCreateWindow("SMAK - Super Model Army Knife");

	InitUI();

	GLenum err = glewInit();
	if (GLEW_OK != err)
		exit(0);

	glutPassiveMotionFunc(&CModelWindow::MouseMotionCallback);
	glutMotionFunc(&CModelWindow::MouseDraggedCallback);
	glutMouseFunc(&CModelWindow::MouseInputCallback);
	glutReshapeFunc(&CModelWindow::WindowResizeCallback);
	glutDisplayFunc(&CModelWindow::DisplayCallback);
	glutVisibilityFunc(&CModelWindow::VisibleCallback);
	glutKeyboardFunc(&CModelWindow::KeyPressCallback);
	glutSpecialFunc(&CModelWindow::SpecialCallback);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glLineWidth(1.0);

	WindowResize(iScreenWidth*2/3, iScreenHeight*2/3);

	GLfloat flLightDiffuse[] = {0.9f, 1.0f, 0.9f, 1.0f};
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

	ilInit();
}

void CModelWindow::Run()
{
	while (true)
	{
		glutMainLoopEvent();
		Render();
		modelgui::CRootPanel::Get()->Think();
		modelgui::CRootPanel::Get()->Paint();
		glutSwapBuffers();
	}
}

void CModelWindow::DestroyAll()
{
	m_Scene.DestroyAll();

	m_szFileLoaded[0] = '\0';

	m_aiObjects.clear();
	m_iObjectsCreated = 0;
	m_aoMaterials.clear();
}

void CModelWindow::ReadFile(const wchar_t* pszFile)
{
	if (!pszFile)
		return;

	// Save it in here in case m_szFileLoaded was passed into ReadFile, in which case it would be destroyed by DestroyAll.
	std::wstring sFile = pszFile;
	std::wstring sExtension;

	size_t iFileLength = wcslen(pszFile);
	sExtension = pszFile+iFileLength-4;

	DestroyAll();

	CModelConverter c(&m_Scene);

	if (wcscmp(sExtension.c_str(), L".obj") == 0)
		c.ReadOBJ(sFile.c_str());
	else if (wcscmp(sExtension.c_str(), L".sia") == 0)
		c.ReadSIA(sFile.c_str());
	else if (wcscmp(sExtension.c_str(), L".dae") == 0)
		c.ReadDAE(sFile.c_str());

	wcscpy(m_szFileLoaded, sFile.c_str());

	LoadIntoGL();
}

void CModelWindow::ReloadFromFile()
{
	ReadFile(m_szFileLoaded);
}

void CModelWindow::LoadIntoGL()
{
	LoadTexturesIntoGL();
	CreateGLLists();
}

void CModelWindow::LoadTexturesIntoGL()
{
	for (size_t i = 0; i < m_Scene.GetNumMaterials(); i++)
	{
		CConversionMaterial* pMaterial = m_Scene.GetMaterial(i);

		ILuint iDevILId;
		ilGenImages(1, &iDevILId);
		ilBindImage(iDevILId);

		m_aoMaterials.push_back(CMaterial(0));

		assert(m_aoMaterials.size()-1 == i);

		const wchar_t* pszTexture = pMaterial->GetTexture();

		if (!pszTexture || !*pszTexture)
			continue;

		ILboolean bSuccess = ilLoadImage(pszTexture);

		if (!bSuccess)
			bSuccess = ilLoadImage(pszTexture);

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

				m_aoMaterials[i].m_iBase = iGLId;
			}
		}
	}
}

void CModelWindow::CreateGLLists()
{
	float flFarthest = 0;

	for (size_t i = 0; i < m_Scene.GetNumMeshes(); i++)
	{
		CConversionMesh* pMesh = m_Scene.GetMesh(i);

		GLuint iObject;

		if (i < m_aiObjects.size())
		{
			iObject = (GLuint)m_aiObjects[i];
			glDeleteLists(iObject, 1);
		}
		else
		{
			iObject = (GLuint)GetNextObjectId();
			m_aiObjects.push_back((size_t)iObject);
		}

		glNewList(iObject, GL_COMPILE);

		bool bMultiTexture = false;

		for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
		{
			size_t k;
			CConversionFace* pFace = pMesh->GetFace(j);

			if (pFace->m == ~0)
				glBindTexture(GL_TEXTURE_2D, 0);
			else
			{
				CMaterial* pMaterial = &m_aoMaterials[pFace->m];

				if (GLEW_VERSION_1_3 && pMaterial->m_iAO)
				{
					bMultiTexture = true;

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iBase);
					glEnable(GL_TEXTURE_2D);

					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iAO);
					glEnable(GL_TEXTURE_2D);
				}
				else
				{
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iBase);
				}
			}

			glBegin(GL_POLYGON);

			for (k = 0; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(k);

				Vector vecVertex = pMesh->GetVertex(pVertex->v);
				Vector vecNormal = pMesh->GetNormal(pVertex->vn);
				Vector vecUV = pMesh->GetUV(pVertex->vt);

				// Why? I dunno.
				vecUV.y = -vecUV.y;

				if (pFace->m != ~0 && m_Scene.GetMaterial(pFace->m))
				{
					CConversionMaterial* pMaterial = m_Scene.GetMaterial(pFace->m);
					glMaterialfv(GL_FRONT, GL_AMBIENT, pMaterial->m_vecAmbient);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, pMaterial->m_vecDiffuse);
					glMaterialfv(GL_FRONT, GL_SPECULAR, pMaterial->m_vecSpecular);
					glMaterialfv(GL_FRONT, GL_EMISSION, pMaterial->m_vecEmissive);
					glMaterialf(GL_FRONT, GL_SHININESS, pMaterial->m_flShininess);
				}

				if (bMultiTexture)
				{
					glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
					glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
				}
				else
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

#if 0
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glBegin(GL_LINES);
				glColor3f(0.8f, 0.8f, 0.8f);
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(0)->v));
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(1)->v));

				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(1)->v));
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(2)->v));

				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(2)->v));
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(0)->v));
			glEnd();
			for (k = 0; k < pFace->GetNumVertices()-2; k++)
			{
				glBegin(GL_LINES);
					glColor3f(0.8f, 0.8f, 0.8f);
					glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k+1)->v));
					glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k+2)->v));

					glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k+2)->v));
					glVertex3fv(pMesh->GetVertex(pFace->GetVertex(0)->v));
				glEnd();
			}
#endif

		}

		glEndList();
	}

	m_flCameraDistance = sqrt(flFarthest);
	if (m_flCameraDistance < 100)
		m_flCameraDistance = 100;
}

void CModelWindow::SaveFile(const wchar_t* pszFile)
{
	if (!pszFile)
		return;

	CModelConverter c(&m_Scene);

	size_t iFileLength = wcslen(pszFile);
	std::wstring sExtension = pszFile+iFileLength-4;

	if (wcscmp(sExtension.c_str(), L".smd") == 0)
		c.WriteSMDs(pszFile);
}

void CModelWindow::Render()
{
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

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

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

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glLoadIdentity();
	gluPerspective(
			44.0,
			(float)m_iWindowWidth/(float)m_iWindowHeight,
			1,
			10000.0
		);

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

	if (m_avecDebugLines.size())
	{
		glPointSize(3);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
			glColor3f(1, 1, 1);
			for (size_t i = 0; i < m_avecDebugLines.size(); i+=2)
			{
				glVertex3fv(m_avecDebugLines[i]);
				glVertex3fv(m_avecDebugLines[i+1]);
			}
		glEnd();
	}

	glPopMatrix();
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
	GLfloat lightColor[] = {0.9f, 1.0f, 0.9f, 1.0f};

	// Reposition the light source.
	flLightPosition[0] = (float)cos(m_flLightPitch * (M_PI*2 / 360)) * (float)cos(m_flLightYaw * (M_PI*2 / 360)) * m_flCameraDistance/4;
	flLightPosition[1] = (float)sin(m_flLightPitch * (M_PI*2 / 360)) * m_flCameraDistance/4;
	flLightPosition[2] = (float)cos(m_flLightPitch * (M_PI*2 / 360)) * (float)sin(m_flLightYaw * (M_PI*2 / 360)) * m_flCameraDistance/4;
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
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);

	// It uses this color if the texture is missing.
	GLfloat flMaterialColor[] = {0.7f, 0.7f, 0.7f, 1.0f};

	for (size_t i = 0; i < m_aiObjects.size(); i++)
	{
		glPushMatrix();
		glMaterialfv(GL_FRONT, GL_DIFFUSE, flMaterialColor);
		glColor4fv(flMaterialColor);
		glCallList((GLuint)m_aiObjects[i]);
		glPopMatrix();
	}

	if (GLEW_VERSION_1_3)
	{
		// Disable the multi-texture stuff now that object drawing is done.
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}
}

void CModelWindow::WindowResize(int w, int h)
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

	modelgui::CRootPanel::Get()->Layout();

	Render();
}

void CModelWindow::Display()
{
}

void CModelWindow::Visible(int vis)
{
}

size_t CModelWindow::GetNextObjectId()
{
	return (m_iObjectsCreated++)+1;
}

void CModelWindow::ClearDebugLines()
{
	m_avecDebugLines.clear();
};

void CModelWindow::AddDebugLine(Vector vecStart, Vector vecEnd)
{
	m_avecDebugLines.push_back(vecStart);
	m_avecDebugLines.push_back(vecEnd);
};
