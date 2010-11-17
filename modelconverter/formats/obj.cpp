#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../modelconverter.h"
#include "strutils.h"

void CModelConverter::ReadOBJ(const eastl::string16& sFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	FILE* fp = _wfopen(sFilename.c_str(), L"r");

	if (!fp)
	{
		printf("No input file. Sorry!\n");
		return;
	}

	CConversionSceneNode* pScene = m_pScene->GetScene(m_pScene->AddScene(GetFilename(sFilename).append(L".obj")));

	CConversionMesh* pMesh = m_pScene->GetMesh(m_pScene->AddMesh(GetFilename(sFilename)));
	// Make sure it exists.
	CConversionSceneNode* pMeshNode = m_pScene->GetDefaultSceneMeshInstance(pScene, pMesh);

	size_t iCurrentMaterial = ~0;
	size_t iSmoothingGroup = ~0;

	bool bSmoothingGroups = false;

	eastl::string16 sLastTask;

	int iTotalVertices = 0;
	int iTotalFaces = 0;
	int iVerticesComplete = 0;
	int iFacesComplete = 0;

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Reading file into memory...", 0);

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

		if (pszLine[0] == '#')
		{
			// ZBrush is kind enough to notate exactly how many vertices and faces we have in the comments at the top of the file.
			if (wcsncmp(pszLine, L"#Vertex Count", 13) == 0)
			{
				iTotalVertices = _wtoi(pszLine+13);
				pMesh->SetTotalVertices(iTotalVertices);
			}

			if (wcsncmp(pszLine, L"#Face Count", 11) == 0)
			{
				iTotalFaces = _wtoi(pszLine+11);
				pMesh->SetTotalFaces(iTotalFaces);

				// Don't kill the video card while we're loading the faces.
				if (iTotalFaces > 10000)
					pMeshNode->GetMeshInstance(0)->SetVisible(false);
			}

			continue;
		}

		wchar_t szToken[1024];
		wcscpy(szToken, pszLine);
		wchar_t* pszToken = NULL;
		pszToken = wcstok(szToken, L" ");

		if (wcscmp(pszToken, L"mtllib") == 0)
		{
			eastl::string16 sDirectory = GetDirectory(sFilename);
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
					m_pWorkListener->WorkProgress(iVerticesComplete++);
				else
				{
					m_pWorkListener->SetAction(L"Reading vertex data", iTotalVertices);
					sLastTask = eastl::string16(pszToken);
				}
			}

			// A vertex.
			float v[3];
			// scanf is pretty slow even for such a short string due to lots of mallocs.
			const wchar_t* pszToken = pszLine+1;
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
		else if (wcscmp(pszToken, L"vn") == 0)
		{
			if (m_pWorkListener)
			{
				if (wcscmp(sLastTask.c_str(), pszToken) == 0)
					m_pWorkListener->WorkProgress(0);
				else
					m_pWorkListener->SetAction(L"Reading vertex normal data", 0);
			}
			sLastTask = eastl::string16(pszToken);

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
			sLastTask = eastl::string16(pszToken);

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
			eastl::string16 sMaterial = eastl::string16(pszLine+7);
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
					m_pWorkListener->WorkProgress(iFacesComplete++);
				else
				{
					m_pWorkListener->SetAction(L"Reading polygon data", iTotalFaces);
					sLastTask = eastl::string16(pszToken);
				}
			}

			if (iCurrentMaterial == ~0)
				iCurrentMaterial = m_pScene->AddDefaultSceneMaterial(pScene, pMesh, pMesh->GetName());

			// A face.
			size_t iFace = pMesh->AddFace(iCurrentMaterial);

			// If we get to 10k faces force the mesh off so it doesn't kill the video card.
			if (iFace == 10000)
				pMeshNode->GetMeshInstance(0)->SetVisible(false);

			pMesh->GetFace(iFace)->m_iSmoothingGroup = iSmoothingGroup;

			// HACK! CModelWindow::SetAction() ends up calling wcstok so we reset it here.
			wcscpy(szToken, pszLine);
			pszToken = wcstok(szToken, L" ");

			while (pszToken = wcstok(NULL, L" "))
			{
				// We don't use size_t because SOME EXPORTS put out negative numbers.
				long f[3];
				bool bValues[3];
				bValues[0] = false;
				bValues[1] = false;
				bValues[2] = false;

				// scanf is pretty slow even for such a short string due to lots of mallocs.
				const wchar_t* pszValues = pszToken;
				int iValue = 0;
				do
				{
					if (!pszValues)
						break;

					if (!bValues[0] || pszValues[0] == L'/')
					{
						if (pszValues[0] == L'/')
							pszValues++;

						bValues[iValue] = true;
						f[iValue++] = (long)_wtoi(pszValues);
						if (iValue >= 3)
							break;
					}

					// Don't advance if we're on a slash, because that means empty slashes. ie, 11//12 <-- the 12 would get skipped.
					if (pszValues[0] != L'/')
						pszValues++;
				}
				while (*pszValues);

				if (bValues[0])
				{
					if (f[0] < 0)
						f[0] = (long)pMesh->GetNumVertices()+f[0]+1;
					assert ( f[0] >= 1 && f[0] < (long)pMesh->GetNumVertices()+1 );
				}

				if (bValues[1] && pMesh->GetNumUVs())
				{
					if (f[1] < 0)
						f[1] = (long)pMesh->GetNumUVs()+f[1]+1;
					assert ( f[1] >= 1 && f[1] < (long)pMesh->GetNumUVs()+1 );
				}

				if (bValues[2] && pMesh->GetNumNormals())
				{
					if (f[2] < 0)
						f[2] = (long)pMesh->GetNumNormals()+f[2]+1;
					assert ( f[2] >= 1 && f[2] < (long)pMesh->GetNumNormals()+1 );
				}

				// OBJ uses 1-based indexing.
				// Convert to 0-based indexing.
				f[0]--;
				f[1]--;
				f[2]--;

				if (!pMesh->GetNumUVs())
					f[1] = ~0;
				if (bValues[2] == false || !pMesh->GetNumNormals())
					f[2] = ~0;

				pMesh->AddVertexToFace(iFace, f[0], f[1], f[2]);
			}
		}
	}

	free(pszEntireFile);

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

