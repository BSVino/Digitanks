#ifndef CF_STRUTILS_H
#define CF_STRUTILS_H
#ifdef _WIN32
#pragma once
#endif

#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <cctype>

// It's inline so I don't have to make a strutils.cpp :P
inline void wcstok(const std::wstring& str, std::vector<std::wstring>& tokens, const std::wstring& delimiters = L" \r\n\t")
{
    // Skip delimiters at beginning.
    std::wstring::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::wstring::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::wstring::npos != pos || std::wstring::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

inline std::string& ltrim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

inline std::string& rtrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

inline std::string& trim(std::string &s)
{
	return ltrim(rtrim(s));
}

#endif
