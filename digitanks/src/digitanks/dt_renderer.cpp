#include "dt_renderer.h"

#include <maths.h>

#include <renderer/shaders.h>
#include <models/models.h>
#include <textures/texturelibrary.h>
#include <renderer/shaders.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/roperenderer.h>
#include <glgui/rootpanel.h>

#include <digitanksentity.h>
#include <digitanksgame.h>
#include <ui/digitankswindow.h>
#include <dt_camera.h>

CDigitanksRenderer::CDigitanksRenderer()
	: CGameRenderer(DigitanksWindow()->GetWindowWidth(), DigitanksWindow()->GetWindowHeight()),
#ifdef _DEBUG
	m_oDebugBuffer("debug"),
#endif
	m_oExplosionBuffer("explosion"), m_oVisibility1Buffer("vis1"), m_oVisibility2Buffer("vis2"),
	m_oVisibilityMaskedBuffer("vismasked"), m_oAvailableAreaBuffer("availablearea")
{
}

void CDigitanksRenderer::Initialize()
{
	BaseClass::Initialize();

	DigitanksWindow()->RenderLoading();

	m_hVignetting = CTextureLibrary::AddTexture("textures/vignetting.png");

	SetSkybox(
		CTextureLibrary::AddTexture("textures/skybox/standard-ft.png", 1),
		CTextureLibrary::AddTexture("textures/skybox/standard-bk.png", 1),
		CTextureLibrary::AddTexture("textures/skybox/standard-lf.png", 1),
		CTextureLibrary::AddTexture("textures/skybox/standard-rt.png", 1),
		CTextureLibrary::AddTexture("textures/skybox/standard-up.png", 1),
		CTextureLibrary::AddTexture("textures/skybox/standard-dn.png", 1)
		);

	m_oExplosionBuffer = CreateFrameBuffer("explosion", m_iViewportWidth, m_iViewportHeight, (fb_options_e)(FB_TEXTURE|FB_SCENE_DEPTH));
	m_oVisibility1Buffer = CreateFrameBuffer("vis1", m_iViewportWidth, m_iViewportHeight, (fb_options_e)(FB_TEXTURE | FB_SCENE_DEPTH));
	m_oVisibility2Buffer = CreateFrameBuffer("vis2", m_iViewportWidth, m_iViewportHeight, (fb_options_e)(FB_TEXTURE));
	m_oVisibilityMaskedBuffer = CreateFrameBuffer("vismasked", m_iViewportWidth, m_iViewportHeight, (fb_options_e)(FB_TEXTURE | FB_SCENE_DEPTH));
	m_oAvailableAreaBuffer = CreateFrameBuffer("availablearea", m_iViewportWidth, m_iViewportHeight, (fb_options_e)(FB_TEXTURE));

#ifdef _DEBUG
	m_oDebugBuffer = CreateFrameBuffer("debug", m_iViewportWidth, m_iViewportHeight, (fb_options_e)(FB_TEXTURE));
#endif

	m_hNoise = CTextureLibrary::AddTexture("textures/noise.png");

	m_iRing1 = CModelLibrary::Get()->AddModel("models/skybox/ring1.toy");
	m_iRing2 = CModelLibrary::Get()->AddModel("models/skybox/ring2.toy");
	m_iRing3 = CModelLibrary::Get()->AddModel("models/skybox/ring3.toy");
	m_flRing1Yaw = 0;
	m_flRing2Yaw = 90;
	m_flRing3Yaw = 190;

	m_iVortex = CModelLibrary::Get()->AddModel("models/skybox/vortex.toy");
	m_flVortexYaw = 0;

	m_iDigiverse = CModelLibrary::Get()->AddModel("models/skybox/digiverse.toy");
	m_iFloaters[0] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float01.toy");
	m_iFloaters[1] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float02.toy");
	m_iFloaters[2] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float03.toy");
	m_iFloaters[3] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float04.toy");
	m_iFloaters[4] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float05.toy");
	m_iFloaters[5] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float06.toy");
	m_iFloaters[6] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float07.toy");
	m_iFloaters[7] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float08.toy");
	m_iFloaters[8] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float09.toy");
	m_iFloaters[9] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float10.toy");
	m_iFloaters[10] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float11.toy");
	m_iFloaters[11] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float12.toy");
	m_iFloaters[12] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float13.toy");
	m_iFloaters[13] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float14.toy");
	m_iFloaters[14] = CModelLibrary::Get()->AddModel("models/skybox/floaters/float15.toy");

	m_flLastBloomPulse = -100;
}

