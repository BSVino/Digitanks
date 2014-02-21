#pragma once

#include <geometry.h>
#include <trs.h>

namespace tinker
{
	namespace protobuf
	{
		class Toy;
	}
}

class CToy
{
	friend class CToyUtil;

public:
	CToy();
	~CToy();

public:
	bool        ReadFromStream(std::fstream& stream);

	const AABB& GetVisBounds();
	const AABB& GetPhysBounds();
	size_t      GetNumMaterials();
	std::string GetMaterialName(size_t iMaterial);
	size_t      GetMaterialNumVerts(size_t iMaterial);
	const float*GetMaterialVerts(size_t iMaterial);
	const float*GetMaterialVert(size_t iMaterial, size_t iVert);

	size_t      GetVertexSizeInBytes(size_t iMaterial);
	int         GetVertexPositionOffsetInBytes(size_t iMaterial);
	int         GetVertexUVOffsetInBytes(size_t iMaterial);
	int         GetVertexNormalOffsetInBytes(size_t iMaterial);
	int         GetVertexTangentOffsetInBytes(size_t iMaterial);
	int         GetVertexBitangentOffsetInBytes(size_t iMaterial);

	size_t      GetPhysicsNumVerts();
	size_t      GetPhysicsNumTris();
	size_t      GetPhysicsNumBoxes();
	const float*GetPhysicsVerts();
	const float*GetPhysicsVert(size_t iVert);
	const int*  GetPhysicsTris();
	const int*  GetPhysicsTri(size_t iTri);
	TRS         GetPhysicsBox(size_t iBox);
	Vector      GetPhysicsBoxHalfSize(size_t iBox);

	size_t      GetNumSceneAreas();
	AABB        GetSceneAreaAABB(size_t iSceneArea);
	size_t      GetSceneAreaNumVisible(size_t iSceneArea);
	size_t      GetSceneAreasVisible(size_t iSceneArea, size_t iArea);
	std::string GetSceneAreaFileName(size_t iSceneArea);

public:
	void        DeallocateMesh();

protected:
	tinker::protobuf::Toy* m_pToy;

	AABB        m_aabbVisualBounds;
	AABB        m_aabbPhysicsBounds;

public:
	static AABB s_aabbBoxDimensions;
};
