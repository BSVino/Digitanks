#include "hud.h"

#include <GL/glew.h>
#include <sstream>

#include "digitankswindow.h"
#include "digitanks/digitanksgame.h"
#include "debugdraw.h"
#include "instructor.h"
#include "game/camera.h"
#include "renderer/renderer.h"
#include <game/digitanks/cpu.h>
#include <game/digitanks/projectile.h>
#include <game/digitanks/loader.h>
#include <sound/sound.h>

using namespace glgui;

CPowerBar::CPowerBar(powerbar_type_t ePowerbarType)
	: CLabel(0, 0, 100, 100, "")
{
	m_ePowerbarType = ePowerbarType;
}

void CPowerBar::Think()
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentSelection())
		return;

	CSelectable* pSelection = DigitanksGame()->GetCurrentSelection();

	char szLabel[100];
	if (m_ePowerbarType == POWERBAR_HEALTH)
	{
		if (pSelection->TakesDamage())
		{
			sprintf(szLabel, "Hull Strength: %.1f/%.1f", pSelection->GetHealth(), pSelection->GetTotalHealth());
			SetText(szLabel);
		}
		else
			SetText("");
	}
	else if (m_ePowerbarType == POWERBAR_ATTACK)
	{
		if (strlen(pSelection->GetPowerBar1Text()))
		{
			sprintf(szLabel, "%s: %d%%", pSelection->GetPowerBar1Text(), (int)(pSelection->GetPowerBar1Value()*100));
			SetText(szLabel);
		}
		else
			SetText("");
	}
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
	{
		if (strlen(pSelection->GetPowerBar2Text()))
		{
			sprintf(szLabel, "%s: %d%%", pSelection->GetPowerBar2Text(), (int)(pSelection->GetPowerBar2Value()*100));
			SetText(szLabel);
		}
		else
			SetText("");
	}
	else
	{
		if (strlen(pSelection->GetPowerBar3Text()))
		{
			sprintf(szLabel, "%s: %d%%", pSelection->GetPowerBar3Text(), (int)(pSelection->GetPowerBar3Value()*100));
			SetText(szLabel);
		}
		else
			SetText("");
	}
}

void CPowerBar::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentSelection())
		return;

	CSelectable* pSelection = DigitanksGame()->GetCurrentSelection();

	if (m_ePowerbarType == POWERBAR_HEALTH)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetHealth() / pSelection->GetTotalHealth())-2, h-2, Color(0, 150, 0));
	else if (m_ePowerbarType == POWERBAR_ATTACK)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetPowerBar1Size())-2, h-2, Color(150, 0, 0));
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetPowerBar2Size())-2, h-2, Color(0, 0, 150));
	else
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetPowerBar3Size())-2, h-2, Color(100, 100, 0));

	BaseClass::Paint(x, y, w, h);
}

CHUD::CHUD()
	: CPanel(0, 0, 100, 100)
{
	m_pGame = NULL;

	m_pHealthBar = new CPowerBar(POWERBAR_HEALTH);
	m_pAttackPower = new CPowerBar(POWERBAR_ATTACK);
	m_pDefensePower = new CPowerBar(POWERBAR_DEFENSE);
	m_pMovementPower = new CPowerBar(POWERBAR_MOVEMENT);

	AddControl(m_pHealthBar);
	AddControl(m_pAttackPower);
	AddControl(m_pDefensePower);
	AddControl(m_pMovementPower);

	m_pButtonPanel = new CMouseCapturePanel();
	AddControl(m_pButtonPanel);

	// TODO: Remove entirely if not needed
/*	m_pAutoButton = new CButton(0, 0, 0, 0, "Auto");
	m_pAutoButton->SetClickedListener(this, Auto);
	AddControl(m_pAutoButton);*/

	m_pButton1 = new CPictureButton("");
	m_pButton1->SetCursorInListener(this, Button1CursorIn);
	m_pButton1->SetCursorOutListener(this, Button1CursorOut);
	m_pButtonPanel->AddControl(m_pButton1);

	m_pButton2 = new CPictureButton("");
	m_pButton2->SetCursorInListener(this, Button2CursorIn);
	m_pButton2->SetCursorOutListener(this, Button2CursorOut);
	m_pButtonPanel->AddControl(m_pButton2);

	m_pButton3 = new CPictureButton("");
	m_pButton3->SetCursorInListener(this, Button3CursorIn);
	m_pButton3->SetCursorOutListener(this, Button3CursorOut);
	m_pButtonPanel->AddControl(m_pButton3);

	m_pButton4 = new CPictureButton("");
	m_pButton4->SetCursorInListener(this, Button4CursorIn);
	m_pButton4->SetCursorOutListener(this, Button4CursorOut);
	m_pButtonPanel->AddControl(m_pButton4);

	m_pButton5 = new CPictureButton("");
	m_pButton5->SetCursorInListener(this, Button5CursorIn);
	m_pButton5->SetCursorOutListener(this, Button5CursorOut);
	m_pButtonPanel->AddControl(m_pButton5);

	m_pButtonHelp1 = new CLabel(0, 0, 50, 50, "");
	m_pButtonPanel->AddControl(m_pButtonHelp1);

	m_pButtonHelp2 = new CLabel(0, 0, 50, 50, "");
	m_pButtonPanel->AddControl(m_pButtonHelp2);

	m_pButtonHelp3 = new CLabel(0, 0, 50, 50, "");
	m_pButtonPanel->AddControl(m_pButtonHelp3);

	m_pButtonHelp4 = new CLabel(0, 0, 50, 50, "");
	m_pButtonPanel->AddControl(m_pButtonHelp4);

	m_pButtonHelp5 = new CLabel(0, 0, 50, 50, "");
	m_pButtonPanel->AddControl(m_pButtonHelp5);

	m_pFireAttack = new CLabel(0, 0, 50, 50, "");
	m_pFireDefend = new CLabel(0, 0, 50, 50, "");
	AddControl(m_pFireAttack);
	AddControl(m_pFireDefend);

	m_pAttackInfo = new CLabel(0, 0, 100, 150, "");
	m_pAttackInfo->SetWrap(false);
	m_pAttackInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pAttackInfo);
	m_iAttackInfoPanel = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-attack-info.png", true);

	m_pScoreboard = new CLabel(0, 0, 100, 150, "");
	m_pScoreboard->SetWrap(false);
	m_pScoreboard->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pScoreboard->SetFontFaceSize(10);
	AddControl(m_pScoreboard);

	m_pFrontShieldInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pFrontShieldInfo);

	m_pRearShieldInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pRearShieldInfo);

	m_pLeftShieldInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pLeftShieldInfo);

	m_pRightShieldInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pRightShieldInfo);

	m_pTankInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pTankInfo);
	m_iTankInfoPanel = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-tank-info.png", true);

	m_pTurnInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pTurnInfo);
	m_iTurnInfoPanel = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-turn-report.png", true);

	m_pButtonInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pButtonInfo);

	m_pPressEnter = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pPressEnter);

	SetupMenu(MENUMODE_MAIN);

	m_pFPS = new CLabel(0, 0, 100, 20, "");
	AddControl(m_pFPS);

	m_pFPS->SetAlign(CLabel::TA_TOPLEFT);
	m_pFPS->SetPos(20, 20);
	m_pFPS->SetText("Free Demo");

	m_pPowerInfo = new CLabel(0, 0, 200, 20, "");
	AddControl(m_pPowerInfo);

	m_pPowerInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pPowerInfo->SetPos(200, 20);

	m_pFleetInfo = new CLabel(0, 0, 200, 20, "");
	AddControl(m_pFleetInfo);

	m_pFleetInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pFleetInfo->SetPos(200, 20);

	m_pBandwidthInfo = new CLabel(0, 0, 200, 20, "");
	AddControl(m_pBandwidthInfo);

	m_pBandwidthInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pBandwidthInfo->SetPos(200, 20);

	m_pUpdatesButton = new CPictureButton("Download Updates", CRenderer::LoadTextureIntoGL(L"textures/hud/hud-download-updates.png", true));
	m_pUpdatesButton->SetClickedListener(this, OpenUpdates);
	m_pUpdatesButton->ShowBackground(false);
	AddControl(m_pUpdatesButton);

	m_pUpdatesPanel = new CUpdatesPanel();
	m_pUpdatesPanel->SetVisible(false);
	AddControl(m_pUpdatesPanel, true);

	m_pTurnButton = new CPictureButton("TURN", CRenderer::LoadTextureIntoGL(L"textures/hud/turn.png", true));
	m_pTurnButton->SetClickedListener(this, EndTurn);
	m_pTurnButton->ShowBackground(false);
	AddControl(m_pTurnButton);

	m_flAttackInfoAlpha = m_flAttackInfoAlphaGoal = 0;

	m_flTurnInfoLerp = m_flTurnInfoLerpGoal = 0;
	m_flTurnInfoHeight = m_flTurnInfoHeightGoal = 0;

	m_iTurnSound = CSoundLibrary::Get()->AddSound("sound/turn.wav");

	m_iHUDGraphic = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-main.png", true);

	m_iAvatarIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/tank-avatar.png");
	m_iShieldIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/tank-avatar-shield.png");

	m_iSpeechBubble = CRenderer::LoadTextureIntoGL(L"textures/hud/bubble.png");

	m_iPowerIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-power.png");
	m_iFleetPointsIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-fleet.png");
	m_iBandwidthIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-bandwidth.png");

	//m_iCompetitionWatermark = CRenderer::LoadTextureIntoGL(L"textures/competition.png");
}

