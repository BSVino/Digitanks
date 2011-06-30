#include "renderer.h"

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <maths.h>
#include <simplex.h>

#include <modelconverter/convmesh.h>
#include <models/models.h>
#include <shaders/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <game/gameserver.h>

CRenderingContext::CRenderingContext(CRenderer* pRenderer)
{
	m_pRenderer = pRenderer;

	m_bMatrixTransformations = false;
	m_bBoundTexture = false;
	m_bFBO = false;
	m_iProgram = 0;
	m_bAttribs = false;

	m_bColorSwap = false;

	m_eBlend = BLEND_NONE;
	m_flAlpha = 1;
}

CRenderingContext::~CRenderingContext()
{
	if (m_bMatrixTransformations)
		glPopMatrix();

	if (m_bBoundTexture)
		glBindTexture(GL_TEXTURE_2D, 0);

	if (m_bFBO)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_pRenderer->GetSceneBuffer()->m_iFB);
		glViewport(0, 0, (GLsizei)m_pRenderer->GetSceneBuffer()->m_iWidth, (GLsizei)m_pRenderer->GetSceneBuffer()->m_iHeight);
	}

	if (m_iProgram)
		glUseProgram(0);

	if (m_bAttribs)
		glPopAttrib();
}

void CRenderingContext::Transform(const Matrix4x4& m)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glMultMatrixf(m.Transposed());	// GL uses column major.
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

void CRenderingContext::ResetTransformations()
{
	if (m_bMatrixTransformations)
	{
		m_bMatrixTransformations = false;
		glPopMatrix();
	}
}

