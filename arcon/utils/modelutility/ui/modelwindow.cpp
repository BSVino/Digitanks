#include "modelwindow.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <maths.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <modelconverter/modelconverter.h>
#include "modelgui.h"
#include "scenetree.h"
#include "../shaders/shaders.h"

//#define RAYTRACE_DEBUG
#ifdef RAYTRACE_DEBUG
#include <raytracer/raytracer.h>
#endif

extern "C" {
static void CALLBACK RenderTesselateBegin(GLenum ePrim);
static void CALLBACK RenderTesselateVertex(void* pVertexData, void* pPolygonData);
static void CALLBACK RenderTesselateEnd();
}

CModelWindow* CModelWindow::s_pModelWindow = NULL;

CModelWindow::CModelWindow()
{
	s_pModelWindow = this;

	int argc = 1;
	char* argv = "smak";

	m_bLoadingFile = false;

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

	LoadSMAKTexture();

	m_iWireframeTexture = LoadTextureIntoGL(L"wireframe.png");
	m_iSmoothTexture = LoadTextureIntoGL(L"smooth.png");
	m_iUVTexture = LoadTextureIntoGL(L"uv.png");
	m_iLightTexture = LoadTextureIntoGL(L"light.png");
	m_iTextureTexture = LoadTextureIntoGL(L"texture.png");
	m_iNormalTexture = LoadTextureIntoGL(L"normal.png");
	m_iAOTexture = LoadTextureIntoGL(L"ao.png");
	m_iCAOTexture = LoadTextureIntoGL(L"aocolor.png");
	m_iArrowTexture = LoadTextureIntoGL(L"arrow.png");
	m_iEditTexture = LoadTextureIntoGL(L"pencil.png");
	m_iVisibilityTexture = LoadTextureIntoGL(L"eye.png");

	InitUI();

	SetRenderMode(false);
	SetDisplayWireframe(false);
	SetDisplayUVWireframe(true);
	SetDisplayLight(true);
	SetDisplayTexture(true);
	SetDisplayAO(false);
	SetDisplayColorAO(false);

	m_pTesselator = gluNewTess();
	gluTessCallback(m_pTesselator, GLU_TESS_BEGIN, (void(CALLBACK*)())RenderTesselateBegin);
	gluTessCallback(m_pTesselator, GLU_TESS_VERTEX_DATA, (void(CALLBACK*)())RenderTesselateVertex);
	gluTessCallback(m_pTesselator, GLU_TESS_END, (void(CALLBACK*)())RenderTesselateEnd);
	gluTessProperty(m_pTesselator, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

	GLenum err = glewInit();
	if (GLEW_OK != err)
		exit(0);

	CompileShaders();

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

	CSceneTreePanel::Get()->UpdateTree();
}

CModelWindow::~CModelWindow()
{
	if (m_pLightHalo)
		delete m_pLightHalo;

	if (m_pLightBeam)
		delete m_pLightBeam;

	gluDeleteTess(m_pTesselator);
}

void CModelWindow::CompileShaders()
{
	GLuint iVertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* pszShaderSource = GetVSModelShader();
	glShaderSource(iVertexShader, 1, &pszShaderSource, NULL);
	glCompileShader(iVertexShader);

#ifdef _DEBUG
	int iLogLength = 0;
	char szLog[1024];
	glGetShaderInfoLog(iVertexShader, 1024, &iLogLength, szLog);
#endif

	GLuint iFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	pszShaderSource = GetFSModelShader();
	glShaderSource(iFragmentShader, 1, &pszShaderSource, NULL);
	glCompileShader(iFragmentShader);

#ifdef _DEBUG
	glGetShaderInfoLog(iFragmentShader, 1024, &iLogLength, szLog);
#endif

	GLuint iShaderProgram = glCreateProgram();
	glAttachShader(iShaderProgram, iVertexShader);
	glAttachShader(iShaderProgram, iFragmentShader);
	glLinkProgram(iShaderProgram);

#ifdef _DEBUG
	glGetProgramInfoLog(iShaderProgram, 1024, &iLogLength, szLog);
#endif

	m_iShaderProgram = (size_t)iShaderProgram;
}

void CModelWindow::Run()
{
	while (true)
	{
		glutMainLoopEvent();
		Render();
		modelgui::CRootPanel::Get()->Think();
		modelgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);
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

	CRootPanel::Get()->UpdateScene();

	CSceneTreePanel::Get()->UpdateTree();

	CRootPanel::Get()->Layout();
}

void CModelWindow::ReadFile(const wchar_t* pszFile)
{
	if (!pszFile)
		return;

	if (m_bLoadingFile)
		return;

	// Save it in here in case m_szFileLoaded was passed into ReadFile, in which case it would be destroyed by DestroyAll.
	std::wstring sFile = pszFile;

	DestroyAll();

	ReadFileIntoScene(sFile.c_str());
}

void CModelWindow::ReadFileIntoScene(const wchar_t* pszFile)
{
	m_bLoadingFile = true;

	CModelConverter c(&m_Scene);

	c.SetWorkListener(this);

	if (!c.ReadModel(pszFile))
	{
		m_bLoadingFile = false;
		return;
	}

	wcscpy(m_szFileLoaded, pszFile);

	BeginProgress();
	SetAction(L"Loading into video hardware", 0);
	LoadIntoGL();
	EndProgress();

	m_flCameraDistance = m_Scene.m_oExtends.Size().Length() * 1.5f;

	CSceneTreePanel::Get()->UpdateTree();

	m_bLoadingFile = false;
}

void CModelWindow::ReloadFromFile()
{
	ReadFile(m_szFileLoaded);
}

void CModelWindow::LoadIntoGL()
{
	LoadTexturesIntoGL();

	ClearDebugLines();
}

size_t CModelWindow::LoadTextureIntoGL(std::wstring sFilename)
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

void CModelWindow::LoadTexturesIntoGL()
{
	for (size_t i = 0; i < m_Scene.GetNumMaterials(); i++)
	{
		CConversionMaterial* pMaterial = m_Scene.GetMaterial(i);

		if (i < m_aoMaterials.size())
			continue;

		m_aoMaterials.push_back(CMaterial(0));

		size_t iTexture = LoadTextureIntoGL(pMaterial->GetDiffuseTexture());

		if (iTexture)
			m_aoMaterials[i].m_iBase = iTexture;
	}

	if (!m_aoMaterials.size())
	{
		m_aoMaterials.push_back(CMaterial(0));
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

	Vector vecSceneCenter = m_Scene.m_oExtends.Center();

	Vector vecCameraVector = AngleVector(EAngle(m_flCameraPitch, m_flCameraYaw, 0)) * m_flCameraDistance + vecSceneCenter;

	gluLookAt(vecCameraVector.x, vecCameraVector.y, vecCameraVector.z,
		vecSceneCenter.x, vecSceneCenter.y, vecSceneCenter.z,
		0.0, 1.0, 0.0);

	// Reposition the light source.
	Vector vecLightDirection = AngleVector(EAngle(m_flLightPitch, m_flLightYaw, 0));

	m_vecLightPosition = vecLightDirection * m_flCameraDistance/2;

	GLfloat flLightPosition[4];
	flLightPosition[0] = m_vecLightPosition.x;
	flLightPosition[1] = m_vecLightPosition.y;
	flLightPosition[2] = m_vecLightPosition.z;
	flLightPosition[3] = 0;

	// Tell GL new light source position.
    glLightfv(GL_LIGHT0, GL_POSITION, flLightPosition);

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

#ifdef RAYTRACE_DEBUG
	static raytrace::CRaytracer* pTracer = NULL;
	Vector vecStart(0.55841917f, 0.28102291f, 2.4405572f);
	Vector vecDirection(-0.093336411f, 0.99130136f, -0.092789143f);
	if (!pTracer && GetScene()->GetNumScenes() && !m_bLoadingFile)
	{
		pTracer = new raytrace::CRaytracer(GetScene());
		pTracer->AddMeshInstance(GetScene()->GetScene(0)->GetChild(2)->GetMeshInstance(0));
		pTracer->BuildTree();
		AddDebugLine(vecStart, vecStart+vecDirection*10);
	}
	if (pTracer)
		pTracer->Raytrace(Ray(vecStart, vecDirection));
#endif

	glPopMatrix();
	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void CModelWindow::RenderGround(void)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();

	glTranslatef(m_Scene.m_oExtends.Center().x, m_Scene.m_oExtends.Center().y, m_Scene.m_oExtends.Center().z);

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

				if (j == 10)
					glVertex3fv(Vector(0, 0, 0));
				else
					glVertex3fv(vecEndX);

				if (j == 10)
				{
					glColor3f(0.7f, 0.2f, 0.2f);
					glVertex3fv(Vector(0, 0, 0));
					glVertex3fv(Vector(100, 0, 0));
				}

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

				if (j == 10)
					glVertex3fv(Vector(0, 0, 0));
				else
					glVertex3fv(vecEndZ);

				if (j == 10)
				{
					glColor3f(0.2f, 0.2f, 0.7f);
					glVertex3fv(Vector(0, 0, 0));
					glVertex3fv(Vector(0, 0, 100));
				}

			glEnd();

			vecStartX.x += 10;
			vecEndX.x += 10;
			vecStartZ.z += 10;
			vecEndZ.z += 10;
		}
	}

	glPopMatrix();
}

