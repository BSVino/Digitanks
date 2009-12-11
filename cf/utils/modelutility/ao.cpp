#include "crunch.h"

#include <assert.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include <geometry.h>
#include <maths.h>
#include <matrix.h>
#include <raytracer/raytracer.h>
#include "shaders/shaders.h"

#if 0
#ifdef _DEBUG
#define AO_DEBUG
#endif
#endif

#ifdef AO_DEBUG
#include "ui/modelwindow.h"

void DrawTexture(GLuint iTexture);
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
	SetRenderPreviewViewport(0, 0, 100, 100);
	SetUseFrontBuffer(false);

	m_pWorkListener = NULL;

	m_bIsGenerating = false;
	m_bDoneGenerating = false;
	m_bStopGenerating = false;
}

CAOGenerator::~CAOGenerator()
{
	free(m_pPixels);
	free(m_bPixelMask);
	delete[] m_avecShadowValues;
	delete[] m_avecShadowGeneratedValues;
	delete[] m_aiShadowReads;
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
	m_iPixelDepth = 3;
	size_t iBufferSize = m_iRPVW*m_iRPVH*sizeof(GLfloat)*m_iPixelDepth;
	m_pPixels = (GLfloat*)malloc(iBufferSize);
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

	// Create a list with the required polys so it draws quicker.
	m_iSceneList = glGenLists(1);

	glNewList(m_iSceneList, GL_COMPILE);

	// Overload the render preview viewport as a method for storing our pixels.
	SetRenderPreviewViewport(0, 0, (int)m_iWidth, (int)m_iHeight);

	for (size_t m = 0; m < m_pScene->GetNumMeshes(); m++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m);
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);

			glBegin(GL_POLYGON);

			for (size_t k = 0; k < pFace->GetNumVertices(); k++)
			{
				// Translate here so it takes up the whole viewport when flattened by the shader.
				Vector vecUV = pMesh->GetUV(pFace->GetVertex(k)->vt) * 2 - Vector(1,1,1);
				vecUV.y = -vecUV.y;
				glTexCoord2fv(vecUV);
				glNormal3fv(pMesh->GetNormal(pFace->GetVertex(k)->vn));
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k)->v));
			}

			glEnd();
		}
	}

	glEndList();
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

	size_t m;

	// Create a list with the required polys so it draws quicker.
	m_iSceneList = glGenLists((GLsizei)m_paoMaterials->size()+2);

	glNewList(m_iSceneList, GL_COMPILE);
	// Draw the back faces separately so they can be batched.
	glBegin(GL_TRIANGLES);
	for (m = 0; m < m_pScene->GetNumMeshes(); m++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m);
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);

			for (size_t k = 0; k < pFace->GetNumVertices()-2; k++)
			{
				// Wind backwards so they face the other way.
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(0)->v) - pFace->GetNormal() * 0.01f);
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k+2)->v) - pFace->GetNormal() * 0.01f);
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k+1)->v) - pFace->GetNormal() * 0.01f);
			}
		}
	}
	glEnd();
	glEndList();

	glNewList(m_iSceneList+1, GL_COMPILE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(0, 0, 0);

	for (m = 0; m < m_pScene->GetNumMeshes(); m++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m);
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);

			if (pFace->m != ~0)
				continue;

			glBegin(GL_TRIANGLE_FAN);

			for (size_t k = 0; k < pFace->GetNumVertices(); k++)
				glVertex3fv(pMesh->GetVertex(pFace->GetVertex(k)->v));

			glEnd();
		}
	}

	glEndList();

	// Put the all materials in separate lists so that they can be batched too.
	for (m = 0; m < m_pScene->GetNumMaterials(); m++)
	{
		glNewList(m_iSceneList+m+2, GL_COMPILE);

		glBindTexture(GL_TEXTURE_2D, (GLuint)(*m_paoMaterials)[m].m_iBase);
		glColor3f(1, 1, 1);

		for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
		{
			CConversionMesh* pMesh = m_pScene->GetMesh(i);
			for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
			{
				CConversionFace* pFace = pMesh->GetFace(f);

				if (pFace->m != m)
					continue;

				glBegin(GL_TRIANGLE_FAN);

				for (size_t k = 0; k < pFace->GetNumVertices(); k++)
				{
					CConversionVertex* pVertex = pFace->GetVertex(k);

					Vector vecVertex = pMesh->GetVertex(pVertex->v);
					Vector vecNormal = pMesh->GetNormal(pVertex->vn);
					Vector vecUV = pMesh->GetUV(pVertex->vt);

					// Why? I dunno.
					vecUV.y = -vecUV.y;

					glTexCoord2fv(vecUV);
					glNormal3fv(vecNormal);
					glVertex3fv(vecVertex);
				}

				glEnd();
			}
		}

		glEndList();
	}
}

