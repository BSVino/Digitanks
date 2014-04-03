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

#include "application.h"

#include <time.h>
#include <iostream>
#include <fstream>
#include <SDL.h>

#include <strutils.h>
#include <tinker_platform.h>
#include <mtrand.h>
#include <tvector.h>

#include <tinker/keys.h>
#include <tinker/cvar.h>
#include <glgui/rootpanel.h>
#include <tinker/renderer/renderer.h>
#include <profiler.h>

#ifndef TINKER_NO_TOOLS
#include <tools/manipulator/manipulator.h>
#endif

#include "renderer/tinker_gl.h"
#include "console.h"

#if !defined(__ANDROID__)
#define NO_GL_DEBUG
#endif

CApplication* CApplication::s_pApplication = NULL;

CApplication::CApplication(int argc, char** argv)
	: CShell(argc, argv)
{
	s_pApplication = this;

	m_pWindow = nullptr;

	int iMode = SDL_INIT_VIDEO | SDL_INIT_TIMER;

#ifdef _DEBUG
	iMode |= SDL_INIT_NOPARACHUTE;
#endif

	SDL_Init(iMode);

	m_flGUIScale = 1;

	// SDL has no support for Android high DPI, so we'll just scale it.
#ifdef __ANDROID__
	float xdpi, ydpi;
	GetScreenDPI(xdpi, ydpi);
	m_flGUIScale = 96.0f / ((xdpi + ydpi) / 2);
#endif

	srand((unsigned int)time(NULL));
	mtsrand((size_t)time(NULL));

	for (int i = 0; i < argc; i++)
		m_apszCommandLine.push_back(argv[i]);

	m_bIsOpen = false;
	m_bMultisampling = false;

	m_pRenderer = NULL;
	m_pConsole = NULL;

	SetMouseCursorEnabled(true);
	m_bMouseDownInGUI = false;
	m_flLastMousePress = -1;

	for (int i = 1; i < argc; i++)
	{
		if (m_apszCommandLine[i][0] == '+')
			CCommand::Run(&m_apszCommandLine[i][1]);
	}
}

#if defined(NO_GL_DEBUG)

#ifdef _DEBUG
#define GL_DEBUG_VALUE "1"
#else
#define GL_DEBUG_VALUE "0"
#endif

CVar gl_debug("gl_debug", GL_DEBUG_VALUE);

#ifndef CALLBACK
#define CALLBACK
#endif