void CModelWindow::RenderLightSource()
{
	if (!m_bDisplayLight)
		return;

	float flScale = m_flCameraDistance/60;

	glPushMatrix();
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

		glDisable(GL_LIGHTING);

		glTranslatef(m_Scene.m_oExtends.Center().x, m_Scene.m_oExtends.Center().y, m_Scene.m_oExtends.Center().z);
		glTranslatef(m_vecLightPosition[0], m_vecLightPosition[1], m_vecLightPosition[2]);
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

			float flDot = vecCameraDirection.Dot(m_vecLightPosition.Normalized());

			if (flDot < -0.2)
			{
				float flScale = RemapVal(flDot, -0.2f, -1.0f, 0.0f, 1.0f);
				glColor4f(1.0, 1.0, 1.0, flScale);

				flScale *= 10;

				glBegin(GL_QUADS);
					glTexCoord2f(1, 0);
					glVertex3f(0, -flScale, -flScale);
					glTexCoord2f(1, 1);
					glVertex3f(0, -flScale, flScale);
					glTexCoord2f(0, 1);
					glVertex3f(0, flScale, flScale);
					glTexCoord2f(0, 0);
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
				glTexCoord2f(1, 1);
				glVertex3f(-25, -5, 0);
				glTexCoord2f(0, 1);
				glVertex3f(-25, 5, 0);
				glTexCoord2f(0, 0);
				glVertex3f(0, 5, 0);
				glTexCoord2f(1, 0);
				glVertex3f(0, -5, 0);
			glEnd();

			flDot = vecCameraDirection.Dot(vecLightUp);

			glColor4f(1.0, 1.0, 1.0, fabs(flDot));

			glBegin(GL_QUADS);
				glTexCoord2f(1, 1);
				glVertex3f(-25, 0, -5);
				glTexCoord2f(0, 1);
				glVertex3f(-25, 0, 5);
				glTexCoord2f(0, 0);
				glVertex3f(0, 0, 5);
				glTexCoord2f(1, 0);
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
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);

	for (size_t i = 0; i < m_Scene.GetNumScenes(); i++)
		RenderSceneNode(m_Scene.GetScene(i));

	if (GLEW_VERSION_1_3)
	{
		// Disable the multi-texture stuff now that object drawing is done.
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE3);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE4);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}

	glPopAttrib();
}