void CHUD::Layout()
{
	SetSize(GetParent()->GetWidth(), GetParent()->GetHeight());

	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	m_pAttackInfo->SetPos(iWidth - 165, iHeight - 150 - 90 - 10);
	m_pAttackInfo->SetSize(165, 90);

	m_pHealthBar->SetPos(iWidth/2 - 720/2 + 170, iHeight - 140);
	m_pHealthBar->SetSize(200, 20);

	m_pAttackPower->SetPos(iWidth/2 - 720/2 + 170, iHeight - 90);
	m_pAttackPower->SetSize(200, 20);

	m_pDefensePower->SetPos(iWidth/2 - 720/2 + 170, iHeight - 60);
	m_pDefensePower->SetSize(200, 20);

	m_pMovementPower->SetPos(iWidth/2 - 720/2 + 170, iHeight - 30);
	m_pMovementPower->SetSize(200, 20);

	m_pButtonPanel->SetPos(iWidth/2 - 720/2 + 380, iHeight - 140);
	m_pButtonPanel->SetRight(m_pButtonPanel->GetLeft() + 330);
	m_pButtonPanel->SetBottom(iHeight - 10);

	m_pButton1->SetSize(50, 50);
	m_pButton2->SetSize(50, 50);
	m_pButton3->SetSize(50, 50);
	m_pButton4->SetSize(50, 50);
	m_pButton5->SetSize(50, 50);

//	m_pAutoButton->SetPos(iWidth/2 - 1024/2 + 820, iHeight - 135);
//	m_pAutoButton->SetSize(50, 20);

	m_pButton1->SetPos(20, 20);
	m_pButton2->SetPos(80, 20);
	m_pButton3->SetPos(140, 20);
	m_pButton4->SetPos(200, 20);
	m_pButton5->SetPos(260, 20);

	m_pButtonHelp1->SetPos(20, 70);
	m_pButtonHelp2->SetPos(80, 70);
	m_pButtonHelp3->SetPos(140, 70);
	m_pButtonHelp4->SetPos(200, 70);
	m_pButtonHelp5->SetPos(260, 70);
	m_pButtonHelp1->SetWrap(false);
	m_pButtonHelp2->SetWrap(false);
	m_pButtonHelp3->SetWrap(false);
	m_pButtonHelp4->SetWrap(false);
	m_pButtonHelp5->SetWrap(false);
	m_pButtonHelp1->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pButtonHelp2->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pButtonHelp3->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pButtonHelp4->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pButtonHelp5->SetAlign(glgui::CLabel::TA_MIDDLECENTER);

	m_pLeftShieldInfo->SetDimensions(iWidth/2 - 720/2 + 10 + 150/2 - 50/2 - 40, iHeight - 150 + 10 + 130/2 - 50/2, 10, 50);
	m_pRightShieldInfo->SetDimensions(iWidth/2 - 720/2 + 10 + 150/2 + 50/2 + 30, iHeight - 150 + 10 + 130/2 - 50/2, 10, 50);
	m_pRearShieldInfo->SetDimensions(iWidth/2 - 720/2 + 10 + 150/2 - 50/2, iHeight - 150 + 10 + 130/2 + 50/2 + 25, 50, 10);
	m_pFrontShieldInfo->SetDimensions(iWidth/2 - 720/2 + 10 + 150/2 - 50/2, iHeight - 150 + 10 + 130/2 - 50/2 - 35, 50, 10);

	m_pLeftShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pRightShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pRearShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pFrontShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);

	m_pLeftShieldInfo->SetWrap(false);
	m_pRightShieldInfo->SetWrap(false);
	m_pRearShieldInfo->SetWrap(false);
	m_pFrontShieldInfo->SetWrap(false);

	m_pTankInfo->SetSize(140, 240);
	m_pTankInfo->SetPos(10, iHeight - m_pTankInfo->GetHeight() + 10 + 7);
	m_pTankInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pTankInfo->SetWrap(true);
	m_pTankInfo->SetFontFaceSize(10);

	m_pTurnInfo->SetSize(278, 150);
	m_pTurnInfo->SetPos(iWidth/2 - m_pTurnInfo->GetWidth()/2, 0);
	m_pTurnInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pTurnInfo->SetWrap(true);
	m_pTurnInfo->SetFontFaceSize(10);

	m_pButtonInfo->SetSize(250, 250);
	m_pButtonInfo->SetPos(iWidth/2, iHeight - 400);
	m_pButtonInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pButtonInfo->SetWrap(true);

	m_pUpdatesButton->SetSize(175, 40);
	m_pUpdatesButton->SetPos(m_pTurnInfo->GetLeft() - 20 - m_pUpdatesButton->GetWidth(), 0);
	m_pUpdatesButton->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pUpdatesButton->SetWrap(false);

	m_pUpdatesPanel->Layout();

	m_pPressEnter->SetDimensions(iWidth/2 - 100/2, iHeight*2/3, 100, 50);
	m_pPressEnter->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pPressEnter->SetWrap(false);

	m_pPowerInfo->SetAlign(CLabel::TA_TOPLEFT);
	m_pPowerInfo->SetPos(iWidth - 290, 20);
	m_pPowerInfo->SetWrap(false);

	m_pFleetInfo->SetAlign(CLabel::TA_TOPLEFT);
	m_pFleetInfo->SetPos(iWidth - 220, 20);
	m_pFleetInfo->SetWrap(false);

	m_pBandwidthInfo->SetAlign(CLabel::TA_TOPLEFT);
	m_pBandwidthInfo->SetPos(iWidth - 150, 20);
	m_pBandwidthInfo->SetWrap(false);

	m_pTurnButton->SetPos(iWidth - 140, iHeight - 100);
	m_pTurnButton->SetSize(120, 90);

	UpdateTeamInfo();
	UpdateInfo();
	SetupMenu();
}

