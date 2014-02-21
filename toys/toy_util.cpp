#include "toy_util.h"

#include <common.h>
#include <files.h>
#include <tinker_platform.h>
#include <mesh.h>
#include <tvector.h>

#include <shell.h>

#include "toy.h"
#include "toy.pb.h"

CToyUtil::CToyUtil()
{
	m_aabbVisBounds.m_vecMins = Vector(999999, 999999, 999999);
	m_aabbVisBounds.m_vecMaxs = Vector(-999999, -999999, -999999);
	m_aabbPhysBounds.m_vecMins = Vector(999999, 999999, 999999);
	m_aabbPhysBounds.m_vecMaxs = Vector(-999999, -999999, -999999);

	m_flNeighborDistance = 1;
	m_bUseLocalTransforms = true;	// Use local by default

	m_bUseUV = false;
	m_bUseNormals = false;
}

tstring CToyUtil::GetGameDirectoryFile(const tstring& sFile) const
{
	return FindAbsolutePath(m_sGameDirectory + DIR_SEP + sFile);
}

tstring CToyUtil::GetScriptDirectoryFile(const tstring& sFile) const
{
	return FindAbsolutePath(m_sScriptDirectory + DIR_SEP + sFile);
}

void CToyUtil::AddMaterial(const tstring& sMaterial, const tstring& sOriginalFile)
{
//	tstring sOriginal = sOriginalFile;
//	if (!sOriginal.length())
//		sOriginal = sMaterial;

	tstring sFullName = sMaterial;
	if (!sFullName.endswith(".mat"))
		sFullName += ".mat";

	if (!sFullName.length())
	{
		m_asMaterials.push_back(sFullName);
		//m_asCopyTextures.push_back(sOriginal);
	}
	else
	{
		tstring sDirectoryPath = FindAbsolutePath(m_sGameDirectory);
		tstring sMaterialPath = FindAbsolutePath(sFullName);

		if (m_sGameDirectory.length() && sMaterialPath.find(sDirectoryPath) == 0)
		{
			size_t iLength = sDirectoryPath.length()+1;
			if (sDirectoryPath.back() == '\\' || sDirectoryPath.back() == '/')
				iLength = sDirectoryPath.length();

			m_asMaterials.push_back(ToForwardSlashes(sMaterialPath.substr(iLength)));
		}
		else
		{
			tstring sScriptPath = FindAbsolutePath(m_sScriptDirectory);
			if (m_sScriptDirectory.length() && sMaterialPath.find(sScriptPath) == 0)
			{
				size_t iLength = sScriptPath.length()+1;
				if (sScriptPath.back() == '\\' || sScriptPath.back() == '/')
					iLength = sScriptPath.length();

				m_asMaterials.push_back(GetFilenameAndExtension(ToForwardSlashes(sMaterialPath.substr(iLength))));
			}
			else
				m_asMaterials.push_back(GetFilenameAndExtension(sFullName));
		}

		//m_asCopyTextures.push_back(sOriginal);
	}

	m_aaflData.push_back();
}

void CToyUtil::UseUV(bool bUse)
{
	TAssert(!m_aaflData.size());
	m_bUseUV = bUse;
}

void CToyUtil::UseNormals(bool bUse)
{
	TAssert(!m_aaflData.size());
	m_bUseNormals = bUse;
}

void CToyUtil::AddVertex(size_t iMaterial, const Vector& vecPosition, const Vector2D& vecUV, const Matrix4x4& mNormals)
{
	m_aaflData[iMaterial].push_back(vecPosition.x);
	m_aaflData[iMaterial].push_back(vecPosition.y);
	m_aaflData[iMaterial].push_back(vecPosition.z);

	if (m_bUseUV)
	{
		m_aaflData[iMaterial].push_back(vecUV.x);
		m_aaflData[iMaterial].push_back(vecUV.y);
	}

	if (m_bUseNormals)
	{
		m_aaflData[iMaterial].push_back(mNormals.GetColumn(0).x);
		m_aaflData[iMaterial].push_back(mNormals.GetColumn(0).y);
		m_aaflData[iMaterial].push_back(mNormals.GetColumn(0).z);
		m_aaflData[iMaterial].push_back(mNormals.GetColumn(1).x);
		m_aaflData[iMaterial].push_back(mNormals.GetColumn(1).y);
		m_aaflData[iMaterial].push_back(mNormals.GetColumn(1).z);
		m_aaflData[iMaterial].push_back(mNormals.GetColumn(2).x);
		m_aaflData[iMaterial].push_back(mNormals.GetColumn(2).y);
		m_aaflData[iMaterial].push_back(mNormals.GetColumn(2).z);
	}

	for (int i = 0; i < 3; i++)
	{
		if (vecPosition[i] < m_aabbVisBounds.m_vecMins[i])
			m_aabbVisBounds.m_vecMins[i] = vecPosition[i];
		if (vecPosition[i] > m_aabbVisBounds.m_vecMaxs[i])
			m_aabbVisBounds.m_vecMaxs[i] = vecPosition[i];
	}
}

