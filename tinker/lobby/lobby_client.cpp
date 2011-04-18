#include "lobby_client.h"

#include <network/network.h>
#include <network/commands.h>
#include <tinker/application.h>

extern CNetworkCommand JoinLobby;
extern CNetworkCommand LeaveLobby;
extern CNetworkCommand UpdateLobbyInfo;
extern CNetworkCommand UpdatePlayerInfo;

SERVER_COMMAND(LobbyInfo)
{
	if (pCmd->GetNumArguments() < 2)
	{
		TMsg("LobbyInfo with not enough arguments\n");
		return;
	}

	eastl::string16 sValue = sParameters.substr(sParameters.find(L' ')+1);
	CGameLobbyClient::UpdateLobby(pCmd->Arg(0), sValue);
}

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
			CGameLobbyClient::AddPlayer(pCmd->ArgAsUInt(0));
		else if (!pCmd->ArgAsBool(2) && iLobbyClient != ~0)
			CGameLobbyClient::RemovePlayer(pCmd->ArgAsUInt(0));
	}
	else
	{
		if (iLobbyClient == ~0)
		{
			TMsg(sprintf(L"Can't find lobby player %d\n", iLobbyClient));
			assert(!"Can't find lobby player.");
			return;
		}

		eastl::string16 sValue = sParameters.substr(sParameters.find(L' ', sParameters.find(L' ')+1)+1);
		CGameLobbyClient::UpdatePlayer(pCmd->ArgAsUInt(0), pCmd->Arg(1), sValue);
	}
}

bool CGameLobbyClient::s_bInLobby = false;
eastl::vector<CLobbyPlayer> CGameLobbyClient::s_aClients;
eastl::map<eastl::string16, eastl::string16> CGameLobbyClient::s_asInfo;
INetworkListener* CGameLobbyClient::s_pfnLobbyUpdateListener = NULL;
INetworkListener::Callback CGameLobbyClient::s_pfnLobbyUpdateCallback = NULL;

void CGameLobbyClient::JoinLobby(size_t iLobby)
{
	s_aClients.clear();
	::JoinLobby.RunCommand(sprintf(L"%d", (int)iLobby));

	s_bInLobby = true;
}

void CGameLobbyClient::LeaveLobby()
{
	::LeaveLobby.RunCommand(L"");
	CNetwork::Disconnect();
	s_aClients.clear();

	s_bInLobby = false;
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

void CGameLobbyClient::UpdateLobbyInfo(const eastl::string16& sKey, const eastl::string16& sValue)
{
	::UpdateLobbyInfo.RunCommand(sKey + L" " + sValue);
}

void CGameLobbyClient::UpdateLobby(const eastl::string16& sKey, const eastl::string16& sValue)
{
	s_asInfo[sKey] = sValue;

	UpdateListener();
}

eastl::string16 CGameLobbyClient::GetInfoValue(const eastl::string16& sKey)
{
	eastl::map<eastl::string16, eastl::string16>::iterator it = s_asInfo.find(sKey);

	if (it == s_asInfo.end())
		return L"";

	return it->second;
}

void CGameLobbyClient::UpdatePlayerInfo(const eastl::string16& sKey, const eastl::string16& sValue)
{
	::UpdatePlayerInfo.RunCommand(sKey + L" " + sValue);
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
