#include <assert.h>
#include <algorithm>

#include "convmesh.h"
#include <geometry.h>

CConversionMesh::CConversionMesh(class CConversionScene* pScene, const wchar_t* pszName)
{
	m_pScene = pScene;
	wcscpy(m_szName, pszName);
}

void CConversionMesh::Clear()
{
	m_szName[0] = L'\0';

	m_aVertices.clear();
	m_aNormals.clear();
	m_aUVs.clear();
	m_aBones.clear();
	m_aFaces.clear();
	m_aEdges.clear();

	m_vecOrigin = Vector();
}

void CConversionMesh::CalculateEdgeData()
{
	// No edges? Well then let's generate them.
	if (!GetNumEdges())
	{
		if (m_pScene->m_pWorkListener)
			m_pScene->m_pWorkListener->SetAction(L"Generating edges", GetNumFaces());

		// For every face, find and create edges.
		for (size_t iFace = 0; iFace < GetNumFaces(); iFace++)
		{
			CConversionFace* pFace = GetFace(iFace);

			for (size_t iVertex = 0; iVertex < pFace->GetNumVertices(); iVertex++)
			{
				size_t iAdjVertex;
				if (iVertex == pFace->GetNumVertices()-1)
					iAdjVertex = 0;
				else
					iAdjVertex = iVertex+1;

				// Find the other face with these two vertices!
				std::vector<size_t>& aFaces = m_aaVertexFaceMap[pFace->GetVertex(iVertex)->v];
				for (size_t iFace2 = 0; iFace2 < aFaces.size(); iFace2++)
				{
					if (iFace == aFaces[iFace2])
						continue;

					CConversionFace* pFace2 = GetFace(aFaces[iFace2]);

					size_t iVertex1 = pFace2->FindVertex(pFace->GetVertex(iVertex)->v);
					if (iVertex1 == ~0)
						continue;

					size_t iVertex2 = pFace2->FindVertex(pFace->GetVertex(iAdjVertex)->v);
					if (iVertex2 == ~0)
						continue;

					// I'm paranoid. Since these are unsigned make sure to subtract the larger by the smaller.
					if ((iVertex2>iVertex1?iVertex2-iVertex1:iVertex1-iVertex2) == 1)
					{
						// Check for duplicate edges first.
						bool bFoundEdge = false;
						size_t iEdge;
						for (iEdge = 0; iEdge < pFace->GetNumEdges(); iEdge++)
						{
							CConversionEdge* pEdge = GetEdge(pFace->GetEdge(iEdge));
							if (pEdge->HasVertex(pFace->GetVertex(iVertex)->v) && pEdge->HasVertex(pFace->GetVertex(iAdjVertex)->v))
							{
								bFoundEdge = true;
								break;
							}
						}

						// Since the edge gets added to both faces we only need to check one face.

						if (bFoundEdge)
							break;

						// By Jove we found it.
						iEdge = AddEdge(pFace->GetVertex(iVertex)->v, pFace->GetVertex(iAdjVertex)->v);
						CConversionEdge* pEdge = GetEdge(iEdge);
						pEdge->f1 = iFace;
						pEdge->f2 = aFaces[iFace2];
						AddEdgeToFace(iFace, iEdge);
						AddEdgeToFace(aFaces[iFace2], iEdge);
						break;
					}
				}
			}

			if (m_pScene->m_pWorkListener)
				m_pScene->m_pWorkListener->WorkProgress(iFace);
		}
	}
	else
	{
		if (m_pScene->m_pWorkListener)
			m_pScene->m_pWorkListener->SetAction(L"Finding edges", GetNumFaces());

		// For every edge, mark the vertexes it contains.
		for (size_t iFace = 0; iFace < GetNumFaces(); iFace++)
		{
			CConversionFace* pFace = GetFace(iFace);

			for (size_t iEdge = 0; iEdge < pFace->GetNumEdges(); iEdge++)
			{
				CConversionEdge* pEdge = GetEdge(pFace->GetEdge(iEdge));

				// There can't be more than two faces per edge. It's inconceivable!
				assert(pEdge->f1 == ((size_t)~0) || pEdge->f2 == ((size_t)~0));
				if (pEdge->f1 == ~0)
					pEdge->f1 = iFace;
				else
					pEdge->f2 = iFace;
			}

			if (m_pScene->m_pWorkListener)
				m_pScene->m_pWorkListener->WorkProgress(iFace);
		}
	}

	if (m_pScene->m_pWorkListener)
		m_pScene->m_pWorkListener->SetAction(L"Calculating edge data", GetNumEdges());

	// For every conversion vertex, mark the edges it meets.
	for (size_t iEdge = 0; iEdge < GetNumEdges(); iEdge++)
	{
		CConversionEdge* pEdge = GetEdge(iEdge);

		if (pEdge->f1 != ((size_t)~0))
		{
			// Find every vertex on this face and mark its vertices as having this edge.
			CConversionFace* pF1 = GetFace(pEdge->f1);
			for (size_t iVertex = 0; iVertex < pF1->GetNumVertices(); iVertex++)
			{
				CConversionVertex* pVertex = pF1->GetVertex(iVertex);
				if (pVertex->v == pEdge->v1)
					pVertex->m_aEdges.push_back(iEdge);
				else if (pVertex->v == pEdge->v2)
					pVertex->m_aEdges.push_back(iEdge);
			}
		}

		if (pEdge->f2 != ((size_t)~0))
		{
			// Find every vertex on this face and mark its vertices as having this edge.
			CConversionFace* pF2 = GetFace(pEdge->f2);
			for (size_t iVertex = 0; iVertex < pF2->GetNumVertices(); iVertex++)
			{
				CConversionVertex* pVertex = pF2->GetVertex(iVertex);
				if (pVertex->v == pEdge->v1)
					pVertex->m_aEdges.push_back(iEdge);
				else if (pVertex->v == pEdge->v2)
					pVertex->m_aEdges.push_back(iEdge);
			}
		}

		if (m_pScene->m_pWorkListener)
			m_pScene->m_pWorkListener->WorkProgress(iEdge);
	}
}

