#ifndef CF_CONVMESH_H
#define CF_CONVMESH_H

#include <vector>

#include "vector.h"

// These belong specifically to a face and are never shared.
class CConversionVertex
{
public:
									CConversionVertex(size_t v, size_t vt, size_t vn);

	size_t							v, vt, vn;

	std::vector<size_t>				m_aEdges;	// Index into parent's edge list
};

class CConversionFace
{
public:
									CConversionFace(class CConversionScene* pScene, size_t iMesh, size_t m);

	Vector							GetNormal();

	void							FindAdjacentFaces(std::vector<size_t>& aResult, size_t iVert = (size_t)~0, bool bIgnoreCreased = false);
	void							FindAdjacentFacesInternal(std::vector<size_t>& aResult, size_t iVert, bool bIgnoreCreased);

	size_t							GetNumVertices() { return m_aVertices.size(); }
	CConversionVertex*				GetVertex(size_t i) { return &m_aVertices[i]; }
	size_t							FindVertex(size_t i);

	size_t							GetNumEdges() { return m_aEdges.size(); }
	size_t							GetEdge(size_t i) { return m_aEdges[i]; }
	bool							HasEdge(size_t i);

	class CConversionScene*			m_pScene;
	size_t							m_iMesh;

	std::vector<CConversionVertex>	m_aVertices;
	std::vector<size_t>				m_aEdges;	// Index into parent's vertex edge list

	size_t							m;
};

// These are unique, so two faces may share the same one.
class CConversionEdge
{
public:
									CConversionEdge(size_t v1, size_t v2, bool bCreased = false);

	bool							HasVertex(size_t i);

	size_t							v1, v2;
	bool							m_bCreased;

	size_t							f1, f2;	// Index into parent's face list
};

class CConversionBone
{
public:
									CConversionBone(const wchar_t* pszName);

	wchar_t							m_szName[1024];
};

typedef enum
{
	ILLUM_NONE,
	ILLUM_NOSPEC,
	ILLUM_FULL
} IllumType_t;

class CConversionMaterial
{
public:
									CConversionMaterial(const wchar_t* pszName,
										Vector vecAmbient = Vector(0.2f, 0.2f, 0.2f),
										Vector vecDiffuse = Vector(0.8f, 0.8f, 0.8f),
										Vector vecSpecular = Vector(1.0f, 1.0f, 1.0f),
										Vector vecEmissive = Vector(0.0f, 0.0f, 0.0f),
										float flTransparency = 1.0f,
										float flShininess = 0);

	wchar_t*						GetName() { return m_szName; }
	wchar_t*						GetTexture() { return m_szTexture; }

	wchar_t							m_szName[1024];

	Vector							m_vecAmbient;
	Vector							m_vecDiffuse;
	Vector							m_vecSpecular;
	Vector							m_vecEmissive;
	float							m_flTransparency;
	float							m_flShininess;
	IllumType_t						m_eIllumType;

	wchar_t							m_szTexture[1024];
};

class CConversionMesh
{
public:
									CConversionMesh(class CConversionScene* pScene, const wchar_t* pszName);

	void							Clear();

	void							CalculateEdgeData();
	void							CalculateVertexNormals();
	void							TranslateOrigin();

	wchar_t*						GetName() { return m_szName; };

	size_t							AddVertex(float x, float y, float z);
	size_t							AddNormal(float x, float y, float z);
	size_t							AddUV(float u, float v);
	size_t							AddBone(const wchar_t* pszName);
	size_t							AddEdge(size_t v1, size_t v2);
	size_t							AddFace(size_t iMaterial);

	void							AddVertexToFace(size_t iFace, size_t v, size_t vt, size_t vn);
	void							AddEdgeToFace(size_t iFace, size_t iEdge);

	void							RemoveFace(size_t iFace);

	size_t							GetNumVertices() { return m_aVertices.size(); };
	Vector							GetVertex(size_t i) { return m_aVertices[i]; }
	size_t							GetNumNormals() { return m_aNormals.size(); };
	Vector							GetNormal(size_t i) { return m_aNormals[i]; }
	size_t							GetNumUVs() { return m_aUVs.size(); };
	Vector							GetUV(size_t i) { return m_aUVs[i]; }

	size_t							GetNumBones() { return m_aBones.size(); };
	wchar_t*						GetBoneName(size_t i) { return m_aBones[i].m_szName; };

	size_t							GetNumEdges() { return m_aEdges.size(); };
	CConversionEdge*				GetEdge(size_t i) { return &m_aEdges[i]; };

	size_t							GetNumFaces() { return m_aFaces.size(); };
	size_t							FindFace(CConversionFace* pFace);
	CConversionFace*				GetFace(size_t i) { return &m_aFaces[i]; };

	wchar_t							m_szName[1024];
	class CConversionScene*			m_pScene;

	// A vector of Vectors? Holy crap!
	std::vector<Vector>				m_aVertices;	
	std::vector<Vector>				m_aNormals;	
	std::vector<Vector>				m_aUVs;		// Really don't feel like making a 2d vector just for this.
	std::vector<CConversionBone>	m_aBones;
	std::vector<CConversionEdge>	m_aEdges;
	std::vector<CConversionFace>	m_aFaces;

	Vector							m_vecOrigin;
};

class CConversionScene
{
public:
	void								DestroyAll();

	size_t								AddMaterial(const wchar_t* pszName);
	size_t								AddMaterial(CConversionMaterial& oMaterial);
	size_t								GetNumMaterials() { return m_aMaterials.size(); };
	size_t								FindMaterial(const wchar_t* pszName);
	CConversionMaterial*				GetMaterial(size_t i) { if (i >= m_aMaterials.size()) return NULL; return &m_aMaterials[i]; };

	size_t								AddMesh(const wchar_t* pszName);
	size_t								GetNumMeshes() { return m_aMeshes.size(); };
	CConversionMesh*					GetMesh(size_t i) { return &m_aMeshes[i]; };
	size_t								FindMesh(const wchar_t* pszName);
	size_t								FindMesh(CConversionMesh* pMesh);

	std::vector<CConversionMesh>		m_aMeshes;
	std::vector<CConversionMaterial>	m_aMaterials;
};

#endif