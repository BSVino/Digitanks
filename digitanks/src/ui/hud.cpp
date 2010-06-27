#include "hud.h"

#include "digitankswindow.h"
#include "digitanks/digitanksgame.h"
#include "debugdraw.h"
#include "instructor.h"
#include "game/camera.h"
#include "renderer/renderer.h"
#include <game/digitanks/cpu.h>

// windows.h screws up virtual functions in some of the above headers
#include <GL/glew.h>
#include <GL/freeglut.h>

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
		sprintf(szLabel, "Health: %.1f/%.1f", pSelection->GetHealth(), pSelection->GetTotalHealth());
		SetText(szLabel);
	}
	else if (m_ePowerbarType == POWERBAR_ATTACK)
	{
		sprintf(szLabel, "%s: %d%%", pSelection->GetPowerBar1Text(), (int)(pSelection->GetPowerBar1Value()*100));
		SetText(szLabel);
	}
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
	{
		sprintf(szLabel, "%s: %d%%", pSelection->GetPowerBar2Text(), (int)(pSelection->GetPowerBar2Value()*100));
		SetText(szLabel);
	}
	else
	{
		sprintf(szLabel, "%s: %d%%", pSelection->GetPowerBar3Text(), (int)(pSelection->GetPowerBar3Value()*100));
		SetText(szLabel);
	}
}

void CPowerBar::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentSelection())
		return;

	CSelectable* pSelection = DigitanksGame()->GetCurrentSelection();

	CRootPanel::PaintRect(x, y, w, h, Color(255, 255, 255, 128));

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

	m_pAutoButton = new CButton(0, 0, 0, 0, "Auto");
	m_pAutoButton->SetClickedListener(this, Auto);
	AddControl(m_pAutoButton);

	m_pButton1 = new CPictureButton("");
	AddControl(m_pButton1);

	m_pButton2 = new CPictureButton("");
	AddControl(m_pButton2);

	m_pButton3 = new CPictureButton("");
	AddControl(m_pButton3);

	m_pButton4 = new CPictureButton("");
	AddControl(m_pButton4);

	m_pButton5 = new CPictureButton("");
	AddControl(m_pButton5);

	m_pButtonHelp1 = new CLabel(0, 0, 50, 50, "");
	AddControl(m_pButtonHelp1);

	m_pButtonHelp2 = new CLabel(0, 0, 50, 50, "");
	AddControl(m_pButtonHelp2);

	m_pButtonHelp3 = new CLabel(0, 0, 50, 50, "");
	AddControl(m_pButtonHelp3);

	m_pButtonHelp4 = new CLabel(0, 0, 50, 50, "");
	AddControl(m_pButtonHelp4);

	m_pButtonHelp5 = new CLabel(0, 0, 50, 50, "");
	AddControl(m_pButtonHelp5);

	m_pFireAttack = new CLabel(0, 0, 50, 50, "");
	m_pFireDefend = new CLabel(0, 0, 50, 50, "");
	AddControl(m_pFireAttack);
	AddControl(m_pFireDefend);

	m_pAttackInfo = new CLabel(0, 0, 100, 150, "");
	m_pAttackInfo->SetWrap(false);
	m_pAttackInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pAttackInfo);

	m_pLowShieldsWarning = new CLabel(0, 0, 100, 150, "");
	m_pLowShieldsWarning->SetWrap(false);
	m_pLowShieldsWarning->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	AddControl(m_pLowShieldsWarning);

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

	m_pPressEnter = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pPressEnter);

	SetupMenu(MENUMODE_MAIN);

	m_pOpenTutorial = new CLabel(0, 0, 100, 20, "Press 't' to view the tutorial");
	AddControl(m_pOpenTutorial);

	m_pFPS = new CLabel(0, 0, 100, 20, "");
	AddControl(m_pFPS);

	m_pFPS->SetAlign(CLabel::TA_TOPLEFT);
	m_pFPS->SetPos(20, 20);

	m_iAvatarIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/tank-avatar.png");
	m_iShieldIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/tank-avatar-shield.png");

	m_iSpeechBubble = CRenderer::LoadTextureIntoGL(L"textures/hud/bubble.png");

	//m_iCompetitionWatermark = CRenderer::LoadTextureIntoGL(L"textures/competition.png");
}

