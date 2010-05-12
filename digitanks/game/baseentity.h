#ifndef DT_BASEENTITY_H
#define DT_BASEENTITY_H

#include <map>
#include <vector>
#include <vector.h>
#include <assert.h>

#include "entityhandle.h"
class CBaseEntity
{
	friend class CGame;

public:
											CBaseEntity();
											~CBaseEntity();

public:
	Vector									GetOrigin() const { return m_vecOrigin; };
	void									SetOrigin(const Vector& vecOrigin) { m_vecOrigin = vecOrigin; };

	Vector									GetLastOrigin() const { return m_vecLastOrigin; };
	void									SetLastOrigin(const Vector& vecOrigin) { m_vecLastOrigin = vecOrigin; };

	Vector									GetVelocity() const { return m_vecVelocity; };
	void									SetVelocity(const Vector& vecVelocity) { m_vecVelocity = vecVelocity; };

	EAngle									GetAngles() const { return m_angAngles; };
	void									SetAngles(const EAngle& angAngles) { m_angAngles = angAngles; };

	Vector									GetGravity() const { return m_vecGravity; };
	void									SetGravity(Vector vecGravity) { m_vecGravity = vecGravity; };

	bool									GetSimulated() { return m_bSimulated; };
	void									SetSimulated(bool bSimulated) { m_bSimulated = bSimulated; };

	size_t									GetHandle() const { return m_iHandle; }

	virtual float							GetTotalHealth() { return m_flTotalHealth; }
	virtual float							GetHealth() { return m_flHealth; }
	virtual bool							IsAlive() { return m_flHealth > 0; }

	virtual void							TakeDamage(CBaseEntity* pAttacker, float flDamage);
	virtual void							Killed();

	virtual void							Render() {};

	void									Delete();
	bool									IsDeleted() { return m_bDeleted; }
	void									SetDeleted() { m_bDeleted = true; }

	virtual void							Think() {};

	virtual bool							ShouldTouch(CBaseEntity* pOther) const { return false; };
	virtual bool							IsTouching(CBaseEntity* pOther) const { return false; };
	virtual void							Touching(CBaseEntity* pOther) {};

	virtual int								GetCollisionGroup() { return m_iCollisionGroup; }
	virtual void							SetCollisionGroup(int iCollisionGroup) { m_iCollisionGroup = iCollisionGroup; }

	static CBaseEntity*						GetEntity(size_t iHandle);
	static size_t							GetEntityHandle(size_t i);
	static CBaseEntity*						GetEntityNumber(size_t i);
	static size_t							GetNumEntities();

protected:
	Vector									m_vecOrigin;
	Vector									m_vecLastOrigin;
	EAngle									m_angAngles;
	Vector									m_vecVelocity;
	Vector									m_vecGravity;

	bool									m_bSimulated;

	size_t									m_iHandle;

	bool									m_bTakeDamage;
	float									m_flTotalHealth;
	float									m_flHealth;

	bool									m_bDeleted;

	std::vector<CEntityHandle<CBaseEntity> >	m_ahTouching;

	int										m_iCollisionGroup;

private:
	static std::map<size_t, CBaseEntity*>	s_apEntityList;
	static size_t							s_iNextEntityListIndex;
};

#endif