void CRenderingContext::SetBlend(blendtype_t eBlend)
{
	if (!m_bAttribs)
		PushAttribs();

	if (eBlend)
	{
		glEnable(GL_BLEND);

		if (eBlend == BLEND_ALPHA)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	m_eBlend = eBlend;
}

void CRenderingContext::SetDepthMask(bool bDepthMask)
{
	if (!m_bAttribs)
		PushAttribs();

	glDepthMask(bDepthMask);
}

void CRenderingContext::SetDepthTest(bool bDepthTest)
{
	if (!m_bAttribs)
		PushAttribs();

	if (bDepthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void CRenderingContext::SetBackCulling(bool bCull)
{
	if (!m_bAttribs)
		PushAttribs();

	if (bCull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void CRenderingContext::SetColorSwap(Color clrSwap)
{
	m_bColorSwap = true;
	m_clrSwap = clrSwap;
}

void CRenderingContext::RenderModel(size_t iModel, CModel* pCompilingModel)
{
	CModel* pModel = CModelLibrary::Get()->GetModel(iModel);

	if (!pModel)
		return;

	if (pModel->m_bStatic && !pCompilingModel)
	{
		if (m_pRenderer->IsBatching())
		{
			TAssert(m_eBlend == BLEND_NONE);

			Matrix4x4 mTransformations;
			glGetFloatv(GL_MODELVIEW_MATRIX, mTransformations);

			m_pRenderer->AddToBatch(pModel, mTransformations, m_bColorSwap, m_clrSwap);
		}
		else
		{
			glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LIGHTING_BIT|GL_TEXTURE_BIT);

			TAssert(pModel->m_iCallListTexture);
			glBindTexture(GL_TEXTURE_2D, pModel->m_iCallListTexture);

			if (m_pRenderer->ShouldUseShaders())
			{
				GLuint iProgram = (GLuint)CShaderLibrary::GetModelProgram();
				glUseProgram(iProgram);

				GLuint bDiffuse = glGetUniformLocation(iProgram, "bDiffuse");
				glUniform1i(bDiffuse, true);

				GLuint iDiffuse = glGetUniformLocation(iProgram, "iDiffuse");
				glUniform1i(iDiffuse, 0);

				GLuint flAlpha = glGetUniformLocation(iProgram, "flAlpha");
				glUniform1f(flAlpha, m_flAlpha);

				GLuint bColorSwapInAlpha = glGetUniformLocation(iProgram, "bColorSwapInAlpha");
				glUniform1i(bColorSwapInAlpha, m_bColorSwap);

				if (m_bColorSwap)
				{
					GLuint vecColorSwap = glGetUniformLocation(iProgram, "vecColorSwap");
					Vector vecColor((float)m_clrSwap.r()/255, (float)m_clrSwap.g()/255, (float)m_clrSwap.b()/255);
					glUniform3fv(vecColorSwap, 1, vecColor);
				}

				glColor4f(1, 1, 1, 1);

				glCallList((GLuint)pModel->m_iCallList);

				glUseProgram(0);
			}
			else
			{
				if (m_bColorSwap)
					glColor4f(((float)m_clrSwap.r())/255, ((float)m_clrSwap.g())/255, ((float)m_clrSwap.b())/255, m_flAlpha);
				else
					glColor4f(1, 1, 1, m_flAlpha);

				glCallList((GLuint)pModel->m_iCallList);
			}

			glPopAttrib();
		}
	}
	else
	{
		for (size_t i = 0; i < pModel->m_pScene->GetNumScenes(); i++)
			RenderSceneNode(pModel, pModel->m_pScene, pModel->m_pScene->GetScene(i), pCompilingModel);
	}

	if (!pCompilingModel && m_pRenderer->ShouldUseShaders() && !m_pRenderer->IsBatching())
		m_pRenderer->ClearProgram();
}

void CRenderingContext::RenderSceneNode(CModel* pModel, CConversionScene* pScene, CConversionSceneNode* pNode, CModel* pCompilingModel)
{
	if (!pNode)
		return;

	if (!pNode->IsVisible())
		return;

	bool bTransformationsIdentity = false;
	if (pNode->m_mTransformations.IsIdentity())
		bTransformationsIdentity = true;

	if (!bTransformationsIdentity)
	{
		glPushMatrix();

		glMultMatrixf(pNode->m_mTransformations.Transposed());	// GL uses column major.
	}

	for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		RenderSceneNode(pModel, pScene, pNode->GetChild(i), pCompilingModel);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		RenderMeshInstance(pModel, pScene, pNode->GetMeshInstance(m), pCompilingModel);

	if (!bTransformationsIdentity)
		glPopMatrix();
}

void CRenderingContext::RenderMeshInstance(CModel* pModel, CConversionScene* pScene, CConversionMeshInstance* pMeshInstance, CModel* pCompilingModel)
{
	if (!pMeshInstance->IsVisible())
		return;

	if (!pCompilingModel)
		glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LIGHTING_BIT|GL_TEXTURE_BIT);

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

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

		if (pCompilingModel)
		{
			if (pMaterial)
			{
				GLuint iTexture = (GLuint)pModel->m_aiTextures[pConversionMaterialMap->m_iMaterial];

				if (!pModel->m_iCallListTexture)
					pModel->m_iCallListTexture = iTexture;
				else
					// If you hit this you have more than one texture in a call list that's building.
					// That's a no-no because these call lists are batched.
					TAssert(pModel->m_iCallListTexture == iTexture);
			}
		}

		if (!pCompilingModel)
		{
			bool bTexture = false;
			if (pMaterial)
			{
				GLuint iTexture = (GLuint)pModel->m_aiTextures[pConversionMaterialMap->m_iMaterial];
				glBindTexture(GL_TEXTURE_2D, iTexture);

				bTexture = !!iTexture;
			}
			else
				glBindTexture(GL_TEXTURE_2D, 0);

			if (m_pRenderer->ShouldUseShaders())
			{
				GLuint iProgram = (GLuint)CShaderLibrary::GetModelProgram();
				glUseProgram(iProgram);

				GLuint bDiffuse = glGetUniformLocation(iProgram, "bDiffuse");
				glUniform1i(bDiffuse, bTexture);

				GLuint iDiffuse = glGetUniformLocation(iProgram, "iDiffuse");
				glUniform1i(iDiffuse, 0);

				GLuint flAlpha = glGetUniformLocation(iProgram, "flAlpha");
				glUniform1f(flAlpha, m_flAlpha);

				GLuint bColorSwapInAlpha = glGetUniformLocation(iProgram, "bColorSwapInAlpha");
				glUniform1i(bColorSwapInAlpha, m_bColorSwap);

				if (m_bColorSwap)
				{
					GLuint vecColorSwap = glGetUniformLocation(iProgram, "vecColorSwap");
					Vector vecColor((float)m_clrSwap.r()/255, (float)m_clrSwap.g()/255, (float)m_clrSwap.b()/255);
					glUniform3fv(vecColorSwap, 1, vecColor);
				}
			}
			else
			{
				if (m_bColorSwap)
					glColor4f(((float)m_clrSwap.r())/255, ((float)m_clrSwap.g())/255, ((float)m_clrSwap.b())/255, m_flAlpha);
				else
					glColor4f(pMaterial->m_vecDiffuse.x, pMaterial->m_vecDiffuse.y, pMaterial->m_vecDiffuse.z, m_flAlpha);
			}
		}

		glBegin(GL_POLYGON);

		for (k = 0; k < pFace->GetNumVertices(); k++)
		{
			CConversionVertex* pVertex = pFace->GetVertex(k);

			glTexCoord2fv(pMesh->GetUV(pVertex->vu));
			glVertex3fv(pMesh->GetVertex(pVertex->v));
		}

		glEnd();
	}

	if (!pCompilingModel)
		glPopAttrib();
}

void CRenderingContext::RenderSphere()
{
	static size_t iSphereCallList = 0;

	if (iSphereCallList == 0)
	{
		GLUquadricObj* pQuadric = gluNewQuadric();
		iSphereCallList = glGenLists(1);
		glNewList((GLuint)iSphereCallList, GL_COMPILE);
		gluSphere(pQuadric, 1, 20, 10);
		glEndList();
		gluDeleteQuadric(pQuadric);
	}

	glCallList(iSphereCallList);
}

void CRenderingContext::UseFrameBuffer(const CFrameBuffer* pBuffer)
{
	TAssert(m_pRenderer->ShouldUseFramebuffers());

	m_bFBO = true;
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)pBuffer->m_iFB);
	glViewport(0, 0, (GLsizei)pBuffer->m_iWidth, (GLsizei)pBuffer->m_iHeight);
}

void CRenderingContext::UseProgram(size_t iProgram)
{
	TAssert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	m_iProgram = iProgram;
	glUseProgram((GLuint)iProgram);
}

void CRenderingContext::SetUniform(const char* pszName, int iValue)
{
	TAssert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1i(iUniform, iValue);
}

void CRenderingContext::SetUniform(const char* pszName, float flValue)
{
	TAssert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1f(iUniform, flValue);
}

void CRenderingContext::SetUniform(const char* pszName, const Vector& vecValue)
{
	TAssert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform3fv(iUniform, 1, vecValue);
}

void CRenderingContext::SetUniform(const char* pszName, const Color& clrValue)
{
	TAssert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform3fv(iUniform, 1, Vector(clrValue));
}

void CRenderingContext::BindTexture(size_t iTexture)
{
	glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
	m_bBoundTexture = true;
}

void CRenderingContext::SetColor(Color c)
{
	if (!m_bAttribs)
		PushAttribs();

	glColor4ub(c.r(), c.g(), c.b(), (unsigned char)(c.a()*m_flAlpha));
}

