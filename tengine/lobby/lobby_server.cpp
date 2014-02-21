#include "lobby_server.h"

#include <network/network.h>
#include <network/commands.h>
#include <tinker/application.h>
#include <tinker/cvar.h>

extern CNetworkCommand FullUpdate;
extern CNetworkCommand LobbyInfo;
extern CNetworkCommand LobbyPlayerInfo;
extern CNetworkCommand ServerChatSay;
extern CNetworkCommand BeginGame;

CLIENT_COMMAND(CONNECTION_LOBBY, JoinLobby)
{
	if (pCmd->GetNumArguments() < 1)
	{
		TMsg("JoinLobby not enough arguments\n");
		return;
	}

	CGameLobbyServer::AddPlayer(pCmd->ArgAsUInt(0), iClient);
}

CLIENT_COMMAND(CONNECTION_LOBBY, LeaveLobby)
{
	CGameLobbyServer::RemovePlayer(CGameLobbyServer::GetClientPlayerID(iClient));
}

CLIENT_COMMAND(CONNECTION_LOBBY, UpdateLobbyInfo)
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

	if (pPlayer->GetInfoValue("host") != "1")
		return;

	tstring sValue = sParameters.substr(sParameters.find(' ')+1);
	CGameLobbyServer::UpdateLobby(CGameLobbyServer::GetPlayerLobby(iID), pCmd->Arg(0), sValue);
}

CLIENT_COMMAND(CONNECTION_LOBBY, UpdatePlayerInfo)
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

	if (pPlayer->GetInfoValue("host") != "1" && pPlayer->iClient != iClient)
		return;

	tstring sValue = sParameters.substr(sParameters.find(' ', sParameters.find(' ')+1)+1);
	CGameLobbyServer::UpdatePlayer(pCmd->ArgAsUInt(0), pCmd->Arg(1), sValue);
}

CLIENT_COMMAND(CONNECTION_LOBBY, AddLocalPlayer)
{
	size_t iLobby = CGameLobbyServer::GetPlayerLobby(CGameLobbyServer::GetClientPlayerID(iClient));
	if (!CGameLobbyServer::GetLobby(iLobby))
		return;

	CLobbyPlayer* pSender = CGameLobbyServer::GetLobby(iLobby)->GetPlayerByClient(iClient);
	if (!pSender)
		return;

	if (pSender->GetInfoValue("host") != "1")
		return;

	size_t iID = CGameLobbyServer::AddPlayer(iLobby, NETWORK_LOCAL);
	CGameLobbyServer::UpdatePlayer(iID, "bot", "0");
	CGameLobbyServer::UpdatePlayer(iID, "name", "Player");
	CGameLobbyServer::UpdatePlayer(iID, "ready", "1");
}

CLIENT_COMMAND(CONNECTION_LOBBY, AddBot)
{
	size_t iLobby = CGameLobbyServer::GetPlayerLobby(CGameLobbyServer::GetClientPlayerID(iClient));
	if (!CGameLobbyServer::GetLobby(iLobby))
		return;

	CLobbyPlayer* pSender = CGameLobbyServer::GetLobby(iLobby)->GetPlayerByClient(iClient);
	if (!pSender)
		return;

	if (pSender->GetInfoValue("host") != "1")
		return;

	size_t iID = CGameLobbyServer::AddPlayer(iLobby, NETWORK_BOT);
	CGameLobbyServer::UpdatePlayer(iID, "bot", "1");
	CGameLobbyServer::UpdatePlayer(iID, "name", "Bot");
	CGameLobbyServer::UpdatePlayer(iID, "ready", "1");
}

CLIENT_COMMAND(CONNECTION_LOBBY, RemovePlayer)
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

	if (pSender->GetInfoValue("host") != "1")
		return;

	CGameLobbyServer::RemovePlayer(pCmd->ArgAsUInt(0));
}

tvector<CGameLobby> CGameLobbyServer::s_aLobbies;
tmap<size_t, size_t> CGameLobbyServer::s_aiPlayerLobbies;
tmap<size_t, size_t> CGameLobbyServer::s_aiClientPlayerIDs;
ILobbyListener* CGameLobbyServer::s_pListener = NULL;

CVar lobby_debug("lobby_debug", "0");