void CConversionMesh::CalculateVertexNormals()
{
	if (m_pScene->m_pWorkListener)
		m_pScene->m_pWorkListener->SetAction(L"Calculating vertex normals", GetNumFaces());

	m_aNormals.clear();

	// Got to calculate vertex normals now. We have to do it after we read faces because we need all of the face data loaded first.
	for (size_t iFace = 0; iFace < GetNumFaces(); iFace++)
	{
		CConversionFace* pFace = GetFace(iFace);

		// Loop through all vertices to calculate normals for
		for (size_t iVertex = 0; iVertex < pFace->GetNumVertices(); iVertex++)
		{
			CConversionVertex* pVertex = pFace->GetVertex(iVertex);

			// Build a list of faces that this vertex should use to calculate normals with.
			std::vector<size_t> aNormalFaces;
			pFace->FindAdjacentFaces(aNormalFaces, pVertex->v, true);

			Vector vecNormal = Vector();
			size_t iNormalFaces = 0;
			for (size_t iNormalFace = 0; iNormalFace < aNormalFaces.size(); iNormalFace++)
			{
				CConversionFace* pOtherFace = GetFace(aNormalFaces[iNormalFace]);

				if (pOtherFace->m_iSmoothingGroup != pFace->m_iSmoothingGroup)
					continue;

				iNormalFaces++;
				vecNormal += pOtherFace->GetNormal();
			}

			vecNormal /= (float)iNormalFaces;
			vecNormal.Normalize();

			// Find similar normals to save memory?
			// ... nah!
			pVertex->vn = AddNormal(vecNormal.x, vecNormal.y, vecNormal.z);
		}

		if (m_pScene->m_pWorkListener)
			m_pScene->m_pWorkListener->WorkProgress(iFace);
	}
}

