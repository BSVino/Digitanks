#include <assert.h>
#include <algorithm>
#include <utility>

#include "convmesh.h"
#include <geometry.h>

CConversionMesh::CConversionMesh(class CConversionScene* pScene, const std::wstring& sName)
{
	m_pScene = pScene;
	m_sName = sName;

	m_bVisible = true;
}

void CConversionMesh::Clear()
{
	m_sName = std::wstring(L"");

	m_aVertices.clear();
	m_aNormals.clear();
	m_aUVs.clear();
	m_aBones.clear();
	m_aFaces.clear();
	m_aEdges.clear();
}

void CConversionMesh::CalculateEdgeData()
{
	// No edges? Well then let's generate them.
	if (!GetNumEdges())
	{
		if (m_pScene->m_pWorkListener)
			m_pScene->m_pWorkListener->SetAction(L"Generating edges", GetNumFaces());

		// For every face, find and create edges.
		for (size_t iFace = 0; iFace < GetNumFaces(); iFace++)
		{
			CConversionFace* pFace = GetFace(iFace);

			for (size_t iVertex = 0; iVertex < pFace->GetNumVertices(); iVertex++)
			{
				size_t iAdjVertex;
				if (iVertex == pFace->GetNumVertices()-1)
					iAdjVertex = 0;
				else
					iAdjVertex = iVertex+1;

				// Find the other face with these two vertices!
				std::vector<size_t>& aFaces = m_aaVertexFaceMap[pFace->GetVertex(iVertex)->v];
				for (size_t iFace2 = 0; iFace2 < aFaces.size(); iFace2++)
				{
					if (iFace == aFaces[iFace2])
						continue;

					CConversionFace* pFace2 = GetFace(aFaces[iFace2]);

					size_t iVertex1 = pFace2->FindVertex(pFace->GetVertex(iVertex)->v);
					if (iVertex1 == ~0)
						continue;

					size_t iVertex2 = pFace2->FindVertex(pFace->GetVertex(iAdjVertex)->v);
					if (iVertex2 == ~0)
						continue;

					// I'm paranoid. Since these are unsigned make sure to subtract the larger by the smaller.
					if ((iVertex2>iVertex1?iVertex2-iVertex1:iVertex1-iVertex2) == 1)
					{
						// Check for duplicate edges first.
						bool bFoundEdge = false;
						size_t iEdge;
						for (iEdge = 0; iEdge < pFace->GetNumEdges(); iEdge++)
						{
							CConversionEdge* pEdge = GetEdge(pFace->GetEdge(iEdge));
							if (pEdge->HasVertex(pFace->GetVertex(iVertex)->v) && pEdge->HasVertex(pFace->GetVertex(iAdjVertex)->v))
							{
								bFoundEdge = true;
								break;
							}
						}

						// Since the edge gets added to both faces we only need to check one face.

						if (bFoundEdge)
							break;

						// By Jove we found it.
						iEdge = AddEdge(pFace->GetVertex(iVertex)->v, pFace->GetVertex(iAdjVertex)->v);
						CConversionEdge* pEdge = GetEdge(iEdge);
						if (std::find(pEdge->m_aiFaces.begin(), pEdge->m_aiFaces.end(), iFace) == pEdge->m_aiFaces.end())
							pEdge->m_aiFaces.push_back(iFace);
						if (std::find(pEdge->m_aiFaces.begin(), pEdge->m_aiFaces.end(), aFaces[iFace2]) == pEdge->m_aiFaces.end())
							pEdge->m_aiFaces.push_back(aFaces[iFace2]);
						AddEdgeToFace(iFace, iEdge);
						AddEdgeToFace(aFaces[iFace2], iEdge);
						break;
					}
				}
			}

			if (m_pScene->m_pWorkListener)
				m_pScene->m_pWorkListener->WorkProgress(iFace);
		}
	}
	else
	{
		if (m_pScene->m_pWorkListener)
			m_pScene->m_pWorkListener->SetAction(L"Finding edges", GetNumFaces());

		// For every edge, mark the vertexes it contains.
		for (size_t iFace = 0; iFace < GetNumFaces(); iFace++)
		{
			CConversionFace* pFace = GetFace(iFace);

			for (size_t iEdge = 0; iEdge < pFace->GetNumEdges(); iEdge++)
			{
				CConversionEdge* pEdge = GetEdge(pFace->GetEdge(iEdge));

				pEdge->m_aiFaces.push_back(iFace);
			}

			if (m_pScene->m_pWorkListener)
				m_pScene->m_pWorkListener->WorkProgress(iFace);
		}
	}

	if (m_pScene->m_pWorkListener)
		m_pScene->m_pWorkListener->SetAction(L"Calculating edge data", GetNumEdges());

	// For every conversion vertex, mark the edges it meets.
	for (size_t iEdge = 0; iEdge < GetNumEdges(); iEdge++)
	{
		CConversionEdge* pEdge = GetEdge(iEdge);

		for (size_t iEdgeFace = 0; iEdgeFace < pEdge->m_aiFaces.size(); iEdgeFace++)
		{
			// Find every vertex on this face and mark its vertices as having this edge.
			CConversionFace* pF1 = GetFace(pEdge->m_aiFaces[iEdgeFace]);
			for (size_t iVertex = 0; iVertex < pF1->GetNumVertices(); iVertex++)
			{
				CConversionVertex* pVertex = pF1->GetVertex(iVertex);
				if (pVertex->v == pEdge->v1)
					pVertex->m_aEdges.push_back(iEdge);
				else if (pVertex->v == pEdge->v2)
					pVertex->m_aEdges.push_back(iEdge);
			}
		}

		if (m_pScene->m_pWorkListener)
			m_pScene->m_pWorkListener->WorkProgress(iEdge);
	}
}

