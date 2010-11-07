#ifndef CF_MODELCONVERTER_H
#define CF_MODELCONVERTER_H

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <iostream>
#include <fstream>

#include <worklistener.h>
#include "convmesh.h"

class CModelConverter
{
public:
						CModelConverter(CConversionScene* pScene);

public:
	bool				ReadModel(const eastl::string16& sFilename);

	void				ReadOBJ(const eastl::string16& sFilename);
	void				ReadMTL(const eastl::string16& sFilename);

	// SIA and its utility functions.
	void				ReadSIA(const eastl::string16& sFilename);
	void				ReadSIAMat(std::wifstream& infile, CConversionSceneNode* pScene, const eastl::string16& sFilename);
	void				ReadSIAShape(std::wifstream& infile, CConversionSceneNode* pScene, bool bCare = true);

	void				ReadDAE(const eastl::string16& sFilename);
	void				ReadDAESceneTree(class FCDSceneNode* pNode, CConversionSceneNode* pScene);

	void				WriteSMDs(const eastl::string16& sFilename = L"");
	void				WriteSMD(size_t iMesh, const eastl::string16& sFilename = L"");

	eastl::string16		GetFilename(const eastl::string16& sFilename);
	eastl::string16		GetDirectory(const eastl::string16& sFilename);
	bool				IsWhitespace(eastl::string16::value_type cChar);
	eastl::string16		StripWhitespace(eastl::string16 sLine);

	void				SetScene(CConversionScene* pScene) { m_pScene = pScene; };
	CConversionScene*	GetScene() { return m_pScene; };

	void				SetWorkListener(IWorkListener* pWorkListener) { m_pWorkListener = pWorkListener; }

protected:
	CConversionScene*	m_pScene;

	IWorkListener*		m_pWorkListener;
};

#endif