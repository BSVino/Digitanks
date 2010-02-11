#include "raytracer.h"

#include <assert.h>

#ifdef DEBUG_WITH_GL
#include <GL/freeglut.h>

void DrawBox(AABB& b, float c);
void DrawTri(Vector v1, Vector v2, Vector v3, float r, float g, float b);
#endif

using namespace raytrace;

CRaytracer::CRaytracer(CConversionScene* pScene)
{
	m_pScene = pScene;
	m_pTree = NULL;
}

CRaytracer::~CRaytracer()
{
	if (m_pTree)
		delete m_pTree;
}

bool CRaytracer::Raytrace(const Ray& rayTrace, Vector* pvecHit)
{
	if (!m_pTree)
		return RaytraceBruteForce(rayTrace, pvecHit);

	return m_pTree->Raytrace(rayTrace, pvecHit);
}

bool CRaytracer::RaytraceBruteForce(const Ray& rayTrace, Vector* pvecHit)
{
	// Brute force method.
	Vector vecClosest;
	bool bFound = false;

	for (size_t m = 0; m < m_pScene->GetNumMeshes(); m++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m);
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);
			for (size_t t = 0; t < pFace->GetNumVertices()-2; t++)
			{
				CConversionVertex* pV1 = pFace->GetVertex(0);
				CConversionVertex* pV2 = pFace->GetVertex(t+1);
				CConversionVertex* pV3 = pFace->GetVertex(t+2);

				Vector v1 = pMesh->GetVertex(pV1->v);
				Vector v2 = pMesh->GetVertex(pV2->v);
				Vector v3 = pMesh->GetVertex(pV3->v);

				Vector vecHit;
				if (RayIntersectsTriangle(rayTrace, v1, v2, v3, &vecHit))
				{
					if (!bFound || (vecHit - rayTrace.m_vecPos).LengthSqr() < (vecClosest - rayTrace.m_vecPos).LengthSqr())
					{
						bFound = true;
						vecClosest = vecHit;
					}
				}
			}
		}
	}

	if (bFound)
	{
		if (pvecHit)
			*pvecHit = vecClosest;
		return true;
	}

	return false;
}

void CRaytracer::BuildTree()
{
	// Add all scene tris.
	if (!m_pScene->GetNumScenes())
		return;

	m_pTree = new CKDTree();

	AddMeshesFromNode(m_pScene->GetScene(0));

	m_pTree->BuildTree();
}

void CRaytracer::AddMeshesFromNode(CConversionSceneNode* pNode)
{
	for (size_t c = 0; c < pNode->GetNumChildren(); c++)
		AddMeshesFromNode(pNode->GetChild(c));

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
	{
		CConversionMeshInstance* pMeshInstance = pNode->GetMeshInstance(m);
		for (size_t f = 0; f < pMeshInstance->GetMesh()->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMeshInstance->GetMesh()->GetFace(f);
			for (size_t t = 0; t < pFace->GetNumVertices()-2; t++)
			{
				CConversionVertex* pV1 = pFace->GetVertex(0);
				CConversionVertex* pV2 = pFace->GetVertex(t+1);
				CConversionVertex* pV3 = pFace->GetVertex(t+2);

				Vector v1 = pMeshInstance->GetVertex(pV1->v);
				Vector v2 = pMeshInstance->GetVertex(pV2->v);
				Vector v3 = pMeshInstance->GetVertex(pV3->v);

				m_pTree->AddTriangle(v1, v2, v3);
			}
		}
	}
}

CKDTree::CKDTree()
{
	m_pTop = new CKDNode();
}

CKDTree::~CKDTree()
{
	delete m_pTop;
}

void CKDTree::AddTriangle(Vector v1, Vector v2, Vector v3)
{
	m_pTop->AddTriangle(v1, v2, v3);
}

void CKDTree::BuildTree()
{
	m_pTop->CalcBounds();
	m_pTop->Build();
}

bool CKDTree::Raytrace(const Ray& rayTrace, Vector* pvecHit)
{
	if (!RayIntersectsAABB(rayTrace, m_pTop->GetBounds()))
		return false;

	return m_pTop->Raytrace(rayTrace, pvecHit);
}

