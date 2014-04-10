#pragma once

#include "tphysics.h"

class CHeightmapMesh : public CPhysicsMesh
{
public:
	CHeightmapMesh(size_t iWidth, size_t iHeight, const Vector* pvecVerts);

private:
	size_t m_iWidth;
	size_t m_iHeight;

	const Vector* m_pvecVerts;
};