void CModelConverter::ReadMTL(const eastl::string16& sFilename)
{
	FILE* fp = _wfopen(sFilename.c_str(), L"r");

	if (!fp)
		return;

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Reading materials", 0);

	size_t iCurrentMaterial = ~0;

	const size_t iCount = 1024;
	wchar_t szLine[iCount];
	wchar_t* pszLine = NULL;
	eastl::string16 sLine;
	while (pszLine = fgetws(szLine, iCount, fp))
	{
		sLine = pszLine;
		sLine = StripWhitespace(sLine);

		if (sLine.length() == 0)
			continue;

		if (sLine[0] == '#')
			continue;

		wchar_t szToken[1024];
		wcscpy(szToken, sLine.c_str());
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
			swscanf(sLine.c_str(), L"Ka %f %f %f", &r, &g, &b);
			pMaterial->m_vecAmbient.x = r;
			pMaterial->m_vecAmbient.y = g;
			pMaterial->m_vecAmbient.z = b;
		}
		else if (wcscmp(pszToken, L"Kd") == 0)
		{
			float r, g, b;
			swscanf(sLine.c_str(), L"Kd %f %f %f", &r, &g, &b);
			pMaterial->m_vecDiffuse.x = r;
			pMaterial->m_vecDiffuse.y = g;
			pMaterial->m_vecDiffuse.z = b;
		}
		else if (wcscmp(pszToken, L"Ks") == 0)
		{
			float r, g, b;
			swscanf(sLine.c_str(), L"Ks %f %f %f", &r, &g, &b);
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

				pMaterial->m_sDiffuseTexture = eastl::string16(pszToken);
			}
			else
			{
				eastl::string16 sDirectory = GetDirectory(sFilename);
				wchar_t szTexture[1024];
				swprintf(szTexture, L"%s/%s", sDirectory.c_str(), pszToken);

				pMaterial->m_sDiffuseTexture = eastl::string16(szTexture);
			}
		}
	}

	fclose(fp);
}

