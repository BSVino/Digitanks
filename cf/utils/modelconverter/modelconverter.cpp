#include <stdio.h>
#include <string.h>

#include "modelconverter.h"
#include "strutils.h"

CModelConverter::CModelConverter(CConversionScene* pScene)
{
	m_pScene = pScene;
}

void CModelConverter::ReadOBJ(const wchar_t* pszFilename)
{
	FILE* fp = _wfopen(pszFilename, L"r");

	if (!fp)
	{
		printf("No input file. Sorry!\n");
		return;
	}

	CConversionMesh* pMesh = m_pScene->GetMesh(m_pScene->AddMesh(pszFilename));

	size_t iCurrentMaterial = ~0;

	const size_t iChars = 1024;
	wchar_t szLine[iChars];
	wchar_t* pszLine = NULL;
	while (pszLine = fgetws(szLine, iChars, fp))
	{
		pszLine = StripWhitespace(pszLine);

		if (wcslen(pszLine) == 0)
			continue;

		if (pszLine[0] == '#')
			continue;

		wchar_t szToken[1024];
		wcscpy(szToken, pszLine);
		wchar_t* pszToken = NULL;
		pszToken = wcstok(szToken, L" ");

		if (wcscmp(pszToken, L"mtllib") == 0)
		{
			wchar_t szDirectory[1024];
			wcscpy(szDirectory, pszFilename);
			GetDirectory(szDirectory);
			wchar_t szMaterial[1024];
			swprintf(szMaterial, L"%s/%s", szDirectory, pszLine + 7);
			ReadMTL(szMaterial);
		}
		else if (wcscmp(pszToken, L"o") == 0)
		{
			// Dunno what this does.
		}
		else if (wcscmp(pszToken, L"v") == 0)
		{
			// A vertex.
			float x, y, z;
			swscanf(pszLine, L"v %f %f %f", &x, &y, &z);
			pMesh->AddVertex(x, y, z);
		}
		else if (wcscmp(pszToken, L"vn") == 0)
		{
			// A vertex normal.
			float x, y, z;
			swscanf(pszLine, L"vn %f %f %f", &x, &y, &z);
			pMesh->AddNormal(x, y, z);
		}
		else if (wcscmp(pszToken, L"vt") == 0)
		{
			// A UV coordinate for a vertex.
			float u, v;
			swscanf(pszLine, L"vt %f %f", &u, &v);
			pMesh->AddUV(u, v);
		}
		else if (wcscmp(pszToken, L"g") == 0)
		{
			// A group of faces.
			pMesh->AddBone(pszLine+2);
		}
		else if (wcscmp(pszToken, L"usemtl") == 0)
		{
			// All following faces should use this material.
			wchar_t* pszMaterial = pszLine+7;
			size_t iMaterial = m_pScene->FindMaterial(pszMaterial);
			if (iMaterial == ((size_t)~0))
				iCurrentMaterial = m_pScene->AddMaterial(pszMaterial);
			else
				iCurrentMaterial = iMaterial;
		}
		else if (wcscmp(pszToken, L"f") == 0)
		{
			// A face.
			size_t iFace = pMesh->AddFace(iCurrentMaterial);

			while (pszToken = wcstok(NULL, L" "))
			{
				size_t v, vt, vn;
				if (swscanf(pszToken, L"%d/%d/%d", &v, &vt, &vn) == 3)
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

	m_pScene->CalculateExtends();
}

void CModelConverter::ReadMTL(const wchar_t* pszFilename)
{
	FILE* fp = _wfopen(pszFilename, L"r");

	if (!fp)
		return;

	size_t iCurrentMaterial = ~0;

	const size_t iCount = 1024;
	wchar_t szLine[iCount];
	wchar_t* pszLine = NULL;
	while (pszLine = fgetws(szLine, iCount, fp))
	{
		pszLine = StripWhitespace(pszLine);

		if (wcslen(pszLine) == 0)
			continue;

		if (pszLine[0] == '#')
			continue;

		wchar_t szToken[1024];
		wcscpy(szToken, pszLine);
		wchar_t* pszToken = NULL;
		pszToken = wcstok(szToken, L" ");

		CConversionMaterial* pMaterial = NULL;
		if (iCurrentMaterial != ~0)
			pMaterial = m_pScene->GetMaterial(iCurrentMaterial);

		if (wcscmp(pszToken, L"newmtl") == 0)
		{
			pszToken = wcstok(NULL, L" ");
			iCurrentMaterial = m_pScene->AddMaterial(CConversionMaterial(pszToken, Vector(0.2f,0.2f,0.2f), Vector(0.8f,0.8f,0.8f), Vector(1,1,1), Vector(0,0,0), 1.0, 0));
		}
		else if (wcscmp(pszToken, L"Ka") == 0)
		{
			float r, g, b;
			swscanf(pszLine, L"Ka %f %f %f", &r, &g, &b);
			pMaterial->m_vecAmbient.x = r;
			pMaterial->m_vecAmbient.y = g;
			pMaterial->m_vecAmbient.z = b;
		}
		else if (wcscmp(pszToken, L"Kd") == 0)
		{
			float r, g, b;
			swscanf(pszLine, L"Kd %f %f %f", &r, &g, &b);
			pMaterial->m_vecDiffuse.x = r;
			pMaterial->m_vecDiffuse.y = g;
			pMaterial->m_vecDiffuse.z = b;
		}
		else if (wcscmp(pszToken, L"Ks") == 0)
		{
			float r, g, b;
			swscanf(pszLine, L"Ks %f %f %f", &r, &g, &b);
			pMaterial->m_vecSpecular.x = r;
			pMaterial->m_vecSpecular.y = g;
			pMaterial->m_vecSpecular.z = b;
		}
		else if (wcscmp(pszToken, L"d") == 0 || wcscmp(pszToken, L"Tr") == 0)
		{
			pszToken = wcstok(NULL, L" ");
			pMaterial->m_flTransparency = (float)_wtof(pszToken);
		}
		else if (wcscmp(pszToken, L"Ns") == 0)
		{
			pszToken = wcstok(NULL, L" ");
			pMaterial->m_flShininess = (float)_wtof(pszToken);
		}
		else if (wcscmp(pszToken, L"illum") == 0)
		{
			pszToken = wcstok(NULL, L" ");
			pMaterial->m_eIllumType = (IllumType_t)_wtoi(pszToken);
		}
		else if (wcscmp(pszToken, L"map_Kd") == 0)
		{
			pszToken = wcstok(NULL, L" ");

			if (FILE* fpTest = _wfopen(pszToken, L"r"))
			{
				fclose(fpTest);

				wcscpy(pMaterial->m_szTexture, pszToken);
			}
			else
			{
				wchar_t szDirectory[1024];
				wcscpy(szDirectory, pszFilename);
				GetDirectory(szDirectory);
				wchar_t szTexture[1024];
				swprintf(szTexture, L"%s/%s", szDirectory, pszToken);

				wcscpy(pMaterial->m_szTexture, szTexture);
			}
		}
	}

	fclose(fp);
}

// Silo ascii
void CModelConverter::ReadSIA(const wchar_t* pszFilename)
{
	std::wifstream infile;
	infile.open(pszFilename, std::wifstream::in);

	if (!infile.is_open())
	{
		printf("No input file. Sorry!\n");
		return;
	}

	std::wstring sLine;
	while (infile.good())
	{
		std::getline(infile, sLine);

		sLine = StripWhitespace(sLine);

		if (sLine.length() == 0)
			continue;

		std::vector<std::wstring> aTokens;
		wcstok(sLine, aTokens, L" ");
		const wchar_t* pszToken = aTokens[0].c_str();

		if (wcscmp(pszToken, L"-Version") == 0)
		{
			// Warning if version is later than 1.0, we may not support it
			int iMajor, iMinor;
			swscanf(sLine.c_str(), L"-Version %d.%d", &iMajor, &iMinor);
			if (iMajor != 1 && iMinor != 0)
				printf("WARNING: I was programmed for version 1.0, this file is version %d.%d, so this might not work exactly right!\n", iMajor, iMinor);
		}
		else if (wcscmp(pszToken, L"-Mat") == 0)
		{
			ReadSIAMat(infile, pszFilename);
		}
		else if (wcscmp(pszToken, L"-Shape") == 0)
		{
			ReadSIAShape(infile);
		}
		else if (wcscmp(pszToken, L"-Texshape") == 0)
		{
			// This is the 3d UV space of the object, but we only care about its 2d UV space which is contained in rhw -Shape section, so meh.
			ReadSIAShape(infile, false);
		}
	}

	infile.close();

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
	{
		m_pScene->GetMesh(i)->CalculateEdgeData();
		m_pScene->GetMesh(i)->CalculateVertexNormals();
		m_pScene->GetMesh(i)->TranslateOrigin();
	}

	m_pScene->CalculateExtends();
}

void CModelConverter::ReadSIAMat(std::wifstream& infile, const wchar_t* pszFilename)
{
	size_t iCurrentMaterial = m_pScene->AddMaterial(L"");
	CConversionMaterial* pMaterial = m_pScene->GetMaterial(iCurrentMaterial);

	std::wstring sLine;
	while (infile.good())
	{
		std::getline(infile, sLine);

		sLine = StripWhitespace(sLine);

		if (sLine.length() == 0)
			continue;

		std::vector<std::wstring> aTokens;
		wcstok(sLine, aTokens, L" ");
		const wchar_t* pszToken = aTokens[0].c_str();

		if (wcscmp(pszToken, L"-name") == 0)
		{
			const wchar_t* pszMaterial = sLine.c_str()+6;
			std::wstring sName = pszMaterial;
			std::vector<std::wstring> aName;
			wcstok(sName, aName, L"\"");	// Strip out the quotation marks.

			wcscpy(pMaterial->m_szName, aName[0].c_str());
		}
		else if (wcscmp(pszToken, L"-dif") == 0)
		{
			float r, g, b;
			swscanf(sLine.c_str(), L"-dif %f %f %f", &r, &g, &b);
			pMaterial->m_vecDiffuse.x = r;
			pMaterial->m_vecDiffuse.y = g;
			pMaterial->m_vecDiffuse.z = b;
		}
		else if (wcscmp(pszToken, L"-amb") == 0)
		{
			float r, g, b;
			swscanf(sLine.c_str(), L"-amb %f %f %f", &r, &g, &b);
			pMaterial->m_vecAmbient.x = r;
			pMaterial->m_vecAmbient.y = g;
			pMaterial->m_vecAmbient.z = b;
		}
		else if (wcscmp(pszToken, L"-spec") == 0)
		{
			float r, g, b;
			swscanf(sLine.c_str(), L"-spec %f %f %f", &r, &g, &b);
			pMaterial->m_vecSpecular.x = r;
			pMaterial->m_vecSpecular.y = g;
			pMaterial->m_vecSpecular.z = b;
		}
		else if (wcscmp(pszToken, L"-emis") == 0)
		{
			float r, g, b;
			swscanf(sLine.c_str(), L"-emis %f %f %f", &r, &g, &b);
			pMaterial->m_vecEmissive.x = r;
			pMaterial->m_vecEmissive.y = g;
			pMaterial->m_vecEmissive.z = b;
		}
		else if (wcscmp(pszToken, L"-shin") == 0)
		{
			float flShininess;
			swscanf(sLine.c_str(), L"-shin %f", &flShininess);
			pMaterial->m_flShininess = flShininess;
		}
		else if (wcscmp(pszToken, L"-tex") == 0)
		{
			const wchar_t* pszTexture = sLine.c_str()+5;

			std::wstring sName = pszTexture;
			std::vector<std::wstring> aName;
			wcstok(sName, aName, L"\"");	// Strip out the quotation marks.

			wchar_t szDirectory[1024];
			wcscpy(szDirectory, pszFilename);
			GetDirectory(szDirectory);
			wchar_t szTexture[1024];
			swprintf(szTexture, L"%s/%s", szDirectory, aName[0].c_str());

			wcscpy(pMaterial->m_szTexture, szTexture);
		}
		else if (wcscmp(pszToken, L"-endMat") == 0)
		{
			return;
		}
	}
}

void CModelConverter::ReadSIAShape(std::wifstream& infile, bool bCare)
{
	size_t iCurrentMaterial = ~0;

	CConversionMesh* pMesh = NULL;
	size_t iAddV = 0;
	size_t iAddE = 0;
	size_t iAddUV = 0;
	size_t iAddN = 0;

	std::wstring sLine;
	while (infile.good())
	{
		std::getline(infile, sLine);

		sLine = StripWhitespace(sLine);

		if (sLine.length() == 0)
			continue;

		std::vector<std::wstring> aTokens;
		wcstok(sLine, aTokens, L" ");
		const wchar_t* pszToken = aTokens[0].c_str();

		if (!bCare)
		{
			if (wcscmp(pszToken, L"-endShape") == 0)
				return;
			else
				continue;
		}

		if (wcscmp(pszToken, L"-snam") == 0)
		{
			// We name our mesh.
			std::wstring sName = sLine.c_str()+6;
			std::vector<std::wstring> aName;
			wcstok(sName, aName, L"\"");	// Strip out the quotation marks.

			if (bCare)
			{
				size_t iMesh = m_pScene->FindMesh(aName[0].c_str());
				if (iMesh == (size_t)~0)
				{
					iMesh = m_pScene->AddMesh(aName[0].c_str());
					pMesh = m_pScene->GetMesh(iMesh);
					pMesh->AddBone(aName[0].c_str());
				}
				else
				{
					pMesh = m_pScene->GetMesh(iMesh);
					iAddV = pMesh->GetNumVertices();
					iAddE = pMesh->GetNumEdges();
					iAddUV = pMesh->GetNumUVs();
					iAddN = pMesh->GetNumNormals();
				}
			}
		}
		else if (wcscmp(pszToken, L"-vert") == 0)
		{
			// A vertex.
			float x, y, z;
			swscanf(sLine.c_str(), L"-vert %f %f %f", &x, &y, &z);
			pMesh->AddVertex(x, y, z);
		}
		else if (wcscmp(pszToken, L"-edge") == 0)
		{
			// An edge. We only need them so we can tell where the creases are, so we can calculate normals properly.
			int v1, v2;
			swscanf(sLine.c_str(), L"-edge %d %d", &v1, &v2);
			pMesh->AddEdge(v1+iAddV, v2+iAddV);
		}
		else if (wcscmp(pszToken, L"-creas") == 0)
		{
			// An edge. We only need them so we can tell where the creases are, so we can calculate normals properly.
			std::wstring sCreases = sLine.c_str()+7;
			std::vector<std::wstring> aCreases;
			wcstok(sCreases, aCreases, L" ");

			for (size_t i = 1; i < aCreases.size(); i++)
				pMesh->GetEdge(_wtoi(aCreases[i].c_str())+iAddE)->m_bCreased = true;
		}
		else if (wcscmp(pszToken, L"-setmat") == 0)
		{
			const wchar_t* pszMaterial = sLine.c_str()+8;
			iCurrentMaterial = _wtoi(pszMaterial);
		}
		else if (wcscmp(pszToken, L"-face") == 0)
		{
			// A face.
			size_t iFace = pMesh->AddFace(iCurrentMaterial);

			std::wstring sFaces = sLine.c_str()+8;
			std::vector<std::wstring> aFaces;
			wcstok(sFaces, aFaces, L" ");

			for (size_t i = 0; i < aFaces.size(); i += 4)
			{
				size_t iVertex = _wtoi(aFaces[i].c_str())+iAddV;
				size_t iEdge = _wtoi(aFaces[i+1].c_str())+iAddE;

				float flU = (float)_wtof(aFaces[i+2].c_str());
				float flV = (float)_wtof(aFaces[i+3].c_str());
				size_t iUV = pMesh->AddUV(flU, flV);
				size_t iNormal = pMesh->AddNormal(0, 0, 1);	// For now!

				pMesh->AddVertexToFace(iFace, iVertex, iUV, iNormal);
				pMesh->AddEdgeToFace(iFace, iEdge);
			}
		}
		else if (wcscmp(pszToken, L"-axis") == 0)
		{
			// Object's center point. There is rotation information included in this node, but we don't use it at the moment.
			float x, y, z;
			swscanf(sLine.c_str(), L"-axis %f %f %f", &x, &y, &z);
			pMesh->m_vecOrigin = Vector(x, y, z);
		}
		else if (wcscmp(pszToken, L"-endShape") == 0)
		{
			break;
		}
	}
}

void CModelConverter::WriteSMDs(const wchar_t* pszFilename)
{
	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
		WriteSMD(i, pszFilename);
}

void CModelConverter::WriteSMD(size_t iMesh, const wchar_t* pszFilename)
{
	CConversionMesh* pMesh = m_pScene->GetMesh(iMesh);

	wchar_t szFile[1024];
	szFile[0] = '\0';
	if (pszFilename)
	{
		wcscpy(szFile, pszFilename);
		wcscpy(szFile, L"_");
	}
	wcscpy(szFile, pMesh->GetBoneName(0));

	wchar_t* pszFile = MakeFilename(szFile);

	std::wstring sSMDFile(pszFile);
	sSMDFile.append(L".smd");

	FILE* fp = _wfopen(sSMDFile.c_str(), L"w");

	// SMD file format: http://developer.valvesoftware.com/wiki/SMD

	// Header section
	fwprintf(fp, L"version 1\n\n");

	// Nodes section
	fwprintf(fp, L"nodes\n");
	// Only bothering with one node, we're only doing static props with this code for now.
	fwprintf(fp, L"0 \"%s\" -1\n", pMesh->GetBoneName(0));
	fwprintf(fp, L"end\n\n");

	// Skeleton section
	fwprintf(fp, L"skeleton\n");
	fwprintf(fp, L"time 0\n");
	fwprintf(fp, L"0 0.000000 0.000000 0.000000 1.570796 0.000000 0.0000001\n");
	fwprintf(fp, L"end\n\n");
	
	fwprintf(fp, L"triangles\n");
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

			if (iMaterial == ((size_t)~0) || !m_pScene->GetMaterial(iMaterial))
			{
				printf("ERROR! Can't find a material for a triangle.\n");
				fwprintf(fp, L"error\n");
			}
			else
				fwprintf(fp, L"%s\n", m_pScene->GetMaterial(iMaterial)->GetName());

			// <int|Parent bone> <float|PosX PosY PosZ> <normal|NormX NormY NormZ> <normal|U V>
			// Studio coordinates are not the same as game coordinates. Studio (x, y, z) is game (x, -z, y) and vice versa.
			fwprintf(fp, L"0 \t %f %f %f \t %f %f %f \t %f %f\n", v1.x, -v1.z, v1.y, n1.x, -n1.z, n1.y, uv1.x, uv1.y);
			fwprintf(fp, L"0 \t %f %f %f \t %f %f %f \t %f %f\n", v2.x, -v2.z, v2.y, n2.x, -n2.z, n2.y, uv2.x, uv2.y);
			fwprintf(fp, L"0 \t %f %f %f \t %f %f %f \t %f %f\n", v3.x, -v3.z, v3.y, n3.x, -n3.z, n3.y, uv3.x, uv3.y);
		}
	}
	fwprintf(fp, L"end\n");

	fclose(fp);
}

// Takes a path + filename + extension and removes path and extension to return only the filename.
// It is destructive on the string that is passed into it.
wchar_t* CModelConverter::MakeFilename(wchar_t* pszPathFilename)
{
	wchar_t* pszFile = pszPathFilename;
	int iLastChar = -1;
	int i = -1;

	while (pszFile[++i])
		if (pszFile[i] == L'\\' || pszFile[i] == L'/')
			iLastChar = i;

	pszFile += iLastChar+1;

	i = -1;
	while (pszFile[++i])
		if (pszFile[i] == L'.')
			iLastChar = i;

	if (iLastChar >= 0)
		pszFile[iLastChar] = L'\0';

	return pszFile;
}

// Destructive
wchar_t* CModelConverter::GetDirectory(wchar_t* pszFilename)
{
	int iLastSlash = -1;
	int i = -1;

	while (pszFilename[++i])
		if (pszFilename[i] == L'\\' || pszFilename[i] == L'/')
			iLastSlash = i;

	if (iLastSlash >= 0)
		pszFilename[iLastSlash] = L'\0';

	return pszFilename;
}

bool CModelConverter::IsWhitespace(wchar_t cChar)
{
	return (cChar == L' ' || cChar == L'\t' || cChar == L'\r' || cChar == L'\n');
}

wchar_t* CModelConverter::StripWhitespace(wchar_t* pszLine)
{
	if (!pszLine)
		return NULL;

	wchar_t* pszSpace = pszLine;
	while (IsWhitespace(pszSpace[0]) && pszSpace[0] != L'\0')
		pszSpace++;

	int iEnd = ((int)wcslen(pszSpace))-1;
	while (iEnd >= 0 && IsWhitespace(pszSpace[iEnd]))
		iEnd--;

	if (iEnd >= -1)
	{
		pszSpace[iEnd+1] = L'\0';
	}

	return pszSpace;
}

std::wstring CModelConverter::StripWhitespace(std::wstring sLine)
{
	int i = 0;
	while (IsWhitespace(sLine[i]) && sLine[i] != L'\0')
		i++;

	int iEnd = ((int)sLine.length())-1;
	while (iEnd >= 0 && IsWhitespace(sLine[iEnd]))
		iEnd--;

	if (iEnd >= -1)
		sLine[iEnd+1] = L'\0';

	return sLine.substr(i);
}
