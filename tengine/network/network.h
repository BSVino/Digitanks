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
	typedef void (*Callback)(int iConnection, INetworkListener*, class CNetworkParameters*);
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

typedef enum
{
	NETWORK_LOCAL			= -1,	// -1 means the host who is also a client.
	NETWORK_BOT				= -2,	// -2 is a bot running on the host.
	NETWORK_TOCLIENTS		= -3,	// This message is replicated to all clients when run on the server. It's also sent to the host through prediction.
	NETWORK_TOREMOTECLIENTS	= -4,	// This message is replicated to all clients when run on the server, but not predicted to the host.
	NETWORK_TOSERVER		= -5,	// This message is a command sent to the server.
	NETWORK_TOEVERYONE		= -6,	// This message is all of the above.
} network_id_t;

enum
{
	CONNECTION_UNDEFINED = 0,
	CONNECTION_LOBBY,
	CONNECTION_GAME,
};

#define NET_CALLBACK(type, pfn) \
	virtual void pfn(int iConnection, CNetworkParameters* p); \
	static void pfn##Callback(int iConnection, INetworkListener* obj, CNetworkParameters* p) \
	{ \
		((type*)obj)->pfn(iConnection, p); \
	}

class CNetwork
{
public:
	static void			Initialize();
	static void			Deinitialize();

	static size_t		GetNumConnections();
	static class CNetworkConnection* GetConnection(int iConnection);

	static bool			IsInitialized() { return s_bInitialized; }

	static void			Think();

	static void			SetClientInfo(size_t iInstallID, const tstring& sNickname);

	static size_t		GetInstallID() { return s_iInstallID; }
	static const tstring& GetNickname() { return s_sNickname; }

protected:
	static bool			s_bInitialized;

	static size_t			s_iInstallID;
	static tstring	s_sNickname;
};

class CNetworkConnection : public INetworkListener
{
public:
						CNetworkConnection(int iConnection);
	virtual				~CNetworkConnection() {};

public:
	virtual void		ClearRegisteredFunctions();
	virtual void		RegisterFunction(const char* pszName, INetworkListener* pListener, INetworkListener::Callback pfnCallback, size_t iParameters, ...);

	virtual void		CreateHost(int iPort) = 0;
	virtual void		SetCallbacks(INetworkListener* pListener, INetworkListener::Callback pfnClientConnect, INetworkListener::Callback pfnClientEnterGame, INetworkListener::Callback pfnClientDisconnect);
	virtual void		ConnectToHost(const char* pszHost, int iPort) = 0;

	virtual bool		IsConnected() { return m_bConnected; };
	virtual bool		IsHost() = 0;

	virtual bool		IsRunningClientFunctions();
	virtual void		SetRunningClientFunctions(bool bRunningClientFunctions);
	virtual bool		ShouldRunClientFunction() { return IsHost() || IsRunningClientFunctions(); };
	virtual bool		ShouldReplicateClientFunction() { return !IsRunningClientFunctions(); };

	virtual void		Disconnect(bool bForced = false) = 0;
	virtual void		DisconnectClient(int iClient) = 0;

	virtual void		Think() = 0;

	virtual void		CallFunction(int iClient, const char* pszName, ...);
	virtual void		CallFunctionParameters(int iClient, const char* pszName, CNetworkParameters* p);
	virtual void		CallFunction(int iClient, CRegisteredFunction* pFunction, CNetworkParameters* p, bool bNoCurrentClient = false) = 0;
	virtual void		CallbackFunction(const char* pszName, CNetworkParameters* p) = 0;

	virtual void		SetLoading(bool bLoading) = 0;			// Client only, tells the server whether I'm loading or not
	virtual void		SetClientLoading(int iClient, bool bLoading) = 0;
	virtual bool		GetClientLoading(int iClient) = 0;

	virtual size_t		GetCurrentClient() { return m_iCurrentClient; };

	virtual size_t		GetClientsConnected() = 0;
	virtual size_t		GetClientConnectionId(size_t iClient) = 0;	// Server only, for iterating over GetClientsConnected() clients, returns ~0 if invalid

	virtual void		SetClientInfo(size_t iClient, size_t iInstallID, const tstring& sNickname) = 0;

	virtual size_t		GetClientInstallID(size_t iClient) = 0;
	virtual const tstring& GetClientNickname(size_t iClient) = 0;

	// Client only
	virtual void		SetClientID(size_t iID) { m_iClientID = iID; };
	virtual size_t		GetClientID();

	NET_CALLBACK(CNetworkConnection, NetworkCommand);

protected:
	int					m_iConnection;
	bool				m_bConnected;
	bool				m_bLoading;
	eastl::map<eastl::string, CRegisteredFunction> m_aFunctions;
	INetworkListener*	m_pClientListener;
	INetworkListener::Callback m_pfnClientConnect;
	INetworkListener::Callback m_pfnClientEnterGame;
	INetworkListener::Callback m_pfnClientDisconnect;

	bool				m_bIsRunningClientFunctions;
	size_t				m_iCurrentClient;
	size_t				m_iClientID;
};

inline CNetworkConnection* Network(int iNetwork)
{
	return CNetwork::GetConnection(iNetwork);
}

inline CNetworkConnection* LobbyNetwork()
{
	return CNetwork::GetConnection(CONNECTION_LOBBY);
}

inline CNetworkConnection* GameNetwork()
{
	return CNetwork::GetConnection(CONNECTION_GAME);
}

#endif
