#ifndef _TINKER_LOBBY_H
#define _TINKER_LOBBY_H

#include <tmap.h>

#include <tstring.h>

class CLobbyPlayer
{
public:
	tstring									GetInfoValue(const tstring& sKey)
	{
		tmap<tstring, tstring>::iterator it = asInfo.find(sKey);

		if (it == asInfo.end())
			return "";

		return it->second;
	}

public:
	size_t					iID;
	size_t					iClient;
	tmap<tstring, tstring>	asInfo;
};

#endif
