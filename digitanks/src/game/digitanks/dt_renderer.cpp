#include "dt_renderer.h"

#include <GL/glew.h>

#include <maths.h>

#include <shaders/shaders.h>
#include <game/digitanks/digitanksentity.h>
#include <game/digitanks/digitanksgame.h>
#include <models/models.h>

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

	m_iVignetting = CRenderer::LoadTextureIntoGL(L"textures/vignetting.png");

	m_iSkyboxFT = CRenderer::LoadTextureIntoGL(L"textures/skybox/standard-ft.png", 2);
	m_iSkyboxLF = CRenderer::LoadTextureIntoGL(L"textures/skybox/standard-lf.png", 2);
	m_iSkyboxBK = CRenderer::LoadTextureIntoGL(L"textures/skybox/standard-bk.png", 2);
	m_iSkyboxRT = CRenderer::LoadTextureIntoGL(L"textures/skybox/standard-rt.png", 2);
	m_iSkyboxDN = CRenderer::LoadTextureIntoGL(L"textures/skybox/standard-dn.png", 2);
	m_iSkyboxUP = CRenderer::LoadTextureIntoGL(L"textures/skybox/standard-up.png", 2);

	m_iRing1 = CModelLibrary::Get()->AddModel(L"models/skybox/ring1.obj", true);
	m_iRing2 = CModelLibrary::Get()->AddModel(L"models/skybox/ring2.obj", true);
	m_iRing3 = CModelLibrary::Get()->AddModel(L"models/skybox/ring3.obj", true);
	m_flRing1Yaw = 0;
	m_flRing2Yaw = 90;
	m_flRing3Yaw = 190;

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
		m_oBuildableAreaBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, false, false);

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
	if (ShouldUseFramebuffers())
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oExplosionBuffer.m_iFB);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oVisibility2Buffer.m_iFB);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oVisibilityMaskedBuffer.m_iFB);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oBuildableAreaBuffer.m_iFB);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	BaseClass::SetupFrame();
}

void CDigitanksRenderer::StartRendering()
{
	BaseClass::StartRendering();

	RenderSkybox();
}

void CDigitanksRenderer::RenderSkybox()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_MENU || DigitanksGame()->GetGameType() == GAMETYPE_EMPTY)
		return;

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
	if (ShouldUseShaders())
		ClearProgram();

	RenderFogOfWar();
	RenderBuildableAreas();

	BaseClass::FinishRendering();
}

void CDigitanksRenderer::RenderFogOfWar()
{
	if (!DigitanksGame()->ShouldRenderFogOfWar())
		return;

	// Render each visibility volume one at a time. If we do them all at once they interfere with each other.
	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
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

		if (ShouldUseShaders())
			ClearProgram();

		// Copy the results to the second buffer
		if (ShouldUseFramebuffers())
			RenderMapToBuffer(m_oVisibility1Buffer.m_iMap, &m_oVisibility2Buffer);
		else
		{
			glReadBuffer(GL_AUX1);
			glDrawBuffer(GL_AUX2);
		}

		c.SetBlend(BLEND_NONE);
		glEnable(GL_DEPTH_TEST);
	}

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);

	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
}

void CDigitanksRenderer::RenderBuildableAreas()
{
	if (!HardwareSupportsFramebuffers())
		return;

	if (DigitanksGame()->GetControlMode() != MODE_BUILD)
		return;

	// Render each visibility volume one at a time. If we do them all at once they interfere with each other.
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);
		if (!pDTEntity)
			continue;

		if (pDTEntity->BuildableArea() == 0)
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
		pDTEntity->RenderBuildableArea();

		glCullFace(GL_BACK);
		c.SetColor(Color(0, 0, 0));
		pDTEntity->RenderBuildableArea();

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
			RenderMapToBuffer(m_oVisibility1Buffer.m_iMap, &m_oBuildableAreaBuffer);
		else
		{
			glReadBuffer(GL_AUX1);
			glDrawBuffer(GL_AUX2);
		}

		c.SetBlend(BLEND_NONE);
		glEnable(GL_DEPTH_TEST);
	}

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);

	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
}

void CDigitanksRenderer::RenderOffscreenBuffers()
{
	if (ShouldUseFramebuffers() && ShouldUseShaders())
	{
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

	// Draw the fog of war.
	if (ShouldUseFramebuffers() && ShouldUseShaders() && DigitanksGame()->ShouldRenderFogOfWar())
	{
		UseProgram(0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glColor4ubv(Color(50, 250, 50, 100));
		RenderMapToBuffer(m_oBuildableAreaBuffer.m_iMap, &m_oSceneBuffer);
		glColor4ubv(Color(255, 255, 255));
		glDisable(GL_BLEND);

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

		if (DigitanksGame()->GetControlMode() == MODE_BUILD)
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
			glUniform1f(flBrightness, 0.7f - 0.1f*i);
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
	glEnable(GL_BLEND);

	if (ShouldUseFramebuffers())
	{
		glBlendFunc(GL_ONE, GL_ONE);
		for (size_t i = 0; i < BLOOM_FILTERS; i++)
			RenderMapFullscreen(m_oBloom1Buffers[i].m_iMap);

		float flBloomPulseLength = 2.0f;
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
