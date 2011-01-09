#ifndef TINKER_APPLICATION_H
#define TINKER_APPLICATION_H

#include <EASTL/string.h>
#include <EASTL/vector.h>

#include <vector.h>
#include <color.h>
#include <configfile.h>

class CApplication
{
public:
								CApplication(int argc, char** argv);
	virtual 					~CApplication();

public:
	void						SetMultisampling(bool bMultisampling) { m_bMultisampling = bMultisampling; }

	void						OpenWindow(size_t iWidth, size_t iHeight, bool bFullscreen, bool bResizeable);

	void						DumpGLInfo();

	virtual eastl::string		WindowTitle() { return "Tinker"; }
	virtual eastl::string16		AppDirectory() { return L"Tinker"; }

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

	void						SetMouseCursorEnabled(bool bEnabled);

	int							GetWindowWidth() { return (int)m_iWindowWidth; };
	int							GetWindowHeight() { return (int)m_iWindowHeight; };

	bool						IsFullscreen() { return m_bFullscreen; };

	bool						HasCommandLineSwitch(const char* pszSwitch);
	const char*					GetCommandLineSwitchValue(const char* pszSwitch);

	void						InitRegistrationFile();
	bool						IsRegistered();
	void						ReadProductCode();
	eastl::string				GetProductCode();
	void						SetLicenseKey(eastl::string sKey);
	bool						QueryRegistrationKey(eastl::string16 sServer, eastl::string16 sURI, eastl::string16 sKey, eastl::string sProduct, eastl::string16& sError);
	void						SaveProductCode();
	eastl::string				GenerateCode();

	static CApplication*		Get() { return s_pApplication; };

protected:
	size_t						m_iWindowWidth;
	size_t						m_iWindowHeight;
	bool						m_bFullscreen;
	bool						m_bIsOpen;

	bool						m_bMultisampling;

	eastl::vector<const char*>	m_apszCommandLine;

	ConfigFile					m_oRegFile;
	eastl::string				m_sCode;
	eastl::string				m_sKey;

	static CApplication*		s_pApplication;
};

#endif