#pragma once

#include "tphysics.h"

class CHeightmapMesh : public CPhysicsMesh
{
public:
	CHeightmapMesh(size_t iWidth, size_t iHeight, const AABB& aabbBounds, const float* pflVerts);

public:
	virtual void TraceLine(size_t iExtraHandle, CTraceResult& tr, const Vector& v1, const Vector& v2);

private:
	float GetHeight(size_t x, size_t y);
	Vector GetPosition(size_t x, size_t y);

private:
	size_t m_iWidth;
	size_t m_iHeight;

	AABB m_aabbBounds;

	const float* m_pflVerts;
};
