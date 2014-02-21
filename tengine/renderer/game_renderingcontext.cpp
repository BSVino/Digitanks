#include "game_renderingcontext.h"

#include <maths.h>
#include <simplex.h>

#include <models/models.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <game/gameserver.h>
#include <textures/texturelibrary.h>
#include <renderer/renderer.h>
#include <toys/toy.h>
#include <textures/materiallibrary.h>
#include <tools/workbench.h>
#include <game/entities/game.h>
#include <game/entities/character.h>

#include "game_renderer.h"

CGameRenderingContext::CGameRenderingContext(CGameRenderer* pRenderer, bool bInherit)
	: CRenderingContext(pRenderer, bInherit)
{
	m_pRenderer = pRenderer;
}

void CGameRenderingContext::RenderModel(size_t iModel, const CBaseEntity* pEntity)
{
	CModel* pModel = CModelLibrary::GetModel(iModel);

	if (!pModel)
		return;

	bool bBatchThis = true;
	if (!m_pRenderer->IsBatching())
		bBatchThis = false;
	else if (m_pRenderer && GetContext().m_pFrameBuffer != m_pRenderer->GetSceneBuffer())
		bBatchThis = false;

	if (bBatchThis)
	{
		TAssert(GetContext().m_eBlend == BLEND_NONE);

		m_pRenderer->AddToBatch(pModel, pEntity, GetContext().m_mTransformations, m_clrRender, GetContext().m_bWinding);
	}
	else
	{
		m_pRenderer->m_pRendering = pEntity;

		CRenderContext& oContext = GetContext();

		for (size_t m = 0; m < pModel->m_aiVertexBuffers.size(); m++)
		{
			if (!pModel->m_aiVertexBufferSizes[m])
				continue;

			CMaterialHandle& hMaterial = pModel->m_ahMaterials[m];

			if (!hMaterial)
				continue;

			UseMaterial(hMaterial);

			if ((!m_pRenderer->IsRenderingTransparent()) ^ (oContext.m_eBlend == BLEND_NONE))
				continue;

			if (pEntity)
			{
				if (!pEntity->ModifyShader(this))
					continue;
			}

			if (!m_pRenderer->ModifyShader(pEntity, this))
				continue;

			RenderModel(pModel, m);
		}

		m_pRenderer->m_pRendering = nullptr;
	}

	if (pModel->m_pToy && pModel->m_pToy->GetNumSceneAreas())
	{
		size_t iSceneArea = m_pRenderer->GetSceneAreaPosition(pModel);

		auto pLocalPlayer = Game()?Game()->GetLocalPlayer():nullptr;
		CCharacter* pCharacter = pLocalPlayer?pLocalPlayer->GetCharacter():nullptr;
		bool bNoClip = pCharacter?pCharacter->GetNoClip():false;

		if (CWorkbench::IsActive() || bNoClip || iSceneArea >= pModel->m_pToy->GetNumSceneAreas())
		{
			for (size_t i = 0; i < pModel->m_pToy->GetNumSceneAreas(); i++)
			{
				AABB aabbBounds = pModel->m_pToy->GetSceneAreaAABB(i);
				if (!m_pRenderer->IsSphereInFrustum(aabbBounds.Center(), aabbBounds.Size().Length()/2))
					continue;

				RenderModel(CModelLibrary::FindModel(pModel->m_pToy->GetSceneAreaFileName(i)), pEntity);
			}
		}
		else
		{
			for (size_t i = 0; i < pModel->m_pToy->GetSceneAreaNumVisible(iSceneArea); i++)
			{
				size_t iSceneAreaToRender = pModel->m_pToy->GetSceneAreasVisible(iSceneArea, i);

				AABB aabbBounds = pModel->m_pToy->GetSceneAreaAABB(iSceneAreaToRender);
				if (!m_pRenderer->IsSphereInFrustum(aabbBounds.Center(), aabbBounds.Size().Length()/2))
					continue;

				RenderModel(CModelLibrary::FindModel(pModel->m_pToy->GetSceneAreaFileName(iSceneAreaToRender)), pEntity);
			}
		}
	}
}

