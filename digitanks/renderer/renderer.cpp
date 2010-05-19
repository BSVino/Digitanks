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
