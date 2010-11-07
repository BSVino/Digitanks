#include <GL/glew.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <platform.h>

#include "ui/digitankswindow.h"

extern void RunIntro();

void CreateApplication(int argc, char** argv)
{
	CDigitanksWindow oWindow(argc, argv);

	if (!oWindow.HasCommandLineSwitch("--no-intro"))
		RunIntro();

	oWindow.OpenWindow();
	oWindow.Run();
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	// Make sure we open up an assert messagebox window instead of just aborting like it does for console apps.
	_set_error_mode(_OUT_TO_MSGBOX);

#ifndef _DEBUG
	__try
	{
#endif
#endif

		// Put in a different function to avoid warnings and errors associated with object deconstructors and try/catch blocks.
		CreateApplication(argc, argv);

#if defined(_WIN32) && !defined(_DEBUG)
	}
	__except (CreateMinidump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
}
