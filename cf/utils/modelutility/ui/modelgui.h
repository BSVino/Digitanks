#ifndef MODELGUI_H
#define MODELGUI_H

#include <vector>
#include <color.h>
#include <FTGL/ftgl.h>
#include <maths.h>

// Not my favorite hack.
#define EVENT_CALLBACK(type, pfn) \
	void pfn##Callback(); \
	static void pfn(modelgui::IEventListener* obj) \
	{ \
		((type*)obj)->pfn##Callback(); \
	}

namespace modelgui
{
	extern Color g_clrPanel;
	extern Color g_clrBox;
	extern Color g_clrBoxHi;

	class CRect
	{
	public:
		CRect(float x, float y, float w, float h) { this->x = x; this->y = y; this->w = w; this->h = h; };
		CRect() { CRect(0, 0, 0, 0); };

		float Size() { return w*h; }

		float x;
		float y;
		float w;
		float h;
	};

	class IPopup
	{
	public:
		virtual void		Open()=0;
		virtual void		Close()=0;
	
		virtual void		GetAbsDimensions(int &x, int &y, int &w, int &h)=0;	// Screen space

		virtual void		SetVisible(bool bVisible)=0;
	};

	class IControl
	{
	public:
		virtual IControl*	GetParent()=0;
		virtual void		SetParent(IControl* pParent)=0;

		virtual void		LevelShutdown()=0;
		virtual void		LoadTextures()=0;

		virtual void		SetSize(int w, int h)=0;
		virtual void		SetPos(int x, int y)=0;
		virtual void		GetSize(int &w, int &h)=0;
		virtual void		GetPos(int &x, int &y)=0;
		virtual void		GetAbsPos(int &x, int &y)=0;	// Screen space
		virtual void		GetAbsDimensions(int &x, int &y, int &w, int &h)=0;	// Screen space
		virtual void		GetBR(int &x, int &y)=0;
		virtual int			GetWidth()=0;
		virtual int			GetHeight()=0;
		virtual void		SetAlpha(int a)=0;
		virtual int			GetAlpha()=0;

		virtual bool		IsVisible()=0;
		virtual void		SetVisible(bool bVisible)=0;
		virtual void		Paint()=0;
		virtual void		Paint(int x, int y)=0;
		virtual void		Paint(int x, int y, int w, int h)=0;
		virtual void		Layout()=0;
		virtual void		Think()=0;
		virtual void		UpdateScene()=0;

		virtual bool		KeyPressed(int iKey)=0;
		virtual bool		KeyReleased(int iKey)=0;
		virtual bool		MousePressed(int iButton, int mx, int my)=0;
		virtual bool		MouseReleased(int iButton, int mx, int my)=0;
		virtual bool		IsCursorListener()=0;
		virtual void		CursorMoved(int x, int y)=0;
		virtual void		CursorIn()=0;
		virtual void		CursorOut()=0;

		virtual void		Destructor()=0;
		virtual void		Delete()=0;
	};

	class IDroppable;

	// An object that can be grabbed and dragged around the screen.
	class IDraggable
	{
	public:
		typedef enum
		{
			DC_UNSPECIFIED	= 0,
			DC_WEAPONICON,
			DC_RUNEICON,
			DC_RUNECOMBO,
		} DragClass_t;		// Where the hookers go to learn their trade.
		virtual void			Destructor()=0;
		virtual void			Delete()=0;

		virtual void			SetHoldingRect(const CRect)=0;
		virtual CRect			GetHoldingRect()=0;

		virtual IDroppable*		GetDroppable()=0;
		virtual void			SetDroppable(IDroppable* pDroppable)=0;

		virtual void			Paint()=0;
		virtual void			Paint(int x, int y)=0;
		virtual void			Paint(int x, int y, int w, int h)=0;
		virtual void			Paint(int x, int y, int w, int h, bool bFloating)=0;

		virtual DragClass_t		GetClass()=0;
		virtual IDraggable&		MakeCopy()=0;
	};

	// A place where an IDraggable is allowed to be dropped.
	class IDroppable
	{
	public:
		virtual void			Destructor()=0;
		virtual void			Delete()=0;

		virtual IControl*		GetParent()=0;
		virtual void			SetParent(IControl* pParent)=0;

		// Get the place where a droppable object should be.
		virtual const CRect		GetHoldingRect()=0;

		virtual void			AddDraggable(IDraggable*)=0;
		virtual void			SetDraggable(IDraggable*, bool bDelete = true)=0;
		virtual void			ClearDraggables(bool bDelete = true)=0;
		virtual IDraggable*		GetDraggable(int i)=0;
		virtual IDraggable*		GetCurrentDraggable()=0;