void CModelWindow::RenderSceneNode(CConversionSceneNode* pNode)
{
	if (!pNode)
		return;

	if (!pNode->IsVisible())
		return;

	glPushMatrix();

	glMultMatrixf(pNode->m_mTransformations.Transposed());	// GL uses column major.

	for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		RenderSceneNode(pNode->GetChild(i));

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		RenderMeshInstance(pNode->GetMeshInstance(m));

	glPopMatrix();
}

// Ew!
GLuint g_iTangentAttrib;
GLuint g_iBitangentAttrib;
bool g_bNormalMap;

extern "C" {
static void CALLBACK RenderTesselateBegin(GLenum ePrim)
{
	glBegin(ePrim);
}

static void CALLBACK RenderTesselateVertex(void* pVertexData, void* pPolygonData)
{
	CConversionMesh* pMesh = (CConversionMesh*)pPolygonData;
	CConversionVertex* pVertex = (CConversionVertex*)pVertexData;

	Vector vecVertex = pMesh->GetVertex(pVertex->v);
	Vector vecUV = pMesh->GetUV(pVertex->vu);

	if (GLEW_VERSION_1_3)
	{
		glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
		glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
		glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
		glMultiTexCoord2fv(GL_TEXTURE3, vecUV); 
		glMultiTexCoord2fv(GL_TEXTURE4, vecUV); 
	}
	else
		glTexCoord2fv(vecUV);

	if (g_bNormalMap)
	{
		glVertexAttrib3fv(g_iTangentAttrib, pMesh->GetTangent(pVertex->vt));
		glVertexAttrib3fv(g_iBitangentAttrib, pMesh->GetBitangent(pVertex->vb));
		glNormal3fv(pMesh->GetNormal(pVertex->vn));
	}
	else
		glNormal3fv(pMesh->GetNormal(pVertex->vn));

	glVertex3fv(vecVertex);
}

static void CALLBACK RenderTesselateEnd()
{
	glEnd();
}
}