void CHUD::Think()
{
	if (Game()->IsLoading())
		return;

	BaseClass::Think();

	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	Vector vecPoint;
	bool bMouseOnGrid = false;
	CBaseEntity* pHit = NULL;
	if (DigitanksGame()->GetControlMode() != MODE_NONE)
		bMouseOnGrid = CDigitanksWindow::Get()->GetMouseGridPosition(vecPoint, &pHit);

	if (m_bHUDActive && bMouseOnGrid && pCurrentTank)
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE || DigitanksGame()->GetControlMode() == MODE_TURN || DigitanksGame()->GetControlMode() == MODE_AIM)
			UpdateInfo();

		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		{
			Vector vecMove = vecPoint;
			vecMove.y = pCurrentTank->FindHoverHeight(vecMove);
			pCurrentTank->SetPreviewMove(vecMove);
		}

		if (DigitanksGame()->GetControlMode() == MODE_TURN)
		{
			if ((vecPoint - pCurrentTank->GetDesiredMove()).LengthSqr() > 4*4)
			{
				Vector vecTurn = vecPoint - pCurrentTank->GetDesiredMove();
				vecTurn.Normalize();
				float flTurn = atan2(vecTurn.z, vecTurn.x) * 180/M_PI;
				pCurrentTank->SetPreviewTurn(flTurn);
			}
			else
				pCurrentTank->SetPreviewTurn(pCurrentTank->GetAngles().y);
		}

		if (DigitanksGame()->GetControlMode() == MODE_AIM)
		{
			CDigitanksEntity* pDTHit = dynamic_cast<CDigitanksEntity*>(pHit);
			if (pDTHit && pDTHit->GetVisibility() > 0)
				pCurrentTank->SetPreviewAim(pDTHit->GetOrigin());
			else
				pCurrentTank->SetPreviewAim(vecPoint);
		}
	}

	CStructure* pCurrentStructure = DigitanksGame()->GetCurrentStructure();
	if (m_bHUDActive && bMouseOnGrid && pCurrentStructure)
	{
		if (DigitanksGame()->GetControlMode() == MODE_BUILD)
		{
			CCPU* pCPU = dynamic_cast<CCPU*>(pCurrentStructure);
			if (pCPU)
				pCPU->SetPreviewBuild(vecPoint);
		}
	}

	if (m_eMenuMode == MENUMODE_MAIN)
	{
		if (m_bHUDActive && pCurrentTank && pCurrentTank->HasBonusPoints())
		{
			float flRamp = 1;
			if (!CDigitanksWindow::Get()->GetInstructor()->GetActive() || CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial() >= CInstructor::TUTORIAL_UPGRADE)
				flRamp = fabs(fmod(DigitanksGame()->GetGameTime(), 2)-1);
			m_pButton5->SetButtonColor(Color((int)RemapVal(flRamp, 0, 1, 0, 250), (int)RemapVal(flRamp, 0, 1, 0, 200), 0));
		}
	}

	if (DigitanksGame()->GetCurrentTeam() == DigitanksGame()->GetLocalDigitanksTeam())
	{
		bool bShowEnter = true;

		if (DigitanksGame()->GetControlMode() != MODE_NONE)
			bShowEnter = false;

		if (!CDigitanksWindow::Get()->GetInstructor()->IsFeatureDisabled(DISABLE_ENTER))
			bShowEnter = false;

		m_pPressEnter->SetVisible(bShowEnter);
	}
	else
	{
		m_pPressEnter->SetVisible(true);
	}

	if (pCurrentTank)
	{
		m_pFireAttack->SetVisible(DigitanksGame()->GetControlMode() == MODE_FIRE);
		m_pFireDefend->SetVisible(DigitanksGame()->GetControlMode() == MODE_FIRE);
		m_pFireAttack->SetAlign(CLabel::TA_MIDDLECENTER);
		m_pFireDefend->SetAlign(CLabel::TA_MIDDLECENTER);
		m_pFireAttack->SetWrap(false);
		m_pFireDefend->SetWrap(false);
	}

	if (wcslen(m_pAttackInfo->GetText()))
		m_flAttackInfoAlphaGoal = 1.0f;
	else
		m_flAttackInfoAlphaGoal = 0.0f;

	m_flAttackInfoAlpha = Approach(m_flAttackInfoAlphaGoal, m_flAttackInfoAlpha, Game()->GetFrameTime());

	m_flTurnInfoHeightGoal = m_pTurnInfo->GetTextHeight();
	m_flTurnInfoLerp = Approach(m_flTurnInfoLerpGoal, m_flTurnInfoLerp, Game()->GetFrameTime());
	m_flTurnInfoHeight = Approach(m_flTurnInfoHeightGoal, m_flTurnInfoHeight, Game()->GetFrameTime()*100);

	float flTurnInfoHeight = m_flTurnInfoHeight+10;
	m_pTurnInfo->SetSize(m_pTurnInfo->GetWidth(), (int)flTurnInfoHeight);
	m_pTurnInfo->SetPos(m_pTurnInfo->GetLeft(), 10 - (int)(Lerp(1.0f-m_flTurnInfoLerp, 0.2f)*flTurnInfoHeight));

	m_pUpdatesButton->SetVisible(!!DigitanksGame()->GetUpdateGrid());

	m_pScoreboard->SetVisible(DigitanksGame()->ShouldShowScores());

	char szFPS[100];
	sprintf(szFPS, "Free Demo\n%d fps", (int)(1/Game()->GetFrameTime()));
	m_pFPS->SetText(szFPS);
}

