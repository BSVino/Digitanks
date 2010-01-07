#include "license.h"

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("ERROR: Missing product key, couldn't generate an unlock code.\n");
		return 1;
	}

	std::string sKey;
	GenerateKey(std::string(argv[1]), sKey);

	puts(sKey.c_str());

	return 0;
}
