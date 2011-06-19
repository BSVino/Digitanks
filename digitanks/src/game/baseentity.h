#ifndef DT_BASEENTITY_H
#define DT_BASEENTITY_H

#include <EASTL/map.h>
#include <EASTL/vector.h>
#include <vector.h>

#include <common.h>
#include <network/network.h>

#include "entityhandle.h"

typedef enum
{
	DAMAGE_GENERIC = 1,
	DAMAGE_EXPLOSION,
	DAMAGE_COLLISION,
	DAMAGE_BURN,
	DAMAGE_LASER,
} damagetype_t;

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
		DATA_OMIT = 0,
		DATA_COPYTYPE,
		DATA_COPYARRAY,
		DATA_COPYVECTOR,
		DATA_NETVAR,
		DATA_STRING,
		DATA_STRING16,
		DATA_OUTPUT,
	} datatype_t;

	typedef void (*ResizeVector)(char* pData, size_t iVectorSize);

	datatype_t				m_eType;
	const char*				m_pszVariableName;
	size_t					m_iOffset;
	size_t					m_iSizeOfVariable;
	size_t					m_iSizeOfType;
	ResizeVector			m_pfnResizeVector;
};

typedef void (*EntityInputCallback)(const class CBaseEntity* pTarget, const eastl::vector<tstring>& sArgs);
class CEntityInput
{
public:
	eastl::string							m_sName;
	EntityInputCallback						m_pfnCallback;
};

#define DECLARE_ENTITY_INPUT(name) \
	virtual void name(const eastl::vector<tstring>& sArgs); \
	static void name##InputCallback(const class CBaseEntity* pTarget, const eastl::vector<tstring>& sArgs) \
	{ \
		((ThisClass*)pTarget)->name(sArgs); \
	}

class CEntityOutput
{
public:
	void									Call();
	void									AddTarget(const eastl::string& sTargetName, const eastl::string& sInput, const eastl::string& sArgs, bool bKill);
	void									Clear();

public:
	class CEntityOutputTarget
	{
	public:
		eastl::string						m_sTargetName;
		eastl::string						m_sInput;
		eastl::string						m_sArgs;
		bool								m_bKill;
	};

	eastl::vector<CEntityOutputTarget>		m_aTargets;
};

#define DECLARE_ENTITY_OUTPUT(name) \
	CEntityOutput			m_Output_##name; \

class CEntityRegistration
{
public:
	const char*				m_pszEntityName;
	const char*				m_pszParentClass;
	size_t					m_iParentRegistration;
	EntityRegisterCallback	m_pfnRegisterCallback;
	EntityCreateCallback	m_pfnCreateCallback;
	eastl::vector<CSaveData>	m_aSaveData;
	eastl::vector<CNetworkedVariableData>	m_aNetworkVariables;
	eastl::map<eastl::string, CEntityInput>		m_aInputs;
};

