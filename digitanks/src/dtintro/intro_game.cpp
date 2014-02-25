#include "intro_game.h"

#include <tinker/application.h>
#include <tinker/cvar.h>

#include <renderer/renderer.h>
#include <game/level.h>
#include <tengine/ui/hudviewport.h>

#include "intro_camera.h"
#include "intro_renderer.h"
#include "script.h"
#include "intro_window.h"
#include "screen.h"
#include "bug.h"
#include "digitank.h"
#include "script.h"
#include "general_window.h"
#include "general.h"

CGame* CreateGame()
{
	return GameServer()->Create<CIntroGame>("CIntroGame");
}

CResource<CLevel> CreateLevel()
{
	return CResource<CLevel>(new CLevel());
}

CHUDViewport* CreateHUD()
{
	CHUDViewport* pHUD = new CHUDViewport();
	return pHUD;
}

tstring GetInitialGameMode()
{
	return "intro";
}

pfnConditionsMet Game_GetInstructorConditions(const tstring& sConditions)
{
	return false;
}

REGISTER_ENTITY(CIntroGame);

NETVAR_TABLE_BEGIN(CIntroGame);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CIntroGame);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CIntroGame);
INPUTS_TABLE_END();

void CIntroGame::Precache()
{
	PrecacheParticleSystem("intro-explosion");
}

void CIntroGame::SetupGame(tstring sType)
{
	SetupIntro();
}

void SetupIntro(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
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

	static_cast<CIntroGame*>(GameServer()->GetGame())->SetupIntro();
}

CCommand intro_setup("intro_setup", ::SetupIntro);

