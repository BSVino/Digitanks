#ifndef _TINKER_LOBBY_CLIENT_H
#define _TINKER_LOBBY_CLIENT_H

#include "lobby.h"

#include <tvector.h>

#include <network/network.h>

class CGameLobbyClient
{
public:
	// NOMENCLATURE:
	// S_Function - Send a command to the server.
	// R_Function - Receives a command from the server. (Don't call this from outside lobby_client.cpp)
	// L_Function - Local data query. No network calls.

	static void							S_JoinLobby(size_t iLobby);
	static void							S_LeaveLobby();
	static bool							L_IsInLobby() { return s_bInLobby; }

	static size_t						L_GetLocalPlayerID();

	static size_t						L_GetNumPlayers();
	static size_t						L_GetPlayerIndexByID(size_t iID);
	static size_t						L_GetPlayerIndexByClient(size_t iClient);
	static CLobbyPlayer*				L_GetPlayer(size_t iIndex);
	static CLobbyPlayer*				L_GetPlayerByID(size_t iID);
	static CLobbyPlayer*				L_GetPlayerByClient(size_t iClient);

	static void							R_AddPlayer(size_t iID, size_t iClient);
	static void							R_RemovePlayer(size_t iID);

	static void							S_AddLocalPlayer();
	static void							S_AddBot();
	static void							S_RemovePlayer(size_t iID);

	static void							S_UpdateLobby(const tstring& sKey, const tstring& sValue);
	static void							R_UpdateLobby(const tstring& sKey, const tstring& sValue);
	static tstring				L_GetInfoValue(const tstring& sKey);

	static void							S_UpdatePlayer(const tstring& sKey, const tstring& sValue);
	static void							S_UpdatePlayer(size_t iID, const tstring& sKey, const tstring& sValue);
	static void							R_UpdatePlayer(size_t iID, const tstring& sKey, const tstring& sValue);

	static void							R_Clear();

	static bool							L_IsHost();

	static void							SetLobbyUpdateCallback(INetworkListener::Callback pfnCallback);
	static void							UpdateListener();

	static void							SetLobbyJoinCallback(INetworkListener::Callback pfnCallback);
	static void							SetLobbyLeaveCallback(INetworkListener::Callback pfnCallback);

	static void							SetBeginGameCallback(INetworkListener::Callback pfnCallback);
	static void							BeginGame();

	static void							ClientEnterGame(int iConnection, class INetworkListener*, class CNetworkParameters*);
	static void							ClientDisconnect(int iConnection, class INetworkListener*, class CNetworkParameters*);

protected:
	static bool							s_bInLobby;
	static tvector<CLobbyPlayer>		s_aClients;
	static tmap<tstring, tstring>		s_asInfo;

	static INetworkListener::Callback	s_pfnLobbyUpdateCallback;
	static INetworkListener::Callback	s_pfnLobbyJoinCallback;
	static INetworkListener::Callback	s_pfnLobbyLeaveCallback;

	static INetworkListener::Callback	s_pfnBeginGameCallback;
};

#endif