void CALLBACK GLDebugCallback(GLenum iSource, GLenum iType, GLuint /*id*/, GLenum iSeverity, GLsizei /*iLength*/, const GLchar* pszMessage, GLvoid* /*pUserParam*/)
{
	if (iType != GL_DEBUG_TYPE_PERFORMANCE_ARB)
	{
		TAssert(iSeverity != GL_DEBUG_SEVERITY_HIGH_ARB);
		TAssert(iSeverity != GL_DEBUG_SEVERITY_MEDIUM_ARB);
	}

	if (gl_debug.GetBool())
	{
		tstring sMessage = "OpenGL Debug Message (";

		if (iSource == GL_DEBUG_SOURCE_API_ARB)
			sMessage += "Source: API ";
		else if (iSource == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
			sMessage += "Source: Window System ";
		else if (iSource == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
			sMessage += "Source: Shader Compiler ";
		else if (iSource == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
			sMessage += "Source: Third Party ";
		else if (iSource == GL_DEBUG_SOURCE_APPLICATION_ARB)
			sMessage += "Source: Application ";
		else if (iSource == GL_DEBUG_SOURCE_OTHER_ARB)
			sMessage += "Source: Other ";

		if (iType == GL_DEBUG_TYPE_ERROR_ARB)
			sMessage += "Type: Error ";
		else if (iType == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
			sMessage += "Type: Deprecated Behavior ";
		else if (iType == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
			sMessage += "Type: Undefined Behavior ";
		else if (iType == GL_DEBUG_TYPE_PORTABILITY_ARB)
			sMessage += "Type: Portability ";
		else if (iType == GL_DEBUG_TYPE_PERFORMANCE_ARB)
			sMessage += "Type: Performance ";
		else if (iType == GL_DEBUG_TYPE_OTHER_ARB)
			sMessage += "Type: Other ";

		if (iSeverity == GL_DEBUG_SEVERITY_HIGH_ARB)
			sMessage += "Severity: High) ";
		else if (iSeverity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
			sMessage += "Severity: Medium) ";
		else if (iSeverity == GL_DEBUG_SEVERITY_LOW_ARB)
			sMessage += "Severity: Low) ";

		sMessage += tstring(pszMessage) + "\n";

		TMsg(sMessage.c_str());
	}
}
#endif

void CApplication::OpenWindow(size_t iWidth, size_t iHeight, bool bFullscreen, bool bResizeable)
{
	m_bFullscreen = bFullscreen;

	if (HasCommandLineSwitch("--fullscreen"))
		m_bFullscreen = true;

	if (HasCommandLineSwitch("--windowed"))
		m_bFullscreen = false;

	m_iWindowWidth = iWidth;
	m_iWindowHeight = iHeight;

	TMsg(tsprintf("Opening %dx%d %s %s window.\n", iWidth, iHeight, bFullscreen?"fullscreen":"windowed", bResizeable?"resizeable":"fixed-size"));

	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	int iModes = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;

	if (m_bFullscreen)
		iModes |= SDL_WINDOW_FULLSCREEN;

	if (bResizeable)
		iModes |= SDL_WINDOW_RESIZABLE;

	if (NULL == (m_pWindow = SDL_CreateWindow(WindowTitle().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, iWidth, iHeight, iModes)))
	{
		SDL_Quit();
		return;
	}

	if (!bResizeable)
		SDL_SetWindowBordered(m_pWindow, SDL_FALSE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	if (m_bMultisampling)
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

#if defined(NO_GL_DEBUG)
	if (HasCommandLineSwitch("--debug-gl"))
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

		if (!glDebugMessageCallbackARB)
			TMsg("Your drivers do not support GL_ARB_debug_output, so no GL debug output will be shown.\n");
	}
#endif

	SDL_GL_CreateContext(m_pWindow);

	SDL_GL_SetSwapInterval(1);

#if defined(__gl3w_h_)
	GLenum err = gl3wInit();
	if (0 != err)
		exit(0);
#endif

	InitJoystickInput();

	SetMouseCursorEnabled(true);

	DumpGLInfo();

#if defined(NO_GL_DEBUG)
	if (glDebugMessageCallbackARB)
	{
		glDebugMessageCallbackARB(GLDebugCallback, nullptr);

		tstring sMessage("OpenGL Debug Output Activated");
		glDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, 0, GL_DEBUG_SEVERITY_LOW_ARB, sMessage.length(), sMessage.c_str());
	}
#endif

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1.0);

	m_bIsOpen = true;

	m_pRenderer = CreateRenderer();
	m_pRenderer->Initialize();

	glgui::RootPanel()->SetSize((float)m_pRenderer->GetDrawableWidth(), (float)m_pRenderer->GetDrawableHeight());
	glgui::RootPanel()->Layout();
}

CApplication::~CApplication()
{
	delete m_pRenderer;
	glgui::RootPanel()->RemoveControl(m_pConsole);
	m_pConsole = nullptr;
	glgui::CRootPanel::Reset();
	s_pApplication = nullptr;

	SDL_DestroyWindow(m_pWindow);
	SDL_Quit();
}

#define MAKE_PARAMETER(name) \
{ #name, name } \

void CApplication::DumpGLInfo()
{
#if defined(__gl3w_h_)
	gl3wInit();
#endif

	if (!GetAppDataDirectory("glinfo.txt").length())
		return;

	std::ifstream i(GetAppDataDirectory("glinfo.txt").c_str());
	if (i)
		return;
	i.close();

	std::ofstream o(GetAppDataDirectory("glinfo.txt").c_str());
	if (!o || !o.is_open())
		return;

	o << "Vendor: " << (char*)glGetString(GL_VENDOR) << std::endl;
	o << "Renderer: " << (char*)glGetString(GL_RENDERER) << std::endl;
	o << "Version: " << (char*)glGetString(GL_VERSION) << std::endl;

	char* pszShadingLanguageVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	if (pszShadingLanguageVersion)
		o << "Shading Language Version: " << pszShadingLanguageVersion << std::endl;

	char* pszExtensions = (char*)glGetString(GL_EXTENSIONS);
	tstring sExtensions;
	if (pszExtensions)
		sExtensions = pszExtensions;
	tvector<tstring> asExtensions;
	strtok(sExtensions, asExtensions);
	o << "Extensions:" << std::endl;
	for (size_t i = 0; i < asExtensions.size(); i++)
		o << "\t" << asExtensions[i].c_str() << std::endl;

	typedef struct
	{
		const char* pszName;
		int iParameter;
	} GLParameter;

	GLParameter aParameters[] =
	{
		MAKE_PARAMETER(GL_MAX_TEXTURE_SIZE),
		MAKE_PARAMETER(GL_MAX_VIEWPORT_DIMS),
		MAKE_PARAMETER(GL_MAX_3D_TEXTURE_SIZE),
		MAKE_PARAMETER(GL_MAX_ELEMENTS_VERTICES),
		MAKE_PARAMETER(GL_MAX_ELEMENTS_INDICES),
		MAKE_PARAMETER(GL_MAX_TEXTURE_LOD_BIAS),
		MAKE_PARAMETER(GL_MAX_DRAW_BUFFERS),
		MAKE_PARAMETER(GL_MAX_VERTEX_ATTRIBS),
		MAKE_PARAMETER(GL_MAX_TEXTURE_IMAGE_UNITS),
		MAKE_PARAMETER(GL_MAX_VERTEX_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS),
		MAKE_PARAMETER(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS),
		MAKE_PARAMETER(GL_MAX_ARRAY_TEXTURE_LAYERS),
		MAKE_PARAMETER(GL_MAX_VARYING_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_VERTEX_OUTPUT_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_RENDERBUFFER_SIZE),
		MAKE_PARAMETER(GL_MAX_COLOR_ATTACHMENTS),
		MAKE_PARAMETER(GL_MAX_SAMPLES),
		MAKE_PARAMETER(GL_MAX_VERTEX_UNIFORM_BLOCKS),
		MAKE_PARAMETER(GL_MAX_FRAGMENT_UNIFORM_BLOCKS),
		MAKE_PARAMETER(GL_MAX_COMBINED_UNIFORM_BLOCKS),
		MAKE_PARAMETER(GL_MAX_UNIFORM_BUFFER_BINDINGS),
		MAKE_PARAMETER(GL_MAX_UNIFORM_BLOCK_SIZE),
		MAKE_PARAMETER(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS),

#ifndef __ANDROID__
		MAKE_PARAMETER(GL_MAX_VARYING_FLOATS),
		MAKE_PARAMETER(GL_MAX_CLIP_DISTANCES),
		MAKE_PARAMETER(GL_MAX_TEXTURE_BUFFER_SIZE),
		MAKE_PARAMETER(GL_MAX_RECTANGLE_TEXTURE_SIZE),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_OUTPUT_VERTICES),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_INPUT_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_GEOMETRY_UNIFORM_BLOCKS),
		MAKE_PARAMETER(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS),
		MAKE_PARAMETER(GL_MAX_COLOR_TEXTURE_SAMPLES),
		MAKE_PARAMETER(GL_MAX_DEPTH_TEXTURE_SAMPLES),
		MAKE_PARAMETER(GL_MAX_INTEGER_SAMPLES),
#endif
	};

	// Clear it
	glGetError();

	o << std::endl;

	for (size_t i = 0; i < sizeof(aParameters)/sizeof(GLParameter); i++)
	{
		GLint iValue[4];
		glGetIntegerv(aParameters[i].iParameter, &iValue[0]);

		if (glGetError() != GL_NO_ERROR)
			continue;

		o << aParameters[i].pszName << ": " << iValue[0] << std::endl;
	}
}

tstring CApplication::GetAppDataDirectory(const tstring& sFile)
{
	const char* pszPath = SDL_GetPrefPath("Tinker", AppDirectory().c_str());

	if (!pszPath)
	{
#ifdef __ANDROID__
		const char* p = SDL_AndroidGetExternalStoragePath();
		tstring sPath = tstring(p) + "/" + sFile;
		return sPath;
#endif

		return "";
	}

	tstring sPath = tstring(pszPath) + sFile;

	SDL_free((void*)pszPath);

	return sPath;
}

tinker_keys_t MapMouseKey(Uint8 c);

void CApplication::PollEvents()
{
	SDL_Event e;

	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_QUIT:
			if (WindowClose())
				m_bIsOpen = false;
			break;

		case SDL_WINDOWEVENT:
			switch (e.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				WindowResize(e.window.data1, e.window.data2);
				break;
			}
			break;

		case SDL_KEYDOWN:
			KeyEvent((int)e.key.keysym.scancode, e.key.state);
			break;

		case SDL_KEYUP:
			KeyEvent((int)e.key.keysym.scancode, e.key.state);
			break;

		case SDL_TEXTINPUT:
			// This won't support unicode very well.
			for (size_t i = 0; i < strlen(e.text.text); i++)
				CharEvent((int)e.text.text[i]);
			break;

		case SDL_MOUSEMOTION:
			MouseMotion(e.motion.x, e.motion.y);
			break;

		case SDL_MOUSEBUTTONUP:
			MouseInput(MapMouseKey(e.button.button), TINKER_MOUSE_RELEASED);
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (e.button.clicks == 2)
				MouseInput(MapMouseKey(e.button.button), TINKER_MOUSE_DOUBLECLICK);
			else
				MouseInput(MapMouseKey(e.button.button), TINKER_MOUSE_PRESSED);
			break;

		case SDL_MOUSEWHEEL:
			MouseWheel(e.wheel.x, e.wheel.y);
			break;
		}
	}

	ProcessJoystickInput();
}

void CApplication::SwapBuffers()
{
	SDL_GL_SwapWindow(m_pWindow);
}

double CApplication::GetTime()
{
	return (double)(SDL_GetTicks())/1000;
}

bool CApplication::IsOpen()
{
	if (m_pWindow)
		return (!!(SDL_GetWindowFlags(m_pWindow)&SDL_WINDOW_SHOWN)) && m_bIsOpen;
	else
		return m_bIsOpen;
}

void Quit(class CCommand* /*pCommand*/, tvector<tstring>& /*asTokens*/, const tstring& /*sCommand*/)
{
	CApplication::Get()->Close();
}

CCommand quit("quit", ::Quit);

void CApplication::Close()
{
	m_bIsOpen = false;
}

bool CApplication::HasFocus()
{
	return !!(SDL_GetWindowFlags(m_pWindow)&SDL_WINDOW_MOUSE_FOCUS);
}

void CApplication::Render()
{
	TPROF("CApplication::Render");

	if (GetRenderer())
		GetRenderer()->RenderFrame();

	if (CShaderLibrary::IsCompiled())
	{
		glgui::RootPanel()->Think(GetTime());
		glgui::RootPanel()->Paint(0, 0, (float)GetRenderer()->GetDrawableWidth(), (float)GetRenderer()->GetDrawableHeight());
	}
}

int CApplication::WindowClose()
{
	return GL_TRUE;
}

void CApplication::WindowResize(int w, int h)
{
	m_iWindowWidth = w;
	m_iWindowHeight = h;

	if (m_pRenderer)
	{
		size_t x, y;
		GetViewportSize(x, y);
		m_pRenderer->ViewportResize(x, y);
		glgui::RootPanel()->SetSize((float)x, (float)y);
	}
	else
		glgui::RootPanel()->SetSize((float)w, (float)h);

	glgui::RootPanel()->Layout();

	Render();

	SwapBuffers();
}

void CApplication::MouseMotion(int x, int y)
{
	glgui::CRootPanel::Get()->CursorMoved(x, y);

#ifndef TINKER_NO_TOOLS
	CManipulatorTool::MouseMoved(x, y);
#endif
}

bool CApplication::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	int mx, my;
	GetMousePosition(mx, my);
	if (iState == TINKER_MOUSE_PRESSED)
	{
		if (glgui::CRootPanel::Get()->MousePressed(iButton, mx, my))
		{
			m_bMouseDownInGUI = true;
			return true;
		}
		else
			m_bMouseDownInGUI = false;
	}
	else if (iState == TINKER_MOUSE_RELEASED)
	{
		if (glgui::CRootPanel::Get()->MouseReleased(iButton, mx, my))
			return true;

		if (m_bMouseDownInGUI)
		{
			m_bMouseDownInGUI = false;
			return true;
		}
	}
	else if (iState == TINKER_MOUSE_DOUBLECLICK)
	{
		if (glgui::CRootPanel::Get()->MouseDoubleClicked(iButton, mx, my))
			return true;

		if (m_bMouseDownInGUI)
		{
			m_bMouseDownInGUI = false;
			return true;
		}
	}

#ifndef TINKER_NO_TOOLS
	if (CManipulatorTool::MouseInput(iButton, iState, mx, my))
		return true;
#endif

	return false;
}

