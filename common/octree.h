#ifndef LW_OCTREE_H
#define LW_OCTREE_H

#include <EASTL/vector.h>

#include <vector.h>
#include <geometry.h>

//#define DEBUG_WITH_GL
#ifdef DEBUG_WITH_GL
#include <../../glew/include/GL/glew.h>

void DrawBox(const TemplateAABB<double>& b, const Color& c);
void DrawTri(const TemplateVector<double>& v1, const TemplateVector<double>& v2, const TemplateVector<double>& v3, float r, float g, float b, int iLineWidth);

template <class T, class F>
class CQuadTreeBranch;
class CBranchData;
#endif

template <class T, typename F> class COctree;
template <class T, typename F> class COctreeBranch;

template <class T, typename F>
class COctreeObject
{
public:
	COctreeObject(const T& oData, const TemplateAABB<F>& oSize)
	{
		m_oData = oData;
		m_oSize = oSize;
	}

public:
	T				m_oData;
	TemplateAABB<F>	m_oSize;
};

template <class T, typename F=float>
class COctreeBranch
{
public:
	COctreeBranch(COctreeBranch<T, F>* pParent, COctree<T, F>* pTree, const TemplateAABB<F>& oBounds, unsigned short iDepth, unsigned char iParentIndex)
	{
		m_pParent = pParent;
		m_pTree = pTree;

		m_pBranches[0] = NULL;
		m_pBranches[1] = NULL;
		m_pBranches[2] = NULL;
		m_pBranches[3] = NULL;
		m_pBranches[4] = NULL;
		m_pBranches[5] = NULL;
		m_pBranches[6] = NULL;
		m_pBranches[7] = NULL;

		m_oBounds = oBounds;

		m_iDepth = iDepth;
		m_iParentIndex = iParentIndex;

		m_bCenterCalculated = false;

		m_aObjects.reserve(m_pTree->GetMaxObjects());
	}

	~COctreeBranch()
	{
		for (size_t i = 0; i < 8; i++)
		{
			if (m_pBranches[i])
				delete m_pBranches[i];
		}
	}

public:
	void						TryPush();
	void						TryPull();

	void						RemoveObject(const T& oObject);

	bool						Raytrace(const TemplateVector<F>& vecStart, const TemplateVector<F>& vecEnd, CCollisionResult& tr);

	TemplateVector<F>			GetCenter();

	void						CheckBranchSanity();

public:
	COctreeBranch<T, F>*		m_pParent;
	COctree<T, F>*				m_pTree;

	TemplateAABB<F>				m_oBounds;

	unsigned short				m_iDepth;
	unsigned char				m_iParentIndex;

	union
	{
		struct
		{
			COctreeBranch<T, F>*	m_pBranchxyz;
			COctreeBranch<T, F>*	m_pBranchXyz;
			COctreeBranch<T, F>*	m_pBranchxYz;
			COctreeBranch<T, F>*	m_pBranchXYz;
			COctreeBranch<T, F>*	m_pBranchxyZ;
			COctreeBranch<T, F>*	m_pBranchXyZ;
			COctreeBranch<T, F>*	m_pBranchxYZ;
			COctreeBranch<T, F>*	m_pBranchXYZ;
		};
		COctreeBranch<T, F>*		m_pBranches[8];
	};

	T							m_oData;

	bool						m_bCenterCalculated;
	TemplateVector<F>			m_vecCenter;

	tvector<COctreeObject<T, F> >	m_aObjects;
};

template <class T, typename F=float>
class COctree
{
public:
	COctree(F flSize)
	{
		m_flSize = flSize;
		m_pOctreeHead = NULL;

		m_iMaxObjects = 8;
		m_iMaxDepth = 11;
	};

	~COctree()
	{
		if (m_pOctreeHead)
			delete m_pOctreeHead;
	}
	
public:
	void						Init(F flSize);

	void						AddObject(const T& oData, const TemplateAABB<F>& oSize);
	void						RemoveObject(const T& oData);

	class COctreeBranch<T, F>*	FindLeaf(const TemplateAABB<F>& oArea);
	class COctreeBranch<T, F>*	FindLeaf(const TemplateVector<F>& vecPoint);

