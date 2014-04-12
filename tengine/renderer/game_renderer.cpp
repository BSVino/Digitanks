#include "game_renderer.h"

#include <tsort.h>
#include <maths.h>
#include <simplex.h>

#include <common/worklistener.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <game/gameserver.h>
#include <textures/texturelibrary.h>
#include <game/cameramanager.h>
#include <physics/physics.h>
#include <toys/toy.h>
#include <textures/materiallibrary.h>
#include <renderer/particles.h>
#include <tools/workbench.h>
#include <game/entities/game.h>
#include <game/entities/weapon.h>
#include <game/entities/character.h>

#include "game_renderingcontext.h"

CVar r_batch("r_batch", "1");

CGameRenderer::CGameRenderer(size_t iWidth, size_t iHeight)
	: CRenderer(iWidth, iHeight)
{
	TMsg("Initializing game renderer\n");

	m_bBatching = false;
	m_bDrawBackground = false;

	m_bBatchThisFrame = r_batch.GetBool();

	DisableSkybox();

	m_pRendering = nullptr;

	const int iSize = 64;
	Vector aclrInvalid[iSize*iSize];
	for (size_t i = 0; i < iSize; i++)
	{
		for (size_t j = 0; j < iSize; j++)
		{
			if ((i/4+j/4)%2 == 0)
				aclrInvalid[i*iSize+j] = Vector(1, 0, 1);
			else
				aclrInvalid[i*iSize+j] = Vector(0, 0, 0);
		}
	}

	m_hInvalidTexture = CTextureLibrary::AddTexture(aclrInvalid, iSize, iSize);
}

CVar r_cullfrustum("r_frustumculling", "on");

void CGameRenderer::Render()
{
	TPROF("CGameRenderer::Render");

	// Must delay this setup until the shaders are loaded.
	if (!m_hInvalidMaterial.IsValid())
	{
		m_hInvalidMaterial = CMaterialLibrary::CreateBlankMaterial("invalid");
		m_hInvalidMaterial->SetShader("model");
		m_hInvalidMaterial->SetParameter("DiffuseTexture", m_hInvalidTexture);
	}

	CCameraManager* pCamera = GameServer()->GetCameraManager();

	SetCameraPosition(pCamera->GetCameraPosition());
	SetCameraDirection(pCamera->GetCameraDirection());
	SetCameraUp(pCamera->GetCameraUp());
	SetCameraFOV(pCamera->GetCameraFOV());
	SetCameraOrthoHeight(pCamera->GetCameraOrthoHeight());
	SetCameraNear(pCamera->GetCameraNear());
	SetCameraFar(pCamera->GetCameraFar());
	SetRenderOrthographic(pCamera->ShouldRenderOrthographic());
	SetCustomProjection(pCamera->UseCustomProjection());
	SetCustomProjection(pCamera->GetCustomProjection());

	PreRender();

	{
		CGameRenderingContext c(this);
		ModifyContext(&c);
		SetupFrame(&c);
		StartRendering(&c);

		if (CWorkbench::IsActive())
			CWorkbench::RenderScene();
		else
			RenderEverything();

		FinishRendering(&c);
		FinishFrame(&c);
	}

	PostRender();
}

static Vector g_vecNearPlanePoint;
static Vector g_vecNearPlaneNormal;

bool DistanceCompare(CBaseEntity* a, CBaseEntity* b)
{
	float flADistance = DistanceToPlaneSqr(a->BaseGetRenderOrigin(), g_vecNearPlanePoint, g_vecNearPlaneNormal);
	float flBDistance = DistanceToPlaneSqr(b->BaseGetRenderOrigin(), g_vecNearPlanePoint, g_vecNearPlaneNormal);

	return flADistance > flBDistance;
}

