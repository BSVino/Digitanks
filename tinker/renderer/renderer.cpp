/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "renderer.h"

#include <SDL_image.h>

#include <maths.h>
#include <tinker_platform.h>
#include <stb_image_write.h>

#include <common/worklistener.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <textures/texturelibrary.h>

#include "tinker_gl.h"
#include "renderingcontext.h"

static tvector<CFrameBuffer> g_aFrameBuffers;
tvector<CFrameBuffer>& CFrameBuffer::GetFrameBuffers()
{
	return g_aFrameBuffers;
}

CFrameBuffer::CFrameBuffer()
{
	m_sName = "unnamed";

	m_iRB = m_iMap = m_iDepth = m_iDepthTexture = m_iFB = 0;
}

CFrameBuffer::CFrameBuffer(const tstring& sName)
{
	m_sName = sName;

	m_iRB = m_iMap = m_iDepth = m_iDepthTexture = m_iFB = 0;
}

void CFrameBuffer::Destroy()
{
	RemoveFromBufferList(this);

	if (m_iMap)
		glDeleteTextures(1, &m_iMap);

	if (m_iDepthTexture)
		glDeleteTextures(1, &m_iDepthTexture);

	if (m_iRB)
		glDeleteRenderbuffers(1, &m_iRB);

	if (m_iDepth)
		glDeleteRenderbuffers(1, &m_iDepth);

	if (m_iFB)
		glDeleteFramebuffers(1, &m_iFB);

	m_iRB = m_iMap = m_iDepth = m_iDepthTexture = m_iFB = 0;
}

void CFrameBuffer::AddToBufferList(CFrameBuffer* pBuffer)
{
	g_aFrameBuffers.push_back(*pBuffer);
}

void CFrameBuffer::RemoveFromBufferList(CFrameBuffer* pBuffer)
{
	for (size_t i = 0; i < g_aFrameBuffers.size(); i++)
	{
		if (pBuffer->m_iFB == g_aFrameBuffers[i].m_iFB)
		{
			g_aFrameBuffers.erase(g_aFrameBuffers.begin() + i);
			return;
		}
	}
}

CRenderer::CRenderer(size_t iWidth, size_t iHeight)
{
	TMsg(tsprintf("Initializing %dx%d renderer\n", iWidth, iHeight));

	if (!HardwareSupported())
	{
		TError("Hardware not supported!");
		Alert("Your hardware does not support OpenGL 3.0. Please try updating your drivers.");
		exit(1);
	}

	int iResult = IMG_Init(IMG_INIT_PNG);
	TAssert(iResult & IMG_INIT_PNG);

	m_bUseMultisampleTextures = T_PLATFORM_SUPPORTS_MULTISAMPLE;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glGetIntegerv(GL_SAMPLES, &m_iScreenSamples);

	SetSize(iWidth, iHeight);

	m_bCustomProjection = false;
	m_bFrustumOverride = false;
	m_bDrawBackground = true;

	m_bRenderOrthographic = true;
	m_flCameraOrthoHeight = 10;

	m_vecCameraDirection = Vector(1, 0, 0);
	m_vecCameraUp = Vector(0, 0, 1);
}

CRenderer::~CRenderer()
{
	CShaderLibrary::Destroy();

	m_oSceneBuffer.Destroy();
	m_oResolvedSceneBuffer.Destroy();

	for (int i = 0; i < BLOOM_FILTERS; i++)
	{
		m_oBloom1Buffers[i].Destroy();
		m_oBloom2Buffers[i].Destroy();
	}
}

void CRenderer::Initialize()
{
	LoadShaders();
	CShaderLibrary::CompileShaders(m_iScreenSamples);

	ViewportResize(m_iViewportWidth, m_iViewportHeight);

	if (!CShaderLibrary::IsCompiled())
	{
		TError("Shader compilation error!");
		Alert("There was a problem compiling shaders. Please send the files shaders.txt and glinfo.txt to jorge@lunarworkshop.com");
		OpenExplorer(Application()->GetAppDataDirectory());
		exit(1);
	}
	else
		TMsg(tsprintf("%d shaders loaded.\n", CShaderLibrary::GetNumShaders()));
}

void CRenderer::LoadShaders()
{
	CShaderLibrary::Initialize();

	tvector<tstring> asShaders = ListDirectory(T_ASSETS_PREFIX "shaders", false);

	int iShadersLoaded = 0;
	for (size_t i = 0; i < asShaders.size(); i++)
	{
		tstring sShader = asShaders[i];
		if (!sShader.endswith(".txt"))
			continue;

		CShaderLibrary::AddShader("shaders/" + sShader);
		iShadersLoaded++;
	}
}

void CRenderer::ViewportResize(size_t w, size_t h)
{
	m_oSceneBuffer.Destroy();
	m_oSceneBuffer = CreateFrameBuffer("scene", w, h, (fb_options_e)(FB_TEXTURE|FB_DEPTH|FB_MULTISAMPLE));

	if (m_iScreenSamples)
	{
		m_oResolvedSceneBuffer.Destroy();
		m_oResolvedSceneBuffer = CreateFrameBuffer("scene_resolved", w, h, (fb_options_e)(FB_TEXTURE|FB_DEPTH));
	}

	size_t iWidth = m_oSceneBuffer.m_iWidth;
	size_t iHeight = m_oSceneBuffer.m_iHeight;
	for (size_t i = 0; i < BLOOM_FILTERS; i++)
	{
		m_oBloom1Buffers[i].Destroy();
		m_oBloom2Buffers[i].Destroy();
		m_oBloom1Buffers[i] = CreateFrameBuffer(tsprintf("bloom1_%d", i), iWidth, iHeight, (fb_options_e)(FB_TEXTURE|FB_LINEAR));
		m_oBloom2Buffers[i] = CreateFrameBuffer(tsprintf("bloom2_%d", i), iWidth, iHeight, (fb_options_e)(FB_TEXTURE));
		iWidth /= 2;
		iHeight /= 2;
	}
}