void CRenderingContext::BeginRenderTris()
{
	glBegin(GL_TRIANGLES);
}

void CRenderingContext::BeginRenderQuads()
{
	glBegin(GL_QUADS);
}

void CRenderingContext::TexCoord(float s, float t)
{
	glTexCoord2f(s, t);
}

void CRenderingContext::TexCoord(const Vector& v)
{
	glTexCoord2fv(v);
}

void CRenderingContext::Vertex(const Vector& v)
{
	glVertex3fv(v);
}

void CRenderingContext::RenderCallList(size_t iCallList)
{
	glCallList((GLuint)iCallList);
}

void CRenderingContext::EndRender()
{
	glEnd();
}

void CRenderingContext::PushAttribs()
{
	m_bAttribs = true;
	// Push all the attribs we'll ever need. I don't want to have to worry about popping them in order.
	glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_CURRENT_BIT);
}

CFrameBuffer::CFrameBuffer()
{
	m_iMap = m_iDepth = m_iFB = 0;
}

CRopeRenderer::CRopeRenderer(CRenderer *pRenderer, size_t iTexture, Vector vecStart, float flWidth)
	: m_oContext(pRenderer)
{
	m_pRenderer = pRenderer;

	m_oContext.BindTexture(iTexture);
	m_vecLastLink = vecStart;
	m_bFirstLink = true;

	m_flWidth = m_flLastLinkWidth = flWidth;

	m_flTextureScale = 1;
	m_flTextureOffset = 0;

	m_bUseForward = false;

	m_oContext.SetBlend(BLEND_ADDITIVE);
	m_oContext.SetDepthMask(false);

	m_clrRope = Color(255, 255, 255, 255);
	m_oContext.BeginRenderQuads();
}

void CRopeRenderer::AddLink(Vector vecLink)
{
	Vector vecForward;
	if (m_bUseForward)
		vecForward = m_vecForward;
	else
		vecForward = m_pRenderer->GetCameraVector();

	Vector vecUp = (vecLink - m_vecLastLink).Normalized();
	Vector vecLastRight = vecForward.Cross(vecUp)*(m_flLastLinkWidth/2);
	Vector vecRight = vecForward.Cross(vecUp)*(m_flWidth/2);

	float flAddV = (1/m_flTextureScale);

	m_oContext.SetColor(m_clrRope);

	if (!m_bFirstLink)
	{
		// Finish the previous link
		m_oContext.TexCoord(1, m_flTextureOffset+flAddV);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);
		m_oContext.TexCoord(0, m_flTextureOffset+flAddV);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);

		m_flTextureOffset += flAddV;
	}

	m_bFirstLink = false;

	// Start this link
	m_oContext.TexCoord(0, m_flTextureOffset);
	m_oContext.Vertex(m_vecLastLink-vecLastRight);
	m_oContext.TexCoord(1, m_flTextureOffset);
	m_oContext.Vertex(m_vecLastLink+vecLastRight);

	m_vecLastLink = vecLink;
	m_flLastLinkWidth = m_flWidth;
}

void CRopeRenderer::FinishSegment(Vector vecLink, Vector vecNextSegmentStart, float flNextSegmentWidth)
{
	Vector vecForward;
	if (m_bUseForward)
		vecForward = m_vecForward;
	else
		vecForward = m_pRenderer->GetCameraVector();

	Vector vecUp = (vecLink - m_vecLastLink).Normalized();
	Vector vecLastRight = vecForward.Cross(vecUp)*(m_flLastLinkWidth/2);
	Vector vecRight = vecForward.Cross(vecUp)*(m_flWidth/2);

	float flAddV = (1/m_flTextureScale);

	if (m_bFirstLink)
	{
		// Start the previous link
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);

		m_flTextureOffset += flAddV;

		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(vecLink+vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(vecLink-vecRight);
	}
	else
	{
		m_flTextureOffset += flAddV;

		// Finish the last link
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);

		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);

		m_flTextureOffset += flAddV;

		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(vecLink+vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(vecLink-vecRight);
	}

	m_bFirstLink = true;
	m_vecLastLink = vecNextSegmentStart;
	m_flLastLinkWidth = flNextSegmentWidth;
}

void CRopeRenderer::Finish(Vector vecLink)
{
	Vector vecForward;
	if (m_bUseForward)
		vecForward = m_vecForward;
	else
		vecForward = m_pRenderer->GetCameraVector();

	Vector vecUp = (vecLink - m_vecLastLink).Normalized();
	Vector vecLastRight = vecForward.Cross(vecUp)*(m_flLastLinkWidth/2);
	Vector vecRight = vecForward.Cross(vecUp)*(m_flWidth/2);

	float flAddV = (1/m_flTextureScale);

	if (m_bFirstLink)
	{
		// Start the previous link
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);

		m_flTextureOffset += flAddV;

		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(vecLink+vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(vecLink-vecRight);
	}
	else
	{
		m_flTextureOffset += flAddV;

		// Finish the last link
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);

		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);

		m_flTextureOffset += flAddV;

		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(vecLink+vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(vecLink-vecRight);
	}

	m_oContext.EndRender();
}

void CRopeRenderer::SetForward(Vector vecForward)
{
	m_bUseForward = true;
	m_vecForward = vecForward;
}

CVar r_batch("r_batch", "1");

