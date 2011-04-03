#include "dt_renderer.h"

#include <GL/glew.h>

#include <maths.h>

#include <shaders/shaders.h>
#include <game/digitanks/digitanksentity.h>
#include <game/digitanks/digitanksgame.h>
#include <models/models.h>
#include <models/texturelibrary.h>
#include <game/digitanks/dt_camera.h>
#include <shaders/shaders.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>

#include <ui/digitankswindow.h>

CDigitanksRenderer::CDigitanksRenderer()
	: CRenderer(DigitanksWindow()->GetWindowWidth(), DigitanksWindow()->GetWindowHeight())
{
	m_bUseFramebuffers = DigitanksWindow()->WantsFramebuffers() && HardwareSupportsFramebuffers();

	if (DigitanksWindow()->HasCommandLineSwitch("--no-framebuffers"))
		m_bUseFramebuffers = false;

	if (!DigitanksWindow()->WantsShaders())
		m_bUseShaders = false;

	if (DigitanksWindow()->HasCommandLineSwitch("--no-shaders"))
		m_bUseShaders = false;

	m_iVignetting = CTextureLibrary::AddTexture(L"textures/vignetting.png");

	m_iSkyboxFT = CTextureLibrary::AddTexture(L"textures/skybox/standard-ft.png", 2);
	m_iSkyboxLF = CTextureLibrary::AddTexture(L"textures/skybox/standard-lf.png", 2);
	m_iSkyboxBK = CTextureLibrary::AddTexture(L"textures/skybox/standard-bk.png", 2);
	m_iSkyboxRT = CTextureLibrary::AddTexture(L"textures/skybox/standard-rt.png", 2);
	m_iSkyboxDN = CTextureLibrary::AddTexture(L"textures/skybox/standard-dn.png", 2);
	m_iSkyboxUP = CTextureLibrary::AddTexture(L"textures/skybox/standard-up.png", 2);

	m_iRing1 = CModelLibrary::Get()->AddModel(L"models/skybox/ring1.obj", true);
	m_iRing2 = CModelLibrary::Get()->AddModel(L"models/skybox/ring2.obj", true);
	m_iRing3 = CModelLibrary::Get()->AddModel(L"models/skybox/ring3.obj", true);
	m_flRing1Yaw = 0;
	m_flRing2Yaw = 90;
	m_flRing3Yaw = 190;

	m_iVortex = CModelLibrary::Get()->AddModel(L"models/skybox/vortex.obj", true);
	m_flVortexYaw = 0;

	m_iDigiverse = CModelLibrary::Get()->AddModel(L"models/skybox/digiverse.obj", true);
	m_iFloaters[0] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float01.obj", true);
	m_iFloaters[1] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float02.obj", true);
	m_iFloaters[2] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float03.obj", true);
	m_iFloaters[3] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float04.obj", true);
	m_iFloaters[4] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float05.obj", true);
	m_iFloaters[5] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float06.obj", true);
	m_iFloaters[6] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float07.obj", true);
	m_iFloaters[7] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float08.obj", true);
	m_iFloaters[8] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float09.obj", true);
	m_iFloaters[9] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float10.obj", true);
	m_iFloaters[10] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float11.obj", true);
	m_iFloaters[11] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float12.obj", true);
	m_iFloaters[12] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float13.obj", true);
	m_iFloaters[13] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float14.obj", true);
	m_iFloaters[14] = CModelLibrary::Get()->AddModel(L"models/skybox/floaters/float15.obj", true);

	m_flLastBloomPulse = -100;
}

void CDigitanksRenderer::Initialize()
{
	BaseClass::Initialize();

	if (ShouldUseFramebuffers())
	{
		m_oExplosionBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, false, false);
		m_oVisibility1Buffer = CreateFrameBuffer(m_iWidth, m_iHeight, false, false);
		m_oVisibility2Buffer = CreateFrameBuffer(m_iWidth, m_iHeight, false, false);
		m_oVisibilityMaskedBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, false, false);
		m_oAvailableAreaBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, false, false);

		// Bind the regular scene's depth buffer to these buffers so we can use it for depth compares.
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oExplosionBuffer.m_iFB);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)m_oSceneBuffer.m_iDepth);
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oVisibility1Buffer.m_iFB);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)m_oSceneBuffer.m_iDepth);
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oVisibilityMaskedBuffer.m_iFB);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)m_oSceneBuffer.m_iDepth);
		glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	}
}

void CDigitanksRenderer::SetupFrame()
{
	TPROF("CDigitanksRenderer::SetupFrame");

	if (ShouldUseFramebuffers())
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oExplosionBuffer.m_iFB);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oVisibility2Buffer.m_iFB);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oVisibilityMaskedBuffer.m_iFB);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oAvailableAreaBuffer.m_iFB);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	BaseClass::SetupFrame();
}

void CDigitanksRenderer::StartRendering()
{
	TPROF("CDigitanksRenderer::StartRendering");

	ClearTendrilBatches();

	BaseClass::StartRendering();

	RenderSkybox();
}