CKDNode::CKDNode(CKDNode* pParent, AABB oBounds)
{
	m_pParent = pParent;
	m_oBounds = oBounds;

	if (m_pParent)
		m_iDepth = m_pParent->m_iDepth + 1;
	else
		m_iDepth = 0;

	m_pLeft = NULL;
	m_pRight = NULL;
}

CKDNode::~CKDNode()
{
	if (m_pLeft)
		delete m_pLeft;
	if (m_pRight)
		delete m_pRight;
}

void CKDNode::AddTriangle(Vector v1, Vector v2, Vector v3)
{
	m_aTris.push_back(CKDTri(v1, v2, v3));
}

void CKDNode::CalcBounds()
{
	if (!m_aTris.size())
	{
		m_oBounds = AABB();
		return;
	}

	// Initialize to the first value just so that it doesn't use the origin.
	// If it uses the origin and all of the tris have high values it will
	// wrongly include the origin in the bounds.
	Vector vecLowest(m_aTris[0].v[0]);
	Vector vecHighest(m_aTris[0].v[0]);

	for (size_t i = 0; i < m_aTris.size(); i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			for (size_t k = 0; k < 3; k++)
			{
				if (m_aTris[i].v[j][k] < vecLowest[k])
					vecLowest[k] = m_aTris[i].v[j][k];

				if (m_aTris[i].v[j][k] > vecHighest[k])
					vecHighest[k] = m_aTris[i].v[j][k];
			}
		}
	}

	m_oBounds = AABB(vecLowest, vecHighest);
}

void CKDNode::BuildTriList()
{
	m_aTris.clear();

	if (m_pParent)
	{
		// Copy all tris from my parent that intersect my bounding box
		for (size_t i = 0; i < m_pParent->m_aTris.size(); i++)
		{
			CKDTri oTri = m_pParent->m_aTris[i];
			if (TriangleIntersectsAABB(m_oBounds, oTri.v[0], oTri.v[1], oTri.v[2]))
				AddTriangle(oTri.v[0], oTri.v[1], oTri.v[2]);
		}
	}
}

void CKDNode::Build()
{
	// Find a better number!
	if (m_aTris.size() <= 3)
		return;

	// Find a better number!
	if (m_iDepth > 15)
		return;

	Vector vecBoundsSize = m_oBounds.Size();

	// Find the largest axis.
	if (vecBoundsSize.x > vecBoundsSize.y)
		m_iSplitAxis = 0;
	else
		m_iSplitAxis = 1;
	if (vecBoundsSize.z > vecBoundsSize[m_iSplitAxis])
		m_iSplitAxis = 2;

	// Use better algorithm!
	m_flSplitPos = m_oBounds.m_vecMins[m_iSplitAxis] + vecBoundsSize[m_iSplitAxis]/2;

	AABB oBoundsLeft, oBoundsRight;
	oBoundsLeft = oBoundsRight = m_oBounds;

	oBoundsLeft.m_vecMaxs[m_iSplitAxis] = m_flSplitPos;
	oBoundsRight.m_vecMins[m_iSplitAxis] = m_flSplitPos;

	m_pLeft = new CKDNode(this, oBoundsLeft);
	m_pRight = new CKDNode(this, oBoundsRight);

	m_pLeft->BuildTriList();
	m_pRight->BuildTriList();

	m_pLeft->Build();
	m_pRight->Build();
}