void CHUD::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	if (Game()->IsLoading())
		return;

	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	do {
		CRenderingContext c(Game()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);
		CRootPanel::PaintTexture(m_iHUDGraphic, iWidth/2 - 720/2, iHeight-160, 720, 160);

		CRootPanel::PaintTexture(m_iPowerIcon, iWidth - 320, 10, 20, 20);
		CRootPanel::PaintTexture(m_iFleetPointsIcon, iWidth - 250, 10, 20, 20);
		CRootPanel::PaintTexture(m_iBandwidthIcon, iWidth - 180, 10, 20, 20);

		CRootPanel::PaintTexture(m_iTurnInfoPanel, m_pTurnInfo->GetLeft()-15, m_pTurnInfo->GetBottom()-585, 278, 600);

		if (m_flAttackInfoAlpha > 0)
			CRootPanel::PaintTexture(m_iAttackInfoPanel, iWidth-175, m_pAttackInfo->GetTop()-15, 175, 110, Color(255, 255, 255, (int)(255*m_flAttackInfoAlpha)));

		CRootPanel::PaintTexture(m_iTankInfoPanel, 0, iHeight-250, 150, 250);
	} while (false);

	CRootPanel::PaintRect(m_pScoreboard->GetLeft()-3, m_pScoreboard->GetTop()-9, m_pScoreboard->GetWidth()+6, m_pScoreboard->GetHeight()+6, Color(0, 0, 0, 100));

	if (m_pButtonInfo->GetText()[0] != L'\0')
		CRootPanel::PaintRect(m_pButtonInfo->GetLeft()-3, m_pButtonInfo->GetTop()-9, m_pButtonInfo->GetWidth()+6, m_pButtonInfo->GetHeight()+6, Color(0, 0, 0));

	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
	{
		CDigitanksTeam* pTeam = DigitanksGame()->GetDigitanksTeam(i);
		for (size_t j = 0; j < pTeam->GetNumTanks(); j++)
		{
			CDigitank* pTank = pTeam->GetTank(j);

			if (pTank->GetVisibility() == 0)
				continue;

			Vector vecOrigin = pTank->GetDesiredMove();
			Vector vecScreen = Game()->GetRenderer()->ScreenPosition(vecOrigin);

			if (!CDigitanksWindow::Get()->IsAltDown() && pTank->GetTeam() != DigitanksGame()->GetLocalDigitanksTeam())
				continue;

			CRootPanel::PaintRect((int)vecScreen.x - 51, (int)vecScreen.y - 61, 102, 5, Color(255, 255, 255, 128));
			CRootPanel::PaintRect((int)vecScreen.x - 50, (int)vecScreen.y - 60, (int)(100.0f*pTank->GetHealth()/pTank->GetTotalHealth()), 3, Color(100, 255, 100));

			float flAttackPower = pTank->GetAttackPower(true);
			float flDefensePower = pTank->GetDefensePower(true);
			float flMovementPower = pTank->GetMovementPower(true);
			float flTotalPower = flAttackPower + flDefensePower + flMovementPower;
			flAttackPower = flAttackPower/flTotalPower;
			flDefensePower = flDefensePower/flTotalPower;
			flMovementPower = flMovementPower/flTotalPower;
			CRootPanel::PaintRect((int)vecScreen.x - 51, (int)vecScreen.y - 51, 102, 5, Color(255, 255, 255, 128));
			CRootPanel::PaintRect((int)vecScreen.x - 50, (int)vecScreen.y - 50, (int)(100.0f*flAttackPower), 3, Color(255, 0, 0));
			CRootPanel::PaintRect((int)vecScreen.x - 50 + (int)(100.0f*flAttackPower), (int)vecScreen.y - 50, (int)(100.0f*flDefensePower), 3, Color(0, 0, 255));
			CRootPanel::PaintRect((int)vecScreen.x - 50 + (int)(100.0f*(1-flMovementPower)), (int)vecScreen.y - 50, (int)(100.0f*flMovementPower), 3, Color(255, 255, 0));

			if (m_bHUDActive && DigitanksGame()->GetCurrentTeam()->IsCurrentSelection(pTank) && DigitanksGame()->GetControlMode() == MODE_FIRE)
			{
				int iHeight = (int)(200 * (pTank->GetBasePower()-pTank->GetBaseMovementPower())/pTank->GetBasePower());

				if (iHeight < 20)
					iHeight = 20;

				int iTop = (int)vecScreen.y - iHeight/2;
				int iBottom = (int)vecScreen.y + iHeight/2;

				m_pFireAttack->SetSize(0, 20);
				m_pFireDefend->SetSize(0, 20);

				char szLabel[100];
				sprintf(szLabel, "Damage: %d%%", (int)(pTank->GetAttackPower(true)/pTank->GetBasePower()*100));
				m_pFireAttack->SetText(szLabel);
				sprintf(szLabel, "Defense: %d%%", (int)(pTank->GetDefensePower(true)/pTank->GetBasePower()*100));
				m_pFireDefend->SetText(szLabel);

				m_pFireAttack->EnsureTextFits();
				m_pFireDefend->EnsureTextFits();

				m_pFireAttack->SetPos((int)vecScreen.x + 70 - m_pFireAttack->GetWidth()/2, iTop-20);
				m_pFireDefend->SetPos((int)vecScreen.x + 70 - m_pFireDefend->GetWidth()/2, iBottom);

				int mx, my;
				glgui::CRootPanel::GetFullscreenMousePos(mx, my);

				float flAttackPercentage = RemapValClamped((float)my, (float)iTop, (float)iBottom, 1, 0);

				CRootPanel::PaintRect((int)vecScreen.x + 60, iTop, 20, iHeight, Color(255, 255, 255, 128));

				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + 1, 18, (int)((1-flAttackPercentage)*(iHeight-2)), Color(0, 0, 255, 255));
				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + 1 + (int)((1-flAttackPercentage)*(iHeight-2)), 18, (int)(flAttackPercentage*(iHeight-2)), Color(255, 0, 0, 255));
				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + (int)((1-flAttackPercentage)*(iHeight-2)) - 2, 18, 6, Color(128, 128, 128, 255));

				if (CDigitanksWindow::Get()->IsShiftDown())
				{
					CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentTeam();
					for (size_t t = 0; t < pTeam->GetNumTanks(); t++)
					{
						CDigitank* pTank = pTeam->GetTank(t);
						pTank->SetAttackPower(flAttackPercentage * (pTank->GetBasePower()-pTank->GetBaseMovementPower()));
					}
				}
				else
					DigitanksGame()->GetCurrentTank()->SetAttackPower(flAttackPercentage * (pTank->GetBasePower()-pTank->GetBaseMovementPower()));

				UpdateInfo();
			}
		}
	}

	CPanel::Paint(x, y, w, h);

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	if (pTank)
	{
		CRenderingContext c(Game()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);

		CRootPanel::PaintTexture(m_iAvatarIcon, iWidth/2 - 720/2 + 10 + 150/2 - 50/2, iHeight - 150 + 10 + 130/2 - 50/2, 50, 50, pTank->GetTeam()->GetColor());

		c.SetBlend(BLEND_ADDITIVE);

		c.Translate(Vector((float)(iWidth/2 - 720/2 + 10 + 150/2), (float)(iHeight - 150 + 10 + 130/2), 0));

		do
		{
			int iShield = (int)(255*pTank->GetFrontShieldStrength());
			if (iShield > 255)
				iShield = 255;
			CRootPanel::PaintTexture(m_iShieldIcon, -50/2, -50/2 - 20, 50, 10, Color(255, 255, 255, iShield));
		}
		while (false);

		do
		{
			CRenderingContext c(Game()->GetRenderer());
			c.Rotate(90, Vector(0, 0, 1));

			int iShield = (int)(255*pTank->GetRightShieldStrength());
			if (iShield > 255)
				iShield = 255;
			CRootPanel::PaintTexture(m_iShieldIcon, -50/2, -50/2 - 20, 50, 10, Color(255, 255, 255, iShield));
		}
		while (false);

		do
		{
			CRenderingContext c(Game()->GetRenderer());
			c.Rotate(180, Vector(0, 0, 1));

			int iShield = (int)(255*pTank->GetRearShieldStrength());
			if (iShield > 255)
				iShield = 255;
			CRootPanel::PaintTexture(m_iShieldIcon, -50/2, -50/2 - 20, 50, 10, Color(255, 255, 255, iShield));
		}
		while (false);

		do
		{
			CRenderingContext c(Game()->GetRenderer());
			c.Rotate(270, Vector(0, 0, 1));

			int iShield = (int)(255*pTank->GetLeftShieldStrength());
			if (iShield > 255)
				iShield = 255;
			CRootPanel::PaintTexture(m_iShieldIcon, -50/2, -50/2 - 20, 50, 10, Color(255, 255, 255, iShield));
		}
		while (false);
	}

	CSelectable* pSelection = DigitanksGame()->GetLocalDigitanksTeam()->GetCurrentSelection();

	if (pSelection && !IsUpdatesPanelOpen())
	{
		Vector vecMin;
		Vector vecMax;

		float flBound = pSelection->GetBoundingRadius();

		std::vector<Vector> aVecs;
		aVecs.push_back(Vector(-flBound, -1, -flBound));
		aVecs.push_back(Vector(flBound, -1, -flBound));
		aVecs.push_back(Vector(-flBound, flBound, -flBound));
		aVecs.push_back(Vector(flBound, flBound, -flBound));
		aVecs.push_back(Vector(-flBound, -1, flBound));
		aVecs.push_back(Vector(-flBound, flBound, flBound));
		aVecs.push_back(Vector(flBound, -1, flBound));
		aVecs.push_back(Vector(flBound, flBound, flBound));

		Vector vecOrigin;
		if (pTank)
			vecOrigin = pTank->GetDesiredMove();
		else
			vecOrigin = pSelection->GetOrigin();

		for (size_t v = 0; v < aVecs.size(); v++)
		{
			Vector vecCorner = Game()->GetRenderer()->ScreenPosition(vecOrigin+aVecs[v]);
			if (v == 0)
			{
				vecMin = vecMax = vecCorner;
				continue;
			}

			for (size_t x = 0; x < 3; x++)
			{
				if (vecCorner.x < vecMin.x)
					vecMin.x = vecCorner.x;
				if (vecCorner.y < vecMin.y)
					vecMin.y = vecCorner.y;
				if (vecCorner.x > vecMax.x)
					vecMax.x = vecCorner.x;
				if (vecCorner.y > vecMax.y)
					vecMax.y = vecCorner.y;
			}
		}

		CRootPanel::PaintRect((int)vecMin.x, (int)vecMin.y, (int)(vecMax.x-vecMin.x), 1, Color(255, 255, 255));
		CRootPanel::PaintRect((int)vecMin.x, (int)vecMin.y, 1, (int)(vecMax.y-vecMin.y), Color(255, 255, 255));
		CRootPanel::PaintRect((int)vecMax.x, (int)vecMin.y, 1, (int)(vecMax.y-vecMin.y), Color(255, 255, 255));
		CRootPanel::PaintRect((int)vecMin.x, (int)vecMax.y, (int)(vecMax.x-vecMin.x), 1, Color(255, 255, 255));
	}

/*	while (true)
	{
		CRenderingContext c(Game()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);
		CRootPanel::PaintTexture(m_iCompetitionWatermark, 40, 40, 128, 128);
		break;
	}*/
}

