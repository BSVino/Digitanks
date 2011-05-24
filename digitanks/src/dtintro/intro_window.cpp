#include "intro_window.h"

#include <time.h>

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <mtrand.h>

#include <tinker/keys.h>
#include <game/gameserver.h>
#include <glgui/glgui.h>
#include <renderer/renderer.h>

#include "screen.h"

void CIntroWindow::SetupIntro()
{
	mtsrand((size_t)time(NULL));

	GameServer()->Initialize();

	glgui::CRootPanel::Get()->Layout();

	CScreen* pScreen = GameServer()->Create<CScreen>("CScreen");
	pScreen->SetScreenshot(CRenderer::LoadTextureIntoGL(m_iScreenshot));
	pScreen->SetOrigin(Vector(-100, 0, 0));

	GameServer()->SetLoading(false);
}

void CIntroWindow::RenderLoading()
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	ilBindImage(m_iScreenshot);

	glWindowPos2i(0, 0);

	int iWidth = ilGetInteger(IL_IMAGE_WIDTH);
	int iHeight = ilGetInteger(IL_IMAGE_HEIGHT);
	unsigned char* pData = (unsigned char*)ilGetData();
	glDrawPixels(iWidth, iHeight, GL_RGB, GL_UNSIGNED_BYTE, pData);

	ilBindImage(0);

	SwapBuffers();
}

void CIntroWindow::DoKeyPress(int c)
{
	BaseClass::DoKeyPress(c);

	if (c == TINKER_KEY_ESCAPE)
		exit(0);
}