void CDigitanksRenderer::RenderSkybox()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_MENU || DigitanksGame()->GetGameType() == GAMETYPE_EMPTY)
		return;

	TPROF("CDigitanksRenderer::RenderSkybox");

	if (true)
	{
		glPushAttrib(GL_CURRENT_BIT|GL_ENABLE_BIT);
		glPushMatrix();
		glTranslatef(m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z);

		if (GLEW_ARB_multitexture || GLEW_VERSION_1_3)
			glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxFT);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 1); glVertex3f(100, 100, -100);
			glTexCoord2i(0, 0); glVertex3f(100, -100, -100);
			glTexCoord2i(1, 0); glVertex3f(100, -100, 100);
			glTexCoord2i(1, 1); glVertex3f(100, 100, 100);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxLF);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 1); glVertex3f(-100, 100, -100);
			glTexCoord2i(0, 0); glVertex3f(-100, -100, -100);
			glTexCoord2i(1, 0); glVertex3f(100, -100, -100);
			glTexCoord2i(1, 1); glVertex3f(100, 100, -100);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxBK);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 1); glVertex3f(-100, 100, 100);
			glTexCoord2i(0, 0); glVertex3f(-100, -100, 100);
			glTexCoord2i(1, 0); glVertex3f(-100, -100, -100);
			glTexCoord2i(1, 1); glVertex3f(-100, 100, -100);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxRT);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 1); glVertex3f(100, 100, 100);
			glTexCoord2i(0, 0); glVertex3f(100, -100, 100);
			glTexCoord2i(1, 0); glVertex3f(-100, -100, 100);
			glTexCoord2i(1, 1); glVertex3f(-100, 100, 100);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxUP);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 1); glVertex3f(-100, 100, -100);
			glTexCoord2i(0, 0); glVertex3f(100, 100, -100);
			glTexCoord2i(1, 0); glVertex3f(100, 100, 100);
			glTexCoord2i(1, 1); glVertex3f(-100, 100, 100);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, (GLuint)m_iSkyboxDN);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 1); glVertex3f(100, -100, -100);
			glTexCoord2i(0, 0); glVertex3f(-100, -100, -100);
			glTexCoord2i(1, 0); glVertex3f(-100, -100, 100);
			glTexCoord2i(1, 1); glVertex3f(100, -100, 100);
		glEnd();

		glPopMatrix();
		glPopAttrib();
	}

	glClear(GL_DEPTH_BUFFER_BIT);

	// Set camera 1/16 to match the scale of the skybox
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_vecCameraPosition.x/16, m_vecCameraPosition.y/16, m_vecCameraPosition.z/16,
		m_vecCameraTarget.x/16, m_vecCameraTarget.y/16, m_vecCameraTarget.z/16,
		0.0, 1.0, 0.0);

	if (true)
	{
		CRenderingContext r(this);
		r.SetBlend(BLEND_ALPHA);
		r.Rotate(m_flVortexYaw, Vector(0, 1, 0));
		r.RenderModel(m_iVortex);

		r.SetBlend(BLEND_ADDITIVE);
		r.ResetTransformations();
		r.Translate(Vector(0, 1, 0));
		r.Rotate(-m_flVortexYaw, Vector(0, 1, 0));
		r.RenderModel(m_iVortex);

		m_flVortexYaw -= GameServer()->GetFrameTime()*2;
	}

	if (true)
	{
		CRenderingContext r(this);
		r.SetBlend(BLEND_ALPHA);
		r.RenderModel(m_iDigiverse);
	}

	if (true)
	{
		float flGameTime = GameServer()->GetGameTime();
		CRenderingContext r(this);

		r.Translate(Vector(-20.6999f, 1.0f, 74.3044f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 4.0f), 0.2f), SLerp(Oscillate(flGameTime, 5.0f), 0.2f), SLerp(Oscillate(flGameTime, 6.0f), 0.2f)));
		r.RenderModel(m_iFloaters[0]);
		r.ResetTransformations();

		r.Translate(Vector(23.2488f, 1.0f, 72.435f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 6.0f), 0.2f), SLerp(Oscillate(flGameTime, 5.5f), 0.2f), SLerp(Oscillate(flGameTime, 4.0f), 0.2f)));
		r.RenderModel(m_iFloaters[1]);
		r.ResetTransformations();

		r.Translate(Vector(-51.14f, 1.0f, 40.3445f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 4.5f), 0.2f), SLerp(Oscillate(flGameTime, 5.0f), 0.2f), SLerp(Oscillate(flGameTime, 6.5f), 0.2f)));
		r.RenderModel(m_iFloaters[2]);
		r.ResetTransformations();

		r.Translate(Vector(-14.3265f, 1.0f, 46.879f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 6.5f), 0.2f), SLerp(Oscillate(flGameTime, 5.5f), 0.2f), SLerp(Oscillate(flGameTime, 4.5f), 0.2f)));
		r.RenderModel(m_iFloaters[3]);
		r.ResetTransformations();

		r.Translate(Vector(20.1533f, 1.0f, 33.2295f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 4.1f), 0.2f), SLerp(Oscillate(flGameTime, 5.1f), 0.2f), SLerp(Oscillate(flGameTime, 6.1f), 0.2f)));
		r.RenderModel(m_iFloaters[4]);
		r.ResetTransformations();

		r.Translate(Vector(56.8932f, 1.0f, 18.9258f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 6.1f), 0.2f), SLerp(Oscillate(flGameTime, 5.9f), 0.2f), SLerp(Oscillate(flGameTime, 4.1f), 0.2f)));
		r.RenderModel(m_iFloaters[5]);
		r.ResetTransformations();

		r.Translate(Vector(-43.3788f, 1.0f, -1.81977f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 4.9f), 0.2f), SLerp(Oscillate(flGameTime, 5.9f), 0.2f), SLerp(Oscillate(flGameTime, 6.9f), 0.2f)));
		r.RenderModel(m_iFloaters[6]);
		r.ResetTransformations();

		r.Translate(Vector(-69.7944f, 1.0f, -15.5551f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 6.9f), 0.2f), SLerp(Oscillate(flGameTime, 5.1f), 0.2f), SLerp(Oscillate(flGameTime, 4.9f), 0.2f)));
		r.RenderModel(m_iFloaters[7]);
		r.ResetTransformations();

		r.Translate(Vector(38.0865f, 1.0f, -11.1743f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 4.6f), 0.2f), SLerp(Oscillate(flGameTime, 5.4f), 0.2f), SLerp(Oscillate(flGameTime, 6.6f), 0.2f)));
		r.RenderModel(m_iFloaters[8]);
		r.ResetTransformations();

		r.Translate(Vector(-16.6582f, 1.0f, -28.5136f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 6.4f), 0.2f), SLerp(Oscillate(flGameTime, 5.6f), 0.2f), SLerp(Oscillate(flGameTime, 4.4f), 0.2f)));
		r.RenderModel(m_iFloaters[9]);
		r.ResetTransformations();

		r.Translate(Vector(65.7498f, 1.0f, -27.7423f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 4.3f), 0.2f), SLerp(Oscillate(flGameTime, 5.7f), 0.2f), SLerp(Oscillate(flGameTime, 6.3f), 0.2f)));
		r.RenderModel(m_iFloaters[10]);
		r.ResetTransformations();

		r.Translate(Vector(-47.0167f, 1.0f, -48.2766f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 6.7f), 0.2f), SLerp(Oscillate(flGameTime, 5.3f), 0.2f), SLerp(Oscillate(flGameTime, 4.7f), 0.2f)));
		r.RenderModel(m_iFloaters[11]);
		r.ResetTransformations();

		r.Translate(Vector(-13.8358f, 1.0f, -62.4203f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 4.2f), 0.2f), SLerp(Oscillate(flGameTime, 5.8f), 0.2f), SLerp(Oscillate(flGameTime, 6.0f), 0.2f)));
		r.RenderModel(m_iFloaters[12]);
		r.ResetTransformations();

		r.Translate(Vector(15.7742f, 1.0f, -40.5895f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 6.8f), 0.2f), SLerp(Oscillate(flGameTime, 5.2f), 0.2f), SLerp(Oscillate(flGameTime, 4.8f), 0.2f)));
		r.RenderModel(m_iFloaters[13]);
		r.ResetTransformations();

		r.Translate(Vector(32.4053f, 1.0f, -53.7385f)*1.5f);
		r.Translate(Vector(SLerp(Oscillate(flGameTime, 4.0f), 0.2f), SLerp(Oscillate(flGameTime, 5.0f), 0.2f), SLerp(Oscillate(flGameTime, 6.2f), 0.2f)));
		r.RenderModel(m_iFloaters[14]);
	}

	if (true)
	{
		CRenderingContext r(this);

		r.SetBlend(BLEND_ADDITIVE);
		r.SetDepthMask(false);
		r.Rotate(m_flRing1Yaw, Vector(0, 1, 0));
		r.SetAlpha(Oscillate(GameServer()->GetGameTime(), 25));
		r.RenderModel(m_iRing1);

		m_flRing1Yaw += GameServer()->GetFrameTime()*5;
	}

	if (true)
	{
		CRenderingContext r(this);

		r.SetBlend(BLEND_ADDITIVE);
		r.SetDepthMask(false);
		r.Rotate(m_flRing2Yaw, Vector(0, 1, 0));
		r.SetAlpha(Oscillate(GameServer()->GetGameTime(), 30));
		r.RenderModel(m_iRing2);

		m_flRing2Yaw -= GameServer()->GetFrameTime()*5;
	}

	if (true)
	{
		CRenderingContext r(this);

		r.SetBlend(BLEND_ADDITIVE);
		r.SetDepthMask(false);
		r.Rotate(m_flRing3Yaw, Vector(0, 1, 0));
		r.SetAlpha(Oscillate(GameServer()->GetGameTime(), 35));
		r.RenderModel(m_iRing3);

		m_flRing3Yaw -= GameServer()->GetFrameTime()*10;
	}

	// Reset the camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z,
		m_vecCameraTarget.x, m_vecCameraTarget.y, m_vecCameraTarget.z,
		0.0, 1.0, 0.0);

	glClear(GL_DEPTH_BUFFER_BIT);
}

