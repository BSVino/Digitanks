#ifndef CF_MODELCONVERTER_H
#define CF_MODELCONVERTER_H

#include <iostream>
#include <fstream>
#include <vector>

#include <worklistener.h>
#include "convmesh.h"

class CModelConverter
{
public:
						CModelConverter(CConversionScene* pScene);

	void				ReadOBJ(const wchar_t* pszFilename);
	void				ReadMTL(const wchar_t* pszFilename);

	// SIA and its utility functions.
	void				ReadSIA(const wchar_t* pszFilename);
	void				ReadSIAMat(std::wifstream& infile, const wchar_t* pszFilename);
	void				ReadSIAShape(std::wifstream& infile, bool bCare = true);

	void				ReadDAE(const wchar_t* pszFilename);

	void				WriteSMDs(const wchar_t* pszFilename = NULL);
	void				WriteSMD(size_t iMesh, const wchar_t* pszFilename = NULL);

	wchar_t*			MakeFilename(wchar_t* pszPathFilename);
	wchar_t*			GetDirectory(wchar_t* pszFilename);
	bool				IsWhitespace(wchar_t cChar);
	wchar_t*			StripWhitespace(wchar_t* pszLine);
	std::wstring		StripWhitespace(std::wstring sLine);

	void				SetScene(CConversionScene* pScene) { m_pScene = pScene; };
	CConversionScene*	GetScene() { return m_pScene; };

	void				SetWorkListener(IWorkListener* pWorkListener) { m_pWorkListener = pWorkListener; }

protected:
	CConversionScene*	m_pScene;

	IWorkListener*		m_pWorkListener;
};

#endif