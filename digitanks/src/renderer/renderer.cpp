#include "renderer.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <maths.h>

#include <modelconverter/convmesh.h>
#include <models/models.h>
#include <shaders/shaders.h>

CRenderingContext::CRenderingContext()
{
	m_bMatrixTransformations = false;
	m_flAlpha = 1;
}

CRenderingContext::~CRenderingContext()
{
	if (m_bMatrixTransformations)
		glPopMatrix();
}

void CRenderingContext::Translate(Vector vecTranslate)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glTranslatef(vecTranslate.x, vecTranslate.y, vecTranslate.z);
}

void CRenderingContext::Rotate(float flAngle, Vector vecAxis)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glRotatef(flAngle, vecAxis.x, vecAxis.y, vecAxis.z);
}

void CRenderingContext::Scale(float flX, float flY, float flZ)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glScalef(flX, flY, flZ);
}

void CRenderingContext::RenderModel(size_t iModel)
{
	CModel* pModel = CModelLibrary::Get()->GetModel(iModel);

	glPushAttrib(GL_ENABLE_BIT);

	if (m_flAlpha < 1.0f)
	{
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	if (pModel->m_bStatic)
	{
		GLuint iProgram = (GLuint)CShaderLibrary::GetModelProgram();
		glUseProgram(iProgram);

		GLuint flAlpha = glGetUniformLocation(iProgram, "flAlpha");
		glUniform1f(flAlpha, m_flAlpha);

		glCallList((GLuint)pModel->m_iCallList);

		glUseProgram(0);
	}
	else
	{
		for (size_t i = 0; i < pModel->m_pScene->GetNumScenes(); i++)
			RenderSceneNode(pModel->m_pScene, pModel->m_pScene->GetScene(i));
	}

	glPopAttrib();
}

void CRenderingContext::RenderSceneNode(CConversionScene* pScene, CConversionSceneNode* pNode)
{
	if (!pNode)
		return;

	if (!pNode->IsVisible())
		return;

	glPushMatrix();

	glMultMatrixf(pNode->m_mTransformations.Transposed());	// GL uses column major.

	for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		RenderSceneNode(pScene, pNode->GetChild(i));

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		RenderMeshInstance(pScene, pNode->GetMeshInstance(m));

	glPopMatrix();
}

void CRenderingContext::RenderMeshInstance(CConversionScene* pScene, CConversionMeshInstance* pMeshInstance)
{
	if (!pMeshInstance->IsVisible())
		return;

	// It uses this color if the texture is missing.
	GLfloat flMaterialColor[] = {0.7f, 0.7f, 0.7f, m_flAlpha};

	glMaterialfv(GL_FRONT, GL_DIFFUSE, flMaterialColor);
	glColor4fv(flMaterialColor);

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	Vector vecDiffuse(0.7f, 0.7f, 0.7f);

	for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
	{
		size_t k;
		CConversionFace* pFace = pMesh->GetFace(j);

		if (pFace->m == ~0)
			continue;

		CConversionMaterial* pMaterial = NULL;
		CConversionMaterialMap* pConversionMaterialMap = pMeshInstance->GetMappedMaterial(pFace->m);

		if (pConversionMaterialMap)
		{
			if (!pConversionMaterialMap->IsVisible())
				continue;

			pMaterial = pScene->GetMaterial(pConversionMaterialMap->m_iMaterial);
			if (pMaterial && !pMaterial->IsVisible())
				continue;
		}

		if (pMaterial)
			vecDiffuse = pMaterial->m_vecDiffuse;

		glBegin(GL_POLYGON);

		for (k = 0; k < pFace->GetNumVertices(); k++)
		{
			CConversionVertex* pVertex = pFace->GetVertex(k);

			// Give the tank a little bit of manual shading since there's no lights.
			float flDarken = RemapVal(pMesh->GetNormal(pVertex->vn).Dot(Vector(0, 1, 0)), -1, 1, 0.3f, 1.0f);
			glColor4f(vecDiffuse.x * flDarken, vecDiffuse.y * flDarken, vecDiffuse.z * flDarken, m_flAlpha);

			glVertex3fv(pMesh->GetVertex(pVertex->v));
		}

		glEnd();
	}
}

CRenderer::CRenderer(size_t iWidth, size_t iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	m_oSceneBuffer = CreateFrameBuffer();
}

CFrameBuffer CRenderer::CreateFrameBuffer()
{
	CFrameBuffer oBuffer;

	glGenTextures(1, &oBuffer.m_iMap);
	glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_iWidth, (GLsizei)m_iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffersEXT(1, &oBuffer.m_iDepth);
	glBindRenderbufferEXT( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
	glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)m_iWidth, (GLsizei)m_iHeight );
	glBindRenderbufferEXT( GL_RENDERBUFFER, 0 );

	glGenFramebuffersEXT(1, &oBuffer.m_iFB);
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	return oBuffer;
}

void CRenderer::SetupFrame()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oSceneBuffer.m_iFB);

	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);

	glClear(GL_DEPTH_BUFFER_BIT);
}

void CRenderer::DrawBackground()
{
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
}

void CRenderer::StartRendering()
{
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(
			44.0,
			(float)m_iWidth/(float)m_iHeight,
			10,
			1000.0
		);

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();

	gluLookAt(m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z,
		m_vecCameraTarget.x, m_vecCameraTarget.y, m_vecCameraTarget.z,
		0.0, 1.0, 0.0);

	glGetDoublev( GL_MODELVIEW_MATRIX, m_aiModelView );
	glGetDoublev( GL_PROJECTION_MATRIX, m_aiProjection );
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
}

void CRenderer::FinishRendering()
{
	glPopMatrix();
	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
    glOrtho(0, m_iWidth, m_iHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);

	glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glColor4f(1, 1, 1, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (GLuint)m_oSceneBuffer.m_iMap);
	glEnable(GL_TEXTURE_2D);

	glUseProgram(0);

	glBegin(GL_QUADS);
		glTexCoord2i(0, 1); glVertex2d(0, 0);
		glTexCoord2i(0, 0); glVertex2d(0, m_iHeight);
		glTexCoord2i(1, 0); glVertex2d(m_iWidth, m_iHeight);
		glTexCoord2i(1, 1); glVertex2d(m_iWidth, 0);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();

	glFinish();
}

void CRenderer::SetSize(int w, int h)
{
	m_iWidth = w;
	m_iHeight = h;
}

Vector CRenderer::ScreenPosition(Vector vecWorld)
{
	GLdouble x, y, z;
	gluProject(
		vecWorld.x, vecWorld.y, vecWorld.z,
		(GLdouble*)m_aiModelView, (GLdouble*)m_aiProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)m_iHeight - (float)y, (float)z);
}

Vector CRenderer::WorldPosition(Vector vecScreen)
{
	GLdouble x, y, z;
	gluUnProject(
		vecScreen.x, (float)m_iHeight - vecScreen.y, vecScreen.z,
		(GLdouble*)m_aiModelView, (GLdouble*)m_aiProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)y, (float)z);
}

size_t CRenderer::CreateCallList(size_t iModel)
{
	size_t iCallList = glGenLists(1);

	glNewList((GLuint)iCallList, GL_COMPILE);
	CRenderingContext c;
	c.RenderModel(iModel);
	glEndList();

	return iCallList;
}