tinker_keys_t MapScancode(SDL_Scancode c)
{
	switch (c)
	{
	case SDL_SCANCODE_AC_BACK:
		return TINKER_KEY_APP_BACK;

	case SDL_SCANCODE_MENU:
		return TINKER_KEY_APP_MENU;

	case SDL_SCANCODE_ESCAPE:
		return TINKER_KEY_ESCAPE;

	case SDL_SCANCODE_F1:
		return TINKER_KEY_F1;

	case SDL_SCANCODE_F2:
		return TINKER_KEY_F2;

	case SDL_SCANCODE_F3:
		return TINKER_KEY_F3;

	case SDL_SCANCODE_F4:
		return TINKER_KEY_F4;

	case SDL_SCANCODE_F5:
		return TINKER_KEY_F5;

	case SDL_SCANCODE_F6:
		return TINKER_KEY_F6;

	case SDL_SCANCODE_F7:
		return TINKER_KEY_F7;

	case SDL_SCANCODE_F8:
		return TINKER_KEY_F8;

	case SDL_SCANCODE_F9:
		return TINKER_KEY_F9;

	case SDL_SCANCODE_F10:
		return TINKER_KEY_F10;

	case SDL_SCANCODE_F11:
		return TINKER_KEY_F11;

	case SDL_SCANCODE_F12:
		return TINKER_KEY_F12;

	case SDL_SCANCODE_UP:
		return TINKER_KEY_UP;

	case SDL_SCANCODE_DOWN:
		return TINKER_KEY_DOWN;

	case SDL_SCANCODE_LEFT:
		return TINKER_KEY_LEFT;

	case SDL_SCANCODE_RIGHT:
		return TINKER_KEY_RIGHT;

	case SDL_SCANCODE_LSHIFT:
		return TINKER_KEY_LSHIFT;

	case SDL_SCANCODE_RSHIFT:
		return TINKER_KEY_RSHIFT;

	case SDL_SCANCODE_LCTRL:
		return TINKER_KEY_LCTRL;

	case SDL_SCANCODE_RCTRL:
		return TINKER_KEY_RCTRL;

	case SDL_SCANCODE_LALT:
		return TINKER_KEY_LALT;

	case SDL_SCANCODE_RALT:
		return TINKER_KEY_RALT;

	case SDL_SCANCODE_TAB:
		return TINKER_KEY_TAB;

	case SDL_SCANCODE_RETURN:
		return TINKER_KEY_ENTER;

	case SDL_SCANCODE_BACKSPACE:
		return TINKER_KEY_BACKSPACE;

	case SDL_SCANCODE_INSERT:
		return TINKER_KEY_INSERT;

	case SDL_SCANCODE_DELETE:
		return TINKER_KEY_DEL;

	case SDL_SCANCODE_PAGEUP:
		return TINKER_KEY_PAGEUP;

	case SDL_SCANCODE_PAGEDOWN:
		return TINKER_KEY_PAGEDOWN;

	case SDL_SCANCODE_HOME:
		return TINKER_KEY_HOME;

	case SDL_SCANCODE_END:
		return TINKER_KEY_END;

	case SDL_SCANCODE_KP_0:
		return TINKER_KEY_KP_0;

	case SDL_SCANCODE_KP_1:
		return TINKER_KEY_KP_1;

	case SDL_SCANCODE_KP_2:
		return TINKER_KEY_KP_2;

	case SDL_SCANCODE_KP_3:
		return TINKER_KEY_KP_3;

	case SDL_SCANCODE_KP_4:
		return TINKER_KEY_KP_4;

	case SDL_SCANCODE_KP_5:
		return TINKER_KEY_KP_5;

	case SDL_SCANCODE_KP_6:
		return TINKER_KEY_KP_6;

	case SDL_SCANCODE_KP_7:
		return TINKER_KEY_KP_7;

	case SDL_SCANCODE_KP_8:
		return TINKER_KEY_KP_8;

	case SDL_SCANCODE_KP_9:
		return TINKER_KEY_KP_9;

	case SDL_SCANCODE_KP_DIVIDE:
		return TINKER_KEY_KP_DIVIDE;

	case SDL_SCANCODE_KP_MULTIPLY:
		return TINKER_KEY_KP_MULTIPLY;

	case SDL_SCANCODE_KP_MINUS:
		return TINKER_KEY_KP_SUBTRACT;

	case SDL_SCANCODE_KP_PLUS:
		return TINKER_KEY_KP_ADD;

	case SDL_SCANCODE_KP_PERIOD:
		return TINKER_KEY_KP_DECIMAL;

	case SDL_SCANCODE_KP_EQUALS:
		return TINKER_KEY_KP_EQUAL;

	case SDL_SCANCODE_KP_ENTER:
		return TINKER_KEY_KP_ENTER;

	default:
		break;
	}

	if (c >= SDL_SCANCODE_A && c <= SDL_SCANCODE_Z)
		return (tinker_keys_t)('A' + c - SDL_SCANCODE_A);

	if (c >= SDL_SCANCODE_1 && c <= SDL_SCANCODE_9)
		return (tinker_keys_t)('1' + c - SDL_SCANCODE_1);

	if (c == SDL_SCANCODE_0)
		return (tinker_keys_t)'0';

	return TINKER_KEY_UNKNOWN;
}