	bool						Raytrace(const TemplateVector<F>& vecStart, const TemplateVector<F>& vecEnd, CCollisionResult& tr, COctreeBranch<T, F>* pBranch = NULL);

	// Trying to resist a snarky joke with the name of this method.
	const COctreeBranch<T, F>*	GetHead() { return m_pOctreeHead; }

	size_t						GetMaxObjects() { return m_iMaxObjects; };
	size_t						GetMaxDepth() { return m_iMaxDepth; };

	tmap<T, tvector<COctreeBranch<T, F>*> >&	GetObjectBranches() { return m_aapObjectBranches; }

protected:
	F							m_flSize;
	class COctreeBranch<T, F>*	m_pOctreeHead;

	size_t						m_iMaxObjects;
	size_t						m_iMaxDepth;

	tmap<T, tvector<COctreeBranch<T, F>*> >	m_aapObjectBranches;
};

template <class T, typename F>
void COctree<T, F>::Init(F flSize)
{
	m_flSize = flSize;
	m_pOctreeHead = new COctreeBranch<T, F>(NULL, this, TemplateAABB<F>(TemplateVector<F>(-flSize, -flSize, -flSize), TemplateVector<F>(flSize, flSize, flSize)), 0, 0);
}

template <class T, typename F>
void COctree<T, F>::AddObject(const T& oData, const TemplateAABB<F>& oSize)
{
	if (!m_pOctreeHead)
		Init(m_flSize);

	COctreeBranch<T, F>* pBranch = FindLeaf(oSize);

	pBranch->m_aObjects.push_back(COctreeObject<T, F>(oData, oSize));
	m_aapObjectBranches[oData].push_back(pBranch);

	pBranch->TryPush();

	//pBranch->CheckBranchSanity();
}

template <class T, typename F>
void COctree<T, F>::RemoveObject(const T& oData)
{
	if (!m_pOctreeHead)
		return;

	tvector<COctreeBranch<T, F>*> apBranches = m_aapObjectBranches[oData];
	size_t iSize = apBranches.size();
	for (size_t i = 0; i < iSize; i++)
	{
		COctreeBranch<T, F>* pBranch = apBranches[i];
		pBranch->RemoveObject(oData);
	}

	m_aapObjectBranches.erase(oData);
}

template <class T, typename F>
COctreeBranch<T, F>* COctree<T, F>::FindLeaf(const TemplateAABB<F>& oArea)
{
	if (!m_pOctreeHead)
		return NULL;

	if (oArea.m_vecMins.x < -m_flSize)
		return NULL;

	if (oArea.m_vecMins.y < -m_flSize)
		return NULL;

	if (oArea.m_vecMins.z < -m_flSize)
		return NULL;

	if (oArea.m_vecMaxs.x > m_flSize)
		return NULL;

	if (oArea.m_vecMaxs.y > m_flSize)
		return NULL;

	if (oArea.m_vecMaxs.z > m_flSize)
		return NULL;

	COctreeBranch<T, F>* pCurrent = m_pOctreeHead;
	while (pCurrent->m_pBranches[0])
	{
		bool bFound = false;
		for (size_t i = 0; i < 8; i++)
		{
			if (oArea.m_vecMins.x < pCurrent->m_pBranches[i]->m_oBounds.m_vecMins.x)
				continue;

			if (oArea.m_vecMins.y < pCurrent->m_pBranches[i]->m_oBounds.m_vecMins.y)
				continue;

			if (oArea.m_vecMins.z < pCurrent->m_pBranches[i]->m_oBounds.m_vecMins.z)
				continue;

			if (oArea.m_vecMaxs.x > pCurrent->m_pBranches[i]->m_oBounds.m_vecMaxs.x)
				continue;

			if (oArea.m_vecMaxs.y > pCurrent->m_pBranches[i]->m_oBounds.m_vecMaxs.y)
				continue;

			if (oArea.m_vecMaxs.z > pCurrent->m_pBranches[i]->m_oBounds.m_vecMaxs.z)
				continue;

			pCurrent = pCurrent->m_pBranches[i];
			bFound = true;
			break;
		}

		if (!bFound)
			return pCurrent;
	}

	return pCurrent;
}

