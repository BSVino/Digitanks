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
		if (argc < 2)
		{
			TMsg(tstring("Usage: ") + Shell()->GetBinaryName() + " input.obj [output.toy] [--physics input.obj]\n");
			return 1;
		}

		tstring sOutput;
		if (argc < 3)
			sOutput = tstring(args[1]).substr(0, strlen(args[1])-4) + ".toy";
		else
			sOutput = args[2];

		tstring sPhysics;
		if (Shell()->HasCommandLineSwitch("--physics"))
			sPhysics = Shell()->GetCommandLineSwitchValue("--physics");

		bool bGlobalTransformations = Shell()->HasCommandLineSwitch("--use-global-transforms");
		bool bAllowConcave = Shell()->HasCommandLineSwitch("--concave");

		if (!g.BuildFiles(sOutput, sInput, sPhysics, bGlobalTransformations, bAllowConcave))
			return 1;
	}

	return 0;
}
