#include "selectable.h"

#include <ui/digitankswindow.h>
#include <ui/hud.h>

void CSelectable::OnCurrentSelection()
{
	if (!AllowControlMode(DigitanksGame()->GetControlMode()))
		DigitanksGame()->SetControlMode(MODE_NONE);

	if (DigitanksGame()->GetListener())
		DigitanksGame()->GetListener()->NewCurrentSelection();
}

void CSelectable::SetupMenu(menumode_t eMenuMode)
{
}
