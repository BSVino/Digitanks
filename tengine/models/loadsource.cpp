#include "models.h"

#include <stdio.h>

#include <tinker_platform.h>
#include <files.h>

#include <modelconverter/modelconverter.h>
#include <renderer/renderer.h>
#include <textures/materiallibrary.h>
#include <tinker/application.h>
#include <datamanager/data.h>

tvector<tstring>			g_asTextures;
tvector<tvector<float> >	g_aaflData;
AABB						g_aabbVisBounds;
AABB						g_aabbPhysBounds;

void AddVertex(size_t iMaterial, const Vector& v, const Vector2D& vt)
{
	g_aaflData[iMaterial].push_back(v.x);
	g_aaflData[iMaterial].push_back(v.y);
	g_aaflData[iMaterial].push_back(v.z);
	g_aaflData[iMaterial].push_back(vt.x);
	g_aaflData[iMaterial].push_back(vt.y);

	for (int i = 0; i < 3; i++)
	{
		if (v[i] < g_aabbVisBounds.m_vecMins[i])
			g_aabbVisBounds.m_vecMins[i] = v[i];
		if (v[i] > g_aabbVisBounds.m_vecMaxs[i])
			g_aabbVisBounds.m_vecMaxs[i] = v[i];
	}
}

void LoadMeshInstanceIntoToy(CConversionScene* pScene, CConversionMeshInstance* pMeshInstance, const Matrix4x4& mParentTransformations)
{
	if (!pMeshInstance->IsVisible())
		return;

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	for (size_t m = 0; m < pScene->GetNumMaterials(); m++)
	{
		for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
		{
			size_t k;
			CConversionFace* pFace = pMesh->GetFace(j);

			if (pFace->m == ~0)
				continue;

			CConversionMaterial* pMaterial = NULL;
			CConversionMaterialMap* pConversionMaterialMap = pMeshInstance->GetMappedMaterial(pFace->m);

			if (!pConversionMaterialMap)
				continue;

			if (!pConversionMaterialMap->IsVisible())
				continue;

			if (pConversionMaterialMap->m_iMaterial != m)
				continue;

			while (g_asTextures.size() <= pConversionMaterialMap->m_iMaterial)
			{
				g_asTextures.push_back(pScene->GetMaterial(pConversionMaterialMap->m_iMaterial)->GetDiffuseTexture());
				g_aaflData.push_back();
			}

			size_t iMaterial = pConversionMaterialMap->m_iMaterial;

			CConversionVertex* pVertex0 = pFace->GetVertex(0);

			for (k = 2; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex1 = pFace->GetVertex(k-1);
				CConversionVertex* pVertex2 = pFace->GetVertex(k);

				AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex0->v), pMesh->GetUV(pVertex0->vu));
				AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex1->v), pMesh->GetUV(pVertex1->vu));
				AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex2->v), pMesh->GetUV(pVertex2->vu));
			}
		}
	}
}

void LoadSceneNodeIntoToy(CConversionScene* pScene, CConversionSceneNode* pNode, const Matrix4x4& mParentTransformations)
{
	if (!pNode)
		return;

	if (!pNode->IsVisible())
		return;

	Matrix4x4 mTransformations = mParentTransformations;
	if (!CModelLibrary::LoadThisSceneWithLocalTransforms())
		mTransformations = mTransformations * pNode->m_mTransformations;

	for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		LoadSceneNodeIntoToy(pScene, pNode->GetChild(i), mTransformations);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		LoadMeshInstanceIntoToy(pScene, pNode->GetMeshInstance(m), mTransformations);
}

void LoadSceneIntoToy(CConversionScene* pScene)
{
	Matrix4x4 mUpLeftSwap(Vector(1, 0, 0), Vector(0, 0, 1), Vector(0, -1, 0));

	for (size_t i = 0; i < pScene->GetNumScenes(); i++)
		LoadSceneNodeIntoToy(pScene, pScene->GetScene(i), mUpLeftSwap);
}

