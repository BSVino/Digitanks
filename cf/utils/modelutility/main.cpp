#include <modelconverter/modelconverter.h>
#include "ui/modelwindow.h"

int main(int argc, char** argv)
{
	CModelWindow oWindow;

	if (argc >= 2)
	{
		wchar_t szFile[1024];
		mbstowcs(szFile, argv[1], strlen(argv[1])+1);

		oWindow.ReadFile(szFile);
	}

	oWindow.Run();
}
