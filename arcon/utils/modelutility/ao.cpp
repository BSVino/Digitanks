#include "crunch.h"

#include <assert.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <time.h>

#include <geometry.h>
#include <maths.h>
#include <matrix.h>
#include <raytracer/raytracer.h>
#include "shaders/shaders.h"
#include <platform.h>

#if 0
#ifdef _DEBUG
#define AO_DEBUG
#endif
#endif

#ifdef AO_DEBUG
#include "ui/modelwindow.h"

void DrawTexture(GLuint iTexture, float flScale = 1.0f);
#endif

CAOGenerator::CAOGenerator(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials)
{
	m_eAOMethod = AOMETHOD_NONE;
	m_pScene = pScene;
	m_paoMaterials = paoMaterials;

	m_avecShadowValues = NULL;
	m_avecShadowGeneratedValues = NULL;
	m_aiShadowReads = NULL;
	m_bPixelMask = NULL;
	m_pPixels = NULL;

	// Default options
	SetSize(512, 512);
	m_bUseTexture = true;
	m_iBleed = 5;
	m_iSamples = 15;
	m_bRandomize = false;
	m_bCreaseEdges = true;
	m_bGroundOcclusion = false;
	m_flRayFalloff = 1;

	SetRenderPreviewViewport(0, 0, 100, 100);
	SetUseFrontBuffer(false);

	m_pWorkListener = NULL;

	m_bIsGenerating = false;
	m_bDoneGenerating = false;
	m_bStopGenerating = false;

	m_pRaytraceParallelizer = NULL;
}

CAOGenerator::~CAOGenerator()
{
	free(m_pPixels);
	free(m_bPixelMask);
	delete[] m_avecShadowValues;
	delete[] m_avecShadowGeneratedValues;
	delete[] m_aiShadowReads;

	if (m_pRaytraceParallelizer)
		delete m_pRaytraceParallelizer;
}

void CAOGenerator::SetSize(size_t iWidth, size_t iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	if (m_avecShadowValues)
	{
		delete[] m_avecShadowValues;
		delete[] m_avecShadowGeneratedValues;
		delete[] m_aiShadowReads;
	}

	// Shadow volume result buffer.
	m_avecShadowValues = new Vector[iWidth*iHeight];
	m_avecShadowGeneratedValues = new Vector[iWidth*iHeight];
	m_aiShadowReads = new size_t[iWidth*iHeight];

	// Big hack incoming!
	memset(&m_avecShadowValues[0].x, 0, iWidth*iHeight*sizeof(Vector));
	memset(&m_avecShadowGeneratedValues[0].x, 0, iWidth*iHeight*sizeof(Vector));
	memset(&m_aiShadowReads[0], 0, iWidth*iHeight*sizeof(size_t));

	if (m_bPixelMask)
		free(m_bPixelMask);
	m_bPixelMask = (bool*)malloc(m_iWidth*m_iHeight*sizeof(bool));
}

void CAOGenerator::SetRenderPreviewViewport(int x, int y, int w, int h)
{
	m_iRPVX = x;
	m_iRPVY = y;
	m_iRPVW = w;
	m_iRPVH = h;

	if (m_pPixels)
		free(m_pPixels);

	// Pixel reading buffer
	m_iPixelDepth = 4;
	size_t iBufferSize = m_iRPVW*m_iRPVH*sizeof(GLfloat)*m_iPixelDepth;
	m_pPixels = (GLfloat*)malloc(iBufferSize);
}

extern "C" {
static void CALLBACK ShadowMapTesselateBegin(GLenum ePrim)
{
	glBegin(ePrim);
}

// I don't like it either.
static bool g_bCreaseEdges = false;
static size_t g_iCreaseFace = 0;
static void CALLBACK ShadowMapTesselateVertex(void* pVertexData, void* pPolygonData)
{
	CConversionMeshInstance* pMeshInstance = (CConversionMeshInstance*)pPolygonData;
	CConversionMesh* pMesh = pMeshInstance->GetMesh();
	CConversionVertex* pVertex = (CConversionVertex*)pVertexData;

	Vector vecVertex = pMeshInstance->GetVertex(pVertex->v);
	Vector vecNormal;
	if (g_bCreaseEdges)
		vecNormal = pMesh->GetFace(g_iCreaseFace)->GetNormal();
	else
		vecNormal = pMeshInstance->GetNormal(pVertex->vn);

	// Translate here so it takes up the whole viewport when flattened by the shader.
	Vector vecUV = pMesh->GetUV(pVertex->vu) * 2 - Vector(1,1,1);

	glTexCoord2fv(vecUV);
	glNormal3fv(vecNormal);
	glVertex3fv(vecVertex);
}

static void CALLBACK ShadowMapTesselateEnd()
{
	glEnd();
}
}

void CAOGenerator::ShadowMapSetupScene()
{
	// Tuck away our current stack so we can return to it later.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	GLUtesselator* pTesselator = gluNewTess();
	gluTessCallback(pTesselator, GLU_TESS_BEGIN, (void(CALLBACK*)())ShadowMapTesselateBegin);
	gluTessCallback(pTesselator, GLU_TESS_VERTEX_DATA, (void(CALLBACK*)())ShadowMapTesselateVertex);
	gluTessCallback(pTesselator, GLU_TESS_END, (void(CALLBACK*)())ShadowMapTesselateEnd);
	gluTessProperty(pTesselator, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

	g_bCreaseEdges = m_bCreaseEdges;

	// Create a list with the required polys so it draws quicker.
	m_iSceneList = glGenLists(2);

	glNewList(m_iSceneList, GL_COMPILE);

	// Overload the render preview viewport as a method for storing our pixels.
	SetRenderPreviewViewport(0, 0, (int)m_iWidth, (int)m_iHeight);

	ShadowMapSetupSceneNode(m_pScene->GetScene(0), pTesselator, true);

	glEndList();

	glNewList(m_iSceneList+1, GL_COMPILE);

	// Overload the render preview viewport as a method for storing our pixels.
	SetRenderPreviewViewport(0, 0, (int)m_iWidth, (int)m_iHeight);

	ShadowMapSetupSceneNode(m_pScene->GetScene(0), pTesselator, false);

	glEndList();

	gluDeleteTess(pTesselator);
}

void CAOGenerator::ShadowMapSetupSceneNode(CConversionSceneNode* pNode, GLUtesselator* pTesselator, bool bDepth)
{
	if (!pNode)
		return;

	for (size_t c = 0; c < pNode->GetNumChildren(); c++)
		ShadowMapSetupSceneNode(pNode->GetChild(c), pTesselator, bDepth);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
	{
		CConversionMeshInstance* pMeshInstance = pNode->GetMeshInstance(m);
		CConversionMesh* pMesh = pMeshInstance->GetMesh();
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);

			if (!bDepth)
			{
				// Allow this in the depth model so that it still projects a shadow, but we don't produce a map for it.
				if (pFace->m != ~0 && pMeshInstance->GetMappedMaterial(pFace->m))
				{
					if (!pMeshInstance->GetMappedMaterial(pFace->m)->IsVisible())
						continue;

					CConversionMaterial* pMaterial = m_pScene->GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial);
					if (pMaterial && !pMaterial->IsVisible())
						continue;
				}
			}

			if (g_bCreaseEdges)
				g_iCreaseFace = f;

			gluTessBeginPolygon(pTesselator, pMeshInstance);
			gluTessBeginContour(pTesselator);

			for (size_t k = 0; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(k);

				Vector vecVertex = pMeshInstance->GetVertex(pVertex->v);
				GLdouble afCoords[3] = { vecVertex.x, vecVertex.y, vecVertex.z };
				gluTessVertex(pTesselator, afCoords, pVertex);
			}

			gluTessEndContour(pTesselator);
			gluTessEndPolygon(pTesselator);
		}
	}
}

