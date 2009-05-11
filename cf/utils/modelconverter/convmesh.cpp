#include "convmesh.h"

void CConversionMesh::Clear()
{
	m_aVertices.clear();
	m_aNormals.clear();
	m_aUVs.clear();
	m_aBones.clear();
	m_aFaces.clear();
}

void CConversionMesh::AddVertex(float x, float y, float z)
{
	m_aVertices.push_back(Vector(x, y, z));
}

void CConversionMesh::AddNormal(float x, float y, float z)
{
	m_aNormals.push_back(Vector(x, y, z));
}

void CConversionMesh::AddUV(float u, float v)
{
	m_aUVs.push_back(Vector(u, v, 0));
}

void CConversionMesh::AddBone(char* pszName)
{
	m_aBones.push_back(CConversionBone(pszName));
}

size_t CConversionMesh::AddMaterial(char* pszName)
{
	m_aMaterials.push_back(CConversionMaterial(pszName));
	return m_aMaterials.size()-1;
}

size_t CConversionMesh::AddFace(size_t iMaterial)
{
	m_aFaces.push_back(CConversionFace(iMaterial));
	return m_aFaces.size()-1;
}

void CConversionMesh::AddVertexToFace(size_t iFace, size_t v, size_t vt, size_t vn)
{
	m_aFaces[iFace].m_aVertices.push_back(CConversionVertex(v, vt, vn));
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

CConversionBone::CConversionBone(char* pszName)
{
	strcpy(m_szName, pszName);
}

CConversionMaterial::CConversionMaterial(char* pszName)
{
	strcpy(m_szName, pszName);
}

CConversionFace::CConversionFace(size_t M)
{
	m = M;
}

CConversionVertex::CConversionVertex(size_t V, size_t VT, size_t VN)
{
	v = V;
	vt = VT;
	vn = VN;
}
