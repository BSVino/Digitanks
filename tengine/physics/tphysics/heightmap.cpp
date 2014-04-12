#include "heightmap.h"

#include <application.h>
#include <game/gameserver.h>
#include <renderer/game_renderer.h>
#include <renderer/renderingcontext.h>

CHeightmapMesh::CHeightmapMesh(size_t iWidth, size_t iHeight, const AABB& aabbBounds, const float* pflVerts)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_aabbBounds = aabbBounds;
	m_pflVerts = pflVerts;
}

void CHeightmapMesh::TraceLine(size_t iExtraHandle, CTraceResult& tr, const Vector& v1, const Vector& v2)
{
	AABB aabbRealBounds = m_aabbBounds;
	aabbRealBounds.m_vecMaxs.z = 99999999.0f;
	aabbRealBounds.m_vecMins.z = -99999999.0f;

	Vector vmin = v1;
	Vector vmax = v2;

	if (vmin.x > vmax.x)
		TSwap(vmin.x, vmax.x);
	if (vmin.y > vmax.y)
		TSwap(vmin.y, vmax.y);
	if (vmin.z > vmax.z)
		TSwap(vmin.z, vmax.z);

	if (!aabbRealBounds.Intersects(AABB(vmin, vmax)))
		return;

	int y1 = (int)RemapVal(v1.y, m_aabbBounds.m_vecMins.y, m_aabbBounds.m_vecMaxs.y, 0, (float)m_iHeight-1);
	int y2 = (int)RemapVal(v2.y, m_aabbBounds.m_vecMins.y, m_aabbBounds.m_vecMaxs.y, 0, (float)m_iHeight-1);

	y1 = Clamp<int>(y1, 0, m_iHeight - 2);
	y2 = Clamp<int>(y2, 0, m_iHeight - 2);

	int j_dir = 1;
	if (y2 < y1)
		j_dir = -1;

	int x_min = (int)RemapVal(v1.x, m_aabbBounds.m_vecMins.x, m_aabbBounds.m_vecMaxs.x, 0, (float)m_iWidth-1);
	int x_max = (int)RemapVal(v2.x, m_aabbBounds.m_vecMins.x, m_aabbBounds.m_vecMaxs.x, 0, (float)m_iWidth-1);

	x_min = Clamp<int>(x_min, 0, m_iWidth - 2);
	x_max = Clamp<int>(x_max, 0, m_iWidth - 2);

	if (x_max < x_min)
		TSwap<int>(x_max, x_min);

	CCollisionResult cr;

	for (int j = y1; j_dir > 0 ? j <= y2 : j >= y2; j += j_dir)
	{
		// Find all possible x squares when y == j
		int x1, x2;

		if (v1.y == v2.y)
			x1 = x2 = (int)RemapVal(v1.x, m_aabbBounds.m_vecMins.x, m_aabbBounds.m_vecMaxs.x, 0, (float)m_iWidth-1);
		else
		{
			// Perf opportunity: Reduce these six RemapVal's into 2 RemapVal's.
			float flYLow = RemapVal((float)j, 0, (float)m_iHeight-1, m_aabbBounds.m_vecMins.y, m_aabbBounds.m_vecMaxs.y);
			float flYHigh = RemapVal((float)j + 1, 0, (float)m_iHeight-1, m_aabbBounds.m_vecMins.y, m_aabbBounds.m_vecMaxs.y);

			float flXStart = RemapVal(flYLow, v1.y, v2.y, v1.x, v2.x);
			float flXEnd = RemapVal(flYHigh, v1.y, v2.y, v1.x, v2.x);

			if (v2.x > v1.x && flXStart > flXEnd || v2.x < v1.x && flXStart < flXEnd)
				TSwap<float>(flXStart, flXEnd);

			x1 = (int)RemapVal(flXStart, m_aabbBounds.m_vecMins.x, m_aabbBounds.m_vecMaxs.x, 0, (float)m_iWidth-1);
			x2 = (int)RemapVal(flXEnd, m_aabbBounds.m_vecMins.x, m_aabbBounds.m_vecMaxs.x, 0, (float)m_iWidth-1);

			if (x1 < x_min && x2 < x_min || x1 > x_max && x1 > x_max)
				// All x squares for this y are out of the heightmap's range.
				continue;
		}

		x1 = Clamp<int>(x1, x_min, x_max);
		x2 = Clamp<int>(x2, x_min, x_max);

		int i_dir = 1;
		if (x2 < x1)
			i_dir = -1;

		for (int i = x1; i_dir > 0 ? i <= x2 : i >= x2; i += i_dir)
		{
			Vector t1 = GetPosition(i, j);
			Vector t2 = GetPosition(i+1, j);
			Vector t3 = GetPosition(i+1, j+1);
			Vector t4 = GetPosition(i, j+1);

			LineSegmentIntersectsTriangle(v1, v2, t1, t2, t3, cr);
			LineSegmentIntersectsTriangle(v1, v2, t1, t3, t4, cr);

			if (cr.flFraction < 1)
			{
				// We can bail here, we know the first hit is the right one
				// because we traverse along the trajectory of the vector.

				if (cr.flFraction < tr.m_flFraction)
				{
					tr.m_flFraction = (float)cr.flFraction;
					tr.m_aHits.push_back();
					tr.m_aHits.back().m_flFraction = (float)cr.flFraction;
					tr.m_aHits.back().m_iHitExtra = iExtraHandle;
					tr.m_aHits.back().m_vecHit = cr.vecHit;
					tr.m_iHitExtra = iExtraHandle;
					tr.m_vecHit = cr.vecHit;
					tr.m_vecNormal = cr.vecNormal;
				}

				return;
			}
		}
	}
}