void CGameRenderer::RenderEverything()
{
	m_apRenderOpaqueList.reserve(CBaseEntity::GetNumEntities());
	m_apRenderOpaqueList.clear();

	m_apRenderTransparentList.reserve(CBaseEntity::GetNumEntities());
	m_apRenderTransparentList.clear();

	bool bFrustumCulling = r_cullfrustum.GetBool() && ShouldCullByFrustum();

	// None of these had better get deleted while we're doing this since they're not handles.
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		if (!pEntity->IsVisible())
			continue;

		bool bRenderOpaque = pEntity->ShouldRender();
		bool bRenderTransparent = pEntity->ShouldRenderTransparent();

		if (!bRenderOpaque && !bRenderTransparent)
			continue;

		if (bFrustumCulling && !IsSphereInFrustum(pEntity->GetRenderCenter(), (float)pEntity->GetBoundingRadius()))
			continue;

		if (bRenderOpaque)
			m_apRenderOpaqueList.push_back(pEntity);

		if (bRenderTransparent)
			m_apRenderTransparentList.push_back(pEntity);
	}

	m_bRenderingTransparent = false;

	BeginBatching();

	// First render all opaque objects
	size_t iEntites = m_apRenderOpaqueList.size();
	for (size_t i = 0; i < iEntites; i++)
		m_apRenderOpaqueList[i]->Render();

	RenderBatches();

	m_bRenderingTransparent = true;

	// Used in DistanceCompare()
	g_vecNearPlanePoint = GameServer()->GetCameraManager()->GetCameraPosition() + GameServer()->GetCameraManager()->GetCameraDirection() * GameServer()->GetCameraManager()->GetCameraNear();
	g_vecNearPlaneNormal = GameServer()->GetCameraManager()->GetCameraDirection();

	sort(m_apRenderTransparentList.begin(), m_apRenderTransparentList.end(), DistanceCompare);

	// Now render all transparent objects.
	iEntites = m_apRenderTransparentList.size();
	for (size_t i = 0; i < iEntites; i++)
		m_apRenderTransparentList[i]->RenderTransparent();

	if (ShouldRenderParticles())
		CParticleSystemLibrary::Render();

	m_bRenderingTransparent = false; // Just in case stuff is drawn in the HUD
}

void CGameRenderer::SetupFrame(class CRenderingContext* pContext)
{
	TPROF("CGameRenderer::SetupFrame");

	m_bBatchThisFrame = r_batch.GetBool();

	BaseClass::SetupFrame(pContext);

	if (m_hSkyboxFT.IsValid())
		DrawSkybox(pContext);
}

void CGameRenderer::DrawSkybox(class CRenderingContext* pContext)
{
	TPROF("CGameRenderer::DrawSkybox");

	CCameraManager* pCamera = GameServer()->GetCameraManager();

	SetCameraPosition(pCamera->GetCameraPosition());
	SetCameraDirection(pCamera->GetCameraDirection());
	SetCameraUp(pCamera->GetCameraUp());
	SetCameraFOV(pCamera->GetCameraFOV());
	SetCameraOrthoHeight(pCamera->GetCameraOrthoHeight());
	SetCameraNear(pCamera->GetCameraNear());
	SetCameraFar(pCamera->GetCameraFar());

	CRenderingContext c(this, true);

	c.SetProjection(Matrix4x4::ProjectPerspective(
			m_flCameraFOV,
			(float)m_iViewportWidth/(float)m_iViewportHeight,
			m_flCameraNear,
			m_flCameraFar
		));

	c.SetView(Matrix4x4::ConstructCameraView(Vector(0, 0, 0), m_vecCameraDirection, m_vecCameraUp));
	c.ResetTransformations();

	c.SetDepthTest(false);
	c.UseProgram("skybox");
	c.SetUniform("iDiffuse", 0);

	ModifySkyboxContext(&c);

	c.BeginRenderVertexArray();
	c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
	c.SetPositionBuffer(&m_avecSkyboxFT[0][0]);
	c.BindTexture(m_hSkyboxFT->m_iGLID);
	c.EndRenderVertexArray(6);

	c.BeginRenderVertexArray();
	c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
	c.SetPositionBuffer(&m_avecSkyboxBK[0][0]);
	c.BindTexture(m_hSkyboxBK->m_iGLID);
	c.EndRenderVertexArray(6);

	c.BeginRenderVertexArray();
	c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
	c.SetPositionBuffer(&m_avecSkyboxLF[0][0]);
	c.BindTexture(m_hSkyboxLF->m_iGLID);
	c.EndRenderVertexArray(6);

	c.BeginRenderVertexArray();
	c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
	c.SetPositionBuffer(&m_avecSkyboxRT[0][0]);
	c.BindTexture(m_hSkyboxRT->m_iGLID);
	c.EndRenderVertexArray(6);

	c.BeginRenderVertexArray();
	c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
	c.SetPositionBuffer(&m_avecSkyboxUP[0][0]);
	c.BindTexture(m_hSkyboxUP->m_iGLID);
	c.EndRenderVertexArray(6);

	c.BeginRenderVertexArray();
	c.SetTexCoordBuffer(&m_avecSkyboxTexCoords[0][0]);
	c.SetPositionBuffer(&m_avecSkyboxDN[0][0]);
	c.BindTexture(m_hSkyboxDN->m_iGLID);
	c.EndRenderVertexArray(6);

	c.ClearDepth();
}

