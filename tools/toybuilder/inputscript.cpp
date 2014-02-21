#include "geppetto.h"

#include <tinker_platform.h>
#include <files.h>
#include <stb_image.h>

#include <tinker/shell.h>
#include <datamanager/dataserializer.h>
#include <modelconverter/modelconverter.h>
#include <toys/toy_util.h>
#include <tinker/textures/materiallibrary.h>

bool CGeppetto::LoadSceneAreas(CData* pData)
{
	tmap<tstring, std::shared_ptr<CConversionScene> > asScenes;
	tmap<tstring, size_t> aiSceneIDs;

	size_t iSceneArea = 0;

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pArea = pData->GetChild(i);

		if (pArea->GetKey() == "NeighborDistance")
		{
			t.SetNeighborDistance(pArea->GetValueFloat());
			continue;
		}

		if (pArea->GetKey() == "UseGlobalTransforms")
		{
			t.UseGlobalTransformations();
			continue;
		}

		if (pArea->GetKey() == "UseLocalTransforms")
		{
			t.UseLocalTransformations();
			continue;
		}

		TAssert(pArea->GetKey() == "Area");
		if (pArea->GetKey() != "Area")
			continue;

		tstring sFile = pArea->FindChildValueString("File");
		TAssert(sFile.length());
		if (!sFile.length())
			continue;

		tstring sMesh = pArea->FindChildValueString("Mesh");
		TAssert(sMesh.length());
		if (!sMesh.length())
			continue;

		tstring sPhysics = pArea->FindChildValueString("Physics");

		auto it = asScenes.find(sFile);
		if (it == asScenes.end())
		{
			TMsg("Reading model '" + GetPath(sFile) + "' ...");
			std::shared_ptr<CConversionScene> pScene(new CConversionScene());
			CModelConverter c(pScene.get());

			if (!c.ReadModel(GetPath(sFile)))
			{
				TError("Couldn't read '" + GetPath(sFile) + "'.\n");
				return false;
			}
			TMsg(" Done.\n");

			asScenes[sFile] = pScene;
		}

		iSceneArea++;

		CToyUtil ts;

		if (t.IsUsingUV())
			ts.UseUV();
		if (t.IsUsingNormals())
			ts.UseNormals();

		ts.SetGameDirectory(t.GetGameDirectory());
		ts.SetOutputDirectory(t.GetOutputDirectory());
		ts.SetOutputFile(sprintf(t.GetOutputFile() + "_sa%d_" + pArea->GetValueString().tolower(), iSceneArea));
		ts.UseLocalTransformations(t.IsUsingLocalTransformations());

		CConversionSceneNode* pMeshNode = asScenes[sFile]->FindSceneNode(sMesh);
		CConversionSceneNode* pPhysicsNode = nullptr;
		if (sPhysics.length())
		{
			pPhysicsNode = asScenes[sFile]->FindSceneNode(sPhysics);
		
			if (!pPhysicsNode)
				TError("Couldn't find a scene node in '" + sFile + "' named '" + sMesh + "'\n");
		}

		TAssert(pMeshNode);

		TMsg("Building scene area toy ...");

		Matrix4x4 mUpLeftSwap(Vector(1, 0, 0), Vector(0, 0, 1), Vector(0, -1, 0));

		if (pMeshNode)
			LoadSceneNodeIntoToy(asScenes[sFile].get(), pMeshNode, mUpLeftSwap, &ts);
		else
			TError("Couldn't find a scene node in '" + sFile + "' named '" + sMesh + "'\n");

		if (pPhysicsNode)
			LoadSceneNodeIntoToyPhysics(asScenes[sFile].get(), pPhysicsNode, mUpLeftSwap, &ts);

		TMsg(" Done.\n");

		tstring sGameOutput = pArea->FindChildValueString("Output");
		if (!sGameOutput.length())
			sGameOutput = t.GetOutputDirectory() + "/" + ts.GetOutputFile() + ".toy";

		tstring sFileOutput = FindAbsolutePath(t.GetGameDirectory() + DIR_SEP + sGameOutput);

		TMsg(sprintf(" Mesh materials: %d\n", ts.GetNumMaterials()));
		TMsg(sprintf(" Mesh tris: %d\n", ts.GetNumVerts()/3));
		TMsg(sprintf(" Physics tris: %d\n", ts.GetNumPhysIndices()/3));
		if (t.IsUsingUV())
			TMsg(" Using UV's\n");
		if (t.IsUsingNormals())
			TMsg(" Using normals\n");

		TMsg("Writing scene area toy '" + sFileOutput + "' ...");
		if (ts.Write(sFileOutput))
			TMsg(" Done.\n");
		else
			TMsg(" FAILED!\n");

		aiSceneIDs[pArea->GetValueString()] = t.AddSceneArea(sGameOutput);
	}

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pArea = pData->GetChild(i);

		if (pArea->GetKey() != "Area")
			continue;

		size_t iArea = aiSceneIDs[pArea->GetValueString()];

		for (size_t i = 0; i < pArea->GetNumChildren(); i++)
		{
			CData* pNeighbor = pArea->GetChild(i);

			if (pNeighbor->GetKey() == "Neighbor")
			{
				TAssert(aiSceneIDs.find(pNeighbor->GetValueString()) != aiSceneIDs.end());
				if (aiSceneIDs.find(pNeighbor->GetValueString()) == aiSceneIDs.end())
				{
					TError("Couldn't find area \"" + pNeighbor->GetValueString() + "\"\n");
					continue;
				}

				t.AddSceneAreaNeighbor(iArea, aiSceneIDs[pNeighbor->GetValueString()]);
				continue;
			}

			if (pNeighbor->GetKey() == "Visible")
			{
				TAssert(aiSceneIDs.find(pNeighbor->GetValueString()) != aiSceneIDs.end());
				if (aiSceneIDs.find(pNeighbor->GetValueString()) == aiSceneIDs.end())
				{
					TError("Couldn't find area \"" + pNeighbor->GetValueString() + "\"\n");
					continue;
				}

				t.AddSceneAreaVisible(iArea, aiSceneIDs[pNeighbor->GetValueString()]);
				continue;
			}
		}
	}

	return true;
}