void CConversionMesh::CalculateVertexNormals()
{
	if (m_pScene->m_pWorkListener)
		m_pScene->m_pWorkListener->SetAction(L"Calculating vertex normals", GetNumFaces());

	m_aNormals.clear();

	// Got to calculate vertex normals now. We have to do it after we read faces because we need all of the face data loaded first.
	for (size_t iFace = 0; iFace < GetNumFaces(); iFace++)
	{
		CConversionFace* pFace = GetFace(iFace);

		// Loop through all vertices to calculate normals for
		for (size_t iVertex = 0; iVertex < pFace->GetNumVertices(); iVertex++)
		{
			CConversionVertex* pVertex = pFace->GetVertex(iVertex);

			// Build a list of faces that this vertex should use to calculate normals with.
			std::vector<size_t> aNormalFaces;
			pFace->FindAdjacentFaces(aNormalFaces, pVertex->v, true);

			Vector vecNormal = Vector();
			size_t iNormalFaces = 0;
			for (size_t iNormalFace = 0; iNormalFace < aNormalFaces.size(); iNormalFace++)
			{
				CConversionFace* pOtherFace = GetFace(aNormalFaces[iNormalFace]);

				if (pOtherFace->m_iSmoothingGroup != pFace->m_iSmoothingGroup)
					continue;

				iNormalFaces++;
				vecNormal += pOtherFace->GetNormal();
			}

			vecNormal /= (float)iNormalFaces;
			vecNormal.Normalize();

			// Find similar normals to save memory?
			// ... nah!
			pVertex->vn = AddNormal(vecNormal.x, vecNormal.y, vecNormal.z);
		}

		if (m_pScene->m_pWorkListener)
			m_pScene->m_pWorkListener->WorkProgress(iFace);
	}
}

