#ifndef _TINKER_LOBBY_H
#define _TINKER_LOBBY_H

#include <EASTL/vector.h>

class CGameLobby
{
public:
										CGameLobby();

public:

protected:
	bool								m_bActive;

	static eastl::vector<CGameLobby>	s_aLobbies;
};

#endif