tinker_keys_t MapMouseKey(Uint8 c)
{
	switch (c)
	{
	case SDL_BUTTON_LEFT:
		return TINKER_KEY_MOUSE_LEFT;

	case SDL_BUTTON_RIGHT:
		return TINKER_KEY_MOUSE_RIGHT;

	case SDL_BUTTON_MIDDLE:
		return TINKER_KEY_MOUSE_MIDDLE;
	}

	return TINKER_KEY_UNKNOWN;
}

tinker_keys_t MapJoystickKey(int c)
{
	switch (c)
	{
	case 0:
	default:;
	/*case GLFW_JOYSTICK_1:
		return TINKER_KEY_JOYSTICK_1;

	case GLFW_JOYSTICK_2:
		return TINKER_KEY_JOYSTICK_2;

	case GLFW_JOYSTICK_3:
		return TINKER_KEY_JOYSTICK_3;

	case GLFW_JOYSTICK_4:
		return TINKER_KEY_JOYSTICK_4;

	case GLFW_JOYSTICK_5:
		return TINKER_KEY_JOYSTICK_5;

	case GLFW_JOYSTICK_6:
		return TINKER_KEY_JOYSTICK_6;

	case GLFW_JOYSTICK_7:
		return TINKER_KEY_JOYSTICK_7;

	case GLFW_JOYSTICK_8:
		return TINKER_KEY_JOYSTICK_8;

	case GLFW_JOYSTICK_9:
		return TINKER_KEY_JOYSTICK_9;

	case GLFW_JOYSTICK_10:
		return TINKER_KEY_JOYSTICK_10;*/
	}

	return TINKER_KEY_UNKNOWN;
}

