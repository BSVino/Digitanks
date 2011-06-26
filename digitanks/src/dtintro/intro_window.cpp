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
#include "digitank.h"
#include "script.h"
#include "general_window.h"
#include "general.h"
#include "intro_renderer.h"

CIntroWindow::CIntroWindow(int argc, char** argv)
	: CGameWindow(argc, argv)
{
	m_pGeneralWindow = NULL;
}

void CIntroWindow::SetupEngine()
{
	mtsrand((size_t)time(NULL));

	GameServer()->Initialize();

	glgui::CRootPanel::Get()->SetLighting(false);
	m_pGeneralWindow = new CGeneralWindow();
	glgui::CRootPanel::Get()->Layout();

	SetupIntro();

	GameServer()->SetLoading(false);

	CVar::SetCVar("r_frustumculling", false);
}

void SetupIntro(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
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

	GetGeneralWindow()->Reset();

	CScreen* pScreen = GameServer()->Create<CScreen>("CScreen");
	pScreen->SetName("screen");
	pScreen->SetScreenshot(CRenderer::LoadTextureIntoGL(m_iScreenshot));
	pScreen->SetOrigin(Vector(0, 0, 0));

	CTeam* pBugsTeam = GameServer()->Create<CTeam>("CTeam");
	pBugsTeam->SetColor(Color(255, 0, 0, 255));

	CBug* pBug;

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetName("bug1");
	pBug->SetOrigin(Vector(-50, -20, -flWidth*0.16f));
	pBug->SetAngles(EAngle(-10, -10, 0));
	pBug->FaceTurret(-40);
	pBugsTeam->AddEntity(pBug);

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetName("bug2");
	pBug->SetOrigin(Vector(-50, 60, -flWidth*0.32f));
	pBug->SetAngles(EAngle(-10, 20, 0));
	pBug->FaceTurret(40);
	pBugsTeam->AddEntity(pBug);

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetName("bug3");
	pBug->SetOrigin(Vector(-50, flHeight*0.7f, -flWidth*0.16f));
	pBug->SetAngles(EAngle(-10, -10, 0));
	pBug->FaceTurret(-40);
	pBugsTeam->AddEntity(pBug);

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetName("bug4");
	pBug->SetOrigin(Vector(-50, 0, -flWidth*0.7f));
	pBug->SetAngles(EAngle(-10, 20, 0));
	pBug->FaceTurret(40);
	pBugsTeam->AddEntity(pBug);

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetName("bug5");
	pBug->SetOrigin(Vector(-50, -flHeight*0.7f, -flWidth*0.16f));
	pBug->SetAngles(EAngle(-10, -10, 0));
	pBug->FaceTurret(-40);
	pBugsTeam->AddEntity(pBug);

	CIntroGeneral* pGeneral = GameServer()->Create<CIntroGeneral>("CIntroGeneral");
	pGeneral->SetName("general");

	CTeam* pDigitanksTeam = GameServer()->Create<CTeam>("CTeam");
	pDigitanksTeam->SetColor(Color(0, 64, 255, 255));

	CDigitank* pDigitank;

	pDigitank = GameServer()->Create<CDigitank>("CDigitank");
	pDigitank->SetName("digitank1");
	pDigitank->SetOrigin(Vector(-50, -flHeight*0.7f, flWidth*0.7f));
	pDigitank->SetAngles(EAngle(-10, -70, 0));
	pDigitank->FaceTurret(0);
	pDigitanksTeam->AddEntity(pDigitank);

	pDigitank = GameServer()->Create<CDigitank>("CDigitank");
	pDigitank->SetName("digitank2");
	pDigitank->SetOrigin(Vector(-50, 60, flWidth*0.7f));
	pDigitank->SetAngles(EAngle(-10, -70, 0));
	pDigitank->FaceTurret(0);
	pDigitanksTeam->AddEntity(pDigitank);

	pDigitank = GameServer()->Create<CDigitank>("CDigitank");
	pDigitank->SetName("digitank3");
	pDigitank->SetOrigin(Vector(-50, flHeight*0.7f, flWidth*0.7f));
	pDigitank->SetAngles(EAngle(-10, -70, 0));
	pDigitank->FaceTurret(0);
	pDigitanksTeam->AddEntity(pDigitank);

	ScriptManager()->ClearScripts();

	CScriptEvent* pEvent;

	CScript* pBustOutScript = ScriptManager()->AddScript("bustout");

	pEvent = pBustOutScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_PARTICLES;
	pEvent->m_flStartTime = 0;
	pEvent->m_sName = "intro-explosion";
	pEvent->m_vecOrigin = Vector(0, 0, 0);

	pEvent = pBustOutScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_PARTICLES;
	pEvent->m_flStartTime = 0.5;
	pEvent->m_sName = "intro-explosion-fragments";
	pEvent->m_vecOrigin = Vector(50, 20, -flWidth*0.25f);

	pEvent = pBustOutScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 0.5f;
	pEvent->m_flEndTime = 2.5f;
	pEvent->m_sName = "bug1";
	pEvent->m_vecOrigin = Vector(250, 60, -flWidth*0.35f);
	pEvent->m_angAngles = EAngle(-10, 20, 0);

	pEvent = pBustOutScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 1.5f;
	pEvent->m_flEndTime = 3.5f;
	pEvent->m_sName = "bug2";
	pEvent->m_vecOrigin = Vector(250, -20, -flWidth*0.14f);
	pEvent->m_angAngles = EAngle(-10, 20, 0);

	pEvent = pBustOutScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 3.5;
	pEvent->m_sName = "bug1";
	pEvent->m_sOutput = "FireRandomly";

	pEvent = pBustOutScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 4.5f;
	pEvent->m_sName = "bug2";
	pEvent->m_sOutput = "FireRandomly";

	pEvent = pBustOutScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 8.5f;
	pEvent->m_sName = "general";
	pEvent->m_sOutput = "Deploy";

	CScript* pGeneralDebug1Script = ScriptManager()->AddScript("general-debug-1");

	pEvent = pGeneralDebug1Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_PARTICLES;
	pEvent->m_flStartTime = 3;
	pEvent->m_sName = "intro-explosion";
	pEvent->m_vecOrigin = Vector(0, 0, 0);

	pEvent = pGeneralDebug1Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 6;
	pEvent->m_sName = "general";
	pEvent->m_sOutput = "RetryDebugging";

	CScript* pGeneralDebug2Script = ScriptManager()->AddScript("general-debug-2");

	pEvent = pGeneralDebug2Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_PARTICLES;
	pEvent->m_flStartTime = 3;
	pEvent->m_sName = "intro-explosion";
	pEvent->m_vecOrigin = Vector(0, 0, 0);

	pEvent = pGeneralDebug2Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 3.5f;
	pEvent->m_flEndTime = 5.5f;
	pEvent->m_sName = "bug3";
	pEvent->m_vecOrigin = Vector(250, flHeight*0.28f, -flWidth*0.16f);
	pEvent->m_angAngles = EAngle(-10, 20, 0);

	pEvent = pGeneralDebug2Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 4.0f;
	pEvent->m_flEndTime = 6.0f;
	pEvent->m_sName = "bug4";
	pEvent->m_vecOrigin = Vector(250, -flHeight*0.1f, -flWidth*0.35f);
	pEvent->m_angAngles = EAngle(-10, 20, 0);

	pEvent = pGeneralDebug2Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 4.5f;
	pEvent->m_flEndTime = 6.5f;
	pEvent->m_sName = "bug5";
	pEvent->m_vecOrigin = Vector(250, -flHeight*0.28f, -flWidth*0.16f);
	pEvent->m_angAngles = EAngle(-10, 20, 0);

	pEvent = pGeneralDebug2Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 4.0;
	pEvent->m_sName = "bug3";
	pEvent->m_sOutput = "FireRandomly";

	pEvent = pGeneralDebug2Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 4.5f;
	pEvent->m_sName = "bug4";
	pEvent->m_sOutput = "FireRandomly";

	pEvent = pGeneralDebug2Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 5.0f;
	pEvent->m_sName = "bug5";
	pEvent->m_sOutput = "FireRandomly";

	pEvent = pGeneralDebug2Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 6;
	pEvent->m_sName = "general";
	pEvent->m_sOutput = "GiveUpDebugging";

	CScript* pDigitanksScript = ScriptManager()->AddScript("digitanks");

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 0.0f;
	pEvent->m_flEndTime = 1.0f;
	pEvent->m_sName = "digitank2";
	pEvent->m_vecOrigin = Vector(250, flHeight*0.25f, flWidth*0.30f);
	pEvent->m_angAngles = EAngle(-20, -40, 0);

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 0.2f;
	pEvent->m_flEndTime = 1.2f;
	pEvent->m_sName = "digitank3";
	pEvent->m_vecOrigin = Vector(250, flHeight*0.35f, flWidth*0.35f);
	pEvent->m_angAngles = EAngle(-20, -40, 0);

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 0.4f;
	pEvent->m_flEndTime = 1.4f;
	pEvent->m_sName = "digitank1";
	pEvent->m_vecOrigin = Vector(250, flHeight*0.1f, flWidth*0.35f);
	pEvent->m_angAngles = EAngle(-20, -40, 0);

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 2;
	pEvent->m_sName = "digitank1";
	pEvent->m_sOutput = "FireAt";
	pEvent->m_sArgs = _T("bug1");

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 2.3f;
	pEvent->m_sName = "digitank2";
	pEvent->m_sOutput = "FireAt";
	pEvent->m_sArgs = _T("bug2");

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 2.6f;
	pEvent->m_sName = "digitank3";
	pEvent->m_sOutput = "FireAt";
	pEvent->m_sArgs = _T("bug3");

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 3;
	pEvent->m_sName = "digitank1";
	pEvent->m_sOutput = "FireAt";
	pEvent->m_sArgs = _T("bug4");

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 3.3f;
	pEvent->m_sName = "digitank2";
	pEvent->m_sOutput = "FireAt";
	pEvent->m_sArgs = _T("bug5");

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 6;
	pEvent->m_sName = "general";
	pEvent->m_sOutput = "DigitanksWon";

	ScriptManager()->PlayScript("bustout");
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

CIntroRenderer* CIntroWindow::GetRenderer()
{
	return static_cast<CIntroRenderer*>(GameServer()->GetRenderer());
}
