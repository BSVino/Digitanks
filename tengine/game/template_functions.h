#include "entityhandle.h"

// GCC 4.4.3 won't compile without the definitions of the functions being after the definition of CBaseEntity

template <class C>
inline CEntityHandle<C>::CEntityHandle(size_t iHandle)
{
	
	if (dynamic_cast<C*>(CBaseEntity::GetEntity(iHandle)))
		m_iHandle = iHandle;
	else
		m_iHandle = ~0;
}

template <class C>
inline bool CEntityHandle<C>::IsEqual(const C* pOther) const
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

template <class C>
inline bool CEntityHandle<C>::IsEqual(C* pOther) const
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

template <class C>
inline C* CEntityHandle<C>::GetPointer() const
{
	return static_cast<C*>(CBaseEntity::GetEntity(m_iHandle));
}