		// I already know.
		virtual void			SetGrabbale(bool bGrabbable)=0;
		virtual bool			IsGrabbale()=0;

		virtual bool			CanDropHere(IDraggable*)=0;

		// Is this droppable a bottomless pit of draggables?
		virtual bool			IsInfinite()=0;

		virtual bool			IsVisible()=0;
	};

	class IEventListener
	{
	public:
		typedef void (*Callback)(IEventListener*);
	};

	class CBaseControl : public IControl
	{
	public:
						CBaseControl(int x, int y, int w, int h);
						CBaseControl(const CRect& Rect);

		virtual void	Destructor();
		virtual void	Delete() { delete this; };

		virtual IControl*	GetParent() { return m_pParent; };
		virtual void	SetParent(IControl* pParent) { m_pParent = pParent; };

		virtual void	LoadTextures() {};

		virtual void	Paint();
		virtual void	Paint(int x, int y);
		virtual void	Paint(int x, int y, int w, int h) {};
		virtual void	Layout() {};
		virtual void	Think() {};
		virtual void	UpdateScene() {};

		virtual void	SetSize(int w, int h) { m_iW = w; m_iH = h; };
		virtual void	SetPos(int x, int y) { m_iX = x; m_iY = y; };
		virtual void	GetSize(int &w, int &h) { w = m_iW; h = m_iH; };
		virtual void	GetPos(int &x, int &y) { x = m_iX; y = m_iY; };
		virtual void	GetAbsPos(int &x, int &y);
		virtual void	GetAbsDimensions(int &x, int &y, int &w, int &h);
		virtual int		GetWidth() { return m_iW; };
		virtual int		GetHeight() { return m_iH; };
		virtual void	SetDimensions(int x, int y, int w, int h) { m_iX = x; m_iY = y; m_iW = w; m_iH = h; };	// Local space
		virtual void	SetDimensions(const CRect& Dims) { SetDimensions((int)Dims.x, (int)Dims.y, (int)Dims.w, (int)Dims.h); };	// Local space
		virtual void	GetBR(int &x, int &y) { x = m_iX + m_iW; y = m_iY + m_iH; };
		virtual void	SetAlpha(int a) { m_iAlpha = a; };
		virtual int		GetAlpha() { return m_iAlpha; };
		virtual void	SetRight(int r);
		virtual void	SetBottom(int b);
		virtual int		GetLeft() { return m_iX; };
		virtual int		GetTop() { return m_iY; };
		virtual int		GetRight() { return m_iX + m_iW; };
		virtual int		GetBottom() { return m_iY + m_iH; };

		virtual void	SetVisible(bool bVis) { m_bVisible = bVis; };
		virtual bool	IsVisible();

		virtual void	LevelShutdown( void ) { return; };
		virtual bool	KeyPressed(int iKey) { return false; };
		virtual bool	KeyReleased(int iKey) { return false; };
		virtual bool	MousePressed(int iButton, int mx, int my) { return false; };
		virtual bool	MouseReleased(int iButton, int mx, int my) { return false; };
		virtual bool	IsCursorListener() { return false; };
		virtual void	CursorMoved(int x, int y) {};
		virtual void	CursorIn() {};
		virtual void	CursorOut() {};

		static void		PaintRect(int x, int y, int w, int h, Color& c = g_clrBox);

	protected:
		IControl*		m_pParent;

		int				m_iX;
		int				m_iY;
		int				m_iW;
		int				m_iH;

		int				m_iAlpha;

		bool			m_bVisible;
	};

	// A panel is a container for other controls. It is for organization
	// purposes only; it does not currently keep its children from drawing
	// outside of it.
	class CPanel : public CBaseControl
	{

#ifdef _DEBUG
		// Just so CBaseControl can get at CPanel's textures for the purpose of debug paint methods.
		friend class CBaseControl;
#endif

	public:
								CPanel(int x, int y, int w, int h);
		virtual void			Destructor();
		virtual void			Delete() { delete this; };

		virtual void			Paint();
		virtual void			Paint(int x, int y);
		virtual void			Paint(int x, int y, int w, int h);
		virtual void			Layout();
		virtual void			Think();
		virtual void			UpdateScene();

		virtual bool			KeyPressed(int code);
		virtual bool			KeyReleased(int code);
		virtual bool			MousePressed(int code, int mx, int my);
		virtual bool			MouseReleased(int code, int mx, int my);
		virtual bool			IsCursorListener() {return true;};
		virtual void			CursorMoved(int mx, int my);
		virtual void			CursorOut();

