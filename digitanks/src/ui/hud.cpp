#include "hud.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "digitankswindow.h"
#include "game/digitanksgame.h"
#include "debugdraw.h"
#include "instructor.h"
#include "game/camera.h"
#include "renderer/renderer.h"

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

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	char szLabel[100];
	if (m_ePowerbarType == POWERBAR_HEALTH)
	{
		sprintf(szLabel, "Health: %.1f/%.1f", pTank->GetHealth(), pTank->GetTotalHealth());
		SetText(szLabel);
	}
	else if (m_ePowerbarType == POWERBAR_ATTACK)
	{
		sprintf(szLabel, "Attack Power: %d%%", (int)(pTank->GetAttackPower(true)/pTank->GetBasePower()*100));
		SetText(szLabel);
	}
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
	{
		sprintf(szLabel, "Defense Power: %d%%", (int)(pTank->GetDefensePower(true)/pTank->GetBasePower()*100));
		SetText(szLabel);
	}
	else
	{
		sprintf(szLabel, "Movement Power: %d%%", (int)(pTank->GetMovementPower(true)/pTank->GetBasePower()*100));
		SetText(szLabel);
	}
}

void CPowerBar::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	CRootPanel::PaintRect(x, y, w, h, Color(255, 255, 255, 128));

	if (m_ePowerbarType == POWERBAR_HEALTH)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetHealth() / pTank->GetTotalHealth())-2, h-2, Color(0, 150, 0));
	else if (m_ePowerbarType == POWERBAR_ATTACK)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetAttackPower(true) / pTank->GetTotalAttackPower())-2, h-2, Color(150, 0, 0));
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetDefensePower(true) / pTank->GetTotalDefensePower())-2, h-2, Color(0, 0, 150));
	else
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetMovementPower(true) / pTank->GetTotalMovementPower())-2, h-2, Color(100, 100, 0));

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

	m_pButton1 = new CButton(0, 0, 50, 50, "");
	AddControl(m_pButton1);

	m_pButton2 = new CButton(0, 0, 50, 50, "");
	AddControl(m_pButton2);

	m_pButton3 = new CButton(0, 0, 50, 50, "");
	AddControl(m_pButton3);

	m_pButton4 = new CButton(0, 0, 50, 50, "");
	AddControl(m_pButton4);

	m_pButton5 = new CButton(0, 0, 50, 50, "");
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

#ifdef _DEBUG
	m_pFPS = new CLabel(0, 0, 100, 20, "");
	AddControl(m_pFPS);
#endif
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
}

