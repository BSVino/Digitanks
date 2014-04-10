#include "heightmap.h"

CHeightmapMesh::CHeightmapMesh(size_t iWidth, size_t iHeight, const Vector* pvecVerts)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_pvecVerts = pvecVerts;
}

