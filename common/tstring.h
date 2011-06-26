#ifndef TINKER_STRING_H
#define TINKER_STRING_H

#include <EASTL/string.h>

typedef eastl::string tstring;
typedef tstring::value_type tchar;

// No support for the c++0x string literals yet
#define _T(x) x

#include "strutils.h"
#include <string>

inline FILE* tfopen(const tstring& sFile, const tstring& sMode)
{
	tstring sBinaryMode = sMode;
	bool bHasB = false;
	for (size_t i = 0; i < sBinaryMode.length(); i++)
	{
		if (sMode[i] == _T('b'))
		{
			bHasB = true;
			break;
		}
	}

	// Open all files in binary mode to preserve unicodeness.
	if (!bHasB)
		sBinaryMode = sMode + _T("b");

#ifdef _MSC_VER
	static FILE* fp;
	_wfopen_s(&fp, sFile.c_str(), sBinaryMode.c_str());
	return fp;
#else
	return fopen(convertstring<tchar, char>(sFile).c_str(), convertstring<tchar, char>(sBinaryMode).c_str());
#endif
}

inline bool fgetts(tstring& str, FILE* fp)
{
#ifdef _MSC_VER
	static wchar_t szLine[1024];
	tchar* r = fgetws(szLine, 1023, fp);

	if (!r)
		return false;

	str = szLine;
	return !!r;
#else
	static char szLine[1024];
	char* r = fgets(szLine, 1023, fp);

	if (!r)
		return false;

	str = convertstring<char, tchar>(szLine);
	return !!r;
#endif
}

inline tchar* tstrncpy(tchar* d, size_t d_size, const tchar* s, size_t n)
{
#ifdef _WIN32
	return std::char_traits<tchar>::_Copy_s(d, d_size, s, n);
#else
	if (d_size < n)
		n = d_size;

	return std::char_traits<tchar>::copy(d, s, n);
#endif
}

inline size_t tstrlen(const tchar* s)
{
	return std::char_traits<tchar>::length(s);
}

inline int tstrncmp(const tchar* s1, const tchar* s2, size_t n)
{
	return std::char_traits<tchar>::compare(s1, s2, n);
}

#endif
