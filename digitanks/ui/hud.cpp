#include "hud.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "digitankswindow.h"
#include "game/digitanksgame.h"

using namespace glgui;

CPowerBar::CPowerBar(powerbar_type_t ePowerbarType)
	: CBaseControl(0, 0, 100, 100)
{
	m_ePowerbarType = ePowerbarType;
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
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetHealth() / pTank->GetTotalHealth())-2, h-2, Color(0, 255, 0));
	else if (m_ePowerbarType == POWERBAR_ATTACK)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetAttackPower(true))-2, h-2, Color(255, 0, 0));
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetDefensePower(true))-2, h-2, Color(255, 255, 0));
	else
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetMovementPower(true))-2, h-2, Color(0, 0, 255));
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

	m_pMoveButton = new CButton(0, 0, 50, 50, "");
	m_pMoveButton->SetClickedListener(this, Move);
	AddControl(m_pMoveButton);

	m_pTurnButton = new CButton(0, 0, 50, 50, "");
	m_pTurnButton->SetClickedListener(this, Turn);
	AddControl(m_pTurnButton);

	m_pFireButton = new CButton(0, 0, 50, 50, "");
	m_pFireButton->SetClickedListener(this, Fire);
	AddControl(m_pFireButton);

	m_pAttackInfo = new CLabel(0, 0, 100, 150, "");
	m_pAttackInfo->SetWrap(false);
	m_pAttackInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pAttackInfo);
}

void CHUD::Layout()
{
	SetSize(GetParent()->GetWidth(), GetParent()->GetHeight());

	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	m_pAttackInfo->SetPos(iWidth/2 - 1024/2 + 190 + 3, iHeight - 150 - 10 - 80 + 3);
	m_pAttackInfo->SetSize(200, 80);

	m_pHealthBar->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 140);
	m_pHealthBar->SetSize(200, 20);

	m_pAttackPower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 90);
	m_pAttackPower->SetSize(200, 20);

	m_pDefensePower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 60);
	m_pDefensePower->SetSize(200, 20);

	m_pMovementPower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 30);
	m_pMovementPower->SetSize(200, 20);

	m_pMoveButton->SetPos(iWidth/2 - 1024/2 + 700, iHeight - 100);
	m_pTurnButton->SetPos(iWidth/2 - 1024/2 + 760, iHeight - 100);
	m_pFireButton->SetPos(iWidth/2 - 1024/2 + 820, iHeight - 100);
}