template <class T, typename F>
COctreeBranch<T, F>* COctree<T, F>::FindLeaf(const TemplateVector<F>& vecPoint)
{
	if (!m_pOctreeHead)
		return NULL;

	if (vecPoint.x < -m_flSize)
		return NULL;

	if (vecPoint.y < -m_flSize)
		return NULL;

	if (vecPoint.z < -m_flSize)
		return NULL;

	if (vecPoint.x > m_flSize)
		return NULL;

	if (vecPoint.y > m_flSize)
		return NULL;

	if (vecPoint.z > m_flSize)
		return NULL;

	COctreeBranch<T, F>* pCurrent = m_pOctreeHead;
	while (pCurrent->m_pBranches[0])
	{
		for (size_t i = 0; i < 8; i++)
		{
			if (vecPoint.x < pCurrent->m_pBranches[i]->m_oBounds.m_vecMin.x)
				continue;

			if (vecPoint.y < pCurrent->m_pBranches[i]->m_oBounds.m_vecMin.y)
				continue;

			if (vecPoint.z < pCurrent->m_pBranches[i]->m_oBounds.m_vecMin.z)
				continue;

			if (vecPoint.x > pCurrent->m_pBranches[i]->m_oBounds.m_vecMax.x)
				continue;

			if (vecPoint.y > pCurrent->m_pBranches[i]->m_oBounds.m_vecMax.y)
				continue;

			if (vecPoint.z > pCurrent->m_pBranches[i]->m_oBounds.m_vecMax.z)
				continue;

			pCurrent = pCurrent->m_pBranches[i];
			break;
		}
	}

	return pCurrent;
}

template <class T, typename F>
bool COctree<T, F>::Raytrace(const TemplateVector<F>& vecStart, const TemplateVector<F>& vecEnd, CCollisionResult& tr, COctreeBranch<T, F>* pBranch)
{
	COctreeBranch<T, F>* pThis = m_pOctreeHead;
	if (pBranch)
		pThis = pBranch;

	if (!pThis)
		return false;

	if (pThis->m_pBranches[0])
	{
		bool bHitBranch = false;
		for (size_t i = 0; i < 8; i++)
		{
			CCollisionResult cr;
			if (!SegmentIntersectsAABB<F>(vecStart, vecEnd, pThis->m_pBranches[i]->m_oBounds, cr))
				continue;

			// If we're already had a collision sooner than the wall of this branch, then this branch can be skipped.
			if (cr.flFraction > tr.flFraction)
				continue;

			bHitBranch |= Raytrace(vecStart, vecEnd, tr, pThis->m_pBranches[i]);
		}

		return bHitBranch;
	}

	// We're in a leaf node.
	return pThis->Raytrace(vecStart, vecEnd, tr);
}

