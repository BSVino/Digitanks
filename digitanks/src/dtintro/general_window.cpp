#include "general_window.h"

#include <game/gameserver.h>
#include <renderer/renderer.h>

#include "script.h"
#include "intro_window.h"
#include "intro_renderer.h"

CGeneralWindow::CGeneralWindow()
	: CPanel(0, 0, 400, 400), m_hGeneral(L"textures/hud/helper-emotions.txt"), m_hGeneralMouth(L"textures/hud/helper-emotions-open.txt")
{
	glgui::CRootPanel::Get()->AddControl(this);

	m_flDeployed = m_flDeployedGoal = 0;

	m_pText = new glgui::CLabel(100, 0, 300, 300, L"");
	AddControl(m_pText);

	m_pButton = new glgui::CButton(0, 0, 100, 30, L"");
	AddControl(m_pButton);
	m_pButton->SetClickedListener(this, ButtonPressed);
	m_pButton->SetButtonColor(Color(255, 0, 0, 255));

	m_flFadeToBlack = 0;
	m_flStartTime = 0;
}

void CGeneralWindow::Layout()
{
	SetSize(400, 300);
	SetPos(100, glgui::CRootPanel::Get()->GetHeight() - (int)(m_flDeployed*GetHeight()));

	m_pText->SetPos(160, 10);
	m_pText->SetSize(230, 230);

	m_pButton->SetPos(160+230/2-m_pButton->GetWidth()/2, GetHeight()-50);
}

void CGeneralWindow::Think()
{
	BaseClass::Think();

	m_flDeployed = Approach(m_flDeployedGoal, m_flDeployed, GameServer()->GetFrameTime());

	SetPos(100, glgui::CRootPanel::Get()->GetHeight() - (int)(m_flDeployed*GetHeight()));

	if (m_flStartTime)
	{
		int iPrintChars = (int)((GameServer()->GetGameTime() - m_flStartTime)*50);
		m_pText->SetPrintChars(iPrintChars);
	}
}

void CGeneralWindow::Paint(int x, int y, int w, int h)
{
	if (m_flFadeToBlack)
	{
		float flAlpha = RemapVal(GameServer()->GetGameTime(), m_flFadeToBlack, m_flFadeToBlack+1.5f, 0, 1);
		glgui::CRootPanel::PaintRect(0, 0, glgui::CRootPanel::Get()->GetWidth(), glgui::CRootPanel::Get()->GetHeight(), Color(0, 0, 0, (int)(255*flAlpha)));
		return;
	}

	glgui::CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, 220));

	BaseClass::Paint(x, y, w, h);

	CRenderingContext c(GameServer()->GetRenderer());
	c.SetBlend(BLEND_ALPHA);
	c.SetColor(Color(255, 255, 255, 255));
	c.UseProgram(0);
	c.BindTexture(0);

	Rect recEmotion = m_hGeneral.GetArea(m_sEmotion);
	glgui::CBaseControl::PaintSheet(m_hGeneral.GetSheet(m_sEmotion), x, y, 150, 300, recEmotion.x, recEmotion.y, recEmotion.w, recEmotion.h, m_hGeneral.GetSheetWidth(m_sEmotion), m_hGeneral.GetSheetHeight(m_sEmotion));
}

void CGeneralWindow::Reset()
{
	m_flDeployed = m_flDeployedGoal = 0;

	m_eStage = STAGE_REPAIR;

	m_pButton->SetVisible(true);
}

void CGeneralWindow::Deploy()
{
	Layout();

	m_flDeployedGoal = 1;

	m_pText->SetText(L"THE GENERAL'S\nANTI-BUG UTILITY\n \nYou are on day 8479\nof your 30 day trial.\n \nI've detected the presence of Bugs in your computer. Would you like me to attempt to repair them for you?");
	m_pButton->SetText(L"Repair");
	m_pButton->SetVisible(true);

	m_sEmotion = "Pleased";

	m_flStartTime = GameServer()->GetGameTime();
}

void CGeneralWindow::RetryDebugging()
{
	Layout();

	m_pText->SetText(L"THE GENERAL HAS FAILED DE-BUGGING\n \nI've sensed a drop in your satisfaction level with this product. Please allow me to regain your confidence with another de-bugging attempt.");
	m_pButton->SetText(L"Retry");
	m_pButton->SetVisible(true);

	m_sEmotion = "Disappointed";

	m_flStartTime = GameServer()->GetGameTime();
}

void CGeneralWindow::GiveUpDebugging()
{
	Layout();

	m_pText->SetText(L"WARNING: ADDITIONAL BUGS FOUND\n \nArgh! They have reinforcements! I can't do anything right.\n \nBut don't look so smug, I don't see you doing any better.");
	m_pButton->SetText(L"Do Better");
	m_pButton->SetVisible(true);

	m_sEmotion = "Surprised";

	m_flStartTime = GameServer()->GetGameTime();
}

void CGeneralWindow::DigitanksWon()
{
	Layout();

	m_pText->SetText(L"Hot bolts, what a brilliant tactic! Digitanks are a computer's natural defense system against Bugs. You're showing the talent of a true Hacker.\n \nLet's see if we can get any intel.");
	m_pButton->SetText(L"Roger");
	m_pButton->SetVisible(true);

	m_sEmotion = "Cheering";

	m_flStartTime = GameServer()->GetGameTime();
}

void CGeneralWindow::ButtonPressedCallback()
{
	if (m_eStage == STAGE_REPAIR)
	{
		ScriptManager()->PlayScript("general-debug-1");

		m_pText->SetText("Please wait...");
		m_pButton->SetVisible(false);

		m_sEmotion = "KillingBugs";

		m_eStage = STAGE_REPAIR2;

		m_flStartTime = GameServer()->GetGameTime();
	}
	else if (m_eStage == STAGE_REPAIR2)
	{
		ScriptManager()->PlayScript("general-debug-2");

		m_pText->SetText("Please wait...");
		m_pButton->SetVisible(false);

		m_sEmotion = "KillingBugsHard";

		m_eStage = STAGE_DIGITANKS;

		m_flStartTime = GameServer()->GetGameTime();
	}
	else if (m_eStage == STAGE_DIGITANKS)
	{
		ScriptManager()->PlayScript("digitanks");

		m_pText->SetText("Running program 'digitanks.exe' ...");
		m_pButton->SetVisible(false);

		m_eStage = STAGE_INTEL;

		m_flStartTime = GameServer()->GetGameTime();
	}
	else if (m_eStage == STAGE_INTEL)
	{
		m_pText->SetText(L"Oh no... it looks like the Bugs have captured some of your files!\n \nWe need to get to your hard drive to rescue your files. Let's go!");
		m_pButton->SetText(L"Affirmative");
		m_pButton->SetVisible(true);

		m_sEmotion = "OhNo";

		m_eStage = STAGE_FINISH;

		m_flStartTime = GameServer()->GetGameTime();
	}
	else
	{
		IntroWindow()->GetRenderer()->ZoomIntoHole();
		m_flFadeToBlack = GameServer()->GetGameTime();
	}
}
