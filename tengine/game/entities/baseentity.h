#ifndef TINKER_BASEENTITY_H
#define TINKER_BASEENTITY_H

#include <tmap.h>
#include <tvector.h>
#include <vector.h>
#include <geometry.h>
#include <matrix.h>
#include <quaternion.h>
#include <common.h>

#include <tengine_config.h>
#include <tengine_config_data.h>

#include <network/network.h>
#include <game/entityhandle.h>
#include <textures/materialhandle.h>
#include <physics/physics.h>

typedef enum
{
	DAMAGE_GENERIC = 1,
	DAMAGE_EXPLOSION,
	DAMAGE_COLLISION,
	DAMAGE_BURN,
	DAMAGE_LASER,
	DAMAGE_MELEE,
} damagetype_t;

typedef void (*EntityRegisterCallback)();
typedef void (*EntityPrecacheCallback)();
typedef size_t (*EntityCreateCallback)();

template<typename T>
size_t NewEntity()
{
	T* pT = new T();
	return pT->GetHandle();
}

template <class C>
void ResizeVectorTmpl(char* pData, size_t iVectorSize)
{
	tvector<C>* pVector = (tvector<C>*)pData;
	pVector->resize(iVectorSize);
}

bool CanUnserializeString_bool(const tstring& sData);
bool CanUnserializeString_size_t(const tstring& sData);
bool CanUnserializeString_TVector(const tstring& sData);
bool CanUnserializeString_Vector2D(const tstring& sData);
bool CanUnserializeString_EAngle(const tstring& sData);
bool CanUnserializeString_Matrix4x4(const tstring& sData);
bool CanUnserializeString_AABB(const tstring& sData);

