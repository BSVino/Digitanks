#include "network.h"

#include <enet/enet.h>
#include <assert.h>

#include <strutils.h>
#include <platform.h>

#include <tinker/application.h>

#include "commands.h"

bool CNetwork::s_bInitialized = false;
bool CNetwork::s_bConnected = false;
bool CNetwork::s_bPumping = true;
bool CNetwork::s_bSendCommands = true;
eastl::map<eastl::string, CRegisteredFunction> CNetwork::s_aFunctions;
INetworkListener* CNetwork::s_pClientListener = NULL;
INetworkListener::Callback CNetwork::s_pfnClientConnect = NULL;
INetworkListener::Callback CNetwork::s_pfnClientDisconnect = NULL;

static ENetHost* g_pClient = NULL;
static ENetPeer* g_pClientPeer = NULL;
static ENetHost* g_pServer = NULL;
static eastl::vector<ENetPeer*> g_apServerPeers;
static bool g_bIsRunningClientFunctions = false;
static size_t g_iCurrentClient = 0;
static size_t g_iClientID = 0;

SERVER_COMMAND(SetClientID)
{
	assert(pCmd->GetNumArguments());

	if (!pCmd->GetNumArguments())
		return;

	g_iClientID = pCmd->ArgAsUInt(0);
}

void CNetwork::Initialize()
{
	enet_initialize();

	s_bInitialized = true;
	s_bConnected = false;
}

void CNetwork::Deinitialize()
{
	enet_deinitialize();

	s_bInitialized = false;
	s_bConnected = false;
}

void CNetwork::RegisterFunction(const char* pszName, INetworkListener* pListener, INetworkListener::Callback pfnCallback, size_t iParameters, ...)
{
	s_aFunctions[pszName].m_pszFunction = pszName;

	s_aFunctions[pszName].m_pParameters.clear();

	va_list args;
	va_start(args, iParameters);

	for (int i = 0; i < (int)iParameters; i++)
		s_aFunctions[pszName].m_pParameters.push_back(va_arg(args, int));

	va_end(args);

	s_aFunctions[pszName].m_pListener = pListener;
	s_aFunctions[pszName].m_pfnCallback = pfnCallback;
}

void CNetwork::ClearRegisteredFunctions()
{
	s_aFunctions.clear();
}

void CNetwork::CreateHost(int iPort)
{
	s_bConnected = false;

	ENetAddress oAddress;

    oAddress.host = ENET_HOST_ANY;

	if (iPort)
		oAddress.port = iPort;
	else
		oAddress.port = 30203;

	TMsg(sprintf(L"Creating host on port %d\n", (int)oAddress.port));

	g_pClient = NULL;
	g_pServer = enet_host_create(&oAddress, NETWORK_MAX_CLIENTS, 1, 0, 0);

	g_iClientID = ~0;
	if (g_pServer == NULL)
	{
		TError(L"There was a problem creating the host.\n");
		return;
	}

	s_bConnected = true;
}

void CNetwork::SetCallbacks(INetworkListener* pListener, INetworkListener::Callback pfnClientConnect, INetworkListener::Callback pfnClientDisconnect)
{
	s_pClientListener = pListener;
	s_pfnClientConnect = pfnClientConnect;
	s_pfnClientDisconnect = pfnClientDisconnect;
}

void CNetwork::ConnectToHost(const char* pszHost, int iPort)
{
	if (!s_bInitialized)
		return;

	g_pServer = NULL;
	g_pClient = enet_host_create(NULL, 1, 1, 0, 0);

    if (g_pClient == NULL)
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
		oAddress.port = 30203;

	TMsg(sprintf(L"Connecting to '%s' on port %d\n", convertstring<char, char16_t>(pszHost).c_str(), (int)oAddress.port));

	g_pClientPeer = enet_host_connect(g_pClient, &oAddress, 1, 0);    

	if (g_pClientPeer == NULL)
	{
		TError(L"There was a problem connecting to the server.\n");
		return;
	}

	g_iClientID = ~0;

	if (enet_host_service(g_pClient, &oEvent, 5000) <= 0 || oEvent.type != ENET_EVENT_TYPE_CONNECT)
	{
		TError(L"Did not receive connection event.\n");
		enet_peer_reset(g_pClientPeer);
		return;
	}

	s_bConnected = true;

	float flStartWaitTime = CApplication::Get()->GetTime();
	while (CApplication::Get()->GetTime() - flStartWaitTime < 10)
	{
		CNetwork::Think();
		if (g_iClientID != ~0)
			break;

		SleepMS(50);
	}

	if (g_iClientID == ~0)
	{
		s_bConnected = false;
		TError(L"Did not receive initial Client ID packet.\n");
		enet_peer_reset(g_pClientPeer);
		return;
	}
}

bool CNetwork::IsHost()
{
	if (!IsConnected())
		return true;

	return !!g_pServer;
}

bool CNetwork::IsRunningClientFunctions()
{
	return g_bIsRunningClientFunctions;
}

void CNetwork::SetRunningClientFunctions(bool bRunningClientFunctions)
{
	g_bIsRunningClientFunctions = bRunningClientFunctions;
}

void CNetwork::Disconnect()
{
	if (!s_bConnected)
		return;

	s_bConnected = false;

	g_iClientID = ~0;

	if (g_pClient)
	{
		enet_host_destroy(g_pClient);
	}

	if (g_pServer)
	{
		s_pClientListener = NULL;
		s_pfnClientConnect = NULL;
		s_pfnClientDisconnect = NULL;

		enet_host_destroy(g_pServer);
	}
}

