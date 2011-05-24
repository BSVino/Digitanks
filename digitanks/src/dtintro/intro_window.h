#ifndef DT_INTRO_WINDOW_H
#define DT_INTRO_WINDOW_H

#include <tinker/gamewindow.h>

class CIntroWindow : public CGameWindow
{
	DECLARE_CLASS(CIntroWindow, CGameWindow);

public:
								CIntroWindow(int argc, char** argv) : CGameWindow(argc, argv) {};

public:
	virtual eastl::string		WindowTitle() { return "Digitanks!"; }
	virtual eastl::string16		AppDirectory() { return L"Digitanks"; }

	void						SetScreenshot(size_t iScreenshot) { m_iScreenshot = iScreenshot; };
	void						SetupEngine();
	void						SetupIntro();

	virtual void				RenderLoading();

	virtual void				DoKeyPress(int c);

protected:
	size_t						m_iScreenshot;
};

#endif
