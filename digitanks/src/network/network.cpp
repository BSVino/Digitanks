#include "network.h"

#include <enet/enet.h>

#include <strutils.h>
#include <platform.h>

#include <tinker/application.h>

#include "commands.h"

bool CNetwork::s_bInitialized = false;

class CENetConnection : public CNetworkConnection
{
public:
								CENetConnection(int iConnection);

public:
	virtual void				CreateHost(int iPort);
	virtual void				ConnectToHost(const char* pszHost, int iPort);

	virtual bool				IsHost();

	virtual void				Disconnect(bool bForced = false);
	virtual void				DisconnectClient(int iClient);

	virtual void				Think();

	virtual void				CallFunction(int iClient, CRegisteredFunction* pFunction, CNetworkParameters* p, bool bNoCurrentClient = false);

	virtual size_t				GetClientsConnected();
	virtual size_t				GetClientConnectionId(size_t iClient);

protected:
	ENetHost*					m_pClient;
	ENetPeer*					m_pClientPeer;
	ENetHost*					m_pServer;
	eastl::vector<ENetPeer*>	m_apServerPeers;
};

SERVER_COMMAND(CONNECTION_UNDEFINED, SetClientID)
{
	TAssert(pCmd->GetNumArguments());

	if (!pCmd->GetNumArguments())
		return;

	Network(iConnection)->SetClientID(pCmd->ArgAsUInt(0));
}

SERVER_COMMAND(CONNECTION_UNDEFINED, ForceDisconnect)
{
	// I've been forcibly disconnected

	if (Network(iConnection)->IsHost())
		return;

	Network(iConnection)->Disconnect(true);
}

CLIENT_COMMAND(CONNECTION_UNDEFINED, ClientDisconnecting)
{
	// This client is disconnecting

	Network(iConnection)->DisconnectClient(iClient);
}

void CNetwork::Initialize()
{
	enet_initialize();

	s_bInitialized = true;
}

void CNetwork::Deinitialize()
{
	enet_deinitialize();

	s_bInitialized = false;
}

eastl::vector<CNetworkConnection*> g_apNetworkConnections;

size_t CNetwork::GetNumConnections()
{
	return g_apNetworkConnections.size();
}

CNetworkConnection* CNetwork::GetConnection(int iConnect)
{
	while ((int)g_apNetworkConnections.size() <= iConnect)
		g_apNetworkConnections.push_back(new CENetConnection(g_apNetworkConnections.size()));

	return g_apNetworkConnections[iConnect];
}

void CNetwork::Think()
{
	for (size_t i = 0; i < g_apNetworkConnections.size(); i++)
		g_apNetworkConnections[i]->Think();
}

CNetworkConnection::CNetworkConnection(int iConnection)
{
	m_iConnection = iConnection;
	m_bConnected = false;
	m_bIsRunningClientFunctions = false;
	m_iCurrentClient = ~0;
	m_iClientID = ~0;
	m_bSendCommands = true;
	m_pClientListener = NULL;
	m_pfnClientConnect = NULL;
	m_pfnClientDisconnect = NULL;

	ClearRegisteredFunctions();
}

CENetConnection::CENetConnection(int iConnection)
	: CNetworkConnection(iConnection)
{
	m_pClient = NULL;
	m_pClientPeer = NULL;
	m_pServer = NULL;
}

void CNetworkConnection::RegisterFunction(const char* pszName, INetworkListener* pListener, INetworkListener::Callback pfnCallback, size_t iParameters, ...)
{
	m_aFunctions[pszName].m_pszFunction = pszName;

	m_aFunctions[pszName].m_pParameters.clear();

	va_list args;
	va_start(args, iParameters);

	for (int i = 0; i < (int)iParameters; i++)
		m_aFunctions[pszName].m_pParameters.push_back(va_arg(args, int));

	va_end(args);

	m_aFunctions[pszName].m_pListener = pListener;
	m_aFunctions[pszName].m_pfnCallback = pfnCallback;
}

void CNetworkConnection::ClearRegisteredFunctions()
{
	m_aFunctions.clear();

	RegisterFunction("NC", this, NetworkCommandCallback, 0);
}