extern "C" {
static void CALLBACK RenderTesselateBegin(GLenum ePrim)
{
	glBegin(ePrim);
}

static void CALLBACK RenderTesselateVertex(void* pVertexData, void* pPolygonData)
{
	CConversionMeshInstance* pMeshInstance = (CConversionMeshInstance*)pPolygonData;
	CConversionMesh* pMesh = pMeshInstance->GetMesh();
	CConversionVertex* pVertex = (CConversionVertex*)pVertexData;

	Vector vecVertex = pMeshInstance->GetVertex(pVertex->v);
	Vector vecNormal = pMeshInstance->GetNormal(pVertex->vn);
	Vector vecUV = pMesh->GetUV(pVertex->vu);

	glTexCoord2fv(vecUV);
	glNormal3fv(vecNormal);
	glVertex3fv(vecVertex);
}

static void CALLBACK RenderTesselateEnd()
{
	glEnd();
}
}

void CAOGenerator::RenderSetupScene()
{
	// Tuck away our current stack so we can return to it later.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Background represents light, so it's white.
	glClearColor(1, 1, 1, 1);

	GLUtesselator* pTesselator = gluNewTess();
	gluTessCallback(pTesselator, GLU_TESS_BEGIN, (void(CALLBACK*)())RenderTesselateBegin);
	gluTessCallback(pTesselator, GLU_TESS_VERTEX_DATA, (void(CALLBACK*)())RenderTesselateVertex);
	gluTessCallback(pTesselator, GLU_TESS_END, (void(CALLBACK*)())RenderTesselateEnd);
	gluTessProperty(pTesselator, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

	// Create a list with the required polys so it draws quicker.
	m_iSceneList = glGenLists(1);

	glNewList(m_iSceneList, GL_COMPILE);

	RenderSetupSceneNode(m_pScene->GetScene(0), pTesselator);

	glEndList();

	gluDeleteTess(pTesselator);
}

void CAOGenerator::RenderSetupSceneNode(CConversionSceneNode* pNode, GLUtesselator* pTesselator)
{
	if (!pNode)
		return;

	for (size_t c = 0; c < pNode->GetNumChildren(); c++)
		RenderSetupSceneNode(pNode->GetChild(c), pTesselator);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
	{
		CConversionMeshInstance* pMeshInstance = pNode->GetMeshInstance(m);
		CConversionMesh* pMesh = pMeshInstance->GetMesh();
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);

			glBindTexture(GL_TEXTURE_2D, (GLuint)(*m_paoMaterials)[m].m_iBase);
			glColor3f(1, 1, 1);

			if (pFace->m != ~0 && pMeshInstance->GetMappedMaterial(pFace->m) && m_pScene->GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial))
			{
				CConversionMaterial* pMaterial = m_pScene->GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial);
				glMaterialfv(GL_FRONT, GL_AMBIENT, pMaterial->m_vecAmbient);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, pMaterial->m_vecDiffuse);
				glMaterialfv(GL_FRONT, GL_SPECULAR, pMaterial->m_vecSpecular);
				glMaterialfv(GL_FRONT, GL_EMISSION, pMaterial->m_vecEmissive);
				glMaterialf(GL_FRONT, GL_SHININESS, pMaterial->m_flShininess);
				glColor4fv(pMaterial->m_vecDiffuse);
			}

			gluTessBeginPolygon(pTesselator, pMeshInstance);
			gluTessBeginContour(pTesselator);

			for (size_t k = 0; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(k);

				Vector vecVertex = pMeshInstance->GetVertex(pVertex->v);
				GLdouble afCoords[3] = { vecVertex.x, vecVertex.y, vecVertex.z };
				gluTessVertex(pTesselator, afCoords, pVertex);
			}

			gluTessEndContour(pTesselator);
			gluTessEndPolygon(pTesselator);
		}
	}
}

