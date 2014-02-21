#include "geppetto.h"

#include <modelconverter/modelconverter.h>
#include <toys/toy_util.h>

void CGeppetto::LoadMeshInstanceIntoToyPhysics(CConversionScene* pScene, CConversionMeshInstance* pMeshInstance, const Matrix4x4& mParentTransformations, CToyUtil* pToy)
{
	if (!pMeshInstance->IsVisible())
		return;

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	size_t iVertsSize = pToy->GetNumPhysVerts();

	for (size_t v = 0; v < pMesh->GetNumVertices(); v++)
		pToy->AddPhysVertex(mParentTransformations * pMesh->GetVertex(v));

	for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
	{
		size_t k;
		CConversionFace* pFace = pMesh->GetFace(j);

		CConversionVertex* pVertex0 = pFace->GetVertex(0);

		for (k = 2; k < pFace->GetNumVertices(); k++)
		{
			CConversionVertex* pVertex1 = pFace->GetVertex(k-1);
			CConversionVertex* pVertex2 = pFace->GetVertex(k);

			pToy->AddPhysTriangle(iVertsSize + pVertex0->v, iVertsSize + pVertex1->v, iVertsSize + pVertex2->v);
		}
	}
}

void CGeppetto::LoadSceneNodeIntoToyPhysics(CConversionScene* pScene, CConversionSceneNode* pNode, const Matrix4x4& mParentTransformations, CToyUtil* pToy)
{
	if (!pNode)
		return;

	if (!pNode->IsVisible())
		return;

	Matrix4x4 mTransformations = mParentTransformations;

	if (!pToy->IsUsingLocalTransformations())
		mTransformations = mParentTransformations * pNode->m_mTransformations;

	for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		LoadSceneNodeIntoToyPhysics(pScene, pNode->GetChild(i), mTransformations, pToy);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		LoadMeshInstanceIntoToyPhysics(pScene, pNode->GetMeshInstance(m), mTransformations, pToy);
}

void CGeppetto::LoadSceneIntoToyPhysics(CConversionScene* pScene, CToyUtil* pToy)
{
	Matrix4x4 mUpLeftSwap(Vector(1, 0, 0), Vector(0, 0, 1), Vector(0, -1, 0));

	for (size_t i = 0; i < pScene->GetNumScenes(); i++)
		LoadSceneNodeIntoToyPhysics(pScene, pScene->GetScene(i), mUpLeftSwap, pToy);
}