size_t CRenderer::GetDrawableWidth()
{
	return (size_t)((float)m_iViewportWidth * Application()->GetGUIScale());
}

size_t CRenderer::GetDrawableHeight()
{
	return (size_t)((float)m_iViewportHeight * Application()->GetGUIScale());
}

CFrameBuffer CRenderer::CreateFrameBuffer(const tstring& sName, size_t iWidth, size_t iHeight, fb_options_e eOptions)
{
	TAssert((eOptions&FB_TEXTURE) ^ (eOptions&FB_RENDERBUFFER));

	if (!(eOptions&(FB_TEXTURE|FB_RENDERBUFFER)))
		eOptions = (fb_options_e)(eOptions|FB_TEXTURE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glGetIntegerv(GL_SAMPLES, &m_iScreenSamples);
	GLsizei iSamples = m_iScreenSamples;

	bool bUseMultisample = true;
	if (iSamples == 0)
		bUseMultisample = false;
	if (!(eOptions&FB_MULTISAMPLE))
		bUseMultisample = false;

	bool bMultisampleTexture = m_bUseMultisampleTextures && bUseMultisample;

	if ((eOptions&FB_TEXTURE) && !bMultisampleTexture)
		bUseMultisample = false;

	GLuint iTextureTarget = GL_TEXTURE_2D;
	if (bUseMultisample)
		iTextureTarget = GL_TEXTURE_2D_MULTISAMPLE;

	CFrameBuffer oBuffer(sName);
	oBuffer.m_bMultiSample = bUseMultisample;

	if (eOptions&FB_TEXTURE)
	{
		glGenTextures(1, &oBuffer.m_iMap);
		glBindTexture(iTextureTarget, (GLuint)oBuffer.m_iMap);
		if (bUseMultisample)
			glTexImage2DMultisample(iTextureTarget, iSamples, GL_RGBA, (GLsizei)iWidth, (GLsizei)iHeight, GL_FALSE);
		else
		{
			glTexParameteri(iTextureTarget, GL_TEXTURE_MIN_FILTER, (eOptions&FB_LINEAR)?GL_LINEAR:GL_NEAREST);
			glTexParameteri(iTextureTarget, GL_TEXTURE_MAG_FILTER, (eOptions&FB_LINEAR)?GL_LINEAR:GL_NEAREST);
			glTexParameteri(iTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(iTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			if (T_PLATFORM_SUPPORTS_HALF_FLOAT && (eOptions&FB_TEXTURE_HALF_FLOAT))
				glTexImage2D(iTextureTarget, 0, GL_RGBA16F, (GLsizei)iWidth, (GLsizei)iHeight, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
			else
				glTexImage2D(iTextureTarget, 0, GL_RGBA, (GLsizei)iWidth, (GLsizei)iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		glBindTexture(iTextureTarget, 0);
	}
	else if (eOptions&FB_RENDERBUFFER)
	{
		glGenRenderbuffers(1, &oBuffer.m_iRB);
		glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)oBuffer.m_iRB );
		if (bUseMultisample)
			glRenderbufferStorageMultisample( GL_RENDERBUFFER, iSamples, GL_RGBA8, (GLsizei)iWidth, (GLsizei)iHeight );
		else
			glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA8, (GLsizei)iWidth, (GLsizei)iHeight );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}

	if (eOptions&FB_DEPTH)
	{
		glGenRenderbuffers(1, &oBuffer.m_iDepth);
		glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
		if (bUseMultisample)
			glRenderbufferStorageMultisample( GL_RENDERBUFFER, iSamples, GL_DEPTH_COMPONENT, (GLsizei)iWidth, (GLsizei)iHeight );
		else
			glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, (GLsizei)iWidth, (GLsizei)iHeight );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}
	else if (eOptions&FB_DEPTH_TEXTURE)
	{
		glGenTextures(1, &oBuffer.m_iDepthTexture);
		glBindTexture(GL_TEXTURE_2D, oBuffer.m_iDepthTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, (GLsizei)iWidth, (GLsizei)iHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (eOptions&FB_SCENE_DEPTH)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)m_oSceneBuffer.m_iDepth);
	}

	glGenFramebuffers(1, &oBuffer.m_iFB);
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	if (eOptions&FB_TEXTURE)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, iTextureTarget, (GLuint)oBuffer.m_iMap, 0);
	else if (eOptions&FB_RENDERBUFFER)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, (GLuint)oBuffer.m_iRB);
	if (eOptions&FB_DEPTH)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
	else if (eOptions&FB_DEPTH_TEXTURE)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, (GLuint)oBuffer.m_iDepthTexture, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		TMsg(tsprintf("Framebuffer '" + sName + "' (%dx%d) incomplete, options: %d status: %d\n", iWidth, iHeight, eOptions, status));
	TAssert(status == GL_FRAMEBUFFER_COMPLETE);

	GLint iFBSamples;
	glGetIntegerv(GL_SAMPLES, &iFBSamples);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	TAssert(iFBSamples == iSamples || iFBSamples == 0 || 0 == iSamples);

	oBuffer.m_iWidth = iWidth;
	oBuffer.m_iHeight = iHeight;

	oBuffer.m_vecTexCoords[0] = Vector2D(0, 1);
	oBuffer.m_vecTexCoords[1] = Vector2D(0, 0);
	oBuffer.m_vecTexCoords[2] = Vector2D(1, 0);
	oBuffer.m_vecTexCoords[3] = Vector2D(1, 1);

	oBuffer.m_vecVertices[0] = Vector2D(0, 0);
	oBuffer.m_vecVertices[1] = Vector2D(0, (float)iHeight);
	oBuffer.m_vecVertices[2] = Vector2D((float)iWidth, (float)iHeight);
	oBuffer.m_vecVertices[3] = Vector2D((float)iWidth, 0);

	CFrameBuffer::AddToBufferList(&oBuffer);

	return oBuffer;
}