void CAOGenerator::Generate()
{
	if (!m_eAOMethod)
		return;

	if (m_pWorkListener)
	{
		m_pWorkListener->BeginProgress();
		m_pWorkListener->SetAction(L"Setting up", 0);
	}

	m_bIsGenerating = true;
	m_bStopGenerating = false;
	m_bDoneGenerating = false;
	m_bIsBleeding = false;

	m_flLowestValue = -1;
	m_flHighestValue = 0;

#ifdef _DEBUG
	if (CModelWindow::Get())
		CModelWindow::Get()->ClearDebugLines();
#endif

	memset(&m_bPixelMask[0], 0, m_iWidth*m_iHeight*sizeof(bool));

	if (m_eAOMethod == AOMETHOD_SHADOWMAP)
	{
		if (!GLEW_ARB_shadow || !GLEW_ARB_depth_texture || !GLEW_ARB_vertex_shader || !GLEW_EXT_framebuffer_object || !(GLEW_ARB_texture_float || GLEW_VERSION_3_0))
		{
			m_bIsGenerating = false;
			// Message here?
			return;
		}

		ShadowMapSetupScene();
		GenerateShadowMaps();
	}
	else
	{
		if (m_eAOMethod == AOMETHOD_RAYTRACE)
			RaytraceSetupThreads();

		if (m_eAOMethod == AOMETHOD_RENDER)
			RenderSetupScene();
#ifdef AO_DEBUG
		// In AO debug mode we need this to do the debug rendering, so do it anyways.
		else
			RenderSetupScene();
#endif

		GenerateByTexel();
	}

	size_t i;

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Averaging reads", m_iWidth*m_iHeight);

	// Average out all of the reads.
	for (i = 0; i < m_iWidth*m_iHeight; i++)
	{
		// Don't immediately return, just skip this loop. We have cleanup work to do.
		if (m_bStopGenerating)
			break;

		if (m_eAOMethod == AOMETHOD_TRIDISTANCE)
		{
			if (m_aiShadowReads[i])
			{
				// Scale us so that the lowest read value (most light) is 1 and the highest read value (least light) is 0.
				float flRealShadowValue = RemapVal(m_avecShadowValues[i].x, m_flLowestValue, m_flHighestValue, 1, 0);

				m_avecShadowValues[i] = Vector(flRealShadowValue, flRealShadowValue, flRealShadowValue);
			}
		}
		else if (m_eAOMethod == AOMETHOD_SHADOWMAP)
			m_avecShadowValues[i] = Vector(m_avecShadowValues[i].x, m_avecShadowValues[i].x, m_avecShadowValues[i].x);

		if (m_aiShadowReads[i])
			m_avecShadowValues[i] /= (float)m_aiShadowReads[i];
		else
			m_avecShadowValues[i] = Vector(0,0,0);

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i);
	}

	if (m_eAOMethod == AOMETHOD_RENDER || m_eAOMethod == AOMETHOD_SHADOWMAP)
	{
		if (m_eAOMethod == AOMETHOD_SHADOWMAP)
			glDeleteLists(m_iSceneList, 2);
		else
			glDeleteLists(m_iSceneList, 1);

		// We now return you to our normal render programming. Thank you for your patronage.
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	if (m_eAOMethod == AOMETHOD_RAYTRACE)
		RaytraceCleanupThreads();

	// Somebody get this ao some clotters and morphine, STAT!
	m_bIsBleeding = true;
	if (!m_bStopGenerating)
		Bleed();
	m_bIsBleeding = false;

	if (!m_bStopGenerating)
		m_bDoneGenerating = true;
	m_bIsGenerating = false;

	// One last call to let them know we're done.
	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

void CAOGenerator::GenerateShadowMaps()
{
	int iProcessScene = 0;
	int iProcessSceneRead = 0;
	int iProgress = 0;

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	// Clear red so that we can pick out later what we want when we're reading pixels.
	glClearColor(1, 0, 0, 0);

	// Shading states
	glColor4f(1, 1, 1, 1);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Depth states
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);

	GLsizei iShadowMapSize = 1024;

	GLuint iShadowMap;
	glGenTextures(1, &iShadowMap);
	glBindTexture(GL_TEXTURE_2D, iShadowMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, iShadowMapSize, iShadowMapSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	GLuint iDepthRB;
	glGenRenderbuffersEXT(1, &iDepthRB);
	glBindRenderbufferEXT( GL_RENDERBUFFER, iDepthRB );
	glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_RGBA, iShadowMapSize, iShadowMapSize );
	glBindRenderbufferEXT( GL_RENDERBUFFER, 0 );

	// A frame buffer for holding the depth buffer shadow render
	GLuint iDepthFB;
	glGenFramebuffersEXT(1, &iDepthFB);
	glBindFramebufferEXT(GL_FRAMEBUFFER, iDepthFB);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, iShadowMap, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, iDepthRB);	// Unused
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	GLuint iUVMap;
	glGenTextures(1, &iUVMap);
	glBindTexture(GL_TEXTURE_2D, iUVMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)m_iWidth, (GLsizei)m_iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint iUVRB;
	glGenRenderbuffersEXT(1, &iUVRB);
	glBindRenderbufferEXT( GL_RENDERBUFFER, iUVRB );
	glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)m_iWidth, (GLsizei)m_iHeight );
	glBindRenderbufferEXT( GL_RENDERBUFFER, 0 );

	// A frame buffer for holding the UV layout once it is rendered flat with the shadow
	GLuint iUVFB;
	glGenFramebuffersEXT(1, &iUVFB);
	glBindFramebufferEXT(GL_FRAMEBUFFER, iUVFB);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, iUVMap, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, iUVRB);	// Unused
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	GLuint iAOMap;
	glGenTextures(1, &iAOMap);
	glBindTexture(GL_TEXTURE_2D, iAOMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (GLsizei)m_iWidth, (GLsizei)m_iHeight, 0, GL_BGRA, GL_HALF_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint iAORB;
	glGenRenderbuffersEXT(1, &iAORB);
	glBindRenderbufferEXT( GL_RENDERBUFFER, iAORB );
	glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)m_iWidth, (GLsizei)m_iHeight );
	glBindRenderbufferEXT( GL_RENDERBUFFER, 0 );

	// A frame buffer for holding the completed AO map
	glGenFramebuffersEXT(1, &m_iAOFB);
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_iAOFB);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, iAOMap, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, iAORB);	// Unused
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	GLuint iSMVertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* pszShaderSource = GetVSFlattenedShadowMap();
	glShaderSource(iSMVertexShader, 1, &pszShaderSource, NULL);
	glCompileShader(iSMVertexShader);

#ifdef _DEBUG
	int iLogLength = 0;
	char szLog[1024];
	glGetShaderInfoLog(iSMVertexShader, 1024, &iLogLength, szLog);
#endif

	GLuint iSMFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	pszShaderSource = GetFSFlattenedShadowMap();
	glShaderSource(iSMFragmentShader, 1, &pszShaderSource, NULL);
	glCompileShader(iSMFragmentShader);

#ifdef _DEBUG
	glGetShaderInfoLog(iSMFragmentShader, 1024, &iLogLength, szLog);
#endif

	GLuint iSMProgram = glCreateProgram();
	glAttachShader(iSMProgram, iSMVertexShader);
	glAttachShader(iSMProgram, iSMFragmentShader);
	glLinkProgram(iSMProgram);

#ifdef _DEBUG
	glGetProgramInfoLog(iSMProgram, 1024, &iLogLength, szLog);
#endif

	GLuint iAOVertexShader = glCreateShader(GL_VERTEX_SHADER);
	pszShaderSource = GetVSAOMap();
	glShaderSource(iAOVertexShader, 1, &pszShaderSource, NULL);
	glCompileShader(iAOVertexShader);

#ifdef _DEBUG
	glGetShaderInfoLog(iAOVertexShader, 1024, &iLogLength, szLog);
#endif

	GLuint iAOFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	pszShaderSource = GetFSAOMap();
	glShaderSource(iAOFragmentShader, 1, &pszShaderSource, NULL);
	glCompileShader(iAOFragmentShader);

#ifdef _DEBUG
	glGetShaderInfoLog(iAOFragmentShader, 1024, &iLogLength, szLog);
#endif

	GLuint iAOProgram = glCreateProgram();
	glAttachShader(iAOProgram, iAOVertexShader);
	glAttachShader(iAOProgram, iAOFragmentShader);
	glLinkProgram(iAOProgram);

#ifdef _DEBUG
	glGetProgramInfoLog(iAOProgram, 1024, &iLogLength, szLog);