void CModelWindow::RenderMeshInstance(CConversionMeshInstance* pMeshInstance)
{
	if (!pMeshInstance->IsVisible())
		return;

	// It uses this color if the texture is missing.
	GLfloat flMaterialColor[] = {0.7f, 0.7f, 0.7f, 1.0f};

	glMaterialfv(GL_FRONT, GL_DIFFUSE, flMaterialColor);
	glColor4fv(flMaterialColor);

	bool bMultiTexture = false;

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
	{
		size_t k;
		CConversionFace* pFace = pMesh->GetFace(j);

		if (pFace->m != ~0 && pMeshInstance->GetMappedMaterial(pFace->m))
		{
			if (!pMeshInstance->GetMappedMaterial(pFace->m)->IsVisible())
				continue;

			CConversionMaterial* pMaterial = m_Scene.GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial);
			if (pMaterial && !pMaterial->IsVisible())
				continue;
		}

		if (!m_bDisplayWireframe)
		{
			bool bTexture = m_bDisplayTexture;
			bool bNormal = m_bDisplayNormal;
			bool bNormal2 = m_bDisplayNormal;
			g_bNormalMap = m_bDisplayNormal;
			bool bAO = m_bDisplayAO;
			bool bCAO = m_bDisplayColorAO;

			if (pFace->m == ~0 || !pMeshInstance->GetMappedMaterial(pFace->m) || pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial == ~0 || m_aoMaterials.size() == 0)
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				bTexture = false;
				bNormal = false;
				bNormal2 = false;
				g_bNormalMap = false;
				bAO = false;
				bCAO = false;
			}
			else
			{
				CMaterial* pMaterial = &m_aoMaterials[pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial];

				if (!pMaterial->m_iBase)
					bTexture = false;
				if (!pMaterial->m_iNormal)
					bNormal = false;
				if (!pMaterial->m_iNormal2)
					bNormal2 = false;
				if (!bNormal && !bNormal2)
					g_bNormalMap = false;
				if (!pMaterial->m_iAO)
					bAO = false;
				if (!pMaterial->m_iColorAO)
					bCAO = false;

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
					if (bNormal)
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iNormal);
						glEnable(GL_TEXTURE_2D);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)0);
						glDisable(GL_TEXTURE_2D);
					}

					glActiveTexture(GL_TEXTURE2);
					if (bNormal2)
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iNormal2);
						glEnable(GL_TEXTURE_2D);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)0);
						glDisable(GL_TEXTURE_2D);
					}

					glActiveTexture(GL_TEXTURE3);
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

					glActiveTexture(GL_TEXTURE4);
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

			if (pFace->m != ~0 && pMeshInstance->GetMappedMaterial(pFace->m) && m_Scene.GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial))
			{
				CConversionMaterial* pMaterial = m_Scene.GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial);
				glMaterialfv(GL_FRONT, GL_AMBIENT, pMaterial->m_vecAmbient);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, pMaterial->m_vecDiffuse);
				glMaterialfv(GL_FRONT, GL_SPECULAR, pMaterial->m_vecSpecular);
				glMaterialfv(GL_FRONT, GL_EMISSION, pMaterial->m_vecEmissive);
				glMaterialf(GL_FRONT, GL_SHININESS, pMaterial->m_flShininess);
				glColor4fv(pMaterial->m_vecDiffuse);
			}

			glUseProgram((GLuint)m_iShaderProgram);

			GLuint bLighting = glGetUniformLocation((GLuint)m_iShaderProgram, "bLighting");
			GLuint bDiffuseTexture = glGetUniformLocation((GLuint)m_iShaderProgram, "bDiffuseTexture");
			GLuint bNormalMap = glGetUniformLocation((GLuint)m_iShaderProgram, "bNormalMap");
			GLuint bNormal2Map = glGetUniformLocation((GLuint)m_iShaderProgram, "bNormal2Map");
			GLuint bAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "bAOMap");
			GLuint bCAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "bCAOMap");

			GLuint iDiffuseTexture = glGetUniformLocation((GLuint)m_iShaderProgram, "iDiffuseTexture");
			GLuint iNormalMap = glGetUniformLocation((GLuint)m_iShaderProgram, "iNormalMap");
			GLuint iNormal2Map = glGetUniformLocation((GLuint)m_iShaderProgram, "iNormal2Map");
			GLuint iAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "iAOMap");
			GLuint iCAOMap = glGetUniformLocation((GLuint)m_iShaderProgram, "iCAOMap");

			g_iTangentAttrib = glGetAttribLocation((GLuint)m_iShaderProgram, "vecTangent");
			g_iBitangentAttrib = glGetAttribLocation((GLuint)m_iShaderProgram, "vecBitangent");

			glUniform1i(iDiffuseTexture, 0);
			glUniform1i(iNormalMap, 1);
			glUniform1i(iNormal2Map, 2);
			glUniform1i(iAOMap, 3);
			glUniform1i(iCAOMap, 4);

			glUniform1i(bLighting, m_bDisplayLight);
			glUniform1i(bDiffuseTexture, bTexture);
			glUniform1i(bNormalMap, bNormal);
			glUniform1i(bNormal2Map, bNormal2);
			glUniform1i(bAOMap, bAO);
			glUniform1i(bCAOMap, bCAO);

			gluTessBeginPolygon(m_pTesselator, pMesh);
			gluTessBeginContour(m_pTesselator);

			for (k = 0; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(k);

				Vector vecVertex = pMesh->GetVertex(pVertex->v);
				GLdouble afCoords[3] = { vecVertex.x, vecVertex.y, vecVertex.z };
				gluTessVertex(m_pTesselator, afCoords, pVertex);
			}

			gluTessEndContour(m_pTesselator);
			gluTessEndPolygon(m_pTesselator);

			glUseProgram(0);
		}

		if (m_bDisplayWireframe)
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

