#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../modelconverter.h"
#include "strutils.h"

#include <string>

// Silo ascii
void CModelConverter::ReadSIA(const eastl::string16& sFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	CConversionSceneNode* pScene = m_pScene->GetScene(m_pScene->AddScene(GetFilename(sFilename).append(L".sia")));

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Reading file into memory...", 0);

	FILE* fp = _wfopen(sFilename.c_str(), L"r");

	if (!fp)
	{
		printf("No input file. Sorry!\n");
		return;
	}

	fseek(fp, 0L, SEEK_END);
	long iOBJSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	// Make sure we allocate more than we need just in case.
	char16_t* pszEntireFile = (char16_t*)malloc((iOBJSize+1) * (sizeof(char16_t)+1));
	char16_t* pszCurrent = pszEntireFile;

	// Read the entire file into an array first for faster processing.
	const size_t iChars = 1024;
	char16_t szLine[iChars];
	const char16_t* pszLine;
	while (pszLine = fgetws(szLine, iChars, fp))
	{
		wcscpy(pszCurrent, pszLine);
		size_t iLength = wcslen(pszLine);

		if (pszCurrent[iLength-1] == L'\n')
		{
			pszCurrent[iLength-1] = L'\0';
			iLength--;
		}

		pszCurrent += iLength;
		pszCurrent++;

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(0);
	}

	pszCurrent[0] = L'\0';

	fclose(fp);

	pszLine = pszEntireFile;
	const char16_t* pszNextLine = NULL;
	while (pszLine < pszCurrent)
	{
		if (pszNextLine)
			pszLine = pszNextLine;

		pszNextLine = pszLine + wcslen(pszLine) + 1;

		// This code used to call StripWhitespace() but that's too slow for very large files w/ millions of lines.
		// Instead we'll just cut the whitespace off the front and deal with whitespace on the end when we come to it.
		while (*pszLine && IsWhitespace(*pszLine))
			pszLine++;

		if (wcslen(pszLine) == 0)
			continue;

		eastl::vector<eastl::string16> aTokens;
		wcstok(pszLine, aTokens, L" ");
		const wchar_t* pszToken = aTokens[0].c_str();

		if (wcscmp(pszToken, L"-Version") == 0)
		{
			// Warning if version is later than 1.0, we may not support it
			int iMajor, iMinor;
			swscanf(pszLine, L"-Version %d.%d", &iMajor, &iMinor);
			if (iMajor != 1 && iMinor != 0)
				printf("WARNING: I was programmed for version 1.0, this file is version %d.%d, so this might not work exactly right!\n", iMajor, iMinor);
		}
		else if (wcscmp(pszToken, L"-Mat") == 0)
		{
			pszNextLine = ReadSIAMat(pszNextLine, pszCurrent, pScene, sFilename);
		}
		else if (wcscmp(pszToken, L"-Shape") == 0)
		{
			pszNextLine = ReadSIAShape(pszNextLine, pszCurrent, pScene);
		}
		else if (wcscmp(pszToken, L"-Texshape") == 0)
		{
			// This is the 3d UV space of the object, but we only care about its 2d UV space which is contained in rhw -Shape section, so meh.
			pszNextLine = ReadSIAShape(pszNextLine, pszCurrent, pScene, false);
		}
	}

	free(pszEntireFile);

	m_pScene->SetWorkListener(m_pWorkListener);

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
	{
		m_pScene->GetMesh(i)->CalculateEdgeData();
		m_pScene->GetMesh(i)->CalculateVertexNormals();
		m_pScene->GetMesh(i)->CalculateVertexTangents();
	}

	m_pScene->CalculateExtends();

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

const char16_t* CModelConverter::ReadSIAMat(const char16_t* pszLine, const char16_t* pszEnd, CConversionSceneNode* pScene, const eastl::string16& sFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Reading materials", 0);

	size_t iCurrentMaterial = m_pScene->AddMaterial(L"");
	CConversionMaterial* pMaterial = m_pScene->GetMaterial(iCurrentMaterial);

	const char16_t* pszNextLine = NULL;
	while (pszLine < pszEnd)
	{
		if (pszNextLine)
			pszLine = pszNextLine;

		pszNextLine = pszLine + wcslen(pszLine) + 1;

		// This code used to call StripWhitespace() but that's too slow for very large files w/ millions of lines.
		// Instead we'll just cut the whitespace off the front and deal with whitespace on the end when we come to it.
		while (*pszLine && IsWhitespace(*pszLine))
			pszLine++;

		if (wcslen(pszLine) == 0)
			continue;

		eastl::vector<eastl::string16> aTokens;
		wcstok(pszLine, aTokens, L" ");
		const wchar_t* pszToken = aTokens[0].c_str();

		if (wcscmp(pszToken, L"-name") == 0)
		{
			eastl::string16 sName = pszLine+6;
			eastl::vector<eastl::string16> aName;
			wcstok(sName, aName, L"\"");	// Strip out the quotation marks.

			pMaterial->m_sName = aName[0];
		}
		else if (wcscmp(pszToken, L"-dif") == 0)
		{
			float r, g, b;
			swscanf(pszLine, L"-dif %f %f %f", &r, &g, &b);
			pMaterial->m_vecDiffuse.x = r;
			pMaterial->m_vecDiffuse.y = g;
			pMaterial->m_vecDiffuse.z = b;
		}
		else if (wcscmp(pszToken, L"-amb") == 0)
		{
			float r, g, b;
			swscanf(pszLine, L"-amb %f %f %f", &r, &g, &b);
			pMaterial->m_vecAmbient.x = r;
			pMaterial->m_vecAmbient.y = g;
			pMaterial->m_vecAmbient.z = b;
		}
		else if (wcscmp(pszToken, L"-spec") == 0)
		{
			float r, g, b;
			swscanf(pszLine, L"-spec %f %f %f", &r, &g, &b);
			pMaterial->m_vecSpecular.x = r;
			pMaterial->m_vecSpecular.y = g;
			pMaterial->m_vecSpecular.z = b;
		}
		else if (wcscmp(pszToken, L"-emis") == 0)
		{
			float r, g, b;
			swscanf(pszLine, L"-emis %f %f %f", &r, &g, &b);
			pMaterial->m_vecEmissive.x = r;
			pMaterial->m_vecEmissive.y = g;
			pMaterial->m_vecEmissive.z = b;
		}
		else if (wcscmp(pszToken, L"-shin") == 0)
		{
			float flShininess;
			swscanf(pszLine, L"-shin %f", &flShininess);
			pMaterial->m_flShininess = flShininess;
		}
		else if (wcscmp(pszToken, L"-tex") == 0)
		{
			const wchar_t* pszTexture = pszLine+5;

			eastl::string16 sName = pszTexture;
			eastl::vector<eastl::string16> aName;
			wcstok(sName, aName, L"\"");	// Strip out the quotation marks.

			eastl::string16 sDirectory = GetDirectory(sFilename);
			wchar_t szTexture[1024];
			swprintf(szTexture, L"%s/%s", sDirectory.c_str(), aName[0].c_str());

			pMaterial->m_sDiffuseTexture = szTexture;
		}
		else if (wcscmp(pszToken, L"-endMat") == 0)
		{
			return pszNextLine;
		}
	}

	return pszNextLine;
}

const char16_t* CModelConverter::ReadSIAShape(const char16_t* pszLine, const char16_t* pszEnd, CConversionSceneNode* pScene, bool bCare)
{
	size_t iCurrentMaterial = ~0;

	CConversionMesh* pMesh = NULL;
	CConversionSceneNode* pMeshNode = NULL;
	size_t iAddV = 0;
	size_t iAddE = 0;
	size_t iAddUV = 0;
	size_t iAddN = 0;

	eastl::string16 sLastTask;

	const char16_t* pszNextLine = NULL;
	while (pszLine < pszEnd)
	{
		if (pszNextLine)
			pszLine = pszNextLine;

		pszNextLine = pszLine + wcslen(pszLine) + 1;

		// This code used to call StripWhitespace() but that's too slow for very large files w/ millions of lines.
		// Instead we'll just cut the whitespace off the front and deal with whitespace on the end when we come to it.
		while (*pszLine && IsWhitespace(*pszLine))
			pszLine++;

		if (wcslen(pszLine) == 0)
			continue;

		const char16_t* pszToken = pszLine;

		while (*pszToken && *pszToken != L' ')
			pszToken++;

		char16_t szToken[1024];
		wcsncpy(szToken, pszLine, pszToken-pszLine);
		szToken[pszToken-pszLine] = L'\0';
		pszToken = szToken;

		if (!bCare)
		{
			if (wcscmp(pszToken, L"-endShape") == 0)
				return pszNextLine;
			else
				continue;
		}

		if (wcscmp(pszToken, L"-snam") == 0)
		{
			// We name our mesh.
			eastl::string16 sName =pszLine+6;
			eastl::vector<eastl::string16> aName;
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
				// Make sure it exists.
				pMeshNode = m_pScene->GetDefaultSceneMeshInstance(pScene, pMesh);
			}
		}
		else if (wcscmp(pszToken, L"-vert") == 0)
		{
			if (m_pWorkListener)
			{
				if (wcscmp(sLastTask.c_str(), pszToken) == 0)
					m_pWorkListener->WorkProgress(0);
				else
				{
					m_pWorkListener->SetAction(L"Reading vertex data", 0);
					sLastTask = eastl::string16(pszToken);
				}
			}

			// A vertex.
			float v[3];
			// scanf is pretty slow even for such a short string due to lots of mallocs.
			const wchar_t* pszToken = pszLine+5;
			int iDimension = 0;
			while (*pszToken)
			{
				while (pszToken[0] == L' ')
					pszToken++;

				v[iDimension++] = (float)_wtof(pszToken);
				if (iDimension >= 3)
					break;

				while (pszToken[0] != L' ')
					pszToken++;
			}
			pMesh->AddVertex(v[0], v[1], v[2]);
		}
		else if (wcscmp(pszToken, L"-edge") == 0)
		{
			if (m_pWorkListener)
			{
				if (wcscmp(sLastTask.c_str(), pszToken) == 0)
					m_pWorkListener->WorkProgress(0);
				else
				{
					m_pWorkListener->SetAction(L"Reading edge data", 0);
					sLastTask = eastl::string16(pszToken);
				}
			}

			// An edge. We only need them so we can tell where the creases are, so we can calculate normals properly.
			int e[2];
			// scanf is pretty slow even for such a short string due to lots of mallocs.
			const wchar_t* pszToken = pszLine+5;
			int iDimension = 0;
			while (*pszToken)
			{
				while (pszToken[0] == L' ')
					pszToken++;

				e[iDimension++] = (int)_wtoi(pszToken);
				if (iDimension >= 2)
					break;

				while (pszToken[0] != L' ')
					pszToken++;
			}
			pMesh->AddEdge(e[0]+iAddV, e[1]+iAddV);
		}
		else if (wcscmp(pszToken, L"-creas") == 0)
		{
			// An edge. We only need them so we can tell where the creases are, so we can calculate normals properly.
			eastl::string16 sCreases = pszLine+7;
			eastl::vector<eastl::string16> aCreases;
			wcstok(sCreases, aCreases, L" ");

			for (size_t i = 1; i < aCreases.size(); i++)
				pMesh->GetEdge(_wtoi(aCreases[i].c_str())+iAddE)->m_bCreased = true;
		}
		else if (wcscmp(pszToken, L"-setmat") == 0)
		{
			const wchar_t* pszMaterial = pszLine+8;
			size_t iNewMaterial = _wtoi(pszMaterial);

			if (iNewMaterial == (size_t)(-1))
				iCurrentMaterial = ~0;
			else
			{
				CConversionMaterial* pMaterial = m_pScene->GetMaterial(iNewMaterial);
				if (pMaterial)
				{
					iCurrentMaterial = pMesh->FindMaterialStub(pMaterial->GetName());
					if (iCurrentMaterial == (size_t)~0)
					{
						size_t iMaterialStub = pMesh->AddMaterialStub(pMaterial->GetName());
						m_pScene->GetDefaultSceneMeshInstance(pScene, pMesh)->GetMeshInstance(0)->AddMappedMaterial(iMaterialStub, iNewMaterial);
						iCurrentMaterial = iMaterialStub;
					}
				}
				else
					iCurrentMaterial = m_pScene->AddDefaultSceneMaterial(pScene, pMesh, pMesh->GetName());
			}
		}
		else if (wcscmp(pszToken, L"-face") == 0)
		{
			if (m_pWorkListener)
			{
				if (wcscmp(sLastTask.c_str(), pszToken) == 0)
					m_pWorkListener->WorkProgress(0);
				else
				{
					m_pWorkListener->SetAction(L"Reading polygon data", 0);
					sLastTask = eastl::string16(pszToken);
				}
			}

			// A face.
			size_t iFace = pMesh->AddFace(iCurrentMaterial);

			if (iFace == 10000)
				pMeshNode->GetMeshInstance(0)->SetVisible(false);

			// scanf is pretty slow even for such a short string due to lots of mallocs.
			const wchar_t* pszToken = pszLine+6;

			size_t iVerts = _wtoi(pszToken);

			while (pszToken[0] != L' ')
				pszToken++;

			size_t iProcessed = 0;
			while (iProcessed++ < iVerts)
			{
				size_t iVertex = _wtoi(++pszToken)+iAddV;

				while (pszToken[0] != L' ')
					pszToken++;

				size_t iEdge = _wtoi(++pszToken)+iAddE;

				while (pszToken[0] != L' ')
					pszToken++;

				float flU = (float)_wtof(++pszToken);

				while (pszToken[0] != L' ')
					pszToken++;

				float flV = (float)_wtof(++pszToken);

				size_t iUV = pMesh->AddUV(flU, flV);

				size_t iNormal = pMesh->AddNormal(0, 0, 1);	// For now!

				pMesh->AddVertexToFace(iFace, iVertex, iUV, iNormal);
				pMesh->AddEdgeToFace(iFace, iEdge);

				while (pszToken[0] != L'\0' && pszToken[0] != L' ')
					pszToken++;
			}
		}
		else if (wcscmp(pszToken, L"-axis") == 0)
		{
			// This is the manipulator position and angles. The code below is untested and probably has the elements in the wrong
			// order. We don't support writing yet so no need to load it so I'm not bothering with it now.
		/*	Matrix4x4& m = pMeshNode->m_mManipulator;
			swscanf(sLine.c_str(), L"-axis %f %f %f %f %f %f %f %f %f",
				&m.m[0][3], &m.m[1][3], &m.m[2][3],
				&m.m[0][0], &m.m[0][1], &m.m[0][2],	// ?
				&m.m[1][0], &m.m[1][1], &m.m[1][2], // ?
				&m.m[2][0], &m.m[2][1], &m.m[2][2]  // ?
				);*/
		}
		else if (wcscmp(pszToken, L"-endShape") == 0)
		{
			break;
		}
	}

	return pszNextLine;
}
