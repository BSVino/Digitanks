#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "modelconverter.h"
#include "strutils.h"

void CModelConverter::ReadOBJ(const char* pszFilename)
{
	FILE* fp = fopen(pszFilename, "r");

	if (!fp)
	{
		printf("No input file. Sorry!\n");
		return;
	}

	CConversionMesh* pMesh = m_Scene.GetMesh(m_Scene.AddMesh(pszFilename));

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
			pMesh->AddVertex(x, y, z);
		}
		else if (strcmp(pszToken, "vn") == 0)
		{
			// A vertex normal.
			float x, y, z;
			sscanf(pszLine, "vn %f %f %f", &x, &y, &z);
			pMesh->AddNormal(x, y, z);
		}
		else if (strcmp(pszToken, "vt") == 0)
		{
			// A UV coordinate for a vertex.
			float u, v;
			sscanf(pszLine, "vt %f %f", &u, &v);
			pMesh->AddUV(u, v);
		}
		else if (strcmp(pszToken, "g") == 0)
		{
			// A group of faces.
			pMesh->AddBone(pszLine+2);
		}
		else if (strcmp(pszToken, "usemtl") == 0)
		{
			// All following faces should use this material.
			char* pszMaterial = pszLine+7;
			size_t iMaterial = m_Scene.FindMaterial(pszMaterial);
			if (iMaterial == ((size_t)~0))
				iCurrentMaterial = m_Scene.AddMaterial(pszMaterial);
			else
				iCurrentMaterial = iMaterial;
		}
		else if (strcmp(pszToken, "f") == 0)
		{
			// A face.
			size_t iFace = pMesh->AddFace(iCurrentMaterial);

			while (pszToken = strtok(NULL, " "))
			{
				size_t v, vt, vn;
				if (sscanf(pszToken, "%d/%d/%d", &v, &vt, &vn) == 3)
					pMesh->AddVertexToFace(iFace, v-1, vt-1, vn-1);
				else
					printf("WARNING! Found an invalid vertex while loading faces.\n");
			}

			if (pMesh->GetFace(iFace)->GetNumVertices() == 0)
			{
				printf("WARNING! Removing an empty face. Probably it doesn't have any UV map on it.\n");
				pMesh->RemoveFace(iFace);
			}
		}
	}

	fclose(fp);
}

// Silo ascii
void CModelConverter::ReadSIA(const char* pszFilename)
{
	std::ifstream infile;
	infile.open(pszFilename, std::ifstream::in);

	if (!infile.is_open())
	{
		printf("No input file. Sorry!\n");
		return;
	}

	std::string sLine;
	while (infile.good())
	{
		std::getline(infile, sLine);

		sLine = StripWhitespace(sLine);

		if (sLine.length() == 0)
			continue;

		std::vector<std::string> aTokens;
		strtok(sLine, aTokens, " ");
		const char* pszToken = aTokens[0].c_str();

		if (strcmp(pszToken, "-Version") == 0)
		{
			// Warning if version is later than 1.0, we may not support it
			int iMajor, iMinor;
			sscanf(sLine.c_str(), "-Version %d.%d", &iMajor, &iMinor);
			if (iMajor != 1 && iMinor != 0)
				printf("WARNING: I was programmed for version 1.0, this file is version %d.%d, so this might not work exactly right!\n", iMajor, iMinor);
		}
		else if (strcmp(pszToken, "-Mat") == 0)
		{
			ReadSIAMat(infile);
		}
		else if (strcmp(pszToken, "-Shape") == 0)
		{
			ReadSIAShape(infile);
		}
		else if (strcmp(pszToken, "-Texshape") == 0)
		{
			// This is the 3d UV space of the object, but we only care about its 2d UV space which is contained in rhw -Shape section, so meh.
			ReadSIAShape(infile, false);
		}
	}

	for (size_t i = 0; i < m_Scene.GetNumMeshes(); i++)
	{
		m_Scene.GetMesh(i)->CalculateEdgeData();
		m_Scene.GetMesh(i)->CalculateVertexNormals();
		m_Scene.GetMesh(i)->TranslateOrigin();
	}

	infile.close();
}