void CConversionMesh::CalculateExtends()
{
	if (!GetNumVertices())
	{
		m_oExtends = AABB();
		return;
	}

	Vector vecMins = m_aVertices[0];
	Vector vecMaxs = m_aVertices[0];

	for (size_t iVertex = 0; iVertex < GetNumVertices(); iVertex++)
	{
		for (size_t i = 0; i < 3; i++)
		{
			if (m_aVertices[iVertex][i] < vecMins[i])
				vecMins[i] = m_aVertices[iVertex][i];
			if (m_aVertices[iVertex][i] > vecMaxs[i])
				vecMaxs[i] = m_aVertices[iVertex][i];
		}
	}

	m_oExtends = AABB(vecMins, vecMaxs);
}

CConversionScene::CConversionScene()
{
	m_pWorkListener = NULL;
}

void CConversionScene::DestroyAll()
{
	m_aMaterials.clear();
	m_aMeshes.clear();
	m_aScenes.clear();
}

void CConversionScene::CalculateExtends()
{
	if (!GetNumMeshes())
	{
		m_oExtends = AABB();
		return;
	}

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Calculating extends", GetNumMeshes() + GetNumScenes());

	for (size_t m = 0; m < GetNumMeshes(); m++)
	{
		GetMesh(m)->CalculateExtends();
		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(m+1);
	}

	for (size_t i = 0; i < GetNumScenes(); i++)
	{
		GetScene(i)->CalculateExtends();

		if (i == 0)
			m_oExtends = GetScene(i)->m_oExtends;
		else
		{
			AABB oMeshExtends = GetScene(i)->m_oExtends;
			for (size_t i = 0; i < 3; i++)
			{
				if (oMeshExtends.m_vecMins[i] < m_oExtends.m_vecMins[i])
					m_oExtends.m_vecMins[i] = oMeshExtends.m_vecMins[i];
				if (oMeshExtends.m_vecMaxs[i] > m_oExtends.m_vecMaxs[i])
					m_oExtends.m_vecMaxs[i] = oMeshExtends.m_vecMaxs[i];
			}
		}

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(GetNumMeshes()+i+1);
	}
}

size_t CConversionScene::AddMaterial(const std::wstring& sName)
{
	m_aMaterials.push_back(CConversionMaterial(sName));
	return m_aMaterials.size()-1;
}

size_t CConversionScene::AddMaterial(CConversionMaterial& oMaterial)
{
	m_aMaterials.push_back(oMaterial);
	return m_aMaterials.size()-1;
}

size_t CConversionScene::FindMaterial(const std::wstring& sName)
{
	for (size_t i = 0; i < m_aMaterials.size(); i++)
		if (sName.compare(m_aMaterials[i].m_sName) == 0)
			return i;

	return ((size_t)~0);
}

size_t CConversionScene::AddMesh(const std::wstring& sName)
{
	m_aMeshes.push_back(CConversionMesh(this, sName));
	return m_aMeshes.size()-1;
}

size_t CConversionScene::FindMesh(const std::wstring& sName)
{
	for (size_t i = 0; i < m_aMeshes.size(); i++)
		if (sName.compare(m_aMeshes[i].m_sName) == 0)
			return i;

	return ((size_t)~0);
}

size_t CConversionScene::FindMesh(CConversionMesh* pMesh)
{
	for (size_t i = 0; i < m_aMeshes.size(); i++)
		if (&m_aMeshes[i] == pMesh)
			return i;

	return ((size_t)~0);
}

size_t CConversionScene::AddScene(const std::wstring& sName)
{
	m_aScenes.push_back(CConversionSceneNode(sName, this, NULL));
	return m_aScenes.size()-1;
}

CConversionSceneNode* CConversionScene::GetDefaultScene()
{
	if (m_aScenes.size() == 0)
		m_aScenes.push_back(CConversionSceneNode(L"Default scene", this, NULL));

	return &m_aScenes[0];
}

