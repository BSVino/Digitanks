#include "lobby_client.h"

#include <network/network.h>
#include <network/commands.h>
#include <tinker/application.h>

extern CNetworkCommand JoinLobby;
extern CNetworkCommand UpdateInfo;

SERVER_COMMAND(LobbyPlayerInfo)
{
	if (pCmd->GetNumArguments() < 3)
	{
		TMsg("LobbyPlayerInfo with not enough arguments\n");
		return;
	}

	size_t iLobbyClient = CGameLobbyClient::GetPlayerIndex(pCmd->ArgAsUInt(0));

	if (pCmd->Arg(1) == L"active")
	{
		if (pCmd->ArgAsBool(2) && iLobbyClient == ~0)
			CGameLobbyClient::AddPlayer(iLobbyClient);
		else
			CGameLobbyClient::RemovePlayer(iLobbyClient);
	}
	else
	{
		if (iLobbyClient == ~0)
		{
			TMsg(sprintf(L"Can't find lobby player %d\n", iLobbyClient));
			assert(!"Can't find lobby player.");
			return;
		}

		CGameLobbyClient::UpdatePlayer(iClient, pCmd->Arg(1), pCmd->Arg(2));
	}
}

eastl::vector<CLobbyPlayer> CGameLobbyClient::s_aClients;
INetworkListener* CGameLobbyClient::s_pfnLobbyUpdateListener = NULL;
INetworkListener::Callback CGameLobbyClient::s_pfnLobbyUpdateCallback = NULL;

void CGameLobbyClient::JoinLobby(size_t iLobby)
{
	::JoinLobby.RunCommand(sprintf(L"%d %d", (int)iLobby, (int)CNetwork::GetClientID()));
}

size_t CGameLobbyClient::GetNumPlayers()
{
	return s_aClients.size();
}

size_t CGameLobbyClient::GetPlayerIndex(size_t iClient)
{
	for (size_t i = 0; i < s_aClients.size(); i++)
	{
		if (s_aClients[i].iClient == iClient)
			return i;
	}

	return ~0;
}

CLobbyPlayer* CGameLobbyClient::GetPlayer(size_t iIndex)
{
	if (iIndex >= s_aClients.size())
		return NULL;

	return &s_aClients[iIndex];
}

CLobbyPlayer* CGameLobbyClient::GetPlayerByClient(size_t iClient)
{
	return GetPlayer(GetPlayerIndex(iClient));
}

void CGameLobbyClient::AddPlayer(size_t iClient)
{
	size_t iPlayer = GetPlayerIndex(iClient);

	if (iPlayer != ~0)
		return;

	CLobbyPlayer* pPlayer = &s_aClients.push_back();
	pPlayer->iClient = iClient;

	UpdateListener();
}

void CGameLobbyClient::RemovePlayer(size_t iClient)
{
	size_t iPlayer = GetPlayerIndex(iClient);

	if (iPlayer == ~0)
		return;

	s_aClients.erase(s_aClients.begin() + iPlayer);

	UpdateListener();
}

void CGameLobbyClient::UpdateInfo(const eastl::string16& sKey, const eastl::string16& sValue)
{
	::UpdateInfo.RunCommand(sKey + L" " + sValue);
}

void CGameLobbyClient::UpdatePlayer(size_t iClient, const eastl::string16& sKey, const eastl::string16& sValue)
{
	CLobbyPlayer* pPlayer = GetPlayerByClient(iClient);
	if (!pPlayer)
		return;

	pPlayer->asInfo[sKey] = sValue;

	UpdateListener();
}

void CGameLobbyClient::SetLobbyUpdateCallback(INetworkListener* pListener, INetworkListener::Callback pfnCallback)
{
	s_pfnLobbyUpdateListener = pListener;
	s_pfnLobbyUpdateCallback = pfnCallback;
}

void CGameLobbyClient::UpdateListener()
{
	if (s_pfnLobbyUpdateListener)
		s_pfnLobbyUpdateCallback(s_pfnLobbyUpdateListener, NULL);
}