void CApplication::MouseInputCallback(int iButton, tinker_mouse_state_t iState)
{
	if (iState == 1)
	{
		if (m_flLastMousePress < 0 || GetTime() - m_flLastMousePress > 0.25f)
			MouseInput(iButton, iState);
		else
			MouseInput(iButton, TINKER_MOUSE_DOUBLECLICK);
		m_flLastMousePress = GetTime();
	}
	else
		MouseInput(iButton, iState);
}

void CApplication::KeyEvent(int c, int e)
{
	if (e == SDL_PRESSED)
		KeyPress(MapScancode((SDL_Scancode)c));
	else
		KeyRelease(MapScancode((SDL_Scancode)c));
}

void CApplication::CharEvent(int c)
{
	if (c == '`')
	{
		ToggleConsole();
		return;
	}

	if (glgui::CRootPanel::Get()->CharPressed(c))
		return;

	DoCharPress(c);
}

bool CApplication::KeyPress(int c)
{
#ifdef __ANDROID__
	if (c == TINKER_KEY_APP_BACK)
	{
		if (GetConsole()->IsOpen())
			ToggleConsole();
		else
			Close();
	}

	if (c == TINKER_KEY_APP_MENU)
		ToggleConsole();
#endif

	if (glgui::CRootPanel::Get()->KeyPressed(c, IsCtrlDown()))
		return true;

	if (c == TINKER_KEY_F4 && IsAltDown())
		exit(0);

	return DoKeyPress(c);
}

