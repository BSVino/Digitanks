#ifndef _TINKER_REPLICATION_H
#define _TINKER_REPLICATION_H

#include <common.h>
#include <color.h>
#include <tengine_config.h>
#include <strutils.h>

#include <tinker/shell.h>

#include "network.h"

#define NET_CALLBACK_ENTITY(type, entity, pfn) \
	virtual void pfn(CNetworkParameters* p) \
	{ \
	CEntityHandle<class entity> hEntity(p->ui1); \
	if (hEntity != NULL) \
		hEntity->pfn(p); \
	} \
	static void pfn##Callback(int iConnection, INetworkListener* obj, CNetworkParameters* p) \
	{ \
		TAssert(iConnection == CONNECTION_GAME); \
		((type*)obj)->pfn(p); \
	}

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
	float					m_flUpdateInterval;
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
	virtual void		Set(size_t iDataSize, void* pValue) {}

public:
	bool				m_bDirty;
	float				m_flLastUpdate;
};

template <class C>
class CNetworkedVariable : public CNetworkedVariableBase
{
public:
	CNetworkedVariable()
	{
		// Don't zero it out, the constructor will set whatever value it wants and the rest can remain undefined.
		// memset(&m_oVariable, 0, sizeof(C));

		m_flEpsilon = 0.0001f;
		m_bInitialized = false;
	}

	CNetworkedVariable(const C& c)
	{
		m_oVariable = c;
		m_flEpsilon = 0.0001f;
		m_bInitialized = true;
	}

public:
	const C& operator=(const C& c);
	const C& operator=(const CNetworkedVariable<C>& c);

	inline bool operator==(const C& c) const
	{
		TAssert(m_bInitialized);
		return c == m_oVariable;
	}

	inline bool operator!=(const C& c) const
	{
		TAssert(m_bInitialized);
		return c != m_oVariable;
	}

	inline bool operator!() const
	{
		TAssert(m_bInitialized);

		return !m_oVariable;
	}

	inline C operator+(const C& c) const
	{
		TAssert(m_bInitialized);
		return m_oVariable + c;
	}

	inline C operator-(const C& c) const
	{
		TAssert(m_bInitialized);
		return m_oVariable - c;
	}

	inline const C& operator+=(const C& c)
	{
		TAssert(m_bInitialized);

		if (c == 0)
			return m_oVariable;

		m_bDirty = true;
		m_oVariable += c;
		return m_oVariable;
	}

	inline const C& operator-=(const C& c)
	{
		TAssert(m_bInitialized);

		if (c == 0)
			return m_oVariable;

		m_bDirty = true;
		m_oVariable -= c;
		return m_oVariable;
	}

	// Suffix
	inline C operator++(int)
	{
		TAssert(m_bInitialized);

		m_bDirty = true;
		C oReturn = m_oVariable;
		m_oVariable++;
		return oReturn;
	}

	// Prefix
	inline const C& operator++()
	{
		TAssert(m_bInitialized);

		m_bDirty = true;
		m_oVariable++;
		return m_oVariable;
	}

	// Suffix
	inline C operator--(int)
	{
		TAssert(m_bInitialized);

		m_bDirty = true;
		C oReturn = m_oVariable;
		m_oVariable--;
		return oReturn;
	}

	// Prefix
	inline const C& operator--()
	{
		TAssert(m_bInitialized);

		m_bDirty = true;
		m_oVariable--;
		return m_oVariable;
	}

	inline operator const C&() const
	{
		TAssert(m_bInitialized);

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

	virtual void*		Serialize(size_t& iSize);
	virtual void		Unserialize(size_t iDataSize, void* pValue);
	virtual void		Set(size_t iDataSize, void* pValue);

	void				SetEpsilon(float flEpsilon) { m_flEpsilon = flEpsilon; }
	float				GetEpsilon() { return m_flEpsilon; }

	bool				IsInitialized() { return m_bInitialized; }

public:
	// Everything below will be serialized, change serialization functions otherwise.
	C					m_oVariable;
	float				m_flEpsilon;
	bool				m_bInitialized;
};

template <>
inline const TFloat& CNetworkedVariable<TFloat>::operator=(const TFloat& c)
{
	TFloat flDifference = c - m_oVariable;
	if (m_bInitialized && ((flDifference>TFloat(0))?flDifference:-flDifference) < m_flEpsilon)
		return m_oVariable;

	m_bDirty = true;
	m_oVariable = c;
	m_bInitialized = true;
	return m_oVariable;
}

template <>
inline const TFloat& CNetworkedVariable<TFloat>::operator=(const CNetworkedVariable<TFloat>& c)
{
	TFloat flDifference = c.m_oVariable - m_oVariable;
	if (m_bInitialized && ((flDifference>TFloat(0))?flDifference:-flDifference) < m_flEpsilon)
		return m_oVariable;

	m_bDirty = true;
	m_oVariable = c.m_oVariable;
	m_bInitialized = true;
	return m_oVariable;
}

template <class C>
inline const C& CNetworkedVariable<C>::operator=(const C& c)
{
	m_bInitialized = true;

	if (c == m_oVariable)
		return m_oVariable;

	m_bDirty = true;
	m_oVariable = c;
	return m_oVariable;
}

template <class C>
inline const C& CNetworkedVariable<C>::operator=(const CNetworkedVariable<C>& c)
{
	m_bInitialized = true;

	if (c.m_oVariable == m_oVariable)
		return m_oVariable;

	m_bDirty = true;
	m_oVariable = c.m_oVariable;
	return m_oVariable;
}

template <class C>
inline void* CNetworkedVariable<C>::Serialize(size_t& iSize)
{
	iSize = sizeof(m_oVariable) + sizeof(m_flEpsilon) + sizeof(m_bInitialized);
	return &m_oVariable;
}

template <class C>
inline void CNetworkedVariable<C>::Unserialize(size_t iDataSize, void* pValue)
{
	TAssert(iDataSize == sizeof(m_oVariable) + sizeof(m_flEpsilon) + sizeof(m_bInitialized));
	memcpy(&m_oVariable, pValue, iDataSize);
	m_bInitialized = true;
}

template <class C>
inline void CNetworkedVariable<C>::Set(size_t iDataSize, void* pValue)
{
	TAssert(iDataSize == sizeof(m_oVariable));
	C* pTValue = (C*)pValue;
	m_oVariable = *pTValue;
	m_bInitialized = true;
}

template <class C>
class CNetworkedSTLVector : public CNetworkedVariable<tvector<C> >
{
public:
	// For some reason GCC 4.4.3 won't build without these.
	using CNetworkedVariable<tvector<C> >::m_bInitialized;
	using CNetworkedVariable<tvector<C> >::m_bDirty;
	using CNetworkedVariable<tvector<C> >::m_oVariable;