#endif

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

	Matrix4x4 mLightProjection;
	
	// Use column major because OpenGL does.
	Matrix4x4 mBias(
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.5f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f); // Bias from [-1, 1] to [0, 1]

	AABB oBox = m_pScene->m_oExtends;
	Vector vecCenter = oBox.Center();
	float flSize = oBox.Size().Length();	// Length of the box's diagonal

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-flSize/2, flSize/2, -flSize/2, flSize/2, 1, flSize*2);
	//gluPerspective(45, 1, 1, flSize*2);
	glGetFloatv(GL_PROJECTION_MATRIX, mLightProjection);

	size_t iSamples = (size_t)sqrt((float)m_iSamples);

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Taking exposures", m_iSamples);

	for (size_t x = 0; x <= iSamples; x++)
	{
		float flPitch = RemapVal(cos(RemapVal((float)x, 0, (float)iSamples, -M_PI/2, M_PI/2)), 0, 1, 90, 0);
		if (x > iSamples/2)
			flPitch = -flPitch;

		for (size_t y = 0; y < iSamples; y++)
		{
			if (x == 0 || x == iSamples)
			{
				// Don't do a bunch of samples from the same spot on the poles.
				if (y != 0)
					continue;
			}

			float flYaw = RemapVal((float)y, 0, (float)iSamples, -180, 180);

			Vector vecDir = AngleVector(EAngle(flPitch, flYaw, 0));
			Vector vecLightPosition = vecDir*flSize + vecCenter;	// Puts us twice as far from the closest vertex

#ifdef AO_DEBUG
//			CModelWindow::Get()->AddDebugLine(vecLightPosition, vecLightPosition-vecDir);
#endif

			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(mLightProjection);

			Matrix4x4 mLightView;

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(
				vecLightPosition.x, vecLightPosition.y, vecLightPosition.z,
				vecCenter.x, vecCenter.y, vecCenter.z,
				0, 1, 0);
			glGetFloatv(GL_MODELVIEW_MATRIX, mLightView);

			// If we're looking from below and ground occlusion is on, don't bother with this render.
			if (!(flPitch < -10 && m_bGroundOcclusion))
			{
				glBindFramebufferEXT(GL_FRAMEBUFFER, iDepthFB);
				glViewport(0, 0, iShadowMapSize, iShadowMapSize);

				glDisable(GL_CULL_FACE);

				glClear(GL_DEPTH_BUFFER_BIT);

				glCallList(m_iSceneList);

				glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

#ifdef AO_DEBUG
				glDrawBuffer(GL_FRONT);
				glReadBuffer(GL_FRONT);
				DrawTexture(iShadowMap, 0.6f);
				glFinish();
#endif
			}

			// OpenGL matrices are column major, so multiply in the wrong order to get the right result.
			Matrix4x4 m1 = mLightProjection*mBias;
			Matrix4x4 mTextureMatrix = mLightView*m1;

			// We're storing the resulting projection in GL_TEXTURE7 for later reference by the shader.
			glBindFramebufferEXT(GL_FRAMEBUFFER, iUVFB);

			glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);

			glClear(GL_COLOR_BUFFER_BIT);

			glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

			glUseProgram(iSMProgram);

			GLuint iLightNormalUniform = glGetUniformLocation(iSMProgram, "vecLightNormal");
			GLuint iShadowMapUniform = glGetUniformLocation(iSMProgram, "iShadowMap");
			GLuint ibOccludeAllUniform = glGetUniformLocation(iSMProgram, "bOccludeAll");

			glMatrixMode(GL_TEXTURE);
			glActiveTexture(GL_TEXTURE7);
			glPushMatrix();
			glLoadIdentity();
			glLoadMatrixf(mTextureMatrix);

			glUniform1i(iShadowMapUniform, 7);
			glUniform3fv(iLightNormalUniform, 1, -vecDir);
			glUniform1i(ibOccludeAllUniform, (flPitch < -10 && m_bGroundOcclusion));

			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, iShadowMap);
			glEnable(GL_TEXTURE_2D);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(
				5, 5, 10,
				0, 0, 0,
				0, 1, 0);

			glDisable(GL_DEPTH_TEST);

			glCallList(m_iSceneList+1);

			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE0);

			glMatrixMode(GL_TEXTURE);
			glPopMatrix();

			glUseProgram(0);

			glPopAttrib();

			glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

#ifdef AO_DEBUG
			glDrawBuffer(GL_FRONT);
			glReadBuffer(GL_FRONT);
			DrawTexture(iUVMap);
			glFinish();
#endif

			int iTimeBefore = glutGet(GLUT_ELAPSED_TIME);

			glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
			glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_iAOFB);
			AccumulateTexture(iUVMap);
			glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

#ifdef AO_DEBUG
			glDrawBuffer(GL_FRONT);
			glReadBuffer(GL_FRONT);
			glViewport((GLint)m_iWidth, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
			glUseProgram(iAOProgram);
			GLuint iAOMapUniform = glGetUniformLocation(iAOProgram, "iAOMap");
			glUniform1i(iAOMapUniform, 0);
			DrawTexture(iAOMap);
			glUseProgram(0);
			glFinish();
#endif

			iProcessSceneRead += (glutGet(GLUT_ELAPSED_TIME) - iTimeBefore);
			iTimeBefore = glutGet(GLUT_ELAPSED_TIME);

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(x*iSamples + y);

			iProgress += (glutGet(GLUT_ELAPSED_TIME) - iTimeBefore);

			if (m_bStopGenerating)
				break;
		}

		if (m_bStopGenerating)
			break;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_iAOFB);
	glReadPixels(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight, GL_RGBA, GL_FLOAT, m_pPixels);
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	if (!m_bStopGenerating)
	{
		size_t iBufferSize = m_iWidth*m_iHeight*m_iPixelDepth;

		if (m_pWorkListener)
			m_pWorkListener->SetAction(L"Reading pixels", iBufferSize/m_iPixelDepth);

		for (size_t p = 0; p < iBufferSize; p+=m_iPixelDepth)
		{
			if (m_pPixels[p+3] == 0.0f)
				continue;

			size_t i = p/m_iPixelDepth;

			m_avecShadowValues[i].x = m_pPixels[p+0];
			m_aiShadowReads[i] = (size_t)m_pPixels[p+3];
			m_bPixelMask[i] = true;

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(p/m_iPixelDepth);
		}
	}

	glCullFace(GL_BACK);
    glShadeModel(GL_SMOOTH);
    glColorMask(1, 1, 1, 1);

	glDetachShader(iSMProgram, iSMVertexShader);
	glDetachShader(iSMProgram, iSMFragmentShader);
	glDeleteProgram(iSMProgram);
	glDeleteShader(iSMVertexShader);
	glDeleteShader(iSMFragmentShader);

	glDetachShader(iAOProgram, iAOVertexShader);
	glDetachShader(iAOProgram, iAOFragmentShader);
	glDeleteProgram(iAOProgram);
	glDeleteShader(iAOVertexShader);
	glDeleteShader(iAOFragmentShader);

	glDeleteTextures(1, &iShadowMap);
	glDeleteTextures(1, &iUVMap);
	glDeleteTextures(1, &iAOMap);

	glDeleteRenderbuffersEXT(1, &iDepthRB);
	glDeleteRenderbuffersEXT(1, &iUVRB);
	glDeleteRenderbuffersEXT(1, &iAORB);

	glDeleteFramebuffersEXT(1, &iDepthFB);
	glDeleteFramebuffersEXT(1, &iUVFB);
	glDeleteFramebuffersEXT(1, &m_iAOFB);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glDepthFunc(GL_LESS);

	glPopAttrib();
}

