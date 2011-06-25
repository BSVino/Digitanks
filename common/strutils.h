#ifndef LW_STRUTILS_H
#define LW_STRUTILS_H
#ifdef _WIN32
#pragma once
#endif

#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <functional>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

template <class F, class T>
inline eastl::basic_string<T> convertstring(eastl::basic_string<F> s);

#include "tstring.h"

// It's inline so I don't have to make a strutils.cpp :P
inline void strtok(const eastl::string& str, eastl::vector<eastl::string>& tokens, const eastl::string& delimiters = " \r\n\t")
{
    // Skip delimiters at beginning.
    eastl::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    eastl::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (eastl::string::npos != pos || eastl::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

// It's inline so I don't have to make a strutils.cpp :P
inline void tstrtok(const tstring& str, eastl::vector<tstring>& tokens, const tstring& delimiters = _T(" \r\n\t"))
{
	tokens.clear();

	// Skip delimiters at beginning.
    tstring::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    tstring::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (tstring::npos != pos || tstring::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

// explode is slightly different in that repeated delineators return multiple tokens.
// ie "a|b||c" returns { "a", "b", "", "c" } whereas strtok will cut out the blank result.
// Basically it works like PHP's explode.
inline void explode(const tstring& str, eastl::vector<tstring>& tokens, const tstring& delimiter = _T(" "))
{
    tstring::size_type lastPos = str.find_first_of(delimiter, 0);
    tstring::size_type pos = 0;

    while (true)
    {
        tokens.push_back(str.substr(pos, lastPos - pos));

		if (lastPos == tstring::npos)
			break;

		pos = lastPos+1;
        lastPos = str.find_first_of(delimiter, pos);
    }
}

inline int isspace(int i)
{
	if (i == ' ')
		return true;

	if (i == '\t')
		return true;

	if (i == '\r')
		return true;

	if (i == '\n')
		return true;

	return false;
}

inline tstring ltrim(tstring s)
{
	s.erase(s.begin(), eastl::find_if(s.begin(), s.end(), eastl::not1(eastl::ptr_fun<int, int>(isspace))));
	return s;
}

inline tstring rtrim(tstring s)
{
	s.erase(eastl::find_if(s.rbegin(), s.rend(), eastl::not1(eastl::ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

inline tstring trim(tstring s)
{
	return ltrim(rtrim(s));
}

inline void writestring(std::ostream& o, const eastl::string& s)
{
	size_t iStringSize = s.length();
	o.write((char*)&iStringSize, sizeof(iStringSize));
	o.write((char*)s.c_str(), s.size()*sizeof(eastl::string::value_type));
}

inline eastl::string readstring(std::istream& i)
{
	size_t iStringSize;
	i.read((char*)&iStringSize, sizeof(iStringSize));

	eastl::string s;
	eastl::string::value_type c[2];
	c[1] = _T('\0');
	for (size_t j = 0; j < iStringSize; j++)
	{
		i.read((char*)&c[0], sizeof(eastl::string::value_type));
		s.append(c);
	}

	return s;
}

inline void writetstring(std::ostream& o, const tstring& s)
{
	size_t iStringSize = s.length();
	o.write((char*)&iStringSize, sizeof(iStringSize));
	o.write((char*)s.c_str(), s.size()*sizeof(tstring::value_type));
}

inline tstring readtstring(std::istream& i)
{
	size_t iStringSize;
	i.read((char*)&iStringSize, sizeof(iStringSize));

	tstring s;
	tstring::value_type c[2];
	c[1] = _T('\0');
	for (size_t j = 0; j < iStringSize; j++)
	{
		i.read((char*)&c[0], sizeof(tstring::value_type));
		s.append(c);
	}

	return s;
}

template <class F, class T>
inline eastl::basic_string<T> convertstring(eastl::basic_string<F> s)
{
	eastl::basic_string<T> t;
	size_t iSize = s.size();
	t.resize(iSize);

	for (size_t i = 0; i < iSize; i++)
		t[i] = (T)s[i];

	return t;
}

inline tstring sprintf(tstring s, ...)
{
	va_list arguments;
	va_start(arguments, s);

	tstring p;
	p.sprintf_va_list(s.c_str(), arguments);

	va_end(arguments);

	return p;
}

inline tstring str_replace(const tstring& s, const tstring& f, const tstring& r)
{
	tstring sResult = s;

	size_t iPosition;
	while ((iPosition = sResult.find(f)) != ~0)
		sResult = sResult.substr(0, iPosition) + r + (sResult.c_str()+iPosition+f.length());

	return sResult;
}

inline int stoi(const tstring& s)
{
	std::istringstream i(convertstring<tchar, char>(s).c_str());
	int x;
	if (!(i >> x))
		return 0;

	return x;
}

inline float stof(const tstring& s)
{
	std::istringstream i(convertstring<tchar, char>(s).c_str());
	float x;
	if (!(i >> x))
		return 0;

	return x;
}

template <class C>
inline C* strtok(C* s, const C* delim, C** last)
{
	C* spanp;
	int c, sc;
	C* tok;


	if (s == NULL && (s = *last) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = (C*)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		*last = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (C *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*last = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

template <class C>
inline C* strdup(const C* s)
{
	size_t len = (1+std::char_traits<C>::length(s)) * sizeof(C);
	C* p = (C*)malloc(len);

  	return p ? (C*)memcpy(p, s, len) : NULL;
}

#endif
