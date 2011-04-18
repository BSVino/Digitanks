#include "lobby_server.h"

#include <network/network.h>
#include <network/commands.h>
#include <tinker/application.h>

extern CNetworkCommand LobbyInfo;
extern CNetworkCommand LobbyPlayerInfo;
extern CNetworkCommand ServerChatSay;

CLIENT_COMMAND(JoinLobby)
{
	if (pCmd->GetNumArguments() < 1)
	{
		TMsg("JoinLobby not enough arguments\n");
		return;
	}

	CGameLobbyServer::JoinLobby(pCmd->ArgAsUInt(0), iClient);
}

CLIENT_COMMAND(LeaveLobby)
{
	CGameLobbyServer::LeaveLobby(iClient);
}

CLIENT_COMMAND(UpdateLobbyInfo)
{
	if (pCmd->GetNumArguments() < 2)
	{
		TMsg("UpdateInfo not enough arguments\n");
		return;
	}

	CGameLobby* pLobby = CGameLobbyServer::GetLobby(CGameLobbyServer::GetPlayerLobby(iClient));
	if (!pLobby)
		return;

	CLobbyPlayer* pPlayer = pLobby->GetPlayerByClient(iClient);
	if (!pPlayer)
		return;

	if (pPlayer->GetInfoValue(L"host") != L"1")
		return;

	eastl::string16 sValue = sParameters.substr(sParameters.find(L' ')+1);
	CGameLobbyServer::UpdateLobby(CGameLobbyServer::GetPlayerLobby(iClient), pCmd->Arg(0), sValue);
}

CLIENT_COMMAND(UpdatePlayerInfo)
{
	if (pCmd->GetNumArguments() < 2)
	{
		TMsg("UpdateInfo not enough arguments\n");
		return;
	}

	eastl::string16 sValue = sParameters.substr(sParameters.find(L' ')+1);
	CGameLobbyServer::UpdatePlayer(iClient, pCmd->Arg(0), sValue);
}

eastl::vector<CGameLobby> CGameLobbyServer::s_aLobbies;
eastl::vector<size_t> CGameLobbyServer::s_iClientLobbies;

size_t CGameLobbyServer::CreateLobby(size_t iPort)
{
	if (s_iClientLobbies.size() == 0)
	{
		s_iClientLobbies.resize(NETWORK_MAX_CLIENTS);

		for (size_t i = 0; i < NETWORK_MAX_CLIENTS; i++)
			s_iClientLobbies[i] = ~0;
	}

	CGameLobby* pLobby = NULL;
	size_t iLobby;
	
	for (size_t i = 0; i < s_aLobbies.size(); i++)
	{
		if (!s_aLobbies[i].m_bActive)
		{
			pLobby = &s_aLobbies[i];
			iLobby = i;
			break;
		}
	}

	if (!pLobby)
	{
		pLobby = &s_aLobbies.push_back();
		iLobby = s_aLobbies.size()-1;
	}

	pLobby->Initialize(iPort);

	return iLobby;
}

void CGameLobbyServer::DestroyLobby(size_t iLobby)
{
	if (iLobby >= s_aLobbies.size())
	{
		assert(!"What lobby is this?");
		return;
	}

	s_aLobbies[iLobby].Shutdown();
}

void CGameLobbyServer::JoinLobby(size_t iLobby, size_t iClient)
{
	if (iLobby >= s_aLobbies.size())
	{
		assert(!"What lobby is this?");
		return;
	}

	s_aLobbies[iLobby].AddPlayer(iClient);

	if (iClient != ~0)
		s_iClientLobbies[iClient] = iLobby;
}

void CGameLobbyServer::LeaveLobby(size_t iClient)
{
	size_t iLobby = 0;
	if (iClient != ~0)
		s_iClientLobbies[iClient] = iLobby;

	if (iLobby >= s_aLobbies.size())
	{
		assert(!"What lobby is this?");
		return;
	}

	s_aLobbies[iLobby].RemovePlayer(iClient);

	if (iClient != ~0)
		s_iClientLobbies[iClient] = ~0;
}

CGameLobby* CGameLobbyServer::GetLobby(size_t iLobby)
{
	if (iLobby >= s_aLobbies.size())
	{
		assert(!"What lobby is this?");
		return NULL;
	}

	return &s_aLobbies[iLobby];
}

size_t CGameLobbyServer::GetActiveLobbies()
{
	size_t iLobbies = 0;
	for (size_t i = 0; i < s_aLobbies.size(); i++)
	{
		if (s_aLobbies[i].m_bActive)
			iLobbies++;
	}

	return iLobbies;
}

size_t CGameLobbyServer::GetPlayerLobby(size_t iClient)
{
	if (iClient == ~0)
		return 0;
	return s_iClientLobbies[iClient];
}

void CGameLobbyServer::UpdateLobby(size_t iLobby, const eastl::string16& sKey, const eastl::string16& sValue)
{
	if (iLobby == ~0)
	{
		assert(!"What lobby is this?");
		return;
	}

	s_aLobbies[iLobby].UpdateInfo(sKey, sValue);
}