size_t CToyUtil::GetNumVerts() const
{
	size_t iVerts = 0;

	for (size_t i = 0; i < m_aaflData.size(); i++)
		iVerts += m_aaflData[i].size();

	size_t iVertSize = GetVertexSizeInBytes()/sizeof(float);

	TAssert(iVerts%iVertSize == 0);

	return iVerts/iVertSize;
}

size_t CToyUtil::GetVertexSizeInBytes() const
{
	size_t iVertSize = 3; // For position
	if (m_bUseNormals)
		iVertSize += 3*3; // For normal, tangent, bitangent
	if (m_bUseUV)
		iVertSize += 2;   // For UV

	return iVertSize*sizeof(float);
}

int CToyUtil::GetVertexUVOffsetInBytes() const
{
	if (!m_bUseUV)
		return -1;

	return 3*sizeof(float);
}

int CToyUtil::GetVertexNormalOffsetInBytes() const
{
	if (!m_bUseNormals)
		return -1;

	size_t iOffset = 3*sizeof(float);
	if (m_bUseUV)
		iOffset += 2*sizeof(float);

	return iOffset;
}

int CToyUtil::GetVertexTangentOffsetInBytes() const
{
	if (!m_bUseNormals)
		return -1;

	size_t iOffset = 6*sizeof(float);
	if (m_bUseUV)
		iOffset += 2*sizeof(float);

	return iOffset;
}

int CToyUtil::GetVertexBitangentOffsetInBytes() const
{
	if (!m_bUseNormals)
		return -1;

	size_t iOffset = 9*sizeof(float);
	if (m_bUseUV)
		iOffset += 2*sizeof(float);

	return iOffset;
}

void CToyUtil::AddPhysTriangle(size_t v1, size_t v2, size_t v3)
{
	m_aiPhysIndices.push_back(v1);
	m_aiPhysIndices.push_back(v2);
	m_aiPhysIndices.push_back(v3);
}

void CToyUtil::AddPhysVertex(Vector vecPosition)
{
	m_avecPhysVerts.push_back(vecPosition);

	for (int i = 0; i < 3; i++)
	{
		if (vecPosition[i] < m_aabbPhysBounds.m_vecMins[i])
			m_aabbPhysBounds.m_vecMins[i] = vecPosition[i];
		if (vecPosition[i] > m_aabbPhysBounds.m_vecMaxs[i])
			m_aabbPhysBounds.m_vecMaxs[i] = vecPosition[i];
	}
}

void CToyUtil::AddPhysBox(const TRS& trsBox)
{
	TAssert(trsBox.m_angRotation.p == 0);
	TAssert(trsBox.m_angRotation.y == 0);
	TAssert(trsBox.m_angRotation.r == 0);

	m_atrsPhysBoxes.push_back(trsBox);

	AABB aabbBox = CToy::s_aabbBoxDimensions;
	aabbBox.m_vecMaxs = trsBox.GetMatrix4x4()*aabbBox.m_vecMaxs;
	aabbBox.m_vecMins = trsBox.GetMatrix4x4()*aabbBox.m_vecMins;

	for (int i = 0; i < 3; i++)
	{
		if (aabbBox.m_vecMins[i] < m_aabbPhysBounds.m_vecMins[i])
			m_aabbPhysBounds.m_vecMins[i] = aabbBox.m_vecMins[i];
		if (aabbBox.m_vecMaxs[i] > m_aabbPhysBounds.m_vecMaxs[i])
			m_aabbPhysBounds.m_vecMaxs[i] = aabbBox.m_vecMaxs[i];
	}
}