void CConversionMesh::TranslateOrigin()
{
	if (m_vecOrigin.LengthSqr() == 0)
		return;

	if (m_pScene->m_pWorkListener)
		m_pScene->m_pWorkListener->SetAction(L"Translating origins", GetNumVertices());

	// Translate each vertex to the axis, essentially centering it around Silo's manipulator.
	for (size_t iVertex = 0; iVertex < GetNumVertices(); iVertex++)
	{
		m_aVertices[iVertex] -= m_vecOrigin;

		if (m_pScene->m_pWorkListener)
			m_pScene->m_pWorkListener->WorkProgress(iVertex);
	}

	m_vecOrigin = Vector(0,0,0);
}

void CConversionMesh::CalculateExtends()
{
	if (!GetNumVertices())
	{
		m_oExtends = AABB();
		return;
	}

	if (m_pScene->m_pWorkListener)
		m_pScene->m_pWorkListener->SetAction(L"Calculating extends", GetNumVertices());

	Vector vecMins = m_aVertices[0];
	Vector vecMaxs = m_aVertices[0];

	for (size_t iVertex = 0; iVertex < GetNumVertices(); iVertex++)
	{
		for (size_t i = 0; i < 3; i++)
		{
			if (m_aVertices[iVertex][i] < vecMins[i])
				vecMins[i] = m_aVertices[iVertex][i];
			if (m_aVertices[iVertex][i] > vecMaxs[i])
				vecMaxs[i] = m_aVertices[iVertex][i];
		}

		if (m_pScene->m_pWorkListener)
			m_pScene->m_pWorkListener->WorkProgress(iVertex);
	}

	m_oExtends = AABB(vecMins, vecMaxs);
}

CConversionScene::CConversionScene()
{
	m_pWorkListener = NULL;
}

void CConversionScene::DestroyAll()
{
	m_aMaterials.clear();
	m_aMeshes.clear();
}

void CConversionScene::CalculateExtends()
{
	if (!GetNumMeshes())
	{
		m_oExtends = AABB();
		return;
	}

	Vector vecMins;
	Vector vecMaxs;

	size_t m;
	for (m = 0; m < GetNumMeshes(); m++)
	{
		GetMesh(m)->CalculateExtends();
		if (m == 0)
		{
			vecMins = GetMesh(m)->m_oExtends.m_vecMins;
			vecMaxs = GetMesh(m)->m_oExtends.m_vecMaxs;
		}
		else
		{
			for (size_t i = 0; i < 3; i++)
			{
				if (GetMesh(m)->m_oExtends.m_vecMins[i] < vecMins[i])
					vecMins[i] = GetMesh(m)->m_oExtends.m_vecMins[i];
				if (GetMesh(m)->m_oExtends.m_vecMaxs[i] > vecMaxs[i])
					vecMaxs[i] = GetMesh(m)->m_oExtends.m_vecMaxs[i];
			}
		}

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(m);
	}

	m_oExtends = AABB(vecMins, vecMaxs);
}

size_t CConversionScene::AddMaterial(const wchar_t* pszName)
{
	m_aMaterials.push_back(CConversionMaterial(pszName));
	return m_aMaterials.size()-1;
}

size_t CConversionScene::AddMaterial(CConversionMaterial& oMaterial)
{
	m_aMaterials.push_back(oMaterial);
	return m_aMaterials.size()-1;
}

size_t CConversionScene::FindMaterial(const wchar_t* pszName)
{
	for (size_t i = 0; i < m_aMaterials.size(); i++)
		if (wcscmp(pszName, m_aMaterials[i].m_szName) == 0)
			return i;

	return ((size_t)~0);
}

size_t CConversionScene::AddMesh(const wchar_t* pszName)
{
	m_aMeshes.push_back(CConversionMesh(this, pszName));
	return m_aMeshes.size()-1;
}

size_t CConversionScene::FindMesh(const wchar_t* pszName)
{
	for (size_t i = 0; i < m_aMeshes.size(); i++)
		if (wcscmp(pszName, m_aMeshes[i].m_szName) == 0)
			return i;

	return ((size_t)~0);
}

