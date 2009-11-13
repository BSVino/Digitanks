#ifndef CF_STRUTILS_H
#define CF_STRUTILS_H
#ifdef _WIN32
#pragma once
#endif

#include <string>
#include <vector>

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

#endif