void CRenderer::DestroyFrameBuffer(CFrameBuffer* pBuffer)
{
	if (!pBuffer)
		return;

	pBuffer->Destroy();
}

void CRenderer::PreFrame()
{
}

void CRenderer::PostFrame()
{
}

void CRenderer::RenderFrame()
{
	PreRender();

	{
		CRenderingContext c(this);
		ModifyContext(&c);
		SetupFrame(&c);
		StartRendering(&c);

		Render(&c);

		FinishRendering(&c);
		FinishFrame(&c);
	}

	PostRender();
}

void CRenderer::PreRender()
{
	Application()->GetViewportSize(m_iViewportWidth, m_iViewportHeight);
}

void CRenderer::PostRender()
{
}

void CRenderer::ModifyContext(class CRenderingContext* pContext)
{
	pContext->UseFrameBuffer(&m_oSceneBuffer);
}

void CRenderer::SetupFrame(class CRenderingContext* pContext)
{
	pContext->ClearDepth();

	if (m_bDrawBackground)
		DrawBackground(pContext);
}

void CRenderer::DrawBackground(class CRenderingContext* pContext)
{
	pContext->ClearColor();
}

void CRenderer::StartRendering(class CRenderingContext* pContext)
{
	TPROF("CRenderer::StartRendering");

	float flAspectRatio = (float)m_iViewportWidth/(float)m_iViewportHeight;

	if (UseCustomProjection())
		pContext->SetProjection(m_mCustomProjection);
	else if (ShouldRenderOrthographic())
		pContext->SetProjection(Matrix4x4::ProjectOrthographic(
				-flAspectRatio*m_flCameraOrthoHeight, flAspectRatio*m_flCameraOrthoHeight,
				-m_flCameraOrthoHeight, m_flCameraOrthoHeight,
				-100, 100
			));
	else
		pContext->SetProjection(Matrix4x4::ProjectPerspective(
				m_flCameraFOV,
				flAspectRatio,
				m_flCameraNear,
				m_flCameraFar
			));

	pContext->SetView(Matrix4x4::ConstructCameraView(m_vecCameraPosition, m_vecCameraDirection, m_vecCameraUp));

	m_aflModelView = pContext->GetView();
	m_aflProjection = pContext->GetProjection();

	if (m_bFrustumOverride)
	{
		Matrix4x4 mProjection = Matrix4x4::ProjectPerspective(
				m_flFrustumFOV,
				(float)m_iViewportWidth/(float)m_iViewportHeight,
				m_flFrustumNear,
				m_flFrustumFar
			);

		Matrix4x4 mView = Matrix4x4::ConstructCameraView(m_vecFrustumPosition, m_vecFrustumDirection, m_vecCameraUp);

		m_oFrustum.CreateFrom(mProjection * mView);
	}
	else
		m_oFrustum.CreateFrom(pContext->GetProjection() * pContext->GetView());

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glViewport(0, 0, (GLsizei)m_iViewportWidth, (GLsizei)m_iViewportHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
	glViewport(0, 0, (GLsizei)m_oSceneBuffer.m_iWidth, (GLsizei)m_oSceneBuffer.m_iHeight);

	if (m_iScreenSamples)
		glEnable(GL_MULTISAMPLE);
}

CVar show_frustum("debug_show_frustum", "no");

void CRenderer::FinishRendering(class CRenderingContext*)
{
	if (m_iScreenSamples)
		glDisable(GL_MULTISAMPLE);

	if (show_frustum.GetBool())
	{
		TUnimplemented();

		for (size_t i = 0; i < 6; i++)
		{
			Vector vecForward = m_oFrustum.p[i].n;
			Vector vecLeft = -vecForward.Cross(Vector(0, 0, 1)).Normalized();
			Vector vecUp = -vecLeft.Cross(vecForward).Normalized();
			Vector vecCenter = vecForward * m_oFrustum.p[i].d;

			vecForward *= 100;
			vecLeft *= 100;
			vecUp *= 100;

/*			glBegin(GL_QUADS);
				glVertex3fv(vecCenter + vecUp + vecRight);
				glVertex3fv(vecCenter - vecUp + vecRight);
				glVertex3fv(vecCenter - vecUp - vecRight);
				glVertex3fv(vecCenter + vecUp - vecRight);
			glEnd();*/
		}
	}
}

void CRenderer::FinishFrame(class CRenderingContext* pContext)
{
	pContext->SetProjection(Matrix4x4());
	pContext->SetView(Matrix4x4());

	RenderOffscreenBuffers(pContext);

	RenderFullscreenBuffers(pContext);
}

CVar r_bloom("r_bloom", "1");

void CRenderer::RenderOffscreenBuffers(class CRenderingContext*)
{
	if (r_bloom.GetBool())
	{
		TPROF("Bloom");

		if (m_iScreenSamples)
			RenderBufferToBuffer(&m_oSceneBuffer, &m_oResolvedSceneBuffer);

		CRenderingContext c(this);

		// Use a bright-pass filter to catch only the bright areas of the image
		c.UseProgram("brightpass");

		c.SetUniform("iSource", 0);
		c.SetUniform("flScale", (float)1/BLOOM_FILTERS);

		for (size_t i = 0; i < BLOOM_FILTERS; i++)
		{
			c.SetUniform("flBrightness", BloomBrightnessCutoff() - 0.1f*i);
			if (m_iScreenSamples)
				RenderMapToBuffer(m_oResolvedSceneBuffer.m_iMap, &m_oBloom1Buffers[i]);
			else
				RenderMapToBuffer(m_oSceneBuffer.m_iMap, &m_oBloom1Buffers[i]);
		}

		// Blur it up! Oooooh darlin' blur it up! Come on baby
		RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
		RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);

		RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
		RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);
	}
}

