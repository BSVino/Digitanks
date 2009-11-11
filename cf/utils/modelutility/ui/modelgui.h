#ifndef MODELGUI_H
#define MODELGUI_H

#include <vector>
#include <color.h>

namespace modelgui
{
	class CRect
	{
	public:
		CRect(int x, int y, int w, int h) { this->x = x; this->y = y; this->w = w; this->h = h; };
		CRect() { CRect(0, 0, 0, 0); };

		int Size() { return w*h; }

		int x;
		int y;
		int w;
		int h;
	};

	int ScreenWidth();
	int ScreenHeight();

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

		virtual bool		IsVisible()=0;
		virtual void		SetVisible(bool bVisible)=0;
		virtual void		Paint()=0;
		virtual void		Paint(int x, int y)=0;
		virtual void		Paint(int x, int y, int w, int h)=0;
		virtual void		Layout()=0;
		virtual void		Think()=0;

		virtual bool		KeyPressed(int iKey)=0;
		virtual bool		KeyReleased(int iKey)=0;
		virtual bool		MousePressed(int iButton)=0;
		virtual bool		MouseReleased(int iButton)=0;
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

		virtual void	Paint() {};
		virtual void	Paint(int x, int y) {};
		virtual void	Paint(int x, int y, int w, int h) {};
		virtual void	Layout() {};
		virtual void	Think() {};

		virtual void	SetSize(int w, int h) { m_iW = w; m_iH = h; };
		virtual void	SetPos(int x, int y) { m_iX = x; m_iY = y; };
		virtual void	GetSize(int &w, int &h) { w = m_iW; h = m_iH; };
		virtual void	GetPos(int &x, int &y) { x = m_iX; y = m_iY; };
		virtual void	GetAbsPos(int &x, int &y);
		virtual void	GetAbsDimensions(int &x, int &y, int &w, int &h);
		virtual int		GetWidth() { return m_iW; };
		virtual int		GetHeight() { return m_iH; };
		virtual void	SetDimensions(int x, int y, int w, int h) { m_iX = x; m_iY = y; m_iW = w; m_iH = h; };	// Local space
		virtual void	SetDimensions(const CRect& Dims) { SetDimensions(Dims.x, Dims.y, Dims.w, Dims.h); };	// Local space
		virtual void	GetBR(int &x, int &y) { x = m_iX + m_iW; y = m_iY + m_iH; };

		virtual void	SetVisible(bool bVis) { m_bVisible = bVis; };
		virtual bool	IsVisible();

		virtual void	LevelShutdown( void ) { return; };
		virtual bool	KeyPressed(int iKey) { return false; };
		virtual bool	KeyReleased(int iKey) { return false; };
		virtual bool	MousePressed(int iButton) { return false; };
		virtual bool	MouseReleased(int iButton) { return false; };
		virtual bool	IsCursorListener() { return false; };
		virtual void	CursorMoved(int x, int y) {};
		virtual void	CursorIn() {};
		virtual void	CursorOut() {};

#ifdef _DEBUG
		virtual void	PaintDebugRect(int x, int y, int w, int h);
#endif

	protected:
		IControl*		m_pParent;

		int				m_iX;
		int				m_iY;
		int				m_iW;
		int				m_iH;

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

		virtual bool			KeyPressed(int code);
		virtual bool			KeyReleased(int code);
		virtual bool			MousePressed(int code);
		virtual bool			MouseReleased(int code);
		virtual bool			IsCursorListener() {return true;};
		virtual void			CursorMoved(int mx, int my);
		virtual void			CursorOut();

		virtual void			AddControl(IControl* pControl, bool bToTail = false);
		virtual void			RemoveControl(IControl* pControl);

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

		virtual bool		MousePressed(int code);

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
		virtual void				Paint();
		virtual void				Layout();

		virtual bool				MousePressed(int code);
		virtual bool				MouseReleased(int code);
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

