#ifndef CF_MODELCONVERTER_H
#define CF_MODELCONVERTER_H

#include <iostream>
#include <fstream>
#include <vector>

#include "convmesh.h"

class CModelConverter
{
public:
	void				ReadOBJ(const char* pszFilename);
	void				ReadMTL(const char* pszFilename);

	// SIA and its utility functions.
	void				ReadSIA(const char* pszFilename);
	void				ReadSIAMat(std::ifstream& infile, const char* pszFilename);
	void				ReadSIAShape(std::ifstream& infile, bool bCare = true);

	void				ReadDAE(const char* pszFilename);

	void				WriteSMDs();
	void				WriteSMD(size_t iMesh);

	char*				MakeFilename(char* pszPathFilename);
	char*				GetDirectory(char* pszFilename);
	bool				IsWhitespace(char cChar);
	char*				StripWhitespace(char* pszLine);
	std::string			StripWhitespace(std::string sLine);

	CConversionScene*	GetScene() { return &m_Scene; };

protected:
	CConversionScene	m_Scene;
};

#endif