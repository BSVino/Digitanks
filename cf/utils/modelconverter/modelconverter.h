#ifndef CF_MODELCONVERTER_H
#define CF_MODELCONVERTER_H

#include <iostream>
#include <fstream>

#include "convmesh.h"

class CModelConverter
{
public:
	void				ReadOBJ(const char* pszFilename);

	// SIA and its utility functions.
	void				ReadSIA(const char* pszFilename);
	void				ReadSIAMat(std::ifstream& infile);
	void				ReadSIAShape(std::ifstream& infile, bool bCare = true);

	void				WriteSMD(const char* pszFilename);

	char*				MakeFilename(char* pszPathFilename);
	bool				IsWhitespace(char cChar);
	char*				StripWhitespace(char* pszLine);
	std::string			StripWhitespace(std::string sLine);

	CConversionMesh		m_Mesh;
};

#endif