		void						AddMenu(const char* pszTitle);

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
			TA_LEFTCENTER	= 1,
			TA_MIDDLECENTER	= 2,
			TA_RIGHTCENTER	= 3,
		} TextAlign;

		virtual void	Paint() { int x = 0, y = 0; GetAbsPos(x, y); Paint(x, y); };
		virtual void	Paint(int x, int y) { Paint(x, y, m_iW, m_iH); };
		virtual void	Paint(int x, int y, int w, int h);
		virtual void	DrawLine(wchar_t* pszText, unsigned iLength, int x, int y, int w, int h);
		virtual void	Layout() {};
		virtual void	Think() {};

		virtual void	SetSize(int w, int h);

		virtual bool	MousePressed(int code) {return false;};
		virtual bool	MouseReleased(int code) {return false;};
		virtual bool	IsCursorListener() {return false;};

		virtual bool	IsEnabled() {return m_bEnabled;};
		virtual void	SetEnabled(bool bEnabled) {m_bEnabled = bEnabled;};

		virtual TextAlign	GetAlign() { return m_eAlign; };
		virtual void	SetAlign(TextAlign eAlign) { m_eAlign = eAlign; };

		virtual bool	GetWrap() { return m_bWrap; };
		virtual void	SetWrap(bool bWrap) { m_bWrap = bWrap; };

		virtual void	SetText(const wchar_t* pszText);
		virtual void	SetText(const char* pszText);
		virtual void	AppendText(const char* pszText);
		virtual void	AppendText(const wchar_t* pszText);
		virtual const wchar_t*	GetText();

		virtual int		GetTextWidth();
		virtual int		GetTextHeight();
		virtual void	ComputeLines(int w = -1, int h = -1);
		virtual void	EnsureTextFits();
		static int		GetTextWidth( /*vgui::HFont& font,*/ const wchar_t *str, int iLength );

		virtual Color	GetFGColor();
		virtual void	SetFGColor(Color FGColor);

	protected:
		bool			m_bEnabled;
		bool			m_bWrap;
		wchar_t*		m_pszText;
		Color			m_FGColor;

		TextAlign		m_eAlign;

		int				m_iTotalLines;
		int				m_iLine;
	};

#if 0 // Until I find a proper way to deal with fonts.
	class CButton : public CLabel
	{
		friend CRootPanel;
		friend class CSlidingPanel;

	public:
						CButton(int x, int y, int w, int h, char* szText, bool bToggle = false);
		virtual void	Destructor();
		virtual void	Delete() { delete this; };

		virtual void	Paint() { CLabel::Paint(); };
		virtual void	Paint(int x, int y, int w, int h);
		virtual void	PaintButton(int x, int y, int w, int h);

		virtual bool	MousePressed(int code);
		virtual bool	MouseReleased(int code);
		virtual bool	IsCursorListener() {return true;};
		virtual void	CursorIn();
		virtual void	CursorOut();

		virtual bool	IsToggleButton() {return m_bToggle;};
		virtual void	SetToggleState(bool bState);
		virtual bool	GetToggleState() {return m_bToggleOn;};

		virtual bool	Push();
		virtual bool	Pop(bool bRegister = true, bool bRevert = false);
		virtual void	SetState(bool bDown, bool bRegister = true);
		virtual bool	GetState() {return m_bDown;};

		virtual void	SetClickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		// Toggle buttons only
		virtual void	SetUnclickedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		virtual bool	IsHighlighted() {return m_bHighlight;};

	protected:
		bool			m_bToggle;
		bool			m_bToggleOn;
		bool			m_bDown;
		bool			m_bHighlight;

		// Need multiple event listeners? Too bad! Make a list.
		IEventListener::Callback m_pfnClickCallback;
		IEventListener*	m_pClickListener;
		KeyValues*		m_pClickParms;

		IEventListener::Callback m_pfnUnclickCallback;
		IEventListener*	m_pUnclickListener;
		KeyValues*		m_pUnclickParms;
	};
#endif

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

		virtual bool				MousePressed(int iButton);

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

	class CMenuBar : public CPanel
	{
	public:
									CMenuBar();

		void						Layout();
	};

	class CMenu : public CLabel
	{
	public:
									CMenu(const char* pszTitle);

		virtual void				Think();
		virtual void				Paint(int x, int y, int w, int h);

		virtual bool				IsCursorListener() { return true; };
		virtual void				CursorIn() { m_flHighlightGoal = 1; };
		virtual void				CursorOut() { m_flHighlightGoal = 0; };

	protected:
		float						m_flHighlightGoal;
		float						m_flHighlight;
	};

};

#endif
