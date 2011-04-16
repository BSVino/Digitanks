#include "lobby.h"

eastl::vector<CGameLobby> CGameLobby::s_aLobbies;

CGameLobby::CGameLobby()
{
	m_bActive = false;
}