size_t CConversionScene::FindMesh(CConversionMesh* pMesh)
{
	for (size_t i = 0; i < m_aMeshes.size(); i++)
		if (&m_aMeshes[i] == pMesh)
			return i;

	return ((size_t)~0);
}

size_t CConversionMesh::AddVertex(float x, float y, float z)
{
	m_aVertices.push_back(Vector(x, y, z));
	m_aaVertexFaceMap.push_back(std::vector<size_t>());
	return m_aVertices.size()-1;
}

size_t CConversionMesh::AddNormal(float x, float y, float z)
{
	m_aNormals.push_back(Vector(x, y, z));
	return m_aNormals.size()-1;
}

size_t CConversionMesh::AddUV(float u, float v)
{
	m_aUVs.push_back(Vector(u, v, 0));
	return m_aUVs.size()-1;
}

size_t CConversionMesh::AddBone(const wchar_t* pszName)
{
	m_aBones.push_back(CConversionBone(pszName));
	return m_aBones.size()-1;
}

size_t CConversionMesh::AddEdge(size_t v1, size_t v2)
{
	m_aEdges.push_back(CConversionEdge(v1, v2));
	return m_aEdges.size()-1;
}

Vector CConversionFace::GetNormal()
{
	assert(GetNumVertices() >= 3);

	// Precompute this shit maybe?

	Vector v1 = m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[0].v);
	Vector v2 = m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[1].v);
	Vector v3 = m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[2].v);

	return (v3 - v2).Normalized().Cross((v1 - v2).Normalized());
}

Vector CConversionFace::GetCenter()
{
	assert(GetNumVertices() >= 3);

	// Precompute this shit maybe?
	Vector v(0, 0, 0);

	for (size_t i = 0; i < m_aVertices.size(); i++)
		v += m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[i].v);

	return v / (float)m_aVertices.size();
}

float CConversionFace::GetArea()
{
	Vector a = m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[0].v);
	Vector b = m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[1].v);

	float flArea = 0;

	for (size_t i = 0; i < m_aVertices.size()-2; i++)
	{
		Vector c = m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[i+2].v);

		flArea += TriangleArea(a, b, c);
	}

	return flArea;
}

float CConversionFace::GetUVArea()
{
	Vector a = m_pScene->GetMesh(m_iMesh)->GetUV(m_aVertices[0].vt);
	Vector b = m_pScene->GetMesh(m_iMesh)->GetUV(m_aVertices[1].vt);

	float flArea = 0;

	for (size_t i = 0; i < m_aVertices.size()-2; i++)
	{
		Vector c = m_pScene->GetMesh(m_iMesh)->GetUV(m_aVertices[i+2].vt);

		flArea += TriangleArea(a, b, c);
	}

	return flArea;
}

void CConversionFace::FindAdjacentFaces(std::vector<size_t>& aResult, size_t iVert, bool bIgnoreCreased)
{
	aResult.push_back(m_pScene->GetMesh(m_iMesh)->FindFace(this));
	FindAdjacentFacesInternal(aResult, iVert, bIgnoreCreased);
}

void CConversionFace::FindAdjacentFacesInternal(std::vector<size_t>& aResult, size_t iVert, bool bIgnoreCreased)
{
	// Crawl along each edge to find adjacent faces.
	for (size_t iEdge = 0; iEdge < GetNumEdges(); iEdge++)
	{
		CConversionEdge* pEdge = m_pScene->GetMesh(m_iMesh)->GetEdge(GetEdge(iEdge));

		if (bIgnoreCreased && pEdge->m_bCreased)
			continue;

		if (iVert != ((size_t)~0) && !pEdge->HasVertex(iVert))
			continue;

		if (pEdge->f1 != ((size_t)~0))
		{
			std::vector<size_t>::iterator it = find(aResult.begin(), aResult.end(), pEdge->f1);
			if (it == aResult.end())
			{
				aResult.push_back(pEdge->f1);
				m_pScene->GetMesh(m_iMesh)->GetFace(pEdge->f1)->FindAdjacentFacesInternal(aResult, iVert, bIgnoreCreased);
			}
		}

		if (pEdge->f2 != ((size_t)~0))
		{
			std::vector<size_t>::iterator it = find(aResult.begin(), aResult.end(), pEdge->f2);
			if (it == aResult.end())
			{
				aResult.push_back(pEdge->f2);
				m_pScene->GetMesh(m_iMesh)->GetFace(pEdge->f2)->FindAdjacentFacesInternal(aResult, iVert, bIgnoreCreased);
			}
		}
	}
}