void CDigitanksRenderer::SetupFrame(class CRenderingContext* pContext)
{
	TPROF("CDigitanksRenderer::SetupFrame");

	{
		CRenderingContext c(this);

		c.UseFrameBuffer(&m_oExplosionBuffer);
		c.ClearColor();

		c.UseFrameBuffer(&m_oVisibility2Buffer);
		c.ClearColor();

		c.UseFrameBuffer(&m_oVisibilityMaskedBuffer);
		c.ClearColor();

		c.UseFrameBuffer(&m_oAvailableAreaBuffer);
		c.ClearColor();
	}

	pContext->UseFrameBuffer(&m_oSceneBuffer);

	BaseClass::SetupFrame(pContext);
}

void CDigitanksRenderer::StartRendering(class CRenderingContext* pContext)
{
	TPROF("CDigitanksRenderer::StartRendering");

	ClearTendrilBatches();

	BaseClass::StartRendering(pContext);
}

void CDigitanksRenderer::DrawSkybox(class CRenderingContext* pContext)
{
	if (!DigitanksGame())
		return;

	if (DigitanksGame()->GetGameType() == GAMETYPE_MENU || DigitanksGame()->GetGameType() == GAMETYPE_EMPTY)
	{
		BaseClass::DrawBackground(pContext);
		return;
	}

	TPROF("CDigitanksRenderer::DrawSkybox");

	BaseClass::DrawSkybox(pContext);

	m_bRenderingTransparent = true;

	CGameRenderingContext r(this, true);

	r.Scale(1.0f/16, 1.0f/16, 1.0f/16);

#ifndef TINKER_OPTIMIZE_SOFTWARE
	if (true)
	{
		CGameRenderingContext r(this, true);
		r.SetBlend(BLEND_ALPHA);
		r.Rotate(m_flVortexYaw, Vector(0, 0, 1));
		r.RenderModel(m_iVortex);

		r.SetBlend(BLEND_ADDITIVE);
		r.ResetTransformations();
		r.Translate(Vector(0, 1, 0));
		r.Rotate(-m_flVortexYaw, Vector(0, 0, 1));
		r.RenderModel(m_iVortex);

		m_flVortexYaw -= (float)GameServer()->GetFrameTime()*2;
	}

	if (true)
	{
		CGameRenderingContext r(this, true);
		r.SetBlend(BLEND_ALPHA);
		r.RenderModel(m_iDigiverse);
	}

	m_bRenderingTransparent = false;

	if (true)
	{
		float flGameTime = (float)GameServer()->GetGameTime();
		CGameRenderingContext r(this, true);

		r.Translate(Vector(-20.6999f, 1.0f, 74.3044f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 4.0f), 0.2f), Gain(Oscillate(flGameTime, 5.0f), 0.2f), Gain(Oscillate(flGameTime, 6.0f), 0.2f)));
		r.RenderModel(m_iFloaters[0]);
		r.ResetTransformations();

		r.Translate(Vector(23.2488f, 1.0f, 72.435f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 6.0f), 0.2f), Gain(Oscillate(flGameTime, 5.5f), 0.2f), Gain(Oscillate(flGameTime, 4.0f), 0.2f)));
		r.RenderModel(m_iFloaters[1]);
		r.ResetTransformations();

		r.Translate(Vector(-51.14f, 1.0f, 40.3445f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 4.5f), 0.2f), Gain(Oscillate(flGameTime, 5.0f), 0.2f), Gain(Oscillate(flGameTime, 6.5f), 0.2f)));
		r.RenderModel(m_iFloaters[2]);
		r.ResetTransformations();

		r.Translate(Vector(-14.3265f, 1.0f, 46.879f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 6.5f), 0.2f), Gain(Oscillate(flGameTime, 5.5f), 0.2f), Gain(Oscillate(flGameTime, 4.5f), 0.2f)));
		r.RenderModel(m_iFloaters[3]);
		r.ResetTransformations();

		r.Translate(Vector(20.1533f, 1.0f, 33.2295f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 4.1f), 0.2f), Gain(Oscillate(flGameTime, 5.1f), 0.2f), Gain(Oscillate(flGameTime, 6.1f), 0.2f)));
		r.RenderModel(m_iFloaters[4]);
		r.ResetTransformations();

		r.Translate(Vector(56.8932f, 1.0f, 18.9258f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 6.1f), 0.2f), Gain(Oscillate(flGameTime, 5.9f), 0.2f), Gain(Oscillate(flGameTime, 4.1f), 0.2f)));
		r.RenderModel(m_iFloaters[5]);
		r.ResetTransformations();

		r.Translate(Vector(-43.3788f, 1.0f, -1.81977f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 4.9f), 0.2f), Gain(Oscillate(flGameTime, 5.9f), 0.2f), Gain(Oscillate(flGameTime, 6.9f), 0.2f)));
		r.RenderModel(m_iFloaters[6]);
		r.ResetTransformations();

		r.Translate(Vector(-69.7944f, 1.0f, -15.5551f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 6.9f), 0.2f), Gain(Oscillate(flGameTime, 5.1f), 0.2f), Gain(Oscillate(flGameTime, 4.9f), 0.2f)));
		r.RenderModel(m_iFloaters[7]);
		r.ResetTransformations();

		r.Translate(Vector(38.0865f, 1.0f, -11.1743f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 4.6f), 0.2f), Gain(Oscillate(flGameTime, 5.4f), 0.2f), Gain(Oscillate(flGameTime, 6.6f), 0.2f)));
		r.RenderModel(m_iFloaters[8]);
		r.ResetTransformations();

		r.Translate(Vector(-16.6582f, 1.0f, -28.5136f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 6.4f), 0.2f), Gain(Oscillate(flGameTime, 5.6f), 0.2f), Gain(Oscillate(flGameTime, 4.4f), 0.2f)));
		r.RenderModel(m_iFloaters[9]);
		r.ResetTransformations();

		r.Translate(Vector(65.7498f, 1.0f, -27.7423f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 4.3f), 0.2f), Gain(Oscillate(flGameTime, 5.7f), 0.2f), Gain(Oscillate(flGameTime, 6.3f), 0.2f)));
		r.RenderModel(m_iFloaters[10]);
		r.ResetTransformations();

		r.Translate(Vector(-47.0167f, 1.0f, -48.2766f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 6.7f), 0.2f), Gain(Oscillate(flGameTime, 5.3f), 0.2f), Gain(Oscillate(flGameTime, 4.7f), 0.2f)));
		r.RenderModel(m_iFloaters[11]);
		r.ResetTransformations();

		r.Translate(Vector(-13.8358f, 1.0f, -62.4203f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 4.2f), 0.2f), Gain(Oscillate(flGameTime, 5.8f), 0.2f), Gain(Oscillate(flGameTime, 6.0f), 0.2f)));
		r.RenderModel(m_iFloaters[12]);
		r.ResetTransformations();

		r.Translate(Vector(15.7742f, 1.0f, -40.5895f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 6.8f), 0.2f), Gain(Oscillate(flGameTime, 5.2f), 0.2f), Gain(Oscillate(flGameTime, 4.8f), 0.2f)));
		r.RenderModel(m_iFloaters[13]);
		r.ResetTransformations();

		r.Translate(Vector(32.4053f, 1.0f, -53.7385f)*1.5f);
		r.Translate(Vector(Gain(Oscillate(flGameTime, 4.0f), 0.2f), Gain(Oscillate(flGameTime, 5.0f), 0.2f), Gain(Oscillate(flGameTime, 6.2f), 0.2f)));
		r.RenderModel(m_iFloaters[14]);
	}

	m_bRenderingTransparent = true;

	if (true)
	{
		CGameRenderingContext r(this, true);

		r.SetBlend(BLEND_ADDITIVE);
		r.SetDepthMask(false);
		r.Rotate(m_flRing1Yaw, Vector(0, 0, 1));
		r.SetAlpha(Oscillate((float)GameServer()->GetGameTime(), 25));
		r.RenderModel(m_iRing1);

		m_flRing1Yaw += (float)GameServer()->GetFrameTime()*5;
	}

	if (true)
	{
		CGameRenderingContext r(this, true);

		r.SetBlend(BLEND_ADDITIVE);
		r.SetDepthMask(false);
		r.Rotate(m_flRing2Yaw, Vector(0, 0, 1));
		r.SetAlpha(Oscillate((float)GameServer()->GetGameTime(), 30));
		r.RenderModel(m_iRing2);

		m_flRing2Yaw -= (float)GameServer()->GetFrameTime()*5;
	}

	if (true)
	{
		CGameRenderingContext r(this, true);

		r.SetBlend(BLEND_ADDITIVE);
		r.SetDepthMask(false);
		r.Rotate(m_flRing3Yaw, Vector(0, 0, 1));
		r.SetAlpha(Oscillate((float)GameServer()->GetGameTime(), 35));
		r.RenderModel(m_iRing3);

		m_flRing3Yaw -= (float)GameServer()->GetFrameTime()*10;
	}
