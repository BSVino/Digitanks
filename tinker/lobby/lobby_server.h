#ifndef _TINKER_LOBBY_SERVER_H
#define _TINKER_LOBBY_SERVER_H

#include <EASTL/vector.h>

#include "lobby.h"

class CGameLobby
{
	friend class CGameLobbyServer;

public:
										CGameLobby();

public:
	void								Initialize(size_t iPort);
	void								Shutdown();

	size_t								GetNumPlayers();
	size_t								GetPlayerIndex(size_t iIndex);
	CLobbyPlayer*						GetPlayer(size_t iIndex);
	CLobbyPlayer*						GetPlayerByClient(size_t iClient);

	void								AddPlayer(size_t iClient);
	void								UpdatePlayer(size_t iClient, const eastl::string16& sKey, const eastl::string16& sValue);

protected:
	bool								m_bActive;
	eastl::vector<CLobbyPlayer>			m_aClients;
};

class CGameLobbyServer
{
public:
	static size_t						CreateLobby(size_t iPort);
	static void							DestroyLobby(size_t iLobby);
	static void							JoinLobby(size_t iLobby, size_t iClient);

	static void							UpdatePlayer(size_t iClient, const eastl::string16& sKey, const eastl::string16& sValue);

protected:
	static eastl::vector<CGameLobby>	s_aLobbies;
	static eastl::vector<size_t>		s_iClientLobbies;
};

#endif