CVar r_bloom_buffer("r_bloom_buffer", "-1");

void CRenderer::RenderFullscreenBuffers(class CRenderingContext*)
{
	TPROF("CRenderer::RenderFullscreenBuffers");

	if (m_iScreenSamples)
	{
		RenderBufferToBuffer(&m_oSceneBuffer, &m_oResolvedSceneBuffer);
		RenderFrameBufferFullscreen(&m_oResolvedSceneBuffer);
	}
	else
		RenderFrameBufferFullscreen(&m_oSceneBuffer);

	if (r_bloom.GetBool())
	{
		if (r_bloom_buffer.GetInt() >= 0 && r_bloom_buffer.GetInt() < BLOOM_FILTERS)
		{
			CRenderingContext c(this);
			RenderFrameBufferFullscreen(&m_oBloom1Buffers[r_bloom_buffer.GetInt()]);
		}
		else
		{
			float flScale = r_bloom.GetFloat() * BloomScale();

			CRenderingContext c(this);

			c.UseProgram("quad");
			c.SetUniform("vecColor", Vector4D(flScale, flScale, flScale, flScale));
			c.SetBlend(BLEND_ADDITIVE);

			for (size_t i = 0; i < BLOOM_FILTERS; i++)
				RenderFrameBufferFullscreen(&m_oBloom1Buffers[i]);
		}
	}
}

#define KERNEL_SIZE   3
//float aflKernel[KERNEL_SIZE] = { 5, 6, 5 };
float aflKernel[KERNEL_SIZE] = { 0.3125f, 0.375f, 0.3125f };

void CRenderer::RenderBloomPass(CFrameBuffer* apSources, CFrameBuffer* apTargets, bool bHorizontal)
{
	CRenderingContext c(this);

	c.UseProgram("blur");

	c.SetUniform("iSource", 0);
	c.SetUniform("aflCoefficients", KERNEL_SIZE, &aflKernel[0]);
	c.SetUniform("flOffsetX", 0.0f);
	c.SetUniform("flOffsetY", 0.0f);

    // Perform the blurring.
    for (size_t i = 0; i < BLOOM_FILTERS; i++)
    {
		if (bHorizontal)
			c.SetUniform("flOffsetX", 1.2f / apSources[i].m_iWidth);
		else
			c.SetUniform("flOffsetY", 1.2f / apSources[i].m_iWidth);

		RenderMapToBuffer(apSources[i].m_iMap, &apTargets[i]);
    }
}

void CRenderer::RenderFrameBufferFullscreen(CFrameBuffer* pBuffer)
{
	if (pBuffer->m_iMap)
		RenderMapFullscreen(pBuffer->m_iMap, pBuffer->m_bMultiSample);
	else if (pBuffer->m_iRB)
		RenderRBFullscreen(pBuffer);
}

void CRenderer::RenderRBFullscreen(CFrameBuffer* /*pSource*/)
{
	TAssert(false);		// ATI cards don't like this at all. Never do it.

#if 0
	glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)pSource->m_iFB);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(0, 0, pSource->m_iWidth, pSource->m_iHeight, 0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST);
#endif
}

