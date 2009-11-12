#ifndef MODELWINDOW_UI_H
#define MODELWINDOW_UI_H

#include "modelgui.h"

using namespace modelgui;

class CAboutPanel : public CPanel
{
public:
							CAboutPanel();

	void					Layout();
	void					Paint(int x, int y, int w, int h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	CLabel*					m_pInfo;

	static CAboutPanel*		s_pAboutPanel;
};

#endif