void CAOGenerator::AccumulateTexture(size_t iTexture)
{
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(-1.0f, -1.0f);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(-1.0f, 1.0f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(1.0f, 1.0f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(1.0f, -1.0f);
	glEnd();

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
}

void CAOGenerator::GenerateByTexel()
{
	float flTotalArea = 0;

	for (size_t m = 0; m < m_pScene->GetNumMeshes(); m++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m);
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);
			flTotalArea += pFace->GetUVArea();
		}
	}

	raytrace::CRaytracer* pTracer = NULL;

	if (m_eAOMethod == AOMETHOD_RAYTRACE)
	{
		if (m_pWorkListener)
			m_pWorkListener->SetAction(L"Building tree", 0);

		pTracer = new raytrace::CRaytracer(m_pScene);
		pTracer->AddMeshesFromNode(m_pScene->GetScene(0));
		pTracer->BuildTree();

		srand((unsigned int)time(0));
	}

	if (m_pWorkListener)
	{
		if (m_eAOMethod == AOMETHOD_RAYTRACE && GetNumberOfProcessors() > 1)
			m_pWorkListener->SetAction(L"Dispatching jobs", (size_t)(flTotalArea*m_iWidth*m_iHeight));
		else
			m_pWorkListener->SetAction(L"Rendering", (size_t)(flTotalArea*m_iWidth*m_iHeight));
	}

	size_t iRendered = 0;

	if (m_pScene->GetNumScenes())
		GenerateNodeByTexel(m_pScene->GetScene(0), pTracer, iRendered);

	if (m_eAOMethod == AOMETHOD_RAYTRACE)
		RaytraceJoinThreads();

	if (m_eAOMethod == AOMETHOD_RAYTRACE)
		delete pTracer;
}

void CAOGenerator::GenerateNodeByTexel(CConversionSceneNode* pNode, raytrace::CRaytracer* pTracer, size_t& iRendered)
{
	for (size_t c = 0; c < pNode->GetNumChildren(); c++)
		GenerateNodeByTexel(pNode->GetChild(c), pTracer, iRendered);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
	{
		CConversionMeshInstance* pMeshInstance = pNode->GetMeshInstance(m);
		CConversionMesh* pMesh = pMeshInstance->GetMesh();

		if (!pMesh->GetNumUVs())
			continue;

		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);

			if (pFace->m != ~0)
			{
				if (!pMeshInstance->GetMappedMaterial(pFace->m)->IsVisible())
					continue;

				CConversionMaterial* pMaterial = m_pScene->GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial);
				if (pMaterial && !pMaterial->IsVisible())
					continue;
			}

			std::vector<Vector> avecPoints;
			std::vector<size_t> aiPoints;
			for (size_t t = 0; t < pFace->GetNumVertices(); t++)
			{
				avecPoints.push_back(pMeshInstance->GetVertex(pFace->GetVertex(t)->v));
				aiPoints.push_back(t);
			}

			while (avecPoints.size() > 3)
			{
				size_t iEar = FindEar(avecPoints);
				size_t iLast = iEar==0?avecPoints.size()-1:iEar-1;
				size_t iNext = iEar==avecPoints.size()-1?0:iEar+1;
				GenerateTriangleByTexel(pMeshInstance, pFace, aiPoints[iLast], aiPoints[iEar], aiPoints[iNext], pTracer, iRendered);
				avecPoints.erase(avecPoints.begin()+iEar);
				aiPoints.erase(aiPoints.begin()+iEar);
				if (m_bStopGenerating)
					break;
			}
			GenerateTriangleByTexel(pMeshInstance, pFace, aiPoints[0], aiPoints[1], aiPoints[2], pTracer, iRendered);
			if (m_bStopGenerating)
				break;
		}
		if (m_bStopGenerating)
			break;
	}
}