size_t CGameLobbyServer::CreateLobby()
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
		s_aLobbies[iLobby].m_iLobbyID = iLobby;
	}

	pLobby->Initialize();

	return iLobby;
}

void CGameLobbyServer::DestroyLobby(size_t iLobby)
{
	if (iLobby >= s_aLobbies.size())
	{
		TAssert(!"What lobby is this?");
		return;
	}

	s_aLobbies[iLobby].Shutdown();
}

size_t CGameLobbyServer::AddPlayer(size_t iLobby, size_t iClient)
{
	if (iLobby >= s_aLobbies.size())
	{
		TAssert(!"What lobby is this?");
		return ~0;
	}

	size_t iID = GetNextPlayerID();

	if (lobby_debug.GetBool())
		TMsg(sprintf(tstring("CGameLobbyServer::AddPlayer(%d, %d) = %d\n"), iLobby, iClient, iID));

	s_aiPlayerLobbies[iID] = iLobby;
	s_aiClientPlayerIDs[iClient] = iID;

	s_aLobbies[iLobby].AddPlayer(iID, iClient);

	return iID;
}

void CGameLobbyServer::RemovePlayer(size_t iID)
{
	size_t iLobby = GetPlayerLobby(iID);

	if (iLobby >= s_aLobbies.size())
	{
		TAssert(!"What lobby is this?");
		return;
	}

	if (lobby_debug.GetBool())
		TMsg(sprintf(tstring("CGameLobbyServer::RemovePlayer(%d)\n"), iID));

	s_aiPlayerLobbies.erase(iID);
	s_aiClientPlayerIDs.erase(s_aLobbies[iLobby].GetPlayerByID(iID)->iClient);

	s_aLobbies[iLobby].RemovePlayer(iID);
}