	CNetworkedSTLVector()
	{
		// Because stl vectors automatically initialize themselves
		m_bInitialized = true;
	}

	inline size_t size() const
	{
		return m_oVariable.size();
	}

	inline C& Index(size_t i)
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

	inline typename tvector<C>::iterator erase(size_t iPosition)
	{
		m_bDirty = true;
		return m_oVariable.erase(m_oVariable.begin()+iPosition);
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
		m_bInitialized = true;
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
		int a = iArraySize%s;
		TAssert(a == 0);
		m_bDirty = true;
		return m_oVariable[s*x + y];
	}

	inline const C& Get2D(size_t s, size_t x, size_t y) const
	{
		int a = iArraySize%s;
		TAssert(a == 0);
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
		TAssert(iElements == iArraySize);
		memcpy(&m_oVariable[0], pValue, iDataSize);
	}

protected:
	C	m_oVariable[iArraySize];
};

class CNetworkedVector : public CNetworkedVariable<TVector>
{
public:
	CNetworkedVector()
	{
		// Because Vectors automatically initialize themselves to the origin
		m_bInitialized = true;
	}

	inline const CNetworkedVector& operator=(const TVector& v)
	{
		if (!m_bInitialized || (m_oVariable - v).LengthSqr() > TFloat(0.0f))
		{
			m_bDirty = true;
			m_oVariable = v;
		}

		m_bInitialized = true;

		return *this;
	}

	inline TVector operator*(TFloat f)
	{
		return m_oVariable * f;
	}

	inline TVector operator*(TFloat f) const
	{
		return m_oVariable * f;
	}

	inline TVector operator/(TFloat f)
	{
		return m_oVariable / f;
	}

	inline TVector operator/(TFloat f) const
	{
		return m_oVariable / f;
	}
};

class CNetworkedEAngle : public CNetworkedVariable<EAngle>
{
public:
	CNetworkedEAngle()
	{
		// Because EAngles automatically initialize themselves
		m_bInitialized = true;
	}

	inline const CNetworkedEAngle& operator=(const EAngle v)
	{
		if (v.p != m_oVariable.p || v.y != m_oVariable.y || v.r != m_oVariable.r)
		{
			m_bDirty = true;
			m_oVariable = v;
		}

		m_bInitialized = true;

		return *this;
	}
};

class CNetworkedColor : public CNetworkedVariable<Color>
{
public:
	CNetworkedColor()
	{
		// Because Colors automatically initialize themselves
		m_bInitialized = true;
	}

	inline const CNetworkedColor& operator=(const Color v)
	{
		if (m_oVariable.r() != v.r() || m_oVariable.g() != v.g() || m_oVariable.b() != v.b())
		{
			m_bDirty = true;
			m_oVariable = v;
		}

		m_bInitialized = true;

		return *this;
	}
};

class CNetworkedString : public CNetworkedVariable<tstring>
{
public:
	CNetworkedString()
	{
		// Strings automatically initialize themselves
		m_bInitialized = true;
	}

	inline const CNetworkedString& operator=(const tstring v)
	{
		if (m_oVariable != v)
		{
			m_bDirty = true;
			m_oVariable = v;
		}

		m_bInitialized = true;

		return *this;
	}

	inline const tstring& operator+=(const tstring& c)
	{
		TAssert(m_bInitialized);

		if (c.length() == 0)
			return m_oVariable;

		m_bDirty = true;
		m_oVariable += c;
		return m_oVariable;
	}

	size_t length() const
	{
		TAssert(m_bInitialized);

		return m_oVariable.length();
	}

	virtual void*		Serialize(size_t& iSize) { iSize = (m_oVariable.size()+1)*sizeof(tstring::value_type); return (void*)m_oVariable.c_str(); }
	virtual void		Unserialize(size_t iDataSize, void* pValue)
	{
		m_oVariable = (tstring::value_type*)pValue;
		m_bInitialized = true;
	}
};

class CGameServerNetwork
{
public:
	static void				UpdateNetworkVariables(int iClient, bool bForceAll = false);
};

#endif
