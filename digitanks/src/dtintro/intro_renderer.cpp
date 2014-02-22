#include "intro_renderer.h"

#include <mtrand.h>

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <textures/materiallibrary.h>
#include <game/gameserver.h>
#include <renderer/shaders.h>
#include <renderer/game_renderingcontext.h>

#include "intro_window.h"

CIntroRenderer::CIntroRenderer()
	: CGameRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
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

	m_bRenderOrthographic = true;
}

#define FRUSTUM_NEAR	0
#define FRUSTUM_FAR		1
#define FRUSTUM_LEFT	2
#define FRUSTUM_RIGHT	3
#define FRUSTUM_UP		4
#define FRUSTUM_DOWN	5

CVar cam_free_ortho("cam_free_ortho", "off");

void CIntroRenderer::Initialize()
{
	BaseClass::Initialize();

	m_hBackdrop = CMaterialLibrary::AddMaterial("textures/intro/backdrop.mat");

	m_bDrawBackground = true;

	CVar::SetCVar("r_bloom", 0.7f);
}

void CIntroRenderer::StartRendering(class CRenderingContext* pContext)
{
	BaseClass::StartRendering(pContext);

	RenderBackdrop();
}

void CIntroRenderer::RenderBackdrop()
{
	float flWidth = (float)CApplication::Get()->GetWindowWidth();

	{
		CGameRenderingContext c(this, true);
		c.UseMaterial(m_hBackdrop);
		c.UseProgram("scroll");

		c.SetUniform("flTime", (float)GameServer()->GetGameTime());

		c.SetBlend(BLEND_ADDITIVE);
		c.SetDepthTest(false);

		c.SetUniform("flAlpha", m_flLayer1Alpha);
		c.SetUniform("flSpeed", m_flLayer1Speed);
		c.SetUniform("vecColor", Color(100, 255, 100, 255));

		c.BeginRenderTriFan();
		c.TexCoord(Vector(-0.2f, 0, 0));
		c.Vertex(Vector(-100, -flWidth/2, -flWidth/2));
		c.TexCoord(Vector(1.8f, 0, 0));
		c.Vertex(Vector(-100, flWidth/2, -flWidth/2));
		c.TexCoord(Vector(1.8f, 2, 0));
		c.Vertex(Vector(-100, flWidth/2, flWidth/2));
		c.TexCoord(Vector(-0.2f, 2, 0));
		c.Vertex(Vector(-100, -flWidth/2, flWidth/2));
		c.EndRender();

		c.SetUniform("flAlpha", m_flLayer2Alpha);
		c.SetUniform("flSpeed", m_flLayer2Speed);
		c.SetUniform("vecColor", Color(0, 255, 0, 255));

		c.BeginRenderTriFan();
		c.TexCoord(Vector(-0.5f, -0.5f, 0));
		c.Vertex(Vector(-200, -flWidth/2, -flWidth/2));
		c.TexCoord(Vector(1.5f, -0.5f, 0));
		c.Vertex(Vector(-200, flWidth/2, -flWidth/2));
		c.TexCoord(Vector(1.5f, 1.5f, 0));
		c.Vertex(Vector(-200, flWidth/2, flWidth/2));
		c.TexCoord(Vector(-0.5f, 1.5f, 0));
		c.Vertex(Vector(-200, -flWidth/2, flWidth/2));
		c.EndRender();

		c.SetUniform("flAlpha", m_flLayer3Alpha);
		c.SetUniform("flSpeed", m_flLayer3Speed);
		c.SetUniform("vecColor", Color(55, 255, 55, 155));

		c.BeginRenderTriFan();
		c.TexCoord(Vector(-2.5f, -2.5f, 0));
		c.Vertex(Vector(-300, -flWidth/2, -flWidth/2));
		c.TexCoord(Vector(1, -2.5f, 0));
		c.Vertex(Vector(-300, flWidth/2, -flWidth/2));
		c.TexCoord(Vector(1, 1, 0));
		c.Vertex(Vector(-300, flWidth/2, flWidth/2));
		c.TexCoord(Vector(-2.5f, 1, 0));
		c.Vertex(Vector(-300, -flWidth/2, flWidth/2));
		c.EndRender();

		c.SetUniform("flAlpha", m_flLayer4Alpha);
		c.SetUniform("flSpeed", m_flLayer4Speed);
		c.SetUniform("vecColor", Color(100, 255, 100, 205));

		c.BeginRenderTriFan();
		c.TexCoord(Vector(0.4f, 0.5f, 0));
		c.Vertex(Vector(-400, -flWidth/2, -flWidth/2));
		c.TexCoord(Vector(2.4f, 0.5f, 0));
		c.Vertex(Vector(-400, flWidth/2, -flWidth/2));
		c.TexCoord(Vector(2.4f, 2.5f, 0));
		c.Vertex(Vector(-400, flWidth/2, flWidth/2));
		c.TexCoord(Vector(0.4f, 2.5f, 0));
		c.Vertex(Vector(-400, -flWidth/2, flWidth/2));
		c.EndRender();

		c.SetUniform("flAlpha", m_flLayer5Alpha);
		c.SetUniform("flSpeed", m_flLayer5Speed);
		c.SetUniform("vecColor", Color(55, 255, 55, 120));

		c.BeginRenderTriFan();
		c.TexCoord(Vector(-.1f, 0.2f, 0));
		c.Vertex(Vector(-500, -flWidth/2, -flWidth/2));
		c.TexCoord(Vector(0.9f, 0.2f, 0));
		c.Vertex(Vector(-500, flWidth/2, -flWidth/2));
		c.TexCoord(Vector(0.9f, 1.2f, 0));
		c.Vertex(Vector(-500, flWidth/2, flWidth/2));
		c.TexCoord(Vector(-.1f, 1.2f, 0));
		c.Vertex(Vector(-500, -flWidth/2, flWidth/2));
		c.EndRender();
	}
}