void CHUD::Think()
{
	BaseClass::Think();

	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	Vector vecPoint;
	bool bMouseOnGrid = CDigitanksWindow::Get()->GetMouseGridPosition(vecPoint);

	if (bMouseOnGrid)
	{
		if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE || CDigitanksWindow::Get()->GetControlMode() == MODE_TURN || CDigitanksWindow::Get()->GetControlMode() == MODE_AIM)
			UpdateAttackInfo();

		if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
			pCurrentTank->SetPreviewMove(vecPoint);

		if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
		{
			if ((vecPoint - pCurrentTank->GetDesiredMove()).LengthSqr() > 3*3)
			{
				Vector vecTurn = vecPoint - pCurrentTank->GetDesiredMove();
				vecTurn.Normalize();
				float flTurn = atan2(vecTurn.z, vecTurn.x) * 180/M_PI;
				pCurrentTank->SetPreviewTurn(flTurn);
			}
			else
				pCurrentTank->SetPreviewTurn(pCurrentTank->GetAngles().y);
		}

		if (CDigitanksWindow::Get()->GetControlMode() == MODE_AIM)
			pCurrentTank->SetPreviewAim(vecPoint);
	}

	if (pCurrentTank && pCurrentTank->GetDefenseScale(true) < 0.3f)
	{
		m_pLowShieldsWarning->SetVisible(true);
		int c = (int)RemapVal(fabs((float)(glutGet(GLUT_ELAPSED_TIME)%1000-500)/500), 0, 1, 128, 255);
		m_pLowShieldsWarning->SetFGColor(Color(255, c, 0, c));
	}
	else
	{
		m_pLowShieldsWarning->SetVisible(false);
	}

	if (m_eMenuMode == MENUMODE_MAIN)
	{
		if (m_bAutoProceed)
		{
			m_pButton1->SetText("Auto");
			m_pButton2->SetText("Auto");
			m_pButton3->SetText("Auto");
			m_pButton4->SetText("Auto");
		}
		else
		{
			m_pButton1->SetText("");
			m_pButton2->SetText("");
			m_pButton3->SetText("");
			m_pButton4->SetText("");
		}

		if (!CDigitanksWindow::Get()->GetControlMode() || CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
			m_pButton1->SetButtonColor(Color(150, 150, 0));
		else
			m_pButton1->SetButtonColor(Color(100, 100, 100));

		if (!CDigitanksWindow::Get()->GetControlMode() || CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
			m_pButton2->SetButtonColor(Color(150, 150, 0));
		else
			m_pButton2->SetButtonColor(Color(100, 100, 100));

		if (!CDigitanksWindow::Get()->GetControlMode() || CDigitanksWindow::Get()->GetControlMode() == MODE_AIM)
			m_pButton3->SetButtonColor(Color(150, 0, 0));
		else
			m_pButton3->SetButtonColor(Color(100, 100, 100));

		if (!CDigitanksWindow::Get()->GetControlMode() || CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
			m_pButton4->SetButtonColor(Color(150, 0, 150));
		else
			m_pButton4->SetButtonColor(Color(100, 100, 100));

		if (pCurrentTank && pCurrentTank->HasBonusPoints())
		{
			float flRamp = 1;
			if (!CDigitanksWindow::Get()->GetInstructor()->GetActive() || CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial() >= CInstructor::TUTORIAL_UPGRADE)
				flRamp = fabs(fmod(DigitanksGame()->GetGameTime(), 2)-1);
			m_pButton5->SetButtonColor(Color((int)RemapVal(flRamp, 0, 1, 0, 250), (int)RemapVal(flRamp, 0, 1, 0, 200), 0));
		}
		else
			m_pButton5->SetButtonColor(g_clrBox);
	}
	else if (m_eMenuMode == MENUMODE_PROMOTE)
	{
		m_pButton1->SetText("");
		m_pButton2->SetText("");
		m_pButton3->SetText("");
		m_pButton4->SetText("");

		if (pCurrentTank && pCurrentTank->HasBonusPoints())
		{
			m_pButton1->SetButtonColor(Color(200, 0, 0));
			m_pButton2->SetButtonColor(Color(0, 0, 200));
			m_pButton3->SetButtonColor(Color(200, 200, 0));
		}
		else
		{
			m_pButton1->SetButtonColor(g_clrBox);
			m_pButton2->SetButtonColor(g_clrBox);
			m_pButton3->SetButtonColor(g_clrBox);
		}
		m_pButton4->SetButtonColor(g_clrBox);
		m_pButton5->SetButtonColor(Color(100, 0, 0));
	}

	if (pCurrentTank)
	{
		bool bShowEnter = true;
		CTeam* pTeam = pCurrentTank->GetTeam();
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

		m_pFireAttack->SetVisible(CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE);
		m_pFireDefend->SetVisible(CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE);
		m_pFireAttack->SetAlign(CLabel::TA_MIDDLECENTER);
		m_pFireDefend->SetAlign(CLabel::TA_MIDDLECENTER);
		m_pFireAttack->SetWrap(false);
		m_pFireDefend->SetWrap(false);
	}

#ifdef _DEBUG
	char szFPS[100];
	sprintf(szFPS, "%d fps", (int)(1/Game()->GetFrameTime()));
	m_pFPS->SetText(szFPS);
#endif
}

void CHUD::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

#ifdef _DEBUG
	// Nobody runs resolutions under 1024x768 anymore.
	// Show me my constraints!
	CRootPanel::PaintRect(iWidth/2 - 1024/2, iHeight - 150, 1024, 200, Color(255, 255, 255, 100));
#endif

	// This is where the minimap will be.
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 10, iHeight - 150 - 30, 170, 170, Color(0, 0, 0, 100));

	// Shield schematic
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 190, iHeight - 150 + 10, 150, 130, Color(0, 0, 0, 100));

	// Tank data
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 350, iHeight - 150 + 10, 100, 130, Color(0, 0, 0, 100));

	// Background for the attack info label
	CRootPanel::PaintRect(m_pAttackInfo->GetLeft()-3, m_pAttackInfo->GetTop()-9, m_pAttackInfo->GetWidth()+6, m_pAttackInfo->GetHeight()+6, Color(0, 0, 0, 100));

	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
	{
		CTeam* pTeam = DigitanksGame()->GetTeam(i);
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

			if (pTank == DigitanksGame()->GetCurrentTank() && CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
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
					CTeam* pTeam = DigitanksGame()->GetCurrentTeam();
					for (size_t t = 0; t < pTeam->GetNumTanks(); t++)
					{
						CDigitank* pTank = pTeam->GetTank(t);
						pTank->SetAttackPower(flAttackPercentage * (pTank->GetBasePower()-pTank->GetBaseMovementPower()));
					}
				}
				else
					DigitanksGame()->GetCurrentTank()->SetAttackPower(flAttackPercentage * (pTank->GetBasePower()-pTank->GetBaseMovementPower()));

				UpdateAttackInfo();
			}
		}
	}

	CPanel::Paint(x, y, w, h);

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	if (pTank)
	{
		CRootPanel::PaintRect(iWidth/2 - 1024/2 + 190 + 150/2 - 50/2, iHeight - 150 + 10 + 130/2 - 50/2, 50, 50, pTank->GetTeam()->GetColor());
		int iShield = (int)(255*pTank->GetLeftShieldStrength());
		if (iShield > 255)
			iShield = 255;
		CRootPanel::PaintRect(iWidth/2 - 1024/2 + 190 + 150/2 - 50/2 - 20, iHeight - 150 + 10 + 130/2 - 50/2, 10, 50, Color(255, 255, 255, iShield));
		iShield = (int)(255*pTank->GetRightShieldStrength());
		if (iShield > 255)
			iShield = 255;
		CRootPanel::PaintRect(iWidth/2 - 1024/2 + 190 + 150/2 + 50/2 + 10, iHeight - 150 + 10 + 130/2 - 50/2, 10, 50, Color(255, 255, 255, iShield));
		iShield = (int)(255*pTank->GetFrontShieldStrength());
		if (iShield > 255)
			iShield = 255;
		CRootPanel::PaintRect(iWidth/2 - 1024/2 + 190 + 150/2 - 50/2, iHeight - 150 + 10 + 130/2 - 50/2 - 20, 50, 10, Color(255, 255, 255, iShield));
		iShield = (int)(255*pTank->GetRearShieldStrength());
		if (iShield > 255)
			iShield = 255;
		CRootPanel::PaintRect(iWidth/2 - 1024/2 + 190 + 150/2 - 50/2, iHeight - 150 + 10 + 130/2 + 50/2 + 10, 50, 10, Color(255, 255, 255, iShield));
	}

	Vector vecMin;
	Vector vecMax;

	std::vector<Vector> aVecs;
	aVecs.push_back(Vector(-3, -1, -3));
	aVecs.push_back(Vector(3, -1, -3));
	aVecs.push_back(Vector(-3, 3, -3));
	aVecs.push_back(Vector(3, 3, -3));
	aVecs.push_back(Vector(-3, -1, 3));
	aVecs.push_back(Vector(-3, 3, 3));
	aVecs.push_back(Vector(3, -1, 3));
	aVecs.push_back(Vector(3, 3, 3));

	Vector vecOrigin = pTank->GetDesiredMove();

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

	if (m_eMenuMode == MENUMODE_MAIN)
	{
		if (m_bAutoProceed)
		{
			// Nothing. The word "Auto" will appear.
		}
		else if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 700 + 10.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 700 + 10.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
		}
		else
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 700 + 10.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 700 + 30.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 20.0f, 0), Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
		}

		if (m_bAutoProceed)
		{
			// Nothing. The word "Auto" will appear.
		}
		else if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
		}
		else
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 20.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 20.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 20.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 30.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 30.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 20.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 20.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 30.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 30.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 30.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 30.0f, iHeight - 100 + 30.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 30.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 30.0f, 0), Color(255, 255, 255));
		}

		if (m_bAutoProceed)
		{
			// Nothing. The word "Auto" will appear.
		}
		else if (CDigitanksWindow::Get()->GetControlMode() == MODE_AIM)
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 820 + 10.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 820 + 40.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 820 + 10.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 820 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
		}
		else
		{
			glPushMatrix();
			glTranslatef(iWidth/2 - 1024/2 + 820 + 25.0f, iHeight - 100 + 25.0f, 0);
			glRotatef(90, 1, 0, 0);
			DebugCircle(Vector(0, 0, 0), 15, Color(255, 255, 255));
			glPopMatrix();
			DebugLine(Vector(iWidth/2 - 1024/2 + 820 + 5.0f, iHeight - 100 + 25.0f, 0), Vector(iWidth/2 - 1024/2 + 820 + 45.0f, iHeight - 100 + 25.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 820 + 25.0f, iHeight - 100 + 5.0f, 0), Vector(iWidth/2 - 1024/2 + 820 + 25.0f, iHeight - 100 + 45.0f, 0), Color(255, 255, 255));
		}

		if (m_bAutoProceed)
		{
			// Nothing. The word "Auto" will appear.
		}
		else if (CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 880 + 10.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 880 + 40.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 880 + 10.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 880 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
		}
		else
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 880 + 20.0f, iHeight - 100 + 5.0f, 0),  Vector(iWidth/2 - 1024/2 + 880 + 30.0f, iHeight - 100 + 5.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 880 + 20.0f, iHeight - 100 + 45.0f, 0), Vector(iWidth/2 - 1024/2 + 880 + 30.0f, iHeight - 100 + 45.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 880 + 20.0f, iHeight - 100 + 20.0f, 0), Vector(iWidth/2 - 1024/2 + 880 + 30.0f, iHeight - 100 + 20.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 880 + 20.0f, iHeight - 100 + 5.0f, 0),  Vector(iWidth/2 - 1024/2 + 880 + 20.0f, iHeight - 100 + 45.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 880 + 30.0f, iHeight - 100 + 5.0f, 0),  Vector(iWidth/2 - 1024/2 + 880 + 30.0f, iHeight - 100 + 45.0f, 0), Color(255, 255, 255));
		}

		DebugLine(Vector(iWidth/2 - 1024/2 + 940 + 20.0f, iHeight - 100 + 20.0f, 0), Vector(iWidth/2 - 1024/2 + 940 + 20.0f, iHeight - 100 + 45.0f, 0), Color(255, 255, 255));
		DebugLine(Vector(iWidth/2 - 1024/2 + 940 + 10.0f, iHeight - 100 + 20.0f, 0), Vector(iWidth/2 - 1024/2 + 940 + 20.0f, iHeight - 100 + 20.0f, 0), Color(255, 255, 255));
		DebugLine(Vector(iWidth/2 - 1024/2 + 940 + 25.0f, iHeight - 100 + 5.0f, 0),  Vector(iWidth/2 - 1024/2 + 940 + 10.0f, iHeight - 100 + 20.0f, 0), Color(255, 255, 255));
		DebugLine(Vector(iWidth/2 - 1024/2 + 940 + 40.0f, iHeight - 100 + 20.0f, 0), Vector(iWidth/2 - 1024/2 + 940 + 25.0f, iHeight - 100 + 5.0f, 0), Color(255, 255, 255));
		DebugLine(Vector(iWidth/2 - 1024/2 + 940 + 30.0f, iHeight - 100 + 20.0f, 0), Vector(iWidth/2 - 1024/2 + 940 + 40.0f, iHeight - 100 + 20.0f, 0), Color(255, 255, 255));
		DebugLine(Vector(iWidth/2 - 1024/2 + 940 + 30.0f, iHeight - 100 + 20.0f, 0), Vector(iWidth/2 - 1024/2 + 940 + 30.0f, iHeight - 100 + 45.0f, 0), Color(255, 255, 255));
		DebugLine(Vector(iWidth/2 - 1024/2 + 940 + 30.0f, iHeight - 100 + 45.0f, 0), Vector(iWidth/2 - 1024/2 + 940 + 20.0f, iHeight - 100 + 45.0f, 0), Color(255, 255, 255));
	}
	else
	{
		DebugLine(Vector(iWidth/2 - 1024/2 + 940 + 10.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 940 + 40.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
		DebugLine(Vector(iWidth/2 - 1024/2 + 940 + 10.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 940 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
	}
}

void CHUD::UpdateAttackInfo()
{
	m_pAttackInfo->SetText("");

	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	if (!pCurrentTank)
		return;

	char szShieldInfo[1024];
	float flShieldMax = pCurrentTank->GetShieldMaxStrength() * pCurrentTank->GetDefenseScale(true);
	sprintf(szShieldInfo, "%.1f/%.1f", pCurrentTank->GetFrontShieldStrength() * pCurrentTank->GetShieldMaxStrength(), flShieldMax);
	m_pFrontShieldInfo->SetText(szShieldInfo);

	sprintf(szShieldInfo, "%.1f/%.1f", pCurrentTank->GetRearShieldStrength() * pCurrentTank->GetShieldMaxStrength(), flShieldMax);
	m_pRearShieldInfo->SetText(szShieldInfo);

	sprintf(szShieldInfo, "%.1f/\n%.1f", pCurrentTank->GetLeftShieldStrength() * pCurrentTank->GetShieldMaxStrength(), flShieldMax);
	m_pLeftShieldInfo->SetText(szShieldInfo);

	sprintf(szShieldInfo, "%.1f/\n%.1f", pCurrentTank->GetRightShieldStrength() * pCurrentTank->GetShieldMaxStrength(), flShieldMax);
	m_pRightShieldInfo->SetText(szShieldInfo);

	m_pTankInfo->SetText("TANK INFO");

	if (pCurrentTank->HasBonusPoints())
	{
		if (pCurrentTank->GetBonusPoints() > 1)
			sprintf(szShieldInfo, "\n \n%d bonus points available", pCurrentTank->GetBonusPoints());
		else
			sprintf(szShieldInfo, "\n \n1 bonus point available");
		m_pTankInfo->AppendText(szShieldInfo);
	}

	if (pCurrentTank->GetBonusAttackPower())
	{
		sprintf(szShieldInfo, "\n \n+%d attack power", (int)pCurrentTank->GetBonusAttackPower());
		m_pTankInfo->AppendText(szShieldInfo);
	}

	if (pCurrentTank->GetBonusDefensePower())
	{
		sprintf(szShieldInfo, "\n \n+%d defense power", (int)pCurrentTank->GetBonusDefensePower());
		m_pTankInfo->AppendText(szShieldInfo);
	}

	if (pCurrentTank->GetBonusMovementPower())
	{
		sprintf(szShieldInfo, "\n \n+%d movement power", (int)pCurrentTank->GetBonusMovementPower());
		m_pTankInfo->AppendText(szShieldInfo);
	}

	Vector vecOrigin;
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE && pCurrentTank->GetPreviewMoveTurnPower() <= pCurrentTank->GetTotalMovementPower())
		vecOrigin = pCurrentTank->GetPreviewMove();
	else
		vecOrigin = pCurrentTank->GetDesiredMove();

	Vector vecAim;
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_AIM)
		vecAim = pCurrentTank->GetPreviewAim();
	else
		vecAim = pCurrentTank->GetDesiredAim();

	Vector vecAttack = vecOrigin - vecAim;
	float flAttackDistance = vecAttack.Length();

	if (flAttackDistance > pCurrentTank->GetMaxRange())
		return;

	if (!pCurrentTank->HasDesiredAim() && !pCurrentTank->IsPreviewAimValid())
		return;

	float flRadius = RemapValClamped(flAttackDistance, pCurrentTank->GetMinRange(), pCurrentTank->GetMaxRange(), 2, TANK_MAX_RANGE_RADIUS);

	CDigitank* pClosestTarget = NULL;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);

		if (!pEntity)
			continue;

		CDigitank* pTargetTank = dynamic_cast<CDigitank*>(pEntity);

		if (!pTargetTank)
			continue;

		if (pTargetTank == pCurrentTank)
			continue;

		if ((pTargetTank->GetOrigin() - vecAim).LengthSqr() > flRadius*flRadius)
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
	{
		m_pAttackInfo->SetText("Hit odds: 0%");
		return;
	}

	float flTargetDistance = (vecAim - pClosestTarget->GetOrigin()).Length();

	if (flTargetDistance > flRadius)
	{
		m_pAttackInfo->SetText("Hit odds: 0%");
		return;
	}

	float flArea = flRadius*2*flRadius*2;			// It's really a square!
	float flHitProbability = 2*4*M_PI / flArea;		// 2 * pi * tank radius

	// If the target is close to the edge, fade the probability down
	flHitProbability = RemapValClamped(flRadius - flTargetDistance, 0, 2, 0, flHitProbability);

	if (flHitProbability <= 0)
	{
		m_pAttackInfo->SetText("Hit odds: 0%");
		return;
	}

	if (flHitProbability > 1)
		flHitProbability = 1;

	float flShieldStrength = (*pClosestTarget->GetShieldForAttackDirection(vecAttack/flAttackDistance));
	float flDamageBlocked = flShieldStrength * pClosestTarget->GetDefenseScale(true);
	float flAttackDamage = pCurrentTank->GetAttackPower(true);

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
		"Hit odds: %d%%\n"
		" \n"
		"Shield Damage: %.1f/%.1f\n"
		"Digitank Damage: %.1f/%.1f\n",
		(int)(flHitProbability*100),
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

void CHUD::SetupMenu(menumode_t eMenuMode)
{
	if (eMenuMode == MENUMODE_MAIN)
	{
		m_pButton1->SetClickedListener(this, Move);
		m_pButton2->SetClickedListener(this, Turn);
		m_pButton3->SetClickedListener(this, Aim);
		m_pButton4->SetClickedListener(this, Fire);
		m_pButton5->SetClickedListener(this, Promote);
		m_pButtonHelp1->SetText("Move");
		m_pButtonHelp2->SetText("Turn");
		m_pButtonHelp3->SetText("Aim");
		m_pButtonHelp4->SetText("Set Power");
		m_pButtonHelp5->SetText("Promote");
	}
	else if (eMenuMode == MENUMODE_PROMOTE)
	{
		m_pButton1->SetClickedListener(this, PromoteAttack);
		m_pButton2->SetClickedListener(this, PromoteDefense);
		m_pButton3->SetClickedListener(this, PromoteMovement);
		m_pButton4->SetClickedListener(NULL, NULL);
		m_pButton5->SetClickedListener(this, GoToMain);
		m_pButtonHelp1->SetText("Upgrade\nAttack");
		m_pButtonHelp2->SetText("Upgrade\nDefense");
		m_pButtonHelp3->SetText("Upgrade\nMovement");
		m_pButtonHelp4->SetText("");
		m_pButtonHelp5->SetText("Return");
	}

	m_eMenuMode = eMenuMode;
}

void CHUD::GameStart()
{
	CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	CDigitanksWindow::Get()->GetInstructor()->Initialize();
	CDigitanksWindow::Get()->GetInstructor()->DisplayFirstTutorial();
}

void CHUD::GameOver()
{
}

void CHUD::NewCurrentTeam()
{
	m_bAutoProceed = true;

	CDigitanksWindow::Get()->SetControlMode(MODE_MOVE);

	Game()->GetCamera()->SetTarget(DigitanksGame()->GetCurrentTeam()->GetTank(0)->GetOrigin());
}

void CHUD::NewCurrentTank()
{
	if (m_bAutoProceed)
	{
		if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE && DigitanksGame()->GetCurrentTank()->HasSelectedMove())
			CDigitanksWindow::Get()->SetControlMode(MODE_AIM);

		if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN && DigitanksGame()->GetCurrentTank()->HasDesiredTurn())
			CDigitanksWindow::Get()->SetControlMode(MODE_NONE);

		if (CDigitanksWindow::Get()->GetControlMode() == MODE_AIM && DigitanksGame()->GetCurrentTank()->HasDesiredAim())
		{
			if (!DigitanksGame()->GetCurrentTank()->ChoseFirepower())
				CDigitanksWindow::Get()->SetControlMode(MODE_FIRE);
			else
				CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
		}

		if (CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE && DigitanksGame()->GetCurrentTank()->ChoseFirepower())
		{
			CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
			m_bAutoProceed = false;
		}
	}

	UpdateAttackInfo();

	Game()->GetCamera()->SetTarget(DigitanksGame()->GetCurrentTank()->GetDesiredMove());
}

