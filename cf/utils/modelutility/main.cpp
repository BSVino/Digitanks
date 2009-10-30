#include <modelconverter/modelconverter.h>
#include "ui/modelwindow.h"

int main(int argc, char** argv)
{
	CModelWindow oWindow;

	if (argc >= 2)
		oWindow.ReadFile(argv[1]);

	oWindow.Run();
}