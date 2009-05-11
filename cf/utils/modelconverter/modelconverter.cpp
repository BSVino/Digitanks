#include <stdio.h>
#include <string.h>

#include "modelconverter.h"

void CModelConverter::ReadOBJ(const char* pszFilename)
{
	FILE* fp = fopen(pszFilename, "r");

	if (!fp)
	{
		printf("No input file. Sorry!\n");
		return;
	}

	m_Mesh.Clear();

	size_t iCurrentMaterial = ~0;

	char szLine[1024];
	char* pszLine = NULL;
	while (pszLine = fgets(szLine, sizeof(szLine), fp))
	{
		pszLine = StripWhitespace(pszLine);

		if (strlen(pszLine) == 0)
			continue;

		if (pszLine[0] == '#')
			continue;

		char szToken[1024];
		strcpy(szToken, pszLine);
		char* pszToken = NULL;
		pszToken = strtok(szToken, " ");

		if (strcmp(pszToken, "mtllib") == 0)
		{
			// External file, material library.
			// We ignore it.
		}
		else if (strcmp(pszToken, "o") == 0)
		{
			// Dunno what this does.
		}
		else if (strcmp(pszToken, "v") == 0)
		{
			// A vertex.
			float x, y, z;
			sscanf(pszLine, "v %f %f %f", &x, &y, &z);
			m_Mesh.AddVertex(x, y, z);
		}
		else if (strcmp(pszToken, "vn") == 0)
		{
			// A vertex normal.
			float x, y, z;
			sscanf(pszLine, "vn %f %f %f", &x, &y, &z);
			m_Mesh.AddNormal(x, y, z);
		}
		else if (strcmp(pszToken, "vt") == 0)
		{
			// A UV coordinate for a vertex.
			float u, v;
			sscanf(pszLine, "vt %f %f", &u, &v);
			m_Mesh.AddUV(u, v);
		}
		else if (strcmp(pszToken, "g") == 0)
		{
			// A group of faces.
			m_Mesh.AddBone(pszLine+2);
		}
		else if (strcmp(pszToken, "usemtl") == 0)
		{
			// All following faces should use this material.
			char* pszMaterial = pszLine+7;
			size_t iMaterial = m_Mesh.FindMaterial(pszMaterial);
			if (iMaterial == ((size_t)~0))
				iCurrentMaterial = m_Mesh.AddMaterial(pszMaterial);
			else
				iCurrentMaterial = iMaterial;
		}
		else if (strcmp(pszToken, "f") == 0)
		{
			// A face.
			size_t iFace = m_Mesh.AddFace(iCurrentMaterial);

			while (pszToken = strtok(NULL, " "))
			{
				size_t v, vt, vn;
				if (sscanf(pszToken, "%d/%d/%d", &v, &vt, &vn) == 3)
					m_Mesh.AddVertexToFace(iFace, v-1, vt-1, vn-1);
				else
					printf("WARNING! Found an invalid vertex while loading faces.\n");
			}

			if (m_Mesh.GetFace(iFace)->GetNumVertices() == 0)
			{
				printf("WARNING! Removing an empty face. Probably it doesn't have any UV map on it.\n");
				m_Mesh.RemoveFace(iFace);
			}
		}
	}

	fclose(fp);
}