CRenderer::CRenderer(size_t iWidth, size_t iHeight)
{
	TMsg(_T("Initializing renderer\n"));

#ifdef TINKER_OPTIMIZE_SOFTWARE
	m_bHardwareSupportsFramebuffers = false;
	m_bHardwareSupportsFramebuffersTestCompleted = true;
	m_bUseFramebuffers = false;

	m_bHardwareSupportsShaders = false;
	m_bHardwareSupportsShadersTestCompleted = true;
	m_bUseShaders = false;
#else
	m_bHardwareSupportsFramebuffers = false;
	m_bHardwareSupportsFramebuffersTestCompleted = false;

	m_bUseFramebuffers = true;

	if (!HardwareSupportsFramebuffers())
		m_bUseFramebuffers = false;

	m_bUseShaders = true;

	m_bHardwareSupportsShaders = false;
	m_bHardwareSupportsShadersTestCompleted = false;

	if (!HardwareSupportsShaders())
		m_bUseShaders = false;
	else
		CShaderLibrary::CompileShaders();

	if (!CShaderLibrary::IsCompiled())
		m_bUseShaders = false;
#endif

	if (m_bUseShaders)
		TMsg(_T("* Using shaders\n"));
	if (m_bUseFramebuffers)
		TMsg(_T("* Using framebuffers\n"));

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	m_bFrustumOverride = false;
	m_bBatching = false;

	m_bBatchThisFrame = r_batch.GetBool();
}

void CRenderer::Initialize()
{
	if (ShouldUseFramebuffers())
	{
		m_oSceneBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, true, true);

		size_t iWidth = m_oSceneBuffer.m_iWidth;
		size_t iHeight = m_oSceneBuffer.m_iHeight;
		for (size_t i = 0; i < BLOOM_FILTERS; i++)
		{
			m_oBloom1Buffers[i] = CreateFrameBuffer(iWidth, iHeight, false, true);
			m_oBloom2Buffers[i] = CreateFrameBuffer(iWidth, iHeight, false, false);
			iWidth /= 2;
			iHeight /= 2;

			if (GameServer()->GetWorkListener())
				GameServer()->GetWorkListener()->WorkProgress(i);
		}

		if (GameServer()->GetWorkListener())
			GameServer()->GetWorkListener()->SetAction(_T("Making noise"), 0);

		m_oNoiseBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, false, false);

		CreateNoise();
	}
}

CFrameBuffer CRenderer::CreateFrameBuffer(size_t iWidth, size_t iHeight, bool bDepth, bool bLinear)
{
	TAssert(ShouldUseFramebuffers());

	if (!GLEW_ARB_texture_non_power_of_two)
	{
		// If non power of two textures are not supported, framebuffers the size of the screen will probably fuck up.
		// I don't know this for sure but I'm not taking any chances. If the extension isn't supported, roll those
		// framebuffer sizes up to the next power of two.
		iWidth--;
		iWidth |= iWidth >> 1;
		iWidth |= iWidth >> 2;
		iWidth |= iWidth >> 4;
		iWidth |= iWidth >> 8;
		iWidth |= iWidth >> 16;
		iWidth++;

		iHeight--;
		iHeight |= iHeight >> 1;
		iHeight |= iHeight >> 2;
		iHeight |= iHeight >> 4;
		iHeight |= iHeight >> 8;
		iHeight |= iHeight >> 16;
		iHeight++;
	}

	CFrameBuffer oBuffer;

	glGenTextures(1, &oBuffer.m_iMap);
	glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bLinear?GL_LINEAR:GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bLinear?GL_LINEAR:GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)iWidth, (GLsizei)iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (bDepth)
	{
		glGenRenderbuffersEXT(1, &oBuffer.m_iDepth);
		glBindRenderbufferEXT( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
		glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)iWidth, (GLsizei)iHeight );
		glBindRenderbufferEXT( GL_RENDERBUFFER, 0 );
	}

	glGenFramebuffersEXT(1, &oBuffer.m_iFB);
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap, 0);
	if (bDepth)
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		printf("Framebuffer not complete!\n");

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	oBuffer.m_iWidth = iWidth;
	oBuffer.m_iHeight = iHeight;

	oBuffer.m_iCallList = glGenLists(1);
	glNewList(oBuffer.m_iCallList, GL_COMPILE);
	glBegin(GL_QUADS);
		glTexCoord2i(0, 1); glVertex2i(0, 0);
		glTexCoord2i(0, 0); glVertex2i(0, (GLint)iHeight);
		glTexCoord2i(1, 0); glVertex2i((GLint)iWidth, (GLint)iHeight);
		glTexCoord2i(1, 1); glVertex2i((GLint)iWidth, 0);
	glEnd();
	glEndList();

	return oBuffer;
}

void CRenderer::CreateNoise()
{
	CSimplexNoise n1(mtrand()+0);
	CSimplexNoise n2(mtrand()+1);
	CSimplexNoise n3(mtrand()+2);

	float flSpaceFactor1 = 0.1f;
	float flHeightFactor1 = 0.5f;
	float flSpaceFactor2 = flSpaceFactor1*3;
	float flHeightFactor2 = flHeightFactor1/3;
	float flSpaceFactor3 = flSpaceFactor2*3;
	float flHeightFactor3 = flHeightFactor2/3;

	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oNoiseBuffer.m_iFB);

	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);

	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
    glOrtho(0, m_iWidth, m_iHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPointSize(1);

	glPushAttrib(GL_CURRENT_BIT);

	for (size_t x = 0; x < m_iWidth; x++)
	{
		for (size_t y = 0; y < m_iHeight; y++)
		{
			float flValue = 0.5f;
			flValue += n1.Noise(x*flSpaceFactor1, y*flSpaceFactor1) * flHeightFactor1;
			flValue += n2.Noise(x*flSpaceFactor2, y*flSpaceFactor2) * flHeightFactor2;
			flValue += n3.Noise(x*flSpaceFactor3, y*flSpaceFactor3) * flHeightFactor3;

			glBegin(GL_POINTS);
				glColor3f(flValue, flValue, flValue);
				glVertex2f((float)x, (float)y);
			glEnd();
		}
	}

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
}

