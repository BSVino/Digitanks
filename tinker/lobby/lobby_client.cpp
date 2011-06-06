#include "lobby_client.h"

#include <network/network.h>
#include <network/commands.h>
#include <tinker/application.h>

extern CNetworkCommand JoinLobby;
extern CNetworkCommand LeaveLobby;
extern CNetworkCommand UpdateLobbyInfo;
extern CNetworkCommand UpdatePlayerInfo;
extern CNetworkCommand AddLocalPlayer;
extern CNetworkCommand AddBot;
extern CNetworkCommand RemovePlayer;

SERVER_COMMAND(CONNECTION_LOBBY, FullUpdate)
{
	CGameLobbyClient::R_Clear();
}

SERVER_COMMAND(CONNECTION_LOBBY, LobbyInfo)
{
	if (pCmd->GetNumArguments() < 2)
	{
		TMsg("LobbyInfo with not enough arguments\n");
		return;
	}

	eastl::string16 sValue = sParameters.substr(sParameters.find(L' ')+1);
	CGameLobbyClient::R_UpdateLobby(pCmd->Arg(0), sValue);
}

SERVER_COMMAND(CONNECTION_LOBBY, LobbyPlayerInfo)
{
	if (pCmd->GetNumArguments() < 3)
	{
		TMsg("LobbyPlayerInfo with not enough arguments\n");
		return;
	}

	if (pCmd->Arg(1) == L"add")
		CGameLobbyClient::R_AddPlayer(pCmd->ArgAsUInt(0), pCmd->ArgAsUInt(2));
	else if (pCmd->Arg(1) == L"remove")
		CGameLobbyClient::R_RemovePlayer(pCmd->ArgAsUInt(0));
	else
	{
		size_t iLobbyClient = CGameLobbyClient::L_GetPlayerIndexByID(pCmd->ArgAsUInt(0));

		if (iLobbyClient == ~0)
		{
			TMsg(sprintf(L"Can't find lobby player %d\n", iLobbyClient));
			TAssert(!"Can't find lobby player.");
			return;
		}

		eastl::string16 sValue = sParameters.substr(sParameters.find(L' ', sParameters.find(L' ')+1)+1);
		CGameLobbyClient::R_UpdatePlayer(pCmd->ArgAsUInt(0), pCmd->Arg(1), sValue);
	}
}

SERVER_COMMAND(CONNECTION_LOBBY, BeginGame)
{
	CGameLobbyClient::BeginGame();
}

bool CGameLobbyClient::s_bInLobby = false;
eastl::vector<CLobbyPlayer> CGameLobbyClient::s_aClients;
eastl::map<eastl::string16, eastl::string16> CGameLobbyClient::s_asInfo;
INetworkListener::Callback CGameLobbyClient::s_pfnLobbyUpdateCallback = NULL;
INetworkListener::Callback CGameLobbyClient::s_pfnLobbyJoinCallback = NULL;
INetworkListener::Callback CGameLobbyClient::s_pfnLobbyLeaveCallback = NULL;
INetworkListener::Callback CGameLobbyClient::s_pfnBeginGameCallback = NULL;

void CGameLobbyClient::S_JoinLobby(size_t iLobby)
{
	s_aClients.clear();
	::JoinLobby.RunCommand(sprintf(L"%d", (int)iLobby));

	s_bInLobby = true;
}

void CGameLobbyClient::S_LeaveLobby()
{
	::LeaveLobby.RunCommand(L"");
	LobbyNetwork()->Disconnect();
	s_aClients.clear();

	s_bInLobby = false;
}

size_t CGameLobbyClient::L_GetLocalPlayerID()
{
	CLobbyPlayer* pPlayer = L_GetPlayerByClient(LobbyNetwork()->GetClientID());

	TAssert(pPlayer);
	if (!pPlayer)
		return ~0;

	return pPlayer->iID;
}

size_t CGameLobbyClient::L_GetNumPlayers()
{
	return s_aClients.size();
}

size_t CGameLobbyClient::L_GetPlayerIndexByID(size_t iID)
{
	for (size_t i = 0; i < s_aClients.size(); i++)
	{
		if (s_aClients[i].iID == iID)
			return i;
	}

	return ~0;
}

size_t CGameLobbyClient::L_GetPlayerIndexByClient(size_t iClient)
{
	for (size_t i = 0; i < s_aClients.size(); i++)
	{
		if (s_aClients[i].iClient == iClient)
			return i;
	}

	return ~0;
}

CLobbyPlayer* CGameLobbyClient::L_GetPlayer(size_t iIndex)
{
	if (iIndex >= s_aClients.size())
		return NULL;

	return &s_aClients[iIndex];
}

CLobbyPlayer* CGameLobbyClient::L_GetPlayerByID(size_t iID)
{
	return L_GetPlayer(L_GetPlayerIndexByID(iID));
}

