#ifndef CF_CONVMESH_H
#define CF_CONVMESH_H

#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/string.h>

#include <worklistener.h>
#include <vector.h>
#include <matrix.h>
#include <geometry.h>

// These belong specifically to a face and are never shared.
class CConversionVertex
{
public:
									CConversionVertex(class CConversionScene* pScene, size_t iMesh, size_t v, size_t vt, size_t vn);

public:
	class CConversionScene*			m_pScene;
	size_t							m_iMesh;

	size_t							v;	// Vertex
	size_t							vu;	// UV
	size_t							vn;	// Normal
	size_t							vt;	// Tangent
	size_t							vb;	// Bitangent

	eastl::vector<size_t>			m_aEdges;	// Index into parent's edge list
};

class CConversionFace
{
public:
									CConversionFace(class CConversionScene* pScene, size_t iMesh, size_t m);

	Vector							GetNormal();
	Vector							GetCenter();
	float							GetArea();
	float							GetUVArea();

	void							FindAdjacentFaces(eastl::vector<size_t>& aResult, size_t iVert = (size_t)~0, bool bIgnoreCreased = false);
	void							FindAdjacentFacesInternal(eastl::vector<size_t>& aResult, size_t iVert, bool bIgnoreCreased);

	size_t							GetNumVertices() { return m_aVertices.size(); }
	CConversionVertex*				GetVertex(size_t i) { return &m_aVertices[i]; }
	size_t							FindVertex(size_t i);

	size_t							GetNumEdges() { return m_aEdges.size(); }
	size_t							GetEdge(size_t i) { return m_aEdges[i]; }
	bool							HasEdge(size_t i);

	eastl::vector<Vector>&			GetVertices(eastl::vector<Vector>& avecVertices);

	Vector							GetBaseVector(Vector vecPoint, int iVector, class CConversionMeshInstance* pMeshInstance = NULL);
	Vector							GetTangent(Vector vecPoint, class CConversionMeshInstance* pMeshInstance = NULL);
	Vector							GetBitangent(Vector vecPoint, class CConversionMeshInstance* pMeshInstance = NULL);
	Vector							GetNormal(Vector vecPoint, class CConversionMeshInstance* pMeshInstance = NULL);

	class CConversionScene*			m_pScene;
	size_t							m_iMesh;

	eastl::vector<CConversionVertex>	m_aVertices;
	eastl::vector<size_t>			m_aEdges;	// Index into parent's vertex edge list

	size_t							m;

	size_t							m_iSmoothingGroup;
};

// These are unique, so two faces may share the same one.
class CConversionEdge
{
public:
									CConversionEdge(size_t v1, size_t v2, bool bCreased = false);

	bool							HasVertex(size_t i);

	size_t							v1, v2;
	bool							m_bCreased;

	eastl::vector<size_t>				m_aiFaces;	// Index into parent's face list
};

class CConversionBone
{
public:
									CConversionBone(const eastl::string16& sName);

	eastl::string16					m_sName;
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
									CConversionMaterial(const eastl::string16& sName,
										Vector vecAmbient = Vector(0.2f, 0.2f, 0.2f),
										Vector vecDiffuse = Vector(0.8f, 0.8f, 0.8f),
										Vector vecSpecular = Vector(0.0f, 0.0f, 0.0f),
										Vector vecEmissive = Vector(0.0f, 0.0f, 0.0f),
										float flTransparency = 1.0f,
										float flShininess = 10.0f);

	eastl::string16					GetName() const { return m_sName; }

	eastl::string16					GetDiffuseTexture() const { return m_sDiffuseTexture; }
	eastl::string16					GetNormalTexture() const { return m_sNormalTexture; }
	eastl::string16					GetAOTexture() const { return m_sAOTexture; }
	eastl::string16					GetCAOTexture() const { return m_sCAOTexture; }

	void							SetVisible(bool bVisible) { m_bVisible = bVisible; };
	bool							IsVisible() { return m_bVisible; };

public:
	eastl::string16					m_sName;

	Vector							m_vecAmbient;
	Vector							m_vecDiffuse;
	Vector							m_vecSpecular;
	Vector							m_vecEmissive;
	float							m_flTransparency;
	float							m_flShininess;
	IllumType_t						m_eIllumType;

	eastl::string16					m_sDiffuseTexture;
	eastl::string16					m_sNormalTexture;
	eastl::string16					m_sAOTexture;
	eastl::string16					m_sCAOTexture;

	bool							m_bVisible;
};

// Each mesh has a material stub table, and the scene tree matches them up to actual materials
class CConversionMaterialStub
{
public:
									CConversionMaterialStub(const eastl::string16& sName);

public:
	eastl::string16					GetName() const { return m_sName; }