		virtual void			AddControl(IControl* pControl, bool bToTail = false);
		virtual void			RemoveControl(IControl* pControl);
		virtual std::vector<IControl*>&	GetControls() { return m_apControls; };

		virtual void			SetHighlighted(bool bHighlight) { m_bHighlight = bHighlight; };
		virtual bool			IsHighlighted() { return m_bHighlight; };

		typedef enum
		{
			BT_NONE	= 0,
			BT_SOME = 1
		} Border;

		void					SetBorder(Border b) { m_eBorder = b; };

	protected:
		virtual void			PaintBorder(int x, int y, int w, int h);

		std::vector<IControl*>	m_apControls;

		// If two controls in the same panel are never layered, a single
		// pointer should suffice. Otherwise a list must be created.
		IControl*				m_pHasCursor;

		Border					m_eBorder;

		bool					m_bHighlight;
		bool					m_bDestructing;
	};

	class CDroppablePanel : public CPanel, public IDroppable
	{
	public:
							CDroppablePanel(int x, int y, int w, int h);
		virtual void		Destructor();
		virtual void		Delete() { delete this; };

		// It's already in CBaseControl, but we need this again for IDroppable.
		virtual IControl*	GetParent() { return CPanel::GetParent(); };
		virtual void		SetParent(IControl* pParent) { return CPanel::SetParent(pParent); };
		virtual bool		IsVisible() { return CPanel::IsVisible(); };

		virtual void		Paint(int x, int y, int w, int h);

		virtual void		SetSize(int w, int h);
		virtual void		SetPos(int x, int y);

		virtual bool		MousePressed(int code, int mx, int my);

		virtual void		AddDraggable(IDraggable* pDragged);
		virtual void		SetDraggable(IDraggable* pDragged, bool bDelete = true);
		virtual void		ClearDraggables(bool bDelete = true);
		virtual IDraggable*	GetDraggable(int i);

		virtual void		SetGrabbale(bool bGrabbable) { m_bGrabbable = bGrabbable; };
		virtual bool		IsGrabbale() { return m_bGrabbable; };

	protected:
		bool				m_bGrabbable;

		std::vector<IDraggable*>	m_apDraggables;
	};

	class CRootPanel : public CPanel
	{
	public:
									CRootPanel( );
									~CRootPanel( );
		virtual void				Destructor( );
		virtual void				Delete() { delete this; };

		virtual void				Think();
		virtual void				Paint(int x, int y, int w, int h);
		virtual void				Layout();

		virtual bool				MousePressed(int code, int mx, int my);
		virtual bool				MouseReleased(int code, int mx, int my);
		virtual void				CursorMoved(int mx, int my);

		// Dragon Drop stuff is in this class, because this is always the
		// top-level panel so all the messages go through it first.
		virtual void				DragonDrop(IDroppable* pDroppable);
		virtual void				AddDroppable(IDroppable* pDroppable);
		virtual void				RemoveDroppable(IDroppable* pDroppable);
		virtual bool				DropDraggable();
		virtual IDraggable*			GetCurrentDraggable() { return m_pDragging?m_pDragging->GetCurrentDraggable():NULL; };
		virtual IDroppable*			GetCurrentDroppable() { return m_pDragging; };

		virtual void				Popup(IPopup* pControl);

		void						SetButtonDown(class CButton* pButton);
		class CButton*				GetButtonDown();

		class CMenuBar*				GetMenuBar() { return m_pMenuBar; };
		class CMenu*				AddMenu(const char* pszTitle);

		float						GetFrameTime() { return m_flFrameTime; };

		static CRootPanel*			Get();

		// Should the window close after the selections have been made? (ie team choosing?)
		// If not, it should advance to the next window.
		static void					SetCloseAfter(bool bCloseAfter);
		static bool					GetCloseAfter();

		static void					GetFullscreenMousePos(int& mx, int& my);
		static void					DrawRect(int x, int y, int x2, int y2);

	private:
		static CRootPanel*			s_pRootPanel;

		std::vector<IDroppable*>	m_apDroppables;
		IDroppable*					m_pDragging;

		IPopup*						m_pPopup;

		// If the mouse is released over nothing, then try popping this button.
		CButton*					m_pButtonDown;

		class CMenuBar*				m_pMenuBar;

		float						m_flFrameTime;

		int							m_iMX;
		int							m_iMY;
	};

	class CLabel : public CBaseControl
	{
		friend CRootPanel;