void CENetConnection::CreateHost(int iPort)
{
	if (!CNetwork::IsInitialized())
		return;

	Disconnect();

	m_bConnected = false;

	ENetAddress oAddress;

    oAddress.host = ENET_HOST_ANY;

	if (iPort)
		oAddress.port = iPort;
	else
	{
		if (m_iConnection == CONNECTION_LOBBY)
			oAddress.port = 30202;
		else
			oAddress.port = 30203;
	}

	TMsg(sprintf(L"Creating host on port %d\n", (int)oAddress.port));

	m_pClient = NULL;
	m_pServer = enet_host_create(&oAddress, NETWORK_MAX_CLIENTS, 1, 0, 0);

	m_iClientID = ~0;
	if (m_pServer == NULL)
	{
		TError(L"There was a problem creating the host.\n");
		return;
	}

	m_bConnected = true;
}

void CNetworkConnection::SetCallbacks(INetworkListener* pListener, INetworkListener::Callback pfnClientConnect, INetworkListener::Callback pfnClientDisconnect)
{
	m_pClientListener = pListener;
	m_pfnClientConnect = pfnClientConnect;
	m_pfnClientDisconnect = pfnClientDisconnect;
}

void CENetConnection::ConnectToHost(const char* pszHost, int iPort)
{
	if (!CNetwork::IsInitialized())
		return;

	Disconnect();

	m_pServer = NULL;
	m_pClient = enet_host_create(NULL, 1, 1, 0, 0);

    if (m_pClient == NULL)
	{
		TError(L"There was a problem creating the client host.\n");
		return;
	}

	ENetAddress oAddress;
	ENetEvent oEvent;

	enet_address_set_host(&oAddress, pszHost);

	if (iPort)
		oAddress.port = iPort;
	else
	{
		if (m_iConnection == CONNECTION_LOBBY)
			oAddress.port = 30202;
		else
			oAddress.port = 30203;
	}

	TMsg(sprintf(L"Connecting to '%s' on port %d\n", convertstring<char, char16_t>(pszHost).c_str(), (int)oAddress.port));

	m_pClientPeer = enet_host_connect(m_pClient, &oAddress, 1, 0);    

	if (m_pClientPeer == NULL)
	{
		TError(L"There was a problem connecting to the server.\n");
		return;
	}

	m_iClientID = ~0;

	if (enet_host_service(m_pClient, &oEvent, 5000) <= 0 || oEvent.type != ENET_EVENT_TYPE_CONNECT)
	{
		TError(L"Did not receive connection event.\n");
		enet_peer_reset(m_pClientPeer);
		return;
	}

	m_bConnected = true;

	float flStartWaitTime = CApplication::Get()->GetTime();
	while (CApplication::Get()->GetTime() - flStartWaitTime < 10)
	{
		Think();
		if (m_iClientID != ~0)
			break;

		SleepMS(50);
	}

	if (m_iClientID == ~0)
	{
		m_bConnected = false;
		TError(L"Did not receive initial Client ID packet.\n");
		enet_peer_reset(m_pClientPeer);
		return;
	}
}

bool CENetConnection::IsHost()
{
	if (!IsConnected())
		return true;

	return !!m_pServer;
}

bool CNetworkConnection::IsRunningClientFunctions()
{
	return m_bIsRunningClientFunctions;
}

void CNetworkConnection::SetRunningClientFunctions(bool bRunningClientFunctions)
{
	m_bIsRunningClientFunctions = bRunningClientFunctions;
}

void CENetConnection::Disconnect(bool bForced)
{
	if (!m_bConnected)
		return;

	if (bForced)
	{
		if (m_iClientID != ~0)
		{
			CNetworkParameters p;
			p.i1 = (int)m_iClientID;
			if (m_pfnClientDisconnect)
				m_pfnClientDisconnect(m_iConnection, m_pClientListener, &p);
		}
	}

	if (m_pClient)
	{
		if (!bForced)
			// Inform server of disconnection.
			ClientDisconnecting.RunCommand(m_iConnection, L"");

		enet_host_destroy(m_pClient);
		m_pClient = NULL;
	}

	if (m_pServer)
	{
		// Inform all clients of disconnection.
		ForceDisconnect.RunCommand(m_iConnection, L"");

		m_pClientListener = NULL;
		m_pfnClientConnect = NULL;
		m_pfnClientDisconnect = NULL;

		enet_host_destroy(m_pServer);
		m_pServer = NULL;
	}

	m_iClientID = ~0;

	m_bConnected = false;
}