template <class T, typename F>
void COctreeBranch<T, F>::TryPush()
{
	if (m_iDepth >= m_pTree->GetMaxDepth())
		return;

	if (!m_pBranches[0])
	{
		// Objects that the branch is not entirely inside.
		// If an object completely encapsulates the branch, we shouldn't consider it for making child branches.
		size_t iObjectsNotEntirelyInside = 0;

		size_t iMaxObjects = m_pTree->GetMaxObjects();
		size_t iSize = m_aObjects.size();
		for (size_t j = 0; j < iSize; j++)
		{
			COctreeObject<T, F>* pObject = &m_aObjects[j];

			if (pObject->m_oSize.m_vecMins.x < m_oBounds.m_vecMins.x &&
				pObject->m_oSize.m_vecMins.y < m_oBounds.m_vecMins.y &&
				pObject->m_oSize.m_vecMins.z < m_oBounds.m_vecMins.z &&
				pObject->m_oSize.m_vecMaxs.x > m_oBounds.m_vecMaxs.x &&
				pObject->m_oSize.m_vecMaxs.y > m_oBounds.m_vecMaxs.y &&
				pObject->m_oSize.m_vecMaxs.z > m_oBounds.m_vecMaxs.z)
				continue;

			iObjectsNotEntirelyInside++;
			if (iObjectsNotEntirelyInside >= iMaxObjects)
				break;
		}

		if (iObjectsNotEntirelyInside >= iMaxObjects)
		{
			F flSize = (m_oBounds.m_vecMaxs.x - m_oBounds.m_vecMins.x)/2;
			m_pBranchxyz = new COctreeBranch<T, F>(this, m_pTree, TemplateAABB<F>(m_oBounds.m_vecMins + TemplateVector<F>(0, 0, 0), m_oBounds.m_vecMins + TemplateVector<F>(flSize, flSize, flSize)), m_iDepth+1, 0);
			m_pBranchXyz = new COctreeBranch<T, F>(this, m_pTree, TemplateAABB<F>(m_oBounds.m_vecMins + TemplateVector<F>(flSize, 0, 0), m_oBounds.m_vecMins + TemplateVector<F>(flSize+flSize, flSize, flSize)), m_iDepth+1, 1);
			m_pBranchxYz = new COctreeBranch<T, F>(this, m_pTree, TemplateAABB<F>(m_oBounds.m_vecMins + TemplateVector<F>(0, flSize, 0), m_oBounds.m_vecMins + TemplateVector<F>(flSize, flSize+flSize, flSize)), m_iDepth+1, 2);
			m_pBranchXYz = new COctreeBranch<T, F>(this, m_pTree, TemplateAABB<F>(m_oBounds.m_vecMins + TemplateVector<F>(flSize, flSize, 0), m_oBounds.m_vecMins + TemplateVector<F>(flSize+flSize, flSize+flSize, flSize)), m_iDepth+1, 3);
			m_pBranchxyZ = new COctreeBranch<T, F>(this, m_pTree, TemplateAABB<F>(m_oBounds.m_vecMins + TemplateVector<F>(0, 0, flSize), m_oBounds.m_vecMins + TemplateVector<F>(flSize, flSize, flSize+flSize)), m_iDepth+1, 0);
			m_pBranchXyZ = new COctreeBranch<T, F>(this, m_pTree, TemplateAABB<F>(m_oBounds.m_vecMins + TemplateVector<F>(flSize, 0, flSize), m_oBounds.m_vecMins + TemplateVector<F>(flSize+flSize, flSize, flSize+flSize)), m_iDepth+1, 1);
			m_pBranchxYZ = new COctreeBranch<T, F>(this, m_pTree, TemplateAABB<F>(m_oBounds.m_vecMins + TemplateVector<F>(0, flSize, flSize), m_oBounds.m_vecMins + TemplateVector<F>(flSize, flSize+flSize, flSize+flSize)), m_iDepth+1, 2);
			m_pBranchXYZ = new COctreeBranch<T, F>(this, m_pTree, TemplateAABB<F>(m_oBounds.m_vecMins + TemplateVector<F>(flSize, flSize, flSize), m_oBounds.m_vecMins + TemplateVector<F>(flSize+flSize, flSize+flSize, flSize+flSize)), m_iDepth+1, 3);
		}
	}

	if (!m_pBranches[0])
		return;

	bool abPushes[8];
	abPushes[0] = abPushes[1] = abPushes[2] = abPushes[3] = abPushes[4] = abPushes[5] = abPushes[6] = abPushes[7] = false;

	size_t iSize = m_aObjects.size();
	for (size_t j = 0; j < iSize; j++)
	{
		COctreeObject<T, F>* pObject = &m_aObjects[j];

		tvector<COctreeBranch<T, F>*>& apBranches = m_pTree->GetObjectBranches()[pObject->m_oData];
		size_t iBranches = apBranches.size();
		for (size_t i = 0; i < iBranches; i++)
		{
			if (apBranches[i] == this)
			{
				apBranches.erase(apBranches.begin()+i);
				break;
			}
		}

		// Push an object down into every child branch that it touches.
		for (size_t i = 0; i < 8; i++)
		{
			if (pObject->m_oSize.m_vecMaxs.x < m_pBranches[i]->m_oBounds.m_vecMins.x)
				continue;

			if (pObject->m_oSize.m_vecMaxs.y < m_pBranches[i]->m_oBounds.m_vecMins.y)
				continue;

			if (pObject->m_oSize.m_vecMaxs.z < m_pBranches[i]->m_oBounds.m_vecMins.z)
				continue;

			if (pObject->m_oSize.m_vecMins.x > m_pBranches[i]->m_oBounds.m_vecMaxs.x)
				continue;

			if (pObject->m_oSize.m_vecMins.y > m_pBranches[i]->m_oBounds.m_vecMaxs.y)
				continue;

			if (pObject->m_oSize.m_vecMins.z > m_pBranches[i]->m_oBounds.m_vecMaxs.z)
				continue;

			m_pBranches[i]->m_aObjects.push_back(*pObject);
			apBranches.push_back(m_pBranches[i]);
			abPushes[i] = true;
		}
	}

	m_aObjects.clear();

	for (size_t i = 0; i < 8; i++)
	{
		if (!abPushes[i])
			continue;

		m_pBranches[i]->TryPush();
	}
}