void CModelConverter::ReadSIAMat(std::ifstream& infile)
{
	std::string sLine;
	while (infile.good())
	{
		std::getline(infile, sLine);

		sLine = StripWhitespace(sLine);

		if (sLine.length() == 0)
			continue;

		std::vector<std::string> aTokens;
		strtok(sLine, aTokens, " ");
		const char* pszToken = aTokens[0].c_str();

		if (strcmp(pszToken, "-name") == 0)
		{
			// All we care about is the name.
			const char* pszMaterial = sLine.c_str()+6;
			std::string sName = pszMaterial;
			std::vector<std::string> aName;
			strtok(sName, aName, "\"");	// Strip out the quotation marks.

			size_t iMaterial = m_Scene.FindMaterial(aName[0].c_str());
			if (iMaterial == ((size_t)~0))
				m_Scene.AddMaterial(aName[0].c_str());
		}
		else if (strcmp(pszToken, "-endMat") == 0)
		{
			return;
		}
	}
}

void CModelConverter::ReadSIAShape(std::ifstream& infile, bool bCare)
{
	size_t iCurrentMaterial = ~0;

	CConversionMesh* pMesh = NULL;
	size_t iAddV = 0;
	size_t iAddE = 0;
	size_t iAddUV = 0;
	size_t iAddN = 0;

	std::string sLine;
	while (infile.good())
	{
		std::getline(infile, sLine);

		sLine = StripWhitespace(sLine);

		if (sLine.length() == 0)
			continue;

		std::vector<std::string> aTokens;
		strtok(sLine, aTokens, " ");
		const char* pszToken = aTokens[0].c_str();

		if (!bCare)
		{
			if (strcmp(pszToken, "-endShape") == 0)
				return;
			else
				continue;
		}

		if (strcmp(pszToken, "-snam") == 0)
		{
			// We name our mesh.
			std::string sName = sLine.c_str()+6;
			std::vector<std::string> aName;
			strtok(sName, aName, "\"");	// Strip out the quotation marks.

			if (bCare)
			{
				size_t iMesh = m_Scene.FindMesh(aName[0].c_str());
				if (iMesh == (size_t)~0)
				{
					iMesh = m_Scene.AddMesh(aName[0].c_str());
					pMesh = m_Scene.GetMesh(iMesh);
					pMesh->AddBone(aName[0].c_str());
				}
				else
				{
					pMesh = m_Scene.GetMesh(iMesh);
					iAddV = pMesh->GetNumVertices();
					iAddE = pMesh->GetNumEdges();
					iAddUV = pMesh->GetNumUVs();
					iAddN = pMesh->GetNumNormals();
				}
			}
		}
		else if (strcmp(pszToken, "-vert") == 0)
		{
			// A vertex.
			float x, y, z;
			sscanf(sLine.c_str(), "-vert %f %f %f", &x, &y, &z);
			pMesh->AddVertex(x, y, z);
		}
		else if (strcmp(pszToken, "-edge") == 0)
		{
			// An edge. We only need them so we can tell where the creases are, so we can calculate normals properly.
			int v1, v2;
			sscanf(sLine.c_str(), "-edge %d %d", &v1, &v2);
			pMesh->AddEdge(v1+iAddV, v2+iAddV);
		}
		else if (strcmp(pszToken, "-creas") == 0)
		{
			// An edge. We only need them so we can tell where the creases are, so we can calculate normals properly.
			std::string sCreases = sLine.c_str()+7;
			std::vector<std::string> aCreases;
			strtok(sCreases, aCreases, " ");

			for (size_t i = 1; i < aCreases.size(); i++)
				pMesh->GetEdge(atoi(aCreases[i].c_str())+iAddE)->m_bCreased = true;
		}
		else if (strcmp(pszToken, "-setmat") == 0)
		{
			const char* pszMaterial = sLine.c_str()+8;
			iCurrentMaterial = atoi(pszMaterial);
		}
		else if (strcmp(pszToken, "-face") == 0)
		{
			// A face.
			size_t iFace = pMesh->AddFace(iCurrentMaterial);

			std::string sFaces = sLine.c_str()+8;
			std::vector<std::string> aFaces;
			strtok(sFaces, aFaces, " ");

			for (size_t i = 0; i < aFaces.size(); i += 4)
			{
				size_t iVertex = atoi(aFaces[i].c_str())+iAddV;
				size_t iEdge = atoi(aFaces[i+1].c_str())+iAddE;

				float flU = (float)atof(aFaces[i+2].c_str());
				float flV = (float)atof(aFaces[i+3].c_str());
				size_t iUV = pMesh->AddUV(flU, flV);
				size_t iNormal = pMesh->AddNormal(0, 0, 1);	// For now!

				pMesh->AddVertexToFace(iFace, iVertex, iUV, iNormal);
				pMesh->AddEdgeToFace(iFace, iEdge);
			}
		}
		else if (strcmp(pszToken, "-axis") == 0)
		{
			// Object's center point. There is rotation information included in this node, but we don't use it at the moment.
			float x, y, z;
			sscanf(sLine.c_str(), "-axis %f %f %f", &x, &y, &z);
			pMesh->m_vecOrigin = Vector(x, y, z);
		}
		else if (strcmp(pszToken, "-endShape") == 0)
		{
			break;
		}
	}
}

