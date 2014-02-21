#include <stdio.h>

#include <tinker_platform.h>

#include <tinker/shell.h>

#include "geppetto.h"

class CToyBuilder : public CShell
{
public:
	CToyBuilder(int argc, char** args) : CShell(argc, args) {};
};

int main(int argc, char** args)
{
	CToyBuilder c(argc, args);

	TMsg("Toy Builder for Lunar Workshop's Tinker Engine\n");

	if (argc <= 1)
	{
		TMsg(tstring("Usage: ") + Shell()->GetBinaryName() + " input.obj output.toy [--physics input.obj]\n");
		TMsg(tstring("Usage: ") + Shell()->GetBinaryName() + " input.txt\n");
		return 1;
	}

	CGeppetto g(Shell()->HasCommandLineSwitch("--force"));

	tstring sInput = args[1];

	if (sInput.endswith(".txt"))
	{
		if (!g.BuildFromInputScript(sInput))
		{
			TMsg(tstring("Usage: ") + Shell()->GetBinaryName() + " input.txt\n");
			TMsg("- input.txt must specify a game directory and an output file.\n");
			return 1;
		}
	}
	else
	{
		if (argc < 3)
		{
			TMsg(tstring("Usage: ") + Shell()->GetBinaryName() + " input.obj output.toy [--physics input.obj]\n");
			return 1;
		}

		tstring sOutput = args[2];
		tstring sPhysics;
		bool bGlobalTransformations = false;

		for (int i = 3; i < argc; i++)
		{
			if (strcmp(args[i], "--physics") == 0)
			{
				if (argc < i+1)
				{
					TMsg(tstring("Usage: ") + Shell()->GetBinaryName() + " input.obj output.toy [--physics input.obj]\n");
					return 1;
				}

				sPhysics = args[i+1];
				i++;
			}
			else if (strcmp(args[i], "--use-global-transforms") == 0)
				bGlobalTransformations = true;
		}

		if (!g.BuildFiles(sOutput, sInput, sPhysics, bGlobalTransformations))
			return 1;
	}

	return 0;
}
