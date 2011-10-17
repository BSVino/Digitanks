#ifndef LW_BASE64_H
#define LW_BASE64_H
#ifdef _WIN32
#pragma once
#endif

#include <string>

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);

#endif