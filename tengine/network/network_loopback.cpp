#include "network.h"

#include <strutils.h>
#include <tinker_platform.h>

#include <tinker/application.h>

#include "commands.h"

bool CNetwork::s_bInitialized = false;
size_t CNetwork::s_iInstallID = false;
tstring CNetwork::s_sNickname;

class CLoopbackClientPeer
{
public:
	float						m_flTimeConnected;
	size_t						m_iInstallID;
	tstring						m_sNickname;
	bool						m_bLoading;
};

class CLoopbackConnection : public CNetworkConnection
{
public:
								CLoopbackConnection(int iConnection);

public:
	virtual void				CreateHost(int iPort);
	virtual void				ConnectToHost(const char* pszHost, int iPort);

	virtual bool				IsHost();

	virtual void				Disconnect(bool bForced = false);
	virtual void				DisconnectClient(int iClient);

	virtual void				Think();

	virtual void				CallFunction(int iClient, CRegisteredFunction* pFunction, CNetworkParameters* p, bool bNoCurrentClient = false);
	virtual void				CallbackFunction(const char* pszName, CNetworkParameters* p);

	virtual size_t				GetClientsConnected();
	virtual size_t				GetClientConnectionId(size_t iClient);

	virtual void				SetLoading(bool bLoading);
	virtual void				SetClientLoading(int iClient, bool bLoading);
	virtual bool				GetClientLoading(int iClient);

	virtual void				SetClientInfo(size_t iClient, size_t iInstallID, const tstring& sNickname);

	virtual size_t				GetClientInstallID(size_t iClient);
	virtual const tstring& GetClientNickname(size_t iClient);
};

SERVER_COMMAND(CONNECTION_UNDEFINED, SetClientID)
{
	TAssert(pCmd->GetNumArguments());

	if (!pCmd->GetNumArguments())
		return;

	Network(iConnection)->SetClientID(pCmd->ArgAsUInt(0));

	CNetwork::SetClientInfo(CNetwork::GetInstallID(), sParameters.substr(sParameters.find(' ')+1));
}

SERVER_COMMAND(CONNECTION_UNDEFINED, ForceDisconnect)
{
	// I've been forcibly disconnected

	if (Network(iConnection)->IsHost())
		return;

	Network(iConnection)->Disconnect(true);
}

CLIENT_COMMAND(CONNECTION_UNDEFINED, ClientInfo)
{
	Network(iConnection)->SetClientInfo(iClient, pCmd->ArgAsUInt(0), sParameters.substr(sParameters.find(' ')+1));
}

CLIENT_COMMAND(CONNECTION_UNDEFINED, ClientDisconnecting)
{
	// This client is disconnecting

	Network(iConnection)->DisconnectClient(iClient);
}

CLIENT_COMMAND(CONNECTION_UNDEFINED, SetLoading)
{
	Network(iConnection)->SetClientLoading(iClient, pCmd->ArgAsBool(0));
}

void CNetwork::Initialize()
{
	s_bInitialized = true;
}

void CNetwork::Deinitialize()
{
	s_bInitialized = false;
}

tvector<CNetworkConnection*> g_apNetworkConnections;

size_t CNetwork::GetNumConnections()
{
	return g_apNetworkConnections.size();
}

CNetworkConnection* CNetwork::GetConnection(int iConnect)
{
	while ((int)g_apNetworkConnections.size() <= iConnect)
		g_apNetworkConnections.push_back(new CLoopbackConnection(g_apNetworkConnections.size()));

	return g_apNetworkConnections[iConnect];
}

void CNetwork::Think()
{
	for (size_t i = 0; i < g_apNetworkConnections.size(); i++)
		g_apNetworkConnections[i]->Think();
}

void CNetwork::SetClientInfo(size_t iInstallID, const tstring& sNickname)
{
	s_iInstallID = iInstallID;
	s_sNickname = sNickname;
}

CNetworkConnection::CNetworkConnection(int iConnection)
{
	m_iConnection = iConnection;
	m_bConnected = false;
	m_bIsRunningClientFunctions = false;
	m_iCurrentClient = ~0;
	m_iClientID = ~0;
	m_bLoading = false;
	m_pClientListener = NULL;
	m_pfnClientConnect = NULL;
	m_pfnClientEnterGame = NULL;
	m_pfnClientDisconnect = NULL;

	ClearRegisteredFunctions();
}