bool CKDNode::Raytrace(const Ray& rayTrace, Vector* pvecHit)
{
	if (!m_pLeft)
	{
#ifdef DEBUG_WITH_GL
		DrawBox(m_oBounds, 0.6f);
#endif

		// No children. Test all triangles in this node.

		Vector vecClosest;
		bool bFound = false;

		for (size_t i = 0; i < m_aTris.size(); i++)
		{
			CKDTri oTri = m_aTris[i];

			Vector vecHit;
			if (RayIntersectsTriangle(rayTrace, oTri.v[0], oTri.v[1], oTri.v[2], &vecHit))
			{
				// Sometimes a try will touch a closer leaf node,
				// but actually intersect the ray in the back, so
				// only accept hits inside this box.
				if (!PointInsideAABB(m_oBounds, vecHit))
				{
#ifdef DEBUG_WITH_GL
					DrawTri(oTri.v[0], oTri.v[1], oTri.v[2], 0.0f, 1.0f, 0.0f);
#endif
					continue;
				}

				if (!bFound || (vecHit - rayTrace.m_vecPos).LengthSqr() < (vecClosest - rayTrace.m_vecPos).LengthSqr())
				{
					bFound = true;
					vecClosest = vecHit;
				}

#ifdef DEBUG_WITH_GL
				DrawTri(oTri.v[0], oTri.v[1], oTri.v[2], 1.0f, 0.0f, 0.0f);
#endif
			}
#ifdef DEBUG_WITH_GL
			else
				DrawTri(oTri.v[0], oTri.v[1], oTri.v[2], 0.0f, 0.0f, 1.0f);
#endif
		}

		if (bFound)
		{
			if (pvecHit)
				*pvecHit = vecClosest;
			return true;
		}

		return false;
	}

	bool bHitsLeft = RayIntersectsAABB(rayTrace, m_pLeft->m_oBounds);
	bool bHitsRight = RayIntersectsAABB(rayTrace, m_pRight->m_oBounds);

#ifdef _DEBUG
	// If it hit this node then it's got to hit one of our child nodes since both child nodes add up to this one.
	if (!(bHitsRight || bHitsLeft))
		_asm { int 3 };
#endif

	if (bHitsLeft && !bHitsRight)
		return m_pLeft->Raytrace(rayTrace, pvecHit);

	if (bHitsRight && !bHitsLeft)
		return m_pRight->Raytrace(rayTrace, pvecHit);

	// Hit a poly in both cases, return the closer one.
	float flDistanceToLeft = (m_pLeft->m_oBounds.Center() - rayTrace.m_vecPos).LengthSqr();
	float flDistanceToRight = (m_pRight->m_oBounds.Center() - rayTrace.m_vecPos).LengthSqr();

	CKDNode* pCloser = m_pLeft;
	CKDNode* pFarther = m_pRight;
	if (flDistanceToRight < flDistanceToLeft)
	{
		pCloser = m_pRight;
		pFarther = m_pLeft;
	}

	if (pCloser->Raytrace(rayTrace, pvecHit))
		return true;
	else
		return pFarther->Raytrace(rayTrace, pvecHit);
}

CKDTri::CKDTri(Vector v1, Vector v2, Vector v3)
{
	v[0] = v1;
	v[1] = v2;
	v[2] = v3;
}

#ifdef DEBUG_WITH_GL
void DrawBox(AABB& b, float c)
{
	glLineWidth(2);
	Vector vecSize = b.Size();
	glColor3f(c, c, c);
	glBegin(GL_LINE_STRIP);
		glVertex3fv(b.m_vecMins);
		glVertex3fv(b.m_vecMins + Vector(0, 0, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, 0, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, 0, 0));
		glVertex3fv(b.m_vecMins);
	glEnd();
	glBegin(GL_LINE_STRIP);
		glVertex3fv(b.m_vecMaxs);
		glVertex3fv(b.m_vecMaxs - Vector(0, 0, vecSize.z));
		glVertex3fv(b.m_vecMaxs - Vector(vecSize.x, 0, vecSize.z));
		glVertex3fv(b.m_vecMaxs - Vector(vecSize.x, 0, 0));
		glVertex3fv(b.m_vecMaxs);
	glEnd();
	glBegin(GL_LINES);
		glVertex3fv(b.m_vecMins);
		glVertex3fv(b.m_vecMins + Vector(0, vecSize.y, 0));
		glVertex3fv(b.m_vecMins + Vector(0, 0, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(0, vecSize.y, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, 0, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, vecSize.y, vecSize.z));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, 0, 0));
		glVertex3fv(b.m_vecMins + Vector(vecSize.x, vecSize.y, 0));
	glEnd();
}

void DrawTri(Vector v1, Vector v2, Vector v3, float r, float g, float b)
{
	glLineWidth(3);
	glColor3f(r, g, b);
	glBegin(GL_LINE_LOOP);
		glVertex3fv(v1);
		glVertex3fv(v2);
		glVertex3fv(v3);
	glEnd();
}
#endif
