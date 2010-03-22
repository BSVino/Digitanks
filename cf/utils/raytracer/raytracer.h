#ifndef CF_RAYTRACE_H
#define CF_RAYTRACE_H

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
								CKDTri(Vector v1, Vector v2, Vector v3, CConversionFace* pFace, CConversionMeshInstance* pMeshInstance = NULL);

	Vector						v[3];

	CConversionFace*			m_pFace;
	CConversionMeshInstance*	m_pMeshInstance;
};

class CKDNode
{
public:
								CKDNode(CKDNode* pParent = NULL, AABB oBounds = AABB());
								~CKDNode();

	void						AddTriangle(Vector v1, Vector v2, Vector v3, CConversionFace* pFace, CConversionMeshInstance* pMeshInstance = NULL);

	void						CalcBounds();
	void						BuildTriList();
	void						Build();

	bool						Raytrace(const Ray& rayTrace, CTraceResult* pTR = NULL);
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

	size_t						m_iDepth;

	std::vector<CKDTri>			m_aTris;

	AABB						m_oBounds;

	size_t						m_iSplitAxis;
	float						m_flSplitPos;
};

class CKDTree
{
public:
								CKDTree();
								~CKDTree();

	void						AddTriangle(Vector v1, Vector v2, Vector v3, CConversionFace* pFace, CConversionMeshInstance* pMeshInstance = NULL);

	void						BuildTree();

	bool						Raytrace(const Ray& rayTrace, CTraceResult* pTR = NULL);
	float						Closest(const Vector& vecPoint);

	const CKDNode*				GetTopNode() const { return m_pTop; };

protected:
	CKDNode*					m_pTop;

	bool						m_bBuilt;
};

class CRaytracer
{
public:
								CRaytracer(CConversionScene* pScene);
								~CRaytracer();

public:
	bool						Raytrace(const Ray& rayTrace, CTraceResult* pTR = NULL);
	bool						RaytraceBruteForce(const Ray& rayTrace, CTraceResult* pTR = NULL);

	float						Closest(const Vector& vecPoint);

	void						AddMeshesFromNode(CConversionSceneNode* pNode);
	void						AddMeshInstance(CConversionMeshInstance* pMeshInstance);
	void						BuildTree();

	const CKDTree*				GetTree() const { return m_pTree; };

protected:
	CConversionScene*			m_pScene;

	CKDTree*					m_pTree;
};

};

#endif