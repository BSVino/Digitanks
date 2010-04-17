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

	virtual void			AddButton(CButton* pButton, char* pszHints, bool bNewSection, IEventListener* pListener = NULL, IEventListener::Callback pfnCallback = NULL);

	virtual void			Think();
	virtual void			Paint(int x, int y, int w, int h);

protected:
	buttonalignment_t		m_eAlign;

	std::vector<int>		m_aiSpaces;
	std::vector<CButton*>	m_apButtons;
	std::vector<CLabel*>	m_apHints;
};

class CProgressBar : public CPanel
{
public:
							CProgressBar();

public:
	void					Layout();
	void					Paint(int x, int y, int w, int h);

	void					SetTotalProgress(size_t iProgress);
	void					SetProgress(size_t iProgress, wchar_t* pszAction = NULL);
	void					SetAction(wchar_t* pszAction);

	static CProgressBar*	Get();

protected:
	size_t					m_iTotalProgress;
	size_t					m_iCurrentProgress;

	CLabel*					m_pAction;
	std::wstring			m_sAction;

	static CProgressBar*	s_pProgressBar;
};

#define HEADER_HEIGHT 16

class CCloseButton : public CButton
{
public:
							CCloseButton() : CButton(0, 0, 10, 10, "") {};

public:
	virtual void			Paint() { CButton::Paint(); };
	virtual void			Paint(int x, int y, int w, int h);
};

class CMinimizeButton : public CButton
{
public:
							CMinimizeButton() : CButton(0, 0, 10, 10, "") {};

public:
	virtual void			Paint() { CButton::Paint(); };
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

	virtual void			HasCloseButton(bool bHasClose) { m_bHasCloseButton = bHasClose; };
	virtual void			Minimize();

	virtual void			SetClearBackground(bool bClearBackground) { m_bClearBackground = bClearBackground; };

	EVENT_CALLBACK(CMovablePanel, MinimizeWindow);
	EVENT_CALLBACK(CMovablePanel, CloseWindow);

protected:
	int						m_iMouseStartX;
	int						m_iMouseStartY;
	int						m_iStartX;
	int						m_iStartY;
	bool					m_bMoving;

	bool					m_bHasCloseButton;
	bool					m_bMinimized;
	int						m_iNonMinimizedHeight;

	bool					m_bClearBackground;

	CLabel*					m_pName;

	CCloseButton*			m_pCloseButton;
	CMinimizeButton*		m_pMinimizeButton;
};

class CAOPanel : public CMovablePanel, public IWorkListener
{
public:
							CAOPanel(bool bColor, CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);

	virtual void			SetVisible(bool bVisible);

	virtual void			Layout();

	virtual bool			KeyPressed(int iKey);

	virtual void			BeginProgress();
	virtual void			SetAction(wchar_t* pszAction, size_t iTotalProgress);
	virtual void			WorkProgress(size_t iProgress, bool bForceDraw = false);
	virtual void			EndProgress();

	virtual bool			IsGenerating() { return m_oGenerator.IsGenerating(); }
	virtual bool			DoneGenerating() { return m_oGenerator.DoneGenerating(); }

	EVENT_CALLBACK(CAOPanel, Generate);
	EVENT_CALLBACK(CAOPanel, SaveMap);
	EVENT_CALLBACK(CAOPanel, AOMethod);

	virtual void			FindBestRayFalloff();

	static void				Open(bool bColor, CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);
	static CAOPanel*		Get(bool bColor);

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

	CLabel*					m_pFalloffLabel;
	CScrollSelector<float>*	m_pFalloffSelector;

	CLabel*					m_pRandomLabel;
	CCheckBox*				m_pRandomCheckBox;

	CLabel*					m_pCreaseLabel;
	CCheckBox*				m_pCreaseCheckBox;

	CLabel*					m_pGroundOcclusionLabel;
	CCheckBox*				m_pGroundOcclusionCheckBox;

	CButton*				m_pGenerate;
	CButton*				m_pSave;

	static CAOPanel*		s_pAOPanel;
	static CAOPanel*		s_pColorAOPanel;
};

class CNormalPanel : public CMovablePanel, public IWorkListener
{
public:
								CNormalPanel(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);

public:
	virtual void				SetVisible(bool bVisible);

	virtual void				Layout();
	virtual void				UpdateScene();

	virtual void				Think();

	virtual void				Paint(int x, int y, int w, int h);

	virtual bool				KeyPressed(int iKey);

	virtual void				BeginProgress();
	virtual void				SetAction(wchar_t* pszAction, size_t iTotalProgress);
	virtual void				WorkProgress(size_t iProgress, bool bForceDraw = false);
	virtual void				EndProgress();

	virtual bool				IsGenerating() { return m_oGenerator.IsGenerating(); }
	virtual bool				DoneGenerating() { return m_oGenerator.DoneGenerating(); }

	EVENT_CALLBACK(CNormalPanel,	Generate);
	EVENT_CALLBACK(CNormalPanel,	SaveMap);
	EVENT_CALLBACK(CNormalPanel,	AddLoRes);
	EVENT_CALLBACK(CNormalPanel,	AddHiRes);
	EVENT_CALLBACK(CNormalPanel,	RemoveLoRes);
	EVENT_CALLBACK(CNormalPanel,	RemoveHiRes);
	EVENT_CALLBACK(CNormalPanel,	AddLoResMesh);
	EVENT_CALLBACK(CNormalPanel,	AddHiResMesh);
	EVENT_CALLBACK(CNormalPanel,	UpdateNormal2);

	static void					Open(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);
	static CNormalPanel*		Get() { return s_pNormalPanel; }

protected:
	CConversionScene*			m_pScene;
	std::vector<CMaterial>*		m_paoMaterials;

	CNormalGenerator			m_oGenerator;

	CLabel*						m_pSizeLabel;
	CScrollSelector<int>*		m_pSizeSelector;

	CLabel*						m_pLoResLabel;
	CTree*						m_pLoRes;

	CLabel*						m_pHiResLabel;
	CTree*						m_pHiRes;

	std::vector<CConversionMeshInstance*>	m_apLoResMeshes;
	std::vector<CConversionMeshInstance*>	m_apHiResMeshes;

	CButton*					m_pAddLoRes;
	CButton*					m_pAddHiRes;

	CButton*					m_pRemoveLoRes;
	CButton*					m_pRemoveHiRes;

	CLabel*						m_pTextureLabel;
	CCheckBox*					m_pTextureCheckBox;

	CScrollSelector<float>*		m_pHiDepthSelector;
	CLabel*						m_pHiDepthLabel;

	CScrollSelector<float>*		m_pLoDepthSelector;
	CLabel*						m_pLoDepthLabel;

	CButton*					m_pGenerate;
	CButton*					m_pSave;

	class CMeshInstancePicker*	m_pMeshInstancePicker;

	static CNormalPanel*		s_pNormalPanel;
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