float CHeightmapMesh::GetHeight(size_t x, size_t y)
{
	TAssert(x >= 0);
	TAssert(y >= 0);
	TAssert(x < m_iWidth);
	TAssert(y < m_iHeight);

	return m_pflVerts[x * m_iWidth + y];
}

Vector CHeightmapMesh::GetPosition(size_t x, size_t y)
{
	return Vector(
		RemapVal((float)x, 0.0f, (float)m_iWidth-1, m_aabbBounds.m_vecMins.x, m_aabbBounds.m_vecMaxs.x),
		RemapVal((float)y, 0.0f, (float)m_iHeight-1, m_aabbBounds.m_vecMins.y, m_aabbBounds.m_vecMaxs.y),
		GetHeight((size_t)x, (size_t)y)
	);
}

void CHeightmapMesh::DebugDraw(const Matrix4x4& mTransform, physics_debug_t iLevel)
{
	if (!m_pflVerts)
		return;

	if (iLevel == PHYS_DEBUG_AABB)
	{
		AABB aabbBounds = m_aabbBounds;
		aabbBounds.m_vecMaxs.z = m_pflVerts[0];
		aabbBounds.m_vecMins.z = m_pflVerts[0];

		for (size_t i = 1; i < m_iHeight * m_iWidth; i++)
		{
			if (m_pflVerts[i] > aabbBounds.m_vecMaxs.z)
				aabbBounds.m_vecMaxs.z = m_pflVerts[i];

			if (m_pflVerts[i] < aabbBounds.m_vecMins.z)
				aabbBounds.m_vecMins.z = m_pflVerts[i];
		}

		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.RenderWireBox(aabbBounds);
	}
	else if (iLevel == PHYS_DEBUG_WIREFRAME || iLevel == PHYS_DEBUG_WF_AND_CONTACTS)
	{
		Vector vecCenter = GetPosition(m_iWidth / 2, m_iHeight / 2);
		Vector vecCorner = GetPosition(0, 0);
		if (!GameServer()->GetRenderer()->IsSphereInFrustum(vecCenter, (vecCenter - vecCorner).Length()))
			return;

		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.BeginRenderDebugLines();
		for (size_t i = 0; i < m_iWidth-1; i++)
		{
			for (size_t j = 0; j < m_iHeight-1; j++)
			{
				c.Vertex(GetPosition(i, j));
				c.Vertex(GetPosition(i+1, j));

				c.Vertex(GetPosition(i, j));
				c.Vertex(GetPosition(i, j+1));
			}
		}
		c.EndRender();
	}
}
