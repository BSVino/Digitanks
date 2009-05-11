#ifndef CF_MODELCONVERTER_H
#define CF_MODELCONVERTER_H

#include "convmesh.h"

class CModelConverter
{
public:
	void				ReadOBJ(const char* pszFilename);
	void				WriteSMD(const char* pszFilename);

	char*				MakeFilename(char* pszPathFilename);
	bool				IsWhitespace(char cChar);
	char*				StripWhitespace(char* pszLine);

	CConversionMesh		m_Mesh;
};

#endif