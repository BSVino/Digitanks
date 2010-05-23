#include "models.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>
#include <IL/ilu.h>
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

size_t CModelLibrary::AddModel(const wchar_t* pszFilename, bool bStatic)
{
	size_t iModel = FindModel(pszFilename);
	if (iModel != ~0)
		return iModel;

	m_apModels.push_back(new CModel(pszFilename));

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

size_t CModelLibrary::FindModel(const wchar_t* pszFilename)
{
	for (size_t i = 0; i < m_apModels.size(); i++)
	{
		if (wcscmp(m_apModels[i]->m_sFilename.c_str(), pszFilename) == 0)
			return i;
	}

	return ~0;
}

CModel::CModel(const wchar_t* pszFilename)
{
	m_sFilename = pszFilename;
	m_pScene = new CConversionScene();
	m_bStatic = false;
	m_iCallList = 0;
	CModelConverter c(m_pScene);
	c.ReadModel(pszFilename);

	m_aiTextures.resize(m_pScene->GetNumMaterials());
	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
		m_aiTextures[i] = LoadTextureIntoGL(i);
}

CModel::~CModel()
{
	delete m_pScene;
}

size_t CModel::LoadTextureIntoGL(size_t iMaterial)
{
	CConversionMaterial* pMaterial = m_pScene->GetMaterial(iMaterial);

	std::wstring sFilename = pMaterial->GetDiffuseTexture();

	if (!sFilename.length())
		return 0;

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilLoadImage(sFilename.c_str());

	if (!bSuccess)
		bSuccess = ilLoadImage(sFilename.c_str());

	ILenum iError = ilGetError();

	if (!bSuccess)
		return 0;

	bSuccess = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!bSuccess)
		return 0;

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);

	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		iluFlipImage();

	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D,
		ilGetInteger(IL_IMAGE_BPP),
		ilGetInteger(IL_IMAGE_WIDTH),
		ilGetInteger(IL_IMAGE_HEIGHT),
		ilGetInteger(IL_IMAGE_FORMAT),
		GL_UNSIGNED_BYTE,
		ilGetData());

	ilDeleteImages(1, &iDevILId);

	return iGLId;
}
