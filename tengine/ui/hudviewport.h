#pragma once

#include <glgui/panel.h>

class CHUDViewport : public glgui::CPanel
{
	DECLARE_CLASS(CHUDViewport, glgui::CPanel);

public:
					CHUDViewport();

public:
	virtual void	Layout();

	virtual void	Paint(float x, float y, float w, float h);
};