void CRenderer::SetupFrame()
{
	TPROF("CRenderer::SetupFrame");

	m_bBatchThisFrame = r_batch.GetBool();

	if (ShouldUseFramebuffers())
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oSceneBuffer.m_iFB);
		glViewport(0, 0, (GLsizei)m_oSceneBuffer.m_iWidth, (GLsizei)m_oSceneBuffer.m_iHeight);
	}
	else
	{
		glReadBuffer(GL_BACK);
		glDrawBuffer(GL_BACK);
		glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	}

	glClear(GL_DEPTH_BUFFER_BIT);

	glColor4f(1, 1, 1, 1);
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

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

#ifdef TINKER_OPTIMIZE_SOFTWARE
	glShadeModel(GL_FLAT);
#else
	glShadeModel(GL_SMOOTH);
#endif

	glBegin(GL_QUADS);
		glColor3ub(0, 0, 0);
		glVertex2f(-1.0f, 1.0f);
		glColor3ub(0, 0, 0);
		glVertex2f(-1.0f, -1.0f);
		glColor3ub(0, 0, 0);
		glVertex2f(1.0f, -1.0f);
		glColor3ub(0, 0, 0);
		glVertex2f(1.0f, 1.0f);
	glEnd();

	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

#define FRUSTUM_NEAR	0
#define FRUSTUM_FAR		1
#define FRUSTUM_LEFT	2
#define FRUSTUM_RIGHT	3
#define FRUSTUM_UP		4
#define FRUSTUM_DOWN	5

void CRenderer::StartRendering()
{
	TPROF("CRenderer::StartRendering");

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluPerspective(
			m_flCameraFOV,
			(float)m_iWidth/(float)m_iHeight,
			m_flCameraNear,
			m_flCameraFar
		);

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();

	gluLookAt(m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z,
		m_vecCameraTarget.x, m_vecCameraTarget.y, m_vecCameraTarget.z,
		0.0, 1.0, 0.0);

	glGetDoublev( GL_MODELVIEW_MATRIX, m_aiModelView );
	glGetDoublev( GL_PROJECTION_MATRIX, m_aiProjection );

	if (m_bFrustumOverride)
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		gluPerspective(
				m_flFrustumFOV,
				(float)m_iWidth/(float)m_iHeight,
				m_flFrustumNear,
				m_flFrustumFar
			);

		glMatrixMode(GL_MODELVIEW);

		glPushMatrix();
		glLoadIdentity();

		gluLookAt(m_vecFrustumPosition.x, m_vecFrustumPosition.y, m_vecFrustumPosition.z,
			m_vecFrustumTarget.x, m_vecFrustumTarget.y, m_vecFrustumTarget.z,
			0.0, 1.0, 0.0);
	}

	Matrix4x4 mModelView, mProjection;
	glGetFloatv( GL_MODELVIEW_MATRIX, mModelView );
	glGetFloatv( GL_PROJECTION_MATRIX, mProjection );

	if (m_bFrustumOverride)
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	Matrix4x4 mFrustum = mModelView * mProjection;

	float* pflFrustum = mFrustum;

	m_aoFrustum[FRUSTUM_RIGHT].n.x = mFrustum.m[0][3] - mFrustum.m[0][0];
	m_aoFrustum[FRUSTUM_RIGHT].n.y = mFrustum.m[1][3] - mFrustum.m[1][0];
	m_aoFrustum[FRUSTUM_RIGHT].n.z = mFrustum.m[2][3] - mFrustum.m[2][0];
	m_aoFrustum[FRUSTUM_RIGHT].d = mFrustum.m[3][3] - mFrustum.m[3][0];

	m_aoFrustum[FRUSTUM_LEFT].n.x = mFrustum.m[0][3] + mFrustum.m[0][0];
	m_aoFrustum[FRUSTUM_LEFT].n.y = mFrustum.m[1][3] + mFrustum.m[1][0];
	m_aoFrustum[FRUSTUM_LEFT].n.z = mFrustum.m[2][3] + mFrustum.m[2][0];
	m_aoFrustum[FRUSTUM_LEFT].d = mFrustum.m[3][3] + mFrustum.m[3][0];

	m_aoFrustum[FRUSTUM_DOWN].n.x = mFrustum.m[0][3] + mFrustum.m[0][1];
	m_aoFrustum[FRUSTUM_DOWN].n.y = mFrustum.m[1][3] + mFrustum.m[1][1];
	m_aoFrustum[FRUSTUM_DOWN].n.z = mFrustum.m[2][3] + mFrustum.m[2][1];
	m_aoFrustum[FRUSTUM_DOWN].d = mFrustum.m[3][3] + mFrustum.m[3][1];

	m_aoFrustum[FRUSTUM_UP].n.x = mFrustum.m[0][3] - mFrustum.m[0][1];
	m_aoFrustum[FRUSTUM_UP].n.y = mFrustum.m[1][3] - mFrustum.m[1][1];
	m_aoFrustum[FRUSTUM_UP].n.z = mFrustum.m[2][3] - mFrustum.m[2][1];
	m_aoFrustum[FRUSTUM_UP].d = mFrustum.m[3][3] - mFrustum.m[3][1];

	m_aoFrustum[FRUSTUM_FAR].n.x = mFrustum.m[0][3] - mFrustum.m[0][2];
	m_aoFrustum[FRUSTUM_FAR].n.y = mFrustum.m[1][3] - mFrustum.m[1][2];
	m_aoFrustum[FRUSTUM_FAR].n.z = mFrustum.m[2][3] - mFrustum.m[2][2];
	m_aoFrustum[FRUSTUM_FAR].d = mFrustum.m[3][3] - mFrustum.m[3][2];

	m_aoFrustum[FRUSTUM_NEAR].n.x = mFrustum.m[0][3] + mFrustum.m[0][2];
	m_aoFrustum[FRUSTUM_NEAR].n.y = mFrustum.m[1][3] + mFrustum.m[1][2];
	m_aoFrustum[FRUSTUM_NEAR].n.z = mFrustum.m[2][3] + mFrustum.m[2][2];
	m_aoFrustum[FRUSTUM_NEAR].d = mFrustum.m[3][3] + mFrustum.m[3][2];

	// Normalize all plane normals
	for(int i = 0; i < 6; i++)
	{
		m_aoFrustum[i].d = -m_aoFrustum[i].d; // Why? I don't know.
		m_aoFrustum[i].Normalize();
	}

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
	glPopAttrib();

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
}