CVar phys_show("phys_show", "no");

void CGameRenderer::FinishRendering(class CRenderingContext* pContext)
{
	BaseClass::FinishRendering(pContext);

	if (phys_show.GetBool() && ShouldRenderPhysicsDebug())
	{
		if (CWorkbench::IsActive())
			EditorPhysics()->DebugDraw((physics_debug_t)phys_show.GetInt());
		else
			GamePhysics()->DebugDraw((physics_debug_t)phys_show.GetInt());
	}
}

void CGameRenderer::FinishFrame(CRenderingContext* pContext)
{
	DrawWeaponViewModel();

	BaseClass::FinishFrame(pContext);
}

void CGameRenderer::DrawWeaponViewModel()
{
	if (!GameServer()->GetGame()->GetNumLocalPlayers())
		return;

	CPlayer* pLocalPlayer = GameServer()->GetGame()->GetLocalPlayer();
	if (!pLocalPlayer)
		return;

	CCharacter* pLocalCharacter = pLocalPlayer->GetCharacter();
	if (!pLocalCharacter)
		return;

	CBaseWeapon* pEquippedWeapon = pLocalCharacter->GetEquippedWeapon();
	if (!pEquippedWeapon)
		return;

	CGameRenderingContext c(this, true);

	c.SetProjection(Matrix4x4::ProjectPerspective(
			m_flCameraFOV,
			(float)m_iViewportWidth / (float)m_iViewportHeight,
			0.001f,
			1
		));

	c.SetView(Matrix4x4::ConstructCameraView(Vector(0, 0, 0), m_vecCameraDirection, m_vecCameraUp));
	c.ResetTransformations();

	c.ClearDepth();

	pEquippedWeapon->DrawViewModel(&c);
}

