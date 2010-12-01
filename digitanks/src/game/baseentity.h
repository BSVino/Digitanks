#ifndef DT_BASEENTITY_H
#define DT_BASEENTITY_H

#include <EASTL/map.h>
#include <EASTL/vector.h>
#include <vector.h>
#include <assert.h>

#include <common.h>
#include <network/network.h>

#include "entityhandle.h"

namespace raytrace
{
	class CRaytracer;
};

typedef void (*EntityRegisterCallback)();
typedef size_t (*EntityCreateCallback)();

template<typename T>
size_t CreateEntity()
{
	T* pT = new T();
	return pT->GetHandle();
}

template <class C>
void ResizeVectorTmpl(char* pData, size_t iVectorSize)
{
	eastl::vector<C>* pVector = (eastl::vector<C>*)pData;
	pVector->resize(iVectorSize);
}

class CSaveData
{
public:
	typedef enum
	{
		DATA_COPYTYPE,
		DATA_COPYARRAY,
		DATA_COPYVECTOR,
		DATA_NETVAR,
		DATA_STRING16,
	} datatype_t;

	typedef void (*ResizeVector)(char* pData, size_t iVectorSize);

	datatype_t				m_eType;
	const char*				m_pszVariableName;
	size_t					m_iOffset;
	size_t					m_iSizeOfVariable;
	size_t					m_iSizeOfType;
	ResizeVector			m_pfnResizeVector;
};