void CHUD::UpdateInfo()
{
	m_pAttackInfo->SetText("");

	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	if (pCurrentTank)
		UpdateTankInfo(pCurrentTank);

	CStructure* pCurrentStructure = DigitanksGame()->GetCurrentStructure();

	if (pCurrentStructure)
		UpdateStructureInfo(pCurrentStructure);

	CSelectable* pCurrentSelection = DigitanksGame()->GetCurrentSelection();

	if (pCurrentSelection)
	{
		std::wstring sInfo;
		pCurrentSelection->UpdateInfo(sInfo);
		m_pTankInfo->SetText(sInfo.c_str());
	}
}

void CHUD::UpdateTankInfo(CDigitank* pTank)
{
	char szShieldInfo[1024];
	sprintf(szShieldInfo, "%.1f/%.1f",
		pTank->GetFrontShieldStrength() * pTank->GetFrontShieldMaxStrength(),
		pTank->GetFrontShieldMaxStrength() * pTank->GetDefenseScale(true));
	m_pFrontShieldInfo->SetText(szShieldInfo);

	sprintf(szShieldInfo, "%.1f/%.1f",
		pTank->GetRearShieldStrength() * pTank->GetRearShieldMaxStrength(),
		pTank->GetRearShieldMaxStrength() * pTank->GetDefenseScale(true));
	m_pRearShieldInfo->SetText(szShieldInfo);

	sprintf(szShieldInfo, "%.1f/\n%.1f",
		pTank->GetLeftShieldStrength() * pTank->GetLeftShieldMaxStrength(),
		pTank->GetLeftShieldMaxStrength() * pTank->GetDefenseScale(true));
	m_pLeftShieldInfo->SetText(szShieldInfo);

	sprintf(szShieldInfo, "%.1f/\n%.1f",
		pTank->GetRightShieldStrength() * pTank->GetRightShieldMaxStrength(),
		pTank->GetRightShieldMaxStrength() * pTank->GetDefenseScale(true));
	m_pRightShieldInfo->SetText(szShieldInfo);
	m_pAttackInfo->SetText(L"");

	Vector vecOrigin;
	if (DigitanksGame()->GetControlMode() == MODE_MOVE && pTank->GetPreviewMoveTurnPower() <= pTank->GetTotalMovementPower())
		vecOrigin = pTank->GetPreviewMove();
	else
		vecOrigin = pTank->GetDesiredMove();

	Vector vecAim;
	if (DigitanksGame()->GetControlMode() == MODE_AIM)
		vecAim = pTank->GetPreviewAim();
	else
		vecAim = pTank->GetDesiredAim();

	Vector vecAttack = vecOrigin - vecAim;
	float flAttackDistance = vecAttack.Length();

	if (flAttackDistance > pTank->GetMaxRange())
		return;

	if (flAttackDistance < pTank->GetMinRange())
		return;

	if (!pTank->HasDesiredAim() && !pTank->IsPreviewAimValid())
		return;

	float flRadius = RemapValClamped(flAttackDistance, pTank->GetEffRange(), pTank->GetMaxRange(), 2, TANK_MAX_RANGE_RADIUS);

	CDigitank* pClosestTarget = NULL;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);

		if (!pEntity)
			continue;

		CDigitank* pTargetTank = dynamic_cast<CDigitank*>(pEntity);

		if (!pTargetTank)
			continue;

		if (pTargetTank == pTank)
			continue;

		if (pTargetTank->GetTeam() == pTank->GetTeam())
			continue;

		Vector vecOrigin2D = pTargetTank->GetOrigin();
		Vector vecAim2D = vecAim;
		vecOrigin2D.y = vecAim2D.y = 0;
		if ((vecOrigin2D - vecAim2D).LengthSqr() > flRadius*flRadius)
			continue;

		if (pTargetTank->GetVisibility() == 0)
			continue;

		if (!pClosestTarget)
		{
			pClosestTarget = pTargetTank;
			continue;
		}

		if ((pTargetTank->GetOrigin() - vecAim).LengthSqr() < (pClosestTarget->GetOrigin() - vecAim).LengthSqr())
			pClosestTarget = pTargetTank;
	}

	if (!pClosestTarget)
		return;

	vecAttack = vecOrigin - pClosestTarget->GetOrigin();

	Vector vecOrigin2D = pClosestTarget->GetOrigin();
	Vector vecAim2D = vecAim;
	vecOrigin2D.y = vecAim2D.y = 0;
	float flTargetDistance = (vecAim2D - vecOrigin2D).Length();

	if (flTargetDistance > flRadius)
		return;

	float flShieldStrength = (*pClosestTarget->GetShieldForAttackDirection(vecAttack.Normalized()));
	float flDamageBlocked = flShieldStrength * pClosestTarget->GetDefenseScale(true);
	float flAttackDamage = pTank->GetAttackPower(true);

	float flShieldDamage;
	float flTankDamage = 0;
	if (flAttackDamage - flDamageBlocked <= 0)
		flShieldDamage = flAttackDamage;
	else
	{
		flShieldDamage = flDamageBlocked;
		flTankDamage = flAttackDamage - flDamageBlocked;
	}

	char szAttackInfo[1024];
	sprintf(szAttackInfo,
		"ATTACK REPORT\n \n"
		"Shield Damage: %.1f/%.1f\n"
		"Digitank Damage: %.1f/%.1f\n",
		flShieldDamage, flShieldStrength * pClosestTarget->GetDefenseScale(true),
		flTankDamage, pClosestTarget->GetHealth()
	);

	m_pAttackInfo->SetText(szAttackInfo);
}

void CHUD::UpdateStructureInfo(CStructure* pStructure)
{
	m_pFrontShieldInfo->SetText("");
	m_pRearShieldInfo->SetText("");
	m_pLeftShieldInfo->SetText("");
	m_pRightShieldInfo->SetText("");

	m_pAttackInfo->SetText(L"");
}

void CHUD::UpdateTeamInfo()
{
	std::stringstream s1;
	CDigitanksTeam* pTeam = DigitanksGame()->GetLocalDigitanksTeam();
	s1 << pTeam->GetTotalProduction() << "\n";
	m_pPowerInfo->SetText(s1.str().c_str());

	std::stringstream s2;
	s2 << pTeam->GetUsedFleetPoints() << "/" << pTeam->GetTotalFleetPoints() << "\n";
	m_pFleetInfo->SetText(s2.str().c_str());

	std::stringstream s3;
	s3 << pTeam->GetUpdateDownloaded() << "/" << pTeam->GetUpdateSize() << "mb @" << pTeam->GetBandwidth() << "mbps";
	m_pBandwidthInfo->SetText(s3.str().c_str());
}

void CHUD::UpdateScoreboard()
{
	std::vector<CDigitanksTeam*> apSortedTeams;

	// Prob not the fastest sorting algorithm but it doesn't need to be.
	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
	{
		CDigitanksTeam* pTeam = DigitanksGame()->GetDigitanksTeam(i);
		if (apSortedTeams.size() == 0)
		{
			apSortedTeams.push_back(pTeam);
			continue;
		}

		bool bFound = false;
		for (size_t j = 0; j < apSortedTeams.size(); j++)
		{
			if (pTeam->GetScore() > apSortedTeams[j]->GetScore())
			{
				apSortedTeams.insert(apSortedTeams.begin()+j, pTeam);
				bFound = true;
				break;
			}
		}

		if (!bFound)
			apSortedTeams.push_back(pTeam);
	}

	std::wstringstream s;
	for (size_t i = 0; i < apSortedTeams.size(); i++)
	{
		CDigitanksTeam* pTeam = apSortedTeams[i];

		if (DigitanksGame()->IsTeamControlledByMe(pTeam))
			s << "[";
		s << pTeam->GetName();
		if (DigitanksGame()->IsTeamControlledByMe(pTeam))
			s << "]";

		s << ": " << pTeam->GetScore() << "\n";
	}

	m_pScoreboard->SetText(s.str().c_str());

	m_pScoreboard->SetSize(100, 9999);
	m_pScoreboard->SetSize(m_pScoreboard->GetWidth(), (int)m_pScoreboard->GetTextHeight());

	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();

	m_pScoreboard->SetPos(iWidth - m_pScoreboard->GetWidth() - 10, m_pAttackInfo->GetTop() - m_pScoreboard->GetHeight() - 20);
}

void CHUD::SetGame(CDigitanksGame *pGame)
{
	m_pGame = pGame;
	m_pGame->SetListener(this);
}

