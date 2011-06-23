#ifndef DT_ENTITYHANDLE_H
#define DT_ENTITYHANDLE_H

#include <common.h>

#include <network/network.h>
#include <network/replication.h>

template <class C>
class CEntityHandle
{
public:
	CEntityHandle()
	{
		m_iHandle = ~0;
	}

	CEntityHandle(C* pEntity)
	{
		if (!pEntity)
			m_iHandle = ~0;
		else
			m_iHandle = pEntity->GetHandle();
	}

	CEntityHandle(size_t iHandle);

public:
	inline const CEntityHandle& operator=(const C* pEntity)
	{
		Set(pEntity);
		return *this;
	}

	inline bool operator==(const C* pEntity) const
	{
		return IsEqual(pEntity);
	}

	inline bool operator==(const CEntityHandle<C>& hEntity) const
	{
		return IsEqual(hEntity);
	}

	inline bool operator!=(const C* pEntity) const
	{
		return !IsEqual(pEntity);
	}

	inline bool operator!=(const CEntityHandle<C>& hEntity) const
	{
		return !IsEqual(hEntity);
	}

	inline bool operator!() const
	{
		return IsEqual(NULL);
	}

	inline operator C*() const
	{
		return GetPointer();
	}

	inline C* operator->() const
	{
		return GetPointer();
	}

	inline bool IsEqual(const C* pOther) const;

	inline C* GetPointer() const;

	inline void Set(const C* pEntity)
	{
		if (!pEntity)
			m_iHandle = ~0;
		else
			m_iHandle = pEntity->GetHandle();
	}

	size_t	GetHandle() { return m_iHandle; }

protected:
	size_t	m_iHandle;
};

// This shit is whack I don't know how the fuck it works.
template <class T>
class CNetworkedHandle : public CNetworkedVariable<CEntityHandle<T> >
{
public:
	// For some reason GCC 4.4.3 won't build without these.
	using CNetworkedVariable<CEntityHandle<T> >::m_oVariable;
	using CNetworkedVariable<CEntityHandle<T> >::m_bDirty;

	inline const CNetworkedHandle& operator=(const T* pEntity)
	{
		if (m_oVariable == pEntity)
			return *this;

		m_bDirty = true;
		m_oVariable.Set(pEntity);
		return *this;
	}

	inline bool operator!=(const T* pOther)
	{
		return !m_oVariable.IsEqual(pOther);
	}

	inline operator T*() const
	{
		return m_oVariable.GetPointer();
	}

	inline T* operator->() const
	{
		return m_oVariable.GetPointer();
	}

	inline T* GetPointer() const
	{
		return m_oVariable.GetPointer();
	}
};

#endif
