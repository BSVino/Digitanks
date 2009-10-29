#include <modelconverter/modelconverter.h>
#include "ui/modelwindow.h"

int main(int argc, char** argv)
{
	CModelWindow oWindow;

	if (argc >= 2)
	{
		size_t iFileLength = strlen(argv[1]);
		const char* pszExtension = argv[1]+iFileLength-4;

		CModelConverter c;

		if (strcmp(pszExtension, ".obj") == 0)
			c.ReadOBJ(argv[1]);
		else if (strcmp(pszExtension, ".sia") == 0)
			c.ReadSIA(argv[1]);
		else if (strcmp(pszExtension, ".dae") == 0)
			c.ReadDAE(argv[1]);

		oWindow.LoadFromScene(c.GetScene());
	}

	oWindow.Run();
}