void CHUD::Layout()
{
	SetSize(GetParent()->GetWidth(), GetParent()->GetHeight());

	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	m_pAttackInfo->SetPos(iWidth/2 - 1024/2 + 190 + 3, iHeight - 150 - 10 - 80 + 3);
	m_pAttackInfo->SetSize(200, 80);

	m_pLowShieldsWarning->SetPos(iWidth/2 - 1024/2 + 590, iHeight - 150 - 10 - 80);
	m_pLowShieldsWarning->SetSize(200, 80);
	m_pLowShieldsWarning->SetText("WARNING: Shields Low!\nIncrease Defense Power");

	m_pHealthBar->SetPos(iWidth/2 - 1024/2 + 470, iHeight - 140);
	m_pHealthBar->SetSize(200, 20);

	m_pAttackPower->SetPos(iWidth/2 - 1024/2 + 470, iHeight - 90);
	m_pAttackPower->SetSize(200, 20);

	m_pDefensePower->SetPos(iWidth/2 - 1024/2 + 470, iHeight - 60);
	m_pDefensePower->SetSize(200, 20);

	m_pMovementPower->SetPos(iWidth/2 - 1024/2 + 470, iHeight - 30);
	m_pMovementPower->SetSize(200, 20);

	m_pButton1->SetSize(50, 50);
	m_pButton2->SetSize(50, 50);
	m_pButton3->SetSize(50, 50);
	m_pButton4->SetSize(50, 50);
	m_pButton5->SetSize(50, 50);

	m_pAutoButton->SetPos(iWidth/2 - 1024/2 + 820, iHeight - 135);
	m_pAutoButton->SetSize(50, 20);

	m_pButton1->SetPos(iWidth/2 - 1024/2 + 700, iHeight - 100);
	m_pButton2->SetPos(iWidth/2 - 1024/2 + 760, iHeight - 100);
	m_pButton3->SetPos(iWidth/2 - 1024/2 + 820, iHeight - 100);
	m_pButton4->SetPos(iWidth/2 - 1024/2 + 880, iHeight - 100);
	m_pButton5->SetPos(iWidth/2 - 1024/2 + 940, iHeight - 100);

	m_pButtonHelp1->SetPos(iWidth/2 - 1024/2 + 700, iHeight - 50);
	m_pButtonHelp2->SetPos(iWidth/2 - 1024/2 + 760, iHeight - 50);
	m_pButtonHelp3->SetPos(iWidth/2 - 1024/2 + 820, iHeight - 50);
	m_pButtonHelp4->SetPos(iWidth/2 - 1024/2 + 880, iHeight - 50);
	m_pButtonHelp5->SetPos(iWidth/2 - 1024/2 + 940, iHeight - 50);
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

	m_pLeftShieldInfo->SetDimensions(iWidth/2 - 1024/2 + 190 + 150/2 - 50/2 - 40, iHeight - 150 + 10 + 130/2 - 50/2, 10, 50);
	m_pRightShieldInfo->SetDimensions(iWidth/2 - 1024/2 + 190 + 150/2 + 50/2 + 30, iHeight - 150 + 10 + 130/2 - 50/2, 10, 50);
	m_pRearShieldInfo->SetDimensions(iWidth/2 - 1024/2 + 190 + 150/2 - 50/2, iHeight - 150 + 10 + 130/2 + 50/2 + 25, 50, 10);
	m_pFrontShieldInfo->SetDimensions(iWidth/2 - 1024/2 + 190 + 150/2 - 50/2, iHeight - 150 + 10 + 130/2 - 50/2 - 35, 50, 10);

	m_pLeftShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pRightShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pRearShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pFrontShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);

	m_pLeftShieldInfo->SetWrap(false);
	m_pRightShieldInfo->SetWrap(false);
	m_pRearShieldInfo->SetWrap(false);
	m_pFrontShieldInfo->SetWrap(false);

	m_pTankInfo->SetDimensions(iWidth/2 - 1024/2 + 350 + 2, iHeight - 150 + 10 + 7, 96, 126);
	m_pTankInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pTankInfo->SetWrap(true);
	m_pTankInfo->SetFontFaceSize(10);

	m_pPressEnter->SetDimensions(iWidth/2 - 100/2, iHeight*2/3, 100, 50);
	m_pPressEnter->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pPressEnter->SetWrap(false);
	m_pPressEnter->SetText("Press <ENTER> to move and fire tanks");

	m_pOpenTutorial->SetPos(iWidth/2 - 100/2, 0);
	m_pOpenTutorial->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pOpenTutorial->SetWrap(false);
}

