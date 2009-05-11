#include <assert.h>
#include <algorithm>

#include "convmesh.h"

void CConversionMesh::Clear()
{
	m_aVertices.clear();
	m_aNormals.clear();
	m_aUVs.clear();
	m_aBones.clear();
	m_aFaces.clear();
	m_aEdges.clear();
	m_aMaterials.clear();

	m_vecOrigin = Vector();
}

void CConversionMesh::CalculateEdgeData()
{
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
	}

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
	}
}

void CConversionMesh::CalculateVertexNormals()
{
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
			for (size_t iNormalFace = 0; iNormalFace < aNormalFaces.size(); iNormalFace++)
				vecNormal += GetFace(aNormalFaces[iNormalFace])->GetNormal();

			vecNormal /= (float)aNormalFaces.size();
			vecNormal.Normalize();

			m_aNormals[pVertex->vn] = vecNormal;
		}
	}
}

void CConversionMesh::TranslateOrigin()
{
	if (m_vecOrigin.LengthSqr() == 0)
		return;

	// Translate each vertex to the axis, essentially centering it around Silo's manipulator.
	for (size_t iVertex = 0; iVertex < GetNumVertices(); iVertex++)
		m_aVertices[iVertex] -= m_vecOrigin;

	m_vecOrigin = Vector(0,0,0);
}

size_t CConversionMesh::AddVertex(float x, float y, float z)
{
	m_aVertices.push_back(Vector(x, y, z));
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

size_t CConversionMesh::AddBone(const char* pszName)
{
	m_aBones.push_back(CConversionBone(pszName));
	return m_aBones.size()-1;
}

size_t CConversionMesh::AddMaterial(const char* pszName)
{
	m_aMaterials.push_back(CConversionMaterial(pszName));
	return m_aMaterials.size()-1;
}

size_t CConversionMesh::AddEdge(size_t v1, size_t v2)
{
	m_aEdges.push_back(CConversionEdge(v1, v2));
	return m_aEdges.size()-1;
}

Vector CConversionFace::GetNormal()
{
	assert(GetNumVertices() >= 3);

	Vector v1 = m_pMesh->GetVertex(m_aVertices[0].v);
	Vector v2 = m_pMesh->GetVertex(m_aVertices[1].v);
	Vector v3 = m_pMesh->GetVertex(m_aVertices[2].v);

	return (v3 - v2).Normalized().Cross((v1 - v2).Normalized());
}

void CConversionFace::FindAdjacentFaces(std::vector<size_t>& aResult, size_t iVert, bool bIgnoreCreased)
{
	aResult.push_back(m_pMesh->FindFace(this));
	FindAdjacentFacesInternal(aResult, iVert, bIgnoreCreased);
}

void CConversionFace::FindAdjacentFacesInternal(std::vector<size_t>& aResult, size_t iVert, bool bIgnoreCreased)
{
	// Crawl along each edge to find adjacent faces.
	for (size_t iEdge = 0; iEdge < GetNumEdges(); iEdge++)
	{
		CConversionEdge* pEdge = m_pMesh->GetEdge(GetEdge(iEdge));

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
				m_pMesh->GetFace(pEdge->f1)->FindAdjacentFacesInternal(aResult, iVert, bIgnoreCreased);
			}
		}

		if (pEdge->f2 != ((size_t)~0))
		{
			std::vector<size_t>::iterator it = find(aResult.begin(), aResult.end(), pEdge->f2);
			if (it == aResult.end())
			{
				aResult.push_back(pEdge->f2);
				m_pMesh->GetFace(pEdge->f2)->FindAdjacentFacesInternal(aResult, iVert, bIgnoreCreased);
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

size_t CConversionMesh::AddFace(size_t iMaterial)
{
	m_aFaces.push_back(CConversionFace(this, iMaterial));
	return m_aFaces.size()-1;
}

void CConversionMesh::AddVertexToFace(size_t iFace, size_t v, size_t vt, size_t vn)
{
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

size_t CConversionMesh::FindMaterial(const char* pszName)
{
	for (size_t i = 0; i < m_aMaterials.size(); i++)
		if (strcmp(pszName, m_aMaterials[i].m_szName) == 0)
			return i;

	return ((size_t)~0);
}

size_t CConversionMesh::FindFace(CConversionFace* pFace)
{
	for (size_t i = 0; i < m_aFaces.size(); i++)
		if (&m_aFaces[i] == pFace)
			return i;

	return ((size_t)~0);
}

CConversionBone::CConversionBone(const char* pszName)
{
	strcpy(m_szName, pszName);
}

CConversionMaterial::CConversionMaterial(const char* pszName)
{
	strcpy(m_szName, pszName);
}

CConversionFace::CConversionFace(class CConversionMesh* pMesh, size_t M)
{
	m_pMesh = pMesh;
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
