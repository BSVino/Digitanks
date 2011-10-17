#ifndef DT_MODELS_H
#define DT_MODELS_H

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <modelconverter/convmesh.h>

class CModel
{
public:
							CModel(const tstring& sFilename);
							~CModel();

public:
	size_t					LoadTextureIntoGL(size_t iMaterial);

public:
	tstring			m_sFilename;
	CConversionScene*		m_pScene;

	bool					m_bStatic;
	size_t					m_iCallList;
	size_t					m_iCallListTexture;

	eastl::vector<size_t>	m_aiTextures;
};

class CModelLibrary
{
public:
							CModelLibrary();
							~CModelLibrary();

public:
	static size_t			GetNumModels() { return Get()->m_apModels.size(); }

	size_t					AddModel(const tstring& sModel, bool bStatic = true);
	size_t					FindModel(const tstring& sModel);
	CModel*					GetModel(size_t i);

public:
	static CModelLibrary*	Get() { return s_pModelLibrary; };

protected:
	eastl::vector<CModel*>	m_apModels;

private:
	static CModelLibrary*	s_pModelLibrary;
};

#endif