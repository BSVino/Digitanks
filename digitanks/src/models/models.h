#ifndef DT_MODELS_H
#define DT_MODELS_H

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <modelconverter/convmesh.h>

class CModel
{
public:
							CModel(const eastl::string16& sFilename);
							~CModel();

public:
	size_t					LoadTextureIntoGL(size_t iMaterial);

public:
	eastl::string16			m_sFilename;
	CConversionScene*		m_pScene;

	bool					m_bStatic;
	size_t					m_iCallList;

	eastl::vector<size_t>	m_aiTextures;
};

class CModelLibrary
{
public:
							CModelLibrary();
							~CModelLibrary();

public:
	size_t					GetNumModels() { return m_apModels.size(); };

	size_t					AddModel(const eastl::string16& sModel, bool bStatic = true);
	size_t					FindModel(const eastl::string16& sModel);
	CModel*					GetModel(size_t i);

public:
	static CModelLibrary*	Get() { return s_pModelLibrary; };

protected:
	eastl::vector<CModel*>	m_apModels;

private:
	static CModelLibrary*	s_pModelLibrary;
};

#endif