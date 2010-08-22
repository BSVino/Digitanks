#include "ui/digitankswindow.h"
#include <platform.h>

#ifdef _WIN32
#include <Windows.h>
#endif

void CreateApplication(int argc, char** argv)
{
	CDigitanksWindow oWindow(argc, argv);

	oWindow.Run();
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	// Make sure we open up an assert messagebox window instead of just aborting like it does for console apps.
	_set_error_mode(_OUT_TO_MSGBOX);

	__try
	{
#endif

		int* p = NULL;
		*p = 42;

		// Put in a different function to avoid warnings and errors associated with object deconstructors and try/catch blocks.
		CreateApplication(argc, argv);

#ifdef _WIN32
	}
	__except (CreateMinidump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
}
