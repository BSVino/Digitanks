#include "network.h"

#include <enet/enet.h>

bool CNetwork::s_bInitialized = false;
bool CNetwork::s_bConnected = false;
std::map<std::string, CRegisteredFunction> CNetwork::s_aFunctions;
INetworkListener* CNetwork::s_pClientListener = NULL;
INetworkListener::Callback CNetwork::s_pfnClientConnect = NULL;
INetworkListener::Callback CNetwork::s_pfnClientDisconnect = NULL;

static ENetHost* g_pClient = NULL;
static ENetPeer* g_pClientPeer = NULL;
static ENetHost* g_pServer = NULL;
static std::vector<ENetPeer*> g_apServerPeers;
static ENetPeer* g_pCurrentPeer = NULL;
static bool g_bIsRunningClientFunctions = false;

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

	va_list args;
	va_start(args, iParameters);

	for (int i = 0; i < (int)iParameters; i++)
		s_aFunctions[pszName].m_pParameters.push_back(va_arg(args, int));

	va_end(args);

	s_aFunctions[pszName].m_pListener = pListener;
	s_aFunctions[pszName].m_pfnCallback = pfnCallback;
}

void CNetwork::CreateHost(int iPort, INetworkListener* pListener, INetworkListener::Callback pfnClientConnect, INetworkListener::Callback pfnClientDisconnect)
{
	s_bConnected = false;

	s_pClientListener = pListener;
	s_pfnClientConnect = pfnClientConnect;
	s_pfnClientDisconnect = pfnClientDisconnect;

	ENetAddress oAddress;

    oAddress.host = ENET_HOST_ANY;

	if (iPort)
		oAddress.port = iPort;
	else
		oAddress.port = 30203;

	g_pServer = enet_host_create(&oAddress, 32, 1, 0, 0);

	if (g_pServer == NULL)
		return;

	s_bConnected = true;
}

void CNetwork::ConnectToHost(const char* pszHost, int iPort)
{
	if (!s_bInitialized)
		return;

	g_pClient = enet_host_create(NULL, 1, 1, 0, 0);

    if (g_pClient == NULL)
		return;

	ENetAddress oAddress;
	ENetEvent oEvent;

	enet_address_set_host(&oAddress, pszHost);

	if (iPort)
		oAddress.port = iPort;
	else
		oAddress.port = 30203;

	g_pClientPeer = enet_host_connect(g_pClient, &oAddress, 1, 0);    

	if (g_pClientPeer == NULL)
		return;

	if (enet_host_service(g_pClient, &oEvent, 5000) > 0 && oEvent.type == ENET_EVENT_TYPE_CONNECT)
	{
		s_bConnected = true;
		return;
	}

	enet_peer_reset(g_pClientPeer);
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

void CNetwork::Disconnect()
{
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

	CNetworkParameters p;

	while (enet_host_service(pHost, &oEvent, 0) > 0)
	{
		switch (oEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			if (IsHost())
			{
				g_apServerPeers.push_back(oEvent.peer);
				p.p1 = &oEvent.data;
				p.i2 = (int)g_apServerPeers.size()-1;
				s_pfnClientConnect(s_pClientListener, &p);
			}
            break;

		case ENET_EVENT_TYPE_RECEIVE:
			g_bIsRunningClientFunctions = true;
			g_pCurrentPeer = oEvent.peer;
			CallbackFunction((const char*)oEvent.packet->data, (CNetworkParameters*)(oEvent.packet->data+strlen((const char*)oEvent.packet->data)+1));
			g_pCurrentPeer = NULL;
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
						g_apServerPeers.erase(g_apServerPeers.begin()+i);
						p.i1 = (int)i;
						s_pfnClientDisconnect(s_pClientListener, &p);
						break;
					}
				}
			}
			break;
        }
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

void CNetwork::CallFunction(int iClient, CRegisteredFunction* pFunction, CNetworkParameters* p)
{
	if (!s_bConnected)
		return;

	size_t iSize = sizeof(*p) + strlen(pFunction->m_pszFunction) + 1;

	ENetPacket* pPacket = enet_packet_create(NULL, iSize, ENET_PACKET_FLAG_RELIABLE);

	strcpy((char*)pPacket->data, pFunction->m_pszFunction);
	memcpy(pPacket->data+strlen(pFunction->m_pszFunction)+1, p, sizeof(*p));

	if (iClient == -1)
	{
		for (size_t i = 0; i < g_apServerPeers.size(); i++)
		{
			if (g_pCurrentPeer == g_apServerPeers[i])
				continue;

			enet_peer_send(g_apServerPeers[i], 0, pPacket);
		}
	}
	else
		enet_peer_send(g_apServerPeers[iClient], 0, pPacket);

	enet_host_flush(g_pServer);
}

void CNetwork::CallbackFunction(const char* pszName, CNetworkParameters* p)
{
	if (s_aFunctions.find(pszName) == s_aFunctions.end())
		return;

	CRegisteredFunction* pFunction = &s_aFunctions[pszName];

	pFunction->m_pfnCallback(pFunction->m_pListener, p);

	// If I'm host and I got this message from a client, forward it to all of the other clients.
	if (IsHost())
		CallFunction(-1, pFunction, p);
}
