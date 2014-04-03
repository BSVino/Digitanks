#include "intro_window.h"

#include <time.h>

#include <mtrand.h>
#include <tinker_platform.h>

#include <tinker/keys.h>
#include <game/gameserver.h>
#include <game/entities/game.h>
#include <glgui/rootpanel.h>
#include <renderer/renderer.h>
#include <renderer/renderingcontext.h>
#include <tinker/cvar.h>

#include "intro_renderer.h"
#include "general_window.h"

CIntroWindow::CIntroWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
	m_pGeneralWindow = NULL;

	m_iScreenshot = 0;
}

void CIntroWindow::GetWindowOpenSize(int& iWindowWidth, int& iWindowHeight)
{
	GetScreenSize(iWindowWidth, iWindowHeight);
}

void CIntroWindow::SetupEngine()
{
	mtsrand((size_t)time(NULL));

	m_pGeneralWindow = new CGeneralWindow();
	glgui::CRootPanel::Get()->Layout();

	CVar::SetCVar("r_frustumculling", false);
}

void CIntroWindow::RenderLoading()
{
	if (!m_iScreenshot)
	{
		m_iScreenshot = CRenderer::LoadTextureIntoGL(m_aclrScreenshot.data(), m_iScreenshotWidth, m_iScreenshotHeight);
		m_aclrScreenshot.clear();
	}

#ifdef _WIN32
	// Windows gives the screenshot data back as BGR.
	// Instead of switching the red and blue channels in the screenshot data,
	// I'll just load a different shader.
	CRenderingContext c(GameServer()->GetRenderer(), true);
	c.UseProgram("quad_rbswap");
	c.SetUniform("vecColor", Vector4D(1, 1, 1, 1));
#endif

	GameServer()->GetRenderer()->RenderMapFullscreen(m_iScreenshot);

	SwapBuffers();
}

bool CIntroWindow::DoKeyPress(int c)
{
	if (c == TINKER_KEY_ESCAPE)
		exit(0);

	return BaseClass::DoKeyPress(c);
}

CIntroRenderer* CIntroWindow::GetRenderer()
{
	return static_cast<CIntroRenderer*>(GameServer()->GetRenderer());
}

CRenderer* CIntroWindow::CreateRenderer()
{
	return new CIntroRenderer();
}
