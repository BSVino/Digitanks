#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../modelconverter.h"
#include "strutils.h"

void CModelConverter::ReadOBJ(const wchar_t* pszFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	FILE* fp = _wfopen(pszFilename, L"r");

	if (!fp)
	{
		printf("No input file. Sorry!\n");
		return;
	}

	CConversionSceneNode* pScene = m_pScene->GetScene(m_pScene->AddScene(GetFilename(pszFilename).append(L".obj")));

	CConversionMesh* pMesh = m_pScene->GetMesh(m_pScene->AddMesh(GetFilename(pszFilename)));
	// Make sure it exists.
	CConversionSceneNode* pMeshNode = m_pScene->GetDefaultSceneMeshInstance(pScene, pMesh);

	size_t iCurrentMaterial = ~0;
	size_t iSmoothingGroup = ~0;

	bool bSmoothingGroups = false;

	std::wstring sLastTask;

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
			std::wstring sDirectory = GetDirectory(std::wstring(pszFilename));
			wchar_t szMaterial[1024];
			swprintf(szMaterial, L"%s/%s", sDirectory.c_str(), pszLine + 7);
			ReadMTL(szMaterial);
		}
		else if (wcscmp(pszToken, L"o") == 0)
		{
			// Dunno what this does.
		}
		else if (wcscmp(pszToken, L"v") == 0)
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
			swscanf(pszLine, L"v %f %f %f", &x, &y, &z);
			pMesh->AddVertex(x, y, z);
		}
		else if (wcscmp(pszToken, L"vn") == 0)
		{
			if (m_pWorkListener)
			{
				if (wcscmp(sLastTask.c_str(), pszToken) == 0)
					m_pWorkListener->WorkProgress(0);
				else
					m_pWorkListener->SetAction(L"Reading vertex normal data", 0);
			}
			sLastTask = std::wstring(pszToken);

			// A vertex normal.
			float x, y, z;
			swscanf(pszLine, L"vn %f %f %f", &x, &y, &z);
			pMesh->AddNormal(x, y, z);
		}
		else if (wcscmp(pszToken, L"vt") == 0)
		{
			if (m_pWorkListener)
			{
				if (wcscmp(sLastTask.c_str(), pszToken) == 0)
					m_pWorkListener->WorkProgress(0);
				else
					m_pWorkListener->SetAction(L"Reading texture coordinate data", 0);
			}
			sLastTask = std::wstring(pszToken);

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
			std::wstring sMaterial = std::wstring(pszMaterial);
			size_t iMaterial = pMesh->FindMaterialStub(sMaterial);
			if (iMaterial == ((size_t)~0))
			{
				size_t iSceneMaterial = m_pScene->FindMaterial(sMaterial);
				if (iSceneMaterial == ((size_t)~0))
					iCurrentMaterial = m_pScene->AddDefaultSceneMaterial(pScene, pMesh, sMaterial);
				else
				{
					size_t iMaterialStub = pMesh->AddMaterialStub(sMaterial);
					m_pScene->GetDefaultSceneMeshInstance(pScene, pMesh)->GetMeshInstance(0)->AddMappedMaterial(iMaterialStub, iSceneMaterial);
					iCurrentMaterial = iMaterialStub;
				}
			}
			else
				iCurrentMaterial = iMaterial;
		}
		else if (wcscmp(pszToken, L"s") == 0)
		{
			if (wcsncmp(pszLine, L"s off", 5) == 0)
			{
				iSmoothingGroup = ~0;
			}
			else
			{
				bSmoothingGroups = true;
				size_t s;
				swscanf(pszLine, L"s %d", &s);
				iSmoothingGroup = s;
			}
		}
		else if (wcscmp(pszToken, L"f") == 0)
		{
			if (m_pWorkListener)
			{
				if (wcscmp(sLastTask.c_str(), pszToken) == 0)
					m_pWorkListener->WorkProgress(0);
				else
					m_pWorkListener->SetAction(L"Reading polygon data", 0);
			}
			sLastTask = std::wstring(pszToken);

			if (iCurrentMaterial == ~0)
				iCurrentMaterial = m_pScene->AddDefaultSceneMaterial(pScene, pMesh, pMesh->GetName());

			// A face.
			size_t iFace = pMesh->AddFace(iCurrentMaterial);

			pMesh->GetFace(iFace)->m_iSmoothingGroup = iSmoothingGroup;

			// HACK! CModelWindow::SetAction() ends up calling wcstok so we reset it here.
			wcscpy(szToken, pszLine);
			pszToken = wcstok(szToken, L" ");

			while (pszToken = wcstok(NULL, L" "))
			{
				// We don't use size_t because SOME EXPORTS put out negative numbers.
				long v, vt, vn;
				std::vector<std::wstring> asTokens;
				explode(std::wstring(pszToken), asTokens, L"/");

				if (asTokens.size() > 0)
				{
					v = _wtol(asTokens[0].c_str());
					if (v < 0)
						v = (long)pMesh->GetNumVertices()+v+1;
					assert ( v >= 1 && v < (long)pMesh->GetNumVertices()+1 );
				}

				if (asTokens.size() > 1 && pMesh->GetNumUVs())
				{
					vt = _wtol(asTokens[1].c_str());
					if (vt < 0)
						vt = (long)pMesh->GetNumUVs()+vt+1;
					assert ( vt >= 1 && vt < (long)pMesh->GetNumUVs()+1 );
				}

				if (asTokens.size() > 2 && pMesh->GetNumNormals())
				{
					vn = _wtol(asTokens[2].c_str());
					if (vn < 0)
						vn = (long)pMesh->GetNumNormals()+vn+1;
					assert ( vn >= 1 && vn < (long)pMesh->GetNumNormals()+1 );
				}

				// OBJ uses 1-based indexing.
				// Convert to 0-based indexing.
				v--;
				vt--;
				vn--;

				if (!pMesh->GetNumUVs())
					vt = ~0;
				if (asTokens.size() == 2 || !pMesh->GetNumNormals())
					vn = ~0;

				pMesh->AddVertexToFace(iFace, v, vt, vn);
			}
		}
	}

	fclose(fp);

	m_pScene->SetWorkListener(m_pWorkListener);

	m_pScene->CalculateExtends();

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
	{
		m_pScene->GetMesh(i)->CalculateEdgeData();

		if (bSmoothingGroups || m_pScene->GetMesh(i)->GetNumNormals() == 0)
			m_pScene->GetMesh(i)->CalculateVertexNormals();

		m_pScene->GetMesh(i)->CalculateVertexTangents();
	}

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

void CModelConverter::ReadMTL(const wchar_t* pszFilename)
{
	FILE* fp = _wfopen(pszFilename, L"r");

	if (!fp)
		return;

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Reading materials", 0);

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
			pMaterial->m_flShininess = (float)_wtof(pszToken)*128/1000;
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

				pMaterial->m_sDiffuseTexture = std::wstring(pszToken);
			}
			else
			{
				wchar_t szDirectory[1024];
				wcscpy(szDirectory, pszFilename);
				std::wstring sDirectory = GetDirectory(szDirectory);
				wchar_t szTexture[1024];
				swprintf(szTexture, L"%s/%s", sDirectory.c_str(), pszToken);

				pMaterial->m_sDiffuseTexture = std::wstring(szTexture);
			}
		}
	}

	fclose(fp);
}