void CHUD::SetupMenu()
{
	SetupMenu(m_eMenuMode);
}

void CHUD::SetupMenu(menumode_t eMenuMode)
{
	m_pButton1->SetClickedListener(NULL, NULL);
	m_pButton2->SetClickedListener(NULL, NULL);
	m_pButton3->SetClickedListener(NULL, NULL);
	m_pButton4->SetClickedListener(NULL, NULL);
	m_pButton5->SetClickedListener(NULL, NULL);

	m_pButtonHelp1->SetText("");
	m_pButtonHelp2->SetText("");
	m_pButtonHelp3->SetText("");
	m_pButtonHelp4->SetText("");
	m_pButtonHelp5->SetText("");

	SetButton1Color(glgui::g_clrBox);
	SetButton2Color(glgui::g_clrBox);
	SetButton3Color(glgui::g_clrBox);
	SetButton4Color(glgui::g_clrBox);
	SetButton5Color(glgui::g_clrBox);

	SetButton1Texture(0);
	SetButton2Texture(0);
	SetButton3Texture(0);
	SetButton4Texture(0);
	SetButton5Texture(0);

	SetButtonInfo(0, L"");
	SetButtonInfo(1, L"");
	SetButtonInfo(2, L"");
	SetButtonInfo(3, L"");
	SetButtonInfo(4, L"");

	if (!IsActive() || !DigitanksGame()->GetCurrentSelection() || DigitanksGame()->GetCurrentSelection()->GetTeam() != Game()->GetLocalTeam())
		return;

	DigitanksGame()->GetCurrentSelection()->SetupMenu(eMenuMode);

	m_eMenuMode = eMenuMode;
}

void CHUD::SetButton1Listener(IEventListener::Callback pfnCallback)
{
	if (pfnCallback)
		m_pButton1->SetClickedListener(this, pfnCallback);
	else
		m_pButton1->SetClickedListener(NULL, NULL);
}

void CHUD::SetButton2Listener(IEventListener::Callback pfnCallback)
{
	if (pfnCallback)
		m_pButton2->SetClickedListener(this, pfnCallback);
	else
		m_pButton2->SetClickedListener(NULL, NULL);
}

void CHUD::SetButton3Listener(IEventListener::Callback pfnCallback)
{
	if (pfnCallback)
		m_pButton3->SetClickedListener(this, pfnCallback);
	else
		m_pButton3->SetClickedListener(NULL, NULL);
}

void CHUD::SetButton4Listener(IEventListener::Callback pfnCallback)
{
	if (pfnCallback)
		m_pButton4->SetClickedListener(this, pfnCallback);
	else
		m_pButton4->SetClickedListener(NULL, NULL);
}

void CHUD::SetButton5Listener(IEventListener::Callback pfnCallback)
{
	if (pfnCallback)
		m_pButton5->SetClickedListener(this, pfnCallback);
	else
		m_pButton5->SetClickedListener(NULL, NULL);
}

void CHUD::SetButton1Help(const char* pszHelp)
{
	m_pButtonHelp1->SetText(pszHelp);
}

void CHUD::SetButton2Help(const char* pszHelp)
{
	m_pButtonHelp2->SetText(pszHelp);
}

void CHUD::SetButton3Help(const char* pszHelp)
{
	m_pButtonHelp3->SetText(pszHelp);
}

void CHUD::SetButton4Help(const char* pszHelp)
{
	m_pButtonHelp4->SetText(pszHelp);
}

void CHUD::SetButton5Help(const char* pszHelp)
{
	m_pButtonHelp5->SetText(pszHelp);
}

void CHUD::SetButton1Texture(size_t iTexture)
{
	m_pButton1->SetTexture(iTexture);
}

void CHUD::SetButton2Texture(size_t iTexture)
{
	m_pButton2->SetTexture(iTexture);
}

void CHUD::SetButton3Texture(size_t iTexture)
{
	m_pButton3->SetTexture(iTexture);
}

void CHUD::SetButton4Texture(size_t iTexture)
{
	m_pButton4->SetTexture(iTexture);
}

void CHUD::SetButton5Texture(size_t iTexture)
{
	m_pButton5->SetTexture(iTexture);
}

void CHUD::SetButton1Color(Color clrButton)
{
	m_pButton1->SetButtonColor(clrButton);
}

void CHUD::SetButton2Color(Color clrButton)
{
	m_pButton2->SetButtonColor(clrButton);
}

void CHUD::SetButton3Color(Color clrButton)
{
	m_pButton3->SetButtonColor(clrButton);
}

void CHUD::SetButton4Color(Color clrButton)
{
	m_pButton4->SetButtonColor(clrButton);
}

void CHUD::SetButton5Color(Color clrButton)
{
	m_pButton5->SetButtonColor(clrButton);
}

void CHUD::SetButtonInfo(int iButton, const wchar_t* pszInfo)
{
	m_aszButtonInfos[iButton] = pszInfo;
}

void CHUD::GameStart()
{
	DigitanksGame()->SetControlMode(MODE_NONE);
	CDigitanksWindow::Get()->GetInstructor()->Initialize();
	ClearTurnInfo();
}

void CHUD::GameOver(bool bPlayerWon)
{
	CDigitanksWindow::Get()->GameOver(bPlayerWon);
}

void CHUD::NewCurrentTeam()
{
	m_bAutoProceed = true;

	if (m_bHUDActive && DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentTeam()) &&
			DigitanksGame()->GetCurrentTank() && !DigitanksGame()->GetCurrentTank()->HasGoalMovePosition() &&
			CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial() != CInstructor::TUTORIAL_THEEND_BASICS)	// Don't set control mode if it's the end so we can open the menu.
		DigitanksGame()->SetControlMode(MODE_MOVE);
	else
		DigitanksGame()->SetControlMode(MODE_NONE);

	UpdateTeamInfo();

	if (DigitanksGame()->GetCurrentTeam() == DigitanksGame()->GetLocalDigitanksTeam())
		m_pPressEnter->SetText("Press <ENTER> to move and fire tanks");
	else
		m_pPressEnter->SetText("Other players are taking their turns...");

	if (DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentTeam()))
		m_flTurnInfoLerpGoal = 1;
	else
		m_flTurnInfoLerpGoal = 0;

	if (DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentTeam()))
	{
		// Don't open the research window on the first turn, give the player a chance to see the game grid first.
		if (DigitanksGame()->GetTurn() >= 1 && DigitanksGame()->GetUpdateGrid() && !DigitanksGame()->GetCurrentTeam()->GetUpdateDownloading())
		{
			bool bShouldOpen = false;
			for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
			{
				for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
				{
					if (DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y].m_eUpdateClass != UPDATECLASS_EMPTY && !DigitanksGame()->GetCurrentTeam()->HasDownloadedUpdate(x, y))
					{
						bShouldOpen = true;
						break;
					}
				}

				if (bShouldOpen)
					break;
			}
			
			if (bShouldOpen)
				m_pUpdatesPanel->SetVisible(true);
		}
	}

	UpdateScoreboard();

	CRootPanel::Get()->Layout();
}

void CHUD::NewCurrentSelection()
{
	UpdateInfo();

	if (DigitanksGame()->GetCurrentTeam() == DigitanksGame()->GetLocalDigitanksTeam())
	{
		if (DigitanksGame()->GetCurrentTank())
			Game()->GetCamera()->SetTarget(DigitanksGame()->GetCurrentTank()->GetDesiredMove());
		else if (DigitanksGame()->GetCurrentSelection())
			Game()->GetCamera()->SetTarget(DigitanksGame()->GetCurrentSelection()->GetOrigin());
	}

	SetupMenu(MENUMODE_MAIN);
}

void CHUD::OnTakeShieldDamage(CDigitank* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, true);

	if (!pVictim->IsAlive() && bDirectHit && flDamage > 0)
		new CHitIndicator(pVictim, L"OVERKILL!");

	else if (bShieldOnly && bDirectHit && flDamage > 0)
		new CHitIndicator(pVictim, L"DIRECT HIT!");
}

void CHUD::OnTakeDamage(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, false);

	if ((pVictim->IsAlive() || bKilled) && bDirectHit && flDamage > 0)
		new CHitIndicator(pVictim, L"DIRECT HIT!");
}

