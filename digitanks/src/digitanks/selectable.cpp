#include "selectable.h"

#include <ui/digitankswindow.h>
#include <ui/hud.h>

REGISTER_ENTITY(CSelectable);

NETVAR_TABLE_BEGIN(CSelectable);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CSelectable);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CSelectable);
	INPUT_DEFINE(Select);
INPUTS_TABLE_END();

void CSelectable::Select(const tvector<tstring>& sArgs)
{
	if (!GetDigitanksPlayer())
		return;

	for (size_t i = 0; i < sArgs.size(); i++)
	{
		if (sArgs[i] == "if-selection-empty")
		{
			if (GetDigitanksPlayer()->GetPrimarySelection())
				return;
		}
	}

	GetDigitanksPlayer()->SetPrimarySelection(this);
}

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