size_t CToyUtil::AddSceneArea(const tstring& sFileName)
{
	CToy* pArea = new CToy();
	bool bRead = CToyUtil::Read(GetGameDirectory() + DIR_SEP + sFileName, pArea);

	if (!bRead)
	{
		delete pArea;

		TAssert(bRead);
		TError("Couldn't find scene area " + sFileName + "\n");

		return ~0;
	}

	CSceneArea& oSceneArea = m_asSceneAreas.push_back();

	oSceneArea.m_sFileName = sFileName;
	oSceneArea.m_aabbArea = pArea->GetVisBounds();

	// I'm visible to myself.
	oSceneArea.m_aiNeighboringAreas.push_back(m_asSceneAreas.size()-1);

	delete pArea;

	return m_asSceneAreas.size()-1;
}

void CToyUtil::AddSceneAreaNeighbor(size_t iSceneArea, size_t iNeighbor)
{
	TAssert(iSceneArea < m_asSceneAreas.size());
	if (iSceneArea >= m_asSceneAreas.size())
		return;

	TAssert(iNeighbor < m_asSceneAreas.size());
	if (iNeighbor >= m_asSceneAreas.size())
		return;

	for (size_t i = 0; i < m_asSceneAreas[iSceneArea].m_aiNeighboringAreas.size(); i++)
	{
		if (m_asSceneAreas[iSceneArea].m_aiNeighboringAreas[i] == iNeighbor)
			return;
	}

	m_asSceneAreas[iSceneArea].m_aiNeighboringAreas.push_back(iNeighbor);
}

void CToyUtil::AddSceneAreaVisible(size_t iSceneArea, size_t iVisible)
{
	TAssert(iSceneArea < m_asSceneAreas.size());
	if (iSceneArea >= m_asSceneAreas.size())
		return;

	TAssert(iVisible < m_asSceneAreas.size());
	if (iVisible >= m_asSceneAreas.size())
		return;

	if (IsVisibleFrom(iSceneArea, iVisible))
		return;

	m_asSceneAreas[iSceneArea].m_aiVisibleAreas.push_back(iVisible);
}

bool CToyUtil::IsVisibleFrom(size_t iSceneArea, size_t iVisible)
{
	for (size_t i = 0; i < m_asSceneAreas[iSceneArea].m_aiVisibleAreas.size(); i++)
	{
		if (m_asSceneAreas[iSceneArea].m_aiVisibleAreas[i] == iVisible)
			return true;
	}

	return false;
}

void CToyUtil::CalculateVisibleAreas()
{
	AABB aabbExpand(Vector(-m_flNeighborDistance, -m_flNeighborDistance, -m_flNeighborDistance)/2, Vector(m_flNeighborDistance, m_flNeighborDistance, m_flNeighborDistance)/2);

	// First auto-detect neighbors. Naive O(n(n-1)/2) distance check.
	for (size_t i = 0; i < m_asSceneAreas.size(); i++)
	{
		// We can skip j <= i since we add neighbors reciprocally
		for (size_t j = i+1; j < m_asSceneAreas.size(); j++)
		{
			// Instead of finding the actual distance, just expand each box by m_flNeighborDistance/2 in every direction and test intersection. It's easier!
			AABB aabbBounds1 = m_asSceneAreas[i].m_aabbArea + aabbExpand;
			AABB aabbBounds2 = m_asSceneAreas[j].m_aabbArea + aabbExpand;

			if (aabbBounds1.Intersects(aabbBounds2))
			{
				AddSceneAreaNeighbor(i, j);
				AddSceneAreaNeighbor(j, i);
				continue;
			}
		}
	}

	for (size_t i = 0; i < m_asSceneAreas.size(); i++)
	{
		for (size_t j = 0; j < m_asSceneAreas[i].m_aiNeighboringAreas.size(); j++)
		{
			size_t iNeighbor = m_asSceneAreas[i].m_aiNeighboringAreas[j];

			// I can always see my neighbors
			AddSceneAreaVisible(i, iNeighbor);
			AddSceneAreaVisible(iNeighbor, i);

			AddVisibleNeighbors(i, iNeighbor);
		}
	}
}