	public:
						CLabel(int x, int y, int w, int h, const char* pszText);
		virtual void	Destructor();
		virtual void	Delete() { delete this; };

		typedef enum
		{
			TA_TOPLEFT		= 0,
			TA_TOPCENTER	= 1,
			TA_LEFTCENTER	= 2,
			TA_MIDDLECENTER	= 3,
			TA_RIGHTCENTER	= 4,
		} TextAlign;

		virtual void	Paint() { int x = 0, y = 0; GetAbsPos(x, y); Paint(x, y); };
		virtual void	Paint(int x, int y) { Paint(x, y, m_iW, m_iH); };
		virtual void	Paint(int x, int y, int w, int h);
		virtual void	DrawLine(wchar_t* pszText, unsigned iLength, int x, int y, int w, int h);
		virtual void	Layout() {};
		virtual void	Think() {};

		virtual void	SetSize(int w, int h);

		virtual bool	MousePressed(int code, int mx, int my) {return false;};
		virtual bool	MouseReleased(int code, int mx, int my) {return false;};
		virtual bool	IsCursorListener() {return false;};

		virtual bool	IsEnabled() {return m_bEnabled;};
		virtual void	SetEnabled(bool bEnabled) {m_bEnabled = bEnabled;};

		virtual TextAlign	GetAlign() { return m_eAlign; };
		virtual void	SetAlign(TextAlign eAlign) { m_eAlign = eAlign; };

		virtual bool	GetWrap() { return m_bWrap; };
		virtual void	SetWrap(bool bWrap) { m_bWrap = bWrap; ComputeLines(); };

		virtual void	SetText(const wchar_t* pszText);
		virtual void	SetText(const char* pszText);
		virtual void	AppendText(const char* pszText);
		virtual void	AppendText(const wchar_t* pszText);
		virtual const wchar_t*	GetText();

		virtual void	SetFontFaceSize(int iSize);

		virtual int		GetTextWidth();
		virtual float	GetTextHeight();
		virtual void	ComputeLines(int w = -1, int h = -1);
		virtual void	EnsureTextFits();
		static int		GetTextWidth( /*vgui::HFont& font,*/ const wchar_t *str, int iLength );

		virtual Color	GetFGColor();
		virtual void	SetFGColor(Color FGColor);
		virtual void	SetAlpha(int a);

	protected:
		bool			m_bEnabled;
		bool			m_bWrap;
		wchar_t*		m_pszText;
		Color			m_FGColor;

		TextAlign		m_eAlign;

		int				m_iTotalLines;
		int				m_iLine;

		FTGLPixmapFont*	m_pFont;
	};

	class CButton : public CLabel
	{
		friend CRootPanel;
		friend class CSlidingPanel;

	public:
						CButton(int x, int y, int w, int h, const char* szText, bool bToggle = false);
		virtual void	Destructor();
		virtual void	Delete() { delete this; };

		virtual void	Think();

		virtual void	Paint() { CLabel::Paint(); };
		virtual void	Paint(int x, int y, int w, int h);
		virtual void	PaintButton(int x, int y, int w, int h);

		virtual bool	MousePressed(int code, int mx, int my);
		virtual bool	MouseReleased(int code, int mx, int my);
		virtual bool	IsCursorListener() {return true;};
		virtual void	CursorIn();
		virtual void	CursorOut();

		virtual bool	IsToggleButton() {return m_bToggle;};
		virtual void	SetToggleButton(bool bToggle);
		virtual void	SetToggleState(bool bState);
		virtual bool	GetToggleState() {return m_bToggleOn;};

		virtual bool	Push();
		virtual bool	Pop(bool bRegister = true, bool bRevert = false);
		virtual void	SetState(bool bDown, bool bRegister = true);
		virtual bool	GetState() {return m_bDown;};

		virtual void	SetClickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		// Toggle buttons only
		virtual void	SetUnclickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		virtual bool	IsHighlighted() {return m_flHighlight > 0;};

	protected:
		bool			m_bToggle;
		bool			m_bToggleOn;
		bool			m_bDown;
		float			m_flHighlightGoal;
		float			m_flHighlight;

		// Need multiple event listeners? Too bad! Make a list.
		IEventListener::Callback m_pfnClickCallback;
		IEventListener*	m_pClickListener;

		IEventListener::Callback m_pfnUnclickCallback;
		IEventListener*	m_pUnclickListener;
	};

	class CPictureButton : public CButton
	{
	public:
						CPictureButton(const char* szText, size_t iTexture = 0, bool bToggle = false);
		virtual void	Delete() { delete this; };

