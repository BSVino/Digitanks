#include "ui/digitankswindow.h"

#include <platform.h>

void CreateApplication(int argc, char** argv)
{
	CDigitanksWindow oWindow(argc, argv);

	if (!oWindow.HasCommandLineSwitch("--no-intro"))
		Exec("dtintro");

	oWindow.OpenWindow();
	oWindow.Run();
}

int main(int argc, char** argv)
{
	CreateApplicationWithErrorHandling(CreateApplication, argc, argv);
}