bool CModel::LoadSourceFile()
{
	CConversionScene* pScene = new CConversionScene();
	CModelConverter c(pScene);

	if (!c.ReadModel(m_sFilename))
	{
		delete pScene;
		return false;
	}

	g_asTextures.clear();
	g_aaflData.clear();
	g_aabbVisBounds = AABB(Vector(999, 999, 999), Vector(-999, -999, -999));
	g_aabbPhysBounds = AABB(Vector(999, 999, 999), Vector(-999, -999, -999));

	LoadSceneIntoToy(pScene);

	size_t iTextures = g_asTextures.size();

	m_ahMaterials.resize(iTextures);
	m_aiVertexBuffers.resize(iTextures);
	m_aiVertexBufferSizes.resize(iTextures);

	for (size_t i = 0; i < iTextures; i++)
	{
		if (g_aaflData[i].size() == 0)
			continue;

		m_aiVertexBuffers[i] = CRenderer::LoadVertexDataIntoGL(g_aaflData[i].size()*4, &g_aaflData[i][0]);
		m_aiVertexBufferSizes[i] = g_aaflData[i].size()/FIXED_FLOATS_PER_VERTEX;

		CData oMaterialData;
		CData* pShader = oMaterialData.AddChild("Shader", "model");
		pShader->AddChild("DiffuseTexture", g_asTextures[i]);
		m_ahMaterials[i] = CMaterialLibrary::AddMaterial(&oMaterialData);

		//TAssert(m_aiMaterials[i]);
		if (!m_ahMaterials[i].IsValid())
			TError(tstring("Couldn't create fake material for texture \"") + g_asTextures[i] + "\"\n");
	}

	m_aabbVisBoundingBox = g_aabbVisBounds;
	m_aabbPhysBoundingBox = g_aabbPhysBounds;

	delete pScene;

	return true;
}

void LoadMesh(CConversionScene* pScene, size_t iMesh)
{
	TAssert(iMesh < pScene->GetNumMeshes());
	if (iMesh >= pScene->GetNumMeshes())
		return;

	// Reserve space for n+1, the last one represents the default material.
	g_aaflData.resize(pScene->GetNumMaterials()+1);

	tvector<Vector> avecPoints;
	tvector<size_t> aiPoints;

	CConversionMesh* pMesh = pScene->GetMesh(iMesh);

	for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
	{
		CConversionFace* pFace = pMesh->GetFace(j);

		size_t iMaterial = pFace->m;
		if (iMaterial == ~0)
			iMaterial = pScene->GetNumMaterials();

		CConversionVertex* pVertex0 = pFace->GetVertex(0);
		CConversionVertex* pVertex1 = pFace->GetVertex(1);

		CConversionVertex* pLastVertex = pFace->GetVertex(pFace->GetNumVertices()-1);

		avecPoints.clear();
		aiPoints.clear();

		for (size_t t = 0; t < pFace->GetNumVertices(); t++)
		{
			avecPoints.push_back(pMesh->GetVertex(pFace->GetVertex(t)->v));
			aiPoints.push_back(t);
		}

		CConversionVertex* pVertex2;

		while (avecPoints.size() > 3)
		{
			size_t iEar = FindEar(avecPoints);
			size_t iLast = iEar==0?avecPoints.size()-1:iEar-1;
			size_t iNext = iEar==avecPoints.size()-1?0:iEar+1;

			pVertex0 = pFace->GetVertex(aiPoints[iLast]);
			pVertex1 = pFace->GetVertex(aiPoints[iEar]);
			pVertex2 = pFace->GetVertex(aiPoints[iNext]);

			AddVertex(iMaterial, pMesh->GetVertex(pVertex0->v), pMesh->GetUV(pVertex0->vu));
			AddVertex(iMaterial, pMesh->GetVertex(pVertex1->v), pMesh->GetUV(pVertex1->vu));
			AddVertex(iMaterial, pMesh->GetVertex(pVertex2->v), pMesh->GetUV(pVertex2->vu));

			avecPoints.erase(avecPoints.begin()+iEar);
			aiPoints.erase(aiPoints.begin()+iEar);
		}

		TAssert(aiPoints.size() == 3);
		if (aiPoints.size() != 3)
			continue;

		pVertex0 = pFace->GetVertex(aiPoints[0]);
		pVertex1 = pFace->GetVertex(aiPoints[1]);
		pVertex2 = pFace->GetVertex(aiPoints[2]);

		AddVertex(iMaterial, pMesh->GetVertex(pVertex0->v), pMesh->GetUV(pVertex0->vu));
		AddVertex(iMaterial, pMesh->GetVertex(pVertex1->v), pMesh->GetUV(pVertex1->vu));
		AddVertex(iMaterial, pMesh->GetVertex(pVertex2->v), pMesh->GetUV(pVertex2->vu));
	}
}

bool CModel::Load(class CConversionScene* pScene, size_t iMesh)
{
	g_aaflData.clear();

	LoadMesh(pScene, iMesh);

	size_t iMaterials = g_aaflData.size();

	for (size_t i = 0; i < iMaterials; i++)
	{
		if (g_aaflData[i].size() == 0)
			continue;

		m_aiVertexBuffers.push_back(CRenderer::LoadVertexDataIntoGL(g_aaflData[i].size()*4, &g_aaflData[i][0]));
		m_aiVertexBufferSizes.push_back(g_aaflData[i].size()/FIXED_FLOATS_PER_VERTEX);
	}

	m_aabbVisBoundingBox = g_aabbVisBounds;
	m_aabbPhysBoundingBox = g_aabbPhysBounds;

	return true;
}
