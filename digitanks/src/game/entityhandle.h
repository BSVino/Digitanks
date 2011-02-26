#ifndef DT_ENTITYHANDLE_H
#define DT_ENTITYHANDLE_H

#include <assert.h>

#include <network/network.h>

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

	CEntityHandle(size_t iHandle)
	{
		if (dynamic_cast<C*>(CBaseEntity::GetEntity(iHandle)))
			m_iHandle = iHandle;
		else
			m_iHandle = ~0;
	}

public:
	inline const CEntityHandle& operator=(const C* pEntity)
	{
		Set(pEntity);
		return *this;
	}

	inline bool operator==(C* pEntity)
	{
		return IsEqual(pEntity);
	}

	inline bool operator==(const C* pEntity) const
	{
		return IsEqual(pEntity);
	}

	inline bool operator!=(C* pEntity)
	{
		return !IsEqual(pEntity);
	}

	inline bool operator!=(const CEntityHandle<C>& hEntity)
	{
		return !IsEqual(hEntity);
	}

	inline bool operator!=(const C* pEntity) const
	{
		return !IsEqual(pEntity);
	}

	inline operator C*() const
	{
		return GetPointer();
	}

	inline C* operator->() const
	{
		return GetPointer();
	}

	inline bool operator!() const
	{
		return IsEqual(NULL);
	}

	inline bool IsEqual(const C* pOther)
	{
		if (!pOther)
		{
			if (!CBaseEntity::GetEntity(m_iHandle))
				return true;

			if (!dynamic_cast<C*>(CBaseEntity::GetEntity(m_iHandle)))
				return true;

			return m_iHandle == ~0;
		}

		return m_iHandle == pOther->GetHandle();
	}

	inline bool IsEqual(const C* pOther) const
	{
		if (!pOther)
		{
			if (!CBaseEntity::GetEntity(m_iHandle))
				return true;

			if (!dynamic_cast<C*>(CBaseEntity::GetEntity(m_iHandle)))
				return true;

			return m_iHandle == ~0;
		}

		return m_iHandle == pOther->GetHandle();
	}

	inline C* GetPointer() const
	{
		return static_cast<C*>(CBaseEntity::GetEntity(m_iHandle));
	}

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