CGameLobby* CGameLobbyServer::GetLobby(size_t iLobby)
{
	if (iLobby >= s_aLobbies.size())
	{
		TAssert(!"What lobby is this?");
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
	tmap<size_t, size_t>::iterator it = s_aiPlayerLobbies.find(iID);
	if (it == s_aiPlayerLobbies.end())
		return ~0;

	return it->second;
}

size_t CGameLobbyServer::GetClientPlayerID(size_t iClient)
{
	tmap<size_t, size_t>::iterator it = s_aiClientPlayerIDs.find(iClient);
	if (it == s_aiClientPlayerIDs.end())
		return ~0;

	return s_aiClientPlayerIDs[iClient];
}

void CGameLobbyServer::UpdateLobby(size_t iLobby, const tstring& sKey, const tstring& sValue)
{
	if (iLobby == ~0)
	{
		TAssert(!"What lobby is this?");
		return;
	}

	if (s_pListener)
	{
		if (!s_pListener->UpdateLobby(iLobby, sKey, sValue))
			return;
	}

	s_aLobbies[iLobby].UpdateInfo(sKey, sValue);
}

void CGameLobbyServer::UpdatePlayer(size_t iID, const tstring& sKey, const tstring& sValue)
{
	size_t iLobby = GetPlayerLobby(iID);
	if (iLobby == ~0)
	{
		TMsg(sprintf(tstring("Can't find lobby for client ID# %d\n"), iID));
		TAssert(!"Can't find lobby for client");
		return;
	}

	if (s_pListener)
	{
		if (!s_pListener->UpdatePlayer(iID, sKey, sValue))
			return;
	}

	s_aLobbies[iLobby].UpdatePlayer(iID, sKey, sValue);
}

void CGameLobbyServer::ClientEnterGame(int iConnection, class INetworkListener*, class CNetworkParameters* pParameters)
{
	TAssert(iConnection == CONNECTION_LOBBY);

	int iClient = pParameters->i1;

	AddPlayer(0, iClient);

	s_aLobbies[0].SendFullUpdate(iClient);

	if (s_pListener)
	{
		if (!s_pListener->ClientConnect(iClient))
		{
			RemovePlayer(GetClientPlayerID(iClient));
			return;
		}
	}

	if (s_aLobbies[0].GetInfoValue("gameactive") == "1")
		::BeginGame.RunCommand("", iClient);
}

void CGameLobbyServer::ClientDisconnect(int iConnection, class INetworkListener*, class CNetworkParameters* pParameters)
{
	TAssert(iConnection == CONNECTION_LOBBY);

	if (s_pListener)
		s_pListener->ClientDisconnect(pParameters->i1);

	int iID = GetClientPlayerID(pParameters->i1);

	if (iID >= 0)
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

void CGameLobby::Initialize()
{
	m_bActive = true;
	m_aClients.clear();
	m_asInfo.clear();

	m_asInfo["gameactive"] = "0";
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

	if (lobby_debug.GetBool())
		TMsg(sprintf(tstring("CGameLobby::AddPlayer(%d, %d)\n"), iID, iClient));

	CLobbyPlayer* pPlayer = &m_aClients.push_back();
	pPlayer->iID = iID;
	pPlayer->iClient = iClient;

	::LobbyPlayerInfo.RunCommand(sprintf(tstring("%d add %d"), iID, iClient));
}

void CGameLobby::RemovePlayer(size_t iID)
{
	size_t iPlayer = GetPlayerIndexByID(iID);
	if (!GetPlayer(iPlayer))
		return;

	if (lobby_debug.GetBool())
		TMsg(sprintf(tstring("CGameLobby::RemovePlayer(%d)\n"), iID));

	::ServerChatSay.RunCommand(CONNECTION_LOBBY, GetPlayer(iPlayer)->GetInfoValue("name") + " has left the lobby.\n");

	m_aClients.erase(m_aClients.begin()+iPlayer);

	::LobbyPlayerInfo.RunCommand(sprintf(tstring("%d remove 0"), iID));
}

void CGameLobby::UpdateInfo(const tstring& sKey, const tstring& sValue)
{
	m_asInfo[sKey] = sValue;

	tstring sCommand = sprintf(sKey + " " + sValue);
	::LobbyInfo.RunCommand(sCommand);
}

tstring CGameLobby::GetInfoValue(const tstring& sKey)
{
	tmap<tstring, tstring>::iterator it = m_asInfo.find(sKey);

	if (it == m_asInfo.end())
		return "";

	return it->second;
}

void CGameLobby::UpdatePlayer(size_t iID, const tstring& sKey, const tstring& sValue)
{
	CLobbyPlayer* pPlayer = GetPlayerByID(iID);
	if (!pPlayer)
		return;

	tstring sPreviousValue = pPlayer->asInfo[sKey];
	if (sKey == "name")
	{
		if (sPreviousValue == "" && sValue.length() > 0)
			::ServerChatSay.RunCommand(CONNECTION_LOBBY, sValue + " has joined the lobby.\n");
		else if (sPreviousValue != sValue)
			::ServerChatSay.RunCommand(CONNECTION_LOBBY, sPreviousValue + " is now known as " + sValue + ".\n");
	}

	pPlayer->asInfo[sKey] = sValue;

	tstring sCommand = sprintf(tstring("%d ") + sKey + " " + sValue, iID);
	::LobbyPlayerInfo.RunCommand(sCommand);

	bool bBeginGame = true;
	for (size_t i = 0; i < m_aClients.size(); i++)
	{
		if (m_aClients[i].GetInfoValue("ready") != "1")
		{
			bBeginGame = false;
			break;
		}
	}

	if (CGameLobbyServer::s_pListener && !CGameLobbyServer::s_pListener->BeginGame(m_iLobbyID))
		bBeginGame = false;

	if (bBeginGame)
	{
		::BeginGame.RunCommand("");
		m_asInfo["gameactive"] = "1";
	}
}

void CGameLobby::SendFullUpdate(size_t iClient)
{
	::FullUpdate.RunCommand("", iClient);

	for (tmap<tstring, tstring>::iterator it = m_asInfo.begin(); it != m_asInfo.end(); it++)
	{
		tstring sCommand = it->first + " " + it->second;
		::LobbyInfo.RunCommand(sCommand, iClient);
	}

	for (size_t i = 0; i < m_aClients.size(); i++)
	{
		CLobbyPlayer* pPlayer = &m_aClients[i];

		::LobbyPlayerInfo.RunCommand(sprintf(tstring("%d add %d"), pPlayer->iID, pPlayer->iClient), iClient);

		for (tmap<tstring, tstring>::iterator it = pPlayer->asInfo.begin(); it != pPlayer->asInfo.end(); it++)
		{
			tstring sCommand = sprintf(tstring("%d ") + it->first + " " + it->second, pPlayer->iID);
			::LobbyPlayerInfo.RunCommand(sCommand, iClient);
		}
	}
}