CConversionSceneNode* CConversionScene::GetDefaultSceneMeshInstance(CConversionMesh* pMesh)
{
	CConversionSceneNode* pScene = GetDefaultScene();

	for (size_t i = 0; i < pScene->GetNumChildren(); i++)
	{
		CConversionSceneNode* pChild = pScene->GetChild(i);
		size_t iMesh = pChild->FindMeshInstance(pMesh);
		if (iMesh != ~0)
			return pChild;
	}

	// Put it in its own child node so that it can be moved around on its own.
	size_t iChild = pScene->AddChild(L"Default mesh");
	pScene->GetChild(iChild)->AddMeshInstance(FindMesh(pMesh));

	return pScene->GetChild(iChild);
}

size_t CConversionScene::AddDefaultSceneMaterial(CConversionMesh* pMesh, const std::wstring& sName)
{
	CConversionMeshInstance* pMeshInstance = GetDefaultSceneMeshInstance(pMesh)->GetMeshInstance(0);

	size_t iMaterialStub = pMesh->AddMaterialStub(sName);
	size_t iMaterial = AddMaterial(sName);
	pMeshInstance->m_aiMaterialsMap.insert(std::pair<size_t, CConversionMaterialMap>(iMaterialStub, CConversionMaterialMap(iMaterialStub, iMaterial)));

	return iMaterialStub;
}

CConversionSceneNode::CConversionSceneNode(const std::wstring& sName, CConversionScene* pScene, CConversionSceneNode* pParent)
{
	m_sName = sName;
	m_pScene = pScene;
	m_pParent = pParent;

	m_bVisible = true;
}

CConversionSceneNode::~CConversionSceneNode()
{
	// Test is (i < size()) because size_t is unsigned so there are no values < 0
	for (size_t i = m_apChildren.size()-1; i < m_apChildren.size(); i--)
		delete m_apChildren[i];
}

void CConversionSceneNode::CalculateExtends()
{
	size_t i, j;

	if (IsEmpty())
	{
		m_oExtends = AABB(Vector(0,0,0), Vector(0,0,0));
		return;
	}

	bool bFirst = true;
	for (i = 0; i < GetNumChildren(); i++)
	{
		CConversionSceneNode* pChild = GetChild(i);
		pChild->CalculateExtends();

		// Don't let extends with 0,0,0 pollute this one.
		if (pChild->IsEmpty())
			continue;

		if (bFirst)
		{
			m_oExtends = pChild->m_oExtends;
			bFirst = false;
		}
		else
		{
			AABB oChildExtends = pChild->m_oExtends;
			for (j = 0; j < 3; j++)
			{
				if (oChildExtends.m_vecMins[j] < m_oExtends.m_vecMins[j])
					m_oExtends.m_vecMins[j] = oChildExtends.m_vecMins[j];
				if (oChildExtends.m_vecMaxs[j] > m_oExtends.m_vecMaxs[j])
					m_oExtends.m_vecMaxs[j] = oChildExtends.m_vecMaxs[j];
			}
		}
	}

	for (i = 0; i < m_aMeshInstances.size(); i++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m_aMeshInstances[i].m_iMesh);

		Matrix4x4 mRoot = GetRootTransformations();

		// Transform the mesh extends for this node.
		AABB oTransformed;
		oTransformed.m_vecMins = mRoot*pMesh->m_oExtends.m_vecMins;
		oTransformed.m_vecMaxs = mRoot*pMesh->m_oExtends.m_vecMaxs;

		if (i == 0)
			m_oExtends = oTransformed;
		else
		{
			// It'd be more accurate to examine every vertex in the mesh, but that sounds slow!
			for (j = 0; j < 3; j++)
			{
				if (oTransformed.m_vecMins[j] < m_oExtends.m_vecMins[j])
					m_oExtends.m_vecMins[j] = oTransformed.m_vecMins[j];
				if (oTransformed.m_vecMaxs[j] > m_oExtends.m_vecMaxs[j])
					m_oExtends.m_vecMaxs[j] = oTransformed.m_vecMaxs[j];
			}
		}
	}
}

