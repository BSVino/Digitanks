#ifndef DT_SELECTABLE_H
#define DT_SELECTABLE_H

#include "digitanksentity.h"

#include "dt_common.h"

class CSelectable : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CSelectable, CDigitanksEntity);

public:
	virtual float				GetBoundingRadius() const { return 4; };

	DECLARE_ENTITY_INPUT(Select);

	virtual void				OnCurrentSelection();
	virtual bool				AllowControlMode(controlmode_t eMode) const { return eMode == MODE_NONE; };
	virtual void				OnControlModeChange(controlmode_t eOldMode, controlmode_t eNewMode) { };

	virtual const char*			GetPowerBar1Text() { return ""; }
	virtual const char*			GetPowerBar2Text() { return ""; }
	virtual const char*			GetPowerBar3Text() { return ""; }

	virtual float				GetPowerBar1Value() { return 0; }
	virtual float				GetPowerBar2Value() { return 0; }
	virtual float				GetPowerBar3Value() { return 0; }

	virtual float				GetPowerBar1Size() { return 0; }
	virtual float				GetPowerBar2Size() { return 0; }
	virtual float				GetPowerBar3Size() { return 0; }

	virtual void				DrawSchema(int x, int y, int w, int h) {};
	virtual void				DrawQueue(int x, int y, int w, int h) {};

	virtual bool				NeedsOrders() { return false; }
	virtual bool				ShowHealthBar() { return true; }

	virtual void				SetupMenu(menumode_t eMenuMode);
};

#endif