#include <assert.h>

#include <FCollada.h>
#include <FCDocument/FCDocument.h>
#include <FCDocument/FCDocumentTools.h>
#include <FCDocument/FCDLibrary.h>
#include <FCDocument/FCDSceneNode.h>
#include <FCDocument/FCDTransform.h>
#include <FCDocument/FCDEntityInstance.h>
#include <FCDocument/FCDGeometryInstance.h>
#include <FCDocument/FCDMaterialInstance.h>
#include <FCDocument/FCDMaterial.h>
#include <FCDocument/FCDEffect.h>
#include <FCDocument/FCDEffectProfile.h>
#include <FCDocument/FCDEffectStandard.h>
#include <FCDocument/FCDEffectParameter.h>
#include <FCDocument/FCDEffectParameterSampler.h>
#include <FCDocument/FCDEffectParameterSurface.h>
#include <FCDocument/FCDImage.h>
#include <FCDocument/FCDGeometry.h>
#include <FCDocument/FCDGeometryMesh.h>
#include <FCDocument/FCDGeometrySource.h>
#include <FCDocument/FCDGeometryPolygons.h>
#include <FCDocument/FCDGeometryPolygonsInput.h>

#include "../modelconverter.h"

void CModelConverter::ReadDAE(const wchar_t* pszFilename)
{
	if (m_pWorkListener)
		m_pWorkListener->BeginProgress();

	FCollada::Initialize();

	FCDocument* pDoc = FCollada::NewTopDocument();

	bool bConvertZToY = false;

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Reading file", 0);

	if (FCollada::LoadDocumentFromFile(pDoc, pszFilename))
	{
		size_t i;

		FCDocumentTools::StandardizeUpAxisAndLength(pDoc, FMVector3(0, 1, 0));

		FCDMaterialLibrary* pMatLib = pDoc->GetMaterialLibrary();
		size_t iEntities = pMatLib->GetEntityCount();

		if (m_pWorkListener)
			m_pWorkListener->SetAction(L"Reading materials", iEntities);

		for (i = 0; i < iEntities; ++i)
		{
			FCDMaterial* pColladaMaterial = pMatLib->GetEntity(i);
			FCDEffect* pEffect = pColladaMaterial->GetEffect();

			size_t iMaterial = m_pScene->AddMaterial(pColladaMaterial->GetName().c_str());
			CConversionMaterial* pMaterial = m_pScene->GetMaterial(iMaterial);

			if (pEffect->GetProfileCount() < 1)
				continue;

			FCDEffectProfile* pEffectProfile = pEffect->GetProfile(0);

			FUDaeProfileType::Type eProfileType = pEffectProfile->GetType();
			if (eProfileType == FUDaeProfileType::COMMON)
			{
				FCDEffectStandard* pStandardProfile = dynamic_cast<FCDEffectStandard*>(pEffectProfile);
				if (pStandardProfile)
				{
					pMaterial->m_vecAmbient = Vector((float*)pStandardProfile->GetAmbientColor());
					pMaterial->m_vecDiffuse = Vector((float*)pStandardProfile->GetDiffuseColor());
					pMaterial->m_vecSpecular = Vector((float*)pStandardProfile->GetSpecularColor());
					pMaterial->m_vecEmissive = Vector((float*)pStandardProfile->GetEmissionColor());
					pMaterial->m_flShininess = pStandardProfile->GetShininess();
				}
			}

			for (size_t j = 0; j < pEffectProfile->GetEffectParameterCount(); j++)
			{
				FCDEffectParameter* pEffectParameter = pEffectProfile->GetEffectParameter(j);

				FCDEffectParameter::Type eType = pEffectParameter->GetType();

				if (eType == FCDEffectParameter::SAMPLER)
				{
					FCDEffectParameterSampler* pSampler = dynamic_cast<FCDEffectParameterSampler*>(pEffectParameter);
					if (pSampler)
					{
						FCDEffectParameterSurface* pSurface = pSampler->GetSurface();
						if (pSurface)
						{
							if (pSurface->GetImageCount())
							{
								// Christ Collada why do you have to make things so damn complicated?
								FCDImage* pImage = pSurface->GetImage(0);

								const wchar_t* pszFilename = pImage->GetFilename().c_str();

								// I'm sick of these damn string copy functions.
								pMaterial->m_sDiffuseTexture = pszFilename;
							}
						}
					}
				}
			}

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(i+1);
		}

		FCDGeometryLibrary* pGeoLib = pDoc->GetGeometryLibrary();
		iEntities = pGeoLib->GetEntityCount();

		if (m_pWorkListener)
			m_pWorkListener->SetAction(L"Loading entities", iEntities);

		for (i = 0; i < iEntities; ++i)
		{
			FCDGeometry* pGeometry = pGeoLib->GetEntity(i);
			if (pGeometry->IsMesh())
			{
				size_t j;

				size_t iMesh = m_pScene->AddMesh(pGeometry->GetName().c_str());
				CConversionMesh* pMesh = m_pScene->GetMesh(iMesh);
				pMesh->AddBone(pGeometry->GetName().c_str());

				FCDGeometryMesh* pGeoMesh = pGeometry->GetMesh();
				FCDGeometrySource* pPositionSource = pGeoMesh->GetPositionSource();
				size_t iVertexCount = pPositionSource->GetValueCount();

				for (j = 0; j < iVertexCount; j++)
				{
					const float* pflValues = pPositionSource->GetValue(j);
					pMesh->AddVertex(pflValues[0], pflValues[1], pflValues[2]);
				}

				FCDGeometrySource* pNormalSource = pGeoMesh->FindSourceByType(FUDaeGeometryInput::NORMAL);
				if (pNormalSource)
				{
					iVertexCount = pNormalSource->GetValueCount();
					for (j = 0; j < iVertexCount; j++)
					{
						const float* pflValues = pNormalSource->GetValue(j);
						pMesh->AddNormal(pflValues[0], pflValues[1], pflValues[2]);
					}
				}

				FCDGeometrySource* pUVSource = pGeoMesh->FindSourceByType(FUDaeGeometryInput::TEXCOORD);
				if (pUVSource)
				{
					iVertexCount = pUVSource->GetValueCount();
					for (j = 0; j < iVertexCount; j++)
					{
						const float* pflValues = pUVSource->GetValue(j);
						pMesh->AddUV(pflValues[0], pflValues[1]);
					}
				}

				for (j = 0; j < pGeoMesh->GetPolygonsCount(); j++)
				{
					FCDGeometryPolygons* pPolygons = pGeoMesh->GetPolygons(j);
					FCDGeometryPolygonsInput* pPositionInput = pPolygons->FindInput(FUDaeGeometryInput::POSITION);
					FCDGeometryPolygonsInput* pNormalInput = pPolygons->FindInput(FUDaeGeometryInput::NORMAL);
					FCDGeometryPolygonsInput* pUVInput = pPolygons->FindInput(FUDaeGeometryInput::TEXCOORD);

					size_t iPositionCount = pPositionInput->GetIndexCount();
					uint32* pPositions = pPositionInput->GetIndices();

					size_t iNormalCount = 0;
					uint32* pNormals = NULL;

					if (pNormalInput)
					{
						iNormalCount = pNormalInput->GetIndexCount();
						pNormals = pNormalInput->GetIndices();
					}

					size_t iUVCount = 0;
					uint32* pUVs = NULL;

					if (pUVInput)
					{
						iUVCount = pUVInput->GetIndexCount();
						pUVs = pUVInput->GetIndices();
					}

					fm::stringT<fchar> sMaterial = pPolygons->GetMaterialSemantic();

					size_t iCurrentMaterial = pMesh->AddMaterialStub(sMaterial.c_str());

					if (pPolygons->TestPolyType() == 3)
					{
						// All triangles!
						for (size_t k = 0; k < iPositionCount; k+=3)
						{
							size_t iFace = pMesh->AddFace(iCurrentMaterial);

							pMesh->AddVertexToFace(iFace, pPositions[k+0], pUVs?pUVs[k+0]:~0, pNormals?pNormals[k+0]:~0);
							pMesh->AddVertexToFace(iFace, pPositions[k+1], pUVs?pUVs[k+1]:~0, pNormals?pNormals[k+1]:~0);
							pMesh->AddVertexToFace(iFace, pPositions[k+2], pUVs?pUVs[k+2]:~0, pNormals?pNormals[k+2]:~0);
						}
					}
					else if (pPolygons->TestPolyType() == 4)
					{
						// All quads!
						for (size_t k = 0; k < iPositionCount; k+=4)
						{
							size_t iFace = pMesh->AddFace(iCurrentMaterial);

							pMesh->AddVertexToFace(iFace, pPositions[k+0], pUVs?pUVs[k+0]:~0, pNormals?pNormals[k+0]:~0);
							pMesh->AddVertexToFace(iFace, pPositions[k+1], pUVs?pUVs[k+1]:~0, pNormals?pNormals[k+1]:~0);
							pMesh->AddVertexToFace(iFace, pPositions[k+2], pUVs?pUVs[k+2]:~0, pNormals?pNormals[k+2]:~0);
							pMesh->AddVertexToFace(iFace, pPositions[k+3], pUVs?pUVs[k+3]:~0, pNormals?pNormals[k+3]:~0);
						}
					}
					else
					{
						size_t iFaces = pPolygons->GetFaceCount();
						for (size_t k = 0; k < iFaces; k++)
						{
							size_t iFace = pMesh->AddFace(iCurrentMaterial);
							size_t o = pPolygons->GetFaceVertexOffset(k);
							size_t iFaceVertexCount = pPolygons->GetFaceVertexCount(k);
							for (size_t v = 0; v < iFaceVertexCount; v++)
								pMesh->AddVertexToFace(iFace, pPositions[o+v], pUVs?pUVs[o+v]:~0, pNormals?pNormals[o+v]:~0);
						}
					}
				}
			}

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(i+1);
		}

		FCDVisualSceneNodeLibrary* pVisualScenes = pDoc->GetVisualSceneLibrary();
		iEntities = pVisualScenes->GetEntityCount();
		for (i = 0; i < iEntities; ++i)
		{
			FCDSceneNode* pNode = pVisualScenes->GetEntity(i);

			size_t iScene = m_pScene->AddScene(pNode->GetName().c_str());
			ReadDAESceneTree(pNode, m_pScene->GetScene(iScene));
		}
	}
	else
		printf("Oops! Some kind of error happened!\n");

	m_pScene->SetWorkListener(m_pWorkListener);

	m_pScene->CalculateExtends();

	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
	{
		m_pScene->GetMesh(i)->CalculateEdgeData();

		if (m_pScene->GetMesh(i)->GetNumNormals() == 0)
			m_pScene->GetMesh(i)->CalculateVertexNormals();

		m_pScene->GetMesh(i)->CalculateVertexTangents();
	}

	pDoc->Release();

	FCollada::Release();

	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

void CModelConverter::ReadDAESceneTree(FCDSceneNode* pNode, CConversionSceneNode* pScene)
{
	size_t iTransforms = pNode->GetTransformCount();

	pScene->m_mTransformations.Identity();

	for (size_t t = 0; t < iTransforms; t++)
	{
		FCDTransform* pTransform = pNode->GetTransform(t);
		FMMatrix44 m = pTransform->ToMatrix();
		pScene->m_mTransformations *= Matrix4x4(m.Transposed());	// Transpose because they use a different convention.
	}

	size_t iInstances = pNode->GetInstanceCount();

	for (size_t i = 0; i < iInstances; i++)
	{
		FCDEntityInstance* pInstance = pNode->GetInstance(i);

		switch (pInstance->GetType())
		{
		case FCDEntityInstance::GEOMETRY:
		{
			FCDGeometryInstance* pGeometryInstance = dynamic_cast<FCDGeometryInstance*>(pInstance);
			FCDEntity* pEntity = pGeometryInstance->GetEntity();
			size_t iMesh = pScene->m_pScene->FindMesh(pEntity->GetName().c_str());
			size_t iMeshInstance = pScene->AddMeshInstance(iMesh);

			size_t iMaterialInstances = pGeometryInstance->GetMaterialInstanceCount();
			for (size_t m = 0; m < iMaterialInstances; m++)
			{
				FCDMaterialInstance* pMaterialInstance = pGeometryInstance->GetMaterialInstance(m);
				std::wstring sMaterial = pMaterialInstance->GetMaterial()->GetName().c_str();
				std::wstring sMaterialStub = pMaterialInstance->GetSemantic().c_str();

				size_t iMaterial = pScene->m_pScene->FindMaterial(sMaterial);
				size_t iMaterialStub = pScene->m_pScene->GetMesh(iMesh)->FindMaterialStub(sMaterialStub);

				pScene->GetMeshInstance(iMeshInstance)->AddMappedMaterial(iMaterialStub, iMaterial);
			}
		}
		}
	}

	size_t iChildren = pNode->GetChildrenCount();
	for (size_t j = 0; j < iChildren; j++)
	{
		FCDSceneNode* pChildNode = pNode->GetChild(j);
		size_t iNode = pScene->AddChild(pChildNode->GetName().c_str());
		ReadDAESceneTree(pChildNode, pScene->GetChild(iNode));
	}
}