	public:
		virtual void	Paint() { CButton::Paint(); };
		virtual void	Paint(int x, int y, int w, int h);

	protected:
		size_t			m_iTexture;
	};

	class CCheckBox : public CButton
	{
	public:
						CCheckBox();

	public:
		void			Paint(int x, int y, int w, int h);
	};

	class CSlidingContainer;
	class CSlidingPanel : public CPanel
	{
	public:
		friend CRootPanel;

		class CInnerPanel : public CPanel
		{
		public:
									CInnerPanel(CSlidingContainer* pMaster);
			
			virtual void			Delete() { delete this; };

			virtual bool			IsVisible();

			CSlidingContainer*		m_pMaster;
		};

									CSlidingPanel(CSlidingContainer* pParent, char* pszTitle);

		virtual void				Delete() { delete this; };

		virtual void				Layout();
		virtual void				Paint(int x, int y, int w, int h);

		virtual void				AddControl(IControl* pControl, bool bToTail = false);

		virtual bool				MousePressed(int iButton, int mx, int my);

		virtual void				SetCurrent(bool bCurrent);

//		virtual void				SetTitle(char* pszNew) { m_pTitle->SetText(pszNew); };
//		virtual void				SetTitle(wchar_t* pszNew) { m_pTitle->SetText(pszNew); };
//		virtual void				AppendTitle(char* pszNew) { m_pTitle->AppendText(pszNew); };
//		virtual void				AppendTitle(wchar_t* pszNew) { m_pTitle->AppendText(pszNew); };

		static const int			SLIDER_COLLAPSED_HEIGHT = 30;

	protected:
		bool						m_bCurrent;

//		CLabel*						m_pTitle;

		CPanel*						m_pInnerPanel;
	};

	class CSlidingContainer : public CPanel
	{
	public:
									CSlidingContainer(int x, int y, int w, int h);

		virtual void				Delete() { delete this; };

		virtual void				Layout();

		virtual void				AddControl(IControl* pControl, bool bToTail = false);

		virtual bool				IsCurrent(int iPanel);
		virtual void				SetCurrent(int iPanel);
		virtual bool				IsCurrent(CSlidingPanel* pPanel);
		virtual void				SetCurrent(CSlidingPanel* pPanel);

		virtual bool				IsCurrentValid();

		virtual int					VisiblePanels();

	protected:
		int							m_iCurrent;
	};

#define MENU_SPACING 10
#define MENU_HEIGHT 22

	class CMenuBar : public CPanel
	{
	public:
									CMenuBar();

		void						Layout();

		void						SetActive(CMenu* pMenu);
	};

	class CMenu : public CButton, public IEventListener
	{
	public:
									CMenu(const char* pszTitle, bool bSubmenu = false);

		virtual void				Think();
		virtual void				Layout();
		virtual void				Paint(int x, int y, int w, int h);

		virtual bool				IsCursorListener() { return true; };
		virtual void				CursorIn();
		virtual void				CursorOut();

		virtual void				SetMenuListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		EVENT_CALLBACK(CMenu, Open);
		EVENT_CALLBACK(CMenu, Close);
		EVENT_CALLBACK(CMenu, Clicked);

		virtual void				AddSubmenu(const char* pszTitle, IEventListener* pListener = NULL, IEventListener::Callback pfnCallback = NULL);

	protected:
		class CSubmenuPanel : public CPanel
		{
		public:
									CSubmenuPanel();

			void					Think();

			void					SetFakeHeight(float flFakeHeight) { m_flFakeHeight = flFakeHeight; };

		protected:
			float					m_flFakeHeight;

			std::vector<float>		m_aflControlHighlightGoal;
			std::vector<float>		m_aflControlHighlight;
		};

		bool						m_bSubmenu;

		float						m_flHighlightGoal;
		float						m_flHighlight;

		float						m_flMenuHighlightGoal;
		float						m_flMenuHighlight;
		float						m_flMenuHeightGoal;
		float						m_flMenuHeight;
		float						m_flMenuSelectionHighlightGoal;
		float						m_flMenuSelectionHighlight;
		CRect						m_MenuSelectionGoal;
		CRect						m_MenuSelection;

		IEventListener::Callback	m_pfnMenuCallback;
		IEventListener*				m_pMenuListener;

		CSubmenuPanel*				m_pMenu;

		std::vector<CMenu*>			m_apEntries;
	};

	template <typename T>
	class CScrollSelection
	{
	public:
		CScrollSelection(T oParam, wchar_t* szLabel)
		{
			m_oParam = oParam;
			m_sLabel = szLabel;
		}