	eastl::string16					m_sName;
};

class CConversionMesh
{
public:
									CConversionMesh(class CConversionScene* pScene, const eastl::string16& sName);

	void							Clear();

	void							CalculateEdgeData();
	void							CalculateVertexNormals();
	void							CalculateExtends();
	void							CalculateVertexTangents();

	eastl::string16					GetName() { return m_sName; };

	size_t							AddVertex(float x, float y, float z);
	size_t							AddNormal(float x, float y, float z);
	size_t							AddTangent(float x, float y, float z);
	size_t							AddBitangent(float x, float y, float z);
	size_t							AddUV(float u, float v);
	size_t							AddBone(const eastl::string16& sName);
	size_t							AddEdge(size_t v1, size_t v2);
	size_t							AddFace(size_t iMaterial);

	void							AddVertexToFace(size_t iFace, size_t v, size_t vt, size_t vn);
	void							AddEdgeToFace(size_t iFace, size_t iEdge);

	void							RemoveFace(size_t iFace);

	size_t							GetNumVertices() { return m_aVertices.size(); };
	Vector							GetVertex(size_t i) { if (i >= m_aVertices.size()) return Vector(0,0,0); return m_aVertices[i]; }
	size_t							GetNumNormals() { return m_aNormals.size(); };
	Vector							GetNormal(size_t i) { if (i >= m_aNormals.size()) return Vector(1,0,0); return m_aNormals[i]; }
	size_t							GetNumTangents() { return m_aTangents.size(); };
	Vector							GetTangent(size_t i) { if (i >= m_aTangents.size()) return Vector(0,1,0); return m_aTangents[i]; }
	size_t							GetNumBitangents() { return m_aBitangents.size(); };
	Vector							GetBitangent(size_t i) { if (i >= m_aBitangents.size()) return Vector(0,0,1); return m_aBitangents[i]; }
	size_t							GetNumUVs() { return m_aUVs.size(); };
	Vector							GetUV(size_t i) { if (!GetNumUVs()) return Vector(0,0,0); return m_aUVs[i]; }
	Vector							GetBaseVector(int iVector, CConversionVertex* pVertex);

	size_t							GetNumBones() { return m_aBones.size(); };
	eastl::string16					GetBoneName(size_t i) { return m_aBones[i].m_sName; };

	size_t							GetNumEdges() { return m_aEdges.size(); };
	CConversionEdge*				GetEdge(size_t i) { return &m_aEdges[i]; };

	size_t							GetNumFaces() { return m_aFaces.size(); };
	size_t							FindFace(CConversionFace* pFace);
	CConversionFace*				GetFace(size_t i) { return &m_aFaces[i]; };

	size_t							AddMaterialStub(const eastl::string16& sName = L"");
	size_t							GetNumMaterialStubs() { return m_aMaterialStubs.size(); };
	size_t							FindMaterialStub(const eastl::string16& sName);
	CConversionMaterialStub*		GetMaterialStub(size_t i) { return &m_aMaterialStubs[i]; };

	void							SetVisible(bool bVisible) { m_bVisible = bVisible; };
	bool							IsVisible() { return m_bVisible; };

public:
	eastl::string16					m_sName;
	class CConversionScene*			m_pScene;

	// A vector of Vectors? Holy crap!
	eastl::vector<Vector>			m_aVertices;
	eastl::vector<Vector>			m_aNormals;
	eastl::vector<Vector>			m_aTangents;
	eastl::vector<Vector>			m_aBitangents;	// Binormals can kiss my ass.
	eastl::vector<Vector>			m_aUVs;		// Really don't feel like making a 2d vector just for this.
	eastl::vector<CConversionBone>	m_aBones;
	eastl::vector<CConversionEdge>	m_aEdges;
	eastl::vector<CConversionFace>	m_aFaces;

	eastl::vector<eastl::vector<size_t> >	m_aaVertexFaceMap;

	eastl::vector<CConversionMaterialStub>	m_aMaterialStubs;

	AABB							m_oExtends;

	bool							m_bVisible;
};

class CConversionMaterialMap
{
public:
									CConversionMaterialMap();
									CConversionMaterialMap(size_t iStub, size_t iMaterial);

public:
	void							SetVisible(bool bVisible) { m_bVisible = bVisible; };
	bool							IsVisible() { return m_bVisible; };

public:
	size_t							m_iStub;
	size_t							m_iMaterial;

	bool							m_bVisible;
};

class CConversionMeshInstance
{
public:
									CConversionMeshInstance(CConversionScene* pScene, class CConversionSceneNode* pParent, size_t iMesh);

public:
	CConversionMesh*				GetMesh();

