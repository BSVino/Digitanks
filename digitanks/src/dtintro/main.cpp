#include "intro_window.h"

extern size_t FullScreenShot();

void CreateApplication(int argc, char** argv)
{
	size_t iScreenshot = FullScreenShot();

	CIntroWindow oWindow(argc, argv);

	oWindow.SetScreenshot(iScreenshot);
	oWindow.OpenWindow();
	oWindow.SetupIntro();
	oWindow.Run();
}

int main(int argc, char** argv)
{
	CreateApplicationWithErrorHandling(CreateApplication, argc, argv);
}
