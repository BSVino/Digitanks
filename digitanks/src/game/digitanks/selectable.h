#ifndef DT_SELECTABLE_H
#define DT_SELECTABLE_H

#include <baseentity.h>

#include "dt_common.h"

class CSelectable : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CSelectable, CBaseEntity);

public:
	virtual float				GetBoundingRadius() const { return 4; };

	virtual void				OnCurrentSelection() {};
	virtual void				OnControlModeChange(controlmode_t eOldMode, controlmode_t eNewMode) {};

	virtual const char*			GetPowerBar1Text() { return ""; }
	virtual const char*			GetPowerBar2Text() { return ""; }
	virtual const char*			GetPowerBar3Text() { return ""; }

	virtual float				GetPowerBar1Value() { return 0; }
	virtual float				GetPowerBar2Value() { return 0; }
	virtual float				GetPowerBar3Value() { return 0; }

	virtual float				GetPowerBar1Size() { return 0; }
	virtual float				GetPowerBar2Size() { return 0; }
	virtual float				GetPowerBar3Size() { return 0; }

	virtual void				SetupMenu(menumode_t eMenuMode) {};
};

#endif