template <class T, typename F>
void COctreeBranch<T, F>::TryPull()
{
	// Detect number of children below and try to pull and whatever.
	return;
}

template <class T, typename F>
bool COctreeBranch<T, F>::Raytrace(const TemplateVector<F>& vecStart, const TemplateVector<F>& vecEnd, CCollisionResult& tr)
{
#ifdef DEBUG_WITH_GL
	DrawBox(m_oBounds, m_aObjects.size()?Color(128, 128, 255):Color(255, 255, 255));
#endif

	// No children. Test all objects in this node.

	TemplateVector<F> vecClosest;
	bool bFound = false;

	for (size_t i = 0; i < m_aObjects.size(); i++)
	{
		COctreeObject<T, F>& oObject = m_aObjects[i];

#ifdef DEBUG_WITH_GL
		CQuadTreeBranch<CBranchData, double>* pQuad = oObject.m_oData.m_pQuad;
#endif

		CCollisionResult tr2;
		if (oObject.m_oData.Raytrace(vecStart, vecEnd, tr2))
		{
			// Sometimes a try will touch a closer leaf node,
			// but actually intersect the ray in the back, so
			// only accept hits inside this box.
			if (!PointInsideAABB<F>(m_oBounds, tr2.vecHit))
			{
#ifdef DEBUG_WITH_GL
				if (pQuad)
				{
					DrawTri(pQuad->m_oData.vec1, pQuad->m_oData.vec2, pQuad->m_oData.vec3, 0.0f, 1.0f, 0.0f, 3);
					DrawTri(pQuad->m_oData.vec2, pQuad->m_oData.vec4, pQuad->m_oData.vec3, 0.0f, 1.0f, 0.0f, 3);
				}
#endif
				continue;
			}

			bFound = true;

			if (tr2.flFraction < tr.flFraction)
			{
				tr.bHit = tr2.bHit;
				tr.flFraction = tr2.flFraction;
				tr.vecHit = tr2.vecHit;
				tr.vecNormal = tr2.vecNormal;
				tr.bStartInside |= tr2.bStartInside;
			}

#ifdef DEBUG_WITH_GL
			if (pQuad)
			{
				DrawTri(pQuad->m_oData.vec1, pQuad->m_oData.vec2, pQuad->m_oData.vec3, 1.0f, 0.0f, 0.0f, 2);
				DrawTri(pQuad->m_oData.vec2, pQuad->m_oData.vec4, pQuad->m_oData.vec3, 1.0f, 0.0f, 0.0f, 2);
			}
#endif
		}
#ifdef DEBUG_WITH_GL
		else
		if (pQuad)
		{
			DrawTri(pQuad->m_oData.vec1, pQuad->m_oData.vec2, pQuad->m_oData.vec3, 0.0f, 0.0f, 1.0f, 2);
			DrawTri(pQuad->m_oData.vec2, pQuad->m_oData.vec4, pQuad->m_oData.vec3, 0.0f, 0.0f, 1.0f, 2);
		}
#endif
	}

	return bFound;
}