CLoopbackConnection::CLoopbackConnection(int iConnection)
	: CNetworkConnection(iConnection)
{
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

void CLoopbackConnection::CreateHost(int iPort)
{
	if (!CNetwork::IsInitialized())
		return;

	Disconnect();

	m_bConnected = false;

	TMsg("Creating host on local loopback\n");

	m_bConnected = true;
}

void CNetworkConnection::SetCallbacks(INetworkListener* pListener, INetworkListener::Callback pfnClientConnect, INetworkListener::Callback pfnClientEnterGame, INetworkListener::Callback pfnClientDisconnect)
{
	m_pClientListener = pListener;
	m_pfnClientConnect = pfnClientConnect;
	m_pfnClientEnterGame = pfnClientEnterGame;
	m_pfnClientDisconnect = pfnClientDisconnect;
}

void CLoopbackConnection::ConnectToHost(const char* pszHost, int iPort)
{
	if (!CNetwork::IsInitialized())
		return;

	Disconnect();

	TAssert(!"Can't connect while running local loopback\n");

	m_iClientID = ~0;

	m_bConnected = false;
}

bool CLoopbackConnection::IsHost()
{
	return true;
}

bool CNetworkConnection::IsRunningClientFunctions()
{
	return m_bIsRunningClientFunctions;
}

void CNetworkConnection::SetRunningClientFunctions(bool bRunningClientFunctions)
{
	m_bIsRunningClientFunctions = bRunningClientFunctions;
}

void CLoopbackConnection::Disconnect(bool bForced)
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

	m_iClientID = ~0;

	m_bConnected = false;
}

void CLoopbackConnection::DisconnectClient(int iClient)
{
}

void CLoopbackConnection::Think()
{
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

void CLoopbackConnection::CallFunction(int iClient, CRegisteredFunction* pFunction, CNetworkParameters* p, bool bNoCurrentClient)
{
}

void CLoopbackConnection::CallbackFunction(const char* pszName, CNetworkParameters* p)
{
	if (m_aFunctions.find(pszName) == m_aFunctions.end())
		return;

	CRegisteredFunction* pFunction = &m_aFunctions[pszName];

	tstring sFunction = pFunction->m_pszFunction;
	if (sFunction != "NC")
	{
		if (m_bLoading)
			return;
	}

	// Since CallbackFunction is called by casting the second argument (p) and the CNetworkParameters destructor won't run.
	// If it did this next line would be a problem!
	if (p)
		p->m_pExtraData = ((unsigned char*)p) + sizeof(*p);

	pFunction->m_pfnCallback(m_iConnection, pFunction->m_pListener, p);

	// If I'm host and I got this message from a client, forward it to all of the other clients.
	if (IsHost())
		CallFunction(NETWORK_TOCLIENTS, pFunction, p, true);
}

void CLoopbackConnection::SetLoading(bool bLoading)
{
	bool bWas = m_bLoading;

	m_bLoading = false;
	::SetLoading.RunCommand(m_iConnection, bLoading?"1":"0", NETWORK_TOSERVER);
	m_bLoading = bLoading;
}

void CLoopbackConnection::SetClientLoading(int iClient, bool bLoading)
{
	if (iClient == ~0)
		return;

	if (!bLoading && !m_bLoading)
	{
		CNetworkParameters p;
		p.i1 = iClient;

		if (m_pfnClientEnterGame)
			m_pfnClientEnterGame(m_iConnection, m_pClientListener, &p);
	}
}

bool CLoopbackConnection::GetClientLoading(int iClient)
{
	if (iClient == ~0)
		return m_bLoading;

	return false;
}

size_t CLoopbackConnection::GetClientsConnected()
{
	return 0;
}

size_t CLoopbackConnection::GetClientConnectionId(size_t iClient)
{
	return ~0;
}

void CLoopbackConnection::SetClientInfo(size_t iClient, size_t iInstallID, const tstring& sNickname)
{
	bool bUnique = false;
	tstring sUniqueNickname = sNickname;
	int iTries = 1;

	bool bLoading = m_bLoading;
	m_bLoading = false;

	::SetClientID.RunCommand(m_iConnection, sprintf(tstring("%u ") + sUniqueNickname, iClient), iClient);

	m_bLoading = bLoading;
}

size_t CLoopbackConnection::GetClientInstallID(size_t iClient)
{
	return CNetwork::GetInstallID();
}

const tstring& CLoopbackConnection::GetClientNickname(size_t iClient)
{
	return CNetwork::GetNickname();
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
	TAssert(sizeof(tstring::value_type) == sizeof(tchar));

	tchar* pszData = (tchar*)p->m_pExtraData;

	tstring sCommand(pszData);

	size_t iSpace = sCommand.find(' ');

	tstring sName;
	tstring sParameters;
	if (tstring::npos == iSpace)
	{
		sName = sCommand;
		sParameters = "";
	}
	else
	{
		sName = sCommand.substr(0, iSpace);
		sParameters = sCommand.substr(iSpace+1);
	}

	CNetworkCommand* pCommand = CNetworkCommand::GetCommand(sName);

	if (!pCommand)
	{
		TMsg(sprintf(tstring("Network command '%s' unknown.\n"), sName.c_str()));
		return;
	}

	int iCurrentClient = Network(iConnection)->GetCurrentClient();

	if (sName != "SetLoading" && sName != "ClientInfo" && sName != "SetClientID")
	{
		if (m_bLoading)
			return;

		if (IsHost() && Network(iConnection)->GetClientLoading(iCurrentClient))
			return;
	}

	pCommand->RunCallback(m_iConnection, iCurrentClient, sParameters);
}