// Add any neighbors of iVisible which are visible to iArea's visible set.
void CToyUtil::AddVisibleNeighbors(size_t iArea, size_t iVisible)
{
	if (iArea == iVisible)
		return;

	tvector<Vector> avecPoints;

	for (size_t i = 0; i < m_asSceneAreas[iVisible].m_aiNeighboringAreas.size(); i++)
	{
		size_t iOther = m_asSceneAreas[iVisible].m_aiNeighboringAreas[i];

		// If this area is already visible, we can skip it to prevent extra work and recursion.
		if (IsVisibleFrom(iArea, iOther))
			continue;

		// Form a convex hull from the bounding boxes of iArea and i
		avecPoints.clear();

		Vector vecMins = m_asSceneAreas[iArea].m_aabbArea.m_vecMins;
		Vector vecMaxs = m_asSceneAreas[iArea].m_aabbArea.m_vecMaxs;
		avecPoints.push_back(vecMins);
		avecPoints.push_back(Vector(vecMins.x, vecMins.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMins.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMins.y, vecMins.z));
		avecPoints.push_back(Vector(vecMins.x, vecMaxs.y, vecMins.z));
		avecPoints.push_back(Vector(vecMins.x, vecMaxs.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMaxs.y, vecMins.z));
		avecPoints.push_back(vecMaxs);

		vecMins = m_asSceneAreas[iOther].m_aabbArea.m_vecMins;
		vecMaxs = m_asSceneAreas[iOther].m_aabbArea.m_vecMaxs;
		avecPoints.push_back(vecMins);
		avecPoints.push_back(Vector(vecMins.x, vecMins.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMins.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMins.y, vecMins.z));
		avecPoints.push_back(Vector(vecMins.x, vecMaxs.y, vecMins.z));
		avecPoints.push_back(Vector(vecMins.x, vecMaxs.y, vecMaxs.z));
		avecPoints.push_back(Vector(vecMaxs.x, vecMaxs.y, vecMins.z));
		avecPoints.push_back(vecMaxs);

#ifdef __linux__
		// Odd linker errors, linker can't find CConvexHullGenerator for some reason
		TUnimplemented();
#else
		CConvexHullGenerator c(avecPoints);

		const tvector<size_t>& avecConvexTriangles = c.GetConvexTriangles();

		// Test to see if iVisible intersects that hull
		AABB aabbShrunkBounds = m_asSceneAreas[iVisible].m_aabbArea + AABB(Vector(0.1f, 0.1f, 0.1f), Vector(-0.1f, -0.1f, -0.1f));	// Shrink the bounds a tad so touching bounds on the far side don't count.
		bool bIntersect = ConvexHullIntersectsAABB(aabbShrunkBounds, avecPoints, avecConvexTriangles);

		if (bIntersect)
		{
			AddSceneAreaVisible(iArea, iOther);
			AddSceneAreaVisible(iOther, iArea);

			// Calling it recursively this way may allow visibility that doesn't exist, eg, in a chain a -> b -> c -> d, a can see d through c but not through b.
			// I'm willing to accept that for now if it doesn't become a problem.
			AddVisibleNeighbors(iArea, iOther);
		}
#endif
	}
}

void FillProtoBufEAngle(tinker::protobuf::EAngle* pEAngle, const EAngle& angFill)
{
	pEAngle->set_p(angFill.p);
	pEAngle->set_y(angFill.y);
	pEAngle->set_r(angFill.r);
}

void FillProtoBufVector(tinker::protobuf::Vector* pVector, const Vector& vecFill)
{
	pVector->set_x(vecFill.x);
	pVector->set_y(vecFill.y);
	pVector->set_z(vecFill.z);
}

void FillProtoBufAABB(tinker::protobuf::AABB* pAABB, const AABB& aabbFill)
{
	FillProtoBufVector(pAABB->mutable_min(), aabbFill.m_vecMins);
	FillProtoBufVector(pAABB->mutable_max(), aabbFill.m_vecMaxs);
}