void CDigitanksRenderer::FinishRendering()
{
	TPROF("CDigitanksRenderer::FinishRendering");

	if (ShouldUseShaders())
		ClearProgram();

	RenderTendrilBatches();
	RenderPreviewModes();
	RenderFogOfWar();
	RenderAvailableAreas();

	BaseClass::FinishRendering();
}

void CDigitanksRenderer::SetupSceneShader()
{
	if (!DigitanksGame()->GetDigitanksCamera()->HasCameraGuidedMissile())
		return;

	GLuint iSceneProgram = (GLuint)CShaderLibrary::GetCameraGuidedProgram();
	UseProgram((GLuint)iSceneProgram);

	// Will be filled in by RenderMapFullscreen()
	GLint iSource = glGetUniformLocation(iSceneProgram, "iSource");
	glUniform1i(iSource, 0);

	GLint flOffsetX = glGetUniformLocation(iSceneProgram, "flOffsetX");
	glUniform1f(flOffsetX, 1.3f / m_oSceneBuffer.m_iWidth);

	GLint flOffsetY = glGetUniformLocation(iSceneProgram, "flOffsetY");
	glUniform1f(flOffsetY, 1.3f / m_oSceneBuffer.m_iHeight);
}

void CDigitanksRenderer::RenderPreviewModes()
{
	TPROF("CDigitanksRenderer::RenderPreviewModes");

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	CSelectable* pCurrentSelection = DigitanksGame()->GetPrimarySelection();
	CDigitank* pCurrentTank = DigitanksGame()->GetPrimarySelectionTank();

	if (!pTeam)
		return;
	
	for (size_t i = 0; i < pTeam->GetNumMembers(); i++)
	{
		CDigitank* pTank = dynamic_cast<CDigitank*>(pTeam->GetMember(i));

		if (pTank)
		{
			if (DigitanksGame()->GetControlMode() == MODE_TURN)
			{
				EAngle angTurn = EAngle(0, pTank->GetAngles().y, 0);
				if (pTank->TurnsWith(pCurrentTank))
				{
					Vector vecLookAt;
					bool bMouseOK = DigitanksWindow()->GetMouseGridPosition(vecLookAt);
					bool bNoTurn = bMouseOK && (vecLookAt - DigitanksGame()->GetPrimarySelectionTank()->GetOrigin()).LengthSqr() < 3*3;

					if (!bNoTurn && bMouseOK)
					{
						Vector vecDirection = (vecLookAt - pTank->GetOrigin()).Normalized();
						float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

						float flTankTurn = AngleDifference(flYaw, pTank->GetAngles().y);
						if (fabs(flTankTurn)/pTank->TurnPerPower() > pTank->GetRemainingMovementEnergy())
							flTankTurn = (flTankTurn / fabs(flTankTurn)) * pTank->GetRemainingMovementEnergy() * pTank->TurnPerPower() * 0.95f;

						angTurn = EAngle(0, pTank->GetAngles().y + flTankTurn, 0);
					}

					CRenderingContext r(GameServer()->GetRenderer());
					r.Translate(pTank->GetRenderOrigin());
					r.Rotate(-pTank->GetAngles().y, Vector(0, 1, 0));
					r.SetAlpha(50.0f/255);
					r.SetBlend(BLEND_ALPHA);
					r.SetColorSwap(pTank->GetTeam()->GetColor());
					r.RenderModel(pTank->GetModel());

					pTank->RenderTurret(true, 50.0f/255);
				}
			}

			if (pCurrentTank && DigitanksGame()->GetControlMode() == MODE_MOVE)
			{
				if (pTank->GetDigitanksTeam()->IsSelected(pTank) && pTank->MovesWith(pCurrentTank))
				{
					Vector vecTankMove = (pCurrentTank->GetPreviewMove() - pCurrentTank->GetOrigin()).Normalized();
					if (vecTankMove.Length() > pTank->GetRemainingMovementDistance())
						vecTankMove = vecTankMove * pTank->GetRemainingMovementDistance() * 0.95f;

					Vector vecNewPosition = pCurrentTank->GetPreviewMove();
					vecNewPosition.y = pTank->FindHoverHeight(vecNewPosition);

					CRenderingContext r(GameServer()->GetRenderer());
					r.Translate(vecNewPosition + Vector(0, 1, 0));
					r.Rotate(-VectorAngles(vecTankMove).y, Vector(0, 1, 0));
					r.SetAlpha(50.0f/255);
					r.SetBlend(BLEND_ALPHA);
					r.SetColorSwap(pTank->GetTeam()->GetColor());
					r.RenderModel(pTank->GetModel());

					pTank->RenderTurret(true, 50.0f/255);
				}
			}

			if (pTank->ShouldDisplayAim())
			{
				glPushAttrib(GL_ENABLE_BIT);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				int iAlpha = 100;
				if (pTank->GetDigitanksTeam()->IsSelected(pTank) && DigitanksGame()->GetControlMode() == MODE_AIM)
					iAlpha = 255;

				Vector vecTankOrigin = pTank->GetOrigin();

				float flGravity = DigitanksGame()->GetGravity();
				float flTime;
				Vector vecForce;
				FindLaunchVelocity(vecTankOrigin, pTank->GetDisplayAim(), flGravity, vecForce, flTime, pTank->ProjectileCurve());

				CRopeRenderer oRope(GameServer()->GetRenderer(), CDigitank::GetAimBeamTexture(), vecTankOrigin, 0.5f);
				oRope.SetColor(Color(255, 0, 0, iAlpha));
				oRope.SetTextureScale(50);
				oRope.SetTextureOffset(-fmod(GameServer()->GetGameTime(), 1));

				size_t iLinks = 20;
				float flTimePerLink = flTime/iLinks;
				for (size_t i = 1; i < iLinks; i++)
				{
					float flCurrentTime = flTimePerLink*i;
					Vector vecCurrentOrigin = vecTankOrigin + vecForce*flCurrentTime + Vector(0, flGravity*flCurrentTime*flCurrentTime/2, 0);
					oRope.AddLink(vecCurrentOrigin);
				}

				oRope.Finish(pTank->GetDisplayAim());

				glPopAttrib();
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
				if (!pCurrentTank->IsPreviewMoveValid())
				{
					bShowGoalMove = true;
					vecGoalMove = pTank->GetPreviewMove();
					iAlpha = 100;
				}
			}

			if (pTank->GetTeam() != DigitanksGame()->GetCurrentLocalDigitanksTeam())
				bShowGoalMove = false;

			if (bShowGoalMove)
			{
				CRenderingContext r(GameServer()->GetRenderer());
				r.SetBlend(BLEND_ALPHA);

				CRopeRenderer oRope(GameServer()->GetRenderer(), CDigitank::GetAutoMoveTexture(), DigitanksGame()->GetTerrain()->SetPointHeight(pTank->GetRealOrigin()) + Vector(0, 1, 0), 2);
				oRope.SetColor(Color(255, 255, 255, iAlpha));
				oRope.SetTextureScale(4);
				oRope.SetTextureOffset(-fmod(GameServer()->GetGameTime(), 1));

				Vector vecPath = vecGoalMove - pTank->GetRealOrigin();
				vecPath.y = 0;

				float flDistance = vecPath.Length2D();
				float flDistancePerSegment = 5;
				Vector vecPathFlat = vecPath;
				vecPathFlat.y = 0;
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

					Vector vecLink = DigitanksGame()->GetTerrain()->SetPointHeight(pTank->GetRealOrigin() + vecDirection*flCurrentDistance) + Vector(0, 1, 0);

					if (DigitanksGame()->GetTerrain()->IsPointOverHole(vecLink))
						break;

					float flDistanceFromSegmentStartSqr = (vecLink - vecLastSegmentStart).LengthSqr();

					float flRamp = flDistanceFromSegmentStartSqr / (flSegmentLength*flSegmentLength);
					if (flDistanceFromSegmentStartSqr > flMaxMoveDistance*flMaxMoveDistance)
					{
						vecLastSegmentStart = DigitanksGame()->GetTerrain()->SetPointHeight(vecLastSegmentStart + vecDirection*flSegmentLength) + Vector(0, 1, 0);

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
				oRope.Finish(DigitanksGame()->GetTerrain()->SetPointHeight(vecGoalMove) + Vector(0, 1, 0));
			}

			if (pTank->GetDigitanksTeam()->IsPrimarySelection(pTank) && DigitanksGame()->GetControlMode() == MODE_AIM && DigitanksGame()->GetAimType() == AIM_MOVEMENT)
			{
				CBaseEntity* pChargeTarget = pTank->GetPreviewCharge();

				if (pChargeTarget)
				{
					Vector vecPreviewTank = pTank->GetChargePosition(pChargeTarget);
					Vector vecChargeDirection = (pChargeTarget->GetOrigin() - pTank->GetOrigin()).Normalized();

					CRenderingContext r(GameServer()->GetRenderer());
					r.Translate(vecPreviewTank + Vector(0, 1, 0));
					r.Rotate(-VectorAngles(vecChargeDirection).y, Vector(0, 1, 0));
					r.SetAlpha(50.0f/255);
					r.SetBlend(BLEND_ALPHA);
					r.SetColorSwap(pTank->GetTeam()->GetColor());
					r.RenderModel(pTank->GetModel());

					pTank->RenderTurret(true, 50.0f/255);
				}
			}
			continue;
		}

		CCPU* pCPU = dynamic_cast<CCPU*>(pTeam->GetMember(i));

		if (pCPU)
		{
			if (DigitanksGame()->GetControlMode() == MODE_BUILD)
			{
				CRenderingContext r(GameServer()->GetRenderer());
				r.Translate(pCPU->GetPreviewBuild() + Vector(0, 3, 0));
				r.Rotate(-pCPU->GetAngles().y, Vector(0, 1, 0));

				if (pCPU->IsPreviewBuildValid())
				{
					r.SetColorSwap(Color(255, 255, 255));
					r.SetAlpha(0.5f);
					r.SetBlend(BLEND_ALPHA);
					DigitanksWindow()->SetMouseCursor(MOUSECURSOR_BUILD);
				}
				else
				{
					r.SetColorSwap(Color(255, 0, 0));
					r.SetAlpha(0.3f);
					r.SetBlend(BLEND_ADDITIVE);
					DigitanksWindow()->SetMouseCursor(MOUSECURSOR_BUILDINVALID);
				}

				size_t iModel = 0;
				switch (pCPU->GetPreviewStructure())
				{
				case STRUCTURE_AUTOTURRET:
					iModel = CModelLibrary::Get()->FindModel(L"models/digitanks/autoturret.obj");
					break;

				case STRUCTURE_MINIBUFFER:
					iModel = CModelLibrary::Get()->FindModel(L"models/structures/minibuffer.obj");
					break;

				case STRUCTURE_BUFFER:
					iModel = CModelLibrary::Get()->FindModel(L"models/structures/buffer.obj");
					break;

				case STRUCTURE_BATTERY:
					iModel = CModelLibrary::Get()->FindModel(L"models/structures/battery.obj");
					break;

				case STRUCTURE_PSU:
					iModel = CModelLibrary::Get()->FindModel(L"models/structures/psu.obj");
					break;

				case STRUCTURE_INFANTRYLOADER:
					iModel = CModelLibrary::Get()->FindModel(L"models/structures/loader-infantry.obj");
					break;

				case STRUCTURE_TANKLOADER:
					iModel = CModelLibrary::Get()->FindModel(L"models/structures/loader-main.obj");
					break;

				case STRUCTURE_ARTILLERYLOADER:
					iModel = CModelLibrary::Get()->FindModel(L"models/structures/loader-artillery.obj");
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
	if (!DigitanksGame()->ShouldRenderFogOfWar() || !ShouldUseFramebuffers() || !ShouldUseShaders())
		return;

	if (!r_fogofwar.GetBool())
		return;

	TPROF("CDigitanksRenderer::RenderFogOfWar");

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	if (!pTeam)
		return;

	// Render each visibility volume one at a time. If we do them all at once they interfere with each other.
	for (size_t i = 0; i < pTeam->GetNumMembers(); i++)
	{
		CBaseEntity* pEntity = pTeam->GetMember(i);
		if (!pEntity)
			continue;

		CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);
		if (!pDTEntity)
			continue;

		if (pDTEntity->VisibleRange() == 0)
			continue;

		CRenderingContext c(this);
		glDisable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		c.UseFrameBuffer(&m_oVisibility1Buffer);
		glClear(GL_COLOR_BUFFER_BIT);

		c.SetDepthMask(false);

		glDepthFunc(GL_GREATER);

		// Render this guy's visibility volume to the first buffer
		glCullFace(GL_FRONT);
		c.SetColor(Color(255, 255, 255));
		pDTEntity->RenderVisibleArea();

		glCullFace(GL_BACK);
		c.SetColor(Color(0, 0, 0));
		pDTEntity->RenderVisibleArea();

		c.SetBlend(BLEND_ADDITIVE);
		glBlendFunc(GL_ONE, GL_ONE);
		c.SetColor(Color(255, 255, 255));
		glDisable(GL_DEPTH_TEST);
		glCullFace(GL_NONE);
		c.SetDepthMask(false);

		ClearProgram();

		// Copy the results to the second buffer
		RenderMapToBuffer(m_oVisibility1Buffer.m_iMap, &m_oVisibility2Buffer);

		c.SetBlend(BLEND_NONE);
		glEnable(GL_DEPTH_TEST);
	}

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);

	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
}

void CDigitanksRenderer::RenderAvailableAreas()
{
	if (!HardwareSupportsFramebuffers())
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

			CRenderingContext c(this);
			glDisable(GL_LIGHTING);
			glDisable(GL_COLOR_MATERIAL);
			if (ShouldUseFramebuffers())
				c.UseFrameBuffer(&m_oVisibility1Buffer);
			else
			{
				glReadBuffer(GL_AUX1);
				glDrawBuffer(GL_AUX1);
			}
			glClear(GL_COLOR_BUFFER_BIT);

			c.SetDepthMask(false);

			glDepthFunc(GL_GREATER);

			// Render this guy's visibility volume to the first buffer
			glCullFace(GL_FRONT);
			c.SetColor(Color(255, 255, 255));
			pDTEntity->RenderAvailableArea(j);

			glCullFace(GL_BACK);
			c.SetColor(Color(0, 0, 0));
			pDTEntity->RenderAvailableArea(j);

			c.SetBlend(BLEND_ADDITIVE);
			glBlendFunc(GL_ONE, GL_ONE);
			c.SetColor(Color(255, 255, 255));
			glDisable(GL_DEPTH_TEST);
			glCullFace(GL_NONE);
			c.SetDepthMask(false);

			if (ShouldUseShaders())
				ClearProgram();

			// Copy the results to the second buffer
			if (ShouldUseFramebuffers())
				RenderMapToBuffer(m_oVisibility1Buffer.m_iMap, &m_oAvailableAreaBuffer);
			else
			{
				glReadBuffer(GL_AUX1);
				glDrawBuffer(GL_AUX2);
			}

			c.SetBlend(BLEND_NONE);
			glEnable(GL_DEPTH_TEST);
		}
	}

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);

	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
}

