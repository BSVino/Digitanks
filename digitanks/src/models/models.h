#ifndef DT_MODELS_H
#define DT_MODELS_H

#include <vector>
#include <modelconverter/convmesh.h>

class CModel
{
public:
							CModel(const wchar_t* pszFilename);
							~CModel();

public:
	size_t					LoadTextureIntoGL(size_t iMaterial);

public:
	std::wstring			m_sFilename;
	CConversionScene*		m_pScene;

	bool					m_bStatic;
	size_t					m_iCallList;

	std::vector<size_t>		m_aiTextures;
};

class CModelLibrary
{
public:
							CModelLibrary();
							~CModelLibrary();

public:
	size_t					GetNumModels() { return m_apModels.size(); };

	size_t					AddModel(const wchar_t* pszFilename, bool bStatic = true);
	size_t					FindModel(const wchar_t* pszFilename);
	CModel*					GetModel(size_t i);

public:
	static CModelLibrary*	Get() { return s_pModelLibrary; };

protected:
	std::vector<CModel*>	m_apModels;

private:
	static CModelLibrary*	s_pModelLibrary;
};

#endif