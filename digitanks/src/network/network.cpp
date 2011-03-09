#include "network.h"

#include <enet/enet.h>
#include <assert.h>

#include <strutils.h>

#include <tinker/application.h>

#include <baseentity.h>
#include <gameserver.h>

bool CNetwork::s_bInitialized = false;
bool CNetwork::s_bConnected = false;
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

void CNetwork::UpdateNetworkVariables(int iClient, bool bForceAll)
{
	size_t iMaxEnts = GameServer()->GetMaxEntities();
	for (size_t i = 0; i < iMaxEnts; i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		size_t iRegistration = pEntity->GetRegistration();

		CEntityRegistration* pRegistration = NULL;
		do
		{
			pRegistration = pEntity->GetRegisteredEntity(iRegistration);

			assert(pRegistration);
			if (!pRegistration)
				break;

			size_t iNetVarsSize = pRegistration->m_aNetworkVariables.size();
			for (size_t j = 0; j < iNetVarsSize; j++)
			{
				CNetworkedVariableData* pVarData = &pRegistration->m_aNetworkVariables[j];
				CNetworkedVariableBase* pVariable = pVarData->GetNetworkedVariableBase(pEntity);

				if (!bForceAll && !pVariable->IsDirty())
					continue;

				CNetworkParameters p;
				p.ui1 = pEntity->GetHandle();

				size_t iDataSize;
				void* pValue = pVariable->Serialize(iDataSize);

				p.CreateExtraData(iDataSize + strlen(pVarData->GetName())+1);
				strcpy((char*)p.m_pExtraData, pVarData->GetName());
				memcpy((unsigned char*)(p.m_pExtraData) + strlen(pVarData->GetName())+1, pValue, iDataSize);

				// UV stands for UpdateValue
				CallFunctionParameters(iClient, "UV", &p);

				// Only reset the dirty flag if all clients got the message.
				if (iClient == NETWORK_TOCLIENTS)
					pVariable->SetDirty(false);
			}
		} while ((iRegistration = pRegistration->m_iParentRegistration) != ~0);
	}
}

void CNetwork::CreateHost(int iPort)
{
	TMsg(sprintf(L"Creating host on port %d\n", iPort));

	s_bConnected = false;

	ENetAddress oAddress;

    oAddress.host = ENET_HOST_ANY;

	if (iPort)
		oAddress.port = iPort;
	else
		oAddress.port = 30203;

	g_pServer = enet_host_create(&oAddress, 32, 1, 0, 0);

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

	TMsg(sprintf(L"Connecting to '%s' on port %d\n", convertstring<char, char16_t>(pszHost), iPort));

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

	g_pClientPeer = enet_host_connect(g_pClient, &oAddress, 1, 0);    

	if (g_pClientPeer == NULL)
	{
		TError(L"There was a problem connecting to the server.\n");
		return;
	}

	if (enet_host_service(g_pClient, &oEvent, 5000) > 0 && oEvent.type == ENET_EVENT_TYPE_CONNECT)
	{
		s_bConnected = true;
		return;
	}

	TError(L"Did not receive connection event.\n");

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

void CNetwork::SetRunningClientFunctions(bool bRunningClientFunctions)
{
	g_bIsRunningClientFunctions = bRunningClientFunctions;
}

void CNetwork::Disconnect()
{
	if (!s_bConnected)
		return;

	s_bConnected = false;

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

	if (IsHost())
		UpdateNetworkVariables(NETWORK_TOCLIENTS);

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

				p.p1 = &oEvent.data;
				p.i2 = iPeer;

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

CNetworkedVariableData::CNetworkedVariableData()
{
	m_iOffset = 0;
}

CNetworkedVariableBase* CNetworkedVariableData::GetNetworkedVariableBase(CBaseEntity* pEntity)
{
	assert(m_iOffset);
	return (CNetworkedVariableBase*)(((size_t)pEntity) + m_iOffset);
}

CNetworkedVariableBase::CNetworkedVariableBase()
{
	m_bDirty = true;
}

void CClientCommand::RunCommand(const eastl::string16& sParameters)
{
	// If we're running client functions then we're going to get this message from the server anyway.
	if (CNetwork::IsRunningClientFunctions())
		return;

	if (CNetwork::IsHost() || !CNetwork::IsConnected())
	{
		wcstok(sParameters, m_asArguments);
		m_pfnCallback(this, -1, sParameters);
		return;
	}

	eastl::string16 sCommand = m_sName + L" " + sParameters;

	CNetworkParameters p;
	p.CreateExtraData(sizeof(eastl::string16::value_type) * (sCommand.length() + 1));
	char16_t* pszData = (char16_t*)p.m_pExtraData;

	assert(sizeof(eastl::string16::value_type) == sizeof(char16_t));
	wcscpy(pszData, sCommand.c_str());

	p.ui1 = GameServer()->GetClientIndex();

	CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "CC", &p);
}

void CClientCommand::RunCallback(size_t iClient, const eastl::string16& sParameters)
{
	wcstok(sParameters, m_asArguments);

	m_pfnCallback(this, iClient, sParameters);
}

size_t CClientCommand::GetNumArguments()
{
	return m_asArguments.size();
}

eastl::string16 CClientCommand::Arg(size_t iArg)
{
	assert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return m_asArguments[iArg];
}

size_t CClientCommand::ArgAsUInt(size_t iArg)
{
	assert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return _wtoi(m_asArguments[iArg].c_str());
}

int CClientCommand::ArgAsInt(size_t iArg)
{
	assert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return _wtoi(m_asArguments[iArg].c_str());
}

float CClientCommand::ArgAsFloat(size_t iArg)
{
	assert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return (float)_wtof(m_asArguments[iArg].c_str());
}

eastl::map<eastl::string16, CClientCommand*>& CClientCommand::GetCommands()
{
	static eastl::map<eastl::string16, CClientCommand*> aCommands;
	return aCommands;
}

CClientCommand* CClientCommand::GetCommand(const eastl::string16& sName)
{
	eastl::map<eastl::string16, CClientCommand*>::iterator it = GetCommands().find(sName);
	if (it == GetCommands().end())
		return NULL;

	return it->second;
}

void CClientCommand::RegisterCommand(CClientCommand* pCommand)
{
	GetCommands()[pCommand->m_sName] = pCommand;
}