void CRenderer::RenderBufferToBuffer(CFrameBuffer* pSource, CFrameBuffer* pDestination)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)pSource->m_iFB);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)pDestination->m_iFB);

	glBlitFramebuffer(0, 0, pSource->m_iWidth, pSource->m_iHeight, 0, 0, pDestination->m_iWidth, pDestination->m_iHeight, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void CRenderer::RenderMapFullscreen(size_t iMap, bool bMapIsMultisample)
{
	CRenderingContext c(this, true);

	c.SetWinding(true);
	c.SetDepthTest(false);
	c.UseFrameBuffer(0);

	if (!c.GetActiveProgram())
	{
		c.UseProgram("quad");
		c.SetUniform("vecColor", Vector4D(1, 1, 1, 1));
	}

	c.SetUniform("iDiffuse", 0);
	c.SetUniform("bDiffuse", true);

	c.BeginRenderVertexArray();

	c.SetTexCoordBuffer(&m_vecFullscreenTexCoords[0][0]);
	c.SetPositionBuffer(&m_vecFullscreenVertices[0][0]);

	c.BindTexture(iMap, 0, bMapIsMultisample);

	c.EndRenderVertexArray(6);
}

void CRenderer::RenderMapToBuffer(size_t iMap, CFrameBuffer* pBuffer, bool bMapIsMultisample)
{
	CRenderingContext c(this, true);

	c.SetWinding(true);
	c.SetDepthTest(false);
	c.UseFrameBuffer(pBuffer);

	if (!c.GetActiveProgram())
	{
		c.UseProgram("quad");
		c.SetUniform("vecColor", Vector4D(1, 1, 1, 1));
	}

	c.SetUniform("iDiffuse", 0);
	c.SetUniform("bDiffuse", true);

	c.BeginRenderVertexArray();

	c.SetTexCoordBuffer(&m_vecFullscreenTexCoords[0][0]);
	c.SetPositionBuffer(&m_vecFullscreenVertices[0][0]);

	c.BindTexture(iMap, 0, bMapIsMultisample);

	c.EndRenderVertexArray(6);
}

const Matrix4x4& CRenderer::GetModelView() const
{
	return m_aflModelView;
}

const Matrix4x4& CRenderer::GetProjection() const
{
	return m_aflProjection;
}

const int* CRenderer::GetViewport() const
{
	return &m_aiViewport[0];
}

void CRenderer::FrustumOverride(Vector vecPosition, Vector vecDirection, float flFOV, float flNear, float flFar)
{
	m_bFrustumOverride = true;
	m_vecFrustumPosition = vecPosition;
	m_vecFrustumDirection = vecDirection;
	m_flFrustumFOV = flFOV;
	m_flFrustumNear = flNear;
	m_flFrustumFar = flFar;
}

void CRenderer::CancelFrustumOverride()
{
	m_bFrustumOverride = false;
}

Vector CRenderer::GetCameraVector()
{
	return m_vecCameraDirection;
}

void CRenderer::GetCameraVectors(Vector* pvecForward, Vector* pvecLeft, Vector* pvecUp)
{
	Vector vecForward = GetCameraVector();
	Vector vecLeft;

	if (pvecForward)
		(*pvecForward) = vecForward;

	if (pvecLeft || pvecUp)
		vecLeft = m_vecCameraUp.Cross(vecForward).Normalized();

	if (pvecLeft)
		(*pvecLeft) = vecLeft;

	if (pvecUp)
		(*pvecUp) = vecForward.Cross(vecLeft).Normalized();
}

bool CRenderer::IsSphereInFrustum(const Vector& vecCenter, float flRadius)
{
	return m_oFrustum.TouchesSphere(vecCenter, flRadius);
}

CTextureHandle CRenderer::GetInvalidTexture() const
{
	return CTextureHandle();
}

CMaterialHandle CRenderer::GetInvalidMaterial() const
{
	return CMaterialHandle();
}

void CRenderer::SetSize(int w, int h)
{
	m_iViewportWidth = w;
	m_iViewportHeight = h;

	m_vecFullscreenTexCoords[0] = Vector2D(0, 1);
	m_vecFullscreenTexCoords[1] = Vector2D(1, 0);
	m_vecFullscreenTexCoords[2] = Vector2D(0, 0);
	m_vecFullscreenTexCoords[3] = Vector2D(0, 1);
	m_vecFullscreenTexCoords[4] = Vector2D(1, 1);
	m_vecFullscreenTexCoords[5] = Vector2D(1, 0);

	m_vecFullscreenVertices[0] = Vector(-1, -1, 0);
	m_vecFullscreenVertices[1] = Vector(1, 1, 0);
	m_vecFullscreenVertices[2] = Vector(-1, 1, 0);
	m_vecFullscreenVertices[3] = Vector(-1, -1, 0);
	m_vecFullscreenVertices[4] = Vector(1, -1, 0);
	m_vecFullscreenVertices[5] = Vector(1, 1, 0);
}

Vector CRenderer::ScreenPosition(Vector vecWorld)
{
	Vector4D v;

	v.x = vecWorld.x;
	v.y = vecWorld.y;
	v.z = vecWorld.z;
	v.w = 1.0;

	v = m_aflProjection * m_aflModelView * v;

	if (v.w == 0.0)
		return Vector(0, 0, 0);

	v.x /= v.w;
	v.y /= v.w;
	v.z /= v.w;

	/* Map x, y and z to range 0-1 */
	v.x = v.x * 0.5f + 0.5f;
	v.y = v.y * 0.5f + 0.5f;
	v.z = v.z * 0.5f + 0.5f;

	/* Map x,y to viewport */
	v.x = v.x * m_aiViewport[2] + m_aiViewport[0];
	v.y = v.y * m_aiViewport[3] + m_aiViewport[1];

	return Vector(v.x, m_iViewportHeight - v.y, v.z);
}

Vector CRenderer::WorldPosition(Vector vecScreen)
{
	Matrix4x4 mFinal = m_aflModelView * m_aflProjection;

	Vector4D v(vecScreen.x, m_iViewportHeight - vecScreen.y, vecScreen.z, 1.0);

	v.x = (v.x - m_aiViewport[0]) / m_aiViewport[2];
	v.y = (v.y - m_aiViewport[1]) / m_aiViewport[3];

	/* Map to range -1 to 1 */
	v.x = v.x * 2 - 1;
	v.y = v.y * 2 - 1;
	v.z = v.z * 2 - 1;

	v = mFinal * v;

	if (v.w == 0.0)
		return Vector();

	return Vector(v.x, v.y, v.z) / v.w;
}

bool CRenderer::HardwareSupported()
{
#ifdef __gl3w_h_
	if (!gl3wIsSupported(3, 0))
		return false;
#endif

	// Compile a test framebuffer. If it fails we don't support framebuffers.

	CFrameBuffer oBuffer("hardware_test");

	glGenTextures(1, &oBuffer.m_iMap);
	glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &oBuffer.m_iDepth);
	glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 512, 512 );
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	glGenFramebuffers(1, &oBuffer.m_iFB);
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		TError(tsprintf("Test framebuffer compile failed. Status: %d\n", status));
		glDeleteTextures(1, &oBuffer.m_iMap);
		glDeleteRenderbuffers(1, &oBuffer.m_iDepth);
		glDeleteFramebuffers(1, &oBuffer.m_iFB);
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	oBuffer.Destroy();

