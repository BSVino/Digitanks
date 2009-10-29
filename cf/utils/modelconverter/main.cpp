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
	else if (strcmp(pszExtension, ".dae") == 0)
	{
		printf("Reading the Collada .dae file... ");
		c.ReadDAE(argv[1]);
	}

	printf("Done.\n");
	printf("\n");

	printf("-------------\n");
	printf("Materials   : %d\n", c.GetScene()->GetNumMaterials());
	printf("Meshes      : %d\n", c.GetScene()->GetNumMeshes());
	printf("\n");

	for (size_t i = 0; i < c.GetScene()->GetNumMeshes(); i++)
	{
		CConversionMesh* pMesh = c.GetScene()->GetMesh(i);

		printf("-------------\n");
		printf("Mesh: %s\n", pMesh->GetBoneName(0));
		printf("Vertices    : %d\n", pMesh->GetNumVertices());
		printf("Normals     : %d\n", pMesh->GetNumNormals());
		printf("UVs         : %d\n", pMesh->GetNumUVs());
		printf("Bones       : %d\n", pMesh->GetNumBones());
		printf("Faces       : %d\n", pMesh->GetNumFaces());
		printf("\n");
	}

	printf("Writing the SMD... ");
	c.WriteSMDs();
	printf("Done.\n");
	printf("\n");

	printf("Press enter to continue...\n");
	getchar();

	return 0;
}
