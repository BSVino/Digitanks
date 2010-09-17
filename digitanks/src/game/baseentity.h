#ifndef DT_BASEENTITY_H
#define DT_BASEENTITY_H

#include <map>
#include <vector>
#include <vector.h>
#include <assert.h>

#include <common.h>

#include "entityhandle.h"

typedef void (*EntityRegisterCallback)();
typedef size_t (*EntityCreateCallback)();

template<typename T>
size_t CreateEntity()
{
	T* pT = new T();
	return pT->GetHandle();
}

class CEntityRegistration
{
public:
	const char*				m_pszEntityName;
	EntityRegisterCallback	m_pfnRegisterCallback;
	EntityCreateCallback	m_pfnCreateCallback;
};

#define REGISTER_ENTITY_CLASS(entity, base) \
DECLARE_CLASS(entity, base); \
public: \
static void RegisterCallback##entity() \
{ \
	entity* pEntity = new entity(); \
	CBaseEntity::Register(pEntity); \
	delete pEntity; \
} \
 \
virtual const char* GetClassName() { return #entity; } \

class CBaseEntity
{
	friend class CGame;

	REGISTER_ENTITY_CLASS(CBaseEntity, CBaseEntity);

public:
											CBaseEntity();
											~CBaseEntity();

public:
	virtual void							Precache() {};
	virtual void							Spawn() {};

	virtual float							GetBoundingRadius() const { return 0; };

	void									SetModel(const wchar_t* pszModel);
	size_t									GetModel() { return m_iModel; };

	virtual Vector							GetRenderOrigin() const { return GetOrigin(); };
	virtual EAngle							GetRenderAngles() const { return GetAngles(); };

	virtual inline Vector					GetOrigin() const { return m_vecOrigin; };
	void									SetOrigin(const Vector& vecOrigin) { m_vecOrigin = vecOrigin; };

	inline Vector							GetLastOrigin() const { return m_vecLastOrigin; };
	void									SetLastOrigin(const Vector& vecOrigin) { m_vecLastOrigin = vecOrigin; };

	inline Vector							GetVelocity() const { return m_vecVelocity; };
	void									SetVelocity(const Vector& vecVelocity) { m_vecVelocity = vecVelocity; };

	inline EAngle							GetAngles() const { return m_angAngles; };
	void									SetAngles(const EAngle& angAngles) { m_angAngles = angAngles; };

	inline Vector							GetGravity() const { return m_vecGravity; };
	void									SetGravity(Vector vecGravity) { m_vecGravity = vecGravity; };

	bool									GetSimulated() { return m_bSimulated; };
	void									SetSimulated(bool bSimulated) { m_bSimulated = bSimulated; };

	size_t									GetHandle() const { return m_iHandle; }

	virtual float							GetTotalHealth() { return m_flTotalHealth; }
	virtual void							SetTotalHealth(float flHealth) { m_flTotalHealth = m_flHealth = flHealth; }
	virtual float							GetHealth() { return m_flHealth; }
	virtual bool							IsAlive() { return m_flHealth > 0; }

	class CTeam*							GetTeam() const { return m_pTeam; };
	void									SetTeam(class CTeam* pTeam) { m_pTeam = pTeam; OnTeamChange(); };
	virtual void							OnTeamChange() {};

	virtual void							ClientUpdate(int iClient);

	virtual void							TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit = true);
	virtual bool							TakesDamage() { return m_bTakeDamage; };
	void									Killed(CBaseEntity* pKilledBy);
	virtual void							OnKilled(CBaseEntity* pKilledBy) {};

	virtual void							PreRender() {};
	virtual void							ModifyContext(class CRenderingContext* pContext) {};
	void									Render();
	virtual void							OnRender() {};
	virtual void							PostRender() {};

	void									Delete();
	virtual void							OnDeleted() {};
	virtual void							OnDeleted(class CBaseEntity* pEntity) {};
	bool									IsDeleted() { return m_bDeleted; }
	void									SetDeleted() { m_bDeleted = true; }

	virtual void							Think() {};

	virtual bool							ShouldSimulate() const { return false; };
	virtual bool							ShouldTouch(CBaseEntity* pOther) const { return false; };
	virtual bool							IsTouching(CBaseEntity* pOther, Vector& vecPoint) const { return false; };
	virtual void							Touching(CBaseEntity* pOther) {};

	void									EmitSound(const char* pszFilename);
	void									StopSound(const char* pszFilename);
	bool									IsSoundPlaying(const char* pszFilename);
	void									SetSoundVolume(const char* pszFilename, float flVolume);

	virtual bool							Collide(const Vector& v1, const Vector& v2, Vector& vecPoint);

	virtual int								GetCollisionGroup() { return m_iCollisionGroup; }
	virtual void							SetCollisionGroup(int iCollisionGroup) { m_iCollisionGroup = iCollisionGroup; }

	virtual size_t							GetSpawnSeed() { return m_iSpawnSeed; }
	virtual void							SetSpawnSeed(size_t iSpawnSeed) { m_iSpawnSeed = iSpawnSeed; }

	static CBaseEntity*						GetEntity(size_t iHandle);
	static size_t							GetEntityHandle(size_t i);
	static CBaseEntity*						GetEntityNumber(size_t i);
	static size_t							GetNumEntities();

	static void								PrecacheModel(const wchar_t* pszModel, bool bStatic = true);
	static void								PrecacheParticleSystem(const wchar_t* pszSystem);
	static void								PrecacheSound(const char* pszSound);

	static void								RegisterEntity(const char* pszEntityName, EntityCreateCallback pfnCreateCallback, EntityRegisterCallback pfnRegisterCallback);
	static void								Register(CBaseEntity* pEntity);
	static size_t							FindRegisteredEntity(const char* pszEntityName);

	template <class T>
	static T*								FindClosest(Vector vecPoint, CBaseEntity* pFurther = NULL);

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
	float									m_flTimeKilled;

	class CTeam*							m_pTeam;

	bool									m_bDeleted;

	std::vector<CEntityHandle<CBaseEntity> >	m_ahTouching;

	int										m_iCollisionGroup;

	size_t									m_iModel;

	size_t									m_iSpawnSeed;

private:
	static std::map<size_t, CBaseEntity*>	s_apEntityList;
	static size_t							s_iOverrideEntityListIndex;
	static size_t							s_iNextEntityListIndex;

	static std::vector<CEntityRegistration>	s_aEntityRegistration;
};

#define REGISTER_ENTITY(entity) \
class CRegister##entity \
{ \
public: \
	CRegister##entity() \
	{ \
		CBaseEntity::RegisterEntity(#entity, &CreateEntity<entity>, &entity::RegisterCallback##entity); \
	} \
} s_Register##entity = CRegister##entity(); \

template <class T>
T* CBaseEntity::FindClosest(Vector vecPoint, CBaseEntity* pFurther)
{
	T* pClosest = NULL;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);

		if (!pEntity)
			continue;

		T* pT = dynamic_cast<T*>(pEntity);

		if (!pT)
			continue;

		if (pT == pFurther)
			continue;

		if (pFurther && (pT->GetOrigin() - vecPoint).LengthSqr() <= (pFurther->GetOrigin() - vecPoint).LengthSqr())
			continue;

		if (!pClosest)
		{
			pClosest = pT;
			continue;
		}

		if ((pT->GetOrigin() - vecPoint).LengthSqr() < (pClosest->GetOrigin() - vecPoint).LengthSqr())
			pClosest = pT;
	}

	return pClosest;
}

#endif
