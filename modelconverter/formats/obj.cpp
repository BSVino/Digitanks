#include <stdio.h>
#include <string.h>

#include <common.h>

#include "../modelconverter.h"
#include "strutils.h"


void CModelConverter::ReadOBJ(const tstring& sFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	FILE* fp = tfopen(sFilename, _T("r"));

	if (!fp)
	{
		printf("No input file. Sorry!\n");
		return;
	}

	CConversionSceneNode* pScene = m_pScene->GetScene(m_pScene->AddScene(GetFilename(sFilename).append(_T(".obj"))));

	CConversionMesh* pMesh = m_pScene->GetMesh(m_pScene->AddMesh(GetFilename(sFilename)));
	// Make sure it exists.
	CConversionSceneNode* pMeshNode = m_pScene->GetDefaultSceneMeshInstance(pScene, pMesh);

	size_t iCurrentMaterial = ~0;
	size_t iSmoothingGroup = ~0;

	bool bSmoothingGroups = false;

	tstring sLastTask;

	int iTotalVertices = 0;
	int iTotalFaces = 0;
	int iVerticesComplete = 0;
	int iFacesComplete = 0;

	if (m_pWorkListener)
		m_pWorkListener->SetAction(_T("Reading file into memory..."), 0);

	fseek(fp, 0L, SEEK_END);
	long iOBJSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	// Make sure we allocate more than we need just in case.
	size_t iFileSize = (iOBJSize+1) * (sizeof(tchar)+1);
	tchar* pszEntireFile = (tchar*)malloc(iFileSize);
	tchar* pszCurrent = pszEntireFile;
	pszCurrent[0] = _T('\0');

	// Read the entire file into an array first for faster processing.
	tstring sLine;
	while (fgetts(sLine, fp))
	{
		tstrncpy(pszCurrent, iFileSize-(pszCurrent-pszEntireFile), sLine.c_str(), sLine.length());
		size_t iLength = sLine.length();

		tchar cLastChar = pszCurrent[iLength-1];
		while (cLastChar == _T('\n') || cLastChar == _T('\r'))
		{
			pszCurrent[iLength-1] = _T('\0');
			iLength--;
			cLastChar = pszCurrent[iLength-1];
		}

		pszCurrent += iLength;
		pszCurrent++;

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(0);
	}

	pszCurrent[0] = _T('\0');

	fclose(fp);

	const tchar* pszLine = pszEntireFile;
	const tchar* pszNextLine = NULL;
	while (pszLine < pszCurrent)
	{
		if (pszNextLine)
			pszLine = pszNextLine;

		pszNextLine = pszLine + tstrlen(pszLine) + 1;

		// This code used to call StripWhitespace() but that's too slow for very large files w/ millions of lines.
		// Instead we'll just cut the whitespace off the front and deal with whitespace on the end when we come to it.
		while (*pszLine && IsWhitespace(*pszLine))
			pszLine++;

		if (tstrlen(pszLine) == 0)
			continue;

		if (pszLine[0] == '#')
		{
			// ZBrush is kind enough to notate exactly how many vertices and faces we have in the comments at the top of the file.
			if (tstrncmp(pszLine, _T("#Vertex Count"), 13) == 0)
			{
				iTotalVertices = stoi(pszLine+13);
				pMesh->SetTotalVertices(iTotalVertices);
			}

			if (tstrncmp(pszLine, _T("#Face Count"), 11) == 0)
			{
				iTotalFaces = stoi(pszLine+11);
				pMesh->SetTotalFaces(iTotalFaces);

				// Don't kill the video card while we're loading the faces.
				if (iTotalFaces > 10000)
					pMeshNode->GetMeshInstance(0)->SetVisible(false);
			}

			continue;
		}

		eastl::vector<tstring> asTokens;
		tstrtok(pszLine, asTokens, _T(" "));
		const tchar* pszToken = asTokens[0].c_str();

		if (tstrncmp(pszToken, _T("mtllib"), 6) == 0)
		{
			tstring sDirectory = GetDirectory(sFilename);
			tstring sMaterial = sprintf(tstring("%s/%s"), sDirectory.c_str(), pszLine + 7);
			ReadMTL(sMaterial);
		}
		else if (tstrncmp(pszToken, _T("o"), 1) == 0)
		{
			// Dunno what this does.
		}
		else if (tstrncmp(pszToken, _T("v"), 1) == 0)
		{
			if (m_pWorkListener)
			{
				if (tstrncmp(sLastTask.c_str(), pszToken, sLastTask.length()) == 0)
					m_pWorkListener->WorkProgress(iVerticesComplete++);
				else
				{
					m_pWorkListener->SetAction(_T("Reading vertex data"), iTotalVertices);
					sLastTask = tstring(pszToken);
				}
			}

			// A vertex.
			float v[3];
			// scanf is pretty slow even for such a short string due to lots of mallocs.
			const tchar* pszToken = pszLine+1;
			int iDimension = 0;
			while (*pszToken)
			{
				while (pszToken[0] == _T(' '))
					pszToken++;

				v[iDimension++] = (float)stof(pszToken);
				if (iDimension >= 3)
					break;

				while (pszToken[0] != _T(' '))
					pszToken++;
			}
			pMesh->AddVertex(v[0], v[1], v[2]);
		}
		else if (tstrncmp(pszToken, _T("vn"), 2) == 0)
		{
			if (m_pWorkListener)
			{
				if (tstrncmp(sLastTask.c_str(), pszToken, sLastTask.length()) == 0)
					m_pWorkListener->WorkProgress(0);
				else
					m_pWorkListener->SetAction(_T("Reading vertex normal data"), 0);
			}
			sLastTask = tstring(pszToken);

			// A vertex normal.
			float x, y, z;
			eastl::vector<tstring> asTokens;
			tstrtok(pszLine, asTokens, _T(" "));
			if (asTokens.size() == 4)
			{
				x = stof(asTokens[1]);
				y = stof(asTokens[2]);
				z = stof(asTokens[3]);
				pMesh->AddNormal(x, y, z);
			}
		}
		else if (tstrncmp(pszToken, _T("vt"), 2) == 0)
		{
			if (m_pWorkListener)
			{
				if (tstrncmp(sLastTask.c_str(), pszToken, sLastTask.length()) == 0)
					m_pWorkListener->WorkProgress(0);
				else
					m_pWorkListener->SetAction(_T("Reading texture coordinate data"), 0);
			}
			sLastTask = tstring(pszToken);

			// A UV coordinate for a vertex.
			float u, v;
			eastl::vector<tstring> asTokens;
			tstrtok(pszLine, asTokens, _T(" "));
			if (asTokens.size() == 3)
			{
				u = stof(asTokens[1]);
				v = stof(asTokens[2]);
				pMesh->AddUV(u, v);
			}
		}
		else if (tstrncmp(pszToken, _T("g"), 1) == 0)
		{
			// A group of faces.
			pMesh->AddBone(pszLine+2);
		}
		else if (tstrncmp(pszToken, _T("usemtl"), 6) == 0)
		{
			// All following faces should use this material.
			tstring sMaterial = tstring(pszLine+7);
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
		else if (tstrncmp(pszToken, _T("s"), 1) == 0)
		{
			if (tstrncmp(pszLine, _T("s off"), 5) == 0)
			{
				iSmoothingGroup = ~0;
			}
			else
			{
				bSmoothingGroups = true;
				eastl::vector<tstring> asTokens;
				tstrtok(pszLine, asTokens, _T(" "));
				if (asTokens.size() == 2)
					iSmoothingGroup = stoi(asTokens[1]);
			}
		}
		else if (tstrncmp(pszToken, _T("f"), 1) == 0)
		{
			if (m_pWorkListener)
			{
				if (tstrncmp(sLastTask.c_str(), pszToken, sLastTask.length()) == 0)
					m_pWorkListener->WorkProgress(iFacesComplete++);
				else
				{
					m_pWorkListener->SetAction(_T("Reading polygon data"), iTotalFaces);
					sLastTask = tstring(pszToken);
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

			for (size_t i = 1; i < asTokens.size(); i++)
			{
				const tchar* pszToken = asTokens[i].c_str();
				if (tstrlen(pszToken) == 0)
					continue;

				// We don't use size_t because SOME EXPORTS put out negative numbers.
				long f[3];
				bool bValues[3];
				bValues[0] = false;
				bValues[1] = false;
				bValues[2] = false;

				// scanf is pretty slow even for such a short string due to lots of mallocs.
				const tchar* pszValues = pszToken;
				int iValue = 0;
				do
				{
					if (!pszValues)
						break;

					if (!bValues[0] || pszValues[0] == _T('/'))
					{
						if (pszValues[0] == _T('/'))
							pszValues++;

						bValues[iValue] = true;
						f[iValue++] = (long)stoi(pszValues);
						if (iValue >= 3)
							break;
					}

					// Don't advance if we're on a slash, because that means empty slashes. ie, 11//12 <-- the 12 would get skipped.
					if (pszValues[0] != _T('/'))
						pszValues++;
				}
				while (*pszValues);

				if (bValues[0])
				{
					if (f[0] < 0)
						f[0] = (long)pMesh->GetNumVertices()+f[0]+1;
					TAssert ( f[0] >= 1 && f[0] < (long)pMesh->GetNumVertices()+1 );
				}

				if (bValues[1] && pMesh->GetNumUVs())
				{
					if (f[1] < 0)
						f[1] = (long)pMesh->GetNumUVs()+f[1]+1;
					TAssert ( f[1] >= 1 && f[1] < (long)pMesh->GetNumUVs()+1 );
				}

				if (bValues[2] && pMesh->GetNumNormals())
				{
					if (f[2] < 0)
						f[2] = (long)pMesh->GetNumNormals()+f[2]+1;
					TAssert ( f[2] >= 1 && f[2] < (long)pMesh->GetNumNormals()+1 );
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

void CModelConverter::ReadMTL(const tstring& sFilename)
{
	FILE* fp = tfopen(sFilename, _T("r"));

	if (!fp)
		return;

	if (m_pWorkListener)
		m_pWorkListener->SetAction(_T("Reading materials"), 0);

	size_t iCurrentMaterial = ~0;

	tstring sLine;
	while (fgetts(sLine, fp))
	{
		sLine = StripWhitespace(sLine);

		if (sLine.length() == 0)
			continue;

		if (sLine[0] == '#')
			continue;

		eastl::vector<tstring> asTokens;
		tstrtok(sLine, asTokens, _T(" "));
		const tchar* pszToken = NULL;
		pszToken = asTokens[0].c_str();

		CConversionMaterial* pMaterial = NULL;
		if (iCurrentMaterial != ~0)
			pMaterial = m_pScene->GetMaterial(iCurrentMaterial);

		if (tstrncmp(pszToken, _T("newmtl"), 6) == 0)
		{
			pszToken = asTokens[1].c_str();
			CConversionMaterial oMaterial(pszToken, Vector(0.2f,0.2f,0.2f), Vector(0.8f,0.8f,0.8f), Vector(1,1,1), Vector(0,0,0), 1.0, 0);
			iCurrentMaterial = m_pScene->AddMaterial(oMaterial);
		}
		else if (tstrncmp(pszToken, _T("Ka"), 2) == 0)
		{
			eastl::vector<tstring> asTokens;
			tstrtok(sLine, asTokens, _T(" "));
			if (asTokens.size() == 4)
			{
				pMaterial->m_vecAmbient.x = stof(asTokens[1]);
				pMaterial->m_vecAmbient.y = stof(asTokens[2]);
				pMaterial->m_vecAmbient.z = stof(asTokens[3]);
			}
		}
		else if (tstrncmp(pszToken, _T("Kd"), 2) == 0)
		{
			eastl::vector<tstring> asTokens;
			tstrtok(sLine, asTokens, _T(" "));
			if (asTokens.size() == 4)
			{
				pMaterial->m_vecDiffuse.x = stof(asTokens[1]);
				pMaterial->m_vecDiffuse.y = stof(asTokens[2]);
				pMaterial->m_vecDiffuse.z = stof(asTokens[3]);
			}
		}
		else if (tstrncmp(pszToken, _T("Ks"), 2) == 0)
		{
			eastl::vector<tstring> asTokens;
			tstrtok(sLine, asTokens, _T(" "));
			if (asTokens.size() == 4)
			{
				pMaterial->m_vecSpecular.x = stof(asTokens[1]);
				pMaterial->m_vecSpecular.y = stof(asTokens[2]);
				pMaterial->m_vecSpecular.z = stof(asTokens[3]);
			}
		}
		else if (tstrncmp(pszToken, _T("d"), 1) == 0 || tstrncmp(pszToken, _T("Tr"), 2) == 0)
		{
			pMaterial->m_flTransparency = (float)stof(asTokens[1]);
		}
		else if (tstrncmp(pszToken, _T("Ns"), 2) == 0)
		{
			pMaterial->m_flShininess = (float)stof(asTokens[1])*128/1000;
		}
		else if (tstrncmp(pszToken, _T("illum"), 5) == 0)
		{
			pMaterial->m_eIllumType = (IllumType_t)stoi(asTokens[1]);
		}
		else if (tstrncmp(pszToken, _T("map_Kd"), 6) == 0)
		{
			pszToken = asTokens[1].c_str();

			FILE* fpTest = tfopen(pszToken, _T("r"));

			if (fpTest)
			{
				fclose(fpTest);

				pMaterial->m_sDiffuseTexture = tstring(pszToken);
			}
			else
			{
				tstring sDirectory = GetDirectory(sFilename);

				pMaterial->m_sDiffuseTexture = sprintf(tstring("%s/%s"), sDirectory.c_str(), pszToken);
			}
		}
	}

	fclose(fp);
}

void CModelConverter::SaveOBJ(const tstring& sFilename)
{
	tstring sMaterialFileName = tstring(GetDirectory(sFilename).c_str()) + _T("/") + GetFilename(sFilename).c_str() + _T(".mtl");

	std::wofstream sMaterialFile(convertstring<tchar, char>(sMaterialFileName).c_str());
	if (!sMaterialFile.is_open())
		return;

	if (m_pWorkListener)
	{
		m_pWorkListener->BeginProgress();
		m_pWorkListener->SetAction(_T("Writing materials file"), 0);
	}

	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		CConversionMaterial* pMaterial = m_pScene->GetMaterial(i);
		sMaterialFile << "newmtl " << pMaterial->GetName().c_str() << std::endl;
		sMaterialFile << "Ka " << pMaterial->m_vecAmbient.x << _T(" ") << pMaterial->m_vecAmbient.y << _T(" ") << pMaterial->m_vecAmbient.z << std::endl;
		sMaterialFile << "Kd " << pMaterial->m_vecDiffuse.x << _T(" ") << pMaterial->m_vecDiffuse.y << _T(" ") << pMaterial->m_vecDiffuse.z << std::endl;
		sMaterialFile << "Ks " << pMaterial->m_vecSpecular.x << _T(" ") << pMaterial->m_vecSpecular.y << _T(" ") << pMaterial->m_vecSpecular.z << std::endl;
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

		// Find the default scene for this mesh.
		CConversionSceneNode* pScene = NULL;
		for (size_t j = 0; j < m_pScene->GetNumScenes(); j++)
		{
			if (m_pScene->GetScene(j)->GetName() == pMesh->GetName() + _T(".obj"))
			{
				pScene = m_pScene->GetScene(j);
				break;
			}
		}

		tstring sNodeName = pMesh->GetName();

		tstring sOBJFilename = tstring(GetDirectory(sFilename).c_str()) + _T("/") + GetFilename(sNodeName).c_str() + _T(".obj");
		tstring sMTLFilename = tstring(GetFilename(sFilename).c_str()) + _T(".mtl");

		if (m_pScene->GetNumMeshes() == 1)
			sOBJFilename = sFilename;

		std::wofstream sOBJFile(convertstring<tchar, char>(sOBJFilename).c_str());
		sOBJFile.precision(8);
		sOBJFile.setf(std::ios::fixed, std::ios::floatfield);

		sOBJFile << _T("mtllib ") << sMTLFilename.c_str() << std::endl;
		sOBJFile << std::endl;

		sOBJFile << _T("o ") << sNodeName.c_str() << std::endl;

		if (m_pWorkListener)
			m_pWorkListener->SetAction((tstring(_T("Writing ")) + sNodeName + _T(" vertices...")).c_str(), pMesh->GetNumVertices());

		for (size_t iVertices = 0; iVertices < pMesh->GetNumVertices(); iVertices++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iVertices);

			Vector vecVertex = pMesh->GetVertex(iVertices);
			sOBJFile << _T("v ") << vecVertex.x << _T(" ") << vecVertex.y << _T(" ") << vecVertex.z << std::endl;
		}

		if (m_pWorkListener)
			m_pWorkListener->SetAction((tstring(_T("Writing ")) + sNodeName + _T(" normals...")).c_str(), pMesh->GetNumNormals());

		for (size_t iNormals = 0; iNormals < pMesh->GetNumNormals(); iNormals++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iNormals);

			Vector vecNormal = pMesh->GetNormal(iNormals);
			sOBJFile << _T("vn ") << vecNormal.x << _T(" ") << vecNormal.y << _T(" ") << vecNormal.z << std::endl;
		}

		if (m_pWorkListener)
			m_pWorkListener->SetAction((tstring(_T("Writing ")) + sNodeName + _T(" UVs...")).c_str(), pMesh->GetNumUVs());

		for (size_t iUVs = 0; iUVs < pMesh->GetNumUVs(); iUVs++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iUVs);

			Vector vecUV = pMesh->GetUV(iUVs);
			sOBJFile << _T("vt ") << vecUV.x << _T(" ") << vecUV.y << std::endl;
		}

		if (m_pWorkListener)
			m_pWorkListener->SetAction((tstring(_T("Writing ")) + sNodeName + _T(" faces...")).c_str(), pMesh->GetNumFaces());

		size_t iLastMaterial = ~0;

		for (size_t iFaces = 0; iFaces < pMesh->GetNumFaces(); iFaces++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iFaces);

			CConversionFace* pFace = pMesh->GetFace(iFaces);

			if (pFace->m != iLastMaterial)
			{
				iLastMaterial = pFace->m;

				CConversionSceneNode* pNode = m_pScene->GetDefaultSceneMeshInstance(pScene, pMesh, false);
				if (!pNode || pNode->GetNumMeshInstances() != 1)
					sOBJFile << "usemtl " << iLastMaterial << std::endl;
				else
				{
					CConversionMaterialMap* pMap = pNode->GetMeshInstance(0)->GetMappedMaterial(iLastMaterial);
					if (!pMap)
						sOBJFile << "usemtl " << iLastMaterial << std::endl;
					else
						sOBJFile << "usemtl " << m_pScene->GetMaterial(pMap->m_iMaterial)->GetName().c_str() << std::endl;
				}
			}

			sOBJFile << _T("f");
			for (size_t iVertsInFace = 0; iVertsInFace < pFace->GetNumVertices(); iVertsInFace++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(iVertsInFace);
				sOBJFile << _T(" ") << pVertex->v+1 << _T("/") << pVertex->vu+1 << _T("/") << pVertex->vn+1;
			}
			sOBJFile << std::endl;
		}

		sOBJFile << std::endl;

		sOBJFile.close();
	}

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}
