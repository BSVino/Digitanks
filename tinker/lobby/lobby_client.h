#ifndef _TINKER_LOBBY_CLIENT_H
#define _TINKER_LOBBY_CLIENT_H

#include "lobby.h"

#include <EASTL/vector.h>

#include <network/network.h>

class CGameLobbyClient
{
public:
	static void							JoinLobby(size_t iLobby);
	static void							LeaveLobby();
	static bool							IsInLobby() { return s_bInLobby; }

	static size_t						GetNumPlayers();
	static size_t						GetPlayerIndex(size_t iIndex);
	static CLobbyPlayer*				GetPlayer(size_t iIndex);
	static CLobbyPlayer*				GetPlayerByClient(size_t iClient);

	static void							AddPlayer(size_t iClient);
	static void							RemovePlayer(size_t iClient);

	static void							UpdateLobbyInfo(const eastl::string16& sKey, const eastl::string16& sValue);	// Updates server
	static void							UpdateLobby(const eastl::string16& sKey, const eastl::string16& sValue); // Update from server
	static eastl::string16				GetInfoValue(const eastl::string16& sKey);

	static void							UpdatePlayerInfo(const eastl::string16& sKey, const eastl::string16& sValue);	// Updates server
	static void							UpdatePlayer(size_t iClient, const eastl::string16& sKey, const eastl::string16& sValue); // Update from server

	static bool							IsHost();

	static void							SetLobbyUpdateCallback(INetworkListener* pListener, INetworkListener::Callback pfnCallback);
	static void							UpdateListener();

	static void							SetBeginGameCallback(INetworkListener::Callback pfnCallback);
	static void							BeginGame();

protected:
	static bool							s_bInLobby;
	static eastl::vector<CLobbyPlayer>	s_aClients;
	static eastl::map<eastl::string16, eastl::string16> s_asInfo;

	static INetworkListener*			s_pfnLobbyUpdateListener;
	static INetworkListener::Callback	s_pfnLobbyUpdateCallback;

	static INetworkListener::Callback	s_pfnBeginGameCallback;
};

#endif