Matrix4x4 CConversionSceneNode::GetRootTransformations()
{
	Matrix4x4 mParent;
	if (m_pParent)
		mParent = m_pParent->GetRootTransformations();

	return mParent * m_mTransformations;
}

bool CConversionSceneNode::IsEmpty()
{
	if (m_aMeshInstances.size())
		return false;

	for (size_t i = 0; i < GetNumChildren(); i++)
		if (!GetChild(i)->IsEmpty())
			return false;

	return true;
}

size_t CConversionSceneNode::AddChild(const std::wstring& sName)
{
	m_apChildren.push_back(new CConversionSceneNode(sName, m_pScene, this));
	return m_apChildren.size()-1;
}

size_t CConversionSceneNode::AddMeshInstance(size_t iMesh)
{
	m_aMeshInstances.push_back(CConversionMeshInstance(m_pScene, this, iMesh));
	return m_aMeshInstances.size()-1;
}

// Returns the first mesh instance it finds that uses this mesh
size_t CConversionSceneNode::FindMeshInstance(CConversionMesh* pMesh)
{
	for (size_t i = 0; i < m_aMeshInstances.size(); i++)
	{
		if (m_pScene->GetMesh(m_aMeshInstances[i].m_iMesh) == pMesh)
			return i;
	}

	return ~0;
}

CConversionMeshInstance::CConversionMeshInstance(CConversionScene* pScene, CConversionSceneNode* pParent, size_t iMesh)
{
	m_pScene = pScene;
	m_pParent = pParent;
	m_iMesh = iMesh;

	m_bVisible = true;
}

CConversionMesh* CConversionMeshInstance::GetMesh()
{
	return m_pScene->GetMesh(m_iMesh);
}

void CConversionMeshInstance::AddMappedMaterial(size_t s, size_t m)
{
	m_aiMaterialsMap.insert(std::pair<size_t, CConversionMaterialMap>(s, CConversionMaterialMap(s, m)));
}

CConversionMaterialMap* CConversionMeshInstance::GetMappedMaterial(size_t m)
{
	if (m_aiMaterialsMap.find(m) == m_aiMaterialsMap.end())
		return NULL;

	return &m_aiMaterialsMap[m];
}

Vector CConversionMeshInstance::GetVertex(size_t i)
{
	return m_pParent->GetRootTransformations()*GetMesh()->GetVertex(i);
}

Vector CConversionMeshInstance::GetNormal(size_t i)
{
	Matrix4x4 mTransformations = m_pParent->GetRootTransformations();
	mTransformations.SetTranslation(Vector(0,0,0));
	return (mTransformations*GetMesh()->GetNormal(i)).Normalized();
}

CConversionMaterialMap::CConversionMaterialMap()
{
	m_iStub = 0;
	m_iMaterial = 0;

	m_bVisible = true;
}

CConversionMaterialMap::CConversionMaterialMap(size_t iStub, size_t iMaterial)
{
	m_iStub = iStub;
	m_iMaterial = iMaterial;

	m_bVisible = true;
}

size_t CConversionMesh::AddVertex(float x, float y, float z)
{
	m_aVertices.push_back(Vector(x, y, z));
	m_aaVertexFaceMap.push_back(std::vector<size_t>());
	return m_aVertices.size()-1;
}

size_t CConversionMesh::AddNormal(float x, float y, float z)
{
	m_aNormals.push_back(Vector(x, y, z));
	return m_aNormals.size()-1;
}

size_t CConversionMesh::AddUV(float u, float v)
{
	m_aUVs.push_back(Vector(u, v, 0));
	return m_aUVs.size()-1;
}

size_t CConversionMesh::AddBone(const std::wstring& sName)
{
	m_aBones.push_back(CConversionBone(sName));
	return m_aBones.size()-1;
}