#ifdef TINKER_OPENGLES_3
	// Compile a test shader. If it fails we don't support shaders.
	const char* pszVertexShader =
		"#version 300 es\n"
		"void main()"
		"{"
		"	gl_Position = vec4(0.0, 0.0, 0.0, 0.0);"
		"}";

	const char* pszFragmentShader =
		"#version 300 es\n"
		"precision highp float;"
		"layout(location = 0) out highp vec4 vecOut;"
		"void main(void)"
		"{"
		"	vecOut = vec4(1.0, 1.0, 1.0, 1.0);"
		"}";
#else
	// Compile a test shader. If it fails we don't support shaders.
	const char* pszVertexShader =
		"#version 130\n"
		"void main()"
		"{"
		"	gl_Position = vec4(0.0, 0.0, 0.0, 0.0);"
		"}";

	const char* pszFragmentShader =
		"#version 130\n"
		"out vec4 vecFragColor;"
		"void main()"
		"{"
		"	vecFragColor = vec4(1.0, 1.0, 1.0, 1.0);"
		"}";
#endif

	GLuint iVShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint iFShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint iProgram = glCreateProgram();

	glShaderSource(iVShader, 1, &pszVertexShader, NULL);
	glCompileShader(iVShader);

	int iVertexCompiled;
	glGetShaderiv(iVShader, GL_COMPILE_STATUS, &iVertexCompiled);

	if (iVertexCompiled != GL_TRUE)
	{
		TMsg("Test vertex shader compile failed.\n");

		int iLogLength = 0;
		char szLog[1024];
		glGetShaderInfoLog((GLuint)iVShader, 1024, &iLogLength, szLog);

		TMsg(szLog);
	}

	glShaderSource(iFShader, 1, &pszFragmentShader, NULL);
	glCompileShader(iFShader);

	int iFragmentCompiled;
	glGetShaderiv(iFShader, GL_COMPILE_STATUS, &iFragmentCompiled);

	if (iFragmentCompiled != GL_TRUE)
	{
		TMsg("Test fragment shader compile failed.\n");

		int iLogLength = 0;
		char szLog[1024];
		glGetShaderInfoLog((GLuint)iFShader, 1024, &iLogLength, szLog);

		TMsg(szLog);
	}

	glAttachShader(iProgram, iVShader);
	glAttachShader(iProgram, iFShader);
	glLinkProgram(iProgram);

	int iProgramLinked;
	glGetProgramiv(iProgram, GL_LINK_STATUS, &iProgramLinked);

	if (!(iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE))
		TError("Test shader compile failed.\n");

	glDetachShader(iProgram, iVShader);
	glDetachShader(iProgram, iFShader);
	glDeleteShader(iVShader);
	glDeleteShader(iFShader);
	glDeleteProgram(iProgram);

	return iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE;
}

size_t CRenderer::LoadVertexDataIntoGL(size_t iSizeInBytes, const float* aflVertices)
{
	// If it's only floats doubles and the occasional int then it should always be a multiple of four bytes.
	TAssert(iSizeInBytes%4 == 0);

	GLuint iVBO;
	glGenBuffers(1, &iVBO);
	glBindBuffer(GL_ARRAY_BUFFER, iVBO);

	glBufferData(GL_ARRAY_BUFFER, iSizeInBytes, 0, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, iSizeInBytes, aflVertices);

	int iSize = 0;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &iSize);
	if(iSizeInBytes != (size_t)iSize)
	{
		glDeleteBuffers(1, &iVBO);
		TAssert(false);
		TError("CRenderer::LoadVertexDataIntoGL(): Data size is mismatch with input array\n");
		return 0;
	}

	return iVBO;
}

size_t CRenderer::LoadIndexDataIntoGL(size_t iSizeInBytes, const unsigned int* aiIndices)
{
	GLuint iVBO;
	glGenBuffers(1, &iVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iVBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSizeInBytes, 0, GL_STATIC_DRAW);

	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, iSizeInBytes, aiIndices);

	int iSize = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &iSize);
	if(iSizeInBytes != (size_t)iSize)
	{
		glDeleteBuffers(1, &iVBO);
		TAssert(false);
		TError("CRenderer::LoadVertexDataIntoGL(): Data size is mismatch with input array\n");
		return 0;
	}

	return iVBO;
}

void CRenderer::UnloadVertexDataFromGL(size_t iBuffer)
{
	glDeleteBuffers(1, (GLuint*)&iBuffer);
}

