#ifndef _TINKER_LOBBY_H
#define _TINKER_LOBBY_H

#include <EASTL/map.h>
#include <EASTL/string.h>

class CLobbyPlayer
{
public:
	eastl::string16									GetInfoValue(const eastl::string16& sKey)
	{
		eastl::map<eastl::string16, eastl::string16>::iterator it = asInfo.find(sKey);

		if (it == asInfo.end())
			return L"";

		return it->second;
	}

public:
	size_t											iID;
	size_t											iClient;
	eastl::map<eastl::string16, eastl::string16>	asInfo;
};

#endif
