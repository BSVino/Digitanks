#include "selectable.h"

#include <ui/digitankswindow.h>
#include <ui/hud.h>

REGISTER_ENTITY(CSelectable);

void CSelectable::OnCurrentSelection()
{
	if (!AllowControlMode(DigitanksGame()->GetControlMode()))
		DigitanksGame()->SetControlMode(MODE_NONE);

	if (DigitanksGame()->GetListener())
		DigitanksGame()->GetListener()->NewCurrentSelection();
}

void CSelectable::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	pHUD->SetButton1Listener(NULL);
	pHUD->SetButton2Listener(NULL);
	pHUD->SetButton3Listener(NULL);
	pHUD->SetButton4Listener(NULL);
	pHUD->SetButton5Listener(NULL);

	pHUD->SetButton1Help("");
	pHUD->SetButton2Help("");
	pHUD->SetButton3Help("");
	pHUD->SetButton4Help("");
	pHUD->SetButton5Help("");

	pHUD->SetButton1Texture(0);
	pHUD->SetButton2Texture(0);
	pHUD->SetButton3Texture(0);
	pHUD->SetButton4Texture(0);
	pHUD->SetButton5Texture(0);

	pHUD->SetButton1Color(glgui::g_clrBox);
	pHUD->SetButton2Color(glgui::g_clrBox);
	pHUD->SetButton3Color(glgui::g_clrBox);
	pHUD->SetButton4Color(glgui::g_clrBox);
	pHUD->SetButton5Color(glgui::g_clrBox);
}