void CNetwork::Think()
{
	ENetEvent oEvent;

	if (!s_bConnected)
		return;

	ENetHost* pHost = g_pClient;
	if (!pHost)
		pHost = g_pServer;
	if (!pHost)
		return;

	if (!s_bPumping)
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
				for (size_t i = 0; i < g_apServerPeers.size(); i++)
				{
					if (!g_apServerPeers[i])
					{
						g_apServerPeers[i] = oEvent.peer;
						iPeer = i;
						break;
					}
				}

				if (iPeer < 0)
				{
					g_apServerPeers.push_back(oEvent.peer);
					iPeer = (int)g_apServerPeers.size()-1;
				}

				SetClientID.RunCommand(sprintf(L"%u", iPeer));

				p.i1 = iPeer;

				s_pfnClientConnect(s_pClientListener, &p);
			}
            break;

		case ENET_EVENT_TYPE_RECEIVE:
			g_bIsRunningClientFunctions = true;

			for (size_t i = 0; i < g_apServerPeers.size(); i++)
			{
				if (oEvent.peer == g_apServerPeers[i])
				{
					g_iCurrentClient = i;
					break;
				}
			}

			if (oEvent.packet->dataLength > (size_t)strlen((const char*)oEvent.packet->data)+1)
				CallbackFunction((const char*)oEvent.packet->data, (CNetworkParameters*)(oEvent.packet->data+strlen((const char*)oEvent.packet->data)+1));
			else
				CallbackFunction((const char*)oEvent.packet->data, NULL);

			g_bIsRunningClientFunctions = false;
			enet_packet_destroy(oEvent.packet);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			if (IsHost())
			{
				for (size_t i = 0; i < g_apServerPeers.size(); i++)
				{
					if (oEvent.peer == g_apServerPeers[i])
					{
						g_apServerPeers[i] = NULL;
						p.i1 = (int)i;
						s_pfnClientDisconnect(s_pClientListener, &p);
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

		if (!s_bPumping)
			break;

		if (!s_bConnected)
			break;
	}
}

void CNetwork::CallFunction(int iClient, const char* pszFunction, ...)
{
	if (!s_bConnected)
		return;

	if (s_aFunctions.find(pszFunction) == s_aFunctions.end())
		return;

	CRegisteredFunction* pFunction = &s_aFunctions[pszFunction];

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

void CNetwork::CallFunctionParameters(int iClient, const char* pszFunction, CNetworkParameters* p)
{
	CRegisteredFunction* pFunction = &s_aFunctions[pszFunction];
	CallFunction(iClient, pFunction, p);
}

void CNetwork::CallFunction(int iClient, CRegisteredFunction* pFunction, CNetworkParameters* p, bool bNoCurrentClient)
{
	if (!s_bConnected)
		return;

	if (!s_bSendCommands)
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

	if (g_pServer)
	{
		assert(iClient != NETWORK_TOSERVER);
		if (iClient == NETWORK_TOCLIENTS || iClient == NETWORK_TOEVERYONE)
		{
			for (size_t i = 0; i < g_apServerPeers.size(); i++)
			{
				if (!g_apServerPeers[i])
					continue;

				if (g_bIsRunningClientFunctions && bNoCurrentClient && g_iCurrentClient == i)
					continue;

				enet_peer_send(g_apServerPeers[i], 0, pPacket);
			}
		}
		else if (iClient >= 0)
		{
			assert (g_apServerPeers[iClient]);

			if (g_apServerPeers[iClient])
				enet_peer_send(g_apServerPeers[iClient], 0, pPacket);
		}

		enet_host_flush(g_pServer);
	}
	else
	{
		assert(iClient != NETWORK_TOCLIENTS);
		if (iClient == NETWORK_TOSERVER || iClient == NETWORK_TOEVERYONE)
		{
			enet_peer_send(g_pClientPeer, 0, pPacket);
			enet_host_flush(g_pClient);
		}
	}
}

void CNetwork::CallbackFunction(const char* pszName, CNetworkParameters* p)
{
	if (s_aFunctions.find(pszName) == s_aFunctions.end())
		return;

	// Since CallbackFunction is called by casting the second argument (p) and the CNetworkParameters destructor won't run.
	// If it did this next line would be a problem!
	if (p)
		p->m_pExtraData = ((unsigned char*)p) + sizeof(*p);

	CRegisteredFunction* pFunction = &s_aFunctions[pszName];

	pFunction->m_pfnCallback(pFunction->m_pListener, p);

	// If I'm host and I got this message from a client, forward it to all of the other clients.
	if (IsHost())
		CallFunction(-1, pFunction, p, true);
}

size_t CNetwork::GetClientsConnected()
{
	size_t iClients = 0;
	for (size_t i = 0; i < g_apServerPeers.size(); i++)
	{
		if (g_apServerPeers[i])
			iClients++;
	}

	return iClients;
}

size_t CNetwork::GetClientConnectionId(size_t iClient)
{
	size_t iClients = 0;
	for (size_t i = 0; i < g_apServerPeers.size(); i++)
	{
		if (g_apServerPeers[i])
		{
			if (iClients == iClient)
				return i;

			iClients++;
		}
	}

	return ~0;
}

size_t CNetwork::GetClientID()
{
	if (IsHost())
		return ~0;

	return g_iClientID;
}
