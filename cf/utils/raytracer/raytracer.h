#ifndef CF_RAYTRACE_H
#define CF_RAYTRACE_H

#include <modelconverter/convmesh.h>
#include <geometry.h>

namespace raytrace {

class CKDTri
{
public:
								CKDTri(Vector v1, Vector v2, Vector v3);

	Vector						v[3];
};

class CKDNode
{
public:
								CKDNode(CKDNode* pParent = NULL, AABB oBounds = AABB());
								~CKDNode();

	void						AddTriangle(Vector v1, Vector v2, Vector v3);

	void						CalcBounds();
	void						BuildTriList();
	void						Build();

	bool						Raytrace(const Ray& rayTrace, Vector* pvecHit = NULL);

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

	void						AddTriangle(Vector v1, Vector v2, Vector v3);

	void						BuildTree();

	bool						Raytrace(const Ray& rayTrace, Vector* pvecHit = NULL);

	const CKDNode*				GetTopNode() const { return m_pTop; };

protected:
	CKDNode*					m_pTop;
};

class CRaytracer
{
public:
								CRaytracer(CConversionScene* pScene);

	bool						Raytrace(const Ray& rayTrace, Vector* pvecHit = NULL);
	bool						RaytraceBruteForce(const Ray& rayTrace, Vector* pvecHit = NULL);

	void						BuildTree();
	void						AddMeshesFromNode(CConversionSceneNode* pNode);

	const CKDTree*				GetTree() const { return m_pTree; };

protected:
	CConversionScene*			m_pScene;

	CKDTree*					m_pTree;
};

};

#endif