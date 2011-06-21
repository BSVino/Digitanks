#include <stdio.h>
#include <string.h>

#include "../modelconverter.h"
#include "common.h"
#include "strutils.h"

#include <string>

// Silo ascii
void CModelConverter::ReadSIA(const tstring& sFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	CConversionSceneNode* pScene = m_pScene->GetScene(m_pScene->AddScene(GetFilename(sFilename).append(_T(".sia"))));

	if (m_pWorkListener)
		m_pWorkListener->SetAction(_T("Reading file into memory..."), 0);

	FILE* fp = tfopen(sFilename, _T("r"));

	if (!fp)
	{
		printf("No input file. Sorry!\n");
		return;
	}

	fseek(fp, 0L, SEEK_END);
	long iOBJSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	// Make sure we allocate more than we need just in case.
	size_t iFileSize = (iOBJSize+1) * (sizeof(tchar)+1);
	tchar* pszEntireFile = (tchar*)malloc(iFileSize);
	tchar* pszCurrent = pszEntireFile;

	// Read the entire file into an array first for faster processing.
	tstring sLine;
	while (fgetts(sLine, fp))
	{
		tstrncpy(pszCurrent, iFileSize-(pszCurrent-pszEntireFile), sLine.c_str(), sLine.length());
		size_t iLength = sLine.length();

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

		eastl::vector<tstring> aTokens;
		tstrtok(pszLine, aTokens, _T(" "));
		const tchar* pszToken = aTokens[0].c_str();

		if (tstrncmp(pszToken, _T("-Version"), 8) == 0)
		{
			// Warning if version is later than 1.0, we may not support it
			int iMajor, iMinor;
			eastl::vector<tstring> asTokens;
			tstrtok(pszLine, asTokens, _T(" ."));

			if (asTokens.size() >= 3)
			{
				iMajor = stoi(asTokens[1]);
				iMinor = stoi(asTokens[2]);
				if (iMajor != 1 && iMinor != 0)
					printf("WARNING: I was programmed for version 1.0, this file is version %d.%d, so this might not work exactly right!\n", iMajor, iMinor);
			}
		}
		else if (tstrncmp(pszToken, _T("-Mat"), 4) == 0)
		{
			pszNextLine = ReadSIAMat(pszNextLine, pszCurrent, pScene, sFilename);
		}
		else if (tstrncmp(pszToken, _T("-Shape"), 6) == 0)
		{
			pszNextLine = ReadSIAShape(pszNextLine, pszCurrent, pScene);
		}
		else if (tstrncmp(pszToken, _T("-Texshape"), 9) == 0)
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

const tchar* CModelConverter::ReadSIAMat(const tchar* pszLine, const tchar* pszEnd, CConversionSceneNode* pScene, const tstring& sFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->SetAction(_T("Reading materials"), 0);

	size_t iCurrentMaterial = m_pScene->AddMaterial(_T(""));
	CConversionMaterial* pMaterial = m_pScene->GetMaterial(iCurrentMaterial);

	const tchar* pszNextLine = NULL;
	while (pszLine < pszEnd)
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

		eastl::vector<tstring> aTokens;
		tstrtok(pszLine, aTokens, _T(" "));
		const tchar* pszToken = aTokens[0].c_str();

		if (tstrncmp(pszToken, _T("-name"), 5) == 0)
		{
			tstring sName = pszLine+6;
			eastl::vector<tstring> aName;
			tstrtok(sName, aName, _T("\""));	// Strip out the quotation marks.

			pMaterial->m_sName = aName[0];
		}
		else if (tstrncmp(pszToken, _T("-dif"), 4) == 0)
		{
			eastl::vector<tstring> asTokens;
			tstrtok(pszLine, asTokens, _T(" "));
			if (asTokens.size() == 4)
			{
				pMaterial->m_vecDiffuse.x = stof(asTokens[1]);
				pMaterial->m_vecDiffuse.y = stof(asTokens[2]);
				pMaterial->m_vecDiffuse.z = stof(asTokens[3]);
			}
		}
		else if (tstrncmp(pszToken, _T("-amb"), 4) == 0)
		{
			eastl::vector<tstring> asTokens;
			tstrtok(pszLine, asTokens, _T(" "));
			if (asTokens.size() == 4)
			{
				pMaterial->m_vecAmbient.x = stof(asTokens[1]);
				pMaterial->m_vecAmbient.y = stof(asTokens[2]);
				pMaterial->m_vecAmbient.z = stof(asTokens[3]);
			}
		}
		else if (tstrncmp(pszToken, _T("-spec"), 5) == 0)
		{
			eastl::vector<tstring> asTokens;
			tstrtok(pszLine, asTokens, _T(" "));
			if (asTokens.size() == 4)
			{
				pMaterial->m_vecSpecular.x = stof(asTokens[1]);
				pMaterial->m_vecSpecular.y = stof(asTokens[2]);
				pMaterial->m_vecSpecular.z = stof(asTokens[3]);
			}
		}
		else if (tstrncmp(pszToken, _T("-emis"), 5) == 0)
		{
			eastl::vector<tstring> asTokens;
			tstrtok(pszLine, asTokens, _T(" "));
			if (asTokens.size() == 4)
			{
				pMaterial->m_vecEmissive.x = stof(asTokens[1]);
				pMaterial->m_vecEmissive.y = stof(asTokens[2]);
				pMaterial->m_vecEmissive.z = stof(asTokens[3]);
			}
		}
		else if (tstrncmp(pszToken, _T("-shin"), 5) == 0)
		{
			eastl::vector<tstring> asTokens;
			tstrtok(pszLine, asTokens, _T(" "));
			if (asTokens.size() == 2)
				pMaterial->m_flShininess = stof(asTokens[1]);
		}
		else if (tstrncmp(pszToken, _T("-tex"), 4) == 0)
		{
			const tchar* pszTexture = pszLine+5;

			tstring sName = pszTexture;
			eastl::vector<tstring> aName;
			tstrtok(sName, aName, _T("\""));	// Strip out the quotation marks.

			tstring sDirectory = GetDirectory(sFilename);

			pMaterial->m_sDiffuseTexture = sprintf(_T("%s/%s"), sDirectory.c_str(), aName[0].c_str());
		}
		else if (tstrncmp(pszToken, _T("-endMat"), 7) == 0)
		{
			return pszNextLine;
		}
	}

	return pszNextLine;
}

const tchar* CModelConverter::ReadSIAShape(const tchar* pszLine, const tchar* pszEnd, CConversionSceneNode* pScene, bool bCare)
{
	size_t iCurrentMaterial = ~0;

	CConversionMesh* pMesh = NULL;
	CConversionSceneNode* pMeshNode = NULL;
	size_t iAddV = 0;
	size_t iAddE = 0;
	size_t iAddUV = 0;
	size_t iAddN = 0;

	tstring sLastTask;
	tstring sToken;

	const tchar* pszNextLine = NULL;
	while (pszLine < pszEnd)
	{
		if (pszNextLine)
			pszLine = pszNextLine;

		size_t iLineLength = tstrlen(pszLine);
		pszNextLine = pszLine + iLineLength + 1;

		// This code used to call StripWhitespace() but that's too slow for very large files w/ millions of lines.
		// Instead we'll just cut the whitespace off the front and deal with whitespace on the end when we come to it.
		while (*pszLine && IsWhitespace(*pszLine))
			pszLine++;

		if (tstrlen(pszLine) == 0)
			continue;

		const tchar* pszToken = pszLine;

		while (*pszToken && *pszToken != L' ')
			pszToken++;

		sToken.reserve(iLineLength);
		sToken.clear();
		sToken.append(pszLine, pszToken-pszLine);
		sToken[pszToken-pszLine] = L'\0';
		pszToken = sToken.c_str();

		if (!bCare)
		{
			if (tstrncmp(pszToken, _T("-endShape"), 9) == 0)
				return pszNextLine;
			else
				continue;
		}

		if (tstrncmp(pszToken, _T("-snam"), 5) == 0)
		{
			// We name our mesh.
			tstring sName =pszLine+6;
			eastl::vector<tstring> aName;
			tstrtok(sName, aName, _T("\""));	// Strip out the quotation marks.

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
		else if (tstrncmp(pszToken, _T("-vert"), 5) == 0)
		{
			if (m_pWorkListener)
			{
				if (sLastTask == pszToken)
					m_pWorkListener->WorkProgress(0);
				else
				{
					m_pWorkListener->SetAction(_T("Reading vertex data"), 0);
					sLastTask = tstring(pszToken);
				}
			}

			// A vertex.
			float v[3];
			// scanf is pretty slow even for such a short string due to lots of mallocs.
			const tchar* pszToken = pszLine+5;
			int iDimension = 0;
			while (*pszToken)
			{
				while (pszToken[0] == L' ')
					pszToken++;

				v[iDimension++] = (float)stof(pszToken);
				if (iDimension >= 3)
					break;

				while (pszToken[0] != L' ')
					pszToken++;
			}
			pMesh->AddVertex(v[0], v[1], v[2]);
		}
		else if (tstrncmp(pszToken, _T("-edge"), 5) == 0)
		{
			if (m_pWorkListener)
			{
				if (sLastTask == pszToken)
					m_pWorkListener->WorkProgress(0);
				else
				{
					m_pWorkListener->SetAction(_T("Reading edge data"), 0);
					sLastTask = tstring(pszToken);
				}
			}

			// An edge. We only need them so we can tell where the creases are, so we can calculate normals properly.
			int e[2];
			// scanf is pretty slow even for such a short string due to lots of mallocs.
			const tchar* pszToken = pszLine+5;
			int iDimension = 0;
			while (*pszToken)
			{
				while (pszToken[0] == L' ')
					pszToken++;

				e[iDimension++] = (int)stoi(pszToken);
				if (iDimension >= 2)
					break;

				while (pszToken[0] != L' ')
					pszToken++;
			}
			pMesh->AddEdge(e[0]+iAddV, e[1]+iAddV);
		}
		else if (tstrncmp(pszToken, _T("-creas"), 6) == 0)
		{
			// An edge. We only need them so we can tell where the creases are, so we can calculate normals properly.
			tstring sCreases = pszLine+7;
			eastl::vector<tstring> aCreases;
			tstrtok(sCreases, aCreases, _T(" "));

			size_t iCreases = aCreases.size();
			// The first one is the number of creases, skip it
			for (size_t i = 1; i < iCreases; i++)
			{
				int iEdge = stoi(aCreases[i].c_str());
				pMesh->GetEdge(iEdge+iAddE)->m_bCreased = true;
			}
		}
		else if (tstrncmp(pszToken, _T("-setmat"), 7) == 0)
		{
			const tchar* pszMaterial = pszLine+8;
			size_t iNewMaterial = stoi(pszMaterial);

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
		else if (tstrncmp(pszToken, _T("-face"), 5) == 0)
		{
			if (m_pWorkListener)
			{
				if (sLastTask == pszToken)
					m_pWorkListener->WorkProgress(0);
				else
				{
					m_pWorkListener->SetAction(_T("Reading polygon data"), 0);
					sLastTask = tstring(pszToken);
				}
			}

			// A face.
			size_t iFace = pMesh->AddFace(iCurrentMaterial);

			if (iFace == 10000)
				pMeshNode->GetMeshInstance(0)->SetVisible(false);

			// scanf is pretty slow even for such a short string due to lots of mallocs.
			const tchar* pszToken = pszLine+6;

			size_t iVerts = stoi(pszToken);

			while (pszToken[0] != L' ')
				pszToken++;

			size_t iProcessed = 0;
			while (iProcessed++ < iVerts)
			{
				size_t iVertex = stoi(++pszToken)+iAddV;

				while (pszToken[0] != L' ')
					pszToken++;

				size_t iEdge = stoi(++pszToken)+iAddE;

				while (pszToken[0] != L' ')
					pszToken++;

				float flU = (float)stof(++pszToken);

				while (pszToken[0] != L' ')
					pszToken++;

				float flV = (float)stof(++pszToken);

				size_t iUV = pMesh->AddUV(flU, flV);

				size_t iNormal = pMesh->AddNormal(0, 0, 1);	// For now!

				pMesh->AddVertexToFace(iFace, iVertex, iUV, iNormal);
				pMesh->AddEdgeToFace(iFace, iEdge);

				while (pszToken[0] != L'\0' && pszToken[0] != L' ')
					pszToken++;
			}
		}
		else if (tstrncmp(pszToken, _T("-axis"), 5) == 0)
		{
			// This is the manipulator position and angles. The code below is untested and probably has the elements in the wrong
			// order. We don't support writing yet so no need to load it so I'm not bothering with it now.
		/*	Matrix4x4& m = pMeshNode->m_mManipulator;
			swscanf(sLine.c_str(), _T("-axis %f %f %f %f %f %f %f %f %f"),
				&m.m[0][3], &m.m[1][3], &m.m[2][3],
				&m.m[0][0], &m.m[0][1], &m.m[0][2],	// ?
				&m.m[1][0], &m.m[1][1], &m.m[1][2], // ?
				&m.m[2][0], &m.m[2][1], &m.m[2][2]  // ?
				);*/
		}
		else if (tstrncmp(pszToken, _T("-endShape"), 9) == 0)
		{
			break;
		}
	}

	return pszNextLine;
}

void CModelConverter::SaveSIA(const tstring& sFilename)
{
	tstring sSIAFileName = tstring(GetDirectory(sFilename).c_str()) + _T("/") + GetFilename(sFilename).c_str() + _T(".sia");

	std::wofstream sFile(convertstring<tchar, char>(sSIAFileName).c_str());
	if (!sFile.is_open())
		return;

	sFile.precision(8);
	sFile.setf(std::ios::fixed, std::ios::floatfield);

	if (m_pWorkListener)
	{
		m_pWorkListener->BeginProgress();
		m_pWorkListener->SetAction(_T("Writing materials..."), 0);
	}

	sFile << _T("-Version 1.0") << std::endl;

	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		sFile << _T("-Mat") << std::endl;

		CConversionMaterial* pMaterial = m_pScene->GetMaterial(i);

		sFile << "-amb " << pMaterial->m_vecAmbient.x << _T(" ") << pMaterial->m_vecAmbient.y << _T(" ") << pMaterial->m_vecAmbient.z << _T(" 0") << std::endl;
		sFile << "-dif " << pMaterial->m_vecDiffuse.x << _T(" ") << pMaterial->m_vecDiffuse.y << _T(" ") << pMaterial->m_vecDiffuse.z << _T(" 0") << std::endl;
		sFile << "-spec " << pMaterial->m_vecSpecular.x << _T(" ") << pMaterial->m_vecSpecular.y << _T(" ") << pMaterial->m_vecSpecular.z << _T(" 0") << std::endl;
		sFile << "-emis " << pMaterial->m_vecEmissive.x << _T(" ") << pMaterial->m_vecEmissive.y << _T(" ") << pMaterial->m_vecEmissive.z << _T(" 0") << std::endl;
		sFile << "-shin " << pMaterial->m_flShininess << std::endl;
		sFile << "-name \"" << pMaterial->GetName().c_str() << _T("\"") << std::endl;
		if (pMaterial->GetDiffuseTexture().length() > 0)
			sFile << "-tex \"" << pMaterial->GetDiffuseTexture().c_str() << _T("\"") << std::endl;
		sFile << _T("-endMat") << std::endl;
	}

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(i);

		size_t iAddV = pMesh->GetNumVertices();
		size_t iAddE = pMesh->GetNumEdges();
		size_t iAddUV = pMesh->GetNumUVs();
		size_t iAddN = pMesh->GetNumNormals();

		// Find the default scene for this mesh.
		CConversionSceneNode* pScene = NULL;
		for (size_t j = 0; j < m_pScene->GetNumScenes(); j++)
		{
			if (m_pScene->GetScene(j)->GetName() == pMesh->GetName() + _T(".sia"))
			{
				pScene = m_pScene->GetScene(j);
				break;
			}
		}

		tstring sNodeName = pMesh->GetName();

		sFile << _T("-Shape") << std::endl;
		sFile << _T("-snam \"") << sNodeName.c_str() << _T("\"") << std::endl;
		sFile << _T("-shad 0") << std::endl;
		sFile << _T("-shadw 1") << std::endl;

		if (m_pWorkListener)
			m_pWorkListener->SetAction((tstring(_T("Writing ")) + sNodeName + _T(" vertices...")).c_str(), pMesh->GetNumVertices());

		for (size_t iVertices = 0; iVertices < pMesh->GetNumVertices(); iVertices++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iVertices);

			Vector vecVertex = pMesh->GetVertex(iVertices);
			sFile << _T("-vert ") << vecVertex.x << _T(" ") << vecVertex.y << _T(" ") << vecVertex.z << std::endl;
		}

		if (m_pWorkListener)
			m_pWorkListener->SetAction((tstring(_T("Writing ")) + sNodeName + _T(" edges...")).c_str(), pMesh->GetNumEdges());

		tstring sCreases;
		tstring p;

		for (size_t iEdges = 0; iEdges < pMesh->GetNumEdges(); iEdges++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iEdges);

			CConversionEdge* pEdge = pMesh->GetEdge(iEdges);
			sFile << _T("-edge ") << pEdge->v1 << _T(" ") << pEdge->v2 << std::endl;

			if (pEdge->m_bCreased)
				sCreases += p.sprintf(_T(" %d"), iEdges);
		}

		if (sCreases.length())
			sFile << _T("-creas") << sCreases.c_str() << std::endl;

		if (m_pWorkListener)
			m_pWorkListener->SetAction((tstring(_T("Writing ")) + sNodeName + _T(" faces...")).c_str(), pMesh->GetNumFaces());

		size_t iMaterial = 0;

		for (size_t iFaces = 0; iFaces < pMesh->GetNumFaces(); iFaces++)
		{
			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(iFaces);

			CConversionFace* pFace = pMesh->GetFace(iFaces);

			if (iFaces == 0 || iMaterial != pFace->m)
			{
				iMaterial = pFace->m;

				if (iMaterial == ~0)
					sFile << _T("-setmat -1") << std::endl;
				else
				{
					CConversionSceneNode* pNode = m_pScene->GetDefaultSceneMeshInstance(pScene, pMesh, false);

					if (!pNode || pNode->GetNumMeshInstances() != 1)
						sFile << _T("-setmat -1") << std::endl;
					else
					{
						CConversionMaterialMap* pMap = pNode->GetMeshInstance(0)->GetMappedMaterial(iMaterial);
						if (pMap)
							sFile << _T("-setmat -1") << std::endl;
						else
							sFile << _T("-setmat ") << pMap->m_iMaterial << std::endl;
					}
				}
			}

			sFile << _T("-face ") << pFace->GetNumVertices();

			TAssert(pFace->GetNumEdges() == pFace->GetNumVertices());

			for (size_t iVertsInFace = 0; iVertsInFace < pFace->GetNumVertices(); iVertsInFace++)
			{
				CConversionVertex* pVertex = pFace->GetVertex(iVertsInFace);
				CConversionVertex* pNextVertex = pFace->GetVertex((iVertsInFace+1)%pFace->GetNumVertices());

				// Find the edge that heads in a counter-clockwise direction.
				size_t iEdge = ~0;
				for (size_t i = 0; i < pFace->GetNumEdges(); i++)
				{
					size_t iEdgeCandidate = pFace->GetEdge(i);
					CConversionEdge* pEdge = pMesh->GetEdge(iEdgeCandidate);
					if ((pEdge->v1 == pVertex->v && pEdge->v2 == pNextVertex->v) || (pEdge->v2 == pVertex->v && pEdge->v1 == pNextVertex->v))
					{
						iEdge = iEdgeCandidate;
						break;
					}
				}
				TAssert(iEdge != ~0);

				Vector vecUV = pMesh->GetUV(pVertex->vu);
				sFile << _T(" ") << pVertex->v << _T(" ") << iEdge << _T(" ") << vecUV.x << _T(" ") << vecUV.y;
			}

			sFile << std::endl;
		}

		sFile << _T("-axis 0 0.5 0 1 0 0 0 1 0 0 0 1") << std::endl;
		sFile << _T("-mirp 0 0 0 1 0 0") << std::endl;
		sFile << _T("-endShape") << std::endl;
	}

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}
