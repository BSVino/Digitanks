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
inline void wcstok(const eastl::string16& str, eastl::vector<eastl::string16>& tokens, const eastl::string16& delimiters = L" \r\n\t")
{
    // Skip delimiters at beginning.
    eastl::string16::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    eastl::string16::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (eastl::string16::npos != pos || eastl::string16::npos != lastPos)
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
inline void explode(const eastl::string16& str, eastl::vector<eastl::string16>& tokens, const eastl::string16& delimiter = L" ")
{
    eastl::string16::size_type lastPos = str.find_first_of(delimiter, 0);
    eastl::string16::size_type pos = 0;

    while (true)
    {
        tokens.push_back(str.substr(pos, lastPos - pos));

		if (lastPos == eastl::string16::npos)
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

inline eastl::string& ltrim(eastl::string &s)
{
	s.erase(s.begin(), eastl::find_if(s.begin(), s.end(), eastl::not1(eastl::ptr_fun<int, int>(isspace))));
	return s;
}

inline eastl::string& rtrim(eastl::string &s)
{
	s.erase(eastl::find_if(s.rbegin(), s.rend(), eastl::not1(eastl::ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

inline eastl::string& trim(eastl::string &s)
{
	return ltrim(rtrim(s));
}

inline void writestring16(std::ostream& o, const eastl::string16& s)
{
	size_t iStringSize = s.length();
	o.write((char*)&iStringSize, sizeof(iStringSize));
	o.write((char*)s.c_str(), s.size()*sizeof(eastl::string16::value_type));
}

inline eastl::string16 readstring16(std::istream& i)
{
	size_t iStringSize;
	i.read((char*)&iStringSize, sizeof(iStringSize));

	eastl::string16 s;
	eastl::string16::value_type c[2];
	c[1] = L'\0';
	for (size_t j = 0; j < iStringSize; j++)
	{
		i.read((char*)&c[0], sizeof(eastl::string16::value_type));
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

#endif
