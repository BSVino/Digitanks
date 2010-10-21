#include "selectable.h"

#include <ui/digitankswindow.h>
#include <ui/hud.h>

NETVAR_TABLE_BEGIN(CSelectable);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CSelectable);
SAVEDATA_TABLE_END();

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