void CDigitanksRenderer::RenderOffscreenBuffers()
{
	TPROF("CDigitanksRenderer::RenderOffscreenBuffers");

	if (ShouldUseFramebuffers() && ShouldUseShaders())
	{
		TPROF("Explosions");

		// Render the explosions back onto the scene buffer, passing through the noise filter.
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oSceneBuffer.m_iFB);

		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, (GLuint)m_oNoiseBuffer.m_iMap);

		GLuint iExplosionProgram = (GLuint)CShaderLibrary::GetExplosionProgram();
		UseProgram(iExplosionProgram);

		GLint iExplosion = glGetUniformLocation(iExplosionProgram, "iExplosion");
		glUniform1i(iExplosion, 0);

		GLint iNoise = glGetUniformLocation(iExplosionProgram, "iNoise");
		glUniform1i(iNoise, 1);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		RenderMapToBuffer(m_oExplosionBuffer.m_iMap, &m_oSceneBuffer);
		glDisable(GL_BLEND);

		ClearProgram();

		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);

		glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	}

	if (ShouldUseFramebuffers() && ShouldUseShaders())
	{
		TPROF("Available areas");

		UseProgram(0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		if (DigitanksGame()->GetControlMode() == MODE_BUILD || DigitanksGame()->GetControlMode() == MODE_AIM)
			glColor4ubv(Color(50, 250, 50, 100));
		else
			glColor4ubv(Color(50, 250, 50, 30));
		RenderMapToBuffer(m_oAvailableAreaBuffer.m_iMap, &m_oSceneBuffer);
		glColor4ubv(Color(255, 255, 255));
		glDisable(GL_BLEND);
	}

	// Draw the fog of war.
	if (ShouldUseFramebuffers() && ShouldUseShaders() && DigitanksGame()->ShouldRenderFogOfWar() && r_fogofwar.GetBool())
	{
		TPROF("Fog of war");

		// Explosion buffer's not in use anymore, reduce reuse recycle!
		RenderMapToBuffer(m_oSceneBuffer.m_iMap, &m_oExplosionBuffer);

		glActiveTexture(GL_TEXTURE1);
	    glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, (GLuint)m_oVisibility2Buffer.m_iMap);

		GLuint iDarkenProgram = (GLuint)CShaderLibrary::GetDarkenProgram();
		UseProgram(iDarkenProgram);

		GLint iDarkMap = glGetUniformLocation(iDarkenProgram, "iDarkMap");
	    glUniform1i(iDarkMap, 1);

		GLint iImage = glGetUniformLocation(iDarkenProgram, "iImage");
	    glUniform1i(iImage, 0);

		GLint flFactor = glGetUniformLocation(iDarkenProgram, "flFactor");
	    glUniform1f(flFactor, 3.0f);

		if (DigitanksGame()->GetControlMode() == MODE_BUILD || DigitanksGame()->GetControlMode() == MODE_AIM)
			glColor4ubv(Color(150, 150, 150));
		else
			glColor4ubv(Color(255, 255, 255));

		RenderMapToBuffer(m_oExplosionBuffer.m_iMap, &m_oSceneBuffer);

		// Render the visibility-masked buffer, using the shadow volumes as a stencil.
		GLuint iStencilProgram = (GLuint)CShaderLibrary::GetStencilProgram();
		UseProgram(iStencilProgram);

		GLint iStencilMap = glGetUniformLocation(iStencilProgram, "iStencilMap");
	    glUniform1i(iStencilMap, 1);

		iImage = glGetUniformLocation(iStencilProgram, "iImage");
	    glUniform1i(iImage, 0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		RenderMapToBuffer(m_oVisibilityMaskedBuffer.m_iMap, &m_oSceneBuffer);
		glDisable(GL_BLEND);

		ClearProgram();

		glActiveTexture(GL_TEXTURE1);
	    glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
	}

	if (ShouldUseFramebuffers() && ShouldUseShaders())
	{
		TPROF("Bloom");

		// Use a bright-pass filter to catch only the bright areas of the image
		GLuint iBrightPass = (GLuint)CShaderLibrary::GetBrightPassProgram();
		UseProgram(iBrightPass);

		GLint iSource = glGetUniformLocation(iBrightPass, "iSource");
		glUniform1i(iSource, 0);

		GLint flScale = glGetUniformLocation(iBrightPass, "flScale");
		glUniform1f(flScale, (float)1/BLOOM_FILTERS);

		GLint flBrightness = glGetUniformLocation(iBrightPass, "flBrightness");

		for (size_t i = 0; i < BLOOM_FILTERS; i++)
		{
			glUniform1f(flBrightness, 0.6f - 0.1f*i);
			RenderMapToBuffer(m_oSceneBuffer.m_iMap, &m_oBloom1Buffers[i]);
		}

		ClearProgram();

		RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
		RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);

		RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
		RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);
	}
}

