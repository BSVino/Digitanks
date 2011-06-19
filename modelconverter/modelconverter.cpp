#include <stdio.h>
#include <string.h>

#include "modelconverter.h"
#include "strutils.h"

CModelConverter::CModelConverter(CConversionScene* pScene)
{
	m_pScene = pScene;
	m_pWorkListener = NULL;
}

bool CModelConverter::ReadModel(const tstring& sFilename)
{
	tstring sExtension;

	size_t iFileLength = sFilename.length();
	sExtension = sFilename.c_str()+iFileLength-4;
	sExtension.make_lower();

	if (sExtension == _T(".obj"))
		ReadOBJ(sFilename);
	else if (sExtension == _T(".sia"))
		ReadSIA(sFilename);
	else if (sExtension == _T(".dae"))
		ReadDAE(sFilename);
	else
		return false;

	return true;
}

bool CModelConverter::SaveModel(const tstring& sFilename)
{
	tstring sExtension;

	size_t iFileLength = sFilename.length();
	sExtension = sFilename.c_str()+iFileLength-4;
	sExtension.make_lower();

	if (sExtension == _T(".obj"))
		SaveOBJ(sFilename);
	else if (sExtension == _T(".sia"))
		SaveSIA(sFilename);
	else if (sExtension == _T(".dae"))
		SaveDAE(sFilename);
	else
		return false;

	return true;
}

// Takes a path + filename + extension and removes path and extension to return only the filename.
tstring CModelConverter::GetFilename(const tstring& sFilename)
{
	int iLastChar = -1;
	int i = -1;

	while (++i < (int)sFilename.length())
		if (sFilename[i] == L'\\' || sFilename[i] == L'/')
			iLastChar = i;

	tstring sReturn = sFilename.c_str() + iLastChar + 1;

	i = -1;
	while (++i < (int)sReturn.length())
		if (sReturn[i] == L'.')
			iLastChar = i;

	if (iLastChar >= 0)
		return sReturn.substr(0, iLastChar);

	return sReturn;
}

tstring CModelConverter::GetDirectory(const tstring& sFilename)
{
	int iLastSlash = -1;
	int i = -1;
	tstring sResult = sFilename;

	while (++i < (int)sResult.length())
		if (sResult[i] == L'\\' || sResult[i] == L'/')
			iLastSlash = i;

	if (iLastSlash >= 0)
		sResult[iLastSlash] = L'\0';

	return sResult;
}

bool CModelConverter::IsWhitespace(tstring::value_type cChar)
{
	return (cChar == L' ' || cChar == L'\t' || cChar == L'\r' || cChar == L'\n');
}

tstring CModelConverter::StripWhitespace(tstring sLine)
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
