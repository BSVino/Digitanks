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
inline void strtok(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " \r\n\t")
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
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

// explode is slightly different in that repeated delineators return multiple tokens.
// ie "a|b||c" returns { "a", "b", "", "c" } whereas strtok will cut out the blank result.
// Basically it works like PHP's explode.
inline void explode(const std::wstring& str, std::vector<std::wstring>& tokens, const std::wstring& delimiter = L" ")
{
    std::wstring::size_type lastPos = str.find_first_of(delimiter, 0);
    std::wstring::size_type pos = 0;

    while (true)
    {
        tokens.push_back(str.substr(pos, lastPos - pos));

		if (lastPos == std::wstring::npos)
			break;

		pos = lastPos+1;
        lastPos = str.find_first_of(delimiter, pos);
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