	void							AddMappedMaterial(size_t s, size_t m);
	CConversionMaterialMap*			GetMappedMaterial(size_t m);

	Vector							GetVertex(size_t i);
	Vector							GetNormal(size_t i);
	Vector							GetTangent(size_t i);
	Vector							GetBitangent(size_t i);
	Vector							GetBaseVector(int iVector, CConversionVertex* pVertex);

	void							SetVisible(bool bVisible) { m_bVisible = bVisible; };
	bool							IsVisible() { return m_bVisible; };

public:
	size_t							m_iMesh;	// Index into CConversionScene::m_aMeshes

	eastl::map<size_t, CConversionMaterialMap>	m_aiMaterialsMap;	// Maps CConversionMesh::m_aMaterialStubs to CConversionScene::m_aMaterials

	class CConversionScene*			m_pScene;
	class CConversionSceneNode*		m_pParent;

	bool							m_bVisible;
};

// A node in the scene tree.
class CConversionSceneNode
{
public:
										CConversionSceneNode(const eastl::string16& sName, CConversionScene* pScene, CConversionSceneNode* pParent);
										~CConversionSceneNode();

public:
	void								CalculateExtends();

	Matrix4x4							GetRootTransformations();

	bool								IsEmpty();

	size_t								AddChild(const eastl::string16& sName);
	size_t								GetNumChildren() { return m_apChildren.size(); };
	CConversionSceneNode*				GetChild(size_t i) { return m_apChildren[i]; };

	size_t								AddMeshInstance(size_t iMesh);
	size_t								GetNumMeshInstances() { return m_aMeshInstances.size(); };
	size_t								FindMeshInstance(CConversionMesh* pMesh);
	CConversionMeshInstance*			GetMeshInstance(size_t i) { return &m_aMeshInstances[i]; };

	eastl::string16						GetName() { return m_sName; }

	void								SetVisible(bool bVisible) { m_bVisible = bVisible; };
	bool								IsVisible() { return m_bVisible; };

public:
	eastl::string16						m_sName;
	CConversionScene*					m_pScene;
	CConversionSceneNode*				m_pParent;

	eastl::vector<CConversionSceneNode*>	m_apChildren;

	eastl::vector<CConversionMeshInstance>	m_aMeshInstances;

	Matrix4x4							m_mTransformations;

	AABB								m_oExtends;

	bool								m_bVisible;
};

class CConversionScene
{
public:
										CConversionScene();
										~CConversionScene();

public:
	void								DestroyAll();

	void								CalculateExtends();

	size_t								AddMaterial(const eastl::string16& sName);
	size_t								AddMaterial(CConversionMaterial& oMaterial);
	size_t								GetNumMaterials() { return m_aMaterials.size(); };
	size_t								FindMaterial(const eastl::string16& sName);
	CConversionMaterial*				GetMaterial(size_t i) { if (i >= m_aMaterials.size()) return NULL; return &m_aMaterials[i]; };

	size_t								AddMesh(const eastl::string16& sName);
	size_t								GetNumMeshes() { return m_aMeshes.size(); };
	CConversionMesh*					GetMesh(size_t i) { return &m_aMeshes[i]; };
	size_t								FindMesh(const eastl::string16& sName);
	size_t								FindMesh(CConversionMesh* pMesh);

	size_t								AddScene(const eastl::string16& sName);
	size_t								GetNumScenes() { return m_apScenes.size(); };
	CConversionSceneNode*				GetScene(size_t i) { if (i >= m_apScenes.size()) return NULL; return m_apScenes[i]; };

	// For model formats that don't have the concept of scenes, this is a node that contains the one and only mesh instance for this mesh.
	// It always returns a node and always the same one.
	CConversionSceneNode*				GetDefaultSceneMeshInstance(CConversionSceneNode* pScene, CConversionMesh* pMesh);

	// For model formats that don't have the concept of scenes,
	// this will add a material to the mesh and map it properly in the scene.
	size_t								AddDefaultSceneMaterial(CConversionSceneNode* pScene, CConversionMesh* pMesh, const eastl::string16& sName);

	bool								DoesFaceHaveValidMaterial(CConversionFace* pFace, class CConversionMeshInstance* pMeshInstance = NULL);

	void								SetWorkListener(IWorkListener* pWorkListener) { m_pWorkListener = pWorkListener; }

	eastl::vector<CConversionMesh>		m_aMeshes;
	eastl::vector<CConversionMaterial>	m_aMaterials;

	eastl::vector<CConversionSceneNode*>	m_apScenes;

	AABB								m_oExtends;

	IWorkListener*						m_pWorkListener;
};

#endif