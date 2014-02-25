#include "dissolver.h"

#include <maths.h>
#include <mtrand.h>

#include <game/entities/baseentity.h>
#include <models/models.h>
#include <renderer/shaders.h>
#include <game/entities/game.h>
#include <renderer/renderer.h>

CModelDissolver* CModelDissolver::s_pModelDissolver = NULL;
static CModelDissolver g_pModelDissolver = CModelDissolver();

CModelDissolver::CModelDissolver()
{
	s_pModelDissolver = this;

	m_iNumTrianglesAlive = 0;

	m_flLifeTime = 2.0f;
}

void CModelDissolver::AddModel(CBaseEntity* pEntity, Color* pclrSwap, Vector* pvecScale)
{
	if (!Get())
		return;

	if (!CModelLibrary::Get())
		return;

	CModel* pModel = pEntity->GetModel();

	if (!pModel)
		return;

	TStubbed("Dissolver");

	Matrix4x4 mTransform;
	mTransform.SetTranslation(pEntity->GetRenderOrigin());
	mTransform.SetAngles(pEntity->GetRenderAngles());

	if (pvecScale)
	{
		Matrix4x4 mScale;
		mScale.SetScale(*pvecScale);
		mTransform = mTransform * mScale;
	}

	Get()->AddScene(pModel, mTransform, pclrSwap);
}

void CModelDissolver::AddScene(CModel* pModel, const Matrix4x4& mTransform, Color* pclrSwap)
{
	//for (size_t i = 0; i < pModel->m_pScene->GetNumScenes(); i++)
		//AddSceneNode(pModel, pModel->m_pScene->GetScene(i), mTransform, pclrSwap);
}

void CModelDissolver::AddSceneNode(CModel* pModel, CConversionSceneNode* pNode, const Matrix4x4& mTransform, Color* pclrSwap)
{
	//for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		//AddSceneNode(pModel, pNode->GetChild(i), mTransform, pclrSwap);

	//for (size_t i = 0; i < pNode->GetNumMeshInstances(); i++)
		//AddMeshInstance(pModel, pNode->GetMeshInstance(i), mTransform, pclrSwap);
}

void CModelDissolver::AddMeshInstance(CModel* pModel, CConversionMeshInstance* pMeshInstance, const Matrix4x4& mTransform, Color* pclrSwap)
{
#if 0
	for (size_t iFace = 0; iFace < pMeshInstance->GetMesh()->GetNumFaces(); iFace++)
	{
		CConversionFace* pFace = pMeshInstance->GetMesh()->GetFace(iFace);
		CConversionVertex* pV0 = pFace->GetVertex(0);

		for (size_t iVertex = 0; iVertex < pFace->GetNumVertices()-2; iVertex++)
		{
			CConversionVertex* pV1 = pFace->GetVertex(iVertex+1);
			CConversionVertex* pV2 = pFace->GetVertex(iVertex+2);

			if (pFace->m == ~0)
				continue;

			CConversionMaterialMap* pConversionMaterialMap = pMeshInstance->GetMappedMaterial(pFace->m);

			if (!pConversionMaterialMap)
				continue;

			AddTriangle(pMeshInstance, pV0, pV1, pV2, pModel->m_aiTextures[pConversionMaterialMap->m_iMaterial], mTransform, pclrSwap);
		}
	}
#endif
}

void CModelDissolver::AddTriangle(CConversionMeshInstance* pMeshInstance, CConversionVertex* pV0, CConversionVertex* pV1, CConversionVertex* pV2, size_t iTexture, const Matrix4x4& mTransform, Color* pclrSwap)
{
#if 0
	m_iNumTrianglesAlive++;

	Vector v1, v2, v3;
	Vector vu1, vu2, vu3;

	v1 = pMeshInstance->GetVertex(pV0->v);
	v2 = pMeshInstance->GetVertex(pV1->v);
	v3 = pMeshInstance->GetVertex(pV2->v);
	vu1 = pMeshInstance->GetMesh()->GetUV(pV0->vu);
	vu2 = pMeshInstance->GetMesh()->GetUV(pV1->vu);
	vu3 = pMeshInstance->GetMesh()->GetUV(pV2->vu);

	// Translate to "tri space" I guess.
	Vector vecAverage = (v1 + v2 + v3)/3;

	CDissolveTri* pNewTri = NULL;

	for (size_t i = 0; i < m_aTriangles.size(); i++)
	{
		CDissolveTri* pTri = &m_aTriangles[i];

		if (pTri->m_bActive)
			continue;

		pNewTri = pTri;
		break;
	}

	if (!pNewTri)
	{
		m_aTriangles.push_back(CDissolveTri(v1-vecAverage, v2-vecAverage, v3-vecAverage, vu1, vu2, vu3));
		pNewTri = &m_aTriangles[m_aTriangles.size()-1];
	}

	pNewTri->Reset();
	pNewTri->m_iTexture = iTexture;

	pNewTri->m_mPosition.Init(mTransform);

	Matrix4x4 mAverage;
	mAverage.SetTranslation(vecAverage);
	pNewTri->m_mPosition *= mAverage;

	Vector vecVelocity = vecAverage - pMeshInstance->m_pScene->m_oExtends.Center();
	pNewTri->m_mVelocity.SetTranslation(vecVelocity);
	pNewTri->m_mVelocity.SetRotation(EAngle(
		RandomFloat(-90, 90),
		RandomFloat(-180, 180),
		RandomFloat(-90, 90)
		));

	pNewTri->m_bColorSwap = !!pclrSwap;
	if (pNewTri->m_bColorSwap)
		pNewTri->m_clrSwap = *pclrSwap;
#endif
}

