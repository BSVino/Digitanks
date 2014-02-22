#include "general_window.h"

#include <mtrand.h>
#include <strutils.h>

#include <glgui/rootpanel.h>
#include <glgui/label.h>
#include <glgui/button.h>

#include <game/gameserver.h>
#include <renderer/renderer.h>
#include <renderer/game_renderingcontext.h>
#include <sound/sound.h>
#include <game/entities/game.h>

#include "script.h"
#include "intro_window.h"
#include "intro_renderer.h"

CGeneralWindow::CGeneralWindow()
	: CPanel(0, 0, 400, 400), m_hAntivirus("textures/intro/antivirus.txt"), m_hGeneral("textures/hud/helper-emotions.txt"), m_hGeneralMouth("textures/hud/helper-emotions-open.txt")
{
	glgui::CRootPanel::Get()->AddControl(this);

	m_flDeployed = m_flDeployedGoal = 0;

	m_pText = AddControl(new glgui::CLabel(100, 0, 300, 300, ""));

	m_pButton = AddControl(new glgui::CButton(0, 0, 100, 30, ""));
	m_pButton->SetClickedListener(this, ButtonPressed);
	m_pButton->SetButtonColor(Color(255, 0, 0, 255));

	m_flFadeToBlack = 0;
	m_flStartTime = 0;
	m_bHelperSpeaking = false;
	m_bProgressBar = false;

	CSoundLibrary::SetSoundVolume(0.6f);
}

void CGeneralWindow::Layout()
{
	SetSize(512, 256);
	SetPos(100, glgui::CRootPanel::Get()->GetHeight() - (int)(m_flDeployed*GetHeight()));

	m_pText->SetPos(230, 30);
	m_pText->SetSize(260, 160);
	m_pText->SetFont("sans-serif", 14);
	m_pText->SetTextColor(Color(0, 0, 0, 255));

	m_pButton->SetPos(230+260/2-m_pButton->GetWidth()/2, GetHeight()-50);
}

void CGeneralWindow::Think()
{
	BaseClass::Think();

	m_flDeployed = Approach(m_flDeployedGoal, m_flDeployed, (float)GameServer()->GetFrameTime());

	SetPos(100, glgui::CRootPanel::Get()->GetHeight() - (int)(m_flDeployed*GetHeight()*1.5f));

	int iPrintChars = (int)((GameServer()->GetGameTime() - m_flStartTime)*50);
	if (m_flStartTime)
		m_pText->SetPrintChars(iPrintChars);

	bool bScrolling = (iPrintChars < (int)m_pText->GetText().length());

	if (bScrolling)
		m_pButton->SetButtonColor(Color(255, 0, 0, 255));
	else
		m_pButton->SetButtonColor(Color(255, 0, 0, 255)*Oscillate((float)GameServer()->GetGameTime(), 1));

	if (bScrolling)
	{
		if (!m_bHelperSpeaking)
		{
			CSoundLibrary::PlaySound(NULL, "sound/helper-speech.wav", true);
			m_bHelperSpeaking = true;
			CSoundLibrary::SetSoundVolume(NULL, "sound/helper-speech.wav", 0.7f);
		}
	}
	else
	{
		if (m_bHelperSpeaking)
		{
			CSoundLibrary::StopSound(NULL, "sound/helper-speech.wav");
			m_bHelperSpeaking = false;
		}
	}
}