void CHUD::OnTakeShieldDamage(CDigitank* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage)
{
	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, true);
}

void CHUD::OnTakeDamage(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage)
{
	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, false);
}

void CHUD::MoveCallback()
{
	m_bAutoProceed = false;

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_MOVE);
}

void CHUD::TurnCallback()
{
	m_bAutoProceed = false;

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_TURN);
}

void CHUD::AimCallback()
{
	m_bAutoProceed = false;

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_AIM)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_AIM);
}

void CHUD::FireCallback()
{
	m_bAutoProceed = false;

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else if (DigitanksGame()->GetCurrentTank() && !DigitanksGame()->GetCurrentTank()->HasDesiredAim())
		CDigitanksWindow::Get()->SetControlMode(MODE_AIM);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_FIRE);
}

void CHUD::PromoteCallback()
{
	SetupMenu(MENUMODE_PROMOTE);
}

void CHUD::PromoteAttackCallback()
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	pTank->PromoteAttack();

	SetupMenu(MENUMODE_MAIN);

	UpdateAttackInfo();

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
}

void CHUD::PromoteDefenseCallback()
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	pTank->PromoteDefense();

	SetupMenu(MENUMODE_MAIN);

	UpdateAttackInfo();

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
}

void CHUD::PromoteMovementCallback()
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	pTank->PromoteMovement();

	SetupMenu(MENUMODE_MAIN);

	UpdateAttackInfo();

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
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
	SetPos((int)vecScreen.x, (int)vecScreen.y);

	if (m_bShield)
		SetFGColor(Color(255, 255, 255));
	else
		SetFGColor(Color(255, 0, 0));

	int iDamage = (int)flDamage;
	if (flDamage > 0 && iDamage < 1)
		iDamage = 1;

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
	SetPos((int)(vecScreen.x+flOffset), (int)(vecScreen.y-flOffset));

	SetAlpha((int)RemapVal(DigitanksGame()->GetGameTime() - m_flTime, 0, flFadeTime, 255, 0));

	BaseClass::Think();
}

void CDamageIndicator::Paint(int x, int y, int w, int h)
{
	BaseClass::Paint(x, y, w, h);
}
