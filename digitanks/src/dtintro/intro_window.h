#ifndef DT_INTRO_WINDOW_H
#define DT_INTRO_WINDOW_H

#include <tengine/ui/gamewindow.h>

class CIntroWindow : public CGameWindow
{
	DECLARE_CLASS(CIntroWindow, CGameWindow);

public:
								CIntroWindow(int argc, char** argv);

public:
	virtual tstring				WindowTitle() { return "Digitanks!"; }
	virtual tstring				AppDirectory() { return "Digitanks"; }

	void						SetScreenshot(const tvector<Color>& aclrScreenshot, int iWidth, int iHeight) { m_aclrScreenshot = aclrScreenshot; m_iScreenshotWidth = iWidth; m_iScreenshotHeight = iHeight; };
	size_t						GetScreenshot() { return m_iScreenshot; };
	void						SetupEngine();
	void						SetupIntro();

	virtual void				RenderLoading();

	virtual bool				DoKeyPress(int c);

	class CGeneralWindow*		GetGeneralWindow() { return m_pGeneralWindow; }

	class CIntroRenderer*		GetRenderer();
	virtual class CRenderer*    CreateRenderer();

protected:
	size_t                      m_iScreenshot;
	tvector<Color>              m_aclrScreenshot;
	int                         m_iScreenshotWidth;
	int                         m_iScreenshotHeight;

	class CGeneralWindow*		m_pGeneralWindow;
};

inline CIntroWindow* IntroWindow()
{
	return static_cast<CIntroWindow*>(CApplication::Get());
}

#endif