#if 0
		glDisable(GL_LIGHTING);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, (GLuint)0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (GLuint)0);

		for (k = 0; k < pFace->GetNumVertices(); k++)
		{
			CConversionVertex* pVertex = pFace->GetVertex(k);

			Vector vecVertex = pMesh->GetVertex(pVertex->v);
			Vector vecNormal = pMesh->GetNormal(pVertex->vn);
			Vector vecTangent = pMesh->GetTangent(pVertex->vt);
			Vector vecBitangent = pMesh->GetBitangent(pVertex->vb);
			//vecNormal = Vector(pVertex->m_mInverseTBN.GetColumn(2));
			//vecTangent = Vector(pVertex->m_mInverseTBN.GetColumn(0));
			//vecBitangent = Vector(pVertex->m_mInverseTBN.GetColumn(1));

			glColor3f(0.2f, 0.2f, 0.8f);

			glBegin(GL_LINES);
			glNormal3fv(vecNormal);
			glVertex3fv(vecVertex);
			glVertex3fv(vecVertex + vecNormal);
			glEnd();

			glColor3f(0.8f, 0.2f, 0.2f);

			glBegin(GL_LINES);
			glNormal3fv(vecTangent);
			glVertex3fv(vecVertex);
			glVertex3fv(vecVertex + vecTangent);
			glEnd();

			glColor3f(0.2f, 0.8f, 0.2f);

			glBegin(GL_LINES);
			glNormal3fv(vecBitangent);
			glVertex3fv(vecVertex);
			glVertex3fv(vecVertex + vecBitangent);
			glEnd();
		}
#endif
	}
}