void CIntroGame::SetupIntro()
{
	Application()->SetMouseCursorEnabled(true);

	float flWidth = (float)CApplication::Get()->GetWindowWidth();
	float flHeight = (float)CApplication::Get()->GetWindowHeight();

	static_cast<CIntroWindow*>(Application())->GetGeneralWindow()->Reset();

	CScreen* pScreen = GameServer()->Create<CScreen>("CScreen");
	pScreen->SetName("screen");
	pScreen->SetScreenshot(static_cast<CIntroWindow*>(Application())->GetScreenshot());
	pScreen->SetGlobalOrigin(Vector(0, 0, 0));

	GameServer()->Create<CIntroCamera>("CIntroCamera");

	CTeam* pBugsTeam = GameServer()->Create<CTeam>("CTeam");
	pBugsTeam->SetColor(Color(255, 0, 0, 255));

	CBug* pBug;

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetName("bug1");
	pBug->SetGlobalOrigin(Vector(-50, flWidth*0.16f, -20));
	pBug->SetGlobalAngles(EAngle(-10, -10, 0));
	pBug->FaceTurret(-40);
	pBugsTeam->AddEntity(pBug);

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetName("bug2");
	pBug->SetGlobalOrigin(Vector(-50, flWidth*0.32f, 60));
	pBug->SetGlobalAngles(EAngle(-10, 20, 0));
	pBug->FaceTurret(40);
	pBugsTeam->AddEntity(pBug);

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetName("bug3");
	pBug->SetGlobalOrigin(Vector(-50, flWidth*0.16f, flHeight*0.7f));
	pBug->SetGlobalAngles(EAngle(-10, -10, 0));
	pBug->FaceTurret(-40);
	pBugsTeam->AddEntity(pBug);

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetName("bug4");
	pBug->SetGlobalOrigin(Vector(-50, flWidth*0.7f, 0));
	pBug->SetGlobalAngles(EAngle(-10, 20, 0));
	pBug->FaceTurret(40);
	pBugsTeam->AddEntity(pBug);

	pBug = GameServer()->Create<CBug>("CBug");
	pBug->SetName("bug5");
	pBug->SetGlobalOrigin(Vector(-50, flWidth*0.16f, -flHeight*0.7f));
	pBug->SetGlobalAngles(EAngle(-10, -10, 0));
	pBug->FaceTurret(-40);
	pBugsTeam->AddEntity(pBug);

	CIntroGeneral* pGeneral = GameServer()->Create<CIntroGeneral>("CIntroGeneral");
	pGeneral->SetName("general");

	CTeam* pDigitanksTeam = GameServer()->Create<CTeam>("CTeam");
	pDigitanksTeam->SetColor(Color(0, 64, 255, 255));

	CDigitank* pDigitank;

	pDigitank = GameServer()->Create<CDigitank>("CDigitank");
	pDigitank->SetName("digitank1");
	pDigitank->SetGlobalOrigin(Vector(-50, -flWidth*0.7f, -flHeight*0.7f));
	pDigitank->SetGlobalAngles(EAngle(-10, -70, 0));
	pDigitank->FaceTurret(0);
	pDigitanksTeam->AddEntity(pDigitank);

	pDigitank = GameServer()->Create<CDigitank>("CDigitank");
	pDigitank->SetName("digitank2");
	pDigitank->SetGlobalOrigin(Vector(-50, -flWidth*0.7f, 60));
	pDigitank->SetGlobalAngles(EAngle(-10, -70, 0));
	pDigitank->FaceTurret(0);
	pDigitanksTeam->AddEntity(pDigitank);

	pDigitank = GameServer()->Create<CDigitank>("CDigitank");
	pDigitank->SetName("digitank3");
	pDigitank->SetGlobalOrigin(Vector(-50, flWidth*0.7f, flHeight*0.7f));
	pDigitank->SetGlobalAngles(EAngle(-10, -70, 0));
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
	pEvent->m_vecOrigin = Vector(50, flWidth*0.25f, 20);

	pEvent = pBustOutScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 0.5f;
	pEvent->m_flEndTime = 2.5f;
	pEvent->m_sName = "bug1";
	pEvent->m_vecOrigin = Vector(250, flWidth*0.35f, 60);
	pEvent->m_angAngles = EAngle(-10, 20, 0);

	pEvent = pBustOutScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 1.5f;
	pEvent->m_flEndTime = 3.5f;
	pEvent->m_sName = "bug2";
	pEvent->m_vecOrigin = Vector(250, flWidth*0.14f, -20);
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
	pEvent->m_vecOrigin = Vector(250, flWidth*0.16f, flHeight*0.28f);
	pEvent->m_angAngles = EAngle(-10, 20, 0);

	pEvent = pGeneralDebug2Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 4.0f;
	pEvent->m_flEndTime = 6.0f;
	pEvent->m_sName = "bug4";
	pEvent->m_vecOrigin = Vector(250, flWidth*0.35f, -flHeight*0.1f);
	pEvent->m_angAngles = EAngle(-10, 20, 0);

	pEvent = pGeneralDebug2Script->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 4.5f;
	pEvent->m_flEndTime = 6.5f;
	pEvent->m_sName = "bug5";
	pEvent->m_vecOrigin = Vector(250, flWidth*0.16f, -flHeight*0.28f);
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
	pEvent->m_vecOrigin = Vector(250, -flWidth*0.30f, flHeight*0.25f);
	pEvent->m_angAngles = EAngle(-20, -40, 0);

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 0.2f;
	pEvent->m_flEndTime = 1.2f;
	pEvent->m_sName = "digitank3";
	pEvent->m_vecOrigin = Vector(250, -flWidth*0.35f, flHeight*0.35f);
	pEvent->m_angAngles = EAngle(-20, -40, 0);

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_MOVEACTOR;
	pEvent->m_flStartTime = 0.4f;
	pEvent->m_flEndTime = 1.4f;
	pEvent->m_sName = "digitank1";
	pEvent->m_vecOrigin = Vector(250, -flWidth*0.35f, flHeight*0.1f);
	pEvent->m_angAngles = EAngle(-20, -40, 0);

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 2;
	pEvent->m_sName = "digitank1";
	pEvent->m_sOutput = "FireAt";
	pEvent->m_sArgs = "bug1";

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 2.3f;
	pEvent->m_sName = "digitank2";
	pEvent->m_sOutput = "FireAt";
	pEvent->m_sArgs = "bug2";

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 2.6f;
	pEvent->m_sName = "digitank3";
	pEvent->m_sOutput = "FireAt";
	pEvent->m_sArgs = "bug3";

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 3;
	pEvent->m_sName = "digitank1";
	pEvent->m_sOutput = "FireAt";
	pEvent->m_sArgs = "bug4";

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 3.3f;
	pEvent->m_sName = "digitank2";
	pEvent->m_sOutput = "FireAt";
	pEvent->m_sArgs = "bug5";

	pEvent = pDigitanksScript->AddScriptEvent();
	pEvent->m_eEventClass = EC_FIREOUTPUT;
	pEvent->m_flStartTime = 6;
	pEvent->m_sName = "general";
	pEvent->m_sOutput = "DigitanksWon";

	ScriptManager()->PlayScript("bustout");
}

void CIntroGame::Think()
{
	BaseClass::Think();

	ScriptManager()->Think();
}

