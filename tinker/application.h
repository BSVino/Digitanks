/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TINKER_APPLICATION_H
#define TINKER_APPLICATION_H

#include <tvector.h>
#include <common.h>
#include <vector.h>
#include <color.h>
#include <geometry.h>

#include "shell.h"
#include "keys.h"

class CApplication : public CShell
{
public:
								CApplication(int argc, char** argv);
	virtual 					~CApplication();

public:
	void						SetMultisampling(bool bMultisampling) { m_bMultisampling = bMultisampling; }

	void						OpenWindow(size_t iWidth, size_t iHeight, bool bFullscreen, bool bResizeable);

	void						DumpGLInfo();

	virtual tstring				WindowTitle() { return "Tinker"; }
	virtual tstring				AppDirectory() { return "Tinker"; }

	tstring                     GetAppDataDirectory(const tstring& sFile = "");

	void                        PollEvents();
	void						SwapBuffers();
	double						GetTime();

	bool						IsOpen();
	void						Close();

	bool						HasFocus();

	static void					RenderCallback() { Get()->Render(); };
	virtual void				Render();

	virtual int					WindowClose();
	virtual void				WindowResize(int x, int y);
	virtual void				MouseMotion(int x, int y);
	void						MouseInputCallback(int iButton, tinker_mouse_state_t iState);
	virtual bool				MouseInput(int iButton, tinker_mouse_state_t iState);
	virtual void				MouseWheel(int /*x*/, int /*y*/) {};
	void						KeyEvent(int c, int e);
	virtual bool				KeyPress(int c);
	virtual void				KeyRelease(int c);
	void						CharEvent(int c);
	void                        IgnoreMouseRelease() { m_bMouseDownInGUI = true; }

	virtual void TouchMotion(int iFinger, float x, float y, float dx, float dy);
	virtual bool TouchInput(int iFinger, tinker_mouse_state_t iState, float x, float y);

	virtual bool				DoKeyPress(int /*c*/) { return false; };
	virtual void				DoKeyRelease(int /*c*/) {};

	virtual bool				DoCharPress(int /*c*/) { return false; };

	bool						IsCtrlDown();
	bool						IsAltDown();
	bool						IsShiftDown();
	bool						IsMouseLeftDown();
	bool						IsMouseRightDown();
	bool						IsMouseMiddleDown();
	void						GetMousePosition(int& x, int& y);

	void						InitJoystickInput();
	void						ProcessJoystickInput();

	virtual bool				JoystickButtonPress(int /*iJoystick*/, int /*c*/) { return false; };
	virtual void				JoystickButtonRelease(int /*iJoystick*/, int /*c*/) {};

	virtual void				JoystickAxis(int /*iJoystick*/, int /*iAxis*/, float /*flValue*/, float /*flChange*/) {};

	void						SetMouseCursorEnabled(bool bEnabled);
	bool						IsMouseCursorEnabled();

	void                        ActivateKeyboard(const FRect& rInputArea);
	void                        DeactivateKeyboard();

	int							GetWindowWidth() { return (int)m_iWindowWidth; };
	int							GetWindowHeight() { return (int)m_iWindowHeight; };
	float                       GetGUIScale() const { return m_flGUIScale; }
	void                        SetGUIScale(float flScale) { m_flGUIScale = flScale; }

	void                        GetViewportSize(size_t& w, size_t& h);

	bool						IsFullscreen() { return m_bFullscreen; };

	virtual void				OnClientDisconnect(int /*iClient*/) {};

	virtual class CRenderer*	CreateRenderer()=0;
	class CRenderer*			GetRenderer() { return m_pRenderer; }

	static bool                 PlatformHasMenuKey();

	static void					OpenConsole();
	static void					CloseConsole();
	static void					ToggleConsole();
	static bool					IsConsoleOpen();
	virtual void				PrintConsole(const tstring& sText);
	virtual void				PrintError(const tstring& sText);
	class CConsole*				GetConsole();

	static void                 GetScreenSize(int& w, int& h);

	static CApplication*		Get() { return s_pApplication; };

protected:
	struct SDL_Window*          m_pWindow;
	size_t						m_iWindowWidth;
	size_t						m_iWindowHeight;
	bool						m_bFullscreen;
	bool						m_bIsOpen;
	float                       m_flGUIScale;

	bool						m_bMultisampling;

	bool						m_bMouseEnabled;
	bool						m_bMouseDownInGUI;
	double						m_flLastMousePress;

	class CRenderer*			m_pRenderer;

	class CConsole*				m_pConsole;

	static CApplication*		s_pApplication;
};

inline CApplication* Application()
{
	return CApplication::Get();
}

inline class CRenderer* CApplication::CreateRenderer()
{
	return nullptr;
}

#endif