#endif

	r.ClearDepth();
}

void CDigitanksRenderer::FinishRendering(class CRenderingContext* pContext)
{
	TPROF("CDigitanksRenderer::FinishRendering");

	RenderTendrilBatches();
	RenderPreviewModes();
	RenderFogOfWar();
	RenderAvailableAreas();

	BaseClass::FinishRendering(pContext);
}

void CDigitanksRenderer::RenderPreviewModes()
{
	TPROF("CDigitanksRenderer::RenderPreviewModes");

	if (!DigitanksGame())
		return;

	CDigitanksPlayer* pTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();
	CSelectable* pCurrentSelection = DigitanksGame()->GetPrimarySelection();
	CDigitank* pCurrentTank = DigitanksGame()->GetPrimarySelectionTank();

	Vector vecPreviewMove;
	Vector vecPreviewDirection;
	if (pCurrentTank)
	{
		vecPreviewMove = pCurrentTank->GetPreviewMove();

		if ((vecPreviewMove - pCurrentTank->GetGlobalOrigin()).LengthSqr() == 0)
			vecPreviewDirection = Vector(0, 0, 0);
		else
			vecPreviewDirection = (vecPreviewMove - pCurrentTank->GetGlobalOrigin()).Normalized();
	}

	if (!pTeam)
		return;

	size_t iFormation = 0;
	
	Vector vecLookAt;
	bool bMouseOK = DigitanksWindow()->GetMouseGridPosition(vecLookAt);

	for (size_t i = 0; i < pTeam->GetNumUnits(); i++)
	{
		const CDigitank* pTank = dynamic_cast<const CDigitank*>(pTeam->GetUnit(i));

		if (pTank)
		{
			if (DigitanksGame()->GetControlMode() == MODE_TURN)
			{
				EAngle angTurn = EAngle(0, pTank->GetGlobalAngles().y, 0);
				if (pTank->TurnsWith(pCurrentTank))
				{
					bool bNoTurn = bMouseOK && (vecLookAt - DigitanksGame()->GetPrimarySelectionTank()->GetGlobalOrigin()).LengthSqr() < 3*3;

					if (!bNoTurn && bMouseOK)
					{
						Vector vecDirection = (vecLookAt - pTank->GetGlobalOrigin()).Normalized();
						float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

						float flTankTurn = AngleDifference(flYaw, pTank->GetGlobalAngles().y);
						if (fabs(flTankTurn)/pTank->TurnPerPower() > pTank->GetRemainingMovementEnergy())
							flTankTurn = (flTankTurn / fabs(flTankTurn)) * pTank->GetRemainingMovementEnergy() * pTank->TurnPerPower() * 0.95f;

						angTurn = EAngle(0, pTank->GetGlobalAngles().y + flTankTurn, 0);
					}

					CGameRenderingContext r(GameServer()->GetRenderer());
					r.Translate(pTank->GetRenderOrigin());
					r.Rotate(-pTank->GetGlobalAngles().y, Vector(0, 0, 1));
					r.SetAlpha(50.0f/255);
					r.SetBlend(BLEND_ALPHA);
					r.SetUniform("bColorSwapInAlpha", true);
					r.SetUniform("vecColorSwap", pTank->GetPlayerOwner()->GetColor());
					r.RenderModel(pTank->GetModelID());

					pTank->RenderTurret(50.0f/255);
				}
			}

			if (pCurrentTank && DigitanksGame()->GetControlMode() == MODE_MOVE)
			{
				if (pTank->GetDigitanksPlayer()->IsSelected(pTank) && pTank->MovesWith(pCurrentTank))
				{
					Vector vecNewPosition = DigitanksGame()->GetFormationPosition(vecPreviewMove, vecPreviewDirection, pTeam->GetNumTanks(), iFormation++);

					vecNewPosition.y = pTank->FindHoverHeight(vecNewPosition);

					CGameRenderingContext r(GameServer()->GetRenderer());
					r.UseProgram("model");
					r.Translate(vecNewPosition + Vector(0, 0, 1));
					r.Rotate(-VectorAngles(vecPreviewDirection).y, Vector(0, 0, 1));

					if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
						r.Scale(2, 2, 2);

					r.SetAlpha(50.0f/255);
					r.SetBlend(BLEND_ALPHA);
					r.SetUniform("bColorSwapInAlpha", true);
					r.SetUniform("vecColorSwap", pTank->GetPlayerOwner()->GetColor());
					r.RenderModel(pTank->GetModelID());

					pTank->RenderTurret(50.0f/255);
				}
			}

			if (pTank->ShouldDisplayAim())
			{
				int iAlpha = 100;
				if (pTank->GetDigitanksPlayer()->IsSelected(pTank) && DigitanksGame()->GetControlMode() == MODE_AIM)
					iAlpha = 255;

				Vector vecTankOrigin = pTank->GetGlobalOrigin();

				float flGravity = DigitanksGame()->GetGravity();
				float flTime;
				Vector vecForce;
				FindLaunchVelocity(vecTankOrigin, pTank->GetDisplayAim(), flGravity, vecForce, flTime, pTank->ProjectileCurve());

				CRopeRenderer oRope(GameServer()->GetRenderer(), CDigitank::GetAimBeamMaterial(), vecTankOrigin, 0.5f);
				oRope.SetColor(Color(255, 0, 0, iAlpha));
				oRope.SetTextureScale(50);
				oRope.SetTextureOffset(-(float)fmod(GameServer()->GetGameTime(), 1.0));

				size_t iLinks = 20;
				float flTimePerLink = flTime/iLinks;
				for (size_t i = 1; i < iLinks; i++)
				{
					float flCurrentTime = flTimePerLink*i;
					Vector vecCurrentOrigin = vecTankOrigin + vecForce*flCurrentTime + Vector(0, 0, flGravity*flCurrentTime*flCurrentTime/2);
					oRope.AddLink(vecCurrentOrigin);
				}

				oRope.Finish(pTank->GetDisplayAim());
			}

			bool bShowGoalMove = false;
			Vector vecGoalMove;
			int iAlpha = 255;
			if (pTank->HasGoalMovePosition())
			{
				bShowGoalMove = true;
				vecGoalMove = pTank->GetGoalMovePosition();
			}
			else if (pTank == pCurrentTank && DigitanksGame()->GetControlMode() == MODE_MOVE)
			{
				if (!pCurrentTank->IsPreviewMoveValid() && bMouseOK)
				{
					bShowGoalMove = true;
					vecGoalMove = pTank->GetPreviewMove();
					iAlpha = 100;
				}
			}

			if (pTank->GetPlayerOwner() != DigitanksGame()->GetCurrentLocalDigitanksPlayer())
				bShowGoalMove = false;

			if (false)// bShowGoalMove)
			{
				CRenderingContext r(GameServer()->GetRenderer());
				r.SetBlend(BLEND_ALPHA);

				CRopeRenderer oRope(GameServer()->GetRenderer(), CDigitank::GetAutoMoveMaterial(), DigitanksGame()->GetTerrain()->GetPointHeight(pTank->GetRealOrigin()) + Vector(0, 0, 1), 2);
				oRope.SetColor(Color(255, 255, 255, iAlpha));
				oRope.SetTextureScale(4);
				oRope.SetTextureOffset(-(float)fmod(GameServer()->GetGameTime(), 1.0));

				Vector vecPath = vecGoalMove - pTank->GetRealOrigin();
				vecPath.z = 0;

				float flDistance = vecPath.Length2D();
				float flDistancePerSegment = 5;
				Vector vecPathFlat = vecPath;
				vecPathFlat.z = 0;
				Vector vecDirection = vecPathFlat.Normalized();
				size_t iSegments = (size_t)(flDistance/flDistancePerSegment);

				Vector vecLastSegmentStart = pTank->GetRealOrigin();
				float flMaxMoveDistance = pTank->GetMaxMovementDistance();
				float flSegmentLength = flMaxMoveDistance;

				if ((vecGoalMove - pTank->GetRealOrigin()).LengthSqr() < flMaxMoveDistance*flMaxMoveDistance)
					flSegmentLength = (vecGoalMove - pTank->GetRealOrigin()).Length();

				for (size_t i = 1; i < iSegments; i++)
				{
					float flCurrentDistance = ((float)i*flDistancePerSegment);

					Vector vecLink = DigitanksGame()->GetTerrain()->GetPointHeight(pTank->GetRealOrigin() + vecDirection*flCurrentDistance) + Vector(0, 0, 1);

					float flDistanceFromSegmentStartSqr = (vecLink - vecLastSegmentStart).LengthSqr();

					float flRamp = flDistanceFromSegmentStartSqr / (flSegmentLength*flSegmentLength);
					if (flDistanceFromSegmentStartSqr > flMaxMoveDistance*flMaxMoveDistance)
					{
						vecLastSegmentStart = DigitanksGame()->GetTerrain()->GetPointHeight(vecLastSegmentStart + vecDirection*flSegmentLength) + Vector(0, 0, 1);

						oRope.SetWidth(0);
						oRope.FinishSegment(vecLastSegmentStart, vecLastSegmentStart, 2);

						// Skip to the next one if it goes backwards.
						if ((vecLink - pTank->GetRealOrigin()).LengthSqr() < (vecLastSegmentStart - pTank->GetRealOrigin()).LengthSqr())
							continue;

						flDistanceFromSegmentStartSqr = (vecLink - vecLastSegmentStart).LengthSqr();

						if ((vecGoalMove - vecLastSegmentStart).LengthSqr() < flMaxMoveDistance*flMaxMoveDistance)
							flSegmentLength = (vecGoalMove - vecLastSegmentStart).Length();

						flRamp = flDistanceFromSegmentStartSqr / (flSegmentLength*flSegmentLength);
					}

					oRope.SetWidth((1-flRamp)*2);
					oRope.AddLink(vecLink);
				}

				oRope.SetWidth(0);
				oRope.Finish(DigitanksGame()->GetTerrain()->GetPointHeight(vecGoalMove) + Vector(0, 0, 1));
			}

			if (pTank->GetDigitanksPlayer()->IsPrimarySelection(pTank) && DigitanksGame()->GetControlMode() == MODE_AIM && DigitanksGame()->GetAimType() == AIM_MOVEMENT)
			{
				CBaseEntity* pChargeTarget = pTank->GetPreviewCharge();

				if (pChargeTarget)
				{
					Vector vecPreviewTank = pTank->GetChargePosition(pChargeTarget);
					Vector vecChargeDirection = (pChargeTarget->GetGlobalOrigin() - pTank->GetGlobalOrigin()).Normalized();

					CGameRenderingContext r(GameServer()->GetRenderer());
					r.Translate(vecPreviewTank + Vector(0, 0, 1));
					r.Rotate(-VectorAngles(vecChargeDirection).y, Vector(0, 0, 1));
					r.SetAlpha(50.0f/255);
					r.SetBlend(BLEND_ALPHA);
					r.SetUniform("bColorSwapInAlpha", true);
					r.SetUniform("vecColorSwap", pTank->GetPlayerOwner()->GetColor());
					r.RenderModel(pTank->GetModelID());

					pTank->RenderTurret(50.0f/255);
				}
			}
			continue;
		}

		CCPU* pCPU = dynamic_cast<CCPU*>(pTeam->GetUnit(i));

		if (pCPU)
		{
			if (DigitanksGame()->GetControlMode() == MODE_BUILD)
			{
				CGameRenderingContext r(GameServer()->GetRenderer());
				r.Translate(pCPU->GetPreviewBuild() + Vector(0, 0, 3));
				r.Rotate(-pCPU->GetGlobalAngles().y, Vector(0, 0, 1));

				if (pCPU->IsPreviewBuildValid())
				{
					r.SetUniform("bColorSwapInAlpha", true);
					r.SetUniform("vecColorSwap", Color(255, 255, 255));
					r.SetAlpha(0.5f);
					r.SetBlend(BLEND_ALPHA);
					DigitanksWindow()->SetMouseCursor(MOUSECURSOR_BUILD);
				}
				else
				{
					r.SetUniform("bColorSwapInAlpha", true);
					r.SetUniform("vecColorSwap", Color(255, 0, 0));
					r.SetAlpha(0.3f);
					r.SetBlend(BLEND_ADDITIVE);
					DigitanksWindow()->SetMouseCursor(MOUSECURSOR_BUILDINVALID);
				}

				size_t iModel = 0;
				switch (pCPU->GetPreviewStructure())
				{
				case STRUCTURE_FIREWALL:
					iModel = CModelLibrary::Get()->FindModel("models/digitanks/autoturret.toy");
					break;

				case STRUCTURE_MINIBUFFER:
					iModel = CModelLibrary::Get()->FindModel("models/structures/minibuffer.toy");
					break;

				case STRUCTURE_BUFFER:
					iModel = CModelLibrary::Get()->FindModel("models/structures/buffer.toy");
					break;

				case STRUCTURE_BATTERY:
					iModel = CModelLibrary::Get()->FindModel("models/structures/battery.toy");
					break;

				case STRUCTURE_PSU:
					iModel = CModelLibrary::Get()->FindModel("models/structures/psu.toy");
					break;

				case STRUCTURE_INFANTRYLOADER:
					iModel = CModelLibrary::Get()->FindModel("models/structures/loader-infantry.toy");
					break;

				case STRUCTURE_TANKLOADER:
					iModel = CModelLibrary::Get()->FindModel("models/structures/loader-main.toy");
					break;

				case STRUCTURE_ARTILLERYLOADER:
					iModel = CModelLibrary::Get()->FindModel("models/structures/loader-artillery.toy");
					break;
				}

				r.RenderModel(iModel);
			}
			continue;
		}
	}
}