size_t CConversionMesh::AddEdge(size_t v1, size_t v2)
{
	m_aEdges.push_back(CConversionEdge(v1, v2));
	return m_aEdges.size()-1;
}

size_t CConversionMesh::AddMaterialStub(const std::wstring& sName)
{
	m_aMaterialStubs.push_back(CConversionMaterialStub(sName));
	return m_aMaterialStubs.size()-1;
}

CConversionMaterialStub::CConversionMaterialStub(const std::wstring& sName)
{
	m_sName = sName;
}

Vector CConversionFace::GetNormal()
{
	assert(GetNumVertices() >= 3);

	// Precompute this shit maybe?

	size_t iPoints = GetNumVertices();

	// This algorithm works better for faces with convex points than a simple cross-product.
	Vector vecFaceNormal;
	for (size_t i = 0; i < iPoints; i++)
	{
		size_t iNext = (i+1)%iPoints;

		Vector vecThis = m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[i].v);
		Vector vecNext = m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[iNext].v);

		vecFaceNormal.x += (vecThis.y - vecNext.y) * (vecThis.z + vecNext.z);
		vecFaceNormal.y += (vecThis.z - vecNext.z) * (vecThis.x + vecNext.x);
		vecFaceNormal.z += (vecThis.x - vecNext.x) * (vecThis.y + vecNext.y);
	}

	return vecFaceNormal.Normalized();
}

Vector CConversionFace::GetCenter()
{
	assert(GetNumVertices() >= 3);

	// Precompute this shit maybe?
	Vector v(0, 0, 0);

	for (size_t i = 0; i < m_aVertices.size(); i++)
		v += m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[i].v);

	return v / (float)m_aVertices.size();
}

float CConversionFace::GetArea()
{
	Vector a = m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[0].v);
	Vector b = m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[1].v);

	float flArea = 0;

	for (size_t i = 0; i < m_aVertices.size()-2; i++)
	{
		Vector c = m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[i+2].v);

		flArea += TriangleArea(a, b, c);
	}

	return flArea;
}

float CConversionFace::GetUVArea()
{
	if (!m_pScene->GetMesh(m_iMesh)->GetNumUVs())
		return 0;

	Vector a = m_pScene->GetMesh(m_iMesh)->GetUV(m_aVertices[0].vt);
	Vector b = m_pScene->GetMesh(m_iMesh)->GetUV(m_aVertices[1].vt);

	float flArea = 0;

	for (size_t i = 0; i < m_aVertices.size()-2; i++)
	{
		Vector c = m_pScene->GetMesh(m_iMesh)->GetUV(m_aVertices[i+2].vt);

		flArea += TriangleArea(a, b, c);
	}

	return flArea;
}

void CConversionFace::FindAdjacentFaces(std::vector<size_t>& aResult, size_t iVert, bool bIgnoreCreased)
{
	aResult.push_back(m_pScene->GetMesh(m_iMesh)->FindFace(this));
	FindAdjacentFacesInternal(aResult, iVert, bIgnoreCreased);
}

void CConversionFace::FindAdjacentFacesInternal(std::vector<size_t>& aResult, size_t iVert, bool bIgnoreCreased)
{
	// Crawl along each edge to find adjacent faces.
	for (size_t iEdge = 0; iEdge < GetNumEdges(); iEdge++)
	{
		CConversionEdge* pEdge = m_pScene->GetMesh(m_iMesh)->GetEdge(GetEdge(iEdge));

		if (bIgnoreCreased && pEdge->m_bCreased)
			continue;

		if (iVert != ((size_t)~0) && !pEdge->HasVertex(iVert))
			continue;

		for (size_t iEdgeFace = 0; iEdgeFace < pEdge->m_aiFaces.size(); iEdgeFace++)
		{
			std::vector<size_t>::iterator it = find(aResult.begin(), aResult.end(), pEdge->m_aiFaces[iEdgeFace]);
			if (it == aResult.end())
			{
				aResult.push_back(pEdge->m_aiFaces[iEdgeFace]);
				m_pScene->GetMesh(m_iMesh)->GetFace(pEdge->m_aiFaces[iEdgeFace])->FindAdjacentFacesInternal(aResult, iVert, bIgnoreCreased);
			}
		}
	}
}

