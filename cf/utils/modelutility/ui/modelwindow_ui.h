#ifndef MODELWINDOW_UI_H
#define MODELWINDOW_UI_H

#include "modelgui.h"

using namespace modelgui;

class CButtonPanel : public CPanel
{
public:
							CButtonPanel();

	virtual void			Layout();

	virtual void			AddButton(CButton* pButton, bool bNewSection, IEventListener* pListener = NULL, IEventListener::Callback pfnCallback = NULL);

protected:
	std::vector<int>		m_aiSpaces;
};

#define HEADER_HEIGHT 16

class CCloseButton : public CButton
{
public:
							CCloseButton() : CButton(0, 0, 10, 10, "") {};

	virtual void			Paint(int x, int y, int w, int h);
};

class CMovablePanel : public CPanel, public IEventListener
{
public:
							CMovablePanel(char* pszName);

	virtual void			Layout();

	virtual void			Think();

	virtual void			Paint(int x, int y, int w, int h);

	virtual bool			MousePressed(int iButton, int mx, int my);
	virtual bool			MouseReleased(int iButton, int mx, int my);

	EVENT_CALLBACK(CMovablePanel, CloseWindow);

protected:
	int						m_iMouseStartX;
	int						m_iMouseStartY;
	int						m_iStartX;
	int						m_iStartY;
	bool					m_bMoving;

	CLabel*					m_pName;

	CCloseButton*			m_pCloseButton;
};

class CColorAOPanel : public CMovablePanel
{
public:
							CColorAOPanel();

	virtual void			Layout();

	static void				Open();
	static void				Close();

protected:
	static CColorAOPanel*	s_pColorAOPanel;
};

class CAboutPanel : public CMovablePanel
{
public:
							CAboutPanel();

	virtual void			Layout();
	virtual void			Paint(int x, int y, int w, int h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	CLabel*					m_pInfo;

	static CAboutPanel*		s_pAboutPanel;
};

#endif