CVar r_fogofwar("r_fogofwar", "1");

void CDigitanksRenderer::RenderFogOfWar()
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->ShouldRenderFogOfWar())
		return;

	if (!r_fogofwar.GetBool())
		return;

	TPROF("CDigitanksRenderer::RenderFogOfWar");

	CDigitanksPlayer* pTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();

	if (!pTeam)
		return;

	// Render each visibility volume one at a time. If we do them all at once they interfere with each other.
	for (size_t i = 0; i < pTeam->GetNumUnits(); i++)
	{
		CBaseEntity* pEntity = pTeam->GetUnit(i);
		if (!pEntity)
			continue;

		CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);
		if (!pDTEntity)
			continue;

		if (pDTEntity->VisibleRange() == 0)
			continue;

		if (!IsSphereInFrustum(pDTEntity->GetRenderOrigin(), pDTEntity->VisibleRange()))
			continue;

		CRenderingContext c(this, true);
		c.UseFrameBuffer(&m_oVisibility1Buffer);
		c.ClearColor();

		c.SetDepthMask(false);
		c.SetDepthFunction(DF_GREATER);

		// Render this guy's visibility volume to the first buffer
		c.SetCullFace(CF_FRONT);
		c.SetColor(Color(255, 255, 255));
		pDTEntity->RenderVisibleArea();

		c.SetCullFace(CF_BACK);
		c.SetColor(Color(0, 0, 0));
		pDTEntity->RenderVisibleArea();

		c.SetBlend(BLEND_BOTH);
		c.SetColor(Color(255, 255, 255));
		c.SetDepthTest(false);

		// Copy the results to the second buffer
		RenderMapToBuffer(m_oVisibility1Buffer.m_iMap, &m_oVisibility2Buffer);
	}
}