		T									m_oParam;
		std::wstring						m_sLabel;
	};

	template <typename T>
	class CScrollSelector : public CPanel
	{
	public:
		CScrollSelector()
			: CPanel(0, 0, 100, 16)
		{
			m_flHandlePositionGoal = 0;
			m_flHandlePosition = 0;
			m_bMovingHandle = false;

			m_iSelection = 0;

			m_pfnSelectedCallback = NULL;
			m_pSelectedListener = NULL;

			m_pOption = new CLabel(0, 0, 100, 100, "");
			m_pOption->SetWrap(false);
			AddControl(m_pOption);
		}

		virtual void Layout()
		{
			m_pOption->SetSize(1, 1);

			// Make sure there's some text to be fit to.
			if (wcscmp(m_pOption->GetText(), L"") == 0)
				m_pOption->SetText(L"-");

			m_pOption->EnsureTextFits();
			m_pOption->SetPos(GetWidth()/2 - m_pOption->GetWidth()/2, GetHeight()-15);

			CPanel::Layout();
		}

		virtual void Think()
		{
			if (m_bMovingHandle)
			{
				int mx, my;
				CRootPanel::GetFullscreenMousePos(mx, my);

				int x, y, w, h;
				GetAbsDimensions(x, y, w, h);

				m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x, (float)(x + w), 0.0f, 1.0f);
			}
			else
				m_flHandlePositionGoal = ((float)GetWidth()/((float)m_aSelections.size()-1)*(float)m_iSelection)/GetWidth();

			m_flHandlePosition = Approach(m_flHandlePositionGoal, m_flHandlePosition, CRootPanel::Get()->GetFrameTime()*10);

			int iSelection = SelectionByHandle();
			m_pOption->SetText(m_aSelections[iSelection].m_sLabel.c_str());

			if (iSelection != m_iSelection)
			{
				m_iSelection = iSelection;
				if (m_pSelectedListener)
					m_pfnSelectedCallback(m_pSelectedListener);
			}
		}

#define HANDLE_SIZE 12

		virtual void Paint(int x, int y, int w, int h)
		{
			//CRootPanel::PaintRect(x, y, w, h, g_clrBoxHi);

			int iLeft = x+HANDLE_SIZE/2;
			int iWidth = w-HANDLE_SIZE;

			CRootPanel::PaintRect(iLeft, y+h/2, iWidth, 1, Color(200, 200, 200, 255));

			for (size_t i = 0; i < m_aSelections.size(); i++)
				CRootPanel::PaintRect(iLeft + iWidth*(int)i/((int)m_aSelections.size()-1), y+h/2-5, 1, 10, Color(200, 200, 200, 255));

			CRootPanel::PaintRect(HandleX()+2, HandleY()+2, HANDLE_SIZE-4, HANDLE_SIZE-4, g_clrBoxHi);

			CPanel::Paint(x, y, w, h);
		}

		virtual bool MousePressed(int code, int mx, int my)
		{
			int x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			int hx, hy;
			hx = HandleX();
			hy = HandleY();

			if (mx >= hx && mx < hx + HANDLE_SIZE && my >= hy && my < hy + HANDLE_SIZE)
				m_bMovingHandle = true;
			else
			{
				m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x + HANDLE_SIZE/2, (float)(x + w - HANDLE_SIZE/2), 0.0f, 1.0f);
				m_iSelection = SelectionByHandle();

				if (m_pSelectedListener)
					m_pfnSelectedCallback(m_pSelectedListener);
			}