void CAOGenerator::GenerateTriangleByTexel(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t v1, size_t v2, size_t v3, raytrace::CRaytracer* pTracer, size_t& iRendered)
{
	CConversionVertex* pV1 = pFace->GetVertex(v1);
	CConversionVertex* pV2 = pFace->GetVertex(v2);
	CConversionVertex* pV3 = pFace->GetVertex(v3);

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	Vector vu1 = pMesh->GetUV(pV1->vu);
	Vector vu2 = pMesh->GetUV(pV2->vu);
	Vector vu3 = pMesh->GetUV(pV3->vu);

	Vector vecLoUV = vu1;
	Vector vecHiUV = vu1;

	if (vu2.x < vecLoUV.x)
		vecLoUV.x = vu2.x;
	if (vu3.x < vecLoUV.x)
		vecLoUV.x = vu3.x;
	if (vu2.x > vecHiUV.x)
		vecHiUV.x = vu2.x;
	if (vu3.x > vecHiUV.x)
		vecHiUV.x = vu3.x;

	if (vu2.y < vecLoUV.y)
		vecLoUV.y = vu2.y;
	if (vu3.y < vecLoUV.y)
		vecLoUV.y = vu3.y;
	if (vu2.y > vecHiUV.y)
		vecHiUV.y = vu2.y;
	if (vu3.y > vecHiUV.y)
		vecHiUV.y = vu3.y;

	size_t iLoX = (size_t)(vecLoUV.x * m_iWidth);
	size_t iLoY = (size_t)(vecLoUV.y * m_iHeight);
	size_t iHiX = (size_t)(vecHiUV.x * m_iWidth);
	size_t iHiY = (size_t)(vecHiUV.y * m_iHeight);

	for (size_t i = iLoX; i <= iHiX; i++)
	{
		for (size_t j = iLoY; j <= iHiY; j++)
		{
			float flU = ((float)i + 0.5f)/(float)m_iWidth;
			float flV = ((float)j + 0.5f)/(float)m_iHeight;

			bool bInside = PointInTriangle(Vector(flU,flV,0), vu1, vu2, vu3);

			if (!bInside)
				continue;

			Vector v1 = pMeshInstance->GetVertex(pV1->v);
			Vector v2 = pMeshInstance->GetVertex(pV2->v);
			Vector v3 = pMeshInstance->GetVertex(pV3->v);

			Vector vn1 = pMeshInstance->GetNormal(pV1->vn);
			Vector vn2 = pMeshInstance->GetNormal(pV2->vn);
			Vector vn3 = pMeshInstance->GetNormal(pV3->vn);

			// Find where the UV is in world space.

			// First build 2x2 a "matrix" of the UV values.
			float mta = vu2.x - vu1.x;
			float mtb = vu3.x - vu1.x;
			float mtc = vu2.y - vu1.y;
			float mtd = vu3.y - vu1.y;

			// Invert it.
			float d = mta*mtd - mtb*mtc;
			float mtia =  mtd / d;
			float mtib = -mtb / d;
			float mtic = -mtc / d;
			float mtid =  mta / d;

			// Now build a 2x3 "matrix" of the vertices.
			float mva = v2.x - v1.x;
			float mvb = v3.x - v1.x;
			float mvc = v2.y - v1.y;
			float mvd = v3.y - v1.y;
			float mve = v2.z - v1.z;
			float mvf = v3.z - v1.z;

			// Multiply them together.
			// [a b]   [a b]   [a b]
			// [c d] * [c d] = [c d]
			// [e f]           [e f]
			// Really wish I had a matrix math library about now!
			float mra = mva*mtia + mvb*mtic;
			float mrb = mva*mtib + mvb*mtid;
			float mrc = mvc*mtia + mvd*mtic;
			float mrd = mvc*mtib + mvd*mtid;
			float mre = mve*mtia + mvf*mtic;
			float mrf = mve*mtib + mvf*mtid;

			// These vectors should be the U and V axis in world space.
			Vector vecUAxis(mra, mrc, mre);
			Vector vecVAxis(mrb, mrd, mrf);

			Vector vecUVOrigin = v1 - vecUAxis * vu1.x - vecVAxis * vu1.y;

			Vector vecUVPosition = vecUVOrigin + vecUAxis * flU + vecVAxis * flV;

			Vector vecNormal;

			if (m_bCreaseEdges)
				vecNormal = pFace->GetNormal();
			else
			{
				float wv1 = DistanceToLine(vecUVPosition, v2, v3) / DistanceToLine(v1, v2, v3);
				float wv2 = DistanceToLine(vecUVPosition, v1, v3) / DistanceToLine(v2, v1, v3);
				float wv3 = DistanceToLine(vecUVPosition, v1, v2) / DistanceToLine(v3, v1, v2);

				vecNormal = vn1 * wv1 + vn2 * wv2 + vn3 * wv3;
			}

#ifdef AO_DEBUG
			//						CModelWindow::Get()->AddDebugLine(vecUVPosition, vecUVPosition + vecNormal/2);
#endif

			size_t iTexel;
			Texel(i, j, iTexel, false);

			if (m_eAOMethod == AOMETHOD_RENDER)
			{
				// Render the scene from this location
				m_avecShadowValues[iTexel] += RenderSceneFromPosition(vecUVPosition, vecNormal, pFace);
			}
			else if (m_eAOMethod == AOMETHOD_RAYTRACE)
			{
				RaytraceSceneMultithreaded(pTracer, vecUVPosition, vecNormal, pMeshInstance, pFace, iTexel);
			}
			else
			{
				float flShadowValue = 0;

				//DebugRenderSceneLookAtPosition(vecUVPosition, vecNormal, pFace);

				// Tri distance method
				for (size_t m2 = 0; m2 < m_pScene->GetNumMeshes(); m2++)
				{
					CConversionMesh* pMesh2 = m_pScene->GetMesh(m2);
					for (size_t f2 = 0; f2 < pMesh2->GetNumFaces(); f2++)
					{
						CConversionFace* pFace2 = pMesh2->GetFace(f2);

						if (pFace == pFace2)
							continue;

						//DebugRenderSceneLookAtPosition(pFace2->GetCenter(), pFace2->GetNormal(), pFace2);

						// If this face is behind us, ignore.
						if ((pFace2->GetCenter() - vecUVPosition).Normalized().Dot(vecNormal) <= 0)
							continue;

						// Skip adjoining faces, they are done in a different algorithm below.
						bool bFoundAdjacent = false;
						for (size_t e = 0; e < pFace->GetNumEdges(); e++)
						{
							CConversionEdge* pEdge = pMesh->GetEdge(pFace->GetEdge(e));

							for (size_t ef = 0; ef < pEdge->m_aiFaces.size(); ef++)
							{
								if (pEdge->m_aiFaces[ef] != ~0 && pMesh->GetFace(pEdge->m_aiFaces[ef]) == pFace2)
								{
									bFoundAdjacent = true;
									break;
								}
							}
						}

						if (bFoundAdjacent)
							continue;

						std::vector<Vector> av;

						float flDistance = DistanceToPolygon(vecUVPosition, pFace2->GetVertices(av), pFace2->GetNormal());
						float flDistanceToCenter = (vecUVPosition - pFace2->GetCenter()).Length();

						// If the difference between the closest distance to the poly and the difference to the center is large
						// then the poly is very close but we're near the edge of it, so that polygon shouldn't affect the
						// lighting as much. If it's across from us then this should result in a higher multiplier.
						float flDistanceMultiplier = exp(-fabs(flDistance - flDistanceToCenter));

						flDistanceMultiplier = (flDistanceMultiplier+1)/2;	// Reduce this effect by half.

						float flArea = sqrt(pFace2->GetArea());

						flShadowValue += flArea * exp(-flDistance) * flDistanceMultiplier * fabs(pFace2->GetNormal().Dot(vecNormal));
					}
				}

				// Loop through all the edges to give us dirty concave corners.
				float flDistanceToOpposite = 0;
				for (size_t e = 0; e < pFace->GetNumEdges(); e++)
				{
					CConversionEdge* pEdge = pMesh->GetEdge(pFace->GetEdge(e));
					CConversionEdge* pAdjacentEdge = NULL;

					bool bAdjacentEdge = false;
					for (size_t ef = 0; ef < pEdge->m_aiFaces.size(); ef++)
					{
						if (pMesh->GetFace(pEdge->m_aiFaces[ef]) == pFace)
						{
							bAdjacentEdge = true;
							break;
						}
					}

					if (bAdjacentEdge)
					{
						pAdjacentEdge = pEdge;

						if (pFace->GetNumEdges() % 2 == 0)
						{
							// Even number of edges, opposite is an edge.
							size_t iOpposite = (e + pFace->GetNumEdges()/2) % pFace->GetNumEdges();
							CConversionEdge* pOppositeEdge = pMesh->GetEdge(pFace->GetEdge(iOpposite));

							// Use the center of the opposite edge for simplicity's sake.
							Vector vecCenter = (pMesh->GetVertex(pOppositeEdge->v1) + pMesh->GetVertex(pOppositeEdge->v2))/2;
							flDistanceToOpposite = DistanceToLine(vecCenter, pMesh->GetVertex(pEdge->v1), pMesh->GetVertex(pEdge->v2));
						}
						else
						{
							// Odd number of edges, opposite is an point.
							size_t iOpposite = (e + pFace->GetNumVertices()/2) % pFace->GetNumVertices();

							Vector vecOppositePoint = pMesh->GetVertex(pFace->GetVertex(iOpposite)->v);
							flDistanceToOpposite = DistanceToLine(vecOppositePoint, pMesh->GetVertex(pEdge->v1), pMesh->GetVertex(pEdge->v2));
						}
					}

					if (pAdjacentEdge)
					{
						CConversionFace* pOtherFace = NULL;

						for (size_t ef = 0; ef < pAdjacentEdge->m_aiFaces.size(); ef++)
						{
							CConversionFace* pPossibleFace = NULL;

							if (pMesh->GetFace(pAdjacentEdge->m_aiFaces[ef]) == pFace)
								pPossibleFace = pMesh->GetFace(pAdjacentEdge->m_aiFaces[ef]);
							else
								continue;

							// If this face is behind us, ignore.
							if ((pPossibleFace->GetCenter() - vecUVPosition).Normalized().Dot(vecNormal) <= 0)
								continue;

							pOtherFace = pPossibleFace;
						}

						if (!pOtherFace)
							continue;

						float flDot = pOtherFace->GetNormal().Dot(vecNormal);

						if (flDot == 0)
							continue;

						Vector v1 = pMesh->GetVertex(pAdjacentEdge->v1);
						Vector v2 = pMesh->GetVertex(pAdjacentEdge->v2);

						float flDistanceToEdge = DistanceToLine(vecUVPosition, v1, v2);
						float flAngleMultiplier = RemapVal(flDot, 1.0f, -1.0f, 0.0f, 1.0f);
						float flDistanceMultiplier = RemapValClamped(flDistanceToEdge, 0, flDistanceToOpposite, 1, 0);

						flShadowValue += exp(-flDistanceToEdge) * flDistanceMultiplier * flAngleMultiplier * 2;
					}
				}

				m_avecShadowValues[iTexel] += Vector(flShadowValue, flShadowValue, flShadowValue);

				if (flShadowValue < m_flLowestValue || m_flLowestValue == -1)
					m_flLowestValue = flShadowValue;

				if (flShadowValue > m_flHighestValue)
					m_flHighestValue = flShadowValue;
			}

			m_aiShadowReads[iTexel]++;
			m_bPixelMask[iTexel] = true;

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(++iRendered);

			if (m_bStopGenerating)
				break;
		}
		if (m_bStopGenerating)
			break;
	}
}