#define BUFFER_OFFSET(i) (size_t)((char *)NULL + (i))

void CGameRenderingContext::RenderModel(CModel* pModel, size_t iMaterial)
{
	SetUniform("vecColor", m_clrRender);

	SetUniform("bNormalsAvailable", !!(pModel->m_pToy->GetVertexNormalOffsetInBytes(iMaterial) > 0));

	TAssert(m_pShader);
	if (!pModel || !m_pShader)
		return;

	BeginRenderVertexArray(pModel->m_aiVertexBuffers[iMaterial]);
	if (pModel->m_pToy)
	{
		SetPositionBuffer((size_t)0u, pModel->m_pToy->GetVertexSizeInBytes(iMaterial));
		if (pModel->m_pToy->GetVertexUVOffsetInBytes(iMaterial) > 0)
			SetTexCoordBuffer(BUFFER_OFFSET(pModel->m_pToy->GetVertexUVOffsetInBytes(iMaterial)), pModel->m_pToy->GetVertexSizeInBytes(iMaterial));
		if (pModel->m_pToy->GetVertexNormalOffsetInBytes(iMaterial) > 0)
			SetNormalsBuffer(BUFFER_OFFSET(pModel->m_pToy->GetVertexNormalOffsetInBytes(iMaterial)), pModel->m_pToy->GetVertexSizeInBytes(iMaterial));
		if (pModel->m_pToy->GetVertexTangentOffsetInBytes(iMaterial) > 0)
			SetTangentsBuffer(BUFFER_OFFSET(pModel->m_pToy->GetVertexTangentOffsetInBytes(iMaterial)), pModel->m_pToy->GetVertexSizeInBytes(iMaterial));
		if (pModel->m_pToy->GetVertexBitangentOffsetInBytes(iMaterial) > 0)
			SetBitangentsBuffer(BUFFER_OFFSET(pModel->m_pToy->GetVertexBitangentOffsetInBytes(iMaterial)), pModel->m_pToy->GetVertexSizeInBytes(iMaterial));
	}
	else
	{
		// If there's no toy then we are using fixed size source models.
		SetPositionBuffer((size_t)0u, 4*FIXED_FLOATS_PER_VERTEX);
		SetTexCoordBuffer(BUFFER_OFFSET(4*FIXED_UV_OFFSET), 4*FIXED_FLOATS_PER_VERTEX);
	}
	EndRenderVertexArray(pModel->m_aiVertexBufferSizes[iMaterial]);
}

void CGameRenderingContext::RenderMaterialModel(const CMaterialHandle& hMaterial, const class CBaseEntity* pEntity)
{
	TAssert(hMaterial.IsValid());
	if (!hMaterial.IsValid())
		return;

	if (!hMaterial->m_ahTextures.size())
		return;

	CTextureHandle hBaseTexture = hMaterial->m_ahTextures[0];

	if (!hBaseTexture)
		return;

	Vector vecLeft = Vector(0, 0.5f, 0) * ((float)hBaseTexture->m_iWidth/hMaterial->m_iTexelsPerMeter);
	Vector vecUp = Vector(0, 0, 0.5f) * ((float)hBaseTexture->m_iHeight/hMaterial->m_iTexelsPerMeter);

	UseMaterial(hMaterial);
	m_pRenderer->ModifyShader(pEntity, this);

	SetUniform("vecColor", m_clrRender);

	BeginRenderTriFan();
		TexCoord(0.0f, 1.0f);
		Vertex(-vecLeft + vecUp);
		TexCoord(0.0f, 0.0f);
		Vertex(-vecLeft - vecUp);
		TexCoord(1.0f, 0.0f);
		Vertex(vecLeft - vecUp);
		TexCoord(1.0f, 1.0f);
		Vertex(vecLeft + vecUp);
	EndRender();
}

void CGameRenderingContext::RenderBillboard(const tstring& sMaterial, float flRadius)
{
	Vector vecLeft, vecUp;
	m_pRenderer->GetCameraVectors(NULL, &vecLeft, &vecUp);

	BaseClass::RenderBillboard(sMaterial, flRadius, vecLeft, vecUp);
}
