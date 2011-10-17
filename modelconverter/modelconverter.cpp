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
		if (sFilename[i] == _T('\\') || sFilename[i] == _T('/'))
			iLastChar = i;

	tstring sReturn = sFilename.c_str() + iLastChar + 1;

	i = -1;
	while (++i < (int)sReturn.length())
		if (sReturn[i] == _T('.'))
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
		if (sResult[i] == _T('\\') || sResult[i] == _T('/'))
			iLastSlash = i;

	if (iLastSlash >= 0)
		sResult[iLastSlash] = _T('\0');

	return sResult;
}

bool CModelConverter::IsWhitespace(tstring::value_type cChar)
{
	return (cChar == _T(' ') || cChar == _T('\t') || cChar == _T('\r') || cChar == _T('\n'));
}

tstring CModelConverter::StripWhitespace(tstring sLine)
{
	int i = 0;
	while (IsWhitespace(sLine[i]) && sLine[i] != _T('\0'))
		i++;

	int iEnd = ((int)sLine.length())-1;
	while (iEnd >= 0 && IsWhitespace(sLine[iEnd]))
		iEnd--;

	if (iEnd >= -1)
		sLine[iEnd+1] = _T('\0');

	return sLine.substr(i);
}
