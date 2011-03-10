#ifndef _NETWORK_H
#define _NETWORK_H

#include <assert.h>

#include <EASTL/map.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>

#include <color.h>
#include <vector.h>
#include <strutils.h>

typedef void (*CommandServerCallback)(class CNetworkCommand* pCmd, size_t iClient, const eastl::string16& sParameters);

class CNetworkCommand
{
public:
	CNetworkCommand(eastl::string16 sName, CommandServerCallback pfnCallback, int iTarget)
	{
		m_sName = str_replace(sName, L" ", L"-");
		m_pfnCallback = pfnCallback;
		m_iMessageTarget = iTarget;
	};

public:
	void					RunCommand(const eastl::string16& sParameters);
	void					RunCallback(size_t iClient, const eastl::string16& sParameters);

	// Flips the message around, it becomes a message to all clients
	int						GetMessageTarget() { return m_iMessageTarget; };

	size_t					GetNumArguments();
	eastl::string16			Arg(size_t i);
	size_t					ArgAsUInt(size_t i);
	int						ArgAsInt(size_t i);
	float					ArgAsFloat(size_t i);

	static eastl::map<eastl::string16, CNetworkCommand*>& GetCommands();
	static CNetworkCommand*	GetCommand(const eastl::string16& sName);
	static void				RegisterCommand(CNetworkCommand* pCommand);

protected:
	eastl::string16			m_sName;
	CommandServerCallback	m_pfnCallback;

	eastl::vector<eastl::string16> m_asArguments;

	int						m_iMessageTarget;
};