void CDigitanksRenderer::RenderAvailableAreas()
{
	if (!DigitanksGame())
		return;

	TPROF("CDigitanksRenderer::RenderAvailableAreas");

	// Render each visibility volume one at a time. If we do them all at once they interfere with each other.
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);
		if (!pDTEntity)
			continue;

		int iAreas = pDTEntity->GetNumAvailableAreas();
		for (int j = 0; j < iAreas; j++)
		{
			if (!pDTEntity->IsAvailableAreaActive(j))
				continue;

			CRenderingContext c(this, true);
			c.UseFrameBuffer(&m_oVisibility1Buffer);
			c.UseProgram("volume");
			c.ClearColor();

			c.SetDepthMask(false);
			c.SetDepthFunction(DF_GREATER);

			// Render this guy's visibility volume to the first buffer
			c.SetCullFace(CF_FRONT);
			c.SetUniform("vecColor", Color(255, 255, 255));
			pDTEntity->RenderAvailableArea(j);

			c.SetCullFace(CF_BACK);
			c.SetUniform("vecColor", Color(0, 0, 0));
			pDTEntity->RenderAvailableArea(j);

			c.UseProgram("quad");
			c.SetBlend(BLEND_BOTH);
			c.SetColor(Color(255, 255, 255));
			c.SetDepthTest(false);
			c.SetDepthMask(false);

			// Copy the results to the second buffer
			RenderMapToBuffer(m_oVisibility1Buffer.m_iMap, &m_oAvailableAreaBuffer);
		}
	}
}