bool CConversionFace::HasEdge(size_t i)
{
	for (size_t iEdge = 0; iEdge < GetNumEdges(); iEdge++)
	{
		if (m_aEdges[iEdge] == i)
			return true;
	}

	return false;
}

size_t CConversionFace::FindVertex(size_t i)
{
	for (size_t iVertex = 0; iVertex < GetNumVertices(); iVertex++)
	{
		if (m_aVertices[iVertex].v == i)
			return iVertex;
	}

	return ~0;
}

std::vector<Vector>& CConversionFace::GetVertices(std::vector<Vector>& avecVertices)
{
	avecVertices.clear();

	for (size_t i = 0; i < m_aVertices.size(); i++)
		avecVertices.push_back(m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[i].v));

	return avecVertices;
}

size_t CConversionMesh::AddFace(size_t iMaterial)
{
	m_aFaces.push_back(CConversionFace(m_pScene, m_pScene->FindMesh(this), iMaterial));
	return m_aFaces.size()-1;
}

void CConversionMesh::AddVertexToFace(size_t iFace, size_t v, size_t vt, size_t vn)
{
	m_aaVertexFaceMap[v].push_back(iFace);
	m_aFaces[iFace].m_aVertices.push_back(CConversionVertex(v, vt, vn));
}

void CConversionMesh::AddEdgeToFace(size_t iFace, size_t iEdge)
{
	m_aFaces[iFace].m_aEdges.push_back(iEdge);
}

void CConversionMesh::RemoveFace(size_t iFace)
{
	m_aFaces.erase(m_aFaces.begin()+iFace);
}

size_t CConversionMesh::FindFace(CConversionFace* pFace)
{
	for (size_t i = 0; i < m_aFaces.size(); i++)
		if (&m_aFaces[i] == pFace)
			return i;

	return ((size_t)~0);
}

CConversionBone::CConversionBone(const wchar_t* pszName)
{
	wcscpy(m_szName, pszName);
}

CConversionMaterial::CConversionMaterial(const wchar_t* pszName, Vector vecAmbient, Vector vecDiffuse, Vector vecSpecular, Vector vecEmissive, float flTransparency, float flShininess)
{
	wcscpy(m_szName, pszName);

	m_vecAmbient = vecAmbient;
	m_vecDiffuse = vecDiffuse;
	m_vecSpecular = vecSpecular;
	m_vecEmissive = vecEmissive;
	m_flTransparency = flTransparency;
	m_flShininess = flShininess;
	m_eIllumType = ILLUM_FULL;

	m_szTexture[0] = '\0';
}

CConversionFace::CConversionFace(class CConversionScene* pScene, size_t iMesh, size_t M)
{
	m_pScene = pScene;
	m_iMesh = iMesh;
	m = M;
}

CConversionEdge::CConversionEdge(size_t V1, size_t V2, bool bCreased)
{
	v1 = V1;
	v2 = V2;
	m_bCreased = bCreased;

	f1 = ~0;
	f2 = ~0;
}

bool CConversionEdge::HasVertex(size_t i)
{
	if (v1 == i || v2 == i)
		return true;

	return false;
}

CConversionVertex::CConversionVertex(size_t V, size_t VT, size_t VN)
{
	v = V;
	vt = VT;
	vn = VN;
}