void CDigitanksRenderer::RenderFullscreenBuffers()
{
	TPROF("CDigitanksRenderer::RenderFullscreenBuffers");

	glEnable(GL_BLEND);

	if (ShouldUseFramebuffers())
	{
		glBlendFunc(GL_ONE, GL_ONE);
		for (size_t i = 0; i < BLOOM_FILTERS; i++)
			RenderMapFullscreen(m_oBloom1Buffers[i].m_iMap);

		float flBloomPulseLength = 0.5f;
		float flGameTime = GameServer()->GetGameTime();
		float flPulseStrength = Lerp(RemapValClamped(flGameTime, m_flLastBloomPulse, m_flLastBloomPulse + flBloomPulseLength, 1, 0), 0.2f);
		if (flPulseStrength > 0)
		{
			glPushAttrib(GL_CURRENT_BIT);
			glColor4f(flPulseStrength, flPulseStrength, flPulseStrength, flPulseStrength);
			for (size_t i = 0; i < BLOOM_FILTERS; i++)
			{
				RenderMapFullscreen(m_oBloom1Buffers[i].m_iMap);
				RenderMapFullscreen(m_oBloom1Buffers[i].m_iMap);
				RenderMapFullscreen(m_oBloom1Buffers[i].m_iMap);
			}
			glPopAttrib();
		}
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	RenderMapFullscreen(m_iVignetting);

	glDisable(GL_BLEND);
}

#define KERNEL_SIZE   3
//float aflKernel[KERNEL_SIZE] = { 5, 6, 5 };
float aflKernel[KERNEL_SIZE] = { 0.3125f, 0.375f, 0.3125f };

void CDigitanksRenderer::RenderBloomPass(CFrameBuffer* apSources, CFrameBuffer* apTargets, bool bHorizontal)
{
	GLuint iBlur = (GLuint)CShaderLibrary::GetBlurProgram();
	UseProgram(iBlur);

	GLint iSource = glGetUniformLocation(iBlur, "iSource");
    glUniform1i(iSource, 0);

	// Can't I get rid of this and hard code it into the shader?
	GLint aflCoefficients = glGetUniformLocation(iBlur, "aflCoefficients");
    glUniform1fv(aflCoefficients, KERNEL_SIZE, aflKernel);

    GLint flOffsetX = glGetUniformLocation(iBlur, "flOffsetX");
    glUniform1f(flOffsetX, 0);

	GLint flOffset = glGetUniformLocation(iBlur, "flOffsetY");
    glUniform1f(flOffset, 0);
    if (bHorizontal)
        flOffset = glGetUniformLocation(iBlur, "flOffsetX");

    // Perform the blurring.
    for (size_t i = 0; i < BLOOM_FILTERS; i++)
    {
		glUniform1f(flOffset, 1.2f / apSources[i].m_iWidth);
		RenderMapToBuffer(apSources[i].m_iMap, &apTargets[i]);
    }

	ClearProgram();
}

void CDigitanksRenderer::BloomPulse()
{
	m_flLastBloomPulse = GameServer()->GetGameTime();
}

void CDigitanksRenderer::ClearTendrilBatches()
{
	m_ahTendrilBatches.clear();
}

void CDigitanksRenderer::AddTendrilBatch(CSupplier* pSupplier)
{
	m_ahTendrilBatches.push_back() = pSupplier;
}

void CDigitanksRenderer::RenderTendrilBatches()
{
	TPROF("CDigitanksRenderer::RenderTendrilBatches");

	GLuint iScrollingTextureProgram;
	if (GameServer()->GetRenderer()->ShouldUseShaders())
		iScrollingTextureProgram = (GLuint)CShaderLibrary::GetScrollingTextureProgram();

	CRenderingContext r(GameServer()->GetRenderer());
	if (DigitanksGame()->ShouldRenderFogOfWar() && DigitanksGame()->GetDigitanksRenderer()->ShouldUseFramebuffers())
		r.UseFrameBuffer(DigitanksGame()->GetDigitanksRenderer()->GetVisibilityMaskedBuffer());
	r.SetDepthMask(false);
	r.BindTexture(CSupplier::GetTendrilBeam());

	if (GameServer()->GetRenderer()->ShouldUseShaders())
	{
		r.UseProgram(iScrollingTextureProgram);

		GLuint flTime = glGetUniformLocation(iScrollingTextureProgram, "flTime");
		glUniform1f(flTime, GameServer()->GetGameTime());

		GLuint iTexture = glGetUniformLocation(iScrollingTextureProgram, "iTexture");
		glUniform1f(iTexture, 0);
	}

	for (size_t i = 0; i < m_ahTendrilBatches.size(); i++)
	{
		CSupplier* pSupplier = m_ahTendrilBatches[i];

		if (!pSupplier)
			continue;

		CDigitanksCamera* pCamera = DigitanksGame()->GetDigitanksCamera();
		Vector vecCamera = pCamera->GetCameraPosition();
		float flDistanceSqr = pSupplier->GetOrigin().DistanceSqr(vecCamera);
		float flFadeDistance = CVar::GetCVarFloat("perf_tendril_fade_distance");

		float flFadeAlpha = RemapValClamped(flDistanceSqr, flFadeDistance*flFadeDistance, (flFadeDistance+20)*(flFadeDistance+20), 1, 0);

		if (flFadeAlpha <= 0)
			continue;

		float flTreeAlpha = 1.0f;
		if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(pSupplier->GetOrigin().x), CTerrain::WorldToArraySpace(pSupplier->GetOrigin().z), TB_TREE))
			flTreeAlpha = 0.3f;

		if (GameServer()->GetRenderer()->ShouldUseShaders())
		{
			GLuint flAlpha = glGetUniformLocation(iScrollingTextureProgram, "flAlpha");
			glUniform1f(flAlpha, flFadeAlpha * flTreeAlpha);
		}

		glCallList(pSupplier->GetTendrilsCallList());
	}
}