void CHUD::Think()
{
	BaseClass::Think();

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
		m_pMoveButton->SetButtonColor(Color(100, 0, 0));
	else
		m_pMoveButton->SetButtonColor(Color(0, 0, 100));

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
		m_pTurnButton->SetButtonColor(Color(100, 0, 0));
	else
		m_pTurnButton->SetButtonColor(Color(0, 0, 100));

	if (CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
		m_pFireButton->SetButtonColor(Color(100, 0, 0));
	else
		m_pFireButton->SetButtonColor(Color(60, 40, 0));
}

void CHUD::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

#ifdef _DEBUG
	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	// Nobody runs resolutions under 1024x768 anymore.
	// Show me my constraints!
	CRootPanel::PaintRect(iWidth/2 - 1024/2, iHeight - 150, 1024, 200, Color(0, 0, 0, 100));

	// This is where the minimap will be.
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 10, iHeight - 150 - 30, 170, 170, Color(255, 255, 255, 100));

	// Shield schematic
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 190, iHeight - 150 + 10, 130, 130, Color(255, 255, 255, 100));

	// Tank data
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 330, iHeight - 150 + 10, 100, 130, Color(255, 255, 255, 100));
#endif

	// Background for the attack info label
	CRootPanel::PaintRect(m_pAttackInfo->GetLeft()-3, m_pAttackInfo->GetTop()-9, m_pAttackInfo->GetWidth()+6, m_pAttackInfo->GetHeight()+6, Color(0, 0, 0, 100));

	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
	{
		CTeam* pTeam = DigitanksGame()->GetTeam(i);
		for (size_t j = 0; j < pTeam->GetNumTanks(); j++)
		{
			CDigitank* pTank = pTeam->GetTank(j);

			Vector vecOrigin;
			if (pTank->HasDesiredMove())
				vecOrigin = pTank->GetDesiredMove();
			else
				vecOrigin = pTank->GetOrigin();

			Vector vecScreen = CDigitanksWindow::Get()->ScreenPosition(vecOrigin);

			CRootPanel::PaintRect((int)vecScreen.x - 51, (int)vecScreen.y - 61, 102, 5, Color(255, 255, 255, 128));
			CRootPanel::PaintRect((int)vecScreen.x - 50, (int)vecScreen.y - 60, (int)(100.0f*pTank->GetHealth()/pTank->GetTotalHealth()), 3, Color(100, 255, 100));

			CRootPanel::PaintRect((int)vecScreen.x - 51, (int)vecScreen.y - 51, 102, 5, Color(255, 255, 255, 128));
			CRootPanel::PaintRect((int)vecScreen.x - 50, (int)vecScreen.y - 50, (int)(100.0f*pTank->GetAttackPower(true)), 3, Color(255, 0, 0));
			CRootPanel::PaintRect((int)vecScreen.x - 50 + (int)(100.0f*pTank->GetAttackPower(true)), (int)vecScreen.y - 50, (int)(100.0f*pTank->GetDefensePower(true)), 3, Color(255, 255, 0));
			CRootPanel::PaintRect((int)vecScreen.x - 50 + (int)(100.0f*(1-pTank->GetMovementPower(true))), (int)vecScreen.y - 50, (int)(100.0f*pTank->GetMovementPower(true)), 3, Color(0, 0, 255));

			if (pTank == DigitanksGame()->GetCurrentTank() && CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
			{
				int iHeight = (int)(200 * (1-pTank->GetMovementPower()));

				if (iHeight < 20)
					iHeight = 20;

				int iTop = (int)vecScreen.y - iHeight/2;
				int iBottom = (int)vecScreen.y + iHeight/2;

				int mx, my;
				glgui::CRootPanel::GetFullscreenMousePos(mx, my);

				float flAttackPercentage = RemapValClamped((float)my, (float)iTop, (float)iBottom, 0, 1);

				CRootPanel::PaintRect((int)vecScreen.x + 60, iTop, 20, iHeight, Color(255, 255, 255, 128));

				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + 1, 18, (int)(flAttackPercentage*(iHeight-2)), Color(255, 0, 0, 255));
				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + 1 + (int)(flAttackPercentage*(iHeight-2)), 18, (int)((1-flAttackPercentage)*(iHeight-2)), Color(255, 255, 0, 255));
				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + (int)(flAttackPercentage*(iHeight-2)) - 2, 18, 6, Color(128, 128, 128, 255));

				DigitanksGame()->GetCurrentTank()->SetAttackPower(flAttackPercentage * (pTank->GetTotalPower()*(1-pTank->GetMovementPower())));

				UpdateAttackInfo();
			}
		}
	}

	CPanel::Paint(x, y, w, h);
}

void CHUD::UpdateAttackInfo()
{
	m_pAttackInfo->SetText("");

	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	if (!pCurrentTank)
		return;

	CDigitank* pTargetTank = pCurrentTank->GetTarget();

	if (!pTargetTank)
		return;

	Vector vecOrigin;
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE && pCurrentTank->GetPreviewMoveTurnPower() <= pCurrentTank->GetTotalPower())
		vecOrigin = pCurrentTank->GetPreviewMove();
	else
		vecOrigin = pCurrentTank->GetDesiredMove();

	Vector vecAttack = vecOrigin - pTargetTank->GetOrigin();
	float flAttackDistance = vecAttack.Length();

	int iHitOdds = (int)RemapValClamped(flAttackDistance, 30, 50, 100, 0);

	if (iHitOdds <= 0)
	{
		m_pAttackInfo->SetText("Hit odds: 0%");
		return;
	}

	float flDamageBlocked = (*pTargetTank->GetShieldForAttackDirection(vecAttack/flAttackDistance)) * pTargetTank->GetDefensePower();
	float flAttackDamage = pCurrentTank->GetAttackPower(true) * pCurrentTank->GetTotalPower();

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
		"Shield Damage: %.2f/%.2f\n"
		"Digitank Damage: %.2f/%.2f\n",
		iHitOdds,
		flShieldDamage, pTargetTank->GetShieldMaxStrength() * pTargetTank->GetDefensePower(),
		flTankDamage, pTargetTank->GetHealth()
	);

	m_pAttackInfo->SetText(szAttackInfo);
}

void CHUD::SetGame(CDigitanksGame *pGame)
{
	m_pGame = pGame;
	m_pGame->SetListener(this);
}

void CHUD::GameStart()
{
	CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
}

void CHUD::GameOver()
{
}

void CHUD::NewCurrentTeam()
{
}

void CHUD::NewCurrentTank()
{
	if (!DigitanksGame()->GetCurrentTank()->HasDesiredMove() && !DigitanksGame()->GetCurrentTank()->HasDesiredTurn())
		CDigitanksWindow::Get()->SetControlMode(MODE_MOVE, true);
}

void CHUD::MoveCallback()
{
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_MOVE);
}

void CHUD::TurnCallback()
{
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_TURN);
}

void CHUD::FireCallback()
{
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_FIRE);
}