bool CToyUtil::Write(const tstring& sFilename)
{
	CalculateVisibleAreas();

	for (size_t i = m_asMaterials.size()-1; i < m_asMaterials.size(); i--)
	{
		// Must have at least one vertex or you get the boot.
		if (!m_aaflData[i].size())
		{
			m_aaflData.erase(m_aaflData.begin()+i);
			m_asMaterials.erase(m_asMaterials.begin()+i);
			//m_asCopyTextures.erase(m_asCopyTextures.begin()+i);
		}
	}

	// Copy textures to the destination folders.
/*	for (size_t i = 0; i < m_asTextures.size(); i++)
	{
		if (!m_asCopyTextures[i].length())
			continue;

		if (!m_asTextures[i].length())
			continue;

		if (!CopyFileTo(m_asCopyTextures[i], m_asTextures[i], true))
			TError("Couldn't copy texture '" + m_asCopyTextures[i] + "' to '" + m_asTextures[i] + "'.\n");
	}*/

	tinker::protobuf::Toy pbToy;

	tinker::protobuf::ToyBase* pToyBase = pbToy.mutable_base();

	FillProtoBufAABB(pToyBase->mutable_physics_bounds(), m_aabbPhysBounds);
	FillProtoBufAABB(pToyBase->mutable_visual_bounds(), m_aabbVisBounds);

	tinker::protobuf::ToyMesh* pToyMesh = m_asMaterials.size()?pbToy.mutable_mesh():nullptr;
	for (size_t i = 0; i < m_asMaterials.size(); i++)
	{
		auto* pBaseMaterial = pToyBase->add_material();
		auto* pMeshMaterial = pToyMesh->add_material();

		pBaseMaterial->set_name(m_asMaterials[i].c_str());
		pBaseMaterial->set_vertex_size_bytes(GetVertexSizeInBytes());
		pBaseMaterial->set_vertex_count(m_aaflData[i].size()/(GetVertexSizeInBytes()/sizeof(float)));
		pBaseMaterial->set_uv_offset(GetVertexUVOffsetInBytes());
		pBaseMaterial->set_normal_offset(GetVertexNormalOffsetInBytes());
		pBaseMaterial->set_tangent_offset(GetVertexTangentOffsetInBytes());
		pBaseMaterial->set_bitangent_offset(GetVertexBitangentOffsetInBytes());

		auto* pData = pMeshMaterial->mutable_data();
		pMeshMaterial->mutable_data()->Reserve(m_aaflData[i].size());

		auto* aflData = m_aaflData[i].data();
		size_t iDataSize = m_aaflData[i].size();

		for (size_t j = 0; j < iDataSize; j++)
			pData->AddAlreadyReserved(aflData[j]);
	}

	tinker::protobuf::ToyPhys* pToyPhys = (m_aiPhysIndices.size()||m_atrsPhysBoxes.size())?pbToy.mutable_phys():nullptr;
	if (m_aiPhysIndices.size())
	{
		auto* pIndices = pToyPhys->mutable_index();
		auto* pVerts = pToyPhys->mutable_vert();

		TAssert(m_aiPhysIndices.size()%3==0);

		pIndices->Reserve(m_aiPhysIndices.size());

		auto* aiData = m_aiPhysIndices.data();
		size_t iDataSize = m_aiPhysIndices.size();

		for (size_t i = 0; i < iDataSize; i++)
			pIndices->AddAlreadyReserved(aiData[i]);

		pVerts->Reserve(m_avecPhysVerts.size()*3);

		auto* aflData = m_avecPhysVerts.data();
		iDataSize = m_avecPhysVerts.size();

		for (size_t i = 0; i < iDataSize; i++)
		{
			pVerts->AddAlreadyReserved(aflData[i].x);
			pVerts->AddAlreadyReserved(aflData[i].y);
			pVerts->AddAlreadyReserved(aflData[i].z);
		}
	}

	if (m_atrsPhysBoxes.size())
	{
		auto* pBoxen = pToyPhys->mutable_box();

		pBoxen->Reserve(m_atrsPhysBoxes.size());

		for (size_t i = 0; i < m_atrsPhysBoxes.size(); i++)
		{
			auto* pBox = pBoxen->Add();
			FillProtoBufVector(pBox->mutable_translation(), m_atrsPhysBoxes[i].m_vecTranslation);
			FillProtoBufEAngle(pBox->mutable_rotation(), m_atrsPhysBoxes[i].m_angRotation);
			FillProtoBufVector(pBox->mutable_scaling(), m_atrsPhysBoxes[i].m_vecScaling);
		}
	}

	for (size_t i = 0; i < m_asSceneAreas.size(); i++)
	{
		auto* pArea = pbToy.add_area();

		FillProtoBufAABB(pArea->mutable_size(), m_asSceneAreas[i].m_aabbArea);
		pArea->set_file(m_asSceneAreas[i].m_sFileName.c_str());

		for (size_t j = 0; j < m_asSceneAreas[i].m_aiVisibleAreas.size(); j++)
			pArea->add_neighbor(m_asSceneAreas[i].m_aiVisibleAreas[j]);
	}

	std::fstream output(sFilename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);

	TAssert(!!output);
	if (!output)
		return false;

	if (!pbToy.SerializeToOstream(&output))
		return false;

	return true;
}

bool CToyUtil::Read(const tstring& sFilename, CToy* pToy)
{
	if (!pToy)
		return false;

	std::fstream input(sFilename.c_str(), std::ios::in | std::ios::binary);

	if (!input)
		return false;

	if (!pToy->ReadFromStream(input))
		return false;

	return true;
}