void CHUD::Think()
{
	if (Game()->IsLoading())
		return;

	BaseClass::Think();

	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	Vector vecPoint;
	bool bMouseOnGrid = CDigitanksWindow::Get()->GetMouseGridPosition(vecPoint);

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
			pCurrentTank->SetPreviewAim(vecPoint);
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

	if (m_bHUDActive && pCurrentTank && pCurrentTank->GetDefenseScale(true) < 0.3f)
	{
		m_pLowShieldsWarning->SetVisible(true);
		int c = (int)RemapVal(fabs((float)(glutGet(GLUT_ELAPSED_TIME)%1000-500)/500), 0, 1, 128, 255);
		m_pLowShieldsWarning->SetFGColor(Color(255, c, 0, c));
	}
	else
	{
		m_pLowShieldsWarning->SetVisible(false);
	}

	if (m_bHUDActive)
	{
		if (ShouldAutoProceed())
		{
			m_pAutoButton->SetText("Auto on");
			m_pAutoButton->SetButtonColor(Color(0, 0, 150));
		}
		else
		{
			m_pAutoButton->SetText("Auto off");
			m_pAutoButton->SetButtonColor(Color(100, 100, 100));
		}
	}
	else
	{
		m_pAutoButton->SetText("Auto off");
		m_pAutoButton->SetButtonColor(Color(100, 100, 100));
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

	if (pCurrentTank)
	{
		bool bShowEnter = true;
		CDigitanksTeam* pTeam = pCurrentTank->GetDigitanksTeam();
		for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
		{
			CDigitank* pTank = pTeam->GetTank(i);
			if (!pTank)
				continue;

			if (!pTank->IsAlive())
				continue;

			if (pTank->HasDesiredMove() || pTank->HasDesiredTurn() || pTank->HasDesiredAim())
				continue;

			bShowEnter = false;
		}

		m_pPressEnter->SetVisible(bShowEnter);

		m_pFireAttack->SetVisible(DigitanksGame()->GetControlMode() == MODE_FIRE);
		m_pFireDefend->SetVisible(DigitanksGame()->GetControlMode() == MODE_FIRE);
		m_pFireAttack->SetAlign(CLabel::TA_MIDDLECENTER);
		m_pFireDefend->SetAlign(CLabel::TA_MIDDLECENTER);
		m_pFireAttack->SetWrap(false);
		m_pFireDefend->SetWrap(false);
	}

	m_pOpenTutorial->SetVisible(!CDigitanksWindow::Get()->GetInstructor()->GetActive());

	m_pFPS->SetText(L"Free Demo");

#ifdef _DEBUG
	char szFPS[100];
	sprintf(szFPS, "\n%d fps", (int)(1/Game()->GetFrameTime()));
	m_pFPS->AppendText(szFPS);
#endif
}

void CHUD::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	if (Game()->IsLoading())
		return;

	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

#ifdef _DEBUG
	// Nobody runs resolutions under 1024x768 anymore.
	// Show me my constraints!
	CRootPanel::PaintRect(iWidth/2 - 1024/2, iHeight - 150, 1024, 200, Color(255, 255, 255, 100));

	// This is where the minimap will be.
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 10, iHeight - 150 - 30, 170, 170, Color(0, 0, 0, 100));
#endif

	// Shield schematic
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 190, iHeight - 150 + 10, 150, 130, Color(0, 0, 0, 100));

	// Tank data
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 350, iHeight - 150 + 10, 100, 130, Color(0, 0, 0, 100));

	// Background for the attack info label
	CRootPanel::PaintRect(m_pAttackInfo->GetLeft()-3, m_pAttackInfo->GetTop()-9, m_pAttackInfo->GetWidth()+6, m_pAttackInfo->GetHeight()+6, Color(0, 0, 0, 100));

	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
	{
		CDigitanksTeam* pTeam = DigitanksGame()->GetDigitanksTeam(i);
		for (size_t j = 0; j < pTeam->GetNumTanks(); j++)
		{
			CDigitank* pTank = pTeam->GetTank(j);

			Vector vecOrigin = pTank->GetDesiredMove();
			Vector vecScreen = Game()->GetRenderer()->ScreenPosition(vecOrigin);

			if (!CDigitanksWindow::Get()->IsAltDown() && pTank->GetTeam() != DigitanksGame()->GetCurrentTeam())
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

			if (m_bHUDActive && DigitanksGame()->IsCurrentSelection(pTank) && DigitanksGame()->GetControlMode() == MODE_FIRE)
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

		CRootPanel::PaintTexture(m_iAvatarIcon, iWidth/2 - 1024/2 + 190 + 150/2 - 50/2, iHeight - 150 + 10 + 130/2 - 50/2, 50, 50, pTank->GetTeam()->GetColor());

		c.SetBlend(BLEND_ADDITIVE);

		c.Translate(Vector((float)(iWidth/2 - 1024/2 + 190 + 150/2), (float)(iHeight - 150 + 10 + 130/2), 0));

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

	CSelectable* pSelection = DigitanksGame()->GetCurrentSelection();

	if (m_bHUDActive && pSelection)
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

	m_pTankInfo->SetText("TANK INFO");

	if (pTank->IsFortified() || pTank->IsFortifying())
		m_pTankInfo->AppendText("\n \n[Fortified]");

	if (pTank->HasBonusPoints())
	{
		if (pTank->GetBonusPoints() > 1)
			sprintf(szShieldInfo, "\n \n%d bonus points available", pTank->GetBonusPoints());
		else
			sprintf(szShieldInfo, "\n \n1 bonus point available");
		m_pTankInfo->AppendText(szShieldInfo);
	}

	if (pTank->GetBonusAttackPower())
	{
		sprintf(szShieldInfo, "\n \n+%d attack power", (int)pTank->GetBonusAttackPower());
		m_pTankInfo->AppendText(szShieldInfo);

		if (pTank->IsFortified() && (int)pTank->GetFortifyAttackPowerBonus() > 0)
		{
			sprintf(szShieldInfo, "\n (+%d from fortify)", (int)pTank->GetFortifyAttackPowerBonus());
			m_pTankInfo->AppendText(szShieldInfo);
		}
	}

	if (pTank->GetBonusDefensePower())
	{
		sprintf(szShieldInfo, "\n \n+%d defense power", (int)pTank->GetBonusDefensePower());
		m_pTankInfo->AppendText(szShieldInfo);

		if (pTank->IsFortified() && (int)pTank->GetFortifyDefensePowerBonus() > 0)
		{
			sprintf(szShieldInfo, "\n (+%d from fortify)", (int)pTank->GetFortifyDefensePowerBonus());
			m_pTankInfo->AppendText(szShieldInfo);
		}
	}

	if (pTank->GetBonusMovementPower())
	{
		sprintf(szShieldInfo, "\n \n+%d movement power", (int)pTank->GetBonusMovementPower());
		m_pTankInfo->AppendText(szShieldInfo);
	}

	m_pAttackInfo->SetText(L"No targets");

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
		"Shield Damage: %.1f/%.1f\n"
		"Digitank Damage: %.1f/%.1f\n",
		flShieldDamage, flShieldStrength * pClosestTarget->GetDefenseScale(true),
		flTankDamage, pClosestTarget->GetHealth()
	);

	m_pAttackInfo->SetText(szAttackInfo);
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
	if (!IsActive() || !DigitanksGame()->GetCurrentSelection())
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
		return;
	}

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

