#include "intro_window.h"

extern tvector<Color> FullScreenShot(int& iWidth, int& iHeight);

void CreateApplication(int argc, char** argv)
{
	int iWidth, iHeight;
	tvector<Color> aclrScreenshot = FullScreenShot(iWidth, iHeight);

	CIntroWindow oWindow(argc, argv);

	oWindow.SetScreenshot(aclrScreenshot, iWidth, iHeight);
	oWindow.OpenWindow();
	oWindow.SetupEngine();
	oWindow.Run();
}

int main(int argc, char** argv)
{
	CreateApplicationWithErrorHandling(CreateApplication, argc, argv);
}
