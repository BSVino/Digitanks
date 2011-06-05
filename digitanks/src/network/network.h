#ifndef _TINKER_NETWORK_H
#define _TINKER_NETWORK_H

#include <common.h>

#include <EASTL/map.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>

#include <color.h>
#include <vector.h>
#include <strutils.h>

#define NETWORK_MAX_CLIENTS 32

class INetworkListener
{
public:
	typedef void (*Callback)(INetworkListener*, class CNetworkParameters*);
};

class CNetworkParameters
{
public:
	CNetworkParameters()
	{
		memset(this, 0, sizeof(CNetworkParameters));

		m_pExtraData = NULL;
		m_iExtraDataSize = 0;
	}

	~CNetworkParameters()
	{
		if (m_pExtraData)
			free(m_pExtraData);
	}

public:
	void CreateExtraData(size_t iSize)
	{
		if (m_pExtraData)
			free(m_pExtraData);

		m_pExtraData = malloc(iSize);
		m_iExtraDataSize = iSize;
	}

	size_t SizeOf()
	{
		return sizeof(*this) + m_iExtraDataSize;
	}

public:
	size_t		m_iExtraDataSize;
	void*		m_pExtraData;

	union
	{
		struct
		{
			union
			{
				float fl1;
				int i1;
				size_t ui1;
				void* p1;
			};

			union
			{
				float fl2;
				int i2;
				size_t ui2;
				void* p2;
			};

			union
			{
				float fl3;
				int i3;
				size_t ui3;
				void* p3;
			};

			union
			{
				float fl4;
				int i4;
				size_t ui4;
				void* p4;
			};

			union
			{
				float fl5;
				int i5;
				size_t ui5;
				void* p5;
			};
		};

		struct
		{
			union
			{
				float fl;
				int i;
				size_t ui;
				void* p;
			};
		} p[5];
	};
};

class CRegisteredFunction
{
public:
	const char*						m_pszFunction;
	INetworkListener*				m_pListener;
	INetworkListener::Callback		m_pfnCallback;
	eastl::vector<size_t>			m_pParameters;
};

enum
{
	NET_INT,
	NET_FLOAT,
	NET_HANDLE,
};

enum
{
	NETWORK_TOCLIENTS	= -1,	// This message is replicated to all clients when run on the server.
	NETWORK_TOSERVER	= -2,	// This message is a command sent to the server.
	NETWORK_TOEVERYONE	= -3,	// This message is all of the above.
};

class CNetwork
{
public:
	static void				Initialize();
	static void				Deinitialize();

	static void				ClearRegisteredFunctions();
	static void				RegisterFunction(const char* pszName, INetworkListener* pListener, INetworkListener::Callback pfnCallback, size_t iParameters, ...);

	static void				CreateHost(int iPort);
	static void				SetCallbacks(INetworkListener* pListener, INetworkListener::Callback pfnClientConnect, INetworkListener::Callback pfnClientDisconnect);
	static void				ConnectToHost(const char* pszHost, int iPort);

	static bool				IsConnected() { return s_bConnected; };
	static bool				IsHost();

	static bool				IsRunningClientFunctions();
	static void				SetRunningClientFunctions(bool bRunningClientFunctions);
	static bool				ShouldRunClientFunction() { return IsHost() || IsRunningClientFunctions(); };
	static bool				ShouldReplicateClientFunction() { return !IsRunningClientFunctions(); };

	static void				Disconnect(bool bForced = false);
	static void				DisconnectClient(int iClient);

	static void				Think();

	static void				CallFunction(int iClient, const char* pszName, ...);
	static void				CallFunctionParameters(int iClient, const char* pszName, CNetworkParameters* p);
	static void				CallFunction(int iClient, CRegisteredFunction* pFunction, CNetworkParameters* p, bool bNoCurrentClient = false);
	static void				CallbackFunction(const char* pszName, CNetworkParameters* p);

	static void				SendCommands(bool bSend) { s_bSendCommands = bSend; };
	static void				SuspendPumping() { s_bPumping = false; };
	static void				ResumePumping() { s_bPumping = true; };

	static size_t			GetClientsConnected();
	static size_t			GetClientConnectionId(size_t iClient);	// Server only, for iterating over GetClientsConnected() clients, returns ~0 if invalid

	static size_t			GetClientID();	// Client only

protected:
	static bool				s_bInitialized;
	static bool				s_bConnected;
	static bool				s_bPumping;
	static bool				s_bSendCommands;
	static eastl::map<eastl::string, CRegisteredFunction> s_aFunctions;
	static INetworkListener* s_pClientListener;
	static INetworkListener::Callback s_pfnClientConnect;
	static INetworkListener::Callback s_pfnClientDisconnect;
};

#endif