bool CGeppetto::BuildFromInputScript(const tstring& sScript)
{
	std::basic_ifstream<tchar> f((GetPath(sScript)).c_str());
	if (!f.is_open())
	{
		TError("Could not read input script '" + sScript + "'\n");
		return false;
	}

	std::shared_ptr<CData> pData(new CData());
	CDataSerializer::Read(f, pData.get());

	CData* pOutput = pData->FindChild("Output");
	if (!pOutput)
	{
		TError("Could not find Output section in input script '" + sScript + "'\n");
		return false;
	}

	CData* pGame = pData->FindChild("Game");
	if (!pGame)
	{
		TError("Could not find Game section in input script '" + sScript + "'\n");
		return false;
	}

	t.SetGameDirectory(FindAbsolutePath(GetPath(pGame->GetValueString())));

	tstring sOutputDir = ToForwardSlashes(pOutput->GetValueString());
	t.SetOutputDirectory(GetDirectory(sOutputDir));
	t.SetOutputFile(GetFilename(sOutputDir));
	t.SetScriptDirectory(GetDirectory((GetPath(sScript))));

	m_sOutput = FindAbsolutePath(t.GetGameDirectory() + DIR_SEP + pOutput->GetValueString());

	CData* pSceneAreas = pData->FindChild("SceneAreas");
	CData* pMesh = pData->FindChild("Mesh");
	CData* pPhysics = pData->FindChild("Physics");
	CData* pPhysicsShapes = pData->FindChild("PhysicsShapes");

	// Find all file modification times.
	time_t iScriptModificationTime = GetFileModificationTime(sScript.c_str());
	time_t iOutputModificationTime = GetFileModificationTime(m_sOutput.c_str());

	tmap<tstring, time_t> aiSceneModificationTimes;

	if (pSceneAreas)
	{
		for (size_t i = 0; i < pSceneAreas->GetNumChildren(); i++)
		{
			CData* pArea = pSceneAreas->GetChild(i);

			if (pArea->GetKey() != "Area")
				continue;

			tstring sFile = pArea->FindChildValueString("File");
			TAssert(sFile.length());
			if (!sFile.length())
				continue;

			auto it = aiSceneModificationTimes.find(sFile);
			if (it == aiSceneModificationTimes.end())
				aiSceneModificationTimes[sFile] = GetFileModificationTime(sFile.c_str());
		}
	}

	time_t iInputModificationTime = 0;
	if (pMesh)
		iInputModificationTime = GetFileModificationTime(pMesh->GetValueString().c_str());
	time_t iPhysicsModificationTime = 0;
	if (pPhysics)
		iPhysicsModificationTime = GetFileModificationTime(pPhysics->GetValueString().c_str());

	bool bRecompile = false;
	if (iScriptModificationTime > iOutputModificationTime)
		bRecompile = true;
	else if (iInputModificationTime > iOutputModificationTime)
		bRecompile = true;
	else if (iPhysicsModificationTime > iOutputModificationTime)
		bRecompile = true;
	else if (m_iBinaryModificationTime > iOutputModificationTime)
		bRecompile = true;
	else
	{
		for (auto it = aiSceneModificationTimes.begin(); it != aiSceneModificationTimes.end(); it++)
		{
			if (it->second > iOutputModificationTime)
			{
				bRecompile = true;
				break;
			}
		}
	}

	if (!bRecompile)
	{
		if (m_bForceCompile)
		{
			TMsg("Forcing rebuild even though no changes detected.\n");
		}
		else
		{
			TMsg("No changes detected. Skipping '" + m_sOutput + "'.\n\n");
			return true;
		}
	}

	CData* pGlobalTransforms = pData->FindChild("UseGlobalTransforms");
	if (pGlobalTransforms)
		t.UseGlobalTransformations();
	else
		t.UseLocalTransformations();

	t.UseUV();
	t.UseNormals();

	if (pMesh)
	{
		tstring sExtension = pMesh->GetValueString().substr(pMesh->GetValueString().length()-4);
		if (sExtension == ".png")
		{
			TUnimplemented();	// Not updated since the switch to materials.
			int x, y, n;
			unsigned char* pData = stbi_load((GetPath(pMesh->GetValueString())).c_str(), &x, &y, &n, 0);

			if (!pData)
			{
				TError("Couldn't load '" + pMesh->GetValueString() + "', reason: " + stbi_failure_reason() + "\n");
				return false;
			}

			stbi_image_free(pData); // Don't need it, just need the dimensions.

			Vector vecUp = Vector(0, 0, 0.5f) * ((float)y/100);
			Vector vecLeft = Vector(0, 0.5f, 0) * ((float)x/100);

			t.UseNormals(false);

			if (IsAbsolutePath(pMesh->GetValueString()))
				t.AddMaterial(GetPath(pMesh->GetValueString()));
			else
				t.AddMaterial(t.GetOutputDirectory() + "/" + pMesh->GetValueString(), GetPath(pMesh->GetValueString()));
			t.AddVertex(0, -vecLeft + vecUp, Vector2D(0.0f, 1.0f));
			t.AddVertex(0, -vecLeft - vecUp, Vector2D(0.0f, 0.0f));
			t.AddVertex(0, vecLeft - vecUp, Vector2D(1.0f, 0.0f));

			t.AddVertex(0, -vecLeft + vecUp, Vector2D(0.0f, 1.0f));
			t.AddVertex(0, vecLeft - vecUp, Vector2D(1.0f, 0.0f));
			t.AddVertex(0, vecLeft + vecUp, Vector2D(1.0f, 1.0f));
		}
		else if (sExtension == ".mat")
		{
			CMaterialHandle hMaterial(pMesh->GetValueString());
			if (!hMaterial.IsValid())
			{
				TError("Input material  '" + pMesh->GetValueString() + "' does not exist or is invalid.\n");
				return false;
			}

			if (!hMaterial->m_ahTextures.size())
			{
				TError("Input material  '" + pMesh->GetValueString() + "' has no textures.\n");
				return false;
			}

			float w = (float)hMaterial->m_ahTextures[0]->m_iWidth;
			float h = (float)hMaterial->m_ahTextures[0]->m_iHeight;

			Vector vecUp = Vector(0, 0.5f, 0) * (h/hMaterial->m_iTexelsPerMeter);
			Vector vecRight = Vector(0, 0, 0.5f) * (w/hMaterial->m_iTexelsPerMeter);

			t.UseNormals(false);

			t.AddMaterial(pMesh->GetValueString());

			t.AddVertex(0, -vecRight + vecUp, Vector2D(0.0f, 1.0f));
			t.AddVertex(0, -vecRight - vecUp, Vector2D(0.0f, 0.0f));
			t.AddVertex(0, vecRight - vecUp, Vector2D(1.0f, 0.0f));

			t.AddVertex(0, -vecRight + vecUp, Vector2D(0.0f, 1.0f));
			t.AddVertex(0, vecRight - vecUp, Vector2D(1.0f, 0.0f));
			t.AddVertex(0, vecRight + vecUp, Vector2D(1.0f, 1.0f));
		}
		else
		{
			TMsg("Reading model '" + GetPath(pMesh->GetValueString()) + "' ...");
			std::shared_ptr<CConversionScene> pScene(new CConversionScene());
			CModelConverter c(pScene.get());

			if (!c.ReadModel(GetPath(pMesh->GetValueString())))
			{
				TError("Couldn't read '" + GetPath(pMesh->GetValueString()) + "'.\n");
				return false;
			}
			TMsg(" Done.\n");

			TMsg("Building toy mesh ...");
			LoadSceneIntoToy(pScene.get(), &t);
			TMsg(" Done.\n");
		}
	}

	if (pPhysics)
	{
		TMsg("Reading physics model '" + GetPath(pPhysics->GetValueString()) + "' ...");
		std::shared_ptr<CConversionScene> pScene(new CConversionScene());
		CModelConverter c(pScene.get());

		if (!c.ReadModel(GetPath(pPhysics->GetValueString())))
		{
			TError("Couldn't read '" + GetPath(pPhysics->GetValueString()) + "'.\n");
			return false;
		}
		TMsg(" Done.\n");

		TMsg("Building toy physics model ...");
		LoadSceneIntoToyPhysics(pScene.get(), &t);
		TMsg(" Done.\n");
	}

	if (pPhysicsShapes)
	{
		for (size_t i = 0; i < pPhysicsShapes->GetNumChildren(); i++)
		{
			CData* pShape = pPhysicsShapes->GetChild(i);

			TAssert(pShape->GetKey() == "Box");
			if (pShape->GetKey() != "Box")
				continue;

			TRS trs = pShape->GetValueTRS();

			t.AddPhysBox(trs);
		}
	}

	if (pSceneAreas)
		LoadSceneAreas(pSceneAreas);

	return Compile();
}
