#include "general_window.h"

#include <game/gameserver.h>
#include <renderer/renderer.h>

CGeneralWindow::CGeneralWindow()
	: CPanel(0, 0, 400, 400), m_hGeneral(L"textures/hud/helper-emotions.txt"), m_hGeneralMouth(L"textures/hud/helper-emotions-open.txt")
{
	glgui::CRootPanel::Get()->AddControl(this);

	m_flDeployed = m_flDeployedGoal = 0;

	m_pText = new glgui::CLabel(100, 0, 300, 300, L"");
	AddControl(m_pText);

	m_pButton = new glgui::CButton(0, 0, 100, 30, L"");
	AddControl(m_pButton);
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
}

void CGeneralWindow::Paint(int x, int y, int w, int h)
{
	glgui::CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, 255));

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
}

void CGeneralWindow::Deploy()
{
	Layout();

	m_flDeployedGoal = 1;

	m_pText->SetText(L"THE GENERAL'S\nANTI-BUG UTILITY\n \nYou are on day 8,479\nof your 30 day trial.\n \nThe General's Anti-Bug Utility has detected the presence of Bugs in your computer. Would you like The General to attempt to repair them for you?");
	m_pButton->SetText(L"Repair");

	m_sEmotion = "Pleased";
}