bool CConversionFace::HasEdge(size_t i)
{
	for (size_t iEdge = 0; iEdge < GetNumEdges(); iEdge++)
	{
		if (m_aEdges[iEdge] == i)
			return true;
	}

	return false;
}

size_t CConversionFace::FindVertex(size_t i)
{
	for (size_t iVertex = 0; iVertex < GetNumVertices(); iVertex++)
	{
		if (m_aVertices[iVertex].v == i)
			return iVertex;
	}

	return ~0;
}

std::vector<Vector>& CConversionFace::GetVertices(std::vector<Vector>& avecVertices)
{
	avecVertices.clear();

	for (size_t i = 0; i < m_aVertices.size(); i++)
		avecVertices.push_back(m_pScene->GetMesh(m_iMesh)->GetVertex(m_aVertices[i].v));

	return avecVertices;
}

size_t CConversionMesh::AddFace(size_t iMaterial)
{
	m_aFaces.push_back(CConversionFace(m_pScene, m_pScene->FindMesh(this), iMaterial));
	return m_aFaces.size()-1;
}

void CConversionMesh::AddVertexToFace(size_t iFace, size_t v, size_t vt, size_t vn)
{
	m_aaVertexFaceMap[v].push_back(iFace);
	m_aFaces[iFace].m_aVertices.push_back(CConversionVertex(v, vt, vn));
}

void CConversionMesh::AddEdgeToFace(size_t iFace, size_t iEdge)
{
	m_aFaces[iFace].m_aEdges.push_back(iEdge);
}

void CConversionMesh::RemoveFace(size_t iFace)
{
	m_aFaces.erase(m_aFaces.begin()+iFace);
}

size_t CConversionMesh::FindFace(CConversionFace* pFace)
{
	for (size_t i = 0; i < m_aFaces.size(); i++)
		if (&m_aFaces[i] == pFace)
			return i;

	return ((size_t)~0);
}

size_t CConversionMesh::FindMaterialStub(const std::wstring& sName)
{
	for (size_t i = 0; i < m_aMaterialStubs.size(); i++)
		if (m_aMaterialStubs[i].GetName().compare(sName) == 0)
			return i;

	return ((size_t)~0);
}

CConversionBone::CConversionBone(const std::wstring& sName)
{
	m_sName = sName;
}

CConversionMaterial::CConversionMaterial(const std::wstring& sName, Vector vecAmbient, Vector vecDiffuse, Vector vecSpecular, Vector vecEmissive, float flTransparency, float flShininess)
{
	m_sName = sName;

	m_vecAmbient = vecAmbient;
	m_vecDiffuse = vecDiffuse;
	m_vecSpecular = vecSpecular;
	m_vecEmissive = vecEmissive;
	m_flTransparency = flTransparency;
	m_flShininess = flShininess;
	m_eIllumType = ILLUM_FULL;

	m_bVisible = true;
}

CConversionFace::CConversionFace(class CConversionScene* pScene, size_t iMesh, size_t M)
{
	m_pScene = pScene;
	m_iMesh = iMesh;
	m = M;
}

CConversionEdge::CConversionEdge(size_t V1, size_t V2, bool bCreased)
{
	v1 = V1;
	v2 = V2;
	m_bCreased = bCreased;
}

bool CConversionEdge::HasVertex(size_t i)
{
	if (v1 == i || v2 == i)
		return true;

	return false;
}

CConversionVertex::CConversionVertex(size_t V, size_t VT, size_t VN)
{
	v = V;
	vt = VT;
	vn = VN;
}