void CHUD::TankSpeak(class CDigitank* pTank, const std::string& sSpeech)
{
	if (pTank->GetVisibility() == 0)
		return;

	// Cleans itself up.
	new CSpeechBubble(pTank, sSpeech, m_iSpeechBubble);
}

void CHUD::ClearTurnInfo()
{
	if (CDigitanksGame::GetLocalDigitanksTeam() != DigitanksGame()->GetCurrentTeam())
		return;

	m_pTurnInfo->SetText("TURN REPORT\n \n");
}

void CHUD::AppendTurnInfo(const wchar_t* pszInfo)
{
	if (CDigitanksGame::GetLocalDigitanksTeam() != DigitanksGame()->GetCurrentTeam())
		return;

	m_pTurnInfo->AppendText("* ");
	m_pTurnInfo->AppendText(pszInfo);
	m_pTurnInfo->AppendText("\n");
}

void CHUD::SetHUDActive(bool bActive)
{
	m_bHUDActive = bActive;

	SetupMenu(m_eMenuMode);

	if (!bActive)
		DigitanksGame()->SetControlMode(MODE_NONE);
}

void CHUD::SetAutoProceed(bool bAuto)
{
	m_bAutoProceed = bAuto;

//	if (bAuto)
//		m_pAutoButton->SetText("Auto on");
//	else
//		m_pAutoButton->SetText("Auto off");
}

void CHUD::ShowButtonInfo(int iButton)
{
	if (iButton < 0 || iButton > 4)
		return;

	m_pButtonInfo->SetText(m_aszButtonInfos[iButton].c_str());
	m_pButtonInfo->SetSize(m_pButtonInfo->GetWidth(), 9999);
	m_pButtonInfo->SetSize(m_pButtonInfo->GetWidth(), (int)m_pButtonInfo->GetTextHeight());

	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	m_pButtonInfo->SetPos(iWidth/2 + 720/2 - m_pButtonInfo->GetWidth() - 50, iHeight - 160 - m_pButtonInfo->GetHeight());
}

void CHUD::HideButtonInfo()
{
	m_pButtonInfo->SetText("");
}

bool CHUD::IsUpdatesPanelOpen()
{
	if (!m_pUpdatesPanel)
		return false;

	return m_pUpdatesPanel->IsVisible();
}

void CHUD::Button1CursorInCallback()
{
	ShowButtonInfo(0);
}

void CHUD::Button1CursorOutCallback()
{
	HideButtonInfo();
}

void CHUD::Button2CursorInCallback()
{
	ShowButtonInfo(1);
}

void CHUD::Button2CursorOutCallback()
{
	HideButtonInfo();
}

void CHUD::Button3CursorInCallback()
{
	ShowButtonInfo(2);
}

void CHUD::Button3CursorOutCallback()
{
	HideButtonInfo();
}

void CHUD::Button4CursorInCallback()
{
	ShowButtonInfo(3);
}

void CHUD::Button4CursorOutCallback()
{
	HideButtonInfo();
}

void CHUD::Button5CursorInCallback()
{
	ShowButtonInfo(4);
}

void CHUD::Button5CursorOutCallback()
{
	HideButtonInfo();
}

void CHUD::EndTurnCallback()
{
	if (CDigitanksGame::GetLocalDigitanksTeam() != DigitanksGame()->GetCurrentTeam())
		return;

	CSoundLibrary::PlaySound(NULL, "sound/turn.wav");
	DigitanksGame()->EndTurn();
}

void CHUD::OpenUpdatesCallback()
{
	if (m_pUpdatesPanel)
		m_pUpdatesPanel->SetVisible(true);
}

void CHUD::AutoCallback()
{
	if (!ShouldAutoProceed())
	{
		CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentTeam();
		for (size_t t = 0; t < pTeam->GetNumTanks(); t++)
		{
			CDigitank* pTank = pTeam->GetTank(t);
			pTank->CancelDesiredMove();
			pTank->CancelDesiredAim();
		}

		DigitanksGame()->SetControlMode(MODE_MOVE);
	}

	SetAutoProceed(!ShouldAutoProceed());
}

void CHUD::MoveCallback()
{
	if (!m_bHUDActive)
		return;

	m_bAutoProceed = false;

	if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		DigitanksGame()->SetControlMode(MODE_NONE);
	else
		DigitanksGame()->SetControlMode(MODE_MOVE);

	SetupMenu();
}

void CHUD::TurnCallback()
{
	if (!m_bHUDActive)
		return;

	m_bAutoProceed = false;

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
		DigitanksGame()->SetControlMode(MODE_NONE);
	else
		DigitanksGame()->SetControlMode(MODE_TURN);

	SetupMenu();
}

void CHUD::AimCallback()
{
	if (!m_bHUDActive)
		return;

	m_bAutoProceed = false;

	if (DigitanksGame()->GetControlMode() == MODE_AIM)
		DigitanksGame()->SetControlMode(MODE_NONE);
	else
		DigitanksGame()->SetControlMode(MODE_AIM);

	SetupMenu();
}

void CHUD::FortifyCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	DigitanksGame()->GetCurrentTank()->Fortify();
	DigitanksGame()->SetControlMode(MODE_NONE);
	SetupMenu(MENUMODE_MAIN);
	UpdateInfo();
}

void CHUD::FireCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	m_bAutoProceed = false;

	if (DigitanksGame()->GetControlMode() == MODE_FIRE)
		DigitanksGame()->SetControlMode(MODE_NONE);
	else
		DigitanksGame()->SetControlMode(MODE_FIRE);

	SetupMenu();
}

void CHUD::PromoteCallback()
{
	if (!m_bHUDActive)
		return;

	SetupMenu(MENUMODE_PROMOTE);
}

void CHUD::PromoteAttackCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	pTank->PromoteAttack();

	SetupMenu(MENUMODE_MAIN);

	UpdateInfo();

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
}

void CHUD::PromoteDefenseCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	pTank->PromoteDefense();

	SetupMenu(MENUMODE_MAIN);

	UpdateInfo();

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
}

void CHUD::PromoteMovementCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	pTank->PromoteMovement();

	SetupMenu(MENUMODE_MAIN);

	UpdateInfo();

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
}

void CHUD::BuildMiniBufferCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_MINIBUFFER);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::BuildBufferCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_BUFFER);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::BuildBatteryCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_BATTERY);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::BuildPSUCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_PSU);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::BuildLoaderCallback()
{
	if (!m_bHUDActive)
		return;

	SetupMenu(MENUMODE_LOADERS);
}

void CHUD::BuildInfantryLoaderCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_INFANTRYLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::BuildTankLoaderCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_TANKLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::BuildArtilleryLoaderCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_ARTILLERYLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::CancelBuildCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->CancelConstruction();

	DigitanksGame()->SetControlMode(MODE_NONE);

	SetupMenu();
}

void CHUD::BuildUnitCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	CLoader* pLoader = dynamic_cast<CLoader*>(pStructure);
	if (!pLoader)
		return;

	pLoader->BeginProduction();
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::CancelBuildUnitCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	CLoader* pLoader = dynamic_cast<CLoader*>(pStructure);
	if (!pLoader)
		return;

	pLoader->CancelProduction();
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::InstallMenuCallback()
{
	if (!m_bHUDActive)
		return;

	SetupMenu(MENUMODE_INSTALL);
}

void CHUD::InstallProductionCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	pStructure->InstallUpdate(UPDATETYPE_PRODUCTION);
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::InstallBandwidthCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	pStructure->InstallUpdate(UPDATETYPE_BANDWIDTH);
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::InstallFleetSupplyCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	pStructure->InstallUpdate(UPDATETYPE_FLEETSUPPLY);
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::InstallEnergyBonusCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	pStructure->InstallUpdate(UPDATETYPE_SUPPORTENERGY);
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::InstallRechargeBonusCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	pStructure->InstallUpdate(UPDATETYPE_SUPPORTRECHARGE);
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::InstallTankAttackCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	pStructure->InstallUpdate(UPDATETYPE_TANKATTACK);
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::InstallTankDefenseCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	pStructure->InstallUpdate(UPDATETYPE_TANKDEFENSE);
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::InstallTankMovementCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	pStructure->InstallUpdate(UPDATETYPE_TANKMOVEMENT);
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::InstallTankHealthCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	pStructure->InstallUpdate(UPDATETYPE_TANKHEALTH);
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::InstallTankRangeCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();

	pStructure->InstallUpdate(UPDATETYPE_TANKRANGE);
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::CancelInstallCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();
	pStructure->CancelInstall();

	DigitanksGame()->SetControlMode(MODE_NONE);

	SetupMenu();
	UpdateInfo();
}

