#include "lobby_server.h"

#include <network/network.h>
#include <network/commands.h>
#include <tinker/application.h>

extern CNetworkCommand FullUpdate;
extern CNetworkCommand LobbyInfo;
extern CNetworkCommand LobbyPlayerInfo;
extern CNetworkCommand ServerChatSay;
extern CNetworkCommand BeginGame;

CLIENT_COMMAND(JoinLobby)
{
	if (pCmd->GetNumArguments() < 1)
	{
		TMsg("JoinLobby not enough arguments\n");
		return;
	}

	CGameLobbyServer::AddPlayer(pCmd->ArgAsUInt(0), iClient);
}

CLIENT_COMMAND(LeaveLobby)
{
	CGameLobbyServer::RemovePlayer(CGameLobbyServer::GetClientPlayerID(iClient));
}

CLIENT_COMMAND(UpdateLobbyInfo)
{
	if (pCmd->GetNumArguments() < 2)
	{
		TMsg("UpdateInfo not enough arguments\n");
		return;
	}

	size_t iID = CGameLobbyServer::GetClientPlayerID(iClient);

	CGameLobby* pLobby = CGameLobbyServer::GetLobby(CGameLobbyServer::GetPlayerLobby(iID));
	if (!pLobby)
		return;

	CLobbyPlayer* pPlayer = pLobby->GetPlayerByClient(iClient);
	if (!pPlayer)
		return;

	if (pPlayer->GetInfoValue(L"host") != L"1")
		return;

	eastl::string16 sValue = sParameters.substr(sParameters.find(L' ')+1);
	CGameLobbyServer::UpdateLobby(CGameLobbyServer::GetPlayerLobby(iID), pCmd->Arg(0), sValue);
}

CLIENT_COMMAND(UpdatePlayerInfo)
{
	if (pCmd->GetNumArguments() < 3)
	{
		TMsg("UpdateInfo not enough arguments\n");
		return;
	}

	size_t iID = CGameLobbyServer::GetClientPlayerID(iClient);

	CGameLobby* pLobby = CGameLobbyServer::GetLobby(CGameLobbyServer::GetPlayerLobby(iID));
	if (!pLobby)
		return;

	CLobbyPlayer* pPlayer = pLobby->GetPlayerByClient(iClient);
	if (!pPlayer)
		return;

	if (pPlayer->GetInfoValue(L"host") != L"1" && pPlayer->iClient != iClient)
		return;

	eastl::string16 sValue = sParameters.substr(sParameters.find(L' ', sParameters.find(L' ')+1)+1);
	CGameLobbyServer::UpdatePlayer(pCmd->ArgAsUInt(0), pCmd->Arg(1), sValue);
}

CLIENT_COMMAND(AddBot)
{
	size_t iLobby = CGameLobbyServer::GetPlayerLobby(CGameLobbyServer::GetClientPlayerID(iClient));
	if (!CGameLobbyServer::GetLobby(iLobby))
		return;

	CLobbyPlayer* pSender = CGameLobbyServer::GetLobby(iLobby)->GetPlayerByClient(iClient);
	if (!pSender)
		return;

	if (pSender->GetInfoValue(L"host") != L"1")
		return;

	size_t iID = CGameLobbyServer::AddPlayer(iLobby, -2);
	CGameLobbyServer::UpdatePlayer(iID, L"bot", L"1");
	CGameLobbyServer::UpdatePlayer(iID, L"name", L"Bot");
	CGameLobbyServer::UpdatePlayer(iID, L"ready", L"1");
}

CLIENT_COMMAND(RemovePlayer)
{
	if (pCmd->GetNumArguments() < 1)
	{
		TMsg("RemovePlayer not enough arguments\n");
		return;
	}

	size_t iLobby = CGameLobbyServer::GetPlayerLobby(CGameLobbyServer::GetClientPlayerID(iClient));
	if (!CGameLobbyServer::GetLobby(iLobby))
		return;

	CLobbyPlayer* pSender = CGameLobbyServer::GetLobby(iLobby)->GetPlayerByClient(iClient);
	if (!pSender)
		return;

	if (pSender->GetInfoValue(L"host") != L"1")
		return;

	CGameLobbyServer::RemovePlayer(pCmd->ArgAsUInt(0));
}