void CAOGenerator::Generate()
{
	if (!m_eAOMethod)
		return;

	m_bIsGenerating = true;
	m_bStopGenerating = false;
	m_bDoneGenerating = false;

	m_flLowestValue = -1;
	m_flHighestValue = 0;

#ifdef AO_DEBUG
	CModelWindow::Get()->ClearDebugLines();
#endif

	memset(&m_bPixelMask[0], 0, m_iWidth*m_iHeight*sizeof(bool));

	if (m_eAOMethod == AOMETHOD_SHADOWMAP)
	{
		if (!GLEW_ARB_shadow || !GLEW_ARB_depth_texture || !GLEW_ARB_vertex_shader)
		{
			// Message here?
			return;
		}

		ShadowMapSetupScene();
		GenerateShadowMaps();
	}
	else
	{
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
	}

	if (m_eAOMethod == AOMETHOD_RENDER || m_eAOMethod == AOMETHOD_SHADOWMAP)
	{
		if (m_eAOMethod == AOMETHOD_RENDER)
			glDeleteLists(m_iSceneList, (GLsizei)m_paoMaterials->size()+1);
		else
			glDeleteLists(m_iSceneList, 1);

		// We now return you to our normal render programming. Thank you for your patronage.
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	if (!m_bStopGenerating)
		Bleed();

	if (!m_bStopGenerating)
		m_bDoneGenerating = true;
	m_bIsGenerating = false;

	// One last call to let them know we're done.
	if (m_pWorkListener)
		m_pWorkListener->WorkProgress();
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

	GLsizei iShadowMapSize = 512;

	GLuint iShadowMap;
	glGenTextures(1, &iShadowMap);
	glBindTexture(GL_TEXTURE_2D, iShadowMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, iShadowMapSize, iShadowMapSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	GLuint iVertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* pszShaderSource = GetVSFlattenedShadowMap();
	glShaderSource(iVertexShader, 1, &pszShaderSource, NULL);
	glCompileShader(iVertexShader);

#ifdef _DEBUG
	int iLogLength = 0;
	char szLog[1024];
	glGetShaderInfoLog(iVertexShader, 1024, &iLogLength, szLog);
#endif

	GLuint iFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	pszShaderSource = GetFSFlattenedShadowMap();
	glShaderSource(iFragmentShader, 1, &pszShaderSource, NULL);
	glCompileShader(iFragmentShader);

#ifdef _DEBUG
	glGetShaderInfoLog(iFragmentShader, 1024, &iLogLength, szLog);
#endif

	GLuint iProgram = glCreateProgram();
	glAttachShader(iProgram, iVertexShader);
	glAttachShader(iProgram, iFragmentShader);
	glLinkProgram(iProgram);

#ifdef _DEBUG
	glGetProgramInfoLog(iProgram, 1024, &iLogLength, szLog);
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

	glMatrixMode(GL_MODELVIEW);

	size_t iSamples = (size_t)sqrt((float)m_iSamples);

	for (size_t x = 0; x <= iSamples; x++)
	{
		float flPitch = RemapVal(cos(RemapVal((float)x, 0, (float)iSamples, -M_PI/2, M_PI/2)), 0, 1, 90, 0);
		if (x < iSamples/2)
			flPitch = -flPitch;

		for (size_t y = 0; y < iSamples; y++)
		{
			if (x == 0 || x == iSamples)
			{
				// Don't do a bunch of samples from the same spot on the poles.
				if (y != 0)
					continue;
			}

			glDrawBuffer(GL_AUX1);
			glReadBuffer(GL_AUX1);

			glViewport(0, 0, iShadowMapSize, iShadowMapSize);

			float flYaw = RemapVal((float)y, 0, (float)iSamples, -180, 180);

			Vector vecDir = AngleVector(EAngle(flPitch, flYaw, 0));
			Vector vecLightPosition = vecDir*flSize + vecCenter;	// Puts us twice as far from the closest vertex

#ifdef AO_DEBUG
			CModelWindow::Get()->AddDebugLine(vecLightPosition, vecLightPosition-vecDir);
#endif

			Matrix4x4 mLightView;

			glLoadIdentity();
			gluLookAt(
				vecLightPosition.x, vecLightPosition.y, vecLightPosition.z,
				vecCenter.x, vecCenter.y, vecCenter.z,
				0, 1, 0);
			glGetFloatv(GL_MODELVIEW_MATRIX, mLightView);

			glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

			glColorMask(0, 0, 0, 0);

			glCallList(m_iSceneList);

			// Copy the depth buffer into our shadow map.
			glBindTexture(GL_TEXTURE_2D, iShadowMap);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, iShadowMapSize, iShadowMapSize);

			glColorMask(1, 1, 1, 1);

#ifdef AO_DEBUG
			glDrawBuffer(GL_FRONT);
			glReadBuffer(GL_FRONT);
			DrawTexture(iShadowMap);
			glFinish();
			glDrawBuffer(GL_AUX1);
			glReadBuffer(GL_AUX1);
#endif

			// OpenGL matrices are column major, so multiply in the wrong order to get the right result.
			Matrix4x4 m1 = mLightProjection*mBias;
			Matrix4x4 mTextureMatrix = mLightView*m1;

			// We're storing the resulting projection in GL_TEXTURE7 for later reference by the shader.
			glMatrixMode(GL_TEXTURE);
			glActiveTexture(GL_TEXTURE7);
			glLoadIdentity();
			glLoadMatrixf(mTextureMatrix);

			glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);

			glClear(GL_DEPTH_BUFFER_BIT);

			glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

			glUseProgram(iProgram);

			GLuint iLightNormalUniform = glGetUniformLocation(iProgram, "vecLightNormal");
			GLuint iShadowMapUniform = glGetUniformLocation(iProgram, "iShadowMap");

			glUniform1i(iShadowMapUniform, 7);
			glUniform3fv(iLightNormalUniform, 1, -vecDir);

			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, iShadowMap);
			glEnable(GL_TEXTURE_2D);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(
				5, 5, 10,
				0, 0, 0,
				0, 1, 0);

#ifdef AO_DEBUG
			glDrawBuffer(GL_FRONT);
			glReadBuffer(GL_FRONT);
			glCallList(m_iSceneList);
			glFinish();
			glDrawBuffer(GL_AUX1);
			glReadBuffer(GL_AUX1);
#endif

			glCallList(m_iSceneList);

			int iTimeBefore = glutGet(GLUT_ELAPSED_TIME);

			glReadPixels(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight, GL_RGB, GL_FLOAT, m_pPixels);

			iProcessSceneRead += (glutGet(GLUT_ELAPSED_TIME) - iTimeBefore);
			iTimeBefore = glutGet(GLUT_ELAPSED_TIME);

			size_t iBufferSize = m_iWidth*m_iHeight*m_iPixelDepth;
			for (size_t p = 0; p < iBufferSize; p+=m_iPixelDepth)
			{
				// Red is the clear color, skip it.
				if (m_pPixels[p+0] == 1.0f && m_pPixels[p+1] == 0.0f && m_pPixels[p+2] == 0.0f)
					continue;

				size_t i = p/m_iPixelDepth;

				// We need speed in this loop.
				m_avecShadowValues[i].x += m_pPixels[p+0];

				m_aiShadowReads[i]++;
				m_bPixelMask[i] = true;
			}

			iProcessScene += (glutGet(GLUT_ELAPSED_TIME) - iTimeBefore);

			glUseProgram(0);

			glDisable(GL_TEXTURE_2D);

			glPopAttrib();

			iTimeBefore = glutGet(GLUT_ELAPSED_TIME);

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress();

			iProgress += (glutGet(GLUT_ELAPSED_TIME) - iTimeBefore);

			if (m_bStopGenerating)
				break;
		}

		if (m_bStopGenerating)
			break;
	}

    glCullFace(GL_BACK);
    glShadeModel(GL_SMOOTH);
    glColorMask(1, 1, 1, 1);

	glDetachShader(iProgram, iVertexShader);
	glDetachShader(iProgram, iFragmentShader);
	glDeleteProgram(iProgram);
	glDeleteShader(iVertexShader);
	glDeleteShader(iFragmentShader);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glDeleteTextures(1, &iShadowMap);

	glDepthFunc(GL_LESS);

	glPopAttrib();
}

