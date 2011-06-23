#ifndef DT_INTRO_WINDOW_H
#define DT_INTRO_WINDOW_H

#include <tinker/gamewindow.h>

class CIntroWindow : public CGameWindow
{
	DECLARE_CLASS(CIntroWindow, CGameWindow);

public:
								CIntroWindow(int argc, char** argv);

public:
	virtual eastl::string		WindowTitle() { return "Digitanks!"; }
	virtual tstring				AppDirectory() { return _T("Digitanks"); }

	void						SetScreenshot(size_t iScreenshot) { m_iScreenshot = iScreenshot; };
	void						SetupEngine();
	void						SetupIntro();

	virtual void				RenderLoading();

	virtual void				DoKeyPress(int c);

	class CGeneralWindow*		GetGeneralWindow() { return m_pGeneralWindow; }

	class CIntroRenderer*		GetRenderer();

protected:
	size_t						m_iScreenshot;

	class CGeneralWindow*		m_pGeneralWindow;
};

inline CIntroWindow* IntroWindow()
{
	return static_cast<CIntroWindow*>(CApplication::Get());
}

#endif
