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
	std::wstring			m_sFilename;
	CConversionScene*		m_pScene;
};

class CModelLibrary
{
public:
							CModelLibrary();
							~CModelLibrary();

public:
	size_t					GetNumModels() { return m_apModels.size(); };

	size_t					AddModel(const wchar_t* pszFilename);
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