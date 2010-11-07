#ifndef TINKER_APPLICATION_H
#define TINKER_APPLICATION_H

#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <vector.h>
#include <color.h>

class CApplication
{
public:
								CApplication(int argc, char** argv);
	virtual 					~CApplication();

public:
	void						OpenWindow(size_t iWidth, size_t iHeight, bool bFullscreen);

	void						DumpGLInfo();

	virtual eastl::string		WindowTitle() { return "Tinker"; }

	void						SwapBuffers();
	bool						IsOpen();
	float						GetTime();

	static void					RenderCallback() { Get()->Render(); };
	virtual void				Render();

	static void					WindowResizeCallback(int x, int y) { Get()->WindowResize(x, y); };
	virtual void				WindowResize(int x, int y);

	static void					MouseMotionCallback(int x, int y) { Get()->MouseMotion(x, y); };
	virtual void				MouseMotion(int x, int y) {};

	static void					MouseInputCallback(int iButton, int iState);
	virtual void				MouseInput(int iButton, int iState) {};

	static void					MouseWheelCallback(int iState) { Get()->MouseWheel(iState); };
	virtual void				MouseWheel(int iState) {};

	static void					KeyEventCallback(int c, int e) { Get()->KeyEvent(c, e); };
	void						KeyEvent(int c, int e);
	virtual void				KeyPress(int c) {};
	virtual void				KeyRelease(int c) {};

	static void					CharEventCallback(int c, int e) { Get()->CharEvent(c, e); };
	void						CharEvent(int c, int e);
	virtual void				CharPress(int c) {};
	virtual void				CharRelease(int c) {};

	bool						IsCtrlDown();
	bool						IsAltDown();
	bool						IsShiftDown();
	bool						IsMouseLeftDown();
	bool						IsMouseRightDown();
	bool						IsMouseMiddleDown();
	void						GetMousePosition(int& x, int& y);

	int							GetWindowWidth() { return (int)m_iWindowWidth; };
	int							GetWindowHeight() { return (int)m_iWindowHeight; };

	bool						IsFullscreen() { return m_bFullscreen; };

	bool						HasCommandLineSwitch(const char* pszSwitch);
	const char*					GetCommandLineSwitchValue(const char* pszSwitch);

	static CApplication*		Get() { return s_pApplication; };

protected:
	size_t						m_iWindowWidth;
	size_t						m_iWindowHeight;
	bool						m_bFullscreen;

	static CApplication*		s_pApplication;

	eastl::vector<const char*>	m_apszCommandLine;
};

#endif