void CModelDissolver::Simulate()
{
#if 0
	CModelDissolver* pDissolver = Get();

	float flGameTime = GameServer()->GetGameTime();
	float flFrameTime = GameServer()->GetFrameTime();

	if (!Get()->m_iNumTrianglesAlive)
		return;

	for (size_t i = 0; i < Get()->m_aTriangles.size(); i++)
	{
		CDissolveTri* pTri = &Get()->m_aTriangles[i];

		if (!pTri->m_bActive)
			continue;

		float flLifeTime = flGameTime - pTri->m_flSpawnTime;
		if (flLifeTime > Get()->m_flLifeTime)
		{
			pTri->m_bActive = false;
			Get()->m_iNumTrianglesAlive--;
			continue;
		}

		Vector vecVelocity = pTri->m_mVelocity.GetTranslation();
		pTri->m_mPosition += (vecVelocity * flFrameTime);

		// This is causing odd scaling so it's revoked for the time being.
//		Matrix4x4 mFrameRotation;
//		mFrameRotation.SetRotation((acos((pTri->m_mVelocity.Trace()-1)/2)*180/M_PI)*flFrameTime, pTri->m_mVelocity.RotationAxis());
//		pTri->m_mPosition *= mFrameRotation;

		pTri->m_mVelocity += (Vector(0, 0, 1) * flFrameTime);

		float flLifeTimeRamp = flLifeTime / Get()->m_flLifeTime;

		pTri->m_flAlpha = RemapValClamped(flLifeTimeRamp, 0.5f, 1, 1, 0);
	}
#endif
}

void CModelDissolver::Render()
{
#if 0
	CRenderer* pRenderer = GameServer()->GetRenderer();

	CRenderingContext c(pRenderer);

	c.SetBlend(BLEND_ALPHA);
	c.SetDepthMask(false);
	c.SetBackCulling(false);
	c.SetColor(Color(255, 255, 255, 255));
	if (pRenderer->ShouldUseShaders())
	{
		c.UseProgram(CShaderLibrary::GetModelProgram());
		c.SetUniform("bDiffuse", true);
		c.SetUniform("iDiffuse", 0);
	}

	for (size_t i = 0; i < Get()->m_aTriangles.size(); i++)
	{
		CDissolveTri* pTri = &Get()->m_aTriangles[i];

		if (!pTri->m_bActive)
			continue;

		c.BindTexture(pTri->m_iTexture);

		if (pRenderer->ShouldUseShaders())
		{
			c.SetUniform("flAlpha", pTri->m_flAlpha);

			c.SetUniform("bColorSwapInAlpha", pTri->m_bColorSwap);
			c.SetUniform("vecColorSwap", pTri->m_clrSwap);
		}

		c.BeginRenderTris();

		c.TexCoord(pTri->vu1);
		c.Vertex(pTri->m_mPosition * pTri->v1);
		c.TexCoord(pTri->vu2);
		c.Vertex(pTri->m_mPosition * pTri->v2);
		c.TexCoord(pTri->vu3);
		c.Vertex(pTri->m_mPosition * pTri->v3);

		c.EndRender();
	}
#endif
}

CDissolveTri::CDissolveTri(const Vector& _v1, const Vector& _v2, const Vector& _v3, const Vector& _vu1, const Vector& _vu2, const Vector& _vu3)
{
	Reset();

	v1 = _v1;
	v2 = _v2;
	v3 = _v3;
	vu1 = _vu1;
	vu2 = _vu2;
	vu3 = _vu3;

	m_bColorSwap = false;
}

void CDissolveTri::Reset()
{
	m_flAlpha = 1;
	m_bActive = true;
	m_flSpawnTime = GameServer()->GetGameTime();
	m_mPosition.Identity();
	m_mVelocity.Identity();
}