void CENetConnection::DisconnectClient(int iClient)
{
	if (!m_bConnected)
		return;

	if ((size_t)iClient >= m_apServerPeers.size())
		return;

	ForceDisconnect.RunCommand(m_iConnection, L"", iClient);

	enet_peer_reset(m_apServerPeers[iClient]);

	m_apServerPeers[iClient] = NULL;

	CNetworkParameters p;
	p.i1 = (int)iClient;
	m_pfnClientDisconnect(m_iConnection, m_pClientListener, &p);
}

void CENetConnection::Think()
{
	ENetEvent oEvent;

	if (!m_bConnected)
		return;

	ENetHost* pHost = m_pClient;
	if (!pHost)
		pHost = m_pServer;
	if (!pHost)
		return;

	CNetworkParameters p;

	while (enet_host_service(pHost, &oEvent, 0) > 0)
	{
		switch (oEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			if (IsHost())
			{
				// Find the first unused peer.
				int iPeer = -1;
				for (size_t i = 0; i < m_apServerPeers.size(); i++)
				{
					if (!m_apServerPeers[i])
					{
						m_apServerPeers[i] = oEvent.peer;
						iPeer = i;
						break;
					}
				}

				if (iPeer < 0)
				{
					m_apServerPeers.push_back(oEvent.peer);
					iPeer = (int)m_apServerPeers.size()-1;
				}

				::SetClientID.RunCommand(m_iConnection, sprintf(L"%u", iPeer));

				p.i1 = iPeer;

				m_pfnClientConnect(m_iConnection, m_pClientListener, &p);
			}
            break;

		case ENET_EVENT_TYPE_RECEIVE:
			m_bIsRunningClientFunctions = true;

			for (size_t i = 0; i < m_apServerPeers.size(); i++)
			{
				if (oEvent.peer == m_apServerPeers[i])
				{
					m_iCurrentClient = i;
					break;
				}
			}

			if (oEvent.packet->dataLength > (size_t)strlen((const char*)oEvent.packet->data)+1)
				CallbackFunction((const char*)oEvent.packet->data, (CNetworkParameters*)(oEvent.packet->data+strlen((const char*)oEvent.packet->data)+1));
			else
				CallbackFunction((const char*)oEvent.packet->data, NULL);

			m_bIsRunningClientFunctions = false;
			enet_packet_destroy(oEvent.packet);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			if (IsHost())
			{
				for (size_t i = 0; i < m_apServerPeers.size(); i++)
				{
					if (oEvent.peer == m_apServerPeers[i])
					{
						m_apServerPeers[i] = NULL;
						p.i1 = (int)i;
						m_pfnClientDisconnect(m_iConnection, m_pClientListener, &p);
						break;
					}
				}
			}
			else
			{
				Disconnect();
				return;
			}
			break;
        }

		if (!m_bConnected)
			break;
	}
}

void CNetworkConnection::CallFunction(int iClient, const char* pszFunction, ...)
{
	if (!m_bConnected)
		return;

	if (m_aFunctions.find(pszFunction) == m_aFunctions.end())
		return;

	CRegisteredFunction* pFunction = &m_aFunctions[pszFunction];

	CNetworkParameters p;

	int iParameters = (int)pFunction->m_pParameters.size();

	va_list args;
	va_start(args, pszFunction);

	for (size_t i = 0; i < pFunction->m_pParameters.size(); i++)
	{
		if (pFunction->m_pParameters[i] == NET_INT)
			p.p[i].i = (va_arg(args, int));
		else if (pFunction->m_pParameters[i] == NET_FLOAT)
			p.p[i].fl = (float)(va_arg(args, double));
		else if (pFunction->m_pParameters[i] == NET_HANDLE)
			p.p[i].ui = (va_arg(args, size_t));
	}

	va_end(args);

	CallFunction(iClient, pFunction, &p);
}

void CNetworkConnection::CallFunctionParameters(int iClient, const char* pszFunction, CNetworkParameters* p)
{
	CRegisteredFunction* pFunction = &m_aFunctions[pszFunction];
	CallFunction(iClient, pFunction, p);
}

