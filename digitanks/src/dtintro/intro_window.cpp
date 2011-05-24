#include "intro_window.h"

#include <time.h>

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include <mtrand.h>

#include <tinker/keys.h>
#include <game/gameserver.h>
#include <game/game.h>
#include <glgui/glgui.h>
#include <renderer/renderer.h>
#include <tinker/cvar.h>

#include "screen.h"
#include "bug.h"

void CIntroWindow::SetupEngine()
{
	mtsrand((size_t)time(NULL));

	GameServer()->Initialize();

	glgui::CRootPanel::Get()->Layout();

	SetupIntro();

	GameServer()->SetLoading(false);
}

void SetupIntro(class CCommand* pCommand, eastl::vector<eastl::string16>& asTokens, const eastl::string16& sCommand)
{
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		if (pEntity == Game())
			continue;

		pEntity->Delete();
	}

	static_cast<CIntroWindow*>(CApplication::Get())->SetupIntro();
}

CCommand intro_setup("intro_setup", ::SetupIntro);

void CIntroWindow::SetupIntro()
{
	float flWidth = (float)CApplication::Get()->GetWindowWidth();
	float flHeight = (float)CApplication::Get()->GetWindowHeight();

	CScreen* pScreen = GameServer()->Create<CScreen>("CScreen");
	pScreen->SetScreenshot(CRenderer::LoadTextureIntoGL(m_iScreenshot));
	pScreen->SetOrigin(Vector(-500, 0, 0));

	CBug* pBug;

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetOrigin(Vector(-500, -20, -flWidth/4));
	pBug->SetAngles(EAngle(-10, -10, 0));
	pBug->FaceTurret(-40);

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetOrigin(Vector(-500, 60, -flWidth*0.23f));
	pBug->SetAngles(EAngle(-10, 20, 0));
	pBug->FaceTurret(40);
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