void CDigitanksRenderer::RenderOffscreenBuffers(class CRenderingContext* pContext)
{
	TPROF("CDigitanksRenderer::RenderOffscreenBuffers");

	if (!DigitanksGame())
		return;

	if (true)
	{
		TPROF("Explosions");

		CGameRenderingContext c(this, true);

		c.UseFrameBuffer(&m_oSceneBuffer);

		c.BindTexture(m_hNoise->m_iGLID, 1);

		c.UseProgram("explosion");
		c.SetUniform("iExplosion", 0);
		c.SetUniform("iNoise", 1);

		c.SetBlend(BLEND_ADDITIVE);

		RenderMapToBuffer(m_oExplosionBuffer.m_iMap, &m_oSceneBuffer);
	}

	if (true)
	{
		TPROF("Available areas");

		CGameRenderingContext c(this, true);

		c.SetBlend(BLEND_ADDITIVE);

		c.UseProgram("quad");

		if (DigitanksGame()->GetControlMode() == MODE_BUILD || DigitanksGame()->GetControlMode() == MODE_AIM)
			c.SetUniform("vecColor", Color(50, 250, 50, 100));
		else
			c.SetUniform("vecColor", Color(50, 250, 50, 30));

		RenderMapToBuffer(m_oAvailableAreaBuffer.m_iMap, &m_oSceneBuffer);
	}

	// Draw the fog of war.
	if (DigitanksGame()->ShouldRenderFogOfWar() && r_fogofwar.GetBool())
	{
		TPROF("Fog of war");

		// Explosion buffer's not in use anymore, reduce reuse recycle!
		RenderMapToBuffer(m_oSceneBuffer.m_iMap, &m_oExplosionBuffer);

		CGameRenderingContext c(this, true);

		c.BindTexture(m_oVisibility2Buffer.m_iMap);

		c.UseProgram("darken");

		c.SetUniform("iImage", 0);
		c.SetUniform("iDarkMap", 1);
		c.SetUniform("flFactor", 3.0f);

		if (DigitanksGame()->GetControlMode() == MODE_BUILD || DigitanksGame()->GetControlMode() == MODE_AIM)
			c.SetUniform("vecColor", Color(150, 150, 150));
		else
			c.SetUniform("vecColor", Color(255, 255, 255));

		RenderMapToBuffer(m_oExplosionBuffer.m_iMap, &m_oSceneBuffer);

		// Render the visibility-masked buffer, using the shadow volumes as a stencil.
		c.UseProgram("stencil");

		c.SetUniform("iStencilMap", 1);
		c.SetUniform("iImage", 0);

		c.SetBlend(BLEND_BOTH);

		RenderMapToBuffer(m_oVisibilityMaskedBuffer.m_iMap, &m_oSceneBuffer);
	}

	BaseClass::RenderOffscreenBuffers(pContext);
}