size_t CRenderer::LoadTextureIntoGL(tstring sFilename, int iClamp)
{
	if (!sFilename.length())
		return 0;

	if (!IsFile(sFilename))
		return 0;

	SDL_Surface* pSurface = IMG_Load(sFilename.c_str());

	if (!pSurface)
	{
		TError("Couldn't load '" + sFilename + "', reason: " + IMG_GetError() + "\n");
		return 0;
	}

	if (!s_bNPO2TextureLoads)
	{
		if (pSurface->w & (pSurface->w - 1))
		{
			TError("Image width is not power of 2.");
			SDL_FreeSurface(pSurface);
			return 0;
		}

		if (pSurface->h & (pSurface->h - 1))
		{
			TError("Image height is not power of 2.");
			SDL_FreeSurface(pSurface);
			return 0;
		}
	}

	TAssert(!SDL_MUSTLOCK(pSurface));

	SDL_PixelFormat fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.format = SDL_PIXELFORMAT_ABGR8888;
	fmt.BitsPerPixel = 32;
	fmt.BytesPerPixel = 4;
	fmt.Rshift = fmt.Ashift = fmt.Rloss = fmt.Gloss = fmt.Bloss = fmt.Aloss = 0;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	fmt.Rmask = 0xff000000;
	fmt.Rshift = 24;
	fmt.Gmask = 0x00ff0000;
	fmt.Gshift = 16;
	fmt.Bmask = 0x0000ff00;
	fmt.Bshift = 8;
	fmt.Amask = 0x000000ff;
#else
	fmt.Rmask = 0x000000ff;
	fmt.Gmask = 0x0000ff00;
	fmt.Gshift = 8;
	fmt.Bmask = 0x00ff0000;
	fmt.Bshift = 16;
	fmt.Amask = 0xff000000;
	fmt.Ashift = 24;
#endif

	bool bCorrectFormat = fmt.format == pSurface->format->format;

	if (!bCorrectFormat)
	{
		SDL_Surface* pSurfaceRGB = SDL_ConvertSurface(pSurface, &fmt, 0);

		// Don't need the original anymore.
		SDL_FreeSurface(pSurface);

		pSurface = pSurfaceRGB;
	}

	size_t iGLId = LoadTextureIntoGL((Color*)pSurface->pixels, pSurface->w, pSurface->h, iClamp);

	SDL_FreeSurface(pSurface);

	return iGLId;
}

size_t CRenderer::LoadTextureIntoGL(Color* pclrData, int x, int y, int iClamp, bool bNearestFiltering)
{
	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bNearestFiltering?GL_NEAREST:GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bNearestFiltering?GL_NEAREST:GL_LINEAR_MIPMAP_LINEAR);

	if (iClamp == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	gluBuild2DMipmaps(GL_TEXTURE_2D,
		4,
		x,
		y,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pclrData);

	glBindTexture(GL_TEXTURE_2D, 0);

	s_iTexturesLoaded++;

	return iGLId;
}

size_t CRenderer::LoadTextureIntoGL(Vector* pvecData, int x, int y, int iClamp, bool bMipMaps)
{
	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (iClamp == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	if (bMipMaps)
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, x, y, GL_RGB, GL_FLOAT, pvecData);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_FLOAT, pvecData);

	glBindTexture(GL_TEXTURE_2D, 0);

	s_iTexturesLoaded++;

	return iGLId;
}

void CRenderer::UnloadTextureFromGL(size_t iGLId)
{
	glDeleteTextures(1, (GLuint*)&iGLId);
	s_iTexturesLoaded--;
}

size_t CRenderer::s_iTexturesLoaded = 0;
bool CRenderer::s_bNPO2TextureLoads = false;
tvector<struct SDL_Surface*> CRenderer::s_apSurfaces;

Color* CRenderer::LoadTextureData(tstring sFilename, int& x, int& y)
{
	if (!sFilename.length())
		return nullptr;

	if (!IsFile(sFilename))
		return nullptr;

	FILE* fp = tfopen_asset(sFilename, "rb");
	if (!fp)
		return nullptr;

	fseek(fp, 0, SEEK_END);
	int iSize = ftell(fp);
	rewind(fp);

	tstring sFile;
	sFile.resize(iSize);
	int iRead = fread((void*)sFile.data(), iSize, 1, fp);
	TAssertNoMsg(iRead == 1);

	SDL_RWops* pRWOps = SDL_RWFromMem((void*)sFile.data(), iSize);
	if (!pRWOps)
		return nullptr;

	SDL_Surface* pSurface = IMG_LoadTyped_RW(pRWOps, 1, NULL);

	if (!pSurface)
	{
		TError("Couldn't load '" + sFilename + "', reason: " + IMG_GetError() + "\n");
		return nullptr;
	}

	x = pSurface->w;
	y = pSurface->h;

	if (!s_bNPO2TextureLoads)
	{
		if (x & (x-1))
		{
			TError("Image width is not power of 2.");
			SDL_FreeSurface(pSurface);
			return nullptr;
		}

		if (y & (y-1))
		{
			TError("Image height is not power of 2.");
			SDL_FreeSurface(pSurface);
			return nullptr;
		}
	}

	SDL_PixelFormat fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.format = SDL_PIXELFORMAT_ABGR8888;
	fmt.BitsPerPixel = 32;
	fmt.BytesPerPixel = 4;
	fmt.Rshift = fmt.Ashift = fmt.Rloss = fmt.Gloss = fmt.Bloss = fmt.Aloss = 0;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	fmt.Rmask = 0xff000000;
	fmt.Rshift = 24;
	fmt.Gmask = 0x00ff0000;
	fmt.Gshift = 16;
	fmt.Bmask = 0x0000ff00;
	fmt.Bshift = 8;
	fmt.Amask = 0x000000ff;
#else
	fmt.Rmask = 0x000000ff;
	fmt.Gmask = 0x0000ff00;
	fmt.Gshift = 8;
	fmt.Bmask = 0x00ff0000;
	fmt.Bshift = 16;
	fmt.Amask = 0xff000000;
	fmt.Ashift = 24;
#endif

	bool bCorrectFormat = fmt.format == pSurface->format->format;

	if (!bCorrectFormat)
	{
		SDL_Surface* pSurfaceRGB = SDL_ConvertSurface(pSurface, &fmt, 0);

		// Don't need the original anymore.
		SDL_FreeSurface(pSurface);

		pSurface = pSurfaceRGB;
	}

	s_apSurfaces.push_back(pSurface);

	return (Color*)pSurface->pixels;
}

