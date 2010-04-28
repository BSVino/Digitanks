#ifndef DT_BASEENTITY_H
#define DT_BASEENTITY_H

#include <map>
#include <vector.h>
#include <assert.h>

class CBaseEntity
{
public:
											CBaseEntity();
											~CBaseEntity();

public:
	Vector									GetOrigin() const { return m_vecOrigin; };
	void									SetOrigin(const Vector& vecOrigin) { m_vecOrigin = vecOrigin; };

	EAngle									GetAngles() const { return m_angAngles; };
	void									SetAngles(const EAngle& angAngles) { m_angAngles = angAngles; };

	size_t									GetHandle() const { return m_iHandle; }
							
	virtual float							GetTotalHealth() { return m_flTotalHealth; }
	virtual float							GetHealth() { return m_flHealth; }
	virtual bool							IsAlive() { return m_flHealth > 0; }

	virtual void							TakeDamage(CBaseEntity* pAttacker, float flDamage);
	virtual void							Killed();

	void									Delete();
	bool									IsDeleted() { return m_bDeleted; }
	void									SetDeleted() { m_bDeleted = true; }

	static CBaseEntity*						GetEntity(size_t iHandle);

protected:
	Vector									m_vecOrigin;
	EAngle									m_angAngles;

	size_t									m_iHandle;

	float									m_flTotalHealth;
	float									m_flHealth;

	bool									m_bDeleted;

private:
	static std::map<size_t, CBaseEntity*>	s_apEntityList;
	static size_t							s_iNextEntityListIndex;
};

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
		m_iHandle = pEntity->GetHandle();
	}

public:
	inline const CEntityHandle& operator=(const C* pEntity)
	{
		m_iHandle = pEntity->GetHandle();
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

	inline operator C*() const
	{
		return GetPointer();
	}

	inline C* operator->() const
	{
		return GetPointer();
	}

	inline bool IsEqual(const C* pOther)
	{
		if (!pOther)
		{
			if (!CBaseEntity::GetEntity(m_iHandle))
				return NULL;

			return m_iHandle == ~0;
		}

		return m_iHandle == pOther->GetHandle();
	}

	inline C* GetPointer() const
	{
		assert(CBaseEntity::GetEntity(m_iHandle));
		return dynamic_cast<C*>(CBaseEntity::GetEntity(m_iHandle));
	}

	size_t	GetHandle() { return m_iHandle; }

protected:
	size_t	m_iHandle;
};

#endif