void CHUD::GameStart()
{
	DigitanksGame()->SetControlMode(MODE_NONE);
	CDigitanksWindow::Get()->GetInstructor()->Initialize();
	CDigitanksWindow::Get()->GetInstructor()->DisplayFirstTutorial();

	m_pOpenTutorial->SetText(L"Press 't' to view the tutorial");
}

void CHUD::GameOver(bool bPlayerWon)
{
	CDigitanksWindow::Get()->GameOver(bPlayerWon);

	m_pOpenTutorial->SetText(L"Press 'Esc' to restart the game");
}

void CHUD::NewCurrentTeam()
{
	m_bAutoProceed = true;

	if (m_bHUDActive && DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentTeam()))
		DigitanksGame()->SetControlMode(MODE_MOVE);
	else
		DigitanksGame()->SetControlMode(MODE_NONE);

	if (DigitanksGame()->GetCurrentSelection())
		Game()->GetCamera()->SetTarget(DigitanksGame()->GetCurrentSelection()->GetOrigin());
}

void CHUD::NewCurrentSelection()
{
	UpdateInfo();

	if (DigitanksGame()->GetCurrentTank())
		Game()->GetCamera()->SetTarget(DigitanksGame()->GetCurrentTank()->GetDesiredMove());
	else
		Game()->GetCamera()->SetTarget(DigitanksGame()->GetCurrentSelection()->GetOrigin());

	SetupMenu(MENUMODE_MAIN);
}

