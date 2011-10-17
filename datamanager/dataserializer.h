#ifndef LW_DATASERIALIZER_H
#define LW_DATASERIALIZER_H

#include "data.h"

#include <iostream>

#include <tstring.h>

class CDataSerializer
{
public:
	static void			Read(std::basic_istream<tchar>& sStream, class CData* pData);
	static void			Save(std::basic_ostream<tchar>& sStream, class CData* pData);
};

#endif