void CRenderer::UnloadTextureData(Color* pData)
{
	size_t iFree = (size_t)~0;

	// Linear search through our surfaces to find the one we should free.
	for (size_t i = 0; i < s_apSurfaces.size(); i++)
	{
		if (s_apSurfaces[i]->pixels == pData)
		{
			iFree = i;
			break;
		}
	}

	TAssert(iFree != ~0);

	if (iFree == ~0)
		return;

	SDL_FreeSurface(s_apSurfaces[iFree]);
	s_apSurfaces.erase(s_apSurfaces.begin() + iFree);
}

void CRenderer::ReadTextureFromGL(CTextureHandle hTexture, Vector* pvecData)
{
	TAssert(hTexture.IsValid());
	if (!hTexture.IsValid())
		return;

#ifdef __ANDROID__
	TUnimplemented();
#else
	glBindTexture(GL_TEXTURE_2D, hTexture->m_iGLID);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, pvecData);

	glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

void CRenderer::ReadTextureFromGL(CTextureHandle hTexture, Color* pclrData)
{
	TAssert(hTexture.IsValid());
	if (!hTexture.IsValid())
		return;

#ifdef __ANDROID__
	TUnimplemented();
#else
	glBindTexture(GL_TEXTURE_2D, hTexture->m_iGLID);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pclrData);

	glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

void CRenderer::WriteTextureToFile(size_t iTexture, tstring sFilename)
{
#ifdef __ANDROID__
	TUnimplemented();
#else
	glBindTexture(GL_TEXTURE_2D, iTexture);

	int iWidth, iHeight;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &iHeight);

	tvector<Color> aclrPixels;
	aclrPixels.resize(iWidth*iHeight);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, aclrPixels.data());

	if (sFilename.endswith(".png"))
		stbi_write_png(sFilename.c_str(), iWidth, iHeight, 4, aclrPixels.data(), 0);
	else if (sFilename.endswith(".tga"))
		stbi_write_tga(sFilename.c_str(), iWidth, iHeight, 4, aclrPixels.data());
	else if (sFilename.endswith(".bmp"))
		stbi_write_bmp(sFilename.c_str(), iWidth, iHeight, 4, aclrPixels.data());
#endif
}

void CRenderer::WriteTextureToFile(Color* pclrData, int w, int h, tstring sFilename)
{
	if (sFilename.endswith(".png"))
		stbi_write_png(sFilename.c_str(), w, h, 4, pclrData, 0);
	else if (sFilename.endswith(".tga"))
		stbi_write_tga(sFilename.c_str(), w, h, 4, pclrData);
	else if (sFilename.endswith(".bmp"))
		stbi_write_bmp(sFilename.c_str(), w, h, 4, pclrData);
}

void R_DumpFBO(class CCommand*, tvector<tstring>& asTokens, const tstring&)
{
	size_t iFBO = 0;
	if (asTokens.size() > 1)
		iFBO = stoi(asTokens[1]);

	int aiViewport[4];
	glGetIntegerv( GL_VIEWPORT, aiViewport );

	int iWidth = aiViewport[2];
	int iHeight = aiViewport[3];

	tvector<Color> aclrPixels;
	aclrPixels.resize(iWidth*iHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)iFBO);
	glViewport(0, 0, (GLsizei)iWidth, (GLsizei)iHeight);

	// In case it's multisampled, blit it over to a normal one first.
	CFrameBuffer oResolvedBuffer = Application()->GetRenderer()->CreateFrameBuffer("resolved", iWidth, iHeight, (fb_options_e)(FB_RENDERBUFFER|FB_DEPTH));

	glBindFramebuffer(GL_READ_FRAMEBUFFER, iFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oResolvedBuffer.m_iFB);
	glBlitFramebuffer(
		0, 0, iWidth, iHeight, 
		0, 0, oResolvedBuffer.m_iWidth, oResolvedBuffer.m_iHeight, 
		GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)oResolvedBuffer.m_iFB);
	glReadPixels(0, 0, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, aclrPixels.data());

	oResolvedBuffer.Destroy();

	for (int i = 0; i < iWidth; i++)
	{
		for (int j = 0; j < iHeight/2; j++)
			std::swap(aclrPixels[j*iWidth + i], aclrPixels[iWidth*(iHeight-j-1) + i]);
	}

	CRenderer::WriteTextureToFile(aclrPixels.data(), iWidth, iHeight, tsprintf("fbo-%d.png", iFBO));
}

CCommand r_dumpfbo(tstring("r_dumpfbo"), ::R_DumpFBO);

void R_ListFBOs(class CCommand*, tvector<tstring>&, const tstring&)
{
	for (size_t i = 0; i < CFrameBuffer::GetFrameBuffers().size(); i++)
	{
		auto& oFrameBuffer = CFrameBuffer::GetFrameBuffers()[i];
		TMsg(tsprintf("Buffer %d \"%s\" (%dx%d): RB:%d, Map:%d, Depth:%d, DepthTexture:%d, Multisample:%s\n",
			oFrameBuffer.m_iFB, oFrameBuffer.m_sName.c_str(), oFrameBuffer.m_iWidth, oFrameBuffer.m_iHeight, oFrameBuffer.m_iRB,
			oFrameBuffer.m_iMap, oFrameBuffer.m_iDepth, oFrameBuffer.m_iDepthTexture, oFrameBuffer.m_bMultiSample?"yes":"no"));
	}
}

CCommand r_listfbos(tstring("r_listfbos"), ::R_ListFBOs);