void CGeneralWindow::Paint(float x, float y, float w, float h)
{
	if (m_flFadeToBlack)
	{
		float flAlpha = (float)RemapVal(GameServer()->GetGameTime(), m_flFadeToBlack, m_flFadeToBlack+1.5f, 0.0, 1.0);
		glgui::CRootPanel::PaintRect(0, 0, glgui::CRootPanel::Get()->GetWidth(), glgui::CRootPanel::Get()->GetHeight(), Color(0, 0, 0, (int)(255*flAlpha)));
		return;
	}

	Rect recAntivirus = m_hAntivirus.GetArea("Antivirus");
	glgui::CBaseControl::PaintSheet(m_hAntivirus.GetSheet("Antivirus"), x, y, w, h, recAntivirus.x, recAntivirus.y, recAntivirus.w, recAntivirus.h, m_hAntivirus.GetSheetWidth("Antivirus"), m_hAntivirus.GetSheetHeight("Antivirus"));

	BaseClass::Paint(x, y, w, h);

	if (!m_sEmotion.length())
		return;

	CGameRenderingContext c(GameServer()->GetRenderer(), true);
	c.SetBlend(BLEND_ALPHA);
	c.SetColor(Color(255, 255, 255, 255));

	Rect recEmotion = m_hGeneral.GetArea(m_sEmotion);
	glgui::CBaseControl::PaintSheet(m_hGeneral.GetSheet(m_sEmotion), x, y, 256, 256, recEmotion.x, recEmotion.y, recEmotion.w, recEmotion.h, m_hGeneral.GetSheetWidth(m_sEmotion), m_hGeneral.GetSheetHeight(m_sEmotion));

	if ((m_bHelperSpeaking || m_bProgressBar) && Oscillate((float)GameServer()->GetGameTime(), 0.2f) > 0.5)
	{
		Rect recMouth = m_hGeneralMouth.GetArea(m_sEmotion);
		glgui::CBaseControl::PaintSheet(m_hGeneralMouth.GetSheet(m_sEmotion), x, y, 256, 256, recMouth.x, recMouth.y, recMouth.w, recMouth.h, m_hGeneralMouth.GetSheetWidth(m_sEmotion), m_hGeneralMouth.GetSheetHeight(m_sEmotion));
	}

	if (m_bProgressBar)
	{
		double flTime = 3;
		glgui::CBaseControl::PaintRect(x + m_pText->GetLeft(), y + 160, m_pText->GetWidth(), 10, Color(255, 255, 255, 255));
		glgui::CBaseControl::PaintRect(x + m_pText->GetLeft() + 2, y + 160 + 2, ((m_pText->GetWidth() - 4) * (float)RemapValClamped(GameServer()->GetGameTime(), m_flStartTime, m_flStartTime+flTime, 0.0, 1.0)), 10 - 4, Color(42, 65, 122, 255));

		static tstring sEstimate;
		static double flLastEstimateUpdate = 0;

		if (!sEstimate.length() || GameServer()->GetGameTime() - flLastEstimateUpdate > 1)
		{
			int iRandomTime = RandomInt(0, 5);
			tstring sRandomTime;
			if (iRandomTime == 0)
				sRandomTime = "centuries";
			else if (iRandomTime == 1)
				sRandomTime = "minutes";
			else if (iRandomTime == 2)
				sRandomTime = "hours";
			else if (iRandomTime == 3)
				sRandomTime = "days";
			else
				sRandomTime = "seconds";

			sEstimate = sprintf(tstring("Estimated time remaining: %d %s"), RandomInt(2, 100), sRandomTime.c_str());

			flLastEstimateUpdate = GameServer()->GetGameTime();
		}

		float flWidth = glgui::CLabel::GetTextWidth(sEstimate, sEstimate.length(), "sans-serif", 12);
		glgui::CLabel::PaintText(sEstimate, sEstimate.length(), "sans-serif", 12, x + m_pText->GetLeft() + m_pText->GetWidth()/2 - flWidth/2, (float)y + 190, Color(0, 0, 0, 255));
	}
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

	m_pText->SetText("H4xx0r Att4xx0r\nl337 ANTI-BUG UTILITY\n \nYou are on day 8479\nof your 30 day trial.\n \nI've detected the presence of Bugs in your computer. Would you like me to attempt to remove them for you?");
	m_pButton->SetText("Remove");
	m_pButton->SetVisible(true);

	m_sEmotion = "PleasedIntro";

	m_flStartTime = GameServer()->GetGameTime();
}

void CGeneralWindow::RetryDebugging()
{
	Layout();

	m_pText->SetText("BUG REMOVAL HAS FAILED\n \nI've sensed a drop in your satisfaction level with this product. Please allow me to regain your confidence with another removal attempt.");
	m_pButton->SetText("Retry");
	m_pButton->SetVisible(true);

	m_sEmotion = "DisappointedIntro";
	m_bProgressBar = false;

	m_flStartTime = GameServer()->GetGameTime();
}

void CGeneralWindow::GiveUpDebugging()
{
	Layout();

	m_pText->SetText("WARNING: ADDITIONAL BUGS FOUND\n \nArgh! They have reinforcements! I can't seem to do anything right.\n \nDon't look so smug, I don't see you doing any better.");
	m_pButton->SetText("Do Better");
	m_pButton->SetVisible(true);

	m_sEmotion = "SurprisedIntro";
	m_bProgressBar = false;

	m_flStartTime = GameServer()->GetGameTime();
}

void CGeneralWindow::DigitanksWon()
{
	Layout();

	m_pText->SetText("Hot bolts, what a brilliant tactic! Digitanks are a computer's natural defense system against Bugs. You're showing the talent of a true Hacker.\n \nLet's see if we can get any intel.");
	m_pButton->SetText("Roger");
	m_pButton->SetVisible(true);

	m_sEmotion = "CheeringIntro";

	m_flStartTime = GameServer()->GetGameTime();
}

void CGeneralWindow::ButtonPressedCallback(const tstring& sArgs)
{
	if (m_eStage == STAGE_REPAIR)
	{
		ScriptManager()->PlayScript("general-debug-1");

		m_pText->SetText("Removing bugs. Please wait...");
		m_pButton->SetVisible(false);
		m_sEmotion = "KillingBugsIntro";
		m_eStage = STAGE_REPAIR2;
		m_flStartTime = GameServer()->GetGameTime();
		m_bProgressBar = true;
	}
	else if (m_eStage == STAGE_REPAIR2)
	{
		ScriptManager()->PlayScript("general-debug-2");

		m_pText->SetText("Removing bugs. Please wait...");
		m_pButton->SetVisible(false);
		m_sEmotion = "KillingBugsHardIntro";
		m_eStage = STAGE_DIGITANKS;
		m_flStartTime = GameServer()->GetGameTime();
		m_bProgressBar = true;
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
		m_pText->SetText("Oh no... it looks like the Bugs have captured some of your files!\n \nWe need to get to your hard drive to rescue your files --\n \nWhat's that? No! Not \"those\" files. I don't want to see what's in \"those\" files. We can rescue your other files!");
		m_pButton->SetText("Let's go!");
		m_pButton->SetVisible(true);

		m_sEmotion = "OhNoIntro";

		m_eStage = STAGE_FINISH;

		m_flStartTime = GameServer()->GetGameTime();
	}
	else
	{
		IntroWindow()->GetRenderer()->ZoomIntoHole();
		m_flFadeToBlack = GameServer()->GetGameTime();
	}
}