#ifdef _DEBUG
CVar r_show_debug("r_show_debug", "0");
#endif

static float g_flBloomPulseLength = 0.5f;
void CDigitanksRenderer::RenderFullscreenBuffers(class CRenderingContext* pContext)
{
	TPROF("CDigitanksRenderer::RenderFullscreenBuffers");

	if (!DigitanksGame())
		return;

	if (true)
	{
		if (GameServer()->GetGameTime() > m_flLastBloomPulse + g_flBloomPulseLength)
			m_flLastBloomPulse = 0;
	}

#ifndef TINKER_OPTIMIZE_SOFTWARE
	if (true)
	{
		CRenderingContext c(this, true);
		c.SetBlend(BLEND_ADDITIVE);
		RenderMapFullscreen(m_hVignetting->m_iGLID);
	}
#endif

	BaseClass::RenderFullscreenBuffers(pContext);

#ifdef _DEBUG
	if (r_show_debug.GetBool())
		RenderFrameBufferFullscreen(&m_oDebugBuffer);
#endif
}

float CDigitanksRenderer::BloomBrightnessCutoff() const
{
	return 0.6f + Bias((float)RemapValClamped(GameServer()->GetGameTime(), m_flLastBloomPulse, m_flLastBloomPulse + g_flBloomPulseLength, 1.0, 0.0), 0.2f) * 0.1f;
}

