#include "models.h"

#include <modelconverter/modelconverter.h>
#include <renderer/renderer.h>

CModelLibrary* CModelLibrary::s_pModelLibrary = NULL;
static CModelLibrary g_ModelLibrary = CModelLibrary();

CModelLibrary::CModelLibrary()
{
	s_pModelLibrary = this;
}

CModelLibrary::~CModelLibrary()
{
	for (size_t i = 0; i < m_apModels.size(); i++)
	{
		delete m_apModels[i];
	}

	s_pModelLibrary = NULL;
}

size_t CModelLibrary::AddModel(const eastl::string16& sModel, bool bStatic)
{
	size_t iModel = FindModel(sModel);
	if (iModel != ~0)
		return iModel;

	m_apModels.push_back(new CModel(sModel));

	iModel = m_apModels.size()-1;
	m_apModels[iModel]->m_iCallList = CRenderer::CreateCallList(iModel);
	m_apModels[iModel]->m_bStatic = bStatic;
	return iModel;
}

CModel* CModelLibrary::GetModel(size_t i)
{
	if (i >= m_apModels.size())
		return NULL;

	return m_apModels[i];
}

size_t CModelLibrary::FindModel(const eastl::string16& sModel)
{
	for (size_t i = 0; i < m_apModels.size(); i++)
	{
		if (m_apModels[i]->m_sFilename == sModel)
			return i;
	}

	return ~0;
}

CModel::CModel(const eastl::string16& sFilename)
{
	m_sFilename = sFilename;
	m_pScene = new CConversionScene();
	m_bStatic = false;
	m_iCallList = 0;
	CModelConverter c(m_pScene);
	c.ReadModel(sFilename);

	m_aiTextures.resize(m_pScene->GetNumMaterials());
	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		m_aiTextures[i] = LoadTextureIntoGL(i);
		m_pScene->GetMaterial(i)->m_vecDiffuse = Vector(1, 1, 1);
	}
}

CModel::~CModel()
{
	delete m_pScene;
}

size_t CModel::LoadTextureIntoGL(size_t iMaterial)
{
	return CRenderer::LoadTextureIntoGL(m_pScene->GetMaterial(iMaterial)->GetDiffuseTexture());
}
