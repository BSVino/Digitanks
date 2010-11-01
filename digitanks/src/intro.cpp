#include <windows.h>
#include <stdio.h>
#include <GL/glew.h>
#include <time.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <maths.h>

#include <platform.h>
#include <sound/sound.h>

void SetupFrame()
{
	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	// Set up the frame.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, iScreenWidth, iScreenHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);
}

void DrawTexture(GLuint iTexture, float x, float y, float w, float h)
{
	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
	glColor4ub(255, 255, 255, 255);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2f(x, y);
		glTexCoord2f(0, 0);
		glVertex2f(x, y+h);
		glTexCoord2f(1, 0);
		glVertex2f(x+w, y+h);
		glTexCoord2f(1, 1);
		glVertex2f(x+w, y);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	glPopAttrib();
}

void DrawFade(float flAlpha)
{
	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4ub(0, 0, 0, (GLubyte)(flAlpha*255));
	glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(0, iScreenHeight);
		glVertex2i(iScreenWidth, iScreenHeight);
		glVertex2i(iScreenWidth, 0);
	glEnd();

	glPopAttrib();
}

void TeardownFrame()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

bool g_bSkip = false;

static LRESULT CALLBACK IntroWindowCallback( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			// escape
			if (wParam == VK_ESCAPE && !(GetKeyState(VK_CONTROL) & 0x8000))
				g_bSkip = true;
            return 0;
		}  

		case WM_SYSCOMMAND:
		{
			switch( wParam )
			{
				// Screensaver trying to start or monitor trying to enter powersave?
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;

				// User trying to access application menu using ALT?
				case SC_KEYMENU:
					return 0;
			}
			break;
		}

		case WM_CLOSE:
			PostQuitMessage( 0 );
			return 0;

		case WM_DESTROY:
            return 0;
    }

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

HHOOK g_hKeyboardHook;
HWND g_hWindow;

LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	bool bSpecialKey = false;
	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) lParam;

	if( nCode == HC_ACTION )
	{
		switch( wParam )
		{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
				// Block these keystrokes: ALT+TAB, ALT+ESC, ALT+F4, CTRL+ESC, LWIN, RWIN, APPS (mysterious menu key)
				bSpecialKey = ( p->vkCode == VK_TAB && p->flags & LLKHF_ALTDOWN ) ||
						( p->vkCode == VK_ESCAPE && p->flags & LLKHF_ALTDOWN ) ||
						( p->vkCode == VK_F4 && p->flags & LLKHF_ALTDOWN ) ||
						( p->vkCode == VK_ESCAPE && (GetKeyState(VK_CONTROL) & 0x8000)) ||
						p->vkCode == VK_LWIN || p->vkCode == VK_RWIN || p->vkCode == VK_APPS;
				break;
		}
	}

	if( bSpecialKey )
	{
		// Send it to the main window callback
		PostMessage( g_hWindow, (UINT) wParam, p->vkCode, 0 );
        return 1;
    }
    else
        return CallNextHookEx( g_hKeyboardHook, nCode, wParam, lParam );
}

ILuint LoadImage(wchar_t* pszFilename)
{
	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilLoadImage(pszFilename);

	if (!bSuccess)
		bSuccess = ilLoadImage(pszFilename);

	ILenum iError = ilGetError();

	if (!bSuccess)
		return 0;

	bSuccess = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!bSuccess)
		return 0;

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);

	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		iluFlipImage();

	return iDevILId;
}

GLuint MakeTexture(ILuint iTex)
{
	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);

	ilBindImage(iTex);

	gluBuild2DMipmaps(GL_TEXTURE_2D,
		ilGetInteger(IL_IMAGE_BPP),
		ilGetInteger(IL_IMAGE_WIDTH),
		ilGetInteger(IL_IMAGE_HEIGHT),
		ilGetInteger(IL_IMAGE_FORMAT),
		GL_UNSIGNED_BYTE,
		ilGetData());

	return iGLId;
}

