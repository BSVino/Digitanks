#include <stdio.h>

#include "modelconverter.h"

#include <direct.h>

int main(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("I need a file to convert!\n");
		return 0;
	}

	size_t iFileLength = strlen(argv[1]);
	const char* pszExtension = argv[1]+iFileLength-4;

	CModelConverter c;

	if (strcmp(pszExtension, ".obj") == 0)
	{
		printf("Reading the OBJ... ");
		c.ReadOBJ(argv[1]);
	}
	else if (strcmp(pszExtension, ".sia") == 0)
	{
		printf("Reading the Silo ASCII file... ");
		c.ReadSIA(argv[1]);
	}

	printf("Done.\n");
	printf("\n");

	printf("-------------\n");
	printf("Vertices    : %d\n", c.m_Mesh.GetNumVertices());
	printf("Normals     : %d\n", c.m_Mesh.GetNumNormals());
	printf("UVs         : %d\n", c.m_Mesh.GetNumUVs());
	printf("Bones       : %d\n", c.m_Mesh.GetNumBones());
	printf("Materials   : %d\n", c.m_Mesh.GetNumMaterials());
	printf("Faces       : %d\n", c.m_Mesh.GetNumFaces());
	printf("\n");

	printf("Writing the SMD... ");
	c.WriteSMD(argv[1]);
	printf("Done.\n");
	printf("\n");

	printf("Press enter to continue...\n");
	getchar();

	return 0;
}