Vector CAOGenerator::RenderSceneFromPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pRenderFace)
{
	GLenum eBuffer = GL_AUX0;
#ifdef AO_DEBUG
	eBuffer = GL_FRONT;
#endif

	if (m_bUseFrontBuffer)
		eBuffer = GL_FRONT;

	glDrawBuffer(eBuffer);

	glDisable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);

	// Bring it away from its poly so that the camera never clips around behind it.
	// Adds .001 because of a bug where GL for some reason won't show the faces unless I do that.
	Vector vecEye = vecPosition + (vecDirection + Vector(.001f, .001f, .001f)) * 0.1f;
	Vector vecLookAt = vecPosition + vecDirection;

	glViewport(m_iRPVX, m_iRPVY, m_iRPVW, m_iRPVH);

	// Set up some rendering stuff.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
			120.0,
			1,
			0.01,
			100.0
		);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(
		vecEye.x, vecEye.y, vecEye.z,
		vecLookAt.x, vecLookAt.y, vecLookAt.z,
		0.0, 1.0, 0.0);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// It uses this color if the texture is missing.
	GLfloat flMaterialColor[] = {0.0, 0.0, 0.0, 1.0};

	glMaterialfv(GL_FRONT, GL_DIFFUSE, flMaterialColor);
	glColor4fv(flMaterialColor);

	glCallList(m_iSceneList);

	glFinish();

#ifdef AO_DEBUG
	DebugRenderSceneLookAtPosition(vecPosition, vecDirection, pRenderFace);
	glFinish();
#endif

	Vector vecShadowColor(0,0,0);

	glReadBuffer(eBuffer);

	glReadPixels(m_iRPVX, m_iRPVY, m_iRPVW, m_iRPVH, GL_RGBA, GL_FLOAT, m_pPixels);

	float flTotal = 0;

	for (size_t p = 0; p < m_iRPVW*m_iRPVH*m_iPixelDepth; p+=m_iPixelDepth)
	{
		float flColumn = fmod((float)p / (float)m_iPixelDepth, (float)m_iRPVW);

		Vector vecUV(flColumn / m_iRPVW, (float)(p / m_iPixelDepth / m_iRPVW) / m_iRPVH, 0);
		Vector vecUVCenter(0.5f, 0.5f, 0);

		// Weight the pixel based on its distance to the center.
		// With the huge FOV that we work with, polygons to the
		// outside are huge on the screen.
		float flWeight = (0.5f-(vecUV - vecUVCenter).Length())*2.0f;
		if (flWeight <= 0.1)
			continue;

		// Pixels in the center of the screen are much, much more important.
		flWeight = SLerp(flWeight, 0.2f);

		Vector vecPixel(m_pPixels[p+0], m_pPixels[p+1], m_pPixels[p+2]);

		vecShadowColor += vecPixel * flWeight;
		flTotal += flWeight;
	}

	vecShadowColor /= flTotal;

	return vecShadowColor;
}

void CAOGenerator::DebugRenderSceneLookAtPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pRenderFace)
{
#ifdef AO_DEBUG
	glDrawBuffer(GL_FRONT);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);

	Vector vecLookAt = vecPosition;
	Vector vecEye = vecPosition + pRenderFace->GetNormal()*10;
	vecEye.y += 2;
	vecEye.x += 2;

	glViewport(0, 100, 512, 512);

	// Set up some rendering stuff.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
			44.0,
			1,
			1,
			100.0
		);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(
		vecEye.x, vecEye.y, vecEye.z,
		vecLookAt.x, vecLookAt.y, vecLookAt.z,
		0.0, 1.0, 0.0);

	// It uses this color if the texture is missing.
	GLfloat flMaterialColor[] = {0.0, 0.0, 0.0, 1.0};

	glMaterialfv(GL_FRONT, GL_DIFFUSE, flMaterialColor);
	glColor4fv(flMaterialColor);

	glCallList(m_iSceneList);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_POLYGON);
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor4f(0, 1, 0, 0.5f);
		for (size_t p = 0; p < pRenderFace->GetNumVertices(); p++)
			glVertex3fv(pRenderFace->m_pScene->GetMesh(pRenderFace->m_iMesh)->GetVertex(pRenderFace->GetVertex(p)->v) + pRenderFace->GetNormal()*0.01f);
	glEnd();

	glDisable(GL_BLEND);

	Vector vecNormalEnd = vecPosition + vecDirection;
	glBindTexture(GL_TEXTURE_2D, 0);
	glLineWidth(2);
	glBegin(GL_LINES);
		glColor4f(1, 1, 1, 1);
		glVertex3f(vecPosition.x, vecPosition.y, vecPosition.z);
		glVertex3f(vecNormalEnd.x, vecNormalEnd.y, vecNormalEnd.z);
	glEnd();

	glFinish();

	glViewport(m_iRPVX, m_iRPVY, m_iRPVW, m_iRPVH);
#endif
}