			return true;
		}

		virtual bool MouseReleased(int code, int mx, int my)
		{
			int x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			if (m_bMovingHandle)
			{
				DoneMovingHandle();
				return true;
			}

			return CPanel::MouseReleased(code, mx, my);
		}

		virtual void CursorOut()
		{
			if (m_bMovingHandle)
			{
				int mx, my;
				CRootPanel::GetFullscreenMousePos(mx, my);

				int x, y, w, h;
				GetAbsDimensions(x, y, w, h);

				// If the mouse went out of the left or right side, make sure we're all the way to that side.
				m_flHandlePositionGoal = RemapValClamped((float)mx, (float)x, (float)(x + w), 0.0f, 1.0f);

				DoneMovingHandle();
			}
		}

		virtual void DoneMovingHandle()
		{
			m_bMovingHandle = false;

			m_iSelection = SelectionByHandle();

			if (m_pSelectedListener)
				m_pfnSelectedCallback(m_pSelectedListener);
		}

		virtual void AddSelection(CScrollSelection<T>& oSelection)
		{
			m_aSelections.push_back(oSelection);
		}

		virtual void SetSelection(size_t i)
		{
			if (i >= m_aSelections.size())
				i = m_aSelections.size() - 1;

			m_iSelection = i;
			m_flHandlePositionGoal = m_flHandlePosition = ((float)GetWidth()/((float)m_aSelections.size()-1)*(float)m_iSelection)/GetWidth();

			if (m_pSelectedListener)
				m_pfnSelectedCallback(m_pSelectedListener);
		}

		virtual T GetSelectionValue()
		{
			return m_aSelections[m_iSelection].m_oParam;
		}

		virtual size_t FindClosestSelectionValue(float flValue)
		{
			size_t iClosest;
			T flClosestValue;
			for (size_t i = 0; i < m_aSelections.size(); i++)
			{
				if (i == 0)
				{
					flClosestValue = m_aSelections[0].m_oParam;
					iClosest = 0;
					continue;
				}

				if (fabs((float)(m_aSelections[i].m_oParam - flValue)) < fabs((float)(flClosestValue - flValue)))
				{
					flClosestValue = m_aSelections[i].m_oParam;
					iClosest = i;
				}
			}

			return iClosest;
		}

		virtual int SelectionByHandle()
		{
			int iSelection = (int)(m_flHandlePositionGoal*m_aSelections.size());

			if (iSelection < 0)
				return 0;

			if (iSelection >= (int)m_aSelections.size())
				return (int)m_aSelections.size()-1;

			return iSelection;
		}

		virtual int HandleX()
		{
			int x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			int iLeft = x+HANDLE_SIZE/2;
			int iWidth = w-HANDLE_SIZE;
			return iLeft + (int)(iWidth*m_flHandlePosition) - HANDLE_SIZE/2;
		}

		virtual int HandleY()
		{
			int x, y, w, h;
			GetAbsDimensions(x, y, w, h);

			return y+h/2-HANDLE_SIZE/2;
		}

		void SetSelectedListener(IEventListener* pListener, IEventListener::Callback pfnCallback)
		{
			m_pfnSelectedCallback = pfnCallback;
			m_pSelectedListener = pListener;
		}

	protected:
		std::vector<CScrollSelection<T>>	m_aSelections;

		CLabel*								m_pOption;

		size_t								m_iSelection;

		float								m_flHandlePosition;
		float								m_flHandlePositionGoal;

		bool								m_bMovingHandle;

		IEventListener::Callback			m_pfnSelectedCallback;
		IEventListener*						m_pSelectedListener;
	};

	class CTreeNode : public CPanel, public modelgui::IEventListener
	{
	public:
											CTreeNode(CTreeNode* pParent, class CTree* pTree, const std::wstring& sText);

	public:
		virtual void						Destructor();
		virtual void						Delete() { delete this; };

		virtual void						LayoutNode();
		void								Paint() { CPanel::Paint(); };
		void								Paint(int x, int y, int w, int h);

		size_t								AddNode(const std::wstring& sName);
		template <typename T>
		size_t								AddNode(const std::wstring& sName, T* pObject)
		{
			// How the hell does this resolve CTreeNodeObject when that class is below this one in the file?
			// Who the hell knows, it's the magick of templates.
			return AddNode(new CTreeNodeObject<T>(pObject, this, m_pTree, sName));
		}
		size_t								AddNode(CTreeNode* pNode);
		void								RemoveNode(CTreeNode* pNode);
		CTreeNode*							GetNode(size_t i);
		size_t								GetNumNodes() { return m_apNodes.size(); };

		virtual void						AddVisibilityButton() {};

		virtual void						Selected();

		bool								IsExpanded() { return m_pExpandButton->IsExpanded(); };
		void								SetExpanded(bool bExpanded) { m_pExpandButton->SetExpanded(bExpanded); };

		void								SetIcon(size_t iTexture) { m_iIconTexture = iTexture; };

		virtual bool						IsVisible();

		EVENT_CALLBACK(CTreeNode, Expand);

	public:
		std::vector<CTreeNode*>				m_apNodes;
		CTreeNode*							m_pParent;
		class CTree*						m_pTree;
		CLabel*								m_pLabel;

		size_t								m_iIconTexture;

		CPictureButton*						m_pVisibilityButton;
		CPictureButton*						m_pEditButton;

		class CExpandButton : public CPictureButton
		{
		public:
											CExpandButton(size_t iTexture);

		public:
			void							Think();
			void							Paint() { CButton::Paint(); };
			void							Paint(int x, int y, int w, int h);

			bool							IsExpanded() { return m_bExpanded; };
			void							SetExpanded(bool bExpanded);

		public:
			bool							m_bExpanded;
			float							m_flExpandedCurrent;
			float							m_flExpandedGoal;
		};

		CExpandButton*						m_pExpandButton;
	};

	class CTree : public CPanel
	{
		friend class CTreeNode;

	public:
											CTree(size_t iArrowTexture, size_t iEditTexture, size_t iVisibilityTexture);

	public:
		virtual void						Destructor();
		virtual void						Delete() { delete this; };

		virtual void						Layout();
		virtual void						Think();
		virtual void						Paint();
		virtual void						Paint(int x, int y);
		virtual void						Paint(int x, int y, int w, int h);

		virtual bool						MousePressed(int code, int mx, int my);

		void								ClearTree();

		size_t								AddNode(const std::wstring& sName);
		template <typename T>
		size_t								AddNode(const std::wstring& sName, T* pObject)
		{
			// How the hell does this resolve CTreeNodeObject when that class is below this one in the file?
			// Who the hell knows, it's the magick of templates.
			return AddNode(new CTreeNodeObject<T>(pObject, NULL, this, sName));
		}
		size_t								AddNode(CTreeNode* pNode);
		void								RemoveNode(CTreeNode* pNode);
		CTreeNode*							GetNode(size_t i);

		virtual CTreeNode*					GetSelectedNode() { if (m_iSelected == ~0) return NULL; return dynamic_cast<CTreeNode*>(m_apControls[m_iSelected]); };
		virtual void						SetSelectedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		void								SetBackgroundColor(const Color& clrBackground) { m_clrBackground = clrBackground; }

	public:
		std::vector<CTreeNode*>				m_apNodes;

		int									m_iCurrentHeight;
		int									m_iCurrentDepth;

		size_t								m_iHilighted;
		size_t								m_iSelected;

		size_t								m_iArrowTexture;
		size_t								m_iVisibilityTexture;
		size_t								m_iEditTexture;

		IEventListener::Callback			m_pfnSelectedCallback;
		IEventListener*						m_pSelectedListener;

		Color								m_clrBackground;
	};

	template <typename T>
	class CTreeNodeObject : public CTreeNode
	{
	public:
		CTreeNodeObject(T* pObject, CTreeNode* pParent, class CTree* pTree, const std::wstring& sName)
			: CTreeNode(pParent, pTree, sName)
		{
			m_pObject = pObject;
		}

	public:
		typedef void (*EditFnCallback)(T*);

		virtual void LayoutNode()
		{
			CTreeNode::LayoutNode();

			int iHeight = (int)m_pLabel->GetTextHeight();

			if (m_pVisibilityButton)
			{
				m_pVisibilityButton->SetPos(GetWidth()-iHeight-14, 0);
				m_pVisibilityButton->SetSize(iHeight, iHeight);
			}

			if (m_pEditButton)
			{
				m_pEditButton->SetPos(GetWidth()-iHeight*2-16, 0);
				m_pEditButton->SetSize(iHeight, iHeight);
			}

			m_pLabel->SetAlpha(m_pObject->IsVisible()?255:100);
		}

		virtual void AddVisibilityButton()
		{
			m_pVisibilityButton = new CPictureButton("@", m_pTree->m_iVisibilityTexture);
			m_pVisibilityButton->SetClickedListener(this, Visibility);
			AddControl(m_pVisibilityButton);
		}

		virtual void AddEditButton(EditFnCallback pfnCallback)
		{
			m_pEditButton = new CPictureButton("*", m_pTree->m_iEditTexture);
			m_pEditButton->SetClickedListener(this, Edit);
			AddControl(m_pEditButton);
			m_pfnCallback = pfnCallback;
		}

		virtual T* GetObject() { return m_pObject; }

		EVENT_CALLBACK(CTreeNodeObject, Visibility);
		EVENT_CALLBACK(CTreeNodeObject, Edit);

	protected:
		T*									m_pObject;

		EditFnCallback						m_pfnCallback;
	};

	template <typename T>
	inline void CTreeNodeObject<T>::VisibilityCallback()
	{
		m_pObject->SetVisible(!m_pObject->IsVisible());

		m_pLabel->SetAlpha(m_pObject->IsVisible()?255:100);
	}

	template <typename T>
	inline void CTreeNodeObject<T>::EditCallback()
	{
		m_pfnCallback(m_pObject);
	}
};

#endif