void CModelWindow::RenderUV()
{
	glViewport(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

	// Switch GL to 2d drawing mode.

	float flRatio = (float)m_iWindowHeight / (float)m_iWindowWidth;

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
	glDisable(GL_CULL_FACE);

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
		if (m_bDisplayNormal)
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iNormal);
			glEnable(GL_TEXTURE_2D);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glDisable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE3);
		if (m_bDisplayNormal)
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iNormal2);
			glEnable(GL_TEXTURE_2D);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)0);
			glDisable(GL_TEXTURE_2D);
		}

		glActiveTexture(GL_TEXTURE4);
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

		glActiveTexture(GL_TEXTURE5);
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
	else if (!pMaterial->m_iBase && !(m_bDisplayAO || m_bDisplayColorAO))
		glColor3f(0.0f, 0.0f, 0.0f);
	else if (m_bDisplayTexture || m_bDisplayNormal || m_bDisplayAO || m_bDisplayColorAO)
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
			glMultiTexCoord2fv(GL_TEXTURE3, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE4, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertex2f(-0.5f, 0.5f);

		vecUV = Vector(1.0f, 1.0f, 0.0f);
		if (bMultiTexture)
		{
			glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE3, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE4, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertex2f(0.5f, 0.5f);

		vecUV = Vector(1.0f, 0.0f, 0.0f);
		if (bMultiTexture)
		{
			glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE3, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE4, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertex2f(0.5f, -0.5f);

		vecUV = Vector(0.0f, 0.0f, 0.0f);
		if (bMultiTexture)
		{
			glMultiTexCoord2fv(GL_TEXTURE0, vecUV);
			glMultiTexCoord2fv(GL_TEXTURE1, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE2, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE3, vecUV); 
			glMultiTexCoord2fv(GL_TEXTURE4, vecUV); 
		}
		else
			glTexCoord2fv(vecUV);
		glVertex2f(-0.5f, -0.5f);

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
		glActiveTexture(GL_TEXTURE3);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE4);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	}

	if (!CModelWindow::Get()->GetSMAKTexture() && (m_bDisplayAO || m_bDisplayColorAO))
	{
		static char szFont[1024];
		sprintf(szFont, "%s\\Fonts\\Arial.ttf", getenv("windir"));

		FTGLPixmapFont*	pFont = new FTGLPixmapFont(szFont);
		pFont->FaceSize(48);

		glColor4ubv(Color(155, 155, 255, 60));

		glRasterPos2f(0.15f, 0.2f);
		pFont->Render("DEMO");

		glRasterPos2f(-0.35f, 0.2f);
		pFont->Render("DEMO");

		glRasterPos2f(0.15f, -0.2f);
		pFont->Render("DEMO");

		glRasterPos2f(-0.35f, -0.2f);
		pFont->Render("DEMO");

		glColor4ubv(Color(255, 255, 255, 180));
		glRasterPos2f(-0.5f, 0.51f);
		pFont->FaceSize(16);
		pFont->Render("This demo version will generate all map sizes, but will downsample to 128x128 when saving.");

		delete pFont;
	}

	if (m_bDisplayUV)
	{
		Vector vecOffset(-0.5f, -0.5f, 0);

		for (size_t i = 0; i < m_Scene.GetNumMeshes(); i++)
		{
			CConversionMesh* pMesh = m_Scene.GetMesh(i);

			if (!pMesh->GetNumUVs())
				continue;

			for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
			{
				size_t k;
				CConversionFace* pFace = pMesh->GetFace(j);

				glBindTexture(GL_TEXTURE_2D, (GLuint)0);
				glColor3f(0.6f, 0.6f, 0.6f);
				glBegin(GL_LINE_STRIP);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(0)->vu) + vecOffset);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(1)->vu) + vecOffset);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(2)->vu) + vecOffset);
				glEnd();
				for (k = 0; k < pFace->GetNumVertices()-2; k++)
				{
					glBegin(GL_LINES);
						glVertex3fv(pMesh->GetUV(pFace->GetVertex(k+1)->vu) + vecOffset);
						glVertex3fv(pMesh->GetUV(pFace->GetVertex(k+2)->vu) + vecOffset);
					glEnd();
				}
				glBegin(GL_LINES);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(k+1)->vu) + vecOffset);
					glVertex3fv(pMesh->GetUV(pFace->GetVertex(0)->vu) + vecOffset);
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
	modelgui::CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);

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

	m_pWireframe->SetVisible(!m_bRenderUV);
	m_pUVWireframe->SetVisible(m_bRenderUV);

	Layout();
}

void CModelWindow::SetDisplayWireframe(bool bWire)
{
	m_bDisplayWireframe = bWire;
	m_pWireframe->SetState(bWire, false);
}

void CModelWindow::SetDisplayUVWireframe(bool bWire)
{
	m_bDisplayUV = bWire;
	m_pUVWireframe->SetState(bWire, false);
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
}

void CModelWindow::SetDisplayNormal(bool bNormal)
{
	m_bDisplayNormal = bNormal;
	m_pNormal->SetState(bNormal, false);
}

void CModelWindow::SetDisplayAO(bool bAO)
{
	m_bDisplayAO = bAO;
	m_pAO->SetState(bAO, false);
}

void CModelWindow::SetDisplayColorAO(bool bColorAO)
{
	m_bDisplayColorAO = bColorAO;
	m_pColorAO->SetState(bColorAO, false);
}

void CModelWindow::ClearDebugLines()
{
	m_avecDebugLines.clear();
}

void CModelWindow::AddDebugLine(Vector vecStart, Vector vecEnd)
{
	m_avecDebugLines.push_back(vecStart);
	m_avecDebugLines.push_back(vecEnd);
}