CVar show_frustum("debug_show_frustum", "no");

void CRenderer::FinishRendering()
{
	TPROF("CRenderer::FinishRendering");

	if (show_frustum.GetBool())
	{
		for (size_t i = 0; i < 6; i++)
		{
			Vector vecForward = m_aoFrustum[i].n;
			Vector vecRight = vecForward.Cross(Vector(0, 1, 0)).Normalized();
			Vector vecUp = vecRight.Cross(vecForward).Normalized();
			Vector vecCenter = vecForward * m_aoFrustum[i].d;

			vecForward *= 100;
			vecRight *= 100;
			vecUp *= 100;

			glBegin(GL_QUADS);
				glVertex3fv(vecCenter + vecUp + vecRight);
				glVertex3fv(vecCenter - vecUp + vecRight);
				glVertex3fv(vecCenter - vecUp - vecRight);
				glVertex3fv(vecCenter + vecUp - vecRight);
			glEnd();
		}
	}

	glPopMatrix();
	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	if (ShouldUseFramebuffers())
		glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
    glOrtho(0, m_iWidth, m_iHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	RenderOffscreenBuffers();

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);

	if (ShouldUseFramebuffers())
	{
		if (ShouldUseShaders())
			SetupSceneShader();

		RenderMapFullscreen(m_oSceneBuffer.m_iMap);

		if (ShouldUseShaders())
			ClearProgram();
	}

	RenderFullscreenBuffers();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

void CRenderer::RenderMapFullscreen(size_t iMap)
{
	if (GLEW_ARB_multitexture || GLEW_VERSION_1_3)
		glActiveTexture(GL_TEXTURE0);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, (GLuint)iMap);

	if (ShouldUseFramebuffers())
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);

	glBegin(GL_QUADS);
		glTexCoord2i(0, 1); glVertex2d(0, 0);
		glTexCoord2i(0, 0); glVertex2d(0, m_iHeight);
		glTexCoord2i(1, 0); glVertex2d(m_iWidth, m_iHeight);
		glTexCoord2i(1, 1); glVertex2d(m_iWidth, 0);
	glEnd();
}

void CRenderer::RenderMapToBuffer(size_t iMap, CFrameBuffer* pBuffer)
{
	TAssert(ShouldUseFramebuffers());

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, pBuffer->m_iWidth, pBuffer->m_iHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, (GLuint)iMap);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)pBuffer->m_iFB);
	glViewport(0, 0, (GLsizei)pBuffer->m_iWidth, (GLsizei)pBuffer->m_iHeight);

	glCallList(pBuffer->m_iCallList);

	glBindTexture(GL_TEXTURE_2D, 0);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CRenderer::FrustumOverride(Vector vecPosition, Vector vecTarget, float flFOV, float flNear, float flFar)
{
	m_bFrustumOverride = true;
	m_vecFrustumPosition = vecPosition;
	m_vecFrustumTarget = vecTarget;
	m_flFrustumFOV = flFOV;
	m_flFrustumNear = flNear;
	m_flFrustumFar = flFar;
}

void CRenderer::CancelFrustumOverride()
{
	m_bFrustumOverride = false;
}

void CRenderer::BeginBatching()
{
	if (!ShouldBatchThisFrame())
		return;

	m_bBatching = true;

	for (eastl::map<size_t, eastl::vector<CRenderBatch> >::iterator it = m_aBatches.begin(); it != m_aBatches.end(); it++)
		it->second.clear();
}

void CRenderer::AddToBatch(class CModel* pModel, const Matrix4x4& mTransformations, bool bClrSwap, const Color& clrSwap)
{
	TAssert(pModel);

	if (!pModel)
		return;

	TAssert(pModel->m_iCallListTexture);

	CRenderBatch* pBatch = &m_aBatches[pModel->m_iCallListTexture].push_back();

	pBatch->pModel = pModel;
	pBatch->mTransformation = mTransformations;
	pBatch->bSwap = bClrSwap;
	pBatch->clrSwap = clrSwap;
}