void CGameRenderer::SetSkybox(const CTextureHandle& ft, const CTextureHandle& bk, const CTextureHandle& lf, const CTextureHandle& rt, const CTextureHandle& up, const CTextureHandle& dn)
{
	m_hSkyboxFT = ft;
	m_hSkyboxLF = lf;
	m_hSkyboxBK = bk;
	m_hSkyboxRT = rt;
	m_hSkyboxDN = dn;
	m_hSkyboxUP = up;

	m_avecSkyboxTexCoords[0] = Vector2D(0, 0);
	m_avecSkyboxTexCoords[1] = Vector2D(1, 0);
	m_avecSkyboxTexCoords[2] = Vector2D(1, 1);
	m_avecSkyboxTexCoords[3] = Vector2D(0, 0);
	m_avecSkyboxTexCoords[4] = Vector2D(1, 1);
	m_avecSkyboxTexCoords[5] = Vector2D(0, 1);

	m_avecSkyboxFT[0] = Vector(100, 100, -100);
	m_avecSkyboxFT[1] = Vector(100, -100, -100);
	m_avecSkyboxFT[2] = Vector(100, -100, 100);
	m_avecSkyboxFT[3] = Vector(100, 100, -100);
	m_avecSkyboxFT[4] = Vector(100, -100, 100);
	m_avecSkyboxFT[5] = Vector(100, 100, 100);

	m_avecSkyboxBK[0] = Vector(-100, -100, -100);
	m_avecSkyboxBK[1] = Vector(-100, 100, -100);
	m_avecSkyboxBK[2] = Vector(-100, 100, 100);
	m_avecSkyboxBK[3] = Vector(-100, -100, -100);
	m_avecSkyboxBK[4] = Vector(-100, 100, 100);
	m_avecSkyboxBK[5] = Vector(-100, -100, 100);

	m_avecSkyboxLF[0] = Vector(-100, 100, -100);
	m_avecSkyboxLF[1] = Vector(100, 100, -100);
	m_avecSkyboxLF[2] = Vector(100, 100, 100);
	m_avecSkyboxLF[3] = Vector(-100, 100, -100);
	m_avecSkyboxLF[4] = Vector(100, 100, 100);
	m_avecSkyboxLF[5] = Vector(-100, 100, 100);

	m_avecSkyboxRT[0] = Vector(100, -100, -100);
	m_avecSkyboxRT[1] = Vector(-100, -100, -100);
	m_avecSkyboxRT[2] = Vector(-100, -100, 100);
	m_avecSkyboxRT[3] = Vector(100, -100, -100);
	m_avecSkyboxRT[4] = Vector(-100, -100, 100);
	m_avecSkyboxRT[5] = Vector(100, -100, 100);

	m_avecSkyboxUP[0] = Vector(100, 100, 100);
	m_avecSkyboxUP[1] = Vector(100, -100, 100);
	m_avecSkyboxUP[2] = Vector(-100, -100, 100);
	m_avecSkyboxUP[3] = Vector(100, 100, 100);
	m_avecSkyboxUP[4] = Vector(-100, -100, 100);
	m_avecSkyboxUP[5] = Vector(-100, 100, 100);

	m_avecSkyboxDN[0] = Vector(-100, 100, -100);
	m_avecSkyboxDN[1] = Vector(-100, -100, -100);
	m_avecSkyboxDN[2] = Vector(100, -100, -100);
	m_avecSkyboxDN[3] = Vector(-100, 100, -100);
	m_avecSkyboxDN[4] = Vector(100, -100, -100);
	m_avecSkyboxDN[5] = Vector(100, 100, -100);
}

void CGameRenderer::DisableSkybox()
{
	m_hSkyboxFT.Reset();
}

void CGameRenderer::BeginBatching()
{
	if (!ShouldBatchThisFrame())
		return;

	m_bBatching = true;

	for (auto it = m_aBatches.begin(); it != m_aBatches.end(); it++)
		it->second.clear();
}

void CGameRenderer::AddToBatch(class CModel* pModel, const CBaseEntity* pEntity, const Matrix4x4& mTransformations, const Color& clrRender, bool bWinding)
{
	TAssert(pModel);
	if (!pModel)
		return;

	TAssert(pEntity);
	if (!pEntity)
		return;

	for (size_t i = 0; i < pModel->m_ahMaterials.size(); i++)
	{
		CRenderBatch* pBatch = &m_aBatches[pModel->m_ahMaterials[i]].push_back();

		pBatch->pEntity = pEntity;
		pBatch->pModel = pModel;
		pBatch->mTransformation = mTransformations;
		pBatch->bWinding = bWinding;
		pBatch->clrRender = clrRender;
		pBatch->iMaterial = i;
	}
}

