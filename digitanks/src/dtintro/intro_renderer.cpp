#include "intro_renderer.h"

#include <GL/glew.h>

#include <mtrand.h>

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <models/texturelibrary.h>
#include <game/gameserver.h>
#include <shaders/shaders.h>

CIntroRenderer::CIntroRenderer()
	: CRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
	m_bUseFramebuffers = false;
//	m_bUseShaders = false;

	m_iBackdrop = CTextureLibrary::AddTextureID(L"textures/intro/backdrop.png");
	m_flLayer1Speed = -RandomFloat(0.1f, 0.5f);
	m_flLayer2Speed = -RandomFloat(0.1f, 0.5f);
	m_flLayer3Speed = -RandomFloat(0.1f, 0.5f);
	m_flLayer4Speed = -RandomFloat(0.1f, 0.5f);
	m_flLayer5Speed = -RandomFloat(0.1f, 0.5f);

	m_flLayer1Alpha = RandomFloat(0.2f, 1);
	m_flLayer2Alpha = RandomFloat(0.2f, 1);
	m_flLayer3Alpha = RandomFloat(0.2f, 1);
	m_flLayer4Alpha = RandomFloat(0.2f, 1);
	m_flLayer5Alpha = RandomFloat(0.2f, 1);
}

#define FRUSTUM_NEAR	0
#define FRUSTUM_FAR		1
#define FRUSTUM_LEFT	2
#define FRUSTUM_RIGHT	3
#define FRUSTUM_UP		4
#define FRUSTUM_DOWN	5

CVar cam_free_ortho("cam_free_ortho", "off");

void CIntroRenderer::StartRendering()
{
	if (!cam_free_ortho.GetBool() && CVar::GetCVarBool("cam_free"))
	{
		BaseClass::StartRendering();
	
		RenderBackdrop();
		return;
	}

	float flWidth = (float)CApplication::Get()->GetWindowWidth();
	float flHeight = (float)CApplication::Get()->GetWindowHeight();

	glViewport(0, 0, (GLsizei)flWidth, (GLsizei)flHeight);

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-flWidth/2, flWidth/2, -flHeight/2, flHeight/2, 1, 2000);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	gluLookAt(m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z,
		m_vecCameraTarget.x, m_vecCameraTarget.y, m_vecCameraTarget.z,
		0.0, 1.0, 0.0);

	glGetDoublev( GL_MODELVIEW_MATRIX, m_aiModelView );
	glGetDoublev( GL_PROJECTION_MATRIX, m_aiProjection );

	Matrix4x4 mModelView, mProjection;
	glGetFloatv( GL_MODELVIEW_MATRIX, mModelView );
	glGetFloatv( GL_PROJECTION_MATRIX, mProjection );

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

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	RenderBackdrop();
}

void CIntroRenderer::FinishRendering()
{
	if (!cam_free_ortho.GetBool() && CVar::GetCVarBool("cam_free"))
	{
		BaseClass::FinishRendering();
		return;
	}

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

void CIntroRenderer::RenderBackdrop()
{
	float flWidth = (float)CApplication::Get()->GetWindowWidth();

	{
		CRenderingContext c(this);
		c.BindTexture(m_iBackdrop);
		c.UseProgram(CShaderLibrary::GetScrollingTextureProgram());

		c.SetUniform("iTexture", 0);
		c.SetUniform("flTime", GameServer()->GetGameTime());

		c.SetBlend(BLEND_ADDITIVE);
		c.SetDepthTest(false);

		c.SetUniform("flAlpha", m_flLayer1Alpha);
		c.SetUniform("flSpeed", m_flLayer1Speed);
		c.SetColor(Color(100, 255, 100, 255));

		c.BeginRenderQuads();
		c.TexCoord(Vector(1.8f, 0, 0));
		c.Vertex(Vector(-100, -flWidth/2, -flWidth/2));
		c.TexCoord(Vector(1.8f, 2, 0));
		c.Vertex(Vector(-100, flWidth/2, -flWidth/2));
		c.TexCoord(Vector(-0.2f, 2, 0));
		c.Vertex(Vector(-100, flWidth/2, flWidth/2));
		c.TexCoord(Vector(-0.2f, 0, 0));
		c.Vertex(Vector(-100, -flWidth/2, flWidth/2));
		c.EndRender();

		c.SetUniform("flAlpha", m_flLayer2Alpha);
		c.SetUniform("flSpeed", m_flLayer2Speed);
		c.SetColor(Color(0, 255, 0, 255));

		c.BeginRenderQuads();
		c.TexCoord(Vector(1.5f, -0.5f, 0));
		c.Vertex(Vector(-200, -flWidth/2, -flWidth/2));
		c.TexCoord(Vector(1.5f, 1.5f, 0));
		c.Vertex(Vector(-200, flWidth/2, -flWidth/2));
		c.TexCoord(Vector(-0.5f, 1.5f, 0));
		c.Vertex(Vector(-200, flWidth/2, flWidth/2));
		c.TexCoord(Vector(-0.5f, -0.5f, 0));
		c.Vertex(Vector(-200, -flWidth/2, flWidth/2));
		c.EndRender();

		c.SetUniform("flAlpha", m_flLayer3Alpha);
		c.SetUniform("flSpeed", m_flLayer3Speed);
		c.SetColor(Color(55, 255, 55, 155));

		c.BeginRenderQuads();
		c.TexCoord(Vector(1, -2.5f, 0));
		c.Vertex(Vector(-300, -flWidth/2, -flWidth/2));
		c.TexCoord(Vector(1, 1, 0));
		c.Vertex(Vector(-300, flWidth/2, -flWidth/2));
		c.TexCoord(Vector(-2.5f, 1, 0));
		c.Vertex(Vector(-300, flWidth/2, flWidth/2));
		c.TexCoord(Vector(-2.5f, -2.5f, 0));
		c.Vertex(Vector(-300, -flWidth/2, flWidth/2));
		c.EndRender();

		c.SetUniform("flAlpha", m_flLayer4Alpha);
		c.SetUniform("flSpeed", m_flLayer4Speed);
		c.SetColor(Color(100, 255, 100, 205));

		c.BeginRenderQuads();
		c.TexCoord(Vector(2.4f, 0.5f, 0));
		c.Vertex(Vector(-400, -flWidth/2, -flWidth/2));
		c.TexCoord(Vector(2.4f, 2.5f, 0));
		c.Vertex(Vector(-400, flWidth/2, -flWidth/2));
		c.TexCoord(Vector(0.4f, 2.5f, 0));
		c.Vertex(Vector(-400, flWidth/2, flWidth/2));
		c.TexCoord(Vector(0.4f, 0.5f, 0));
		c.Vertex(Vector(-400, -flWidth/2, flWidth/2));
		c.EndRender();

		c.SetUniform("flAlpha", m_flLayer5Alpha);
		c.SetUniform("flSpeed", m_flLayer5Speed);
		c.SetColor(Color(55, 255, 55, 120));

		c.BeginRenderQuads();
		c.TexCoord(Vector(0.9f, 0.2f, 0));
		c.Vertex(Vector(-500, -flWidth/2, -flWidth/2));
		c.TexCoord(Vector(0.9f, 1.2f, 0));
		c.Vertex(Vector(-500, flWidth/2, -flWidth/2));
		c.TexCoord(Vector(-.1f, 1.2f, 0));
		c.Vertex(Vector(-500, flWidth/2, flWidth/2));
		c.TexCoord(Vector(-.1f, 0.2f, 0));
		c.Vertex(Vector(-500, -flWidth/2, flWidth/2));
		c.EndRender();
	}
}