void CRenderer::RenderBatches()
{
	TPROF("CRenderer::RenderBatches");

	m_bBatching = false;

	if (!ShouldBatchThisFrame())
		return;

	glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LIGHTING_BIT|GL_TEXTURE_BIT);
	glPushMatrix();

	GLuint iProgram = 0;
	if (ShouldUseShaders())
	{
		iProgram = (GLuint)CShaderLibrary::GetModelProgram();
		glUseProgram(iProgram);

		GLuint bDiffuse = glGetUniformLocation(iProgram, "bDiffuse");
		glUniform1i(bDiffuse, true);

		GLuint iDiffuse = glGetUniformLocation(iProgram, "iDiffuse");
		glUniform1i(iDiffuse, 0);

		GLuint flAlpha = glGetUniformLocation(iProgram, "flAlpha");
		glUniform1f(flAlpha, 1);
	}

	for (eastl::map<size_t, eastl::vector<CRenderBatch> >::iterator it = m_aBatches.begin(); it != m_aBatches.end(); it++)
	{
		glBindTexture(GL_TEXTURE_2D, (GLuint)it->first);

		for (size_t i = 0; i < it->second.size(); i++)
		{
			CRenderBatch* pBatch = &it->second[i];

			if (ShouldUseShaders())
			{
				GLuint bColorSwapInAlpha = glGetUniformLocation(iProgram, "bColorSwapInAlpha");
				glUniform1i(bColorSwapInAlpha, pBatch->bSwap);

				if (pBatch->bSwap)
				{
					GLuint vecColorSwap = glGetUniformLocation(iProgram, "vecColorSwap");
					Vector vecColor((float)pBatch->clrSwap.r()/255, (float)pBatch->clrSwap.g()/255, (float)pBatch->clrSwap.b()/255);
					glUniform3fv(vecColorSwap, 1, vecColor);
				}

				glColor4f(1, 1, 1, 1);
			}
			else
			{
				if (pBatch->bSwap)
					glColor4f(((float)pBatch->clrSwap.r())/255, ((float)pBatch->clrSwap.g())/255, ((float)pBatch->clrSwap.b())/255, 1);
				else
					glColor4f(1, 1, 1, 1);
			}

			glLoadMatrixf(pBatch->mTransformation);
			glCallList((GLuint)pBatch->pModel->m_iCallList);
		}
	}

	if (ShouldUseShaders())
		glUseProgram(0);

	glPopMatrix();
	glPopAttrib();
}

Vector CRenderer::GetCameraVector()
{
	return (m_vecCameraTarget - m_vecCameraPosition).Normalized();
}

void CRenderer::GetCameraVectors(Vector* pvecForward, Vector* pvecRight, Vector* pvecUp)
{
	Vector vecForward = GetCameraVector();
	Vector vecRight;

	if (pvecForward)
		(*pvecForward) = vecForward;

	if (pvecRight || pvecUp)
		vecRight = vecForward.Cross(Vector(0, 1, 0)).Normalized();

	if (pvecRight)
		(*pvecRight) = vecRight;

	if (pvecUp)
		(*pvecUp) = vecRight.Cross(vecForward).Normalized();
}

bool CRenderer::IsSphereInFrustum(Vector vecCenter, float flRadius)
{
	for (size_t i = 0; i < 6; i++)
	{
		// Why does it subtract d and not add like it should be? Don't know, don't care, it works.
		float flDistance = m_aoFrustum[i].n.x*vecCenter.x + m_aoFrustum[i].n.y*vecCenter.y + m_aoFrustum[i].n.z*vecCenter.z - m_aoFrustum[i].d;
		if (flDistance + flRadius < 0)
			return false;
	}

	return true;
}

void CRenderer::SetSize(int w, int h)
{
	m_iWidth = w;
	m_iHeight = h;
}

void CRenderer::ClearProgram()
{
	TAssert(ShouldUseShaders());

	if (!ShouldUseShaders())
		return;

	glUseProgram(0);
}

