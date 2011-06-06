#ifndef LW_TINKER_CONSOLE
#define LW_TINKER_CONSOLE

#include <glgui/glgui.h>

class CConsole : public glgui::CPanel
{
	DECLARE_CLASS(CConsole, glgui::CPanel);

public:
							CConsole();

public:
	virtual void			Destructor();
	virtual void			Delete() { delete this; };

	virtual bool			IsVisible();
	virtual void			SetVisible(bool bVisible);
	virtual bool			IsOpen();

	virtual bool			IsCursorListener();

	virtual void			Layout();
	virtual void			Paint() { Paint(GetLeft(), GetTop(), GetWidth(), GetHeight()); };
	virtual void			Paint(int x, int y, int w, int h);

	void					PrintConsole(eastl::string16 sText);

	virtual bool			KeyPressed(int code, bool bCtrlDown = false);
	virtual bool			CharPressed(int iKey);

	virtual void			SetRenderBackground(bool bBackground) { m_bBackground = bBackground; }

protected:
	glgui::CLabel*			m_pOutput;
	glgui::CTextField*		m_pInput;

	bool					m_bBackground;
};

#endif
