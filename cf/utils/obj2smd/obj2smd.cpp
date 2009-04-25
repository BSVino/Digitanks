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

	CModelConverter c;

	printf("Reading the OBJ... ");
	c.ReadOBJ(argv[1]);
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

	printf("Press any key to continue...\n");
	getchar();

	return 0;
}