void CModelConverter::SaveOBJ(const eastl::string16& sFilename)
{
	eastl::string16 sMaterialFileName = eastl::string16(GetDirectory(sFilename).c_str()) + L"/" + GetFilename(sFilename).c_str() + L".mtl";

	std::wofstream sMaterialFile(sMaterialFileName.c_str());
	if (!sMaterialFile.is_open())
		return;

	if (m_pWorkListener)
	{
		m_pWorkListener->BeginProgress();
		m_pWorkListener->SetAction(L"Writing materials file", 0);
	}

	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		CConversionMaterial* pMaterial = m_pScene->GetMaterial(i);
		sMaterialFile << "newmtl " << pMaterial->GetName().c_str() << std::endl;
		sMaterialFile << "Ka " << pMaterial->m_vecAmbient.x << L" " << pMaterial->m_vecAmbient.y << L" " << pMaterial->m_vecAmbient.z << std::endl;
		sMaterialFile << "Kd " << pMaterial->m_vecDiffuse.x << L" " << pMaterial->m_vecDiffuse.y << L" " << pMaterial->m_vecDiffuse.z << std::endl;
		sMaterialFile << "Ks " << pMaterial->m_vecSpecular.x << L" " << pMaterial->m_vecSpecular.y << L" " << pMaterial->m_vecSpecular.z << std::endl;
		sMaterialFile << "d " << pMaterial->m_flTransparency << std::endl;
		sMaterialFile << "Ns " << pMaterial->m_flShininess << std::endl;
		sMaterialFile << "illum " << pMaterial->m_eIllumType << std::endl;
		if (pMaterial->GetDiffuseTexture().length() > 0)
			sMaterialFile << "map_Kd " << pMaterial->GetDiffuseTexture().c_str() << std::endl;
		sMaterialFile << std::endl;
	}

	sMaterialFile.close();

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(i);

		eastl::string16 sNodeName = pMesh->GetName();

		eastl::string16 sOBJFilename = eastl::string16(GetDirectory(sFilename).c_str()) + L"/" + GetFilename(sNodeName).c_str() + L".obj";
		eastl::string16 sMTLFilename = eastl::string16(GetFilename(sFilename).c_str()) + L".mtl";

		if (m_pScene->GetNumMeshes() == 1)
			sOBJFilename = sFilename;

		std::wofstream sOBJFile(sOBJFilename.c_str());
		sOBJFile.precision(8);
		sOBJFile.setf(std::ios::fixed, std::ios::floatfield);

		sOBJFile << L"mtllib " << sMTLFilename.c_str() << std::endl;
		sOBJFile << std::endl;

		sOBJFile << L"o " << sNodeName.c_str() << std::endl;

		if (m_pWorkListener)
			m_pWorkListener->SetAction((eastl::string16(L"Writing ") + sNodeName + L" vertices...").c_str(), pMesh->GetNumVertices());

		for (size_t iVertices = 0; iVertices < pMesh->GetNumVertices(); iVertices++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iVertices);

			Vector vecVertex = pMesh->GetVertex(iVertices);
			sOBJFile << L"v " << vecVertex.x << L" " << vecVertex.y << L" " << vecVertex.z << std::endl;
		}

		if (m_pWorkListener)
			m_pWorkListener->SetAction((eastl::string16(L"Writing ") + sNodeName + L" normals...").c_str(), pMesh->GetNumNormals());

		for (size_t iNormals = 0; iNormals < pMesh->GetNumNormals(); iNormals++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iNormals);

			Vector vecNormal = pMesh->GetNormal(iNormals);
			sOBJFile << L"vn " << vecNormal.x << L" " << vecNormal.y << L" " << vecNormal.z << std::endl;
		}

		if (m_pWorkListener)
			m_pWorkListener->SetAction((eastl::string16(L"Writing ") + sNodeName + L" UVs...").c_str(), pMesh->GetNumUVs());

		for (size_t iUVs = 0; iUVs < pMesh->GetNumUVs(); iUVs++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iUVs);

			Vector vecUV = pMesh->GetUV(iUVs);
			sOBJFile << L"vt " << vecUV.x << L" " << vecUV.y << std::endl;
		}

		if (m_pWorkListener)
			m_pWorkListener->SetAction((eastl::string16(L"Writing ") + sNodeName + L" faces...").c_str(), pMesh->GetNumFaces());

		for (size_t iFaces = 0; iFaces < pMesh->GetNumFaces(); iFaces++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iFaces);

			sOBJFile << L"f";
			CConversionFace* pFace = pMesh->GetFace(iFaces);
			for (size_t iVertsInFace = 0; iVertsInFace < pFace->GetNumVertices(); iVertsInFace++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(iVertsInFace);
				sOBJFile << L" " << pVertex->v+1 << L"/" << pVertex->vu+1 << L"/" << pVertex->vn+1;
			}
			sOBJFile << std::endl;
		}

		sOBJFile << std::endl;

		sOBJFile.close();
	}

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}