void CHUD::BeginUpgradeCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();
	pStructure->BeginUpgrade();

	DigitanksGame()->SetControlMode(MODE_NONE);

	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::CancelUpgradeCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetCurrentStructure();
	pStructure->CancelUpgrade();

	DigitanksGame()->SetControlMode(MODE_NONE);

	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::GoToMainCallback()
{
	SetupMenu(MENUMODE_MAIN);
}

CDamageIndicator::CDamageIndicator(CBaseEntity* pVictim, float flDamage, bool bShield)
	: CLabel(0, 0, 100, 100, "")
{
	m_hVictim = pVictim;
	m_flDamage = flDamage;
	m_bShield = bShield;
	m_flTime = DigitanksGame()->GetGameTime();
	m_vecLastOrigin = pVictim->GetOrigin();

	glgui::CRootPanel::Get()->AddControl(this, true);

	Vector vecScreen = Game()->GetRenderer()->ScreenPosition(pVictim->GetOrigin());
	if (bShield)
		vecScreen += Vector(10, 10, 0);
	SetPos((int)vecScreen.x, (int)vecScreen.y);

	if (m_bShield)
		SetFGColor(Color(255, 255, 255));
	else if (flDamage < 0)
		SetFGColor(Color(0, 255, 0));
	else
		SetFGColor(Color(255, 0, 0));

	int iDamage = (int)flDamage;

	// Don't let it say 0
	if (flDamage > 0 && iDamage < 1)
		iDamage = 1;
	if (flDamage < 0 && iDamage > -1)
		iDamage = -1;

	if (flDamage < 0.5f && flDamage > -0.1f)
	{
		m_flTime = 0;
		SetVisible(false);
	}

	char szDamage[100];
	if (iDamage < 0)
		sprintf(szDamage, "+%d", -iDamage);
	else
		sprintf(szDamage, "-%d", iDamage);
	SetText(szDamage);

	SetFontFaceSize(18);
	SetAlign(CLabel::TA_TOPLEFT);
}

void CDamageIndicator::Destructor()
{
	glgui::CRootPanel::Get()->RemoveControl(this);
}

void CDamageIndicator::Think()
{
	float flFadeTime = 1.0f;

	if (DigitanksGame()->GetGameTime() - m_flTime > flFadeTime)
	{
		Destructor();
		Delete();
		return;
	}

	if (m_hVictim != NULL)
		m_vecLastOrigin = m_hVictim->GetOrigin();

	float flOffset = RemapVal(DigitanksGame()->GetGameTime() - m_flTime, 0, flFadeTime, 10, 20);

	Vector vecScreen = Game()->GetRenderer()->ScreenPosition(m_vecLastOrigin);
	if (m_bShield)
		vecScreen += Vector(10, 10, 0);
	SetPos((int)(vecScreen.x+flOffset), (int)(vecScreen.y-flOffset));

	SetAlpha((int)RemapVal(DigitanksGame()->GetGameTime() - m_flTime, 0, flFadeTime, 255, 0));

	BaseClass::Think();
}

void CDamageIndicator::Paint(int x, int y, int w, int h)
{
	if (m_bShield)
	{
		int iWidth = GetTextWidth();
		int iHeight = (int)GetTextHeight();
		CRootPanel::PaintRect(x, y-iHeight/2, iWidth, iHeight, Color(0, 0, 0, GetAlpha()/2));
	}

	BaseClass::Paint(x, y, w, h);
}

CHitIndicator::CHitIndicator(CBaseEntity* pVictim, std::wstring sMessage)
	: CLabel(0, 0, 200, 100, "")
{
	m_hVictim = pVictim;
	m_flTime = DigitanksGame()->GetGameTime();
	m_vecLastOrigin = pVictim->GetOrigin();

	glgui::CRootPanel::Get()->AddControl(this, true);

	Vector vecScreen = Game()->GetRenderer()->ScreenPosition(pVictim->GetOrigin());
	vecScreen.x -= 20;
	vecScreen.y -= 20;
	SetPos((int)vecScreen.x, (int)vecScreen.y);

	SetFGColor(Color(255, 255, 255));

	SetText(sMessage.c_str());
	SetWrap(false);

	SetFontFaceSize(20);
	SetAlign(CLabel::TA_TOPLEFT);
}

void CHitIndicator::Destructor()
{
	glgui::CRootPanel::Get()->RemoveControl(this);
}

void CHitIndicator::Think()
{
	float flFadeTime = 1.0f;

	if (DigitanksGame()->GetGameTime() - m_flTime > flFadeTime)
	{
		Destructor();
		Delete();
		return;
	}

	if (m_hVictim != NULL)
		m_vecLastOrigin = m_hVictim->GetOrigin();

	float flOffset = RemapVal(DigitanksGame()->GetGameTime() - m_flTime, 0, flFadeTime, 10, 20);

	Vector vecScreen = Game()->GetRenderer()->ScreenPosition(m_vecLastOrigin);
	vecScreen.x -= 20;
	vecScreen.y -= 20;

	SetPos((int)(vecScreen.x+flOffset), (int)(vecScreen.y-flOffset));

	SetAlpha((int)RemapVal(DigitanksGame()->GetGameTime() - m_flTime, 0, flFadeTime, 255, 0));

	BaseClass::Think();
}

void CHitIndicator::Paint(int x, int y, int w, int h)
{
	int iWidth = GetTextWidth();
	int iHeight = (int)GetTextHeight();
	CRootPanel::PaintRect(x, y-iHeight/2, iWidth, iHeight, Color(0, 0, 0, GetAlpha()/2));

	BaseClass::Paint(x, y, w, h);
}

CSpeechBubble::CSpeechBubble(CDigitank* pSpeaker, std::string sSpeech, size_t iBubble)
	: CLabel(0, 0, 83*2/3, 47*2/3, "")
{
	m_hSpeaker = pSpeaker;
	m_flTime = DigitanksGame()->GetGameTime();
	m_vecLastOrigin = pSpeaker->GetOrigin();
	m_iBubble = iBubble;

	glgui::CRootPanel::Get()->AddControl(this, true);

	SetFGColor(Color(255, 255, 255));

	SetText(sSpeech.c_str());
	SetWrap(false);

	SetFontFaceSize(18);
	SetAlign(CLabel::TA_MIDDLECENTER);
}

void CSpeechBubble::Destructor()
{
	glgui::CRootPanel::Get()->RemoveControl(this);
}

void CSpeechBubble::Think()
{
	float flFadeTime = 3.0f;

	if (DigitanksGame()->GetGameTime() - m_flTime > flFadeTime)
	{
		Destructor();
		Delete();
		return;
	}

	if (m_hSpeaker != NULL)
		m_vecLastOrigin = m_hSpeaker->GetDesiredMove();

	Vector vecScreen = Game()->GetRenderer()->ScreenPosition(m_vecLastOrigin);
	vecScreen.x += 40;
	vecScreen.y -= 70;

	SetPos((int)(vecScreen.x), (int)(vecScreen.y));

	SetAlpha((int)RemapValClamped(DigitanksGame()->GetGameTime() - m_flTime, flFadeTime-1, flFadeTime, 255, 0));

	BaseClass::Think();
}

void CSpeechBubble::Paint(int x, int y, int w, int h)
{
	if (CDigitanksWindow::Get()->GetHUD()->IsUpdatesPanelOpen())
		return;

	do {
		CRenderingContext c(Game()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);
		CRootPanel::PaintTexture(m_iBubble, x, y, w, h, Color(255, 255, 255, GetAlpha()));
	} while (false);

	BaseClass::Paint(x, y, w, h);
}
