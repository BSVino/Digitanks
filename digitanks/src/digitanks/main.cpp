#include "ui/digitankswindow.h"

#include <tinker_platform.h>

#include <SDL_main.h>

void CreateApplication(int argc, char** argv)
{
	CDigitanksWindow oWindow(argc, argv);

#ifndef __ANDROID__
	if (!oWindow.HasCommandLineSwitch("--no-intro"))
		Exec("dtintro");
#endif

	oWindow.OpenWindow();
	oWindow.Run();
}

int main(int argc, char** argv)
{
	CreateApplicationWithErrorHandling(CreateApplication, argc, argv);

	return 0;
}