// The last three arguments are for error reporting if the unserialization goes awry.
bool UnserializeString_bool(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");
size_t UnserializeString_size_t(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");
const TVector UnserializeString_TVector(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");
const Vector2D UnserializeString_Vector2D(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");
const EAngle UnserializeString_EAngle(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");
const Matrix4x4 UnserializeString_Matrix4x4(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");
const AABB UnserializeString_AABB(const tstring& sData, const tstring& sName="", const tstring& sClass="", const tstring& sHandle="");

void UnserializeString_bool(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_int(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_size_t(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_float(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_double(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_tstring(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_TVector(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_Vector(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_Vector2D(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_EAngle(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_Matrix4x4(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_AABB(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_ModelID(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);
void UnserializeString_EntityHandle(const tstring& sData, class CSaveData* pData, class CBaseEntity* pEntity);

class CSaveData
{
public:
	CSaveData();

public:
	typedef enum
	{
		DATA_OMIT = 0,
		DATA_COPYTYPE,
		DATA_COPYARRAY,
		DATA_COPYVECTOR,
		DATA_NETVAR,
		DATA_STRING,
		DATA_OUTPUT,
	} datatype_t;

	typedef void (*UnserializeString)(const tstring& sData, CSaveData* pData, class CBaseEntity* pEntity);
	typedef void (*ResizeVector)(char* pData, size_t iVectorSize);

	datatype_t				m_eType;
	const char*				m_pszVariableName;
	const char*				m_pszType;
	const char*				m_pszHandle;
	size_t					m_iOffset;
	size_t					m_iSizeOfVariable;
	size_t					m_iSizeOfType;
	UnserializeString		m_pfnUnserializeString;
	ResizeVector			m_pfnResizeVector;
	bool					m_bOverride;
	bool					m_bShowInEditor;
	bool					m_bDefault;
	char					m_oDefault[96];
};

typedef void (*EntityInputCallback)(const class CBaseEntity* pTarget, const tvector<tstring>& sArgs);
class CEntityInput
{
public:
	tstring									m_sName;
	EntityInputCallback						m_pfnCallback;
};

#define DECLARE_ENTITY_INPUT(name) \
	virtual void name(const tvector<tstring>& sArgs); \
	static void name##InputCallback(const class CBaseEntity* pTarget, const tvector<tstring>& sArgs) \
	{ \
		((ThisClass*)pTarget)->name(sArgs); \
	}

class CEntityOutput
{
public:
	void									Call();
	void									AddTarget(const tstring& sTargetName, const tstring& sInput, const tstring& sArgs, bool bKill);
	void									Clear();

	const tstring							FormatArgs(tstring sArgs);

	void									SetEntity(class CBaseEntity* pEnt) { m_pEnt = pEnt; }
	void									SetOutputName(const tstring& sOutputName) { m_sOutputName = sOutputName; }

public:
	class CEntityOutputTarget
	{
	public:
		tstring								m_sTargetName;
		tstring								m_sInput;
		tstring								m_sArgs;
		bool								m_bKill;
	};

	tvector<CEntityOutputTarget>			m_aTargets;
	class CBaseEntity*						m_pEnt;
	tstring									m_sOutputName;
};

#define DECLARE_ENTITY_OUTPUT(name) \
	CEntityOutput			m_Output_##name; \

class CEntityRegistration
{
public:
	const char*					m_pszEntityClass;
	const char*					m_pszParentClass;
	EntityRegisterCallback		m_pfnRegisterCallback;
	EntityPrecacheCallback		m_pfnPrecacheCallback;
	EntityCreateCallback		m_pfnCreateCallback;
	tvector<CSaveData>			m_aSaveData;
	tvector<CNetworkedVariableData>	m_aNetworkVariables;
	tmap<tstring, CEntityInput>	m_aInputs;
	tvector<tstring>			m_asPrecaches;
	tvector<CMaterialHandle>	m_ahMaterialPrecaches;
	bool						m_bCreatableInEditor;
};

#define REGISTER_ENTITY_CLASS_NOBASE(entity) \
DECLARE_CLASS(entity, entity); \
public: \
static void RegisterCallback##entity() \
{ \
	entity* pEntity = new entity(); \
	pEntity->m_sClassName = #entity; \
	CBaseEntity::Register(pEntity); \
	delete pEntity; \
} \
static void PrecacheCallback##entity() \
{ \
	entity* pEntity = new entity(); \
	pEntity->m_sClassName = #entity; \
	CBaseEntity::PrecacheCallback(pEntity); \
	delete pEntity; \
} \
static const char* Get##entity##ParentClass() { return NULL; } \
 \
virtual const char* GetClassName() const { return #entity; } \
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

// Third parameter: how many interfaces does the class have?
#define REGISTER_ENTITY_CLASS_INTERFACES(entity, base, iface) \
DECLARE_CLASS(entity, base); \
public: \
static void RegisterCallback##entity() \
{ \
	entity* pEntity = new entity(); \
	pEntity->m_sClassName = #entity; \
	CBaseEntity::Register(pEntity); \
	delete pEntity; \
} \
static void PrecacheCallback##entity() \
{ \
	entity* pEntity = new entity(); \
	pEntity->m_sClassName = #entity; \
	CBaseEntity::PrecacheCallback(pEntity); \
	delete pEntity; \
} \
static const char* Get##entity##ParentClass() { return #base; } \
 \
virtual const char* GetClassName() const { return #entity; } \
virtual void RegisterNetworkVariables(); \
virtual void RegisterSaveData(); \
virtual void RegisterInputData(); \
virtual size_t SizeOfThis() \
{ \
	return sizeof(entity) - sizeof(BaseClass) - iface*4; \
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

#define REGISTER_ENTITY_CLASS(entity, base) \
	REGISTER_ENTITY_CLASS_INTERFACES(entity, base, 0)

#define NETVAR_TABLE_BEGIN(entity) \
void entity::RegisterNetworkVariables() \
{ \
	const char* pszEntity = #entity; \
	CEntityRegistration* pRegistration = GetRegisteredEntity(GetClassName()); \
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

#define SAVEDATA_TABLE_BEGIN_COMMON(entity, editor) \
void entity::RegisterSaveData() \
{ \
	CEntityRegistration* pRegistration = GetRegisteredEntity(GetClassName()); \
	pRegistration->m_aSaveData.clear(); \
	pRegistration->m_bCreatableInEditor = editor; \
	CGameServer* pGameServer = GameServer(); \
	CSaveData* pSaveData = NULL; \

#define SAVEDATA_TABLE_BEGIN(entity) \
	SAVEDATA_TABLE_BEGIN_COMMON(entity, false) \

#define SAVEDATA_TABLE_BEGIN_EDITOR(entity) \
	SAVEDATA_TABLE_BEGIN_COMMON(entity, true) \

#define SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData = &pRegistration->m_aSaveData.push_back(); \
	pSaveData->m_eType = copy; \
	pSaveData->m_pszType = #type; \
	pSaveData->m_pszVariableName = #name; \
	if (copy == CSaveData::DATA_NETVAR) \
		pSaveData->m_iOffset = (((size_t)((void*)((CNetworkedVariableBase*)&name)))) - ((size_t)((void*)this)); \
	else \
		pSaveData->m_iOffset = (((size_t)((void*)&name))) - ((size_t)((void*)this)); \
	pSaveData->m_iSizeOfVariable = sizeof(name); \
	pSaveData->m_iSizeOfType = sizeof(type); \
	pSaveData->m_pfnResizeVector = &ResizeVectorTmpl<type>; \
	pSaveData->m_bOverride = false; \
	pSaveData->m_bDefault = false; \
	pSaveData->m_bShowInEditor = false; \
	pGameServer->GenerateSaveCRC(pSaveData->m_eType); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iOffset); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfVariable); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfType); \

#define SAVEDATA_DEFINE_HANDLE(copy, type, name, handle) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = handle; \
	pSaveData->m_pfnUnserializeString = &UnserializeString_##type; \
	memset(pSaveData->m_oDefault, 0, sizeof(type)); \

#define SAVEDATA_DEFINE_HANDLE_DEFAULT(copy, type, name, handle, def) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = handle; \
	pSaveData->m_pfnUnserializeString = &UnserializeString_##type; \
	{ \
		type iDefault = def; \
		TAssert(sizeof(pSaveData->m_oDefault) >= sizeof(def)); \
		memcpy(pSaveData->m_oDefault, &iDefault, sizeof(def)); \
		pSaveData->m_bDefault = true; \
	} \

#define SAVEDATA_DEFINE_DEFAULT(copy, type, name, def) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = nullptr; \
	pSaveData->m_pfnUnserializeString = &UnserializeString_##type; \
	{ \
		type iDefault = def; \
		TAssert(sizeof(pSaveData->m_oDefault) >= sizeof(def)); \
		memcpy(pSaveData->m_oDefault, &iDefault, sizeof(def)); \
		pSaveData->m_bDefault = true; \
	} \

#define SAVEDATA_DEFINE(copy, type, name) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = nullptr; \
	pSaveData->m_pfnUnserializeString = nullptr; \

#define SAVEDATA_DEFINE_HANDLE_FUNCTION(copy, type, name, handle, function) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = handle; \
	pSaveData->m_pfnUnserializeString = &function; \
	memset(pSaveData->m_oDefault, 0, sizeof(type)); \

#define SAVEDATA_DEFINE_HANDLE_DEFAULT_FUNCTION(copy, type, name, handle, def, function) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = handle; \
	pSaveData->m_pfnUnserializeString = &function; \
	{ \
		type iDefault = def; \
		memcpy(pSaveData->m_oDefault, &iDefault, sizeof(def)); \
		pSaveData->m_bDefault = true; \
	} \

#define SAVEDATA_DEFINE_HANDLE_ENTITY(copy, type, name, handle) \
	SAVEDATA_DEFINE_COMMON(copy, type, name) \
	pSaveData->m_pszHandle = handle; \
	TAssert(strncmp(#type, "CEntityHandle", 13) == 0); \
	pSaveData->m_pfnUnserializeString = &UnserializeString_EntityHandle; \
	memset(pSaveData->m_oDefault, 0, sizeof(type)); \

#define SAVEDATA_OMIT(name) \
	pSaveData = &pRegistration->m_aSaveData.push_back(); \
	pSaveData->m_eType = CSaveData::DATA_OMIT; \
	pSaveData->m_pszVariableName = #name; \
	pSaveData->m_pszHandle = ""; \
	pSaveData->m_iOffset = (((size_t)((void*)&name))) - ((size_t)((void*)this)); \
	pSaveData->m_iSizeOfVariable = sizeof(name); \
	pSaveData->m_iSizeOfType = 0; \
	pSaveData->m_pfnResizeVector = NULL; \
	pSaveData->m_bOverride = false; \
	pSaveData->m_bDefault = false; \
	pSaveData->m_bShowInEditor = false; \
	pGameServer->GenerateSaveCRC(pSaveData->m_eType); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iOffset); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfVariable); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfType); \

#define SAVEDATA_DEFINE_OUTPUT(name) \
	pSaveData = &pRegistration->m_aSaveData.push_back(); \
	pSaveData->m_eType = CSaveData::DATA_OUTPUT; \
	pSaveData->m_pszVariableName = "m_Output_" #name; \
	pSaveData->m_pszHandle = #name; \
	pSaveData->m_iOffset = (((size_t)((void*)&m_Output_##name))) - ((size_t)((void*)this)); \
	pSaveData->m_iSizeOfVariable = sizeof(m_Output_##name); \
	pSaveData->m_iSizeOfType = sizeof(CEntityOutput); \
	pSaveData->m_pfnResizeVector = NULL; \
	pSaveData->m_bOverride = false; \
	pSaveData->m_bDefault = false; \
	pSaveData->m_bShowInEditor = false; \
	pGameServer->GenerateSaveCRC(pSaveData->m_eType); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iOffset); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfVariable); \
	pGameServer->GenerateSaveCRC(pSaveData->m_iSizeOfType); \

#define SAVEDATA_OVERRIDE_DEFAULT(eDataType, type, name, handle, def) \
	type name##Test = def; name##Test = name##Test; /*Test to make sure "def" is of type "type" */ \
	SaveData_OverrideDefault(pRegistration, eDataType, #name, handle, (type)def) \

#define SAVEDATA_EDITOR_VARIABLE(handle) \
	SaveData_EditorVariable(pRegistration, handle) \

#define SAVEDATA_TABLE_END() \
	CheckSaveDataSize(pRegistration); \
} \

#define INPUTS_TABLE_BEGIN(entity) \
void entity::RegisterInputData() \
{ \
	CEntityRegistration* pRegistration = GetRegisteredEntity(GetClassName()); \
	pRegistration->m_aInputs.clear(); \

#define INPUT_DEFINE(name) \
	pRegistration->m_aInputs[#name].m_sName = #name; \
	pRegistration->m_aInputs[#name].m_pfnCallback = &name##InputCallback; \

#define INPUTS_TABLE_END() \
} \

class CTeam;

class CBaseEntity : public IPhysicsEntity
{
	friend class CGameServer;

	REGISTER_ENTITY_CLASS_NOBASE(CBaseEntity);

public:
											CBaseEntity();
	virtual									~CBaseEntity();

public:
	virtual void							Precache() {};
	virtual void							SetSaveDataDefaults();
	virtual void							Spawn();
	DECLARE_ENTITY_OUTPUT(OnSpawn);

	virtual void                            PostLoad();

	void									SetName(const tstring& sName) { m_sName = sName; };
	const tstring&                          GetName() const { return m_sName; };

	CGameEntityData&						GameData() { return m_oGameData; }
	const CGameEntityData&					GameData() const { return m_oGameData; }

	void									SetMass(float flMass) { m_flMass = flMass; };
	float									GetMass() const { return m_flMass; };

	virtual const AABB                      GetVisBoundingBox() const { return m_aabbVisBoundingBox; }
	virtual const AABB                      GetPhysBoundingBox() const { return m_aabbPhysBoundingBox; }
	virtual const TVector					GetLocalCenter() const;
	virtual const TVector					GetGlobalCenter() const;
	virtual const TVector                   GetRenderCenter() const;
	virtual const TFloat					GetBoundingRadius() const;

	virtual const TFloat					GetRenderRadius() const { return GetBoundingRadius(); };

	void									SetModel(const tstring& sModel);
	void									SetModel(size_t iModel);
	size_t									GetModelID() const { return m_iModel; };
	class CModel*							GetModel() const;
	virtual void							OnSetModel() {};

	Matrix4x4								BaseGetRenderTransform() const { return m_oGameData.GetRenderTransform(); };
	Vector									BaseGetRenderOrigin() const { return m_oGameData.GetRenderOrigin(); };

	void									SetMaterialModel(const CMaterialHandle& hMaterial);
	const CMaterialHandle&					GetMaterialModel() const { return m_hMaterialModel; };

	virtual const Matrix4x4					GetRenderTransform() const { return Matrix4x4(GetGlobalTransform()); };
	const Vector							GetRenderOrigin() const { return GetRenderTransform().GetTranslation(); };
	const EAngle							GetRenderAngles() const { return GetRenderTransform().GetAngles(); };

	void									SetMoveParent(CBaseEntity* pParent);
	CBaseEntity*							GetMoveParent() const { return m_hMoveParent; };
	bool									HasMoveParent() const { return m_hMoveParent != NULL; };
	void									InvalidateGlobalTransforms();
	void                                    InvalidateChildrenTransforms();
	const TMatrix							GetParentGlobalTransform() const;

	const TMatrix							GetGlobalTransform() const;
	void									SetGlobalTransform(const TMatrix& m);

	virtual const Matrix4x4                 GetPhysicsTransform() const { return GetGlobalTransform(); }
	virtual void                            SetPhysicsTransform(const Matrix4x4& m) { SetGlobalTransform(TMatrix(m)); }

	const TMatrix							GetGlobalToLocalTransform() const;

	virtual const TVector					GetGlobalOrigin() const;
	virtual const EAngle					GetGlobalAngles() const;

	void									SetGlobalOrigin(const TVector& vecOrigin);
	void									SetGlobalAngles(const EAngle& angAngles);

	virtual const TVector					GetGlobalVelocity();
	virtual const TVector					GetGlobalVelocity() const;
	void									SetGlobalVelocity(const TVector& vecVelocity);

	virtual inline const TVector			GetGlobalGravity() const { return m_vecGlobalGravity; };
	void									SetGlobalGravity(const TVector& vecGravity);

	const TMatrix&							GetLocalTransform() const { return m_mLocalTransform; }
	void									SetLocalTransform(const TMatrix& m);
	virtual void							OnSetLocalTransform(TMatrix& m) {}
	virtual void                            PostSetLocalTransform(const TMatrix& m) {}

	const Quaternion&						GetLocalRotation() const { return m_qLocalRotation; }
	void									SetLocalRotation(const Quaternion& q);

	virtual inline const TVector			GetLocalOrigin() const { return m_vecLocalOrigin; };
	void									SetLocalOrigin(const TVector& vecOrigin);

	inline const TVector					GetLastLocalOrigin() const { return m_vecLastLocalOrigin; };
	void									SetLastLocalOrigin(const TVector& vecOrigin) { m_vecLastLocalOrigin = vecOrigin; };

	inline const TVector					GetLastGlobalOrigin() const;

	inline const TVector					GetLocalVelocity() const { return m_vecLocalVelocity; };
	void									SetLocalVelocity(const TVector& vecVelocity);

	inline const EAngle						GetLocalAngles() const { return m_angLocalAngles; };
	void									SetLocalAngles(const EAngle& angLocalAngles);

	DECLARE_ENTITY_INPUT(SetLocalOrigin);
	DECLARE_ENTITY_INPUT(SetLocalAngles);

	virtual void					SetViewAngles(const EAngle& angView) { m_angView = angView; }
	virtual const EAngle			GetViewAngles() const { return m_angView; }

	DECLARE_ENTITY_INPUT(SetViewAngles);

	inline const Vector						GetScale() const { return m_vecScale.Get(); }

	virtual const Vector					GetUpVector() const { return Vector(0, 0, 1); };

	virtual bool							TransformsChildView() const { return false; };

	bool									IsVisible() const { return m_bVisible; }
	void									SetVisible(bool bVisible);
	virtual void                            OnSetVisible(bool bVisible);

	DECLARE_ENTITY_INPUT(SetVisible);

	void                                    SetInPhysics(bool b) { m_bInPhysics = b; }
	bool									IsInPhysics() const { return m_bInPhysics; };
	void									AddToPhysics(enum collision_type_e eCollisionType);
	void									RemoveFromPhysics();
	virtual collision_group_t               GetCollisionGroup() const { return CG_DEFAULT; }

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
	virtual void							OnTakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit = true) {};
	virtual bool							TakesDamage() const { return m_bTakeDamage; };
	DECLARE_ENTITY_OUTPUT(OnTakeDamage);
	void									Kill();
	void									Killed(CBaseEntity* pKilledBy);
	virtual void							OnKilled(CBaseEntity* pKilledBy);
	DECLARE_ENTITY_OUTPUT(OnKilled);

	void									SetActive(bool bActive);
	bool									IsActive() const { return m_bActive; };
	virtual void							OnActivated() {};
	virtual void							OnDeactivated() {};
	DECLARE_ENTITY_INPUT(Activate);
	DECLARE_ENTITY_INPUT(Deactivate);
	DECLARE_ENTITY_INPUT(ToggleActive);
	DECLARE_ENTITY_INPUT(SetActive);
	DECLARE_ENTITY_OUTPUT(OnActivated);
	DECLARE_ENTITY_OUTPUT(OnDeactivated);

	void                                    Use(CBaseEntity* pUser);
	virtual void                            OnUse(CBaseEntity* pUser);
	DECLARE_ENTITY_INPUT(Use);
	DECLARE_ENTITY_OUTPUT(OnUsed);
	void                                    SetUsable(bool bUsable);
	virtual bool                            IsUsable(const CBaseEntity* pUser) const;
	const TVector                           GetUsePosition() const { return GetGlobalOrigin(); }

	virtual bool							ShouldRender() const;
	virtual bool                            ShouldRenderTransparent() const;
	virtual bool							ShouldRenderModel() const { return true; };
	virtual void							PreRender() const;
	virtual void							ModifyContext(class CRenderingContext* pContext) const {};
	virtual bool							ModifyShader(class CRenderingContext* pContext) const { return true; };
	void									Render() const;
	void                                    RenderTransparent() const;
	virtual void							OnRender(class CGameRenderingContext* pContext) const {};
	virtual void							PostRender() const {};
	virtual bool                            ShouldBatch() const { return true; }

	void									Delete();
	virtual void							OnDeleted() {};
	virtual void							OnDeleted(const CBaseEntity* pEntity);
	bool									IsDeleted() { return m_bDeleted; }
	void									SetDeleted() { m_bDeleted = true; }
	DECLARE_ENTITY_INPUT(Delete);

	void									BaseThink() { m_oGameData.Think(); };
	virtual void							Think() {};

	virtual void							Touching(size_t iOtherHandle) {};
	virtual void							BeginTouchingList() {};
	virtual void							EndTouchingList() {};

	void									CallInput(const tstring& sName, const tstring& sArgs);
	void									CallOutput(const tstring& sName);
	void									AddOutputTarget(const tstring& sName, const tstring& sTargetName, const tstring& sInput, const tstring& sArgs = "", bool bKill = false);
	void									RemoveOutputs(const tstring& sName);
	virtual const tstring					GetOutputValue(const tstring& sOutput, size_t iValue) { return ""; }
	DECLARE_ENTITY_INPUT(RemoveOutput);

	void									EmitSound(const tstring& sSound, float flVolume = 1.0f, bool bLoop = false);
	void									StopSound(const tstring& sModel);
	bool									IsSoundPlaying(const tstring& sModel);
	void									SetSoundVolume(const tstring& sModel, float flVolume);

	virtual const TFloat					Distance(const TVector& vecSpot) const;

	// Physics callback - Should this object collide with pOther at the specified point?
	// At this point the expensive collision checks have passed and the two objects will
	// definitely collide if true is returned here. If two objects should never collide,
	// use collision groups instead to avoid the expensive collision checks.
	virtual bool							ShouldCollideWith(size_t iOtherHandle, const TVector& vecPoint) const;
	virtual bool							ShouldCollideWith(CBaseEntity* pOther, const TVector& vecPoint) const { return true; }
	virtual bool							ShouldCollideWithExtra(size_t, const TVector& vecPoint) const { return true; }

	size_t									GetSpawnSeed() const { return m_iSpawnSeed; }
	void									SetSpawnSeed(size_t iSpawnSeed);

	double									GetSpawnTime() const { return m_flSpawnTime; }
	void									SetSpawnTime(double flSpawnTime) { m_flSpawnTime = flSpawnTime; };

	bool									HasIssuedClientSpawn() { return m_bClientSpawn; }
	void									IssueClientSpawn();
	virtual void							ClientSpawn();

	void SaveData_OverrideDefault(CEntityRegistration*, CSaveData::datatype_t, const char* pszName, const char* pszHandle, bool bDefault);
	void SaveData_OverrideDefault(CEntityRegistration*, CSaveData::datatype_t, const char* pszName, const char* pszHandle, const char* pszDefault);
	void SaveData_OverrideDefault(CEntityRegistration*, CSaveData::datatype_t, const char* pszName, const char* pszHandle, int iDefault);
	void SaveData_OverrideDefault(CEntityRegistration*, CSaveData::datatype_t, const char* pszName, const char* pszHandle, float flDefault);
	void SaveData_OverrideDefault(CEntityRegistration*, CSaveData::datatype_t, const char* pszName, const char* pszHandle, const AABB& aabbDefault);
	void SaveData_EditorVariable(CEntityRegistration*, const char* pszHandle);

	CSaveData*								FindSaveData(const char* pszName, bool bThisClassOnly=false);
	CSaveData*								FindSaveDataByHandle(const char* pszHandle, bool bThisClassOnly=false);
	CNetworkedVariableData*					FindNetworkVariable(const char* pszName, bool bThisClassOnly=false);
	CEntityInput*							FindInput(const char* pszName, bool bThisClassOnly=false);

	virtual void							OnSerialize(std::ostream& o) {};
	virtual bool							OnUnserialize(std::istream& i) { return true; };
	void									CheckSaveDataSize(CEntityRegistration* pRegistration);

	void									CheckTables(const char* pszEntity);

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

	void									PrecacheModel(const tstring& sModel);
	void									PrecacheParticleSystem(const tstring& sSystem);
	void									PrecacheSound(const tstring& sSound);
	void									PrecacheMaterial(const tstring& sMaterial);

public:
	static void								RegisterEntity(const char* pszClassName, const char* pszParentClass, EntityRegisterCallback pfnRegisterCallback, EntityPrecacheCallback pfnPrecacheCallback, EntityCreateCallback pfnCreateCallback);
	static void								Register(CBaseEntity* pEntity);
	static CEntityRegistration*				GetRegisteredEntity(tstring sClassName);
	static void								PrecacheCallback(CBaseEntity* pEntity);

	static size_t							GetNumEntitiesRegistered();
	static CEntityRegistration*				GetEntityRegistration(size_t iEntity);

	static CSaveData*						FindSaveData(const char* pszClassName, const char* pszName, bool bThisClassOnly=false);
	static CSaveData*						FindSaveDataByHandle(const char* pszClassName, const char* pszHandle, bool bThisClassOnly=false);
	static CSaveData*						FindSaveDataValuesByHandle(const char* pszClassName, const char* pszHandle, CSaveData* pSaveData);
	static CSaveData*						FindOutput(const char* pszClassName, const tstring& sOutput, bool bThisClassOnly=false);

	static void								SerializeEntity(std::ostream& o, CBaseEntity* pEntity);
	static bool								UnserializeEntity(std::istream& i);

	static void								Serialize(std::ostream& o, const char* pszClassName, void* pEntity);
	static bool								Unserialize(std::istream& i, const char* pszClassName, void* pEntity);

	template <class T>
	static T*								FindClosest(const TVector& vecPoint, CBaseEntity* pFurther = NULL);

	static CBaseEntity*						GetEntityByName(const tstring& sName);
	static void								FindEntitiesByName(const tstring& sName, tvector<CBaseEntity*>& apEntities);

protected:
	static tmap<tstring, CEntityRegistration>& GetEntityRegistration();

protected:
	tstring									m_sName;
	tstring									m_sClassName;

	float									m_flMass;

	CNetworkedHandle<CBaseEntity>			m_hMoveParent;
	CNetworkedSTLVector<CEntityHandle<CBaseEntity>>	m_ahMoveChildren;

	AABB									m_aabbVisBoundingBox;
	AABB									m_aabbPhysBoundingBox;

	mutable bool							m_bGlobalTransformsDirty;
	mutable TMatrix							m_mGlobalTransform;
	CNetworkedVector						m_vecGlobalGravity;

	TMatrix									m_mLocalTransform;
	Quaternion								m_qLocalRotation;
	CNetworkedVector						m_vecLocalOrigin;
	TVector									m_vecLastLocalOrigin;
	CNetworkedEAngle						m_angLocalAngles;
	CNetworkedVector						m_vecLocalVelocity;
	CNetworkedVariable<Vector>              m_vecScale;
	EAngle                                  m_angView;

	size_t									m_iHandle;

	CNetworkedVariable<bool>				m_bTakeDamage;
	CNetworkedVariable<float>				m_flTotalHealth;
	CNetworkedVariable<float>				m_flHealth;
	double									m_flTimeKilled;
	double									m_flLastTakeDamage;

	CNetworkedVariable<bool>				m_bActive;

	CNetworkedHandle<CTeam>					m_hTeam;

	bool									m_bVisible;
	bool									m_bInPhysics;
	bool									m_bDeleted;
	bool									m_bClientSpawn;

	CNetworkedVariable<int>					m_iCollisionGroup;

	CNetworkedVariable<size_t>				m_iModel;
	CMaterialHandle							m_hMaterialModel;
	bool									m_bRenderInverted;
	bool									m_bDisableBackCulling;

	size_t									m_iSpawnSeed;
	CNetworkedVariable<double>				m_flSpawnTime;

	bool                                    m_bUsable;

	CGameEntityData							m_oGameData;

private:
	static tvector<CBaseEntity*>			s_apEntityList;
	static size_t							s_iEntities;
	static size_t							s_iOverrideEntityListIndex;
	static size_t							s_iNextEntityListIndex;

protected:
	static tstring							s_sCurrentInput;
};

#define REGISTER_ENTITY(entity) \
class CRegister##entity \
{ \
public: \
	CRegister##entity() \
	{ \
		CBaseEntity::RegisterEntity(#entity, entity::Get##entity##ParentClass(), &entity::RegisterCallback##entity, &entity::PrecacheCallback##entity, &NewEntity<entity>); \
	} \
} g_Register##entity = CRegister##entity(); \

#include <game/gameserver.h>
#include <game/template_functions.h>

template <class T>
T* CBaseEntity::FindClosest(const TVector& vecPoint, CBaseEntity* pFurther)
{
	T* pClosest = NULL;

	TFloat flFurtherDistance = 0;
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

		TFloat flEntityDistance = pT->Distance(vecPoint);
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

#endif