float CDigitanksRenderer::BloomScale() const
{
	return 1.0f + Bias((float)RemapValClamped(GameServer()->GetGameTime(), m_flLastBloomPulse, m_flLastBloomPulse + g_flBloomPulseLength, 1.0, 0.0), 0.2f);
}

void CDigitanksRenderer::BloomPulse()
{
	m_flLastBloomPulse = GameServer()->GetGameTime();
}

void CDigitanksRenderer::ClearTendrilBatches()
{
	m_ahTendrilBatches.clear();
}

void CDigitanksRenderer::AddTendrilBatch(const CSupplier* pSupplier)
{
	m_ahTendrilBatches.push_back() = pSupplier;
}

void CDigitanksRenderer::RenderTendrilBatches()
{
	TPROF("CDigitanksRenderer::RenderTendrilBatches");

	if (!DigitanksGame())
		return;

	CRenderingContext r(GameServer()->GetRenderer(), true);

	r.UseProgram("scroll");

	if (DigitanksGame()->ShouldRenderFogOfWar())
		r.UseFrameBuffer(DigitanksGame()->GetDigitanksRenderer()->GetVisibilityMaskedBuffer());

	r.SetDepthMask(false);
	r.UseMaterial(CSupplier::GetTendrilBeam());

	r.SetUniform("flTime", (float)GameServer()->GetGameTime());
	r.SetUniform("iTexture", 0);

	for (size_t i = 0; i < m_ahTendrilBatches.size(); i++)
	{
		CSupplier* pSupplier = m_ahTendrilBatches[i];

		if (!pSupplier)
			continue;

		COverheadCamera* pCamera = DigitanksGame()->GetOverheadCamera();
		Vector vecCamera = pCamera->GetGlobalOrigin();
		float flDistanceSqr = pSupplier->GetGlobalOrigin().DistanceSqr(vecCamera);
		float flFadeDistance = CVar::GetCVarFloat("perf_tendril_fade_distance");

		float flFadeAlpha = RemapValClamped(flDistanceSqr, flFadeDistance*flFadeDistance, (flFadeDistance+20)*(flFadeDistance+20), 1.0f, 0.0f);

		if (flFadeAlpha <= 0)
			continue;

		float flTreeAlpha = 1.0f;
		if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(pSupplier->GetGlobalOrigin().x), CTerrain::WorldToArraySpace(pSupplier->GetGlobalOrigin().y), TB_TREE))
			flTreeAlpha = 0.3f;

		r.SetUniform("flAlpha", flFadeAlpha * flTreeAlpha);

		pSupplier->RenderTendrils(r);
	}
}
