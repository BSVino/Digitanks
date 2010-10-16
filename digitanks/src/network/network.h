#ifndef _NETWORK_H
#define _NETWORK_H

#include <map>
#include <vector>
#include <string>

#include <color.h>
#include <vector.h>

#define NET_CALLBACK(type, pfn) \
	virtual void pfn(CNetworkParameters* p); \
	static void pfn##Callback(INetworkListener* obj, CNetworkParameters* p) \
	{ \
		((type*)obj)->pfn(p); \
	}

#define NET_CALLBACK_ENTITY(type, entity, pfn) \
	virtual void pfn(CNetworkParameters* p) \
	{ \
	CEntityHandle<class entity> hEntity(p->ui1); \
	if (hEntity != NULL) \
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

enum
{
	NETWORK_TOCLIENTS	= -1,	// This message is replicated to all clients when run on the server.
	NETWORK_TOSERVER	= -2,	// This message is a command sent to the server.
	NETWORK_TOEVERYONE	= -3,	// This message is all of the above.
};

typedef void (*NetVarChangeCallback)(class CNetworkedVariableBase* pVariable);

class CNetworkedVariableBase
{
public:
	CNetworkedVariableBase();

public:
	bool				IsDirty() { return m_bDirty; }
	void				SetDirty(bool bDirty) { m_bDirty = bDirty; }

	class CBaseEntity*	GetParent() { return m_pParent; }
	void				SetParent(CBaseEntity* pParent);

	virtual const char*	GetName() { return m_pszName; }
	virtual void		SetName(const char* pszName) { m_pszName = pszName; }

	virtual void		SetCallback(NetVarChangeCallback pfnCallback) { m_pfnChanged = pfnCallback; }

	virtual void*		Serialize(size_t& iSize) { return NULL; }
	virtual void		Unserialize(void* pValue) {}

public:
	bool				m_bDirty;

	// Not using a handle because this object can't exist without its parent.
	class CBaseEntity*	m_pParent;

	const char*			m_pszName;

	NetVarChangeCallback	m_pfnChanged;
};

template <class C>
class CNetworkedVariable : public CNetworkedVariableBase
{
public:
	CNetworkedVariable()
	{
		memset(&m_oVariable, 0, sizeof(C));
	}

	CNetworkedVariable(const C& c)
	{
		m_oVariable = c;
	}

public:
	inline const C& operator=(const C& c)
	{
		if (c == m_oVariable)
			return m_oVariable;

		m_bDirty = true;
		m_oVariable = c;
		return m_oVariable;
	}

	inline const C& operator=(const CNetworkedVariable<C>& c)
	{
		if (c.m_oVariable == m_oVariable)
			return m_oVariable;

		m_bDirty = true;
		m_oVariable = c.m_oVariable;
		return m_oVariable;
	}

	inline bool operator==(const C& c)
	{
		return c == m_oVariable;
	}

	inline bool operator==(const C& c) const
	{
		return c == m_oVariable;
	}

	inline bool operator!=(const C& c)
	{
		return c != m_oVariable;
	}

	inline bool operator!=(const C& c) const
	{
		return c != m_oVariable;
	}

	inline const C& operator+=(const C& c)
	{
		if (c == 0)
			return m_oVariable;

		m_bDirty = true;
		m_oVariable += c;
		return m_oVariable;
	}

	inline const C& operator-=(const C& c)
	{
		if (c == 0)
			return m_oVariable;

		m_bDirty = true;
		m_oVariable -= c;
		return m_oVariable;
	}

	// Suffix
	inline C operator++(int)
	{
		m_bDirty = true;
		C oReturn = m_oVariable;
		m_oVariable++;
		return oReturn;
	}

	// Prefix
	inline const C& operator++()
	{
		m_bDirty = true;
		m_oVariable++;
		return m_oVariable;
	}

	// Suffix
	inline C operator--(int)
	{
		m_bDirty = true;
		C oReturn = m_oVariable;
		m_oVariable--;
		return oReturn;
	}

	// Prefix
	inline const C& operator--()
	{
		m_bDirty = true;
		m_oVariable--;
		return m_oVariable;
	}

	inline operator const C&()
	{
		return m_oVariable;
	}

	inline C Get()
	{
		return m_oVariable;
	}

	inline C Get() const
	{
		return m_oVariable;
	}

	virtual void*		Serialize(size_t& iSize) { iSize = sizeof(C); return &m_oVariable; }
	virtual void		Unserialize(void* pValue) { m_oVariable = *(C*)pValue; }

public:
	C					m_oVariable;
};

class CNetworkedVector : public CNetworkedVariable<Vector>
{
public:
	inline const CNetworkedVector& operator=(const Vector v)
	{
		m_bDirty = true;
		m_oVariable = v;
		return *this;
	}

	inline operator Vector() const
	{
		return m_oVariable;
	}
};

class CNetworkedEAngle : public CNetworkedVariable<EAngle>
{
public:
	inline const CNetworkedEAngle& operator=(const EAngle v)
	{
		m_bDirty = true;
		m_oVariable = v;
		return *this;
	}

	inline operator EAngle() const
	{
		return m_oVariable;
	}
};

class CNetwork
{
public:
	static void				Initialize();
	static void				Deinitialize();

	static void				RegisterFunction(const char* pszName, INetworkListener* pListener, INetworkListener::Callback pfnCallback, size_t iParameters, ...);
	static void				RegisterNetworkVariable(CNetworkedVariableBase* pVariable);
	static void				DeregisterNetworkVariable(CNetworkedVariableBase* pVariable);
	static void				UpdateNetworkVariables(int iClient, bool bForceAll = false);

	static void				CreateHost(int iPort);
	static void				SetCallbacks(INetworkListener* pListener, INetworkListener::Callback pfnClientConnect, INetworkListener::Callback pfnClientDisconnect);
	static void				ConnectToHost(const char* pszHost, int iPort);

	static bool				IsConnected() { return s_bConnected; };
	static bool				IsHost();

	static bool				IsRunningClientFunctions();
	static void				SetRunningClientFunctions(bool bRunningClientFunctions);
	static bool				ShouldRunClientFunction() { return IsHost() || IsRunningClientFunctions(); };
	static bool				ShouldReplicateClientFunction() { return !IsRunningClientFunctions(); };

	static void				Disconnect();

	static void				Think();

	static void				CallFunction(int iClient, const char* pszName, ...);
	static void				CallFunctionParameters(int iClient, const char* pszName, CNetworkParameters* p);
	static void				CallFunction(int iClient, CRegisteredFunction* pFunction, CNetworkParameters* p, bool bNoCurrentClient = false);
	static void				CallbackFunction(const char* pszName, CNetworkParameters* p);

protected:
	static bool				s_bInitialized;
	static bool				s_bConnected;
	static std::map<std::string, CRegisteredFunction> s_aFunctions;
	static INetworkListener* s_pClientListener;
	static INetworkListener::Callback s_pfnClientConnect;
	static INetworkListener::Callback s_pfnClientDisconnect;
	static std::vector<CNetworkedVariableBase*> s_apNetworkedVariables;
};

#endif