template <class T, typename F>
void COctreeBranch<T, F>::RemoveObject(const T& oObject)
{
	size_t iSize = m_aObjects.size();
	for (size_t i = 0; i < iSize; i++)
	{
		if (m_aObjects[i].m_oData == oObject)
		{
			m_aObjects.erase(m_aObjects.begin()+i);
			return;
		}
	}
}

template <class T, typename F>
TemplateVector<F> COctreeBranch<T, F>::GetCenter()
{
	if (!m_bCenterCalculated)
	{
		m_vecCenter = (m_vecMin + m_vecMax)/2;
		m_bCenterCalculated = true;
	}

	return m_vecCenter;
}

template <class T, typename F>
void COctreeBranch<T, F>::CheckBranchSanity()
{
	// If we have branches then we must not have any objects.
	if (m_pBranches[0])
		TAssert(!m_aObjects.size());

	if (m_pBranches[0])
	{
		for (size_t i = 0; i < 8; i++)
			m_pBranches[i]->CheckBranchSanity();
	}
}

#ifdef DEBUG_WITH_GL
inline void DrawBox(const TemplateAABB<double>& b, const Color& c)
{
	DoubleVector min = b.m_vecMins;
	DoubleVector max = b.m_vecMaxs;
	DoubleVector vecSize = b.Size();

	if (SPGame()->GetSPRenderer()->GetRenderingScale() == SCALE_KILOMETER)
	{
		min *= 1000;
		max *= 1000;
		vecSize *= 1000;
	}

	glLineWidth(2);
	glColor3ubv(c);
	glBegin(GL_LINE_STRIP);
		glVertex3fv(Vector(min));
		glVertex3fv(Vector(min + DoubleVector(0, 0, vecSize.z)));
		glVertex3fv(Vector(min + DoubleVector(vecSize.x, 0, vecSize.z)));
		glVertex3fv(Vector(min + DoubleVector(vecSize.x, 0, 0)));
		glVertex3fv(Vector(min));
	glEnd();
	glBegin(GL_LINE_STRIP);
		glVertex3fv(Vector(max));
		glVertex3fv(Vector(max - DoubleVector(0, 0, vecSize.z)));
		glVertex3fv(Vector(max - DoubleVector(vecSize.x, 0, vecSize.z)));
		glVertex3fv(Vector(max - DoubleVector(vecSize.x, 0, 0)));
		glVertex3fv(Vector(max));
	glEnd();
	glBegin(GL_LINES);
		glVertex3fv(Vector(min));
		glVertex3fv(Vector(min + DoubleVector(0, vecSize.y, 0)));
		glVertex3fv(Vector(min + DoubleVector(0, 0, vecSize.z)));
		glVertex3fv(Vector(min + DoubleVector(0, vecSize.y, vecSize.z)));
		glVertex3fv(Vector(min + DoubleVector(vecSize.x, 0, vecSize.z)));
		glVertex3fv(Vector(min + DoubleVector(vecSize.x, vecSize.y, vecSize.z)));
		glVertex3fv(Vector(min + DoubleVector(vecSize.x, 0, 0)));
		glVertex3fv(Vector(min + DoubleVector(vecSize.x, vecSize.y, 0)));
	glEnd();
}

inline void DrawTri(const TemplateVector<double>& v1, const TemplateVector<double>& v2, const TemplateVector<double>& v3, float r, float g, float b, int iLineWidth)
{
	Vector vv1 = v1;
	Vector vv2 = v2;
	Vector vv3 = v3;

	if (SPGame()->GetSPRenderer()->GetRenderingScale() == SCALE_KILOMETER)
	{
		vv1 *= 1000;
		vv2 *= 1000;
		vv3 *= 1000;
	}

	glLineWidth((float)iLineWidth);
	glColor3f(r, g, b);
	glBegin(GL_LINE_LOOP);
		glVertex3fv(Vector(vv1));
		glVertex3fv(Vector(vv2));
		glVertex3fv(Vector(vv3));
	glEnd();
}
#endif

#endif
