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

class CAOPanel : public CMovablePanel, public IWorkListener
{
public:
							CAOPanel(bool bColor, CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);

	virtual void			SetVisible(bool bVisible);

	virtual void			Layout();

	virtual void			WorkProgress();

	EVENT_CALLBACK(CAOPanel, Generate);
	EVENT_CALLBACK(CAOPanel, SaveMap);
	EVENT_CALLBACK(CAOPanel, AOMethod);

	static void				Open(bool bColor, CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);

protected:
	bool					m_bColor;

	CConversionScene*		m_pScene;
	std::vector<CMaterial>*	m_paoMaterials;

	CAOGenerator			m_oGenerator;

	CLabel*					m_pSizeLabel;
	CScrollSelector<int>*	m_pSizeSelector;

	CLabel*					m_pEdgeBleedLabel;
	CScrollSelector<int>*	m_pEdgeBleedSelector;

	CLabel*					m_pAOMethodLabel;
	CScrollSelector<int>*	m_pAOMethodSelector;

	CLabel*					m_pRayDensityLabel;
	CScrollSelector<int>*	m_pRayDensitySelector;

	CLabel*					m_pLightsLabel;
	CScrollSelector<int>*	m_pLightsSelector;

	CButton*				m_pGenerate;
	CButton*				m_pSave;

	static CAOPanel*		s_pAOPanel;
	static CAOPanel*		s_pColorAOPanel;
};

class CHelpPanel : public CMovablePanel
{
public:
							CHelpPanel();

	virtual void			Layout();
	virtual void			Paint(int x, int y, int w, int h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	CLabel*					m_pInfo;

	static CHelpPanel*		s_pHelpPanel;
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

class CRegisterPanel : public CMovablePanel
{
public:
							CRegisterPanel();

	virtual void			Layout();
	virtual void			Paint(int x, int y, int w, int h);

	virtual bool			MousePressed(int iButton, int mx, int my);
	virtual bool			KeyPressed(int iKey);

	static void				Open();
	static void				Close();

	EVENT_CALLBACK(CRegisterPanel, Pirates);

protected:
	CLabel*					m_pInfo;
	CButton*				m_pPirates;

	bool					m_bBadTry;

	static CRegisterPanel*	s_pRegisterPanel;
};

class CPiratesPanel : public CMovablePanel
{
public:
							CPiratesPanel();

	virtual void			Layout();
	virtual void			Paint(int x, int y, int w, int h);

	virtual bool			MousePressed(int iButton, int mx, int my);

	static void				Open();
	static void				Close();

protected:
	CLabel*					m_pInfo;

	static CPiratesPanel*	s_pPiratesPanel;
};

#endif
