#ifndef LW_RAYTRACE_H
#define LW_RAYTRACE_H

#include <modelconverter/convmesh.h>
#include <geometry.h>

namespace raytrace {

class CTraceResult
{
public:
	Vector						m_vecHit;
	CConversionFace*			m_pFace;
	CConversionMeshInstance*	m_pMeshInstance;
};

class CKDTri
{
public:
								CKDTri();
								CKDTri(Vector v1, Vector v2, Vector v3, CConversionFace* pFace, CConversionMeshInstance* pMeshInstance = NULL);

public:
	Vector						v[3];

	CConversionFace*			m_pFace;
	CConversionMeshInstance*	m_pMeshInstance;
};

class CKDNode
{
public:
								CKDNode(CKDNode* pParent = NULL, AABB oBounds = AABB(), class CKDTree* pTree = NULL);
								~CKDNode();

public:
	// Reserves memory for triangles all at once, for faster allocation
	void						ReserveTriangles(size_t iEstimatedTriangles);
	void						AddTriangle(Vector v1, Vector v2, Vector v3, CConversionFace* pFace, CConversionMeshInstance* pMeshInstance = NULL);

	void						RemoveArea(const AABB& oBox);

	void						CalcBounds();
	void						BuildTriList();
	void						PassTriList();
	void						Build();

	bool						Raytrace(const Ray& rayTrace, CTraceResult* pTR = NULL);
	bool						Raytrace(const Vector& vecStart, const Vector& vecEnd, CTraceResult* pTR = NULL);
	float						Closest(const Vector& vecPoint);

	const CKDNode*				GetLeftChild() const { return m_pLeft; };
	const CKDNode*				GetRightChild() const { return m_pRight; };
	AABB						GetBounds() const { return m_oBounds; };
	size_t						GetSplitAxis() const { return m_iSplitAxis; };
	float						GetSplitPos() const { return m_flSplitPos; };

protected:
	CKDNode*					m_pParent;

	CKDNode*					m_pLeft;
	CKDNode*					m_pRight;

	CKDTree*					m_pTree;

	size_t						m_iDepth;

	eastl::vector<CKDTri>		m_aTris;
	size_t						m_iTriangles;	// This node and all child nodes

	AABB						m_oBounds;

	size_t						m_iSplitAxis;
	float						m_flSplitPos;
};

class CKDTree
{
public:
								CKDTree();
								~CKDTree();

public:
	// Reserves memory for triangles all at once, for faster allocation
	void						ReserveTriangles(size_t iEstimatedTriangles);
	void						AddTriangle(Vector v1, Vector v2, Vector v3, CConversionFace* pFace = NULL, CConversionMeshInstance* pMeshInstance = NULL);

	void						RemoveArea(const AABB& oBox);

	void						BuildTree();

	bool						Raytrace(const Ray& rayTrace, CTraceResult* pTR = NULL);
	bool						Raytrace(const Vector& vecStart, const Vector& vecEnd, CTraceResult* pTR = NULL);
	float						Closest(const Vector& vecPoint);

	const CKDNode*				GetTopNode() const { return m_pTop; };

	bool						IsBuilt() { return m_bBuilt; };

protected:
	CKDNode*					m_pTop;

	bool						m_bBuilt;
};

class CRaytracer
{
public:
								CRaytracer(CConversionScene* pScene = NULL);
								~CRaytracer();

public:
	bool						Raytrace(const Ray& rayTrace, CTraceResult* pTR = NULL);
	bool						Raytrace(const Vector& vecStart, const Vector& vecEnd, CTraceResult* pTR = NULL);
	bool						RaytraceBruteForce(const Ray& rayTrace, CTraceResult* pTR = NULL);

	float						Closest(const Vector& vecPoint);

	void						AddMeshesFromNode(CConversionSceneNode* pNode);
	void						AddMeshInstance(CConversionMeshInstance* pMeshInstance);
	void						AddTriangle(Vector v1, Vector v2, Vector v3);
	void						BuildTree();

	void						RemoveArea(const AABB& oBox);

	const CKDTree*				GetTree() const { return m_pTree; };

protected:
	CConversionScene*			m_pScene;

	CKDTree*					m_pTree;
};

};

#endif