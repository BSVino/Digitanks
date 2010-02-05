#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../modelconverter.h"
#include "strutils.h"

// Silo ascii
void CModelConverter::ReadSIA(const wchar_t* pszFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

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

	m_pScene->SetWorkListener(m_pWorkListener);

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
	{
		m_pScene->GetMesh(i)->CalculateEdgeData();
		m_pScene->GetMesh(i)->CalculateVertexNormals();
		m_pScene->GetMesh(i)->TranslateOrigin();
	}

	m_pScene->CalculateExtends();

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

void CModelConverter::ReadSIAMat(std::wifstream& infile, const wchar_t* pszFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Reading materials", 0);

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

	std::wstring sLastTask;

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
			if (m_pWorkListener)
			{
				if (wcscmp(sLastTask.c_str(), pszToken) == 0)
					m_pWorkListener->WorkProgress(0);
				else
					m_pWorkListener->SetAction(L"Reading vertex data", 0);
			}
			sLastTask = std::wstring(pszToken);

			// A vertex.
			float x, y, z;
			swscanf(sLine.c_str(), L"-vert %f %f %f", &x, &y, &z);
			pMesh->AddVertex(x, y, z);
		}
		else if (wcscmp(pszToken, L"-edge") == 0)
		{
			if (m_pWorkListener)
			{
				if (wcscmp(sLastTask.c_str(), pszToken) == 0)
					m_pWorkListener->WorkProgress(0);
				else
					m_pWorkListener->SetAction(L"Reading edge data", 0);
			}
			sLastTask = std::wstring(pszToken);

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
			if (m_pWorkListener)
			{
				if (wcscmp(sLastTask.c_str(), pszToken) == 0)
					m_pWorkListener->WorkProgress(0);
				else
					m_pWorkListener->SetAction(L"Reading polygon data", 0);
			}
			sLastTask = std::wstring(pszToken);

			// A face.
			size_t iFace = pMesh->AddFace(iCurrentMaterial);

			// HACK! CModelWindow::SetAction() ends up calling wcstok so we reset it here.
			wcstok(sLine, aTokens, L" ");

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