void CApplication::KeyRelease(int c)
{
	DoKeyRelease(c);
}

bool CApplication::IsCtrlDown()
{
	return !!(SDL_GetModState()&(KMOD_LCTRL | KMOD_RCTRL));
}

bool CApplication::IsAltDown()
{
	return !!(SDL_GetModState()&(KMOD_LALT | KMOD_RALT));
}

bool CApplication::IsShiftDown()
{
	return !!(SDL_GetModState()&(KMOD_LSHIFT | KMOD_RSHIFT));
}

bool CApplication::IsMouseLeftDown()
{
	return !!(SDL_BUTTON(SDL_BUTTON_LEFT)&SDL_GetMouseState(NULL, NULL));
}

bool CApplication::IsMouseRightDown()
{
	return !!(SDL_BUTTON(SDL_BUTTON_RIGHT)&SDL_GetMouseState(NULL, NULL));
}

bool CApplication::IsMouseMiddleDown()
{
	return !!(SDL_BUTTON(SDL_BUTTON_MIDDLE)&SDL_GetMouseState(NULL, NULL));
}

void CApplication::GetMousePosition(int& x, int& y)
{
	SDL_GetMouseState(&x, &y);
}

class CJoystick
{
public:
	CJoystick()
	{
		m_bPresent = false;
	}

public:
	bool					m_bPresent;
	tvector<float>			m_aflAxis;
	unsigned char			m_iButtons;
	unsigned long long		m_aiButtonStates;
};

