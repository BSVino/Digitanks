#include "ui/digitankswindow.h"

int main(int argc, char** argv)
{
#ifdef _WIN32
	// Make sure we open up an assert messagebox window instead of just aborting like it does for console apps.
	_set_error_mode(_OUT_TO_MSGBOX);
#endif

	CDigitanksWindow oWindow(argc, argv);

	oWindow.Run();
}