void CAOGenerator::Bleed()
{
	bool* abPixelMask = (bool*)malloc(m_iWidth*m_iHeight*sizeof(bool));

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Bleeding edges", m_iBleed);

	for (size_t i = 0; i < m_iBleed; i++)
	{
		// This is for pixels that have been set this frame.
		memset(&abPixelMask[0], 0, m_iWidth*m_iHeight*sizeof(bool));

		for (size_t w = 0; w < m_iWidth; w++)
		{
			for (size_t h = 0; h < m_iHeight; h++)
			{
				Vector vecTotal(0,0,0);
				size_t iTotal = 0;
				size_t iTexel;

				// If the texel has the mask on then it already has a value so skip it.
				if (Texel(w, h, iTexel, true))
					continue;

				if (Texel(w-1, h-1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w-1, h, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w-1, h+1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w, h+1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w+1, h+1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w+1, h, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w+1, h-1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				if (Texel(w, h-1, iTexel))
				{
					vecTotal += m_avecShadowValues[iTexel];
					iTotal++;
				}

				Texel(w, h, iTexel, false);

				if (iTotal)
				{
					vecTotal /= (float)iTotal;
					m_avecShadowValues[iTexel] = vecTotal;
					abPixelMask[iTexel] = true;
				}
			}
		}

		for (size_t p = 0; p < m_iWidth*m_iHeight; p++)
			m_bPixelMask[p] |= abPixelMask[p];

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(i);

		if (m_bStopGenerating)
			break;
	}

	free(abPixelMask);
}

size_t CAOGenerator::GenerateTexture(bool bInMedias)
{
	Vector* avecShadowValues = m_avecShadowValues;

	if (bInMedias)
	{
		// Use this temporary buffer so we don't clobber the original.
		avecShadowValues = m_avecShadowGeneratedValues;

		if (m_eAOMethod == AOMETHOD_SHADOWMAP) // If bleeding, we have reads already so do it the old fasioned way so it actually shows the bleeds expanding.
		{
			if (m_bIsBleeding)
			{
				for (size_t i = 0; i < m_iWidth*m_iHeight; i++)
				{
					// Don't immediately return, just skip this loop. We have cleanup work to do.
					if (m_bStopGenerating)
						break;

					avecShadowValues[i] = m_avecShadowValues[i];
				}
			}
			else
			{
				glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_iAOFB);
				glReadPixels(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight, GL_RGBA, GL_FLOAT, m_pPixels);
				glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

				size_t iBufferSize = m_iWidth*m_iHeight*m_iPixelDepth;
				for (size_t p = 0; p < iBufferSize; p+=m_iPixelDepth)
				{
					size_t i = p/m_iPixelDepth;

					if (m_pPixels[p+3] == 0.0f)
						avecShadowValues[i].x = 0;
					else
						avecShadowValues[i].x = m_pPixels[p+0]/m_pPixels[p+3];

					avecShadowValues[i].y = avecShadowValues[i].z = avecShadowValues[i].x;
				}
			}
		}
		else
		{
			// Average out all of the reads.
			for (size_t i = 0; i < m_iWidth*m_iHeight; i++)
			{
				// Don't immediately return, just skip this loop. We have cleanup work to do.
				if (m_bStopGenerating)
					break;

				if (!m_aiShadowReads[i])
				{
					avecShadowValues[i] = Vector(0,0,0);
					continue;
				}

				if (m_eAOMethod == AOMETHOD_TRIDISTANCE)
				{
					if (m_aiShadowReads[i])
					{
						// Scale us so that the lowest read value (most light) is 1 and the highest read value (least light) is 0.
						float flRealShadowValue = RemapVal(m_avecShadowValues[i].x, m_flLowestValue, m_flHighestValue, 1, 0) / m_aiShadowReads[i];

						avecShadowValues[i] = Vector(flRealShadowValue, flRealShadowValue, flRealShadowValue);
					}
				}
				else
					avecShadowValues[i] = m_avecShadowValues[i] / (float)m_aiShadowReads[i];

				// When exporting to png sometimes a pure white value will suffer integer overflow.
				avecShadowValues[i] *= 0.99f;
			}
		}
	}

	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, (GLint)m_iWidth, (GLint)m_iHeight, GL_RGB, GL_FLOAT, &avecShadowValues[0].x);

	return iGLId;
}

void CAOGenerator::SaveToFile(const wchar_t *pszFilename)
{
	if (!pszFilename)
		return;

	ilEnable(IL_FILE_OVERWRITE);

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	// WHAT A HACK!
	ilTexImage((ILint)m_iWidth, (ILint)m_iHeight, 1, 3, IL_RGB, IL_FLOAT, NULL);

	if (m_eAOMethod != AOMETHOD_SHADOWMAP)
	{
		for (size_t i = 0; i < m_iWidth*m_iHeight; i++)
			// When exporting to png sometimes a pure white value will suffer integer overflow.
			m_avecShadowValues[i] *= 0.99f;
	}

	ilSetData(&m_avecShadowValues[0].x);

	// Formats like PNG and VTF don't work unless it's in integer format.
	ilConvertImage(IL_RGB, IL_UNSIGNED_INT);

	if (!CModelWindow::GetSMAKTexture() && (m_iWidth > 128 || m_iHeight > 128))
	{
		iluImageParameter(ILU_FILTER, ILU_BILINEAR);
		iluScale(128, 128, 1);
	}

	ilSaveImage(pszFilename);

	ilDeleteImages(1,&iDevILId);
}

bool CAOGenerator::Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask)
{
	if (w < 0 || h < 0 || w >= m_iWidth || h >= m_iHeight)
		return false;

	iTexel = m_iHeight*h + w;

	assert(iTexel >= 0 && iTexel < m_iWidth * m_iHeight);

	if (bUseMask && !m_bPixelMask[iTexel])
		return false;

	return true;
}

#ifdef _DEBUG
void DrawSplit(const raytrace::CKDNode* pNode)
{
	// No children, no split.
	if (!pNode->GetLeftChild())
		return;

	AABB oBox = pNode->GetBounds();
	size_t iSplitAxis = pNode->GetSplitAxis();
	float flSplitPos = pNode->GetSplitPos();

	// Construct four points that are the corners or a rectangle representing this portal split.
	Vector v0 = oBox.m_vecMins;
	v0[iSplitAxis] = flSplitPos;
	Vector v1 = v0;
	v1[(iSplitAxis+1)%3] = oBox.m_vecMaxs[(iSplitAxis+1)%3];
	Vector v2 = v0;
	v2[(iSplitAxis+1)%3] = oBox.m_vecMaxs[(iSplitAxis+1)%3];
	v2[(iSplitAxis+2)%3] = oBox.m_vecMaxs[(iSplitAxis+2)%3];
	Vector v3 = v0;
	v3[(iSplitAxis+2)%3] = oBox.m_vecMaxs[(iSplitAxis+2)%3];

	CModelWindow::Get()->AddDebugLine(v0, v1);
	CModelWindow::Get()->AddDebugLine(v1, v2);
	CModelWindow::Get()->AddDebugLine(v2, v3);
	CModelWindow::Get()->AddDebugLine(v3, v0);

	if (pNode->GetLeftChild())
		DrawSplit(pNode->GetLeftChild());
	if (pNode->GetRightChild())
		DrawSplit(pNode->GetRightChild());
}

void DrawTexture(GLuint iTexture, float flScale)
{
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glShadeModel(GL_SMOOTH);

	glBindTexture(GL_TEXTURE_2D, iTexture);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(-flScale, -flScale);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(-flScale, flScale);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(flScale, flScale);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(flScale, -flScale);
	glEnd();

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
}
#endif