void CModelConverter::WriteSMDs()
{
	for (size_t i = 0; i < m_Scene.GetNumMeshes(); i++)
		WriteSMD(i);
}

void CModelConverter::WriteSMD(size_t iMesh)
{
	CConversionMesh* pMesh = m_Scene.GetMesh(iMesh);

	char szFile[1024];
	strcpy(szFile, pMesh->GetBoneName(0));

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
	fprintf(fp, "0 \"%s\" -1\n", pMesh->GetBoneName(0));
	fprintf(fp, "end\n\n");

	// Skeleton section
	fprintf(fp, "skeleton\n");
	fprintf(fp, "time 0\n");
	fprintf(fp, "0 0.000000 0.000000 0.000000 1.570796 0.000000 0.0000001\n");
	fprintf(fp, "end\n\n");
	
	fprintf(fp, "triangles\n");
	for (size_t i = 0; i < pMesh->GetNumFaces(); i++)
	{
		CConversionFace* pFace = pMesh->GetFace(i);

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

			Vector v1 = pMesh->GetVertex(pV1->v);
			Vector v2 = pMesh->GetVertex(pV2->v);
			Vector v3 = pMesh->GetVertex(pV3->v);

			Vector n1 = pMesh->GetNormal(pV1->vn);
			Vector n2 = pMesh->GetNormal(pV2->vn);
			Vector n3 = pMesh->GetNormal(pV3->vn);

			Vector uv1 = pMesh->GetUV(pV1->vt);
			Vector uv2 = pMesh->GetUV(pV2->vt);
			Vector uv3 = pMesh->GetUV(pV3->vt);

			// Material name
			size_t iMaterial = pFace->m;

			if (iMaterial == ((size_t)~0))
			{
				printf("ERROR! Can't find a material for a triangle.\n");
				fprintf(fp, "error\n");
			}
			else
				fprintf(fp, "%s\n", m_Scene.GetMaterial(iMaterial)->GetName());

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

std::string CModelConverter::StripWhitespace(std::string sLine)
{
	int i = 0;
	while (IsWhitespace(sLine[i]) && sLine[i] != '\0')
		i++;

	int iEnd = ((int)sLine.length())-1;
	while (iEnd >= 0 && IsWhitespace(sLine[iEnd]))
		iEnd--;

	if (iEnd >= -1)
		sLine[iEnd+1] = '\0';

	return sLine.substr(i);
}