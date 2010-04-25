#include "digitankswindow.h"

#include "glgui/glgui.h"

#include "hud.h"

using namespace glgui;

void CDigitanksWindow::InitUI()
{
	m_pHUD = new CHUD();
	m_pHUD->SetGame(m_pDigitanksGame);

	CRootPanel::Get()->AddControl(m_pHUD);

	CRootPanel::Get()->Layout();
}

void CDigitanksWindow::Layout()
{
	CRootPanel::Get()->Layout();
}