CLobbyPlayer* CGameLobbyClient::L_GetPlayerByClient(size_t iClient)
{
	return L_GetPlayer(L_GetPlayerIndexByClient(iClient));
}

void CGameLobbyClient::R_AddPlayer(size_t iID, size_t iClient)
{
	size_t iPlayer = L_GetPlayerIndexByID(iID);

	if (iPlayer != ~0)
		return;

	CLobbyPlayer* pPlayer = &s_aClients.push_back();
	pPlayer->iID = iID;
	pPlayer->iClient = iClient;

	if (iClient == LobbyNetwork()->GetClientID())
	{
		s_bInLobby = true;

		if (s_pfnLobbyJoinCallback)
			s_pfnLobbyJoinCallback(CONNECTION_LOBBY, NULL, NULL);
	}

	UpdateListener();
}

void CGameLobbyClient::R_RemovePlayer(size_t iID)
{
	size_t iPlayer = L_GetPlayerIndexByID(iID);

	if (iPlayer == ~0)
		return;

	if (s_aClients[iPlayer].iClient == LobbyNetwork()->GetClientID())
	{
		s_bInLobby = false;

		if (s_pfnLobbyLeaveCallback)
			s_pfnLobbyLeaveCallback(CONNECTION_LOBBY, NULL, NULL);
	}

	s_aClients.erase(s_aClients.begin() + iPlayer);

	UpdateListener();
}

void CGameLobbyClient::S_AddLocalPlayer()
{
	::AddLocalPlayer.RunCommand(L"");
}

void CGameLobbyClient::S_AddBot()
{
	::AddBot.RunCommand(L"");
}

void CGameLobbyClient::S_RemovePlayer(size_t iID)
{
	::RemovePlayer.RunCommand(sprintf(L"%d", iID));
}

void CGameLobbyClient::S_UpdateLobby(const eastl::string16& sKey, const eastl::string16& sValue)
{
	::UpdateLobbyInfo.RunCommand(sKey + L" " + sValue);
}

void CGameLobbyClient::R_UpdateLobby(const eastl::string16& sKey, const eastl::string16& sValue)
{
	s_asInfo[sKey] = sValue;

	UpdateListener();
}

eastl::string16 CGameLobbyClient::L_GetInfoValue(const eastl::string16& sKey)
{
	eastl::map<eastl::string16, eastl::string16>::iterator it = s_asInfo.find(sKey);

	if (it == s_asInfo.end())
		return L"";

	return it->second;
}

void CGameLobbyClient::S_UpdatePlayer(const eastl::string16& sKey, const eastl::string16& sValue)
{
	S_UpdatePlayer(CGameLobbyClient::L_GetLocalPlayerID(), sKey, sValue);
}

void CGameLobbyClient::S_UpdatePlayer(size_t iID, const eastl::string16& sKey, const eastl::string16& sValue)
{
	::UpdatePlayerInfo.RunCommand(sprintf(eastl::string16(L"%d ") + sKey + L" " + sValue, iID));
}

void CGameLobbyClient::R_UpdatePlayer(size_t iID, const eastl::string16& sKey, const eastl::string16& sValue)
{
	CLobbyPlayer* pPlayer = L_GetPlayerByID(iID);
	if (!pPlayer)
		return;

	pPlayer->asInfo[sKey] = sValue;

	UpdateListener();
}

void CGameLobbyClient::R_Clear()
{
	s_aClients.clear();
	s_asInfo.clear();
}

bool CGameLobbyClient::L_IsHost()
{
	CLobbyPlayer* pPlayer = L_GetPlayerByID(L_GetLocalPlayerID());
	TAssert(pPlayer);
	if (!pPlayer)
		return false;

	return pPlayer->GetInfoValue(L"host") == L"1";
}

void CGameLobbyClient::SetLobbyUpdateCallback(INetworkListener::Callback pfnCallback)
{
	s_pfnLobbyUpdateCallback = pfnCallback;
}

void CGameLobbyClient::UpdateListener()
{
	if (s_pfnLobbyUpdateCallback)
		s_pfnLobbyUpdateCallback(CONNECTION_LOBBY, NULL, NULL);
}

void CGameLobbyClient::SetLobbyJoinCallback(INetworkListener::Callback pfnCallback)
{
	s_pfnLobbyJoinCallback = pfnCallback;
}

void CGameLobbyClient::SetLobbyLeaveCallback(INetworkListener::Callback pfnCallback)
{
	s_pfnLobbyLeaveCallback = pfnCallback;
}

void CGameLobbyClient::SetBeginGameCallback(INetworkListener::Callback pfnCallback)
{
	s_pfnBeginGameCallback = pfnCallback;
}

void CGameLobbyClient::BeginGame()
{
	if (s_pfnBeginGameCallback)
		s_pfnBeginGameCallback(CONNECTION_LOBBY, NULL, NULL);

	s_bInLobby = false;
}