#define REGISTER_ENTITY_CLASS_NOBASE(entity) \
DECLARE_CLASS(entity, entity); \
public: \
static void RegisterCallback##entity() \
{ \
	entity* pEntity = new entity(); \
	pEntity->m_iRegistration = FindRegisteredEntity(#entity); \
	CBaseEntity::Register(pEntity); \
	delete pEntity; \
} \
static const char* Get##entity##ParentClass() { return NULL; } \
 \
virtual const char* GetClassName() { return #entity; } \
virtual void RegisterNetworkVariables(); \
virtual void RegisterSaveData(); \
virtual void RegisterInputData(); \
virtual size_t SizeOfThis() \
{ \
	/* -4 because the vtable is 4 bytes */ \
	return sizeof(entity) - 4; \
} \
 \
virtual void Serialize(std::ostream& o) \
{ \
	CBaseEntity::Serialize(o, #entity, this); \
} \
 \
virtual bool Unserialize(std::istream& i) \
{ \
	return CBaseEntity::Unserialize(i, #entity, this); \
} \

#define REGISTER_ENTITY_CLASS(entity, base) \
DECLARE_CLASS(entity, base); \
public: \
static void RegisterCallback##entity() \
{ \
	entity* pEntity = new entity(); \
	pEntity->m_iRegistration = FindRegisteredEntity(#entity); \
	CBaseEntity::Register(pEntity); \
	delete pEntity; \
} \
static const char* Get##entity##ParentClass() { return #base; } \
 \
virtual const char* GetClassName() { return #entity; } \
virtual void RegisterNetworkVariables(); \
virtual void RegisterSaveData(); \
virtual void RegisterInputData(); \
virtual size_t SizeOfThis() \
{ \
	return sizeof(entity) - sizeof(BaseClass); \
} \
 \
virtual void Serialize(std::ostream& o) \
{ \
	BaseClass::Serialize(o); \
 \
	CBaseEntity::Serialize(o, #entity, this); \
} \
 \
virtual bool Unserialize(std::istream& i) \
{ \
	if (!BaseClass::Unserialize(i)) \
		return false; \
 \
	return CBaseEntity::Unserialize(i, #entity, this); \
} \

#define NETVAR_TABLE_BEGIN(entity) \
void entity::RegisterNetworkVariables() \
{ \
	char* pszEntity = #entity; \
	CEntityRegistration* pRegistration = GetRegisteredEntity(GetRegistration()); \
	pRegistration->m_aNetworkVariables.clear(); \
	CGameServer* pGameServer = GameServer(); \
	CNetworkedVariableData* pVarData = NULL; \

#define NETVAR_DEFINE(type, name) \
	pRegistration->m_aNetworkVariables.push_back(CNetworkedVariableData()); \
	pVarData = &pRegistration->m_aNetworkVariables[pRegistration->m_aNetworkVariables.size()-1]; \
	TAssert(!!dynamic_cast<CNetworkedVariableBase*>(&name)); \
	pVarData->m_iOffset = (((size_t)((void*)((CNetworkedVariableBase*)&name)))) - ((size_t)((CBaseEntity*)this)); \
	pVarData->m_pszName = #name; \
	pVarData->m_pfnChanged = NULL; \
	pVarData->m_flUpdateInterval = 0; \

#define NETVAR_DEFINE_CALLBACK(type, name, callback) \
	NETVAR_DEFINE(type, name); \
	pVarData->m_pfnChanged = callback; \

#define NETVAR_DEFINE_INTERVAL(type, name, interval) \
	NETVAR_DEFINE(type, name); \
	pVarData->m_flUpdateInterval = interval; \

#define NETVAR_TABLE_END() \
	CheckTables(pszEntity); \
} \

#define SAVEDATA_TABLE_BEGIN(entity) \
void entity::RegisterSaveData() \
{ \
	CEntityRegistration* pRegistration = GetRegisteredEntity(GetRegistration()); \
	pRegistration->m_aSaveData.clear(); \
	CGameServer* pGameServer = GameServer(); \
	CSaveData* pSaveData = NULL; \

#define SAVEDATA_DEFINE(copy, type, name) \
	pSaveData = &pRegistration->m_aSaveData.push_back(); \
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

#define SAVEDATA_OMIT(name) \
	pSaveData = &pRegistration->m_aSaveData.push_back(); \
	pSaveData->m_eType = CSaveData::DATA_OMIT; \
	pSaveData->m_pszVariableName = #name; \
	pSaveData->m_iOffset = (((size_t)((void*)&name))) - ((size_t)((void*)this)); \
	pSaveData->m_iSizeOfVariable = sizeof(name); \
	pSaveData->m_iSizeOfType = 0; \
	pSaveData->m_pfnResizeVector = NULL; \
	pGameServer->GenerateSaveCRC(pSaveData->m_eType); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iOffset); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfVariable); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfType); \

#define SAVEDATA_DEFINE_OUTPUT(name) \
	pSaveData = &pRegistration->m_aSaveData.push_back(); \
	pSaveData->m_eType = CSaveData::DATA_OUTPUT; \
	pSaveData->m_pszVariableName = "m_Output_" #name; \
	pSaveData->m_iOffset = (((size_t)((void*)&m_Output_##name))) - ((size_t)((void*)this)); \
	pSaveData->m_iSizeOfVariable = sizeof(m_Output_##name); \
	pSaveData->m_iSizeOfType = sizeof(CEntityOutput); \
	pSaveData->m_pfnResizeVector = NULL; \
	pGameServer->GenerateSaveCRC(pSaveData->m_eType); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iOffset); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfVariable); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfType); \

#define SAVEDATA_TABLE_END() \
	CheckSaveDataSize(pRegistration); \
} \

#define INPUTS_TABLE_BEGIN(entity) \
void entity::RegisterInputData() \
{ \
	CEntityRegistration* pRegistration = GetRegisteredEntity(GetRegistration()); \
	pRegistration->m_aInputs.clear(); \

#define INPUT_DEFINE(name) \
	pRegistration->m_aInputs[#name].m_sName = #name; \
	pRegistration->m_aInputs[#name].m_pfnCallback = &name##InputCallback; \

#define INPUTS_TABLE_END() \
	CheckSaveDataSize(pRegistration); \
} \

class CTeam;

class CBaseEntity
{
	friend class CGameServer;

	REGISTER_ENTITY_CLASS_NOBASE(CBaseEntity);

public:
											CBaseEntity();
	virtual									~CBaseEntity();

public:
	virtual void							Precache() {};
	virtual void							Spawn();
	DECLARE_ENTITY_OUTPUT(OnSpawn);

	void									SetName(const eastl::string& sName) { m_sName = sName; };
	eastl::string							GetName() { return m_sName; };

	virtual float							GetBoundingRadius() const { return 0; };
	virtual float							GetRenderRadius() const { return GetBoundingRadius(); };

	void									SetModel(const tstring& sModel);
	void									SetModel(size_t iModel);
	size_t									GetModel() const { return m_iModel; };

	virtual Vector							GetRenderOrigin() const { return GetOrigin(); };
	virtual EAngle							GetRenderAngles() const { return GetAngles(); };

	virtual inline Vector					GetOrigin() const { return m_vecOrigin; };
	void									SetOrigin(const Vector& vecOrigin);
	virtual void							OnSetOrigin(const Vector& vecOrigin) {}

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

	virtual float							GetTotalHealth() const { return m_flTotalHealth; }
	virtual void							SetTotalHealth(float flHealth) { m_flTotalHealth = m_flHealth = flHealth; }
	virtual float							GetHealth() const { return m_flHealth; }
	virtual bool							IsAlive() { return m_flHealth > 0; }

	class CTeam*							GetTeam() const;
	void									SetTeam(class CTeam* pTeam);
	virtual void							OnTeamChange() {};

	virtual void							ClientUpdate(int iClient);
	virtual void							ClientEnterGame();

	virtual void							TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit = true);
	virtual bool							TakesDamage() const { return m_bTakeDamage; };
	DECLARE_ENTITY_OUTPUT(OnTakeDamage);
	void									Kill();
	void									Killed(CBaseEntity* pKilledBy);
	virtual void							OnKilled(CBaseEntity* pKilledBy) {};
	DECLARE_ENTITY_OUTPUT(OnKilled);

	void									SetActive(bool bActive);
	bool									IsActive() const { return m_bActive; };
	virtual void							OnActivated() {};
	virtual void							OnDeactivated() {};
	DECLARE_ENTITY_INPUT(Activate);
	DECLARE_ENTITY_INPUT(Deactivate);
	DECLARE_ENTITY_INPUT(ToggleActive);
	DECLARE_ENTITY_OUTPUT(OnActivated);
	DECLARE_ENTITY_OUTPUT(OnDeactivated);

	virtual bool							ShouldRender() const { return false; };
	virtual bool							ShouldRenderModel() const { return true; };
	virtual void							PreRender(bool bTransparent) const {};
	virtual void							ModifyContext(class CRenderingContext* pContext, bool bTransparent) const {};
	void									Render(bool bTransparent) const;
	virtual void							OnRender(class CRenderingContext* pContext, bool bTransparent) const {};
	virtual void							PostRender(bool bTransparent) const {};

	void									Delete();
	virtual void							OnDeleted() {};
	virtual void							OnDeleted(class CBaseEntity* pEntity) {};
	bool									IsDeleted() { return m_bDeleted; }
	void									SetDeleted() { m_bDeleted = true; }
	DECLARE_ENTITY_INPUT(Delete);

	virtual void							Think() {};

	virtual bool							ShouldSimulate() const { return GetSimulated(); };
	virtual bool							ShouldTouch(CBaseEntity* pOther) const { return false; };
	virtual bool							IsTouching(CBaseEntity* pOther, Vector& vecPoint) const { return false; };
	virtual void							Touching(CBaseEntity* pOther) {};

	void									CallInput(const eastl::string& sName, const tstring& sArgs);
	void									CallOutput(const eastl::string& sName);
	void									AddOutputTarget(const eastl::string& sName, const eastl::string& sTargetName, const eastl::string& sInput, const eastl::string& sArgs = "", bool bKill = false);
	void									RemoveOutputs(const eastl::string& sName);
	DECLARE_ENTITY_INPUT(RemoveOutput);

	void									EmitSound(const tstring& sSound, float flVolume = 1.0f, bool bLoop = false);
	void									StopSound(const tstring& sModel);
	bool									IsSoundPlaying(const tstring& sModel);
	void									SetSoundVolume(const tstring& sModel, float flVolume);

	virtual float							Distance(Vector vecSpot) const;

	virtual bool							Collide(const Vector& v1, const Vector& v2, Vector& vecPoint);

	virtual int								GetCollisionGroup() { return m_iCollisionGroup; }
	virtual void							SetCollisionGroup(int iCollisionGroup) { m_iCollisionGroup = iCollisionGroup; }

	virtual bool							UsesRaytracedCollision() { return false; }

	size_t									GetSpawnSeed() const { return m_iSpawnSeed; }
	void									SetSpawnSeed(size_t iSpawnSeed);

	float									GetSpawnTime() const { return m_flSpawnTime; }
	void									SetSpawnTime(float flSpawnTime) { m_flSpawnTime = flSpawnTime; };

	bool									HasIssuedClientSpawn() { return m_bClientSpawn; }
	void									IssueClientSpawn();
	virtual void							ClientSpawn();

	size_t									GetRegistration() { return m_iRegistration; }

	CSaveData*								GetSaveData(const char* pszName);
	CNetworkedVariableData*					GetNetworkVariable(const char* pszName);
	CEntityInput*							GetInput(const char* pszName);

	virtual void							OnSerialize(std::ostream& o) {};
	virtual bool							OnUnserialize(std::istream& i) { return true; };
	void									CheckSaveDataSize(CEntityRegistration* pRegistration);

	void									CheckTables(char* pszEntity);

	static CBaseEntity*						GetEntity(size_t iHandle);
	template <class T>
	static T*								GetEntityType(size_t iHandle)
	{
		CBaseEntity* pEntity = GetEntity(iHandle);
		if (!pEntity)
			return NULL;

		return dynamic_cast<T*>(pEntity);
	}

	static size_t							GetNumEntities();

	static void								PrecacheModel(const tstring& sModel, bool bStatic = true);
	static void								PrecacheParticleSystem(const tstring& sSystem);
	static void								PrecacheSound(const tstring& sSound);

public:
	static void								RegisterEntity(const char* pszEntityName, const char* pszParentClass, EntityCreateCallback pfnCreateCallback, EntityRegisterCallback pfnRegisterCallback);
	static void								Register(CBaseEntity* pEntity);
	static size_t							FindRegisteredEntity(const char* pszEntityName);
	static CEntityRegistration*				GetRegisteredEntity(size_t iEntity);

	static void								SerializeEntity(std::ostream& o, CBaseEntity* pEntity);
	static bool								UnserializeEntity(std::istream& i);

	static void								Serialize(std::ostream& o, const char* pszClassName, void* pEntity);
	static bool								Unserialize(std::istream& i, const char* pszClassName, void* pEntity);

	template <class T>
	static T*								FindClosest(Vector vecPoint, CBaseEntity* pFurther = NULL);

	static void								FindEntitiesByName(const eastl::string& sName, eastl::vector<CBaseEntity*>& apEntities);

protected:
	static eastl::vector<CEntityRegistration>& GetEntityRegistration();

protected:
	eastl::string							m_sName;

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

	CNetworkedVariable<bool>				m_bActive;

	CNetworkedHandle<CTeam>					m_hTeam;

	bool									m_bDeleted;

	eastl::vector<CEntityHandle<CBaseEntity> >	m_ahTouching;

	CNetworkedVariable<int>					m_iCollisionGroup;
	class raytrace::CRaytracer*				m_pTracer;

	CNetworkedVariable<size_t>				m_iModel;

	size_t									m_iSpawnSeed;
	CNetworkedVariable<float>				m_flSpawnTime;

	bool									m_bClientSpawn;

	size_t									m_iRegistration;

private:
	static eastl::vector<CBaseEntity*>		s_apEntityList;
	static size_t							s_iEntities;
	static size_t							s_iOverrideEntityListIndex;
	static size_t							s_iNextEntityListIndex;
};

#define REGISTER_ENTITY(entity) \
class CRegister##entity \
{ \
public: \
	CRegister##entity() \
	{ \
		CBaseEntity::RegisterEntity(#entity, entity::Get##entity##ParentClass(), &CreateEntity<entity>, &entity::RegisterCallback##entity); \
	} \
} s_Register##entity = CRegister##entity(); \

template <class T>
T* CBaseEntity::FindClosest(Vector vecPoint, CBaseEntity* pFurther)
{
	T* pClosest = NULL;

	float flFurtherDistance = 0;
	if (pFurther)
		flFurtherDistance = pFurther->Distance(vecPoint);

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		T* pT = dynamic_cast<T*>(pEntity);

		if (!pT)
			continue;

		if (pT == pFurther)
			continue;

		float flEntityDistance = pT->Distance(vecPoint);
		if (pFurther && (flEntityDistance <= flFurtherDistance))
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