void CGameLobbyServer::UpdatePlayer(size_t iClient, const eastl::string16& sKey, const eastl::string16& sValue)
{
	if (iClient != ~0 && s_iClientLobbies[iClient] == ~0)
	{
		TMsg(sprintf(L"Can't find lobby for client %d\n", iClient));
		assert(!"Can't find lobby for client");
		return;
	}

	size_t iLobby;
	if (iClient == ~0)
		iLobby = 0;
	else
		iLobby = s_iClientLobbies[iClient];

	s_aLobbies[iLobby].UpdatePlayer(iClient, sKey, sValue);
}

void CGameLobbyServer::ClientConnect(class INetworkListener*, class CNetworkParameters* pParameters)
{
	int iClient = pParameters->i1;

	JoinLobby(0, iClient);

	s_aLobbies[0].SendFullUpdate(iClient);
}

void CGameLobbyServer::ClientDisconnect(class INetworkListener*, class CNetworkParameters* pParameters)
{
	int iClient = pParameters->i1;

	LeaveLobby(iClient);
}

CGameLobby::CGameLobby()
{
	m_bActive = false;
}

void CGameLobby::Initialize(size_t iPort)
{
	CNetwork::Disconnect();
	CNetwork::SetCallbacks(NULL, CGameLobbyServer::ClientConnect, CGameLobbyServer::ClientDisconnect);
	CNetwork::CreateHost(iPort);
	m_bActive = true;
	m_aClients.clear();
}

void CGameLobby::Shutdown()
{
	CNetwork::Disconnect();
	m_bActive = false;
}

size_t CGameLobby::GetNumPlayers()
{
	return m_aClients.size();
}

size_t CGameLobby::GetPlayerIndex(size_t iClient)
{
	for (size_t i = 0; i < m_aClients.size(); i++)
	{
		if (m_aClients[i].iClient == iClient)
			return i;
	}

	return ~0;
}

CLobbyPlayer* CGameLobby::GetPlayer(size_t iIndex)
{
	if (iIndex >= m_aClients.size())
		return NULL;

	return &m_aClients[iIndex];
}

CLobbyPlayer* CGameLobby::GetPlayerByClient(size_t iClient)
{
	return GetPlayer(GetPlayerIndex(iClient));
}

void CGameLobby::AddPlayer(size_t iClient)
{
	if (GetPlayerByClient(iClient))
		return;

	CLobbyPlayer* pPlayer = &m_aClients.push_back();
	pPlayer->iClient = iClient;

	::LobbyPlayerInfo.RunCommand(sprintf(L"%d active 1", iClient));
}

void CGameLobby::RemovePlayer(size_t iClient)
{
	size_t iPlayer = GetPlayerIndex(iClient);
	if (!GetPlayer(iPlayer))
		return;

	::ServerChatSay.RunCommand(GetPlayer(iPlayer)->GetInfoValue(L"name") + L" has left the lobby.\n");

	m_aClients.erase(m_aClients.begin()+iPlayer);

	::LobbyPlayerInfo.RunCommand(sprintf(L"%d active 0", iClient));
}

void CGameLobby::UpdateInfo(const eastl::string16& sKey, const eastl::string16& sValue)
{
	m_asInfo[sKey] = sValue;

	eastl::string16 sCommand = sprintf(sKey + L" " + sValue);
	::LobbyInfo.RunCommand(sCommand);
}

void CGameLobby::UpdatePlayer(size_t iClient, const eastl::string16& sKey, const eastl::string16& sValue)
{
	CLobbyPlayer* pPlayer = GetPlayerByClient(iClient);
	if (!pPlayer)
		return;

	eastl::string16 sPreviousValue = pPlayer->asInfo[sKey];
	if (sKey == L"name")
	{
		if (sPreviousValue == L"" && sValue.length() > 0)
			::ServerChatSay.RunCommand(sValue + L" has joined the lobby.\n");
		else
			::ServerChatSay.RunCommand(sPreviousValue + L" is now known as " + sValue + L".\n");
	}

	pPlayer->asInfo[sKey] = sValue;

	eastl::string16 sCommand = sprintf(eastl::string16(L"%d ") + sKey + L" " + sValue, iClient);
	::LobbyPlayerInfo.RunCommand(sCommand);
}

void CGameLobby::SendFullUpdate(size_t iClient)
{
	for (eastl::map<eastl::string16, eastl::string16>::iterator it = m_asInfo.begin(); it != m_asInfo.end(); it++)
	{
		eastl::string16 sCommand = it->first + L" " + it->second;
		::LobbyInfo.RunCommand(sCommand, iClient);
	}

	for (size_t i = 0; i < m_aClients.size(); i++)
	{
		CLobbyPlayer* pPlayer = &m_aClients[i];

		::LobbyPlayerInfo.RunCommand(sprintf(L"%d active 1", pPlayer->iClient), iClient);

		for (eastl::map<eastl::string16, eastl::string16>::iterator it = pPlayer->asInfo.begin(); it != pPlayer->asInfo.end(); it++)
		{
			eastl::string16 sCommand = sprintf(eastl::string16(L"%d ") + it->first + L" " + it->second, pPlayer->iClient);
			::LobbyPlayerInfo.RunCommand(sCommand, iClient);
		}
	}
}
