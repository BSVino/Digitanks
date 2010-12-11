#ifndef LW_DATASERIALIZER_H
#define LW_DATASERIALIZER_H

#include "data.h"

#include <iostream>

class CDataSerializer
{
public:
	static void			Read(std::istream& sStream, class CData* pData);
};

#endif
