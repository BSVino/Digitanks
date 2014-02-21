#include "toy.h"

#include <stdlib.h>
#include <stdio.h>

#include <common.h>
#include <tstring.h>

#include <shell.h>

#include "toy.pb.h"

AABB CToy::s_aabbBoxDimensions = AABB(-Vector(0.5f, 0.5f, 0.5f), Vector(0.5f, 0.5f, 0.5f));

CToy::CToy()
{
	m_pToy = nullptr;
}

CToy::~CToy()
{
	delete m_pToy;
}

EAngle ReadProtoBufEAngle(const tinker::protobuf::EAngle& pbEAngle)
{
	EAngle angRead;
	angRead.p = pbEAngle.p();
	angRead.y = pbEAngle.y();
	angRead.r = pbEAngle.r();
	return angRead;
}

Vector ReadProtoBufVector(const tinker::protobuf::Vector& pbVector)
{
	Vector vecRead;
	vecRead.x = pbVector.x();
	vecRead.y = pbVector.y();
	vecRead.z = pbVector.z();
	return vecRead;
}

AABB ReadProtoBufAABB(const tinker::protobuf::AABB& pbAABB)
{
	AABB aabbRead;
	aabbRead.m_vecMins = ReadProtoBufVector(pbAABB.min());
	aabbRead.m_vecMaxs = ReadProtoBufVector(pbAABB.max());
	return aabbRead;
}

TRS ReadProtoBufTRS(const tinker::protobuf::TRS& pbTRS)
{
	TRS trsRead;
	trsRead.m_vecTranslation = ReadProtoBufVector(pbTRS.translation());
	trsRead.m_angRotation = ReadProtoBufEAngle(pbTRS.rotation());
	trsRead.m_vecScaling = ReadProtoBufVector(pbTRS.scaling());
	return trsRead;
}

bool CToy::ReadFromStream(std::fstream& stream)
{
	if (m_pToy)
		delete m_pToy;

	m_pToy = new tinker::protobuf::Toy();

	if (!m_pToy->ParseFromIstream(&stream))
		return false;

	m_aabbVisualBounds = ReadProtoBufAABB(m_pToy->base().visual_bounds());
	m_aabbPhysicsBounds = ReadProtoBufAABB(m_pToy->base().physics_bounds());

	return true;
}

const AABB& CToy::GetVisBounds()
{
	return m_aabbVisualBounds;
}

const AABB& CToy::GetPhysBounds()
{
	return m_aabbPhysicsBounds;
}

size_t CToy::GetNumMaterials()
{
	return m_pToy->base().material_size();
}

std::string CToy::GetMaterialName(size_t i)
{
	return m_pToy->base().material(i).name();
}

size_t CToy::GetMaterialNumVerts(size_t i)
{
	return m_pToy->base().material(i).vertex_count();
}

const float* CToy::GetMaterialVerts(size_t iMaterial)
{
	return m_pToy->mesh().material(iMaterial).data().data();
}

const float* CToy::GetMaterialVert(size_t iMaterial, size_t iVert)
{
	return GetMaterialVerts(iMaterial) + iVert*(GetVertexSizeInBytes(iMaterial)/sizeof(float));
}

size_t CToy::GetVertexSizeInBytes(size_t iMaterial)
{
	return m_pToy->base().material(iMaterial).vertex_size_bytes();
}

int CToy::GetVertexPositionOffsetInBytes(size_t iMaterial)
{
	return 0;
}

int CToy::GetVertexUVOffsetInBytes(size_t iMaterial)
{
	return m_pToy->base().material(iMaterial).uv_offset();
}

int CToy::GetVertexNormalOffsetInBytes(size_t iMaterial)
{
	return m_pToy->base().material(iMaterial).normal_offset();
}

int CToy::GetVertexTangentOffsetInBytes(size_t iMaterial)
{
	return m_pToy->base().material(iMaterial).tangent_offset();
}

int CToy::GetVertexBitangentOffsetInBytes(size_t iMaterial)
{
	return m_pToy->base().material(iMaterial).bitangent_offset();
}

size_t CToy::GetPhysicsNumVerts()
{
	return m_pToy->phys().vert().size()/3; // Because 3 floats in a vector
}

size_t CToy::GetPhysicsNumTris()
{
	TAssert(m_pToy->phys().index().size()%3==0);
	return m_pToy->phys().index().size()/3; // Because 3 points in a triangle
}

size_t CToy::GetPhysicsNumBoxes()
{
	return m_pToy->phys().box().size();
}

const float* CToy::GetPhysicsVerts()
{
	return m_pToy->phys().vert().data();
}

const float* CToy::GetPhysicsVert(size_t iVert)
{
	return GetPhysicsVerts() + iVert*3;
}

const int* CToy::GetPhysicsTris()
{
	return m_pToy->phys().index().data();
}

const int* CToy::GetPhysicsTri(size_t iTri)
{
	return GetPhysicsTris() + iTri*3;
}

TRS CToy::GetPhysicsBox(size_t iBox)
{
	return ReadProtoBufTRS(m_pToy->phys().box(iBox));
}

Vector CToy::GetPhysicsBoxHalfSize(size_t iBox)
{
	TRS trs = GetPhysicsBox(iBox);

	TAssert(trs.m_angRotation.p == 0);
	TAssert(trs.m_angRotation.y == 0);
	TAssert(trs.m_angRotation.r == 0);

	Matrix4x4 mTRS = trs.GetMatrix4x4();

	AABB aabbBox = s_aabbBoxDimensions;
	aabbBox.m_vecMins = mTRS*aabbBox.m_vecMins;
	aabbBox.m_vecMaxs = mTRS*aabbBox.m_vecMaxs;

	return aabbBox.m_vecMaxs - aabbBox.Center();
}

size_t CToy::GetNumSceneAreas()
{
	return m_pToy->area().size();
}

AABB CToy::GetSceneAreaAABB(size_t iSceneArea)
{
	return ReadProtoBufAABB(m_pToy->area(iSceneArea).size());
}

size_t CToy::GetSceneAreaNumVisible(size_t iSceneArea)
{
	return m_pToy->area(iSceneArea).neighbor().size();
}

size_t CToy::GetSceneAreasVisible(size_t iSceneArea, size_t iArea)
{
	return m_pToy->area(iSceneArea).neighbor(iArea);
}

std::string CToy::GetSceneAreaFileName(size_t iSceneArea)
{
	return m_pToy->area(iSceneArea).file();
}

void CToy::DeallocateMesh()
{
	delete m_pToy->release_mesh();
}