class CEntityRegistration
{
public:
	const char*				m_pszEntityName;
	EntityRegisterCallback	m_pfnRegisterCallback;
	EntityCreateCallback	m_pfnCreateCallback;
	eastl::vector<CSaveData>	m_aSaveData;
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
virtual void RegisterNetworkVariables(); \
virtual void RegisterSaveData(); \
 \
virtual void Serialize(std::ostream& o) \
{ \
	if (strcmp(#entity, #base) != 0) \
		BaseClass::Serialize(o); \
 \
	CBaseEntity::Serialize(o, #entity, this); \
} \
 \
virtual bool Unserialize(std::istream& i) \
{ \
	if (strcmp(#entity, #base) != 0) \
	{ \
		if (!BaseClass::Unserialize(i)) \
			return false; \
	} \
 \
	return CBaseEntity::Unserialize(i, #entity, this); \
} \

#define NETVAR_TABLE_BEGIN(entity) \
void entity::RegisterNetworkVariables() \
{ \
	char* pszEntity = #entity; \
	BaseClass::RegisterNetworkVariables(); \

#define NETVAR_TABLE_BEGIN_NOBASE(entity) \
void entity::RegisterNetworkVariables() \
{ \
	char* pszEntity = #entity; \

#define NETVAR_DEFINE(type, name) \
	name.SetName(#name); \
	name.SetParent(this); \

#define NETVAR_DEFINE_CALLBACK(type, name, callback) \
	NETVAR_DEFINE(type, name); \
	name.SetCallback(callback); \

#define NETVAR_TABLE_END() \
	CheckTables(pszEntity); \
} \

#define SAVEDATA_TABLE_BEGIN(entity) \
void entity::RegisterSaveData() \
{ \
	size_t iRegisteredEntity = FindRegisteredEntity(#entity); \
	CEntityRegistration* pRegistration = GetRegisteredEntity(iRegisteredEntity); \
	pRegistration->m_aSaveData.clear(); \
	CGameServer* pGameServer = GameServer(); \
	CSaveData* pSaveData = NULL; \

#define SAVEDATA_DEFINE(copy, type, name) \
	pRegistration->m_aSaveData.push_back(CSaveData()); \
	pSaveData = &pRegistration->m_aSaveData[pRegistration->m_aSaveData.size()-1]; \
	pSaveData->m_eType = copy; \
	pSaveData->m_pszVariableName = #name; \
	if (copy == CSaveData::DATA_NETVAR) \
		pSaveData->m_iOffset = (((size_t)((void*)((CNetworkedVariableBase*)&name)))) - ((size_t)((void*)this)); \
	else \
		pSaveData->m_iOffset = (((size_t)((void*)&name))) - ((size_t)((void*)this)); \
	pSaveData->m_iSizeOfVariable = sizeof(name); \
	pSaveData->m_iSizeOfType = sizeof(type); \
	pSaveData->m_pfnResizeVector = &ResizeVectorTmpl<type>; \
	pGameServer->GenerateSaveCRC(pSaveData->m_eType); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iOffset); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfVariable); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfType); \

#define SAVEDATA_TABLE_END() \
} \

class CTeam;

class CBaseEntity
{
	friend class CGameServer;

	REGISTER_ENTITY_CLASS(CBaseEntity, CBaseEntity);

public:
											CBaseEntity();
	virtual									~CBaseEntity();

public:
	virtual void							Precache() {};
	virtual void							Spawn() {};

	virtual float							GetBoundingRadius() const { return 0; };

	void									SetModel(const eastl::string16& sModel);
	void									SetModel(size_t iModel);
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

	bool									GetSimulated() const { return m_bSimulated; };
	void									SetSimulated(bool bSimulated) { m_bSimulated = bSimulated; };

	size_t									GetHandle() const { return m_iHandle; }

	virtual float							GetTotalHealth() { return m_flTotalHealth; }
	virtual void							SetTotalHealth(float flHealth) { m_flTotalHealth = m_flHealth = flHealth; }
	virtual float							GetHealth() { return m_flHealth; }
	virtual bool							IsAlive() { return m_flHealth > 0; }

	class CTeam*							GetTeam() const;
	void									SetTeam(class CTeam* pTeam);
	virtual void							OnTeamChange() {};

	virtual void							ClientUpdate(int iClient);
	virtual void							ClientEnterGame();

	virtual void							TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit = true);
	virtual bool							TakesDamage() { return m_bTakeDamage; };
	void									Kill();
	void									Killed(CBaseEntity* pKilledBy);
	virtual void							OnKilled(CBaseEntity* pKilledBy) {};

	virtual bool							ShouldRender() const { return false; };
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

	virtual bool							ShouldSimulate() const { return GetSimulated(); };
	virtual bool							ShouldTouch(CBaseEntity* pOther) const { return false; };
	virtual bool							IsTouching(CBaseEntity* pOther, Vector& vecPoint) const { return false; };
	virtual void							Touching(CBaseEntity* pOther) {};

	void									EmitSound(const eastl::string16& sModel);
	void									StopSound(const eastl::string16& sModel);
	bool									IsSoundPlaying(const eastl::string16& sModel);
	void									SetSoundVolume(const eastl::string16& sModel, float flVolume);

	virtual float							Distance(Vector vecSpot);

	virtual bool							Collide(const Vector& v1, const Vector& v2, Vector& vecPoint);

	virtual int								GetCollisionGroup() { return m_iCollisionGroup; }
	virtual void							SetCollisionGroup(int iCollisionGroup) { m_iCollisionGroup = iCollisionGroup; }

	virtual bool							UsesRaytracedCollision() { return false; }

	virtual size_t							GetSpawnSeed() { return m_iSpawnSeed; }
	virtual void							SetSpawnSeed(size_t iSpawnSeed);

	void									RegisterNetworkVariable(class CNetworkedVariableBase* pVariable);
	void									DeregisterNetworkVariable(class CNetworkedVariableBase* pVariable);
	void									DeregisterNetworkVariables();
	CNetworkedVariableBase*					GetNetworkVariable(const char* pszName);

	virtual void							OnSerialize(std::ostream& o) {};
	virtual bool							OnUnserialize(std::istream& i) { return true; };

	void									CheckTables(char* pszEntity);

	static CBaseEntity*						GetEntity(size_t iHandle);
	static size_t							GetEntityHandle(size_t i);
	static CBaseEntity*						GetEntityNumber(size_t i);
	static size_t							GetNumEntities();

	static void								PrecacheModel(const eastl::string16& sModel, bool bStatic = true);
	static void								PrecacheParticleSystem(const eastl::string16& sSystem);
	static void								PrecacheSound(const eastl::string16& sSound);

	static void								RegisterEntity(const char* pszEntityName, EntityCreateCallback pfnCreateCallback, EntityRegisterCallback pfnRegisterCallback);
	static void								Register(CBaseEntity* pEntity);
	static size_t							FindRegisteredEntity(const char* pszEntityName);
	static CEntityRegistration*				GetRegisteredEntity(size_t iEntity);

	static void								SerializeEntity(std::ostream& o, CBaseEntity* pEntity);
	static bool								UnserializeEntity(std::istream& i);

	static void								Serialize(std::ostream& o, const char* pszClassName, void* pEntity);
	static bool								Unserialize(std::istream& i, const char* pszClassName, void* pEntity);

	template <class T>
	static T*								FindClosest(Vector vecPoint, CBaseEntity* pFurther = NULL);

protected:
	CNetworkedVector						m_vecOrigin;
	Vector									m_vecLastOrigin;
	CNetworkedEAngle						m_angAngles;
	CNetworkedVector						m_vecVelocity;
	CNetworkedVector						m_vecGravity;

	bool									m_bSimulated;

	size_t									m_iHandle;

	CNetworkedVariable<bool>				m_bTakeDamage;
	CNetworkedVariable<float>				m_flTotalHealth;
	CNetworkedVariable<float>				m_flHealth;
	float									m_flTimeKilled;

	CEntityHandle<CTeam>					m_hTeam;

	bool									m_bDeleted;

	eastl::vector<CEntityHandle<CBaseEntity> >	m_ahTouching;

	CNetworkedVariable<int>					m_iCollisionGroup;
	class raytrace::CRaytracer*				m_pTracer;

	CNetworkedVariable<size_t>				m_iModel;

	size_t									m_iSpawnSeed;

	eastl::map<eastl::string, class CNetworkedVariableBase*>	m_apNetworkVariables;

private:
	static eastl::map<size_t, CBaseEntity*>	s_apEntityList;
	static size_t							s_iOverrideEntityListIndex;
	static size_t							s_iNextEntityListIndex;

	static eastl::vector<CEntityRegistration>	s_aEntityRegistration;
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

		float flEntityDistance = pT->Distance(vecPoint);
		if (pFurther && (flEntityDistance <= pFurther->Distance(vecPoint)))
			continue;

		if (!pClosest)
		{
			pClosest = pT;
			continue;
		}

		if (flEntityDistance < pClosest->Distance(vecPoint))
			pClosest = pT;
	}

	return pClosest;
}

#include "gameserver.h"

#endif