void RunIntro()
{
	/// Take a screenshot of the user's desktop.
	HDC hdcScreen  = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	HDC hdcCapture = CreateCompatibleDC(hdcScreen);
	int nWidth     = GetDeviceCaps(hdcScreen, HORZRES),
		nHeight    = GetDeviceCaps(hdcScreen, VERTRES),
		nBPP       = GetDeviceCaps(hdcScreen, BITSPIXEL);

	LPBYTE lpCapture;
	BITMAPINFO bmiCapture = { {
		sizeof(BITMAPINFOHEADER), nWidth, nHeight, 1, nBPP, BI_RGB, 0, 0, 0, 0, 0,
	} };
	HBITMAP hbmCapture = CreateDIBSection(hdcScreen, &bmiCapture,
		DIB_PAL_COLORS, (LPVOID *)&lpCapture, NULL, 0);
	if(!hbmCapture)
	{
		DeleteDC(hdcCapture);
		DeleteDC(hdcScreen);
		return;
	}

	int nCapture = SaveDC(hdcCapture);
	SelectObject(hdcCapture, hbmCapture);
	BitBlt(hdcCapture, 0, 0, nWidth, nHeight, hdcScreen, 0, 0, SRCCOPY);
	RestoreDC(hdcCapture, nCapture);
	DeleteDC(hdcCapture);
	DeleteDC(hdcScreen);

	int iScreenWidth;
	int iScreenHeight;

	GetScreenSize(iScreenWidth, iScreenHeight);

	WNDCLASS    wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = IntroWindowCallback;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = NULL;
	wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = L"dtintro";
	wc.hIcon = LoadIcon( NULL, IDI_WINLOGO ); 

    // Register the window class
	ATOM hAtom = RegisterClass( &wc );
    if( !hAtom )
        return;

	g_hWindow = CreateWindowEx(
               WS_EX_APPWINDOW,	// |WS_EX_TOPMOST
               wc.lpszClassName,
               L"",
               WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_POPUP,
               0, 0,
               iScreenWidth,
               iScreenHeight,
               NULL,
               NULL,
               NULL,
               NULL );

	HDC hdcWindow = GetDC(g_hWindow);

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int format = ChoosePixelFormat( hdcWindow, &pfd );
	SetPixelFormat( hdcWindow, format, &pfd );

	HGLRC hrcWindow = wglCreateContext( hdcWindow );
	wglMakeCurrent( hdcWindow, hrcWindow );

	glewInit();

	if (!GLEW_ARB_texture_non_power_of_two)
	{
		ShowMessage(L"Looks like your video card doesn't support the features needed by Digitanks! We're terribly sorry.");
		exit(0);
	}

	ilInit();

	// Disable system keys so people can't alt-tab out.
	g_hKeyboardHook = SetWindowsHookEx( WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0 );

	ILuint iErrorWindow = LoadImage(L"textures/intro/error.png");
	ILuint iTankImage = LoadImage(L"textures/intro/tank.png");
	ILuint iTankTexture = MakeTexture(iTankImage);

	// Make a texture out of the screenshot.

	DWORD iTime = timeGetTime();

	while (true)
	{
		if (g_bSkip)
			break;

		float flTime = (float)(timeGetTime()-iTime)/1000;

		SetupFrame();

		if (flTime < 0.5f)
			glDrawPixels(iScreenWidth, iScreenHeight, GL_BGRA, GL_UNSIGNED_BYTE, lpCapture);
		else if (flTime < 0.6f)
			glDrawPixels(iScreenWidth, iScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE, lpCapture);
		else if (flTime < 1.1f)
			glDrawPixels(iScreenWidth, iScreenHeight, GL_BGRA, GL_UNSIGNED_BYTE, lpCapture);
		else if (flTime < 2.2f)
			glDrawPixels(iScreenWidth, iScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE, lpCapture);
		else if (flTime < 2.3f)
			glDrawPixels(iScreenWidth, iScreenHeight, GL_BGRA, GL_UNSIGNED_BYTE, lpCapture);
		else if (flTime < 3.0f)
		{
			glDrawPixels(iScreenWidth, iScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE, lpCapture);
			glDrawPixels(iScreenWidth*8/10, iScreenHeight, GL_BGRA, GL_UNSIGNED_BYTE, lpCapture);
		}
		else if (flTime < 3.7f)
		{
			static int iFrame = 0;
			glDrawPixels(iScreenWidth, iScreenHeight, GL_BGRA, GL_UNSIGNED_BYTE, lpCapture);
			if (iFrame++ == 30)
				glDrawPixels(iScreenWidth*6/10, iScreenHeight/2, GL_RGB, GL_UNSIGNED_BYTE, lpCapture);
			else if (iFrame++ == 90)
				glDrawPixels(iScreenWidth*6/10, iScreenHeight/2, GL_RGB, GL_UNSIGNED_BYTE, lpCapture);
		}
		else if (flTime < 4.0f)
		{
			static int iFrame = 0;
			glDrawPixels(iScreenWidth, iScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE, lpCapture);
			glDrawPixels(iScreenWidth-iFrame++, iScreenHeight, GL_BGRA, GL_UNSIGNED_BYTE, lpCapture);
		}
		else if (flTime < 4.2f)
		{
			glClear(GL_COLOR_BUFFER_BIT);
		}
		else if (flTime < 5.0f)
			glDrawPixels(iScreenWidth, iScreenHeight, GL_RGB, GL_UNSIGNED_BYTE, lpCapture);
		else if (flTime < 5.2f)
		{
			static int iFrame = 0;
			if (iFrame++ == 0)
				PlaySound(L"SystemAsterisk", 0, SND_ALIAS);
		}
		else if (flTime < 9.0f)
		{
			ilBindImage(iErrorWindow);
	        glWindowPos2i(iScreenWidth/2 - 366/2, iScreenHeight/2 - 168/2);
			glDrawPixels(366, 168, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
		}
		else if (flTime < 30.0f)
		{
			static int iFrame = 0;
			if (iFrame++ == 80)
				CSoundLibrary::PlayMusic("sound/assemble-for-victory.ogg");

			glDrawPixels(iScreenWidth, iScreenHeight, GL_RGB, GL_UNSIGNED_BYTE, lpCapture);

			ilBindImage(iErrorWindow);
	        glWindowPos2i(iScreenWidth/2 - 366/2, iScreenHeight/2 - 168/2);
			glDrawPixels(366, 168, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());

	        glWindowPos2i(0, 0);

			float flDistance = (flTime - 9)*20;
			int iColumns = 10;
			int iSpace = 60;
			float flTopLeftX = iScreenWidth - iColumns*iSpace/2.0f;
			float flTopLeftY = -iColumns*iSpace/2.0f;
			for (size_t i = 0; i < 10; i++)
			{
				for (size_t j = 0; j < (size_t)(flDistance/iSpace)+1; j++)
				{
					DrawTexture(iTankTexture, flTopLeftX - flDistance + i*iSpace + j*iSpace, flTopLeftY + flDistance + i*iSpace - j*iSpace, 196/2, 150/2);
				}
			}

			float flFade = RemapValClamped(flTime, 28, 30, 0, 1);
			if (flFade > 0)
				DrawFade(flFade);
		}
		else
			g_bSkip = true;

		TeardownFrame();

		MSG msg;
		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			switch( msg.message )
			{
				case WM_QUIT:
					break;

				default:
					DispatchMessage( &msg );
					break;
			}
		}

		SwapBuffers( hdcWindow );
	}

	DeleteObject(hbmCapture);

//	glBindTexture(GL_TEXTURE_2D, 0);

//	glDeleteTextures(1, &iGLId);

	ilDeleteImages(1, &iErrorWindow);
	ilDeleteImages(1, &iTankImage);
	glDeleteTextures(1, &iTankTexture);

	UnhookWindowsHookEx( g_hKeyboardHook );

	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hrcWindow );

	DeleteDC(hdcWindow);
	DestroyWindow(g_hWindow);
	UnregisterClass( L"dtintro", NULL );
}
