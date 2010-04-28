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

	if (m_ePowerbarType == POWERBAR_ATTACK)
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

	m_pAttackPower = new CPowerBar(POWERBAR_ATTACK);
	m_pDefensePower = new CPowerBar(POWERBAR_DEFENSE);
	m_pMovementPower = new CPowerBar(POWERBAR_MOVEMENT);

	AddControl(m_pAttackPower);
	AddControl(m_pDefensePower);
	AddControl(m_pMovementPower);
}

void CHUD::Layout()
{
	SetSize(GetParent()->GetWidth(), GetParent()->GetHeight());

	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	m_pAttackPower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 90);
	m_pAttackPower->SetSize(200, 20);

	m_pDefensePower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 60);
	m_pDefensePower->SetSize(200, 20);

	m_pMovementPower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 30);
	m_pMovementPower->SetSize(200, 20);
}

void CHUD::Think()
{
	if (!m_pGame)
		return;
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

	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
	{
		CTeam* pTeam = DigitanksGame()->GetTeam(i);
		for (size_t j = 0; j < pTeam->GetNumTanks(); j++)
		{
			CDigitank* pTank = pTeam->GetTank(j);
			Vector vecScreen = CDigitanksWindow::Get()->ScreenPosition(pTank->GetOrigin());
			CRootPanel::PaintRect((int)vecScreen.x - 51, (int)vecScreen.y - 51, 102, 5, Color(255, 255, 255, 128));
			CRootPanel::PaintRect((int)vecScreen.x - 50, (int)vecScreen.y - 50, (int)(100.0f*pTank->GetHealth()/pTank->GetTotalHealth()), 3, Color(100, 255, 100));
		}
	}

	CPanel::Paint(x, y, w, h);
}