static tvector<CJoystick> g_aJoysticks;
static const size_t MAX_JOYSTICKS = 16; // This is how many GLFW supports.

void CApplication::InitJoystickInput()
{
	return;
	/*g_aJoysticks.resize(MAX_JOYSTICKS);

	for (size_t i = 0; i < MAX_JOYSTICKS; i++)
	{
		if (glfwGetJoystickParam(GLFW_JOYSTICK_1 + i, GLFW_PRESENT) == GL_TRUE)
		{
			g_aJoysticks[i].m_bPresent = true;
			g_aJoysticks[i].m_aflAxis.resize(glfwGetJoystickParam(GLFW_JOYSTICK_1 + i, GLFW_AXES));

			for (size_t j = 0; j < g_aJoysticks[i].m_aflAxis.size(); j++)
				g_aJoysticks[i].m_aflAxis[j] = 0;

			g_aJoysticks[i].m_iButtons = glfwGetJoystickParam(GLFW_JOYSTICK_1 + i, GLFW_BUTTONS);
			g_aJoysticks[i].m_aiButtonStates = 0;

			TAssert(g_aJoysticks[i].m_iButtons < sizeof(g_aJoysticks[i].m_aiButtonStates)*8);
		}
	}*/
}

void CApplication::ProcessJoystickInput()
{
	return;

	/*if (g_aJoysticks.size() != MAX_JOYSTICKS)
		return;

	for (size_t i = 0; i < MAX_JOYSTICKS; i++)
	{
		CJoystick& oJoystick = g_aJoysticks[i];

		if (!oJoystick.m_bPresent)
			continue;

		static tvector<float> aflAxis;
		aflAxis.resize(oJoystick.m_aflAxis.size());
		glfwGetJoystickPos(i, &aflAxis[0], oJoystick.m_aflAxis.size());

		for (size_t j = 0; j < oJoystick.m_aflAxis.size(); j++)
		{
			if (aflAxis[j] != oJoystick.m_aflAxis[j])
				JoystickAxis(i, j, aflAxis[j], aflAxis[j]-oJoystick.m_aflAxis[j]);
		}

		oJoystick.m_aflAxis = aflAxis;

		static tvector<unsigned char> aiButtons;
		aiButtons.resize(oJoystick.m_iButtons);
		glfwGetJoystickButtons(i, &aiButtons[0], oJoystick.m_iButtons);

		for (size_t j = 0; j < oJoystick.m_iButtons; j++)
		{
			unsigned long long iButtonMask = (1<<j);
			if (aiButtons[j] == GLFW_PRESS && !(oJoystick.m_aiButtonStates&iButtonMask))
				JoystickButtonPress(i, MapJoystickKey(j));
			else if (aiButtons[j] == GLFW_RELEASE && (oJoystick.m_aiButtonStates&iButtonMask))
				JoystickButtonRelease(i, MapJoystickKey(j));

			if (aiButtons[j] == GLFW_PRESS)
				oJoystick.m_aiButtonStates |= iButtonMask;
			else
				oJoystick.m_aiButtonStates &= ~iButtonMask;
		}
	}*/
}