eastl::vector<CGameLobby> CGameLobbyServer::s_aLobbies;
eastl::map<size_t, size_t> CGameLobbyServer::s_aiPlayerLobbies;
eastl::map<size_t, size_t> CGameLobbyServer::s_aiClientPlayerIDs;
ILobbyListener* CGameLobbyServer::s_pListener = NULL;

size_t CGameLobbyServer::CreateLobby(size_t iPort)
{
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

size_t CGameLobbyServer::AddPlayer(size_t iLobby, size_t iClient)
{
	if (iLobby >= s_aLobbies.size())
	{
		assert(!"What lobby is this?");
		return ~0;
	}

	size_t iID = GetNextPlayerID();

	s_aLobbies[iLobby].AddPlayer(iID, iClient);

	s_aiPlayerLobbies[iID] = iLobby;
	s_aiClientPlayerIDs[iClient] = iID;

	return iID;
}

void CGameLobbyServer::RemovePlayer(size_t iID)
{
	size_t iLobby = GetPlayerLobby(iID);

	if (iLobby >= s_aLobbies.size())
	{
		assert(!"What lobby is this?");
		return;
	}

	s_aiPlayerLobbies.erase(iID);
	s_aiClientPlayerIDs.erase(s_aLobbies[iLobby].GetPlayerByID(iID)->iClient);

	s_aLobbies[iLobby].RemovePlayer(iID);
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

size_t CGameLobbyServer::GetPlayerLobby(size_t iID)
{
	eastl::map<size_t, size_t>::iterator it = s_aiPlayerLobbies.find(iID);
	if (it == s_aiPlayerLobbies.end())
		return ~0;

	return it->second;
}

size_t CGameLobbyServer::GetClientPlayerID(size_t iClient)
{
	eastl::map<size_t, size_t>::iterator it = s_aiClientPlayerIDs.find(iClient);
	if (it == s_aiClientPlayerIDs.end())
		return ~0;

	return s_aiClientPlayerIDs[iClient];
}

void CGameLobbyServer::UpdateLobby(size_t iLobby, const eastl::string16& sKey, const eastl::string16& sValue)
{
	if (iLobby == ~0)
	{
		assert(!"What lobby is this?");
		return;
	}

	if (s_pListener)
	{
		if (!s_pListener->UpdateLobby(iLobby, sKey, sValue))
			return;
	}

	s_aLobbies[iLobby].UpdateInfo(sKey, sValue);
}

void CGameLobbyServer::UpdatePlayer(size_t iID, const eastl::string16& sKey, const eastl::string16& sValue)
{
	size_t iLobby = GetPlayerLobby(iID);
	if (iLobby == ~0)
	{
		TMsg(sprintf(L"Can't find lobby for client ID# %d\n", iID));
		assert(!"Can't find lobby for client");
		return;
	}

	if (s_pListener)
	{
		if (!s_pListener->UpdatePlayer(iID, sKey, sValue))
			return;
	}

	s_aLobbies[iLobby].UpdatePlayer(iID, sKey, sValue);
}

void CGameLobbyServer::ClientConnect(class INetworkListener*, class CNetworkParameters* pParameters)
{
	int iClient = pParameters->i1;

	AddPlayer(0, iClient);

	s_aLobbies[0].SendFullUpdate(iClient);

	if (s_pListener)
	{
		if (!s_pListener->ClientConnect(iClient))
			RemovePlayer(GetClientPlayerID(iClient));
	}
}

void CGameLobbyServer::ClientDisconnect(class INetworkListener*, class CNetworkParameters* pParameters)
{
	if (s_pListener)
		s_pListener->ClientDisconnect(pParameters->i1);

	int iID = GetClientPlayerID(pParameters->i1);

	RemovePlayer(iID);
}

void CGameLobbyServer::SetListener(ILobbyListener* pListener)
{
	s_pListener = pListener;
}

size_t CGameLobbyServer::GetNextPlayerID()
{
	static size_t iNextID = 0;
	return iNextID++;
}

CGameLobby::CGameLobby()
{
	m_bActive = false;
}

void CGameLobby::Initialize(size_t iPort)
{
	m_bActive = true;
	m_aClients.clear();
}

void CGameLobby::Shutdown()
{
	m_bActive = false;
}

size_t CGameLobby::GetNumPlayers()
{
	return m_aClients.size();
}

size_t CGameLobby::GetPlayerIndexByID(size_t iID)
{
	for (size_t i = 0; i < m_aClients.size(); i++)
	{
		if (m_aClients[i].iID == iID)
			return i;
	}

	return ~0;
}

size_t CGameLobby::GetPlayerIndexByClient(size_t iClient)
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

CLobbyPlayer* CGameLobby::GetPlayerByID(size_t iClient)
{
	return GetPlayer(GetPlayerIndexByID(iClient));
}

CLobbyPlayer* CGameLobby::GetPlayerByClient(size_t iClient)
{
	return GetPlayer(GetPlayerIndexByClient(iClient));
}

void CGameLobby::AddPlayer(size_t iID, size_t iClient)
{
	if (GetPlayerByID(iID))
		return;

	CLobbyPlayer* pPlayer = &m_aClients.push_back();
	pPlayer->iID = iID;
	pPlayer->iClient = iClient;

	::LobbyPlayerInfo.RunCommand(sprintf(L"%d add %d", iID, iClient));
}

void CGameLobby::RemovePlayer(size_t iID)
{
	size_t iPlayer = GetPlayerIndexByID(iID);
	if (!GetPlayer(iPlayer))
		return;

	::ServerChatSay.RunCommand(GetPlayer(iPlayer)->GetInfoValue(L"name") + L" has left the lobby.\n");

	m_aClients.erase(m_aClients.begin()+iPlayer);

	::LobbyPlayerInfo.RunCommand(sprintf(L"%d remove 0", iID));
}

void CGameLobby::UpdateInfo(const eastl::string16& sKey, const eastl::string16& sValue)
{
	m_asInfo[sKey] = sValue;

	eastl::string16 sCommand = sprintf(sKey + L" " + sValue);
	::LobbyInfo.RunCommand(sCommand);
}

void CGameLobby::UpdatePlayer(size_t iID, const eastl::string16& sKey, const eastl::string16& sValue)
{
	CLobbyPlayer* pPlayer = GetPlayerByID(iID);
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

	eastl::string16 sCommand = sprintf(eastl::string16(L"%d ") + sKey + L" " + sValue, iID);
	::LobbyPlayerInfo.RunCommand(sCommand);

	bool bAllPlayersReady = true;
	for (size_t i = 0; i < m_aClients.size(); i++)
	{
		if (m_aClients[i].GetInfoValue(L"ready") != L"1")
		{
			bAllPlayersReady = false;
			break;
		}
	}

	if (bAllPlayersReady)
	{
		::BeginGame.RunCommand(L"");
		m_bActive = false;
	}
}

void CGameLobby::SendFullUpdate(size_t iClient)
{
	::FullUpdate.RunCommand(L"", iClient);

	for (eastl::map<eastl::string16, eastl::string16>::iterator it = m_asInfo.begin(); it != m_asInfo.end(); it++)
	{
		eastl::string16 sCommand = it->first + L" " + it->second;
		::LobbyInfo.RunCommand(sCommand, iClient);
	}

	for (size_t i = 0; i < m_aClients.size(); i++)
	{
		CLobbyPlayer* pPlayer = &m_aClients[i];

		::LobbyPlayerInfo.RunCommand(sprintf(L"%d add %d", pPlayer->iID, pPlayer->iClient), iClient);

		for (eastl::map<eastl::string16, eastl::string16>::iterator it = pPlayer->asInfo.begin(); it != pPlayer->asInfo.end(); it++)
		{
			eastl::string16 sCommand = sprintf(eastl::string16(L"%d ") + it->first + L" " + it->second, pPlayer->iID);
			::LobbyPlayerInfo.RunCommand(sCommand, iClient);
		}
	}
}
