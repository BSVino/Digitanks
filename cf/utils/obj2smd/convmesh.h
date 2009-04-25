#ifndef CF_CONVMESH_H
#define CF_CONVMESH_H

#include <vector>

#include "vector.h"

class CConversionVertex
{
public:
									CConversionVertex(size_t v, size_t vt, size_t vn);

	size_t							v, vt, vn;
};

class CConversionFace
{
public:
									CConversionFace(size_t m);

	size_t							GetNumVertices() { return m_aVertices.size(); }
	CConversionVertex*				GetVertex(size_t i) { return &m_aVertices[i]; }

	std::vector<CConversionVertex>	m_aVertices;

	size_t							m;
};

class CConversionBone
{
public:
									CConversionBone(char* pszName);

	char							m_szName[1024];
};

class CConversionMaterial
{
public:
									CConversionMaterial(char* pszName);

	char*							GetName() { return m_szName; }

	char							m_szName[1024];
};

class CConversionMesh
{
public:
	void							Clear();

	void							AddVertex(float x, float y, float z);
	void							AddNormal(float x, float y, float z);
	void							AddUV(float u, float v);
	void							AddBone(char* pszName);
	size_t							AddMaterial(char* pszName);
	size_t							AddFace(size_t iMaterial);

	void							AddVertexToFace(size_t iFace, size_t v, size_t vt, size_t vn);

	void							RemoveFace(size_t iFace);

	size_t							GetNumVertices() { return m_aVertices.size(); };
	Vector							GetVertex(size_t i) { return m_aVertices[i]; }
	size_t							GetNumNormals() { return m_aNormals.size(); };
	Vector							GetNormal(size_t i) { return m_aNormals[i]; }
	size_t							GetNumUVs() { return m_aUVs.size(); };
	Vector							GetUV(size_t i) { return m_aUVs[i]; }

	size_t							GetNumBones() { return m_aBones.size(); };
	char*							GetBoneName(size_t i) { return m_aBones[i].m_szName; };

	size_t							GetNumMaterials() { return m_aMaterials.size(); };
	size_t							FindMaterial(const char* pszName);
	CConversionMaterial*			GetMaterial(size_t i) { return &m_aMaterials[i]; };

	size_t							GetNumFaces() { return m_aFaces.size(); };
	CConversionFace*				GetFace(size_t i) { return &m_aFaces[i]; };

	// A vector of Vectors? Holy crap!
	std::vector<Vector>				m_aVertices;	
	std::vector<Vector>				m_aNormals;	
	std::vector<Vector>				m_aUVs;		// Really don't feel like making a 2d vector just for this.
	std::vector<CConversionBone>	m_aBones;
	std::vector<CConversionMaterial>m_aMaterials;
	std::vector<CConversionFace>	m_aFaces;
};

#endif