#ifndef _NETWORK_H
#define _NETWORK_H

#include <map>
#include <vector>
#include <string>

#include <color.h>

#define NET_CALLBACK(type, pfn) \
	virtual void pfn(CNetworkParameters* p); \
	static void pfn##Callback(INetworkListener* obj, CNetworkParameters* p) \
	{ \
		((type*)obj)->pfn(p); \
	}

#define NET_CALLBACK_ENTITY(type, entity, pfn) \
	virtual void pfn(CNetworkParameters* p) \
	{ \
	CEntityHandle<entity> hEntity(p->ui1); \
	if (hEntity.GetPointer() != NULL && hEntity != NULL) \
		hEntity->pfn(p); \
	} \
	static void pfn##Callback(INetworkListener* obj, CNetworkParameters* p) \
	{ \
		((type*)obj)->pfn(p); \
	}

class CNetworkParameters
{
public:
	CNetworkParameters()
	{
		memset(this, 0, sizeof(CNetworkParameters));
	}

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

class INetworkListener
{
public:
	typedef void (*Callback)(INetworkListener*, CNetworkParameters*);
};

class CRegisteredFunction
{
public:
	const char*						m_pszFunction;
	INetworkListener*				m_pListener;
	INetworkListener::Callback		m_pfnCallback;
	std::vector<size_t>				m_pParameters;
};

enum
{
	NET_INT,
	NET_FLOAT,
	NET_HANDLE,
};

class CNetwork
{
public:
	static void				Initialize();
	static void				Deinitialize();

	static void				RegisterFunction(const char* pszName, INetworkListener* pListener, INetworkListener::Callback pfnCallback, size_t iParameters, ...);

	static void				CreateHost(int iPort, INetworkListener* pListener, INetworkListener::Callback pfnClientConnect, INetworkListener::Callback pfnClientDisconnect);
	static void				ConnectToHost(const char* pszHost, int iPort);

	static bool				IsConnected() { return s_bConnected; };
	static bool				IsHost();

	static bool				IsRunningClientFunctions();
	static bool				ShouldRunClientFunction() { return IsHost() || IsRunningClientFunctions(); };
	static bool				ShouldReplicateClientFunction() { return !IsRunningClientFunctions(); };

	static void				Disconnect();

	static void				Think();

	static void				CallFunction(int iClient, const char* pszName, ...);
	static void				CallFunction(int iClient, CRegisteredFunction* pFunction, CNetworkParameters* p);
	static void				CallbackFunction(const char* pszName, CNetworkParameters* p);

protected:
	static bool				s_bInitialized;
	static bool				s_bConnected;
	static std::map<std::string, CRegisteredFunction> s_aFunctions;
	static INetworkListener* s_pClientListener;
	static INetworkListener::Callback s_pfnClientConnect;
	static INetworkListener::Callback s_pfnClientDisconnect;
};

#endif