#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "modelconverter.h"
#include "strutils.h"

CModelConverter::CModelConverter(CConversionScene* pScene)
{
	m_pScene = pScene;
	m_pWorkListener = NULL;
}

bool CModelConverter::ReadModel(const eastl::string16& sFilename)
{
	eastl::string16 sExtension;

	size_t iFileLength = wcslen(sFilename.c_str());
	sExtension = sFilename.c_str()+iFileLength-4;
	sExtension.make_lower();

	if (sExtension == L".obj")
		ReadOBJ(sFilename);
	else if (sExtension == L".sia")
		ReadSIA(sFilename);
	else if (sExtension == L".dae")
		ReadDAE(sFilename);
	else
		return false;

	return true;
}

bool CModelConverter::SaveModel(const eastl::string16& sFilename)
{
	eastl::string16 sExtension;

	size_t iFileLength = wcslen(sFilename.c_str());
	sExtension = sFilename.c_str()+iFileLength-4;
	sExtension.make_lower();

	if (sExtension == L".obj")
		SaveOBJ(sFilename);
	else if (sExtension == L".sia")
		SaveSIA(sFilename);
	else if (sExtension == L".dae")
		SaveDAE(sFilename);
	else
		return false;

	return true;
}

// Takes a path + filename + extension and removes path and extension to return only the filename.
eastl::string16 CModelConverter::GetFilename(const eastl::string16& sFilename)
{
	int iLastChar = -1;
	int i = -1;

	while (++i < (int)sFilename.length())
		if (sFilename[i] == L'\\' || sFilename[i] == L'/')
			iLastChar = i;

	eastl::string16 sReturn = sFilename.c_str() + iLastChar + 1;

	i = -1;
	while (++i < (int)sReturn.length())
		if (sReturn[i] == L'.')
			iLastChar = i;

	if (iLastChar >= 0)
		return sReturn.substr(0, iLastChar);

	return sReturn;
}

eastl::string16 CModelConverter::GetDirectory(const eastl::string16& sFilename)
{
	int iLastSlash = -1;
	int i = -1;
	eastl::string16 sResult = sFilename;

	while (++i < (int)sResult.length())
		if (sResult[i] == L'\\' || sResult[i] == L'/')
			iLastSlash = i;

	if (iLastSlash >= 0)
		sResult[iLastSlash] = L'\0';

	return sResult;
}

bool CModelConverter::IsWhitespace(eastl::string16::value_type cChar)
{
	return (cChar == L' ' || cChar == L'\t' || cChar == L'\r' || cChar == L'\n');
}

eastl::string16 CModelConverter::StripWhitespace(eastl::string16 sLine)
{
	int i = 0;
	while (IsWhitespace(sLine[i]) && sLine[i] != L'\0')
		i++;

	int iEnd = ((int)sLine.length())-1;
	while (iEnd >= 0 && IsWhitespace(sLine[iEnd]))
		iEnd--;

	if (iEnd >= -1)
		sLine[iEnd+1] = L'\0';

	return sLine.substr(i);
}