void CModelConverter::WriteSMD(const char* pszFilename)
{
	char szFile[1024];
	strcpy(szFile, pszFilename);

	char* pszFile = MakeFilename(szFile);

	std::string sSMDFile(pszFile);
	sSMDFile.append(".smd");

	FILE* fp = fopen(sSMDFile.c_str(), "w");

	// SMD file format: http://developer.valvesoftware.com/wiki/SMD

	// Header section
	fprintf(fp, "version 1\n\n");

	// Nodes section
	fprintf(fp, "nodes\n");
	// Only bothering with one node, we're only doing static props with this code for now.
	fprintf(fp, "0 \"%s\" -1\n", m_Mesh.GetBoneName(0));
	fprintf(fp, "end\n\n");

	// Skeleton section
	fprintf(fp, "skeleton\n");
	fprintf(fp, "time 0\n");
	fprintf(fp, "0 0.000000 0.000000 0.000000 1.570796 0.000000 0.0000001\n");
	fprintf(fp, "end\n\n");
	
	fprintf(fp, "triangles\n");
	for (size_t i = 0; i < m_Mesh.GetNumFaces(); i++)
	{
		CConversionFace* pFace = m_Mesh.GetFace(i);

		if (!pFace->GetNumVertices())
		{
			printf("WARNING! Found a face with no vertices.\n");
			continue;
		}

		CConversionVertex* pV1 = pFace->GetVertex(0);

		for (size_t j = 0; j < pFace->GetNumVertices()-2; j++)
		{
			CConversionVertex* pV2 = pFace->GetVertex(j+1);
			CConversionVertex* pV3 = pFace->GetVertex(j+2);

			Vector v1 = m_Mesh.GetVertex(pV1->v);
			Vector v2 = m_Mesh.GetVertex(pV2->v);
			Vector v3 = m_Mesh.GetVertex(pV3->v);

			Vector n1 = m_Mesh.GetNormal(pV1->vn);
			Vector n2 = m_Mesh.GetNormal(pV2->vn);
			Vector n3 = m_Mesh.GetNormal(pV3->vn);

			Vector uv1 = m_Mesh.GetUV(pV1->vt);
			Vector uv2 = m_Mesh.GetUV(pV2->vt);
			Vector uv3 = m_Mesh.GetUV(pV3->vt);

			// Material name
			size_t iMaterial = pFace->m;

			if (iMaterial == ((size_t)~0))
				fprintf(fp, "# ERROR! Can't find a material for this triangle.\nerror\n");
			else
				fprintf(fp, "%s\n", m_Mesh.GetMaterial(iMaterial)->GetName());

			// <int|Parent bone> <float|PosX PosY PosZ> <normal|NormX NormY NormZ> <normal|U V>
			// Studio coordinates are not the same as game coordinates. Studio (x, y, z) is game (x, -z, y) and vice versa.
			fprintf(fp, "0 \t %f %f %f \t %f %f %f \t %f %f\n", v1.x, -v1.z, v1.y, n1.x, -n1.z, n1.y, uv1.x, uv1.y);
			fprintf(fp, "0 \t %f %f %f \t %f %f %f \t %f %f\n", v2.x, -v2.z, v2.y, n2.x, -n2.z, n2.y, uv2.x, uv2.y);
			fprintf(fp, "0 \t %f %f %f \t %f %f %f \t %f %f\n", v3.x, -v3.z, v3.y, n3.x, -n3.z, n3.y, uv3.x, uv3.y);
		}
	}
	fprintf(fp, "end\n");

	fclose(fp);
}

// Takes a path + filename + extension and removes path and extension to return only the filename.
// It is destructive on the string that is passed into it.
char* CModelConverter::MakeFilename(char* pszPathFilename)
{
	char* pszFile = pszPathFilename;
	int iLastChar = -1;
	int i = -1;

	while (pszFile[++i])
		if (pszFile[i] == '\\' || pszFile[i] == '/')
			iLastChar = i;

	pszFile += iLastChar+1;

	i = -1;
	while (pszFile[++i])
		if (pszFile[i] == '.')
			iLastChar = i;

	pszFile[iLastChar] = '\0';

	return pszFile;
}

bool CModelConverter::IsWhitespace(char cChar)
{
	return (cChar == ' ' || cChar == '\t' || cChar == '\r' || cChar == '\n');
}

char* CModelConverter::StripWhitespace(char* pszLine)
{
	if (!pszLine)
		return NULL;

	char* pszSpace = pszLine;
	while (IsWhitespace(pszSpace[0]) && pszSpace[0] != '\0')
		pszSpace++;

	int iEnd = ((int)strlen(pszSpace))-1;
	while (iEnd >= 0 && IsWhitespace(pszSpace[iEnd]))
		iEnd--;

	if (iEnd >= -1)
	{
		pszSpace[iEnd+1] = '\0';
	}

	return pszSpace;
}