void CHUD::OnTakeShieldDamage(CDigitank* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly)
{
	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, true);

	if (!pVictim->IsAlive() && bDirectHit)
		new CHitIndicator(pVictim, L"OVERKILL!");

	else if (bShieldOnly && bDirectHit)
		new CHitIndicator(pVictim, L"DIRECT HIT!");
}

void CHUD::OnTakeDamage(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled)
{
	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, false);

	if ((pVictim->IsAlive() || bKilled) && bDirectHit)
		new CHitIndicator(pVictim, L"DIRECT HIT!");
}

void CHUD::TankSpeak(class CDigitank* pTank, const std::string& sSpeech)
{
	// Cleans itself up.
	new CSpeechBubble(pTank, sSpeech, m_iSpeechBubble);
}

void CHUD::SetHUDActive(bool bActive)
{
	m_bHUDActive = bActive;

	SetupMenu(m_eMenuMode);

	if (!bActive)
		DigitanksGame()->SetControlMode(MODE_NONE);
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
	else if (DigitanksGame()->GetCurrentTank() && !DigitanksGame()->GetCurrentTank()->HasDesiredAim())
		DigitanksGame()->SetControlMode(MODE_AIM);
	else
		DigitanksGame()->SetControlMode(MODE_FIRE);
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
	else
		SetFGColor(Color(255, 0, 0));

	int iDamage = (int)flDamage;
	if (flDamage > 0 && iDamage < 1)
		iDamage = 1;

	if (flDamage < 0.5f)
	{
		m_flTime = 0;
		SetVisible(false);
	}

	char szDamage[100];
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
	do {
		CRenderingContext c(Game()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);
		CRootPanel::PaintTexture(m_iBubble, x, y, w, h, Color(255, 255, 255, GetAlpha()));
	} while (false);

	BaseClass::Paint(x, y, w, h);
}