void CAOGenerator::GenerateByTexel()
{
	float flTotalTime = 0;
	float flPointInTriangleTime = 0;
	float flMatrixMathTime = 0;
	float flRenderingTime = 0;

	raytrace::CRaytracer* pTracer = NULL;

	if (m_eAOMethod == AOMETHOD_RAYTRACE)
	{
		pTracer = new raytrace::CRaytracer(m_pScene);
		pTracer->BuildTree();
	}

	for (size_t m = 0; m < m_pScene->GetNumMeshes(); m++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m);
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);
			for (size_t t = 0; t < pFace->GetNumVertices()-2; t++)
			{
				CConversionVertex* pV1 = pFace->GetVertex(0);
				CConversionVertex* pV2 = pFace->GetVertex(t+1);
				CConversionVertex* pV3 = pFace->GetVertex(t+2);

				Vector vt1 = pMesh->GetUV(pV1->vt);
				Vector vt2 = pMesh->GetUV(pV2->vt);
				Vector vt3 = pMesh->GetUV(pV3->vt);

				Vector vecLoUV = vt1;
				Vector vecHiUV = vt1;

				if (vt2.x < vecLoUV.x)
					vecLoUV.x = vt2.x;
				if (vt3.x < vecLoUV.x)
					vecLoUV.x = vt3.x;
				if (vt2.x > vecHiUV.x)
					vecHiUV.x = vt2.x;
				if (vt3.x > vecHiUV.x)
					vecHiUV.x = vt3.x;

				if (vt2.y < vecLoUV.y)
					vecLoUV.y = vt2.y;
				if (vt3.y < vecLoUV.y)
					vecLoUV.y = vt3.y;
				if (vt2.y > vecHiUV.y)
					vecHiUV.y = vt2.y;
				if (vt3.y > vecHiUV.y)
					vecHiUV.y = vt3.y;

				size_t iLoX = (size_t)(vecLoUV.x * m_iWidth + 0.5f);	// Add 0.5f so it rounds instead of truncating.
				size_t iLoY = (size_t)(vecLoUV.y * m_iHeight + 0.5f);
				size_t iHiX = (size_t)(vecHiUV.x * m_iWidth + 0.5f);
				size_t iHiY = (size_t)(vecHiUV.y * m_iHeight + 0.5f);

				// Make sure adding 0.5f didn't put us out of the texture
				if (iHiX > m_iWidth - 1)
					iHiX = m_iWidth - 1;
				if (iHiY > m_iHeight - 1)
					iHiY = m_iHeight - 1;

				for (size_t i = iLoX; i <= iHiX; i++)
				{
					for (size_t j = iLoY; j <= iHiY; j++)
					{
						float flU = (float)i/(float)m_iWidth;
						float flV = (float)j/(float)m_iHeight;

						int iTimeBefore = glutGet(GLUT_ELAPSED_TIME);
						bool bInside = PointInTriangle(Vector(flU,flV,0), vt1, vt2, vt3);
						flPointInTriangleTime += (glutGet(GLUT_ELAPSED_TIME) - iTimeBefore);

						if (!bInside)
							continue;

						iTimeBefore = glutGet(GLUT_ELAPSED_TIME);

						Vector v1 = pMesh->GetVertex(pV1->v);
						Vector v2 = pMesh->GetVertex(pV2->v);
						Vector v3 = pMesh->GetVertex(pV3->v);

						Vector vn1 = pMesh->GetNormal(pV1->vn);
						Vector vn2 = pMesh->GetNormal(pV2->vn);
						Vector vn3 = pMesh->GetNormal(pV3->vn);

						// Find where the UV is in world space.

						// First build 2x2 a "matrix" of the UV values.
						float mta = vt2.x - vt1.x;
						float mtb = vt3.x - vt1.x;
						float mtc = vt2.y - vt1.y;
						float mtd = vt3.y - vt1.y;

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

						Vector vecUVOrigin = v1 - vecUAxis * vt1.x - vecVAxis * vt1.y;

						Vector vecUVPosition = vecUVOrigin + vecUAxis * flU + vecVAxis * flV;

						float wv1 = DistanceToLine(vecUVPosition, v2, v3) / DistanceToLine(v1, v2, v3);
						float wv2 = DistanceToLine(vecUVPosition, v1, v3) / DistanceToLine(v2, v1, v3);
						float wv3 = DistanceToLine(vecUVPosition, v1, v2) / DistanceToLine(v3, v1, v2);

						Vector vecNormal = vn1 * wv1 + vn2 * wv2 + vn3 * wv3;

#ifdef AO_DEBUG
						CModelWindow::Get()->AddDebugLine(vecUVPosition, vecUVPosition + vecNormal/2);
#endif

						flMatrixMathTime += (glutGet(GLUT_ELAPSED_TIME) - iTimeBefore);

						iTimeBefore = glutGet(GLUT_ELAPSED_TIME);

						size_t iTexel;
						Texel(i, j, iTexel, false);

						if (m_eAOMethod == AOMETHOD_RENDER)
						{
							// Render the scene from this location
							m_avecShadowValues[iTexel] += RenderSceneFromPosition(vecUVPosition, vecNormal, pFace);
						}
						else if (m_eAOMethod == AOMETHOD_RAYTRACE)
						{
							// Build rotation matrix
							Matrix4x4 m;
							m.SetOrientation(vecNormal);

							// Turn it sideways so that pitch 90 is up, for more uniform sampling
							Matrix4x4 m2;
							m2.SetRotation(EAngle(0, -90, 0));

							m *= m2;

							float flHits = 0;
							float flTotalHits = 0;

							for (size_t x = 0; x < m_iSamples/2; x++)
							{
								float flPitch = RemapVal(cos(RemapVal((float)x, 0, (float)m_iSamples/2, 0, M_PI/2)), 0, 1, 90, 0);
								for (size_t y = 0; y <= m_iSamples; y++)
								{
									float flYaw = RemapVal((float)y, 0, (float)m_iSamples, -180, 180);

									Vector vecDir = AngleVector(EAngle(flPitch, flYaw, 0));

									// Transform relative to the triangle's normal
									Vector vecRay = m * vecDir;

									//RenderSceneFromPosition(vecUVPosition, vecRay, pFace);

									flTotalHits += sin(flPitch * M_PI/180);

									Vector vecHit;
									if (pTracer->Raytrace(Ray(vecUVPosition + pFace->GetNormal()*0.01f, vecRay), &vecHit))
									{
										flHits += sin(flPitch * M_PI/180);
										//CModelWindow::Get()->AddDebugLine(vecUVPosition, vecHit);
									}
								}
							}

							// One last ray directly up, it is skipped in the above loop so it's not done 10 times.
							Vector vecDir = AngleVector(EAngle(90, 0, 0));

							// Transform relative to the triangle's normal
							Vector vecRay = m * vecDir;

							//RenderSceneFromPosition(vecUVPosition, vecRay, pFace);

							flTotalHits++;

							if (pTracer->Raytrace(Ray(vecUVPosition + pFace->GetNormal()*0.01f, vecRay)))
							{
								flHits++;
								//CModelWindow::Get()->AddDebugLine(vecUVPosition, vecUVPosition + vecRay/4);
							}

							float flShadowValue = 1 - ((float)flHits / (float)flTotalHits);
							m_avecShadowValues[iTexel] += Vector(flShadowValue, flShadowValue, flShadowValue);
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

										if ((pEdge->f1 != ~0 && pMesh->GetFace(pEdge->f1) == pFace2) ||
											(pEdge->f2 != ~0 && pMesh->GetFace(pEdge->f2) == pFace2))
										{
											bFoundAdjacent = true;
											break;
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

								if ((pEdge->f1 != ~0 && pMesh->GetFace(pEdge->f1) != pFace) ||
									(pEdge->f2 != ~0 && pMesh->GetFace(pEdge->f2) != pFace))
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
									CConversionFace* pOtherFace;

									// Due to the above logic at least one of these is guaranteed to be valid,
									// and if only one is valid then it must be the one we want.
									if (pAdjacentEdge->f1 == ~0 || pMesh->GetFace(pAdjacentEdge->f1) == pFace)
										pOtherFace = pMesh->GetFace(pAdjacentEdge->f2);
									else
										pOtherFace = pMesh->GetFace(pAdjacentEdge->f1);

									// If this face is behind us, ignore.
									if ((pOtherFace->GetCenter() - vecUVPosition).Normalized().Dot(vecNormal) <= 0)
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

						flRenderingTime += (glutGet(GLUT_ELAPSED_TIME) - iTimeBefore);

						if (m_pWorkListener)
							m_pWorkListener->WorkProgress();

						if (m_bStopGenerating)
							break;
					}
					if (m_bStopGenerating)
						break;
				}
				if (m_bStopGenerating)
					break;
			}
			if (m_bStopGenerating)
				break;
		}
		if (m_bStopGenerating)
			break;
	}

	if (m_eAOMethod == AOMETHOD_RAYTRACE)
	{
		delete pTracer;
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
	// Adds .001 because of a bug where GL for some reason won't show the back faces unless I do that.
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

	// Draw the dark insides
	//glCallList(m_iSceneList);

	// Draw polyons without materials
	glCallList(m_iSceneList+1);

	for (size_t i = 0; i < m_paoMaterials->size(); i++)
		glCallList(m_iSceneList+i+2);

#ifdef AO_DEBUG
	DebugRenderSceneLookAtPosition(vecPosition, vecDirection, pRenderFace);
#endif

	glFinish();

	Vector vecShadowColor(0,0,0);

	glReadBuffer(eBuffer);

	glReadPixels(m_iRPVX, m_iRPVY, m_iRPVW, m_iRPVH, GL_RGB, GL_FLOAT, m_pPixels);

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
		if (flWeight <= 0)
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

	//glCallList(m_iSceneList);

	for (size_t i = 0; i < m_paoMaterials->size(); i++)
		glCallList(m_iSceneList+i+1);

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
			m_pWorkListener->WorkProgress();

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
			else if (m_eAOMethod == AOMETHOD_SHADOWMAP)
			{
				// Shadow map uses only x values for speed reasons.
				float flXDiv = m_avecShadowValues[i].x / m_aiShadowReads[i];
				avecShadowValues[i].x = avecShadowValues[i].y = avecShadowValues[i].z = flXDiv;
			}
			else
				avecShadowValues[i] = m_avecShadowValues[i] / (float)m_aiShadowReads[i];
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
	ilEnable(IL_FILE_OVERWRITE);

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	// WHAT A HACK!
	ilTexImage((ILint)m_iWidth, (ILint)m_iHeight, 1, 3, IL_RGB, IL_FLOAT, NULL);

	ilSetData(&m_avecShadowValues[0].x);

	// I'm glad this flips X and not Y, or it'd be pretty useless to me.
	iluFlipImage();

	ilSaveImage(pszFilename);

	ilDeleteImages(1,&iDevILId);
}

bool CAOGenerator::Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask)
{
	if (w < 0 || h < 0 || w >= m_iWidth || h >= m_iHeight)
		return false;

	float flW = (float)w;
	float flH = (float)h;

	size_t iRow = m_iHeight-h-1;
	iTexel = m_iHeight*iRow + w;

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

void DrawTexture(GLuint iTexture)
{
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
	glEnable(GL_TEXTURE_2D);

	glShadeModel(GL_SMOOTH);

	glBindTexture(GL_TEXTURE_2D, iTexture);

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(-1.0f, 1.0f);

		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(-1.0f, -1.0f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(1.0f, -1.0f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(1.0f, 1.0f);
	glEnd();

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
#endif