void CRenderer::UseProgram(size_t i)
{
	TAssert(ShouldUseShaders());

	if (!ShouldUseShaders())
		return;

	glUseProgram(i);
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

bool CRenderer::HardwareSupportsFramebuffers()
{
	if (m_bHardwareSupportsFramebuffersTestCompleted)
		return m_bHardwareSupportsFramebuffers;

	m_bHardwareSupportsFramebuffersTestCompleted = true;

	if (!GLEW_EXT_framebuffer_object)
	{
		TMsg(_T("EXT_framebuffer_object not supported.\n"));
		m_bHardwareSupportsFramebuffers = false;
		return false;
	}

	// Compile a test framebuffer. If it fails we don't support framebuffers.

	CFrameBuffer oBuffer;

	glGenTextures(1, &oBuffer.m_iMap);
	glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffersEXT(1, &oBuffer.m_iDepth);
	glBindRenderbufferEXT( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
	glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512 );
	glBindRenderbufferEXT( GL_RENDERBUFFER, 0 );

	glGenFramebuffersEXT(1, &oBuffer.m_iFB);
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		TMsg(_T("Test framebuffer compile failed.\n"));
		glDeleteTextures(1, &oBuffer.m_iMap);
		glDeleteRenderbuffersEXT(1, &oBuffer.m_iDepth);
		glDeleteFramebuffersEXT(1, &oBuffer.m_iFB);
		m_bHardwareSupportsFramebuffers = false;
		return false;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	glDeleteTextures(1, &oBuffer.m_iMap);
	glDeleteRenderbuffersEXT(1, &oBuffer.m_iDepth);
	glDeleteFramebuffersEXT(1, &oBuffer.m_iFB);

	m_bHardwareSupportsFramebuffers = true;
	return true;
}

bool CRenderer::HardwareSupportsShaders()
{
	if (m_bHardwareSupportsShadersTestCompleted)
		return m_bHardwareSupportsShaders;

	m_bHardwareSupportsShadersTestCompleted = true;

	if (!GLEW_ARB_fragment_program)
	{
		TMsg(_T("ARB_fragment_program not supported.\n"));
		m_bHardwareSupportsShaders = false;
		return false;
	}

	if (!GLEW_VERSION_2_0)
	{
		TMsg(_T("GL_VERSION_2_0 not supported.\n"));
		m_bHardwareSupportsShaders = false;
		return false;
	}

	// Compile a test shader. If it fails we don't support shaders.
	const char* pszVertexShader =
		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;"
		"	gl_FrontColor = gl_Color;"
		"}";

	const char* pszFragmentShader =
		"void main(void)"
		"{"
		"	gl_FragColor = vec4(1.0,1.0,1.0,1.0);"
		"}";

	GLuint iVShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint iFShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint iProgram = glCreateProgram();

	glShaderSource(iVShader, 1, &pszVertexShader, NULL);
	glCompileShader(iVShader);

	int iVertexCompiled;
	glGetShaderiv(iVShader, GL_COMPILE_STATUS, &iVertexCompiled);


	glShaderSource(iFShader, 1, &pszFragmentShader, NULL);
	glCompileShader(iFShader);

	int iFragmentCompiled;
	glGetShaderiv(iFShader, GL_COMPILE_STATUS, &iFragmentCompiled);


	glAttachShader(iProgram, iVShader);
	glAttachShader(iProgram, iFShader);
	glLinkProgram(iProgram);

	int iProgramLinked;
	glGetProgramiv(iProgram, GL_LINK_STATUS, &iProgramLinked);


	if (iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE)
		m_bHardwareSupportsShaders = true;
	else
		TMsg(_T("Test shader compile failed.\n"));

	glDetachShader(iProgram, iVShader);
	glDetachShader(iProgram, iFShader);
	glDeleteShader(iVShader);
	glDeleteShader(iFShader);
	glDeleteProgram(iProgram);

	return m_bHardwareSupportsShaders;
}

size_t CRenderer::CreateCallList(size_t iModel)
{
	size_t iCallList = glGenLists(1);

	glNewList((GLuint)iCallList, GL_COMPILE);
	CRenderingContext c(NULL);
	c.RenderModel(iModel, CModelLibrary::Get()->GetModel(iModel));
	glEndList();

	return iCallList;
}

size_t CRenderer::LoadTextureIntoGL(tstring sFilename, int iClamp)
{
	if (!sFilename.length())
		return 0;

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilLoadImage(convertstring<tchar, ILchar>(sFilename).c_str());

	if (!bSuccess)
		bSuccess = ilLoadImage(convertstring<tchar, ILchar>(sFilename).c_str());

	ILenum iError = ilGetError();

	if (!bSuccess)
		return 0;

	bSuccess = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!bSuccess)
		return 0;

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);

	if (ImageInfo.Width & (ImageInfo.Width-1))
	{
		//TAssert(!"Image width is not power of 2.");
		ilDeleteImages(1, &iDevILId);
		return 0;
	}

	if (ImageInfo.Height & (ImageInfo.Height-1))
	{
		//TAssert(!"Image height is not power of 2.");
		ilDeleteImages(1, &iDevILId);
		return 0;
	}

	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		iluFlipImage();

	size_t iGLId = LoadTextureIntoGL(iDevILId, iClamp);

	ilBindImage(0);
	ilDeleteImages(1, &iDevILId);

	return iGLId;
}

size_t CRenderer::LoadTextureIntoGL(size_t iImageID, int iClamp)
{
	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (iClamp && !GLEW_EXT_texture_edge_clamp)
		iClamp = 1;

	if (iClamp == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else if (iClamp == 2)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

#ifdef TINKER_OPTIMIZE_SOFTWARE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif

	ilBindImage(iImageID);

	gluBuild2DMipmaps(GL_TEXTURE_2D,
		ilGetInteger(IL_IMAGE_BPP),
		ilGetInteger(IL_IMAGE_WIDTH),
		ilGetInteger(IL_IMAGE_HEIGHT),
		ilGetInteger(IL_IMAGE_FORMAT),
		GL_UNSIGNED_BYTE,
		ilGetData());

	ilBindImage(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	s_iTexturesLoaded++;

	return iGLId;
}

size_t CRenderer::LoadTextureIntoGL(Color* pclrData, int iClamp)
{
	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (iClamp && !GLEW_EXT_texture_edge_clamp)
		iClamp = 1;

	if (iClamp == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else if (iClamp == 2)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

#ifdef TINKER_OPTIMIZE_SOFTWARE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif

	gluBuild2DMipmaps(GL_TEXTURE_2D,
		4,
		256,
		256,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pclrData);

	glBindTexture(GL_TEXTURE_2D, 0);

	s_iTexturesLoaded++;

	return iGLId;
}

void CRenderer::UnloadTextureFromGL(size_t iGLId)
{
	glDeleteTextures(1, &iGLId);
	s_iTexturesLoaded--;
}

size_t CRenderer::s_iTexturesLoaded = 0;

size_t CRenderer::LoadTextureData(tstring sFilename)
{
	if (!sFilename.length())
		return 0;

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilLoadImage(convertstring<tchar, ILchar>(sFilename).c_str());

	if (!bSuccess)
		bSuccess = ilLoadImage(convertstring<tchar, ILchar>(sFilename).c_str());

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

	ilBindImage(0);

	return iDevILId;
}

Color* CRenderer::GetTextureData(size_t iTexture)
{
	if (!iTexture)
		return NULL;

	ilBindImage(iTexture);
	Color* pclr = (Color*)ilGetData();
	ilBindImage(0);
	return pclr;
}

size_t CRenderer::GetTextureWidth(size_t iTexture)
{
	if (!iTexture)
		return 0;

	ilBindImage(iTexture);
	size_t iWidth = ilGetInteger(IL_IMAGE_WIDTH);
	ilBindImage(0);
	return iWidth;
}

size_t CRenderer::GetTextureHeight(size_t iTexture)
{
	if (!iTexture)
		return 0;

	ilBindImage(iTexture);
	size_t iHeight = ilGetInteger(IL_IMAGE_HEIGHT);
	ilBindImage(0);
	return iHeight;
}

void CRenderer::UnloadTextureData(size_t iTexture)
{
	ilBindImage(0);
	ilDeleteImages(1, &iTexture);
}