void CApplication::SetMouseCursorEnabled(bool bEnabled)
{
	int iShown = SDL_ShowCursor(bEnabled);

	if (iShown < 0)
	{
		TError("Couldn't change mouse state: " + tstring(SDL_GetError()));
		return;
	}

	m_bMouseEnabled = !!iShown;
}

bool CApplication::IsMouseCursorEnabled()
{
	return m_bMouseEnabled;
}

void CApplication::ActivateKeyboard(const FRect& rInputArea)
{
	SDL_Rect r;
	r.x = (int)rInputArea.x;
	r.y = (int)rInputArea.y;
	r.w = (int)rInputArea.w;
	r.h = (int)rInputArea.h;
	SDL_SetTextInputRect(&r);
	SDL_StartTextInput();
}

void CApplication::DeactivateKeyboard()
{
	SDL_StopTextInput();
}

void CApplication::GetViewportSize(size_t& w, size_t& h)
{
	int x, y;
	SDL_GL_GetDrawableSize(m_pWindow, &x, &y);
	w = x;
	h = y;
}

void CApplication::PrintConsole(const tstring& sText)
{
	GetConsole()->PrintConsole(sText);
}

void CApplication::PrintError(const tstring& sText)
{
	tstring sTrimmedText = trim(sText);

	GetConsole()->PrintConsole(tstring("[color=FF0000]ERROR: ") + sTrimmedText + "[/color]" + (sText.endswith("\n")?"\n":""));
}

bool CApplication::PlatformHasMenuKey()
{
#ifdef __ANDROID__
	return true;
#else
	return false;
#endif
}

void CApplication::GetScreenSize(int& w, int& h)
{
	TAssert(SDL_GetNumVideoDisplays() > 0);

	if (SDL_GetNumVideoDisplays() == 0)
	{
		w = h = 0;
		return;
	}

	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);

	w = current.w;
	h = current.h;
}
