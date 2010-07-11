#include "dt_renderer.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <shaders/shaders.h>
#include <game/digitanks/digitanksentity.h>
#include <game/digitanks/digitanksgame.h>

#include <ui/digitankswindow.h>

CDigitanksRenderer::CDigitanksRenderer()
	: CRenderer(CDigitanksWindow::Get()->GetWindowWidth(), CDigitanksWindow::Get()->GetWindowHeight())
{
	m_oExplosionBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, false, false);
	m_oVisibility1Buffer = CreateFrameBuffer(m_iWidth, m_iHeight, false, false);
	m_oVisibility2Buffer = CreateFrameBuffer(m_iWidth, m_iHeight, false, false);

	// Bind the regular scene's depth buffer to the explosion buffer so we can use it for depth compares.
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oExplosionBuffer.m_iFB);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)m_oSceneBuffer.m_iDepth);
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	// Same for visibility.
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oVisibility1Buffer.m_iFB);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)m_oSceneBuffer.m_iDepth);
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
}

void CDigitanksRenderer::SetupFrame()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oExplosionBuffer.m_iFB);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oVisibility2Buffer.m_iFB);
	glClear(GL_COLOR_BUFFER_BIT);

	BaseClass::SetupFrame();
}

void CDigitanksRenderer::FinishRendering()
{
	glUseProgram(0);

	RenderFogOfWar();

	BaseClass::FinishRendering();
}

void CDigitanksRenderer::RenderFogOfWar()
{
	if (!DigitanksGame()->ShouldRenderFogOfWar())
		return;

	// Render each visibility volume one at a time. If we do them all at once they interfere with each other.
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
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
		c.UseFrameBuffer(m_oVisibility1Buffer.m_iFB);
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
		glUseProgram(0);

		// Copy the results to the second buffer
		RenderMapToBuffer(m_oVisibility1Buffer.m_iMap, &m_oVisibility2Buffer);

		c.SetBlend(BLEND_NONE);
		glEnable(GL_DEPTH_TEST);
	}

	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
}

void CDigitanksRenderer::RenderOffscreenBuffers()
{
	// Render the explosions back onto the scene buffer, passing through the noise filter.
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oSceneBuffer.m_iFB);

	glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, (GLuint)m_oNoiseBuffer.m_iMap);

	GLuint iExplosionProgram = (GLuint)CShaderLibrary::GetExplosionProgram();
	glUseProgram(iExplosionProgram);

	GLint iExplosion = glGetUniformLocation(iExplosionProgram, "iExplosion");
    glUniform1i(iExplosion, 0);

	GLint iNoise = glGetUniformLocation(iExplosionProgram, "iNoise");
    glUniform1i(iNoise, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	RenderMapToBuffer(m_oExplosionBuffer.m_iMap, &m_oSceneBuffer);
	glDisable(GL_BLEND);

	glUseProgram(0);

	glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);


	// Draw the fog of war.
	// Explosion buffer's not in use anymore, reduce reuse recycle!
	if (DigitanksGame()->ShouldRenderFogOfWar())
	{
		RenderMapToBuffer(m_oSceneBuffer.m_iMap, &m_oExplosionBuffer);

		glActiveTexture(GL_TEXTURE1);
	    glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, (GLuint)m_oVisibility2Buffer.m_iMap);

		GLuint iDarkenProgram = (GLuint)CShaderLibrary::GetDarkenProgram();
		glUseProgram(iDarkenProgram);

		GLint iDarkMap = glGetUniformLocation(iDarkenProgram, "iDarkMap");
	    glUniform1i(iDarkMap, 1);

		GLint iImage = glGetUniformLocation(iDarkenProgram, "iImage");
	    glUniform1i(iImage, 0);

		GLint flFactor = glGetUniformLocation(iDarkenProgram, "flFactor");
	    glUniform1f(flFactor, 3.0f);

		RenderMapToBuffer(m_oExplosionBuffer.m_iMap, &m_oSceneBuffer);

		glUseProgram(0);

		glActiveTexture(GL_TEXTURE1);
	    glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
	}

	// Use a bright-pass filter to catch only the bright areas of the image
	GLuint iBrightPass = (GLuint)CShaderLibrary::GetBrightPassProgram();
	glUseProgram(iBrightPass);

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

	glUseProgram(0);

	RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
	RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);

	RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
	RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);
}

void CDigitanksRenderer::RenderFullscreenBuffers()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	for (size_t i = 0; i < BLOOM_FILTERS; i++)
		RenderMapFullscreen(m_oBloom1Buffers[i].m_iMap);
	glDisable(GL_BLEND);
}

#define KERNEL_SIZE   3
//float aflKernel[KERNEL_SIZE] = { 5, 6, 5 };
float aflKernel[KERNEL_SIZE] = { 0.3125f, 0.375f, 0.3125f };

void CDigitanksRenderer::RenderBloomPass(CFrameBuffer* apSources, CFrameBuffer* apTargets, bool bHorizontal)
{
	GLuint iBlur = (GLuint)CShaderLibrary::GetBlurProgram();
	glUseProgram(iBlur);

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

	glUseProgram(0);
}