void CENetConnection::CallFunction(int iClient, CRegisteredFunction* pFunction, CNetworkParameters* p, bool bNoCurrentClient)
{
	if (!m_bConnected)
		return;

	if (!m_bSendCommands)
		return;

	size_t iPSize = p?p->SizeOf():0;
	size_t iSize = iPSize + strlen(pFunction->m_pszFunction) + 1;

	ENetPacket* pPacket = enet_packet_create(NULL, iSize, ENET_PACKET_FLAG_RELIABLE);

	strcpy((char*)pPacket->data, pFunction->m_pszFunction);
	if (p)
	{
		memcpy(pPacket->data+strlen(pFunction->m_pszFunction)+1, p, sizeof(*p));
		memcpy(pPacket->data+strlen(pFunction->m_pszFunction)+1+sizeof(*p), p->m_pExtraData, p->m_iExtraDataSize);
	}

	if (m_pServer)
	{
		TAssert(iClient != NETWORK_TOSERVER);
		if (iClient == NETWORK_TOCLIENTS || iClient == NETWORK_TOEVERYONE)
		{
			for (size_t i = 0; i < m_apServerPeers.size(); i++)
			{
				if (!m_apServerPeers[i])
					continue;

				if (m_bIsRunningClientFunctions && bNoCurrentClient && m_iCurrentClient == i)
					continue;

				enet_peer_send(m_apServerPeers[i], 0, pPacket);
			}
		}
		else if (iClient >= 0)
		{
			TAssert(m_apServerPeers[iClient]);

			if (m_apServerPeers[iClient])
				enet_peer_send(m_apServerPeers[iClient], 0, pPacket);
		}

		enet_host_flush(m_pServer);
	}
	else
	{
		TAssert(iClient != NETWORK_TOCLIENTS);
		if (iClient == NETWORK_TOSERVER || iClient == NETWORK_TOEVERYONE)
		{
			enet_peer_send(m_pClientPeer, 0, pPacket);
			enet_host_flush(m_pClient);
		}
	}
}

void CNetworkConnection::CallbackFunction(const char* pszName, CNetworkParameters* p)
{
	if (m_aFunctions.find(pszName) == m_aFunctions.end())
		return;

	// Since CallbackFunction is called by casting the second argument (p) and the CNetworkParameters destructor won't run.
	// If it did this next line would be a problem!
	if (p)
		p->m_pExtraData = ((unsigned char*)p) + sizeof(*p);

	CRegisteredFunction* pFunction = &m_aFunctions[pszName];

	pFunction->m_pfnCallback(m_iConnection, pFunction->m_pListener, p);

	// If I'm host and I got this message from a client, forward it to all of the other clients.
	if (IsHost())
		CallFunction(-1, pFunction, p, true);
}

size_t CENetConnection::GetClientsConnected()
{
	size_t iClients = 0;
	for (size_t i = 0; i < m_apServerPeers.size(); i++)
	{
		if (m_apServerPeers[i])
			iClients++;
	}

	return iClients;
}

size_t CENetConnection::GetClientConnectionId(size_t iClient)
{
	size_t iClients = 0;
	for (size_t i = 0; i < m_apServerPeers.size(); i++)
	{
		if (m_apServerPeers[i])
		{
			if (iClients == iClient)
				return i;

			iClients++;
		}
	}

	return ~0;
}

size_t CNetworkConnection::GetClientID()
{
	if (IsHost())
		return ~0;

	return m_iClientID;
}

void CNetworkConnection::NetworkCommand(int iConnection, CNetworkParameters* p)
{
	TAssert(m_iConnection == iConnection);
	TAssert(sizeof(eastl::string16::value_type) == sizeof(char16_t));

	char16_t* pszData = (char16_t*)p->m_pExtraData;

	eastl::string16 sCommand(pszData);

	size_t iSpace = sCommand.find(L' ');

	eastl::string16 sName;
	eastl::string16 sParameters;
	if (eastl::string16::npos == iSpace)
	{
		sName = sCommand;
		sParameters = L"";
	}
	else
	{
		sName = sCommand.substr(0, iSpace);
		sParameters = sCommand.substr(iSpace+1);
	}

	CNetworkCommand* pCommand = CNetworkCommand::GetCommand(sName);

	if (!pCommand)
	{
		TMsg(sprintf(L"Network command '%s' unknown.\n", sName));
		return;
	}

	pCommand->RunCallback(m_iConnection, p->ui1, sParameters);
}