#define CLIENT_COMMAND(name) \
void ClientCommand_##name(CNetworkCommand* pCmd, size_t iClient, const eastl::string16& sParameters); \
CNetworkCommand name(convertstring<char, char16_t>(#name), ClientCommand_##name, NETWORK_TOSERVER); \
class CRegisterClientCommand##name \
{ \
public: \
	CRegisterClientCommand##name() \
	{ \
		CNetworkCommand::RegisterCommand(&name); \
	} \
} g_RegisterClientCommand##name = CRegisterClientCommand##name(); \
void ClientCommand_##name(CNetworkCommand* pCmd, size_t iClient, const eastl::string16& sParameters) \

#define SERVER_COMMAND(name) \
void ServerCommand_##name(CNetworkCommand* pCmd, size_t iClient, const eastl::string16& sParameters); \
CNetworkCommand name(convertstring<char, char16_t>(#name), ServerCommand_##name, NETWORK_TOCLIENTS); \
class CRegisterServerCommand##name \
{ \
public: \
	CRegisterServerCommand##name() \
	{ \
		CNetworkCommand::RegisterCommand(&name); \
	} \
} g_RegisterServerCommand##name = CRegisterServerCommand##name(); \
void ServerCommand_##name(CNetworkCommand* pCmd, size_t iClient, const eastl::string16& sParameters) \

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

typedef void (*NetVarChangeCallback)(class CNetworkedVariableBase* pVariable);

class CNetworkedVariableData
{
public:
	CNetworkedVariableData();

public:
	const char*				GetName() { return m_pszName; }
	void					SetName(const char* pszName) { m_pszName = pszName; }

	class CNetworkedVariableBase*	GetNetworkedVariableBase(class CBaseEntity* pEntity);

public:
	size_t					m_iOffset;
	const char*				m_pszName;
	NetVarChangeCallback	m_pfnChanged;
};

class CNetworkedVariableBase
{
public:
	CNetworkedVariableBase();

public:
	bool				IsDirty() { return m_bDirty; }
	void				SetDirty(bool bDirty) { m_bDirty = bDirty; }

	virtual void*		Serialize(size_t& iSize) { return NULL; }
	virtual void		Unserialize(size_t iDataSize, void* pValue) {}

public:
	bool				m_bDirty;
};

template <class C>
class CNetworkedVariable : public CNetworkedVariableBase
{
public:
	CNetworkedVariable()
	{
		// Don't zero it out, the constructor will set whatever value it wants and the rest can remain undefined.
		// memset(&m_oVariable, 0, sizeof(C));
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

	inline C operator+(const C& c) const
	{
		return m_oVariable + c;
	}

	inline C operator-(const C& c) const
	{
		return m_oVariable - c;
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

	inline operator const C&() const
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
	virtual void		Unserialize(size_t iDataSize, void* pValue) { m_oVariable = *(C*)pValue; }

public:
	C					m_oVariable;
};

template <class C>
class CNetworkedSTLVector : public CNetworkedVariable<eastl::vector<C> >
{
public:
	inline size_t size() const
	{
		return m_oVariable.size();
	}

	inline C& operator[] (size_t i)
	{
		m_bDirty = true;
		return m_oVariable[i];
	}

	inline const C& operator[] (size_t i) const
	{
		return m_oVariable[i];
	}

	inline void clear()
	{
		m_bDirty = true;
		m_oVariable.clear();
	}

	inline void push_back(const C& value)
	{
		m_bDirty = true;
		m_oVariable.push_back(value);
	}

	inline C& push_back()
	{
		m_bDirty = true;
		return m_oVariable.push_back();
	}

	inline typename eastl::vector<C>::iterator erase(typename eastl::vector<C>::iterator position)
	{
		m_bDirty = true;
		return m_oVariable.erase(position);
	}

	inline typename eastl::vector<C>::iterator begin()
	{
		m_bDirty = true;
		return m_oVariable.begin();
	}

	inline const typename eastl::vector<C>::iterator begin() const
	{
		return m_oVariable.begin();
	}

	inline typename eastl::vector<C>::iterator end()
	{
		m_bDirty = true;
		return m_oVariable.end();
	}

	inline const typename eastl::vector<C>::iterator end() const
	{
		return m_oVariable.end();
	}

	virtual void* Serialize(size_t& iSize)
	{
		if (m_oVariable.size())
		{
			iSize = m_oVariable.size()*sizeof(C);
			return (void*)&m_oVariable[0];
		}

		iSize = 0;
		return (void*)&m_oVariable;
	}

	virtual void Unserialize(size_t iDataSize, void* pValue)
	{
		size_t iElements = iDataSize/sizeof(C);
		m_oVariable.clear();
		m_oVariable.reserve(iElements);
		C* pData = (C*)pValue;
		for (size_t i = 0; i < iElements; i++)
			m_oVariable.push_back(*pData++);
	}
};

template <class C, size_t iArraySize>
class CNetworkedArray : public CNetworkedVariableBase
{
public:
	inline size_t size() const
	{
		return iArraySize;
	}

	inline C& operator[] (size_t i)
	{
		m_bDirty = true;
		return m_oVariable[i];
	}

	inline const C& operator[] (size_t i) const
	{
		return m_oVariable[i];
	}

	inline C& Get2D(size_t s, size_t x, size_t y)
	{
		assert(iArraySize%s == 0);
		m_bDirty = true;
		return m_oVariable[s*x + y];
	}

	inline const C& Get2D(size_t s, size_t x, size_t y) const
	{
		assert(iArraySize%s == 0);
		return m_oVariable[s*x + y];
	}

	virtual void* Serialize(size_t& iSize)
	{
		iSize = iArraySize * sizeof(C);
		return (void*)&m_oVariable[0];
	}

	virtual void Unserialize(size_t iDataSize, void* pValue)
	{
		size_t iElements = iDataSize / sizeof(C);
		assert(iElements == iArraySize);
		memcpy(&m_oVariable[0], pValue, iDataSize);
	}

protected:
	C	m_oVariable[iArraySize];
};

class CNetworkedVector : public CNetworkedVariable<Vector>
{
public:
	inline const CNetworkedVector& operator=(const Vector v)
	{
		if ((m_oVariable - v).LengthSqr() > 0)
		{
			m_bDirty = true;
			m_oVariable = v;
		}

		return *this;
	}

	inline Vector operator*(float f)
	{
		return m_oVariable * f;
	}

	inline Vector operator*(float f) const
	{
		return m_oVariable * f;
	}

	inline Vector operator/(float f)
	{
		return m_oVariable / f;
	}

	inline Vector operator/(float f) const
	{
		return m_oVariable / f;
	}
};

class CNetworkedEAngle : public CNetworkedVariable<EAngle>
{
public:
	inline const CNetworkedEAngle& operator=(const EAngle v)
	{
		if (v.p != m_oVariable.p || v.p != m_oVariable.p || v.p != m_oVariable.p)
		{
			m_bDirty = true;
			m_oVariable = v;
		}

		return *this;
	}
};

class CNetworkedColor : public CNetworkedVariable<Color>
{
public:
	inline const CNetworkedColor& operator=(const Color v)
	{
		if (m_oVariable.r() != v.r() || m_oVariable.g() != v.g() || m_oVariable.b() != v.b())
		{
			m_bDirty = true;
			m_oVariable = v;
		}

		return *this;
	}
};

class CNetworkedString : public CNetworkedVariable<eastl::string16>
{
public:
	inline const CNetworkedString& operator=(const eastl::string16 v)
	{
		if (m_oVariable != v)
		{
			m_bDirty = true;
			m_oVariable = v;
		}

		return *this;
	}

	virtual void*		Serialize(size_t& iSize) { iSize = (m_oVariable.size()+1)*sizeof(eastl::string16::value_type); return (void*)m_oVariable.c_str(); }
	virtual void		Unserialize(size_t iDataSize, void* pValue) { m_oVariable = (eastl::string16::value_type*)pValue; }
};

class CNetwork
{
public:
	static void				Initialize();
	static void				Deinitialize();

	static void				ClearRegisteredFunctions();
	static void				RegisterFunction(const char* pszName, INetworkListener* pListener, INetworkListener::Callback pfnCallback, size_t iParameters, ...);
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

	static size_t			GetClientsConnected();
	static size_t			GetClientConnectionId(size_t iClient);

protected:
	static bool				s_bInitialized;
	static bool				s_bConnected;
	static eastl::map<eastl::string, CRegisteredFunction> s_aFunctions;
	static INetworkListener* s_pClientListener;
	static INetworkListener::Callback s_pfnClientConnect;
	static INetworkListener::Callback s_pfnClientDisconnect;
};

#endif