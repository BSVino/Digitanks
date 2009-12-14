#include "modelwindow.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <maths.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>

#include <modelconverter/modelconverter.h>
#include "modelgui.h"

CModelWindow* CModelWindow::s_pModelWindow = NULL;

CModelWindow::CModelWindow()
{
	s_pModelWindow = this;

	int argc = 1;
	char* argv = "smak";

	m_pLightHalo = NULL;
	m_pLightBeam = NULL;

	m_aiObjects.clear();
	m_iObjectsCreated = 0;

	m_flCameraDistance = 100;

	m_bCameraRotating = false;
	m_bCameraDollying = false;
	m_bCameraPanning = false;
	m_bLightRotating = false;

	m_iMouseStartX = 0;
	m_iMouseStartY = 0;

	m_flCameraYaw = 45;
	m_flCameraPitch = 45;

	m_flCameraUVZoom = 1;
	m_flCameraUVX = 0;
	m_flCameraUVY = 0;

	m_flLightYaw = 100;
	m_flLightPitch = 45;

	glutInit(&argc, &argv);

	int iScreenWidth = glutGet(GLUT_SCREEN_WIDTH);
	int iScreenHeight = glutGet(GLUT_SCREEN_HEIGHT);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA | GLUT_MULTISAMPLE);

	m_iWindowWidth = iScreenWidth*2/3;
	m_iWindowHeight = iScreenHeight*2/3;

	glutInitWindowPosition(iScreenWidth/6, iScreenHeight/6);
	glutInitWindowSize((int)m_iWindowWidth, (int)m_iWindowHeight);

	glutCreateWindow("SMAK - Super Model Army Knife");

	ilInit();

	size_t iTexture = LoadTextureIntoGL(L"lighthalo.png");
	if (iTexture)
		m_pLightHalo = new CMaterial(iTexture);

	iTexture = LoadTextureIntoGL(L"lightbeam.png");
	if (iTexture)
		m_pLightBeam = new CMaterial(iTexture);

	m_iWireframeTexture = LoadTextureIntoGL(L"wireframe.png");
	m_iFlatTexture = LoadTextureIntoGL(L"flat.png");
	m_iSmoothTexture = LoadTextureIntoGL(L"smooth.png");
	m_iLightTexture = LoadTextureIntoGL(L"light.png");
	m_iTextureTexture = LoadTextureIntoGL(L"texture.png");
	m_iAOTexture = LoadTextureIntoGL(L"ao.png");
	m_iCAOTexture = LoadTextureIntoGL(L"aocolor.png");

	InitUI();

	SetRenderMode(false);
	SetDisplayType(DT_SMOOTH);
	SetDisplayLight(true);
	SetDisplayTexture(true);
	SetDisplayAO(false);
	SetDisplayColorAO(false);

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
}

CModelWindow::~CModelWindow()
{
	if (m_pLightHalo)
		delete m_pLightHalo;

	if (m_pLightBeam)
		delete m_pLightBeam;
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

	ClearDebugLines();
}

size_t CModelWindow::LoadTextureIntoGL(const wchar_t* pszFilename)
{
	if (!pszFilename || !*pszFilename)
		return 0;

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilLoadImage(pszFilename);

	if (!bSuccess)
		bSuccess = ilLoadImage(pszFilename);

	ILenum iError = ilGetError();

	if (!bSuccess)
		return 0;

	bSuccess = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!bSuccess)
		return 0;

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

void CModelWindow::LoadTexturesIntoGL()
{
	for (size_t i = 0; i < m_Scene.GetNumMaterials(); i++)
	{
		CConversionMaterial* pMaterial = m_Scene.GetMaterial(i);

		m_aoMaterials.push_back(CMaterial(0));

		assert(m_aoMaterials.size()-1 == i);

		size_t iTexture = LoadTextureIntoGL(pMaterial->GetTexture());

		if (iTexture)
			m_aoMaterials[i].m_iBase = iTexture;
	}

	if (!m_aoMaterials.size())
	{
		m_aoMaterials.push_back(CMaterial(0));
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

			if (m_eDisplayType != DT_WIREFRAME)
			{
				if (pFace->m == ~0 || m_aoMaterials.size() == 0)
					glBindTexture(GL_TEXTURE_2D, 0);
				else
				{
					CMaterial* pMaterial = &m_aoMaterials[pFace->m];

					if (GLEW_VERSION_1_3)
					{
						bMultiTexture = true;

						glActiveTexture(GL_TEXTURE0);
						if (m_bDisplayTexture)
						{
							glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iBase);
							glEnable(GL_TEXTURE_2D);
						}
						else
						{
							glBindTexture(GL_TEXTURE_2D, (GLuint)0);
							glDisable(GL_TEXTURE_2D);
						}

						glActiveTexture(GL_TEXTURE1);
						if (m_bDisplayAO)
						{
							glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iAO);
							glEnable(GL_TEXTURE_2D);
						}
						else
						{
							glBindTexture(GL_TEXTURE_2D, (GLuint)0);
							glDisable(GL_TEXTURE_2D);
						}

						glActiveTexture(GL_TEXTURE2);
						if (m_bDisplayColorAO)
						{
							glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iColorAO);
							glEnable(GL_TEXTURE_2D);
						}
						else
						{
							glBindTexture(GL_TEXTURE_2D, (GLuint)0);
							glDisable(GL_TEXTURE_2D);
						}
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
						glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
					}
					else
						glTexCoord2fv(vecUV);

					glNormal3fv(vecNormal);
					glVertex3fv(vecVertex);

					if (vecVertex.LengthSqr() > flFarthest)
						flFarthest = vecVertex.LengthSqr();
				}

				glEnd();
			}

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

			if (m_eDisplayType == DT_WIREFRAME)
			{
				glBindTexture(GL_TEXTURE_2D, (GLuint)0);
				glColor3f(1.0f, 1.0f, 1.0f);
				glBegin(GL_LINE_STRIP);
					glNormal3fv(pMesh->GetNormal(pFace->GetVertex(0)->vn));
					glVertex3fv(pMesh->GetVertex(pFace->GetVertex(0)->v));
					glNormal3fv(pMesh->GetNormal(pFace->GetVertex(1)->vn));
					glVertex3fv(pMesh->GetVertex(pFace->GetVertex(1)->v));
					glNormal3fv(pMesh->GetNormal(pFace->GetVertex(2)->vn));
					glVertex3fv(pMesh->GetVertex(pFace->GetVertex(2)->v));
				glEnd();
				for (k = 0; k < pFace->GetNumVertices()-2; k++)
				{
					glBegin(GL_LINES);
						glNormal3fv(pMesh->GetNormal(pFace->GetVertex(k+1)->vn));
						glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k+1)->v));
						glNormal3fv(pMesh->GetNormal(pFace->GetVertex(k+2)->vn));
						glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k+2)->v));
					glEnd();
				}
				glBegin(GL_LINES);
					glNormal3fv(pMesh->GetNormal(pFace->GetVertex(k+1)->vn));
					glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k+1)->v));
					glNormal3fv(pMesh->GetNormal(pFace->GetVertex(0)->vn));
					glVertex3fv(pMesh->GetVertex(pFace->GetVertex(0)->v));
				glEnd();
			}

		}

		glEndList();
	}
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

	if (m_bRenderUV)
		RenderUV();
	else
		Render3D();
}

void CModelWindow::Render3D()
{
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
			44.0,
			(float)m_iWindowWidth/(float)m_iWindowHeight,
			1,
			10000.0
		);

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();

	Vector vecSceneCenter = m_Scene.m_oExtends.Center();

	Vector vecCameraVector = AngleVector(EAngle(m_flCameraPitch, m_flCameraYaw, 0)) * m_flCameraDistance + vecSceneCenter;

	gluLookAt(vecCameraVector.x, vecCameraVector.y, vecCameraVector.z,
		vecSceneCenter.x, vecSceneCenter.y, vecSceneCenter.z,
		0.0, 1.0, 0.0);

    RenderGround();

	RenderObjects();

	// Render light source on top of objects, since it doesn't use the depth buffer.
	RenderLightSource();

	if (m_avecDebugLines.size())
	{
		glLineWidth(1);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
			glColor3f(0.4f, 0.4f, 0.4f);
			for (size_t i = 0; i < m_avecDebugLines.size(); i+=2)
			{
				glVertex3fv(m_avecDebugLines[i]);
				glVertex3fv(m_avecDebugLines[i+1]);
			}
		glEnd();
		glBegin(GL_POINTS);
			glColor3f(0.6f, 0.6f, 0.6f);
			for (size_t i = 0; i < m_avecDebugLines.size(); i+=2)
			{
				glVertex3fv(m_avecDebugLines[i]);
				glVertex3fv(m_avecDebugLines[i+1]);
			}
		glEnd();
	}

	glPopMatrix();
	glPopAttrib();
}

void CModelWindow::RenderGround(void)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

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
	if (!m_bDisplayLight)
		return;

	GLfloat flLightPosition[4];
	GLfloat lightColor[] = {0.9f, 1.0f, 0.9f, 1.0f};

	// Reposition the light source.
	Vector vecLightDirection = AngleVector(EAngle(m_flLightPitch, m_flLightYaw, 0));

	flLightPosition[0] = vecLightDirection.x * m_flCameraDistance/2;
	flLightPosition[1] = vecLightDirection.y * m_flCameraDistance/2;
	flLightPosition[2] = vecLightDirection.z * m_flCameraDistance/2;
	flLightPosition[3] = 0.0;

	// Tell GL new light source position.
    glLightfv(GL_LIGHT0, GL_POSITION, flLightPosition);

	float flScale = m_flCameraDistance/60;

	glPushMatrix();
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

		glDisable(GL_LIGHTING);

		glTranslatef(flLightPosition[0], flLightPosition[1], flLightPosition[2]);
		glRotatef(-m_flLightYaw, 0, 1, 0);
		glRotatef(m_flLightPitch, 0, 0, 1);
		glScalef(flScale, flScale, flScale);

		if (m_pLightHalo && m_pLightBeam)
		{
			Vector vecCameraDirection = AngleVector(EAngle(m_flCameraPitch, m_flCameraYaw, 0));

			glEnable(GL_CULL_FACE);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glBindTexture(GL_TEXTURE_2D, (GLuint)m_pLightHalo->m_iBase);

			float flDot = vecCameraDirection.Dot(vecLightDirection);

			if (flDot < -0.2)
			{
				float flScale = RemapVal(flDot, -0.2f, -1.0f, 0.0f, 1.0f);
				glColor4f(1.0, 1.0, 1.0, flScale);

				flScale *= 10;

				glBegin(GL_QUADS);
					glTexCoord2f(0, 0);
					glVertex3f(0, -flScale, -flScale);
					glTexCoord2f(0, 1);
					glVertex3f(0, -flScale, flScale);
					glTexCoord2f(1, 1);
					glVertex3f(0, flScale, flScale);
					glTexCoord2f(1, 0);
					glVertex3f(0, flScale, -flScale);
				glEnd();
			}

			glDisable(GL_CULL_FACE);
			glBindTexture(GL_TEXTURE_2D, (GLuint)m_pLightBeam->m_iBase);

			Vector vecLightRight, vecLightUp;
			AngleVectors(EAngle(m_flLightPitch, m_flLightYaw, 0), NULL, &vecLightRight, &vecLightUp);

			flDot = vecCameraDirection.Dot(vecLightRight);

			glColor4f(1.0, 1.0, 1.0, fabs(flDot));

			glBegin(GL_QUADS);
				glTexCoord2f(1, 0);
				glVertex3f(-25, -5, 0);
				glTexCoord2f(0, 0);
				glVertex3f(-25, 5, 0);
				glTexCoord2f(0, 1);
				glVertex3f(0, 5, 0);
				glTexCoord2f(1, 1);
				glVertex3f(0, -5, 0);
			glEnd();

			flDot = vecCameraDirection.Dot(vecLightUp);

			glColor4f(1.0, 1.0, 1.0, fabs(flDot));

			glBegin(GL_QUADS);
				glTexCoord2f(1, 0);
				glVertex3f(-25, 0, -5);
				glTexCoord2f(0, 0);
				glVertex3f(-25, 0, 5);
				glTexCoord2f(0, 1);
				glVertex3f(0, 0, 5);
				glTexCoord2f(1, 1);
				glVertex3f(0, 0, -5);
			glEnd();

			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glColor3f(1.0, 1.0, 0.0);

			// Draw an arrowhead.
			glDisable(GL_CULL_FACE);
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
		}
	glPopAttrib();
	glPopMatrix();
}

void CModelWindow::RenderObjects()
{
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	if (m_bDisplayLight)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);

	if (m_eDisplayType == DT_SMOOTH)
		glShadeModel(GL_SMOOTH);
	else if (m_eDisplayType == DT_FLAT)
		glShadeModel(GL_FLAT);

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
		glActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}

	glPopAttrib();
}

void CModelWindow::RenderUV()
{
	glViewport(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

	// Switch GL to 2d drawing model.
	int aiViewport[4];
	glGetIntegerv(GL_VIEWPORT, aiViewport);

	float flRatio = (float)aiViewport[3] / (float)aiViewport[2];

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -flRatio, flRatio, -1, 1);

	glScalef(m_flCameraUVZoom, m_flCameraUVZoom, 0);
	glTranslatef(m_flCameraUVX, m_flCameraUVY, 0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);

	glShadeModel(GL_FLAT);

	bool bMultiTexture = false;

	CMaterial* pMaterial = NULL;
	if (m_aoMaterials.size())
		pMaterial = &m_aoMaterials[0];

	if (!pMaterial)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (GLEW_VERSION_1_3)
	{
		bMultiTexture = true;

		glActiveTexture(GL_TEXTURE0);
		if (m_bDisplayTexture)
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iBase);
			glEnable(GL_TEXTURE_2D);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glDisable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE1);
		if (m_bDisplayAO)
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iAO);
			glEnable(GL_TEXTURE_2D);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glDisable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE2);
		if (m_bDisplayColorAO)
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iColorAO);
			glEnable(GL_TEXTURE_2D);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glDisable(GL_TEXTURE_2D);
		}
	}
	else
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iBase);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	}

	if (!pMaterial)
		glColor3f(0.8f, 0.8f, 0.8f);
	else if (m_bDisplayTexture || m_bDisplayAO || m_bDisplayColorAO)
		glColor3f(1.0f, 1.0f, 1.0f);
	else
		glColor3f(0.0f, 0.0f, 0.0f);

	Vector vecUV;

	glBegin(GL_QUADS);

		vecUV = Vector(0.0f, 1.0f, 0.0f);
		if (bMultiTexture)
		{
			glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertex2f(-0.5f, -0.5f);

		vecUV = Vector(1.0f, 1.0f, 0.0f);
		if (bMultiTexture)
		{
			glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertex2f(0.5f, -0.5f);

		vecUV = Vector(1.0f, 0.0f, 0.0f);
		if (bMultiTexture)
		{
			glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertex2f(0.5f, 0.5f);

		vecUV = Vector(0.0f, 0.0f, 0.0f);
		if (bMultiTexture)
		{
			glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertex2f(-0.5f, 0.5f);

	glEnd();

	if (GLEW_VERSION_1_3)
	{
		// Disable the multi-texture stuff now that object drawing is done.
		glActiveTexture(GL_TEXTURE1);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE2);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	}

	if (m_eDisplayType == DT_WIREFRAME)
	{
		Vector vecOffset(-0.5f, -0.5f, 0);

		for (size_t i = 0; i < m_Scene.GetNumMeshes(); i++)
		{
			CConversionMesh* pMesh = m_Scene.GetMesh(i);

			for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
			{
				size_t k;
				CConversionFace* pFace = pMesh->GetFace(j);

				glBindTexture(GL_TEXTURE_2D, (GLuint)0);
				glColor3f(1.0f, 1.0f, 1.0f);
				glBegin(GL_LINE_STRIP);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(0)->vt) + vecOffset);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(1)->vt) + vecOffset);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(2)->vt) + vecOffset);
				glEnd();
				for (k = 0; k < pFace->GetNumVertices()-2; k++)
				{
					glBegin(GL_LINES);
						glVertex3fv(pMesh->GetUV(pFace->GetVertex(k+1)->vt) + vecOffset);
						glVertex3fv(pMesh->GetUV(pFace->GetVertex(k+2)->vt) + vecOffset);
					glEnd();
				}
				glBegin(GL_LINES);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(k+1)->vt) + vecOffset);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(0)->vt) + vecOffset);
				glEnd();
			}
		}
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
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

	Render();
	modelgui::CRootPanel::Get()->Layout();
	modelgui::CRootPanel::Get()->Paint();

	glutSwapBuffers();
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

void CModelWindow::SetRenderMode(bool bUV)
{
	m_pRender3D->SetState(false, false);
	m_pRenderUV->SetState(false, false);

	if (bUV)
		m_pRenderUV->SetState(true, false);
	else
		m_pRender3D->SetState(true, false);

	m_bRenderUV = bUV;
}

void CModelWindow::SetDisplayType(displaytype_t eType)
{
	m_pWireframe->SetState(false, false);
	m_pFlat->SetState(false, false);
	m_pSmooth->SetState(false, false);

	if (eType == DT_SMOOTH)
		m_pSmooth->SetState(true, false);
	else if (eType == DT_FLAT)
		m_pFlat->SetState(true, false);
	else if (eType == DT_WIREFRAME)
		m_pWireframe->SetState(true, false);

	m_eDisplayType = eType;

	CreateGLLists();
}

void CModelWindow::SetDisplayLight(bool bLight)
{
	m_bDisplayLight = bLight;
	m_pLight->SetState(bLight, false);
}

void CModelWindow::SetDisplayTexture(bool bTexture)
{
	m_bDisplayTexture = bTexture;
	m_pTexture->SetState(bTexture, false);
	CreateGLLists();
}

void CModelWindow::SetDisplayAO(bool bAO)
{
	m_bDisplayAO = bAO;
	m_pAO->SetState(bAO, false);
	CreateGLLists();
}

void CModelWindow::SetDisplayColorAO(bool bColorAO)
{
	m_bDisplayColorAO = bColorAO;
	m_pColorAO->SetState(bColorAO, false);
	CreateGLLists();
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