void CGameRenderer::RenderBatches()
{
	TPROF("CGameRenderer::RenderBatches");

	m_bBatching = false;

	if (!ShouldBatchThisFrame())
		return;

	CGameRenderingContext c(this, true);
	c.UseFrameBuffer(GetSceneBuffer());

	for (auto it = m_aBatches.begin(); it != m_aBatches.end(); it++)
	{
		size_t iJobs = it->second.size();
		if (!iJobs)
			continue;

		CMaterialHandle hMaterial = it->first;

		if (!hMaterial)
		{
			if (GetInvalidMaterial().IsValid())
				hMaterial = GetInvalidMaterial();
			else
				continue;
		}

		if (!hMaterial)
			continue;

		c.UseMaterial(hMaterial);

		if (IsRenderingTransparent() && hMaterial->m_sBlend == "")
			continue;

		if (!IsRenderingTransparent() && hMaterial->m_sBlend != "")
			continue;

		for (size_t i = 0; i < iJobs; i++)
		{
			CRenderBatch* pBatch = &it->second[i];

			c.SetWinding(pBatch->bWinding);

			c.ResetTransformations();
			c.LoadTransform(pBatch->mTransformation);

			c.SetColor(pBatch->clrRender);

			m_pRendering = pBatch->pEntity;
			m_pRendering->ModifyShader(&c);
			ModifyShader(m_pRendering, &c);
			c.RenderModel(pBatch->pModel, pBatch->iMaterial);
			m_pRendering = nullptr;
		}
	}
}

void CGameRenderer::ClassifySceneAreaPosition(CModel* pModel)
{
	if (!pModel->m_pToy)
		return;

	if (!pModel->m_pToy->GetNumSceneAreas())
		return;

	auto it = m_aiCurrentSceneAreas.find(pModel->m_sFilename);
	if (it == m_aiCurrentSceneAreas.end())
	{
		// No entry? 
		FindSceneAreaPosition(pModel);
		return;
	}

	if (it->second >= pModel->m_pToy->GetNumSceneAreas())
	{
		FindSceneAreaPosition(pModel);
		return;
	}

	if (pModel->m_pToy->GetSceneAreaAABB(it->second).Inside(m_vecCameraPosition))
		return;

	FindSceneAreaPosition(pModel);
}

size_t CGameRenderer::GetSceneAreaPosition(CModel* pModel)
{
	auto it = m_aiCurrentSceneAreas.find(pModel->m_sFilename);

	if (it == m_aiCurrentSceneAreas.end())
		return ~0;

	return it->second;
}

void CGameRenderer::FindSceneAreaPosition(CModel* pModel)
{
	for (size_t i = 0; i < pModel->m_pToy->GetNumSceneAreas(); i++)
	{
		if (pModel->m_pToy->GetSceneAreaAABB(i).Inside(m_vecCameraPosition))
		{
			m_aiCurrentSceneAreas[pModel->m_sFilename] = i;
			return;
		}
	}

	// If there's no entry for this model yet, find the closest.
	if (m_aiCurrentSceneAreas.find(pModel->m_sFilename) == m_aiCurrentSceneAreas.end())
	{
		size_t iClosest = 0;
		for (size_t i = 1; i < pModel->m_pToy->GetNumSceneAreas(); i++)
		{
			if (pModel->m_pToy->GetSceneAreaAABB(i).Center().DistanceSqr(m_vecCameraPosition) < pModel->m_pToy->GetSceneAreaAABB(iClosest).Center().DistanceSqr(m_vecCameraPosition))
				iClosest = i;
		}

		m_aiCurrentSceneAreas[pModel->m_sFilename] = iClosest;
		return;
	}

	// Otherwise if we don't find one don't fuck with it. We'll consider ourselves to still be in the previous one.
}
