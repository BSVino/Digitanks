#ifndef MODELWINDOW_UI_H
#define MODELWINDOW_UI_H

#include "modelgui.h"
#include "../crunch.h"

using namespace modelgui;

typedef enum
{
	BA_TOP,
	BA_BOTTOM,
} buttonalignment_t;

class CButtonPanel : public CPanel
{
public:
							CButtonPanel(buttonalignment_t eAlign);

	virtual void			Layout();

	virtual void			AddButton(CButton* pButton, bool bNewSection, IEventListener* pListener = NULL, IEventListener::Callback pfnCallback = NULL);

protected:
	buttonalignment_t		m_eAlign;

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
							~CMovablePanel();

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

class CColorAOPanel : public CMovablePanel, public IWorkListener
{
public:
							CColorAOPanel(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);

	virtual void			Layout();

	virtual void			WorkProgress();

	EVENT_CALLBACK(CColorAOPanel, Generate);
	EVENT_CALLBACK(CColorAOPanel, SaveMap);

	static void				Open(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);
	static void				Close();

protected:
	CConversionScene*		m_pScene;
	std::vector<CMaterial>*	m_paoMaterials;

	CColorAOGenerator		m_oGenerator;

	CButton*				m_pGenerate;
	CButton*				m_pSave;

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
