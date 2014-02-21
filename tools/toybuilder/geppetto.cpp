#include "geppetto.h"

#include <stdio.h>

#include <tinker_platform.h>
#include <files.h>

#include <tinker/shell.h>
#include <modelconverter/modelconverter.h>

void CGeppetto::LoadMeshInstanceIntoToy(CConversionScene* pScene, CConversionMeshInstance* pMeshInstance, const Matrix4x4& mParentTransformations, CToyUtil* pToy)
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

			while (pToy->GetNumMaterials() <= pConversionMaterialMap->m_iMaterial)
				pToy->AddMaterial(pScene->GetMaterial(pConversionMaterialMap->m_iMaterial)->GetName());

			size_t iMaterial = pConversionMaterialMap->m_iMaterial;

			CConversionVertex* pVertex0 = pFace->GetVertex(0);

			for (k = 2; k < pFace->GetNumVertices(); k++)
			{
				CConversionVertex* pVertex1 = pFace->GetVertex(k-1);
				CConversionVertex* pVertex2 = pFace->GetVertex(k);

				pToy->AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex0->v), pMesh->GetUV(pVertex0->vu), Matrix4x4(pMesh->GetNormal(pVertex0->vn), pMesh->GetTangent(pVertex0->vt), pMesh->GetBitangent(pVertex0->vb)));
				pToy->AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex1->v), pMesh->GetUV(pVertex1->vu), Matrix4x4(pMesh->GetNormal(pVertex1->vn), pMesh->GetTangent(pVertex1->vt), pMesh->GetBitangent(pVertex1->vb)));
				pToy->AddVertex(iMaterial, mParentTransformations * pMesh->GetVertex(pVertex2->v), pMesh->GetUV(pVertex2->vu), Matrix4x4(pMesh->GetNormal(pVertex2->vn), pMesh->GetTangent(pVertex2->vt), pMesh->GetBitangent(pVertex2->vb)));
			}
		}
	}
}

void CGeppetto::LoadSceneNodeIntoToy(CConversionScene* pScene, CConversionSceneNode* pNode, const Matrix4x4& mParentTransformations, CToyUtil* pToy)
{
	if (!pNode)
		return;

	if (!pNode->IsVisible())
		return;

	Matrix4x4 mTransformations = mParentTransformations;

	if (!pToy->IsUsingLocalTransformations())
		mTransformations = mParentTransformations * pNode->m_mTransformations;

	for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		LoadSceneNodeIntoToy(pScene, pNode->GetChild(i), mTransformations, pToy);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		LoadMeshInstanceIntoToy(pScene, pNode->GetMeshInstance(m), mTransformations, pToy);
}

void CGeppetto::LoadSceneIntoToy(CConversionScene* pScene, CToyUtil* pToy)
{
	Matrix4x4 mUpLeftSwap(Vector(1, 0, 0), Vector(0, 0, 1), Vector(0, -1, 0));

	for (size_t i = 0; i < pScene->GetNumScenes(); i++)
		LoadSceneNodeIntoToy(pScene, pScene->GetScene(i), mUpLeftSwap, pToy);
}

CGeppetto::CGeppetto(bool bForce, const tstring& sCWD)
{
	m_iBinaryModificationTime = GetFileModificationTime(Shell()->GetBinaryName().c_str());
	if (!m_iBinaryModificationTime)
		m_iBinaryModificationTime = GetFileModificationTime((Shell()->GetBinaryName() + ".exe").c_str());

	m_bForceCompile = bForce;

	m_sCWD = sCWD;
	if (m_sCWD.length() && m_sCWD.back() != '/' && m_sCWD.back() != '\\')
		m_sCWD.append("/");
}

bool CGeppetto::BuildFiles(const tstring& sOutput, const tstring& sInput, const tstring& sPhysics, bool bGlobalTransforms)
{
	if (bGlobalTransforms)
		t.UseGlobalTransformations();

	m_sOutput = FindAbsolutePath(sOutput);
	t.SetOutputDirectory(m_sOutput.substr(0, m_sOutput.rfind(DIR_SEP)));

	time_t iInputModificationTime = GetFileModificationTime(sInput.c_str());
	time_t iOutputModificationTime = GetFileModificationTime(m_sOutput.c_str());
	time_t iPhysicsModificationTime = GetFileModificationTime(sPhysics.c_str());

	bool bRecompile = false;
	if (iInputModificationTime > iOutputModificationTime)
		bRecompile = true;
	else if (iPhysicsModificationTime > iOutputModificationTime)
		bRecompile = true;
	else if (m_iBinaryModificationTime > iOutputModificationTime)
		bRecompile = true;

	if (!bRecompile)
	{
		if (m_bForceCompile)
		{
			TMsg("Forcing rebuild even though no changes detected.\n");
		}
		else
		{
			TMsg("No changes detected. Skipping '" + m_sOutput  + "'.\n\n");
			return true;
		}
	}

	LoadFromFiles(sInput, sPhysics);

	return Compile();
}

bool CGeppetto::Compile()
{
	TMsg(sprintf(" Mesh materials: %d\n", t.GetNumMaterials()));
	TMsg(sprintf(" Mesh tris: %d\n", t.GetNumVerts()/3));
	TMsg(sprintf(" Physics tris: %d\n", t.GetNumPhysIndices()/3));
	TMsg(sprintf(" Scene areas: %d\n", t.GetNumSceneAreas()));
	if (t.IsUsingUV())
		TMsg(" Using UV's\n");
	if (t.IsUsingNormals())
		TMsg(" Using normals\n");

	TMsg(sprintf("Writing toy '" + m_sOutput + "' ..."));

	if (!t.Write(m_sOutput))
	{
		TMsg("\nError writing to file.\n");
		return false;
	}

	TMsg(" Done.\n");

	TMsg("Toy built successfully.\n\n");

	return true;
}

void CGeppetto::LoadFromFiles(const tstring& sMesh, const tstring& sPhysics)
{
	TMsg("Reading model '" + sMesh + "' ...");
	std::shared_ptr<CConversionScene> pScene(new CConversionScene());
	CModelConverter c(pScene.get());

	c.ReadModel(GetPath(sMesh));
	TMsg(" Done.\n");

	t.UseUV();
	t.UseNormals();

	TMsg("Building toy mesh ...");
	LoadSceneIntoToy(pScene.get(), &t);
	TMsg(" Done.\n");

	pScene.reset();

	if (sPhysics.length())
	{
		TMsg("Reading physics model '" + sPhysics + "' ...");
		std::shared_ptr<CConversionScene> pScene(new CConversionScene());
		CModelConverter c(pScene.get());

		c.ReadModel(GetPath(sPhysics));
		TMsg(" Done.\n");

		TMsg("Building toy physics model ...");
		LoadSceneIntoToyPhysics(pScene.get(), &t);
		TMsg(" Done.\n");
	}
}

tstring CGeppetto::GetPath(const tstring& sPath)
{
	if (IsAbsolutePath(sPath))
		return sPath;

	return m_sCWD + sPath;
}
