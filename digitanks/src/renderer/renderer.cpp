#include "renderer.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <modelconverter/convmesh.h>
#include <models/models.h>
#include <maths.h>

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

	for (size_t i = 0; i < pModel->m_pScene->GetNumScenes(); i++)
		RenderSceneNode(pModel->m_pScene, pModel->m_pScene->GetScene(i));

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

			CConversionMaterial* pMaterial = pScene->GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial);
			if (pMaterial && !pMaterial->IsVisible())
				continue;
		}

		Vector vecDiffuse(0.7f, 0.7f, 0.7f);
		if (pScene->DoesFaceHaveValidMaterial(pFace, pMeshInstance))
		{
			CConversionMaterial* pMaterial = pScene->GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial);
			glMaterialfv(GL_FRONT, GL_AMBIENT, pMaterial->m_vecAmbient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, pMaterial->m_vecDiffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR, pMaterial->m_vecSpecular);
			glMaterialfv(GL_FRONT, GL_EMISSION, pMaterial->m_vecEmissive);
			glMaterialf(GL_FRONT, GL_SHININESS, pMaterial->m_flShininess);
			vecDiffuse = pMaterial->m_vecDiffuse;
		}

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
}

void CRenderer::SetupFrame()
{
	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);
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
