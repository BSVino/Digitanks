#include "baseentity.h"

#include <strutils.h>
#include <mtrand.h>
#include <tvector.h>

#include <models/models.h>
#include <toys/toy.h>
#include <renderer/renderingcontext.h>
#include <renderer/particles.h>
#include <sound/sound.h>
#include <tinker/application.h>
#include <tinker/profiler.h>
#include <network/commands.h>
#include <textures/materiallibrary.h>
#include <tinker/cvar.h>
#include <physics/physics.h>
#include <game/entities/character.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>

#include "game.h"

bool g_bAutoImporting = false;
#include "beam.h"
#include "counter.h"
#include "kinematic.h"
#include "logicgate.h"
#include "mathgate.h"
#include "playerstart.h"
#include "static.h"
#include "target.h"
#include "trigger.h"
#include "prop.h"
// Use this to force import of required entities.
class CAutoImport
{
public:
	CAutoImport()
	{
		g_bAutoImporting = true;
		{
			CBeam b;
			CCounter c;
			CKinematic k;
			CLogicGate l;
			CMathGate m;
			CPlayerStart p;
			CStatic s;
			CTarget t2;
			CTrigger t;
			CProp p2;
		}
		g_bAutoImporting = false;
	}
} g_AutoImport = CAutoImport();

tvector<CBaseEntity*> CBaseEntity::s_apEntityList;
size_t CBaseEntity::s_iEntities = 0;
size_t CBaseEntity::s_iOverrideEntityListIndex = ~0;
size_t CBaseEntity::s_iNextEntityListIndex = 0;
tstring CBaseEntity::s_sCurrentInput;

REGISTER_ENTITY(CBaseEntity);

NETVAR_TABLE_BEGIN(CBaseEntity);
	NETVAR_DEFINE(CEntityHandle<CBaseEntity>, m_hMoveParent);
	NETVAR_DEFINE(CEntityHandle<CBaseEntity>, m_ahMoveChildren);
	NETVAR_DEFINE(TVector, m_vecGlobalGravity);
	NETVAR_DEFINE_INTERVAL(TVector, m_vecLocalOrigin, 0.15f);
	NETVAR_DEFINE_INTERVAL(EAngle, m_angLocalAngles, 0.15f);
	NETVAR_DEFINE_INTERVAL(TVector, m_vecLocalVelocity, 0.15f);
	NETVAR_DEFINE_INTERVAL(Vector, m_vecScale, 0.15f);
	NETVAR_DEFINE(bool, m_bTakeDamage);
	NETVAR_DEFINE(float, m_flTotalHealth);
	NETVAR_DEFINE(float, m_flHealth);
	NETVAR_DEFINE(bool, m_bActive);
	NETVAR_DEFINE(CEntityHandle<CBaseEntity>, m_hTeam);
	NETVAR_DEFINE(int, m_iCollisionGroup);
	NETVAR_DEFINE(size_t, m_iModel);
	NETVAR_DEFINE(double, m_flSpawnTime);
NETVAR_TABLE_END();

void UnserializeString_LocalOrigin(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity);
void UnserializeString_LocalAngles(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity);
void UnserializeString_MoveParent(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity);

SAVEDATA_TABLE_BEGIN(CBaseEntity);
	SAVEDATA_DEFINE_OUTPUT(OnSpawn);
	SAVEDATA_DEFINE_OUTPUT(OnTakeDamage);
	SAVEDATA_DEFINE_OUTPUT(OnKilled);
	SAVEDATA_DEFINE_OUTPUT(OnActivated);
	SAVEDATA_DEFINE_OUTPUT(OnDeactivated);
	SAVEDATA_DEFINE_OUTPUT(OnUsed);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_STRING, tstring, m_sName, "Name");
	SAVEDATA_EDITOR_VARIABLE("Name");
	SAVEDATA_DEFINE(CSaveData::DATA_STRING, tstring, m_sClassName);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, float, m_flMass, "Mass", 50);
	SAVEDATA_DEFINE_HANDLE_FUNCTION(CSaveData::DATA_NETVAR, CEntityHandle<CBaseEntity>, m_hMoveParent, "MoveParent", UnserializeString_MoveParent);
	SAVEDATA_EDITOR_VARIABLE("MoveParent");
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CBaseEntity>, m_ahMoveChildren);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, AABB, m_aabbVisBoundingBox, "BoundingBox", AABB(Vector(-0.5f, -0.5f, -0.5f), Vector(0.5f, 0.5f, 0.5f)));
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bGlobalTransformsDirty);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, TMatrix, m_mGlobalTransform);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_NETVAR, TVector, m_vecGlobalGravity, "GlobalGravity", TVector(0, 0, -9.8f));
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, TMatrix, m_mLocalTransform);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Quaternion, m_qLocalRotation);
	SAVEDATA_DEFINE_HANDLE_FUNCTION(CSaveData::DATA_NETVAR, TVector, m_vecLocalOrigin, "Origin", UnserializeString_LocalOrigin);
	SAVEDATA_EDITOR_VARIABLE("Origin");
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, TVector, m_vecLastLocalOrigin);
	SAVEDATA_DEFINE_HANDLE_FUNCTION(CSaveData::DATA_NETVAR, EAngle, m_angLocalAngles, "Angles", UnserializeString_LocalAngles);
	SAVEDATA_EDITOR_VARIABLE("Angles");
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, TVector, m_vecLocalVelocity);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_NETVAR, Vector, m_vecScale, "Scale", Vector(1, 1, 1));
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, EAngle, m_angView, "ViewAngles");
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iHandle);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_NETVAR, bool, m_bTakeDamage, "TakeDamage", false);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_NETVAR, float, m_flTotalHealth, "TotalHealth");
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_NETVAR, float, m_flHealth, "Health");
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, double, m_flTimeKilled);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, double, m_flLastTakeDamage);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_NETVAR, bool, m_bActive, "Active", true);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CTeam>, m_hTeam);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bVisible, "Visible", true);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bInPhysics);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, bool, m_bDeleted);	// Deleted entities are not saved.
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bClientSpawn);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_iCollisionGroup);
	SAVEDATA_DEFINE_HANDLE_DEFAULT_FUNCTION(CSaveData::DATA_NETVAR, size_t, m_iModel, "Model", ~0, UnserializeString_ModelID);
	SAVEDATA_OVERRIDE_DEFAULT(CSaveData::DATA_NETVAR, const char*, m_iModel, "Model", ""); // Special for model: Store a string instead of an int.
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CMaterialHandle, m_hMaterialModel);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bRenderInverted, "RenderInverted", false);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bDisableBackCulling, "DisableBackCulling", false);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iSpawnSeed);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, double, m_flSpawnTime);
	SAVEDATA_DEFINE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bUsable, false);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBaseEntity);
	INPUT_DEFINE(SetLocalOrigin);
	INPUT_DEFINE(SetLocalAngles);
	INPUT_DEFINE(SetViewAngles);
	INPUT_DEFINE(SetVisible);
	INPUT_DEFINE(Activate);
	INPUT_DEFINE(Deactivate);
	INPUT_DEFINE(ToggleActive);
	INPUT_DEFINE(SetActive);
	INPUT_DEFINE(Use);
	INPUT_DEFINE(RemoveOutput);
	INPUT_DEFINE(Delete);
INPUTS_TABLE_END();

CBaseEntity::CBaseEntity()
{
	if (g_bAutoImporting)
		return;

	m_oGameData.SetEntity(this);

	if (s_iOverrideEntityListIndex == ~0)
		m_iHandle = s_iNextEntityListIndex;
	else
		m_iHandle = s_iOverrideEntityListIndex;

	s_iNextEntityListIndex = (m_iHandle+1)%s_apEntityList.size();
	while (s_apEntityList[s_iNextEntityListIndex] != NULL)
	{
		s_iNextEntityListIndex = (s_iNextEntityListIndex+1)%s_apEntityList.size();
		TAssert(s_iNextEntityListIndex != m_iHandle);
	}

	s_apEntityList[m_iHandle] = this;

	s_iEntities++;

	m_vecScale = Vector(1, 1, 1);

	m_iCollisionGroup = 0;

	m_flTotalHealth = 1;
	m_flHealth = 1;
	m_flTimeKilled = 0;
	m_flLastTakeDamage = -1;

	m_bDeleted = false;
	m_bInPhysics = false;

	m_iSpawnSeed = 0;

	m_bClientSpawn = false;

	m_bGlobalTransformsDirty = true;
	m_hMoveParent = nullptr;
}

CBaseEntity::~CBaseEntity()
{
	if (g_bAutoImporting)
		return;

	if (IsInPhysics())
		RemoveFromPhysics();

	s_apEntityList[m_iHandle] = NULL;

	TAssert(s_iEntities > 0);
	s_iEntities--;
}

void CBaseEntity::SetSaveDataDefaults()
{
	tvector<tstring> asParents;
	CEntityRegistration* pRegistration;
	const char* pszClassName = m_sClassName.c_str();

	while (pszClassName)
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		TAssert(pRegistration);
		if (!pRegistration)
			break;

		asParents.push_back(pszClassName);
		pszClassName = pRegistration->m_pszParentClass;
	}

	// Set all defaults.
	for (size_t k = asParents.size()-1; k < asParents.size(); k--)
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(asParents[k]);

		for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
		{
			CSaveData* pSaveData = &pRegistration->m_aSaveData[i];

			if (!pSaveData->m_bDefault)
				continue;

			char* pDefault = &pSaveData->m_oDefault[0];

			if (pSaveData->m_bOverride && pSaveData->m_pszHandle && pSaveData->m_pszHandle[0])
			{
				size_t j = k;
				while (!pSaveData->m_iOffset && j < asParents.size())
					pSaveData = FindSaveDataByHandle(asParents[j++].c_str(), pSaveData->m_pszHandle);

				TAssert(pSaveData);
				if (!pSaveData)
					continue;
			}

			TAssert(pSaveData->m_iOffset);
			if (!pSaveData->m_iOffset)
				continue;

			if (strcmp(pSaveData->m_pszVariableName, "m_iModel") == 0)
			{
				// Special handling for models.
				SetModel(CModelLibrary::FindModel(pDefault));
				continue;
			}

			char* pData = (char*)this + pSaveData->m_iOffset;
			switch(pSaveData->m_eType)
			{
			case CSaveData::DATA_COPYTYPE:
				memcpy(pData, pDefault, pSaveData->m_iSizeOfType);
				break;

			case CSaveData::DATA_NETVAR:
			{
				CNetworkedVariableBase* pVariable = (CNetworkedVariableBase*)pData;
				pVariable->Set(pSaveData->m_iSizeOfType, pDefault);
				break;
			}

			case CSaveData::DATA_COPYARRAY:
			case CSaveData::DATA_COPYVECTOR:
				TUnimplemented();
				break;

			case CSaveData::DATA_STRING:
			case CSaveData::DATA_OUTPUT:
				break;
			}
		}

		pszClassName = pRegistration->m_pszParentClass;
	}
}

void CBaseEntity::Spawn()
{
}

void CBaseEntity::PostLoad()
{
	if (m_bInPhysics)
		GamePhysics()->SetEntityCollisionDisabled(this, !m_bVisible);
}

const TVector CBaseEntity::GetLocalCenter() const
{
	return GetLocalTransform() * m_aabbVisBoundingBox.Center();
}

const TVector CBaseEntity::GetGlobalCenter() const
{
	return GetGlobalTransform() * m_aabbVisBoundingBox.Center();
}

const TVector CBaseEntity::GetRenderCenter() const
{
	return GetRenderTransform() * m_aabbVisBoundingBox.Center();
}

const TFloat CBaseEntity::GetBoundingRadius() const
{
	if (m_hMaterialModel.IsValid())
	{
		if (!m_hMaterialModel->m_ahTextures.size())
			return 0;

		if (!m_hMaterialModel->m_ahTextures[0].IsValid())
			return 0;

		size_t iWidth = m_hMaterialModel->m_ahTextures[0]->m_iWidth;
		size_t iHeight = m_hMaterialModel->m_ahTextures[0]->m_iHeight;

		return (GetGlobalTransform() * ((TVector(0, (float)iWidth, (float)iHeight)/float(m_hMaterialModel->m_iTexelsPerMeter)) * m_vecScale.Get())).Length()/2.0f;
	}

	return (m_aabbVisBoundingBox.Size()*m_vecScale.Get()).Length()/2;
}

void CBaseEntity::SetModel(const tstring& sModel)
{
	size_t iModel = CModelLibrary::FindModel(sModel);

	if (iModel == ~0)
	{
		TError("Couldn't find model " + sModel + "\n");
		return;
	}

	SetModel(iModel);
}

void CBaseEntity::SetModel(size_t iModel)
{
	m_iModel = iModel;

	if (m_iModel.Get() == ~0)
		return;

	CModel* pModel = CModelLibrary::GetModel(iModel);
	if (pModel)
	{
		m_aabbVisBoundingBox = pModel->m_aabbVisBoundingBox;
		m_aabbPhysBoundingBox = pModel->m_aabbPhysBoundingBox;
	}

	OnSetModel();
}

CModel* CBaseEntity::GetModel() const
{
	return CModelLibrary::GetModel(GetModelID());
}

void CBaseEntity::SetMaterialModel(const CMaterialHandle& hMaterial)
{
	m_hMaterialModel = hMaterial;

	OnSetModel();
}

void CBaseEntity::SetMoveParent(CBaseEntity* pParent)
{
	TAssert(pParent != this);
	if (pParent == this)
		return;

	if (IsInPhysics() && pParent)
	{
		collision_type_t eCollisionType = GamePhysics()->GetEntityCollisionType(this);
		TAssert(eCollisionType == CT_KINEMATIC || eCollisionType == CT_CHARACTER);
	}

	if (m_hMoveParent.GetPointer() == pParent)
		return;

#ifdef _DEBUG
	TMatrix mDebugPreviousGlobal = GetGlobalTransform();
#endif

	if (m_hMoveParent != NULL)
	{
		TMatrix mPreviousGlobal = GetGlobalTransform();
		TVector vecPreviousVelocity = GetGlobalVelocity();
		TVector vecPreviousLastOrigin = mPreviousGlobal * GetLastLocalOrigin();
		EAngle angPreviousGlobalView = (Matrix4x4(m_hMoveParent->GetGlobalTransform()) * Matrix4x4(m_angView)).GetAngles();
		if (!m_hMoveParent->TransformsChildView())
			angPreviousGlobalView = m_angView;

		for (size_t i = 0; i < m_hMoveParent->m_ahMoveChildren.size(); i++)
		{
			if (m_hMoveParent->m_ahMoveChildren[i]->GetHandle() == GetHandle())
			{
				m_hMoveParent->m_ahMoveChildren.erase(i);
				break;
			}
		}
		m_hMoveParent = NULL;

		m_vecLocalVelocity = vecPreviousVelocity;
		m_mLocalTransform = mPreviousGlobal;
		m_vecLastLocalOrigin = vecPreviousLastOrigin;
		m_vecLocalOrigin = mPreviousGlobal.GetTranslation();
		m_qLocalRotation = Quaternion(mPreviousGlobal);
		m_angLocalAngles = mPreviousGlobal.GetAngles();
		m_angView = angPreviousGlobalView;

		InvalidateGlobalTransforms();
	}

	TVector vecPreviousVelocity = GetLocalVelocity();
	TVector vecPreviousLastOrigin = GetLastLocalOrigin();
	TMatrix mPreviousTransform = GetLocalTransform();

	m_hMoveParent = pParent;

	if (!pParent)
	{
#ifdef _DEBUG
//		TAssert(m_mLocalTransform == mDebugPreviousGlobal);
#endif

		return;
	}

	pParent->m_ahMoveChildren.push_back(this);

	TMatrix mGlobalToLocal = m_hMoveParent->GetGlobalToLocalTransform();

	m_vecLastLocalOrigin = mGlobalToLocal * vecPreviousLastOrigin;
	m_mLocalTransform = mGlobalToLocal * mPreviousTransform;
	m_vecLocalOrigin = m_mLocalTransform.GetTranslation();

	if (m_hMoveParent->TransformsChildView())
	{
		m_qLocalRotation = Quaternion(m_mLocalTransform);
		m_angLocalAngles = m_mLocalTransform.GetAngles();
		m_angView = (Matrix4x4(mGlobalToLocal) * Matrix4x4(m_angView)).GetAngles();
	}

	TFloat flVelocityLength = vecPreviousVelocity.Length();
	if (flVelocityLength > TFloat(0))
		m_vecLocalVelocity = mGlobalToLocal.TransformVector(vecPreviousVelocity);
	else
		m_vecLocalVelocity = TVector(0, 0, 0);

	InvalidateGlobalTransforms();

#ifdef _DEBUG
//	TAssert(GetGlobalTransform().GetAngles() == mDebugPreviousGlobal.GetAngles());
//	TAssert((GetGlobalTransform().GetTranslation()-mDebugPreviousGlobal.GetTranslation()).LengthSqr() < 0.05f);
#endif
}

void CBaseEntity::InvalidateGlobalTransforms()
{
	m_bGlobalTransformsDirty = true;

	InvalidateChildrenTransforms();
}

void CBaseEntity::InvalidateChildrenTransforms()
{
	for (size_t i = 0; i < m_ahMoveChildren.size(); i++)
		m_ahMoveChildren[i]->InvalidateGlobalTransforms();
}

const TMatrix CBaseEntity::GetParentGlobalTransform() const
{
	if (!HasMoveParent())
		return TMatrix();

	return GetMoveParent()->GetGlobalTransform();
}

const TMatrix CBaseEntity::GetGlobalTransform() const
{
	if (!m_bGlobalTransformsDirty)
		return m_mGlobalTransform;

	if (!m_hMoveParent)
		m_mGlobalTransform = m_mLocalTransform;
	else
		m_mGlobalTransform = m_hMoveParent->GetGlobalTransform() * m_mLocalTransform;

	m_bGlobalTransformsDirty = false;

	return m_mGlobalTransform;
}

void CBaseEntity::SetGlobalTransform(const TMatrix& m)
{
#ifdef _DEBUG
	if (dynamic_cast<CCharacter*>(this))
		TAssert(m.GetUpVector().Equals(GetUpVector(), 0.0001f));
#endif

	TMatrix mNew = m;
	if (HasMoveParent())
		mNew = GetMoveParent()->GetGlobalToLocalTransform() *  m;

	if (mNew != m_mLocalTransform)
		OnSetLocalTransform(mNew);

	if (HasMoveParent())
	{
		m_mLocalTransform = mNew;
		m_bGlobalTransformsDirty = true;
	}
	else
	{
		m_mGlobalTransform = m_mLocalTransform = mNew;
		m_bGlobalTransformsDirty = false;
	}

	InvalidateChildrenTransforms();

	m_vecLocalOrigin = m_mLocalTransform.GetTranslation();

	if (HasMoveParent() && GetMoveParent()->TransformsChildView())
	{
		m_angLocalAngles = m_mLocalTransform.GetAngles();
		m_qLocalRotation = Quaternion(m_mLocalTransform);
	}

	if (IsInPhysics())
		GamePhysics()->SetEntityTransform(this, GetGlobalTransform());

	PostSetLocalTransform(mNew);
}

const TMatrix CBaseEntity::GetGlobalToLocalTransform() const
{
	if (HasMoveParent())
		return (GetMoveParent()->GetGlobalTransform() * GetLocalTransform()).InvertedRT();
	else
		return GetGlobalTransform().InvertedRT();
}

const TVector CBaseEntity::GetGlobalOrigin() const
{
	return GetGlobalTransform().GetTranslation();
}

const EAngle CBaseEntity::GetGlobalAngles() const
{
	return GetGlobalTransform().GetAngles();
}

void CBaseEntity::SetGlobalOrigin(const TVector& vecOrigin)
{
	if (!m_hMoveParent)
		SetLocalOrigin(vecOrigin);
	else
	{
		TMatrix mGlobalToLocal = GetMoveParent()->GetGlobalToLocalTransform();
		SetLocalOrigin(mGlobalToLocal * vecOrigin);
	}
}

void CBaseEntity::SetGlobalAngles(const EAngle& angAngles)
{
	if (!m_hMoveParent || !GetMoveParent()->TransformsChildView())
		SetLocalAngles(angAngles);
	else
	{
		TMatrix mGlobalToLocal = m_hMoveParent->GetGlobalToLocalTransform();
		mGlobalToLocal.SetTranslation(TVector(0,0,0));
		TMatrix mGlobalAngles;
		mGlobalAngles.SetAngles(angAngles);
		TMatrix mLocalAngles = mGlobalToLocal * mGlobalAngles;
		SetLocalAngles(mLocalAngles.GetAngles());
	}
}

const TVector CBaseEntity::GetGlobalVelocity()
{
	if (IsInPhysics())
		return GamePhysics()->GetEntityVelocity(this);

	return GetParentGlobalTransform().TransformVector(GetLocalVelocity());
}

const TVector CBaseEntity::GetGlobalVelocity() const
{
	return GetParentGlobalTransform().TransformVector(GetLocalVelocity());
}

void CBaseEntity::SetGlobalVelocity(const TVector& vecVelocity)
{
	if (!m_hMoveParent)
		SetLocalVelocity(vecVelocity);
	else
		SetLocalVelocity(GetMoveParent()->GetGlobalToLocalTransform().TransformVector(vecVelocity));
}

void CBaseEntity::SetGlobalGravity(const TVector& vecGravity)
{
	m_vecGlobalGravity = vecGravity;

	if (IsInPhysics())
		GamePhysics()->SetEntityGravity(this, vecGravity);
}

void CBaseEntity::SetLocalTransform(const TMatrix& m)
{
	TMatrix mNew = m;
	OnSetLocalTransform(mNew);

	if (!m_vecLocalOrigin.IsInitialized())
		m_vecLocalOrigin = m.GetTranslation();

	EAngle angNew = mNew.GetAngles();
	if (!m_angLocalAngles.IsInitialized())
		m_angLocalAngles = angNew;

	m_mLocalTransform = mNew;

	if (IsInPhysics())
		GamePhysics()->SetEntityTransform(this, GetPhysicsTransform());

	if ((mNew.GetTranslation() - m_vecLocalOrigin).LengthSqr() > TFloat(0))
		m_vecLocalOrigin = mNew.GetTranslation();

	EAngle angDifference = angNew - m_angLocalAngles;
	if (fabs(angDifference.p) > 0.001f || fabs(angDifference.y) > 0.001f || fabs(angDifference.r) > 0.001f)
	{
		m_angLocalAngles = mNew.GetAngles();
		m_qLocalRotation = Quaternion(mNew);
	}

	PostSetLocalTransform(mNew);

	InvalidateGlobalTransforms();
}

void CBaseEntity::SetLocalRotation(const Quaternion& q)
{
	SetLocalAngles(q.GetAngles());

	InvalidateGlobalTransforms();

	if (IsInPhysics())
	{
		TAssert(!GetMoveParent());
		GamePhysics()->SetEntityTransform(this, GetGlobalTransform());
	}
}

void CBaseEntity::SetLocalOrigin(const TVector& vecOrigin)
{
	if (!m_vecLocalOrigin.IsInitialized())
		m_vecLocalOrigin = vecOrigin;

	if (IsInPhysics())
	{
		if (GetMoveParent())
		{
			collision_type_t eCollisionType = GamePhysics()->GetEntityCollisionType(this);
			TAssert(eCollisionType == CT_KINEMATIC || eCollisionType == CT_CHARACTER);
		}

		TMatrix mLocal = m_mLocalTransform;
		mLocal.SetTranslation(vecOrigin);

		TMatrix mGlobal = GetParentGlobalTransform() * mLocal;

		GamePhysics()->SetEntityTransform(this, mGlobal);
	}

	if ((vecOrigin - m_vecLocalOrigin).LengthSqr() == TFloat(0))
		return;

	TMatrix mNew = m_mLocalTransform;
	mNew.SetTranslation(vecOrigin);
	OnSetLocalTransform(mNew);

	m_vecLocalOrigin = mNew.GetTranslation();
	m_mLocalTransform = mNew;

	InvalidateGlobalTransforms();

	PostSetLocalTransform(mNew);
};

const TVector CBaseEntity::GetLastGlobalOrigin() const
{
	return GetParentGlobalTransform() * GetLastLocalOrigin();
}

void CBaseEntity::SetLocalVelocity(const TVector& vecVelocity)
{
	if (!m_vecLocalVelocity.IsInitialized())
		m_vecLocalVelocity = vecVelocity;

	if (IsInPhysics())
		GamePhysics()->SetEntityVelocity(this, GetParentGlobalTransform().TransformVector(vecVelocity));

	if ((vecVelocity - m_vecLocalVelocity).LengthSqr() == TFloat(0))
		return;

	m_vecLocalVelocity = vecVelocity;
}

void CBaseEntity::SetLocalAngles(const EAngle& angAngles)
{
	if (!m_angLocalAngles.IsInitialized())
		m_angLocalAngles = angAngles;

	if (IsInPhysics())
	{
		if (GetMoveParent())
		{
			collision_type_t eCollisionType = GamePhysics()->GetEntityCollisionType(this);
			TAssert(eCollisionType == CT_KINEMATIC || eCollisionType == CT_CHARACTER);
		}

		TMatrix mLocal = m_mLocalTransform;
		mLocal.SetAngles(angAngles);

		if (!GetMoveParent() || !GetMoveParent()->TransformsChildView())
			GamePhysics()->SetEntityTransform(this, mLocal);
		else
		{
			TMatrix mGlobal = GetParentGlobalTransform() * mLocal;
			GamePhysics()->SetEntityTransform(this, mGlobal);
		}
	}

	EAngle angDifference = angAngles - m_angLocalAngles;
	if (fabs(angDifference.p) < 0.001f && fabs(angDifference.y) < 0.001f && fabs(angDifference.r) < 0.001f)
		return;

	TMatrix mNew = m_mLocalTransform;
	mNew.SetAngles(angAngles);
	OnSetLocalTransform(mNew);

	m_angLocalAngles = mNew.GetAngles();

	m_mLocalTransform = mNew;
	m_qLocalRotation = Quaternion(mNew);

	PostSetLocalTransform(mNew);

	InvalidateGlobalTransforms();
}

void CBaseEntity::SetLocalOrigin(const tvector<tstring>& asArgs)
{
	if (asArgs.size() == 1)
	{
		CBaseEntity* pEntity = GetEntityByName(asArgs[0]);
		if (!pEntity)
		{
			TError("CBaseEntity::SetLocalOrigin could not find entity '" + asArgs[0] + "' to teleport to.\n");
			return;
		}

		Vector vecGlobal = pEntity->GetGlobalOrigin();
		SetLocalOrigin(vecGlobal);
		return;
	}

	if (asArgs.size() != 3)
	{
		TError("CBaseEntity::SetLocalOrigin with != 3 arguments. Was expecting \"x y z\"\n");
		return;
	}

	SetLocalOrigin(Vector((float)stof(asArgs[0]), (float)stof(asArgs[1]), (float)stof(asArgs[2])));
}

void CBaseEntity::SetLocalAngles(const tvector<tstring>& asArgs)
{
	if (asArgs.size() != 3)
	{
		TError("CBaseEntity::SetLocalAngles with != 3 arguments. Was expecting \"p y r\"\n");
		return;
	}

	SetLocalAngles(EAngle((float)stof(asArgs[0]), (float)stof(asArgs[1]), (float)stof(asArgs[2])));
}

void CBaseEntity::SetViewAngles(const tvector<tstring>& asArgs)
{
	if (asArgs.size() != 3)
	{
		TError("CCharacter::SetViewAngles with != 3 arguments. Was expecting \"p y r\"\n");
		return;
	}

	SetViewAngles(EAngle((float)stof(asArgs[0]), (float)stof(asArgs[1]), (float)stof(asArgs[2])));
}

CBaseEntity* CBaseEntity::GetEntity(size_t iHandle)
{
	if (iHandle >= GameServer()->GetMaxEntities())
		return NULL;

	return s_apEntityList[iHandle];
}

size_t CBaseEntity::GetNumEntities()
{
	return s_iEntities;
}

void CBaseEntity::SetVisible(bool bVisible)
{
	if (bVisible == m_bVisible)
		return;

	m_bVisible = bVisible;

	OnSetVisible(bVisible);
}

void CBaseEntity::OnSetVisible(bool bVisible)
{
	if (m_bInPhysics)
		GamePhysics()->SetEntityCollisionDisabled(this, !bVisible);
}

void CBaseEntity::SetVisible(const tvector<tstring>& sArgs)
{
	TAssert(sArgs.size());
	if (!sArgs.size())
	{
		TError("CBaseEntity(" + GetName() + "):SetVisible missing a value. Expecting \"On\" or \"Off\"\n");
		return;
	}

	bool bValue = (sArgs[0].comparei("yes") == 0 || sArgs[0].comparei("true") == 0 || sArgs[0].comparei("on") == 0 || stoi(sArgs[0]) != 0);

	SetVisible(bValue);
}

void CBaseEntity::AddToPhysics(collision_type_t eCollisionType)
{
	TAssert(!IsInPhysics());
	if (IsInPhysics())
		return;

	GamePhysics()->AddEntity(this, eCollisionType);
	m_bInPhysics = true;

	GamePhysics()->SetEntityCollisionDisabled(this, !IsVisible());
}

void CBaseEntity::RemoveFromPhysics()
{
	TAssert(IsInPhysics());
	if (!IsInPhysics())
		return;

	m_bInPhysics = false;
	GamePhysics()->RemoveEntity(this);
}

CTeam* CBaseEntity::GetTeam() const
{
	return m_hTeam;
}

void CBaseEntity::SetTeam(class CTeam* pTeam)
{
	m_hTeam = pTeam;
	OnTeamChange();
}

void CBaseEntity::ClientUpdate(int iClient)
{
}

void CBaseEntity::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit)
{
	if (!m_bTakeDamage)
		return;

	if (!Game()->TakesDamageFrom(this, pAttacker))
		return;

	bool bWasAlive = IsAlive();

	m_flLastTakeDamage = GameServer()->GetGameTime();

	if (GameNetwork()->IsHost())
		m_flHealth -= flDamage;

	OnTakeDamage(pAttacker, pInflictor, eDamageType, flDamage, bDirectHit);
	CallOutput("OnTakeDamage");

	Game()->OnTakeDamage(this, pAttacker, pInflictor, flDamage, bDirectHit, !IsAlive() && bWasAlive);

	if (bWasAlive && m_flHealth <= 0)
		Killed(pAttacker);
}

void CBaseEntity::Kill()
{
	if (!IsAlive())
		return;

	if (GameNetwork()->IsHost())
		m_flHealth = -1;

	Killed(NULL);
}

void CBaseEntity::Killed(CBaseEntity* pKilledBy)
{
	m_flTimeKilled = GameServer()->GetGameTime();

	OnKilled(pKilledBy);
	Game()->OnEntityKilled(this);

	CallOutput("OnKilled");
}

void CBaseEntity::OnKilled(CBaseEntity* pKilledBy)
{
	Delete();
}

void CBaseEntity::SetActive(bool bActive)
{
	if (bActive && !m_bActive)
	{
		OnActivated();
		CallOutput("OnActivated");
	}

	if (m_bActive && !bActive)
	{
		OnDeactivated();
		CallOutput("OnDeactivated");
	}

	m_bActive = bActive;
}

void CBaseEntity::Activate(const tvector<tstring>& sArgs)
{
	SetActive(true);
}

void CBaseEntity::Deactivate(const tvector<tstring>& sArgs)
{
	SetActive(false);
}

void CBaseEntity::ToggleActive(const tvector<tstring>& sArgs)
{
	SetActive(!IsActive());
}

void CBaseEntity::SetActive(const tvector<tstring>& sArgs)
{
	TAssert(sArgs.size());
	if (!sArgs.size())
	{
		TError("CBaseEntity(" + GetName() + "):SetActive missing a value. Expecting \"On\" or \"Off\"\n");
		return;
	}

	bool bValue = (sArgs[0].comparei("yes") == 0 || sArgs[0].comparei("true") == 0 || sArgs[0].comparei("on") == 0 || stoi(sArgs[0]) != 0);

	SetActive(bValue);
}

void CBaseEntity::Use(CBaseEntity* pUser)
{
	if (!IsActive())
		return;

	OnUse(pUser);

	CallOutput("OnUsed");
}

void CBaseEntity::OnUse(CBaseEntity* pUser)
{
}

void CBaseEntity::Use(const tvector<tstring>& sArgs)
{
	Use(nullptr);
}

void CBaseEntity::SetUsable(bool bUsable)
{
	m_bUsable = bUsable;
}

bool CBaseEntity::IsUsable(const CBaseEntity* pUser) const
{
	return m_bUsable;
}

bool CBaseEntity::ShouldRender() const
{
	if ((size_t)m_iModel != ~0)
	{
		CModel* pModel = CModelLibrary::GetModel(m_iModel);

		for (size_t i = 0; i < pModel->m_ahMaterials.size(); i++)
		{
			CMaterialHandle& hMaterial = pModel->m_ahMaterials[i];

			// Invalid material counts as a material, it'll be swapped with the invalid texture
			if (!hMaterial.IsValid())
				return true;

			if (!hMaterial->m_sBlend.length() || hMaterial->m_sBlend == "none")
				return true;
		}

		for (size_t i = 0; i < pModel->m_pToy->GetNumSceneAreas(); i++)
		{
			size_t iSceneAreaModel = CModelLibrary::FindModel(pModel->m_pToy->GetSceneAreaFileName(i));
			CModel* pSceneAreaModel = CModelLibrary::GetModel(iSceneAreaModel);

			TAssert(pSceneAreaModel);

			if (!pSceneAreaModel)
				continue;

			for (size_t j = 0; j < pSceneAreaModel->m_pToy->GetNumMaterials(); j++)
			{
				CMaterialHandle& hMaterial = pSceneAreaModel->m_ahMaterials[j];

				if (!hMaterial.IsValid())
					return true;

				if (!hMaterial->m_sBlend.length() || hMaterial->m_sBlend == "none")
					return true;
			}
		}
	}

	if (m_hMaterialModel.IsValid())
		return (!m_hMaterialModel->m_sBlend.length() || m_hMaterialModel->m_sBlend == "none");

	return false;
}

bool CBaseEntity::ShouldRenderTransparent() const
{
	if (m_hMaterialModel.IsValid())
		return m_hMaterialModel->m_sBlend.length() && m_hMaterialModel->m_sBlend != "none";

	if ((size_t)m_iModel != ~0)
	{
		CModel* pModel = CModelLibrary::GetModel(m_iModel);

		for (size_t i = 0; i < pModel->m_ahMaterials.size(); i++)
		{
			CMaterialHandle& hMaterial = pModel->m_ahMaterials[i];

			if (!hMaterial.IsValid())
				continue;

			if (hMaterial->m_sBlend.length() && hMaterial->m_sBlend != "none")
				return true;
		}
	}

	return false;
}

CVar show_centers("debug_show_centers", "off");

void CBaseEntity::PreRender() const
{
	if (ShouldRenderModel() && CModelLibrary::GetModel(m_iModel))
		GameServer()->GetRenderer()->ClassifySceneAreaPosition(CModelLibrary::GetModel(m_iModel));
}

void CBaseEntity::Render() const
{
	TPROF("CBaseEntity::Render");

	if (!IsVisible())
		return;

	PreRender();

	do {
		CGameRenderingContext r(GameServer()->GetRenderer(), true);

		// If another context already set this, don't clobber it.
		if (!r.GetActiveFrameBuffer())
			r.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		r.Transform(GetRenderTransform());

		if (m_bRenderInverted)
			r.SetWinding(!r.GetWinding());

		if (m_bDisableBackCulling)
			r.SetBackCulling(false);

		ModifyContext(&r);

		if (r.GetAlpha() == 0)
			continue;

		if (ShouldRenderModel())
		{
			if (m_iModel != (size_t)~0)
			{
				TPROF("CRenderingContext::RenderModel()");
				r.RenderModel(GetModelID(), this);
			}

			if (m_hMaterialModel.IsValid())
			{
				TPROF("CRenderingContext::RenderModel(Material)");
				r.Scale((float)m_vecScale.Get().x, (float)m_vecScale.Get().y, (float)m_vecScale.Get().z);
				r.RenderMaterialModel(m_hMaterialModel, this);
			}
		}

		OnRender(&r);
	} while (false);

	PostRender();

	if (show_centers.GetBool())
	{
		CRenderingContext r(GameServer()->GetRenderer(), true);
		r.UseProgram("model");
		r.Translate(GetGlobalCenter());
		r.BeginRenderDebugLines();
			r.Vertex(Vector(-1, 0, 0));
			r.Vertex(Vector(1, 0, 0));
			r.Vertex(Vector(0, -1, 0));
			r.Vertex(Vector(0, 1, 0));
			r.Vertex(Vector(0, 0, -1));
			r.Vertex(Vector(0, 0, 1));
		r.EndRender();
	}
}

void CBaseEntity::RenderTransparent() const
{
	TPROF("CBaseEntity::RenderTransparent");

	if (!IsVisible())
		return;

	PreRender();

	do {
		CGameRenderingContext r(GameServer()->GetRenderer(), true);

		// If another context already set this, don't clobber it.
		if (!r.GetActiveFrameBuffer())
			r.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		r.Transform(GetRenderTransform());

		if (m_bRenderInverted)
			r.SetWinding(!r.GetWinding());

		if (m_bDisableBackCulling)
			r.SetBackCulling(false);

		ModifyContext(&r);

		if (r.GetAlpha() == 0)
			continue;

		if (ShouldRenderModel())
		{
			if (m_iModel != (size_t)~0)
			{
				TPROF("CRenderingContext::RenderModel()");
				r.RenderModel(GetModelID(), this);
			}

			if (m_hMaterialModel.IsValid())
			{
				TPROF("CRenderingContext::RenderModel(Material)");
				r.SetBlend(BLEND_ALPHA);
				r.Scale((float)m_vecScale.Get().x, (float)m_vecScale.Get().y, (float)m_vecScale.Get().z);
				r.RenderMaterialModel(m_hMaterialModel, this);
			}
		}

		OnRender(&r);
	} while (false);

	PostRender();
}

void CBaseEntity::Delete()
{
	GameServer()->Delete(this);
}

void CBaseEntity::Delete(const tvector<tstring>& sArgs)
{
	Delete();
}

void CBaseEntity::OnDeleted(const CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahMoveChildren.size(); i++)
	{
		if (pEntity == m_ahMoveChildren[i])
		{
			m_ahMoveChildren.erase(i);
			break;
		}
	}
}

void CBaseEntity::CallInput(const tstring& sName, const tstring& sArgs)
{
	CEntityInput* pInput = FindInput(sName.c_str());

	if (!pInput)
	{
		TAssert(!"Input missing.");
		TMsg(sprintf(tstring("Input %s not found in %s\n"), sName.c_str(), GetClassName()));
		return;
	}

	tvector<tstring> asArgs;
	tstrtok(sArgs, asArgs);

	s_sCurrentInput = sName + " " + sArgs;
	pInput->m_pfnCallback(this, asArgs);
	s_sCurrentInput.clear();
}

void CBaseEntity::CallOutput(const tstring& sName)
{
	CSaveData* pData = FindSaveData((tstring("m_Output_") + sName).c_str());

	if (!pData)
	{
		TAssert(!"Output missing.");
		TMsg(sprintf(tstring("Called nonexistant output %s of entity %s\n"), sName.c_str(), GetClassName()));
		return;
	}

	CEntityOutput* pOutput = (CEntityOutput*)((size_t)this + (size_t)pData->m_iOffset);
	pOutput->SetEntity(this);
	pOutput->SetOutputName(sName);
	pOutput->Call();
}

void CBaseEntity::AddOutputTarget(const tstring& sName, const tstring& sTargetName, const tstring& sInput, const tstring& sArgs, bool bKill)
{
	CSaveData* pData = FindSaveData((tstring("m_Output_") + sName).c_str());

	if (!pData)
	{
		TAssert(!"Output missing.");
		TMsg(sprintf(tstring("Called nonexistant output %s of entity %s\n"), sName.c_str(), GetClassName()));
		return;
	}

	CEntityOutput* pOutput = (CEntityOutput*)((size_t)this + (size_t)pData->m_iOffset);
	pOutput->AddTarget(sTargetName, sInput, sArgs, bKill);
}

void CBaseEntity::RemoveOutputs(const tstring& sName)
{
	CSaveData* pData = FindSaveData((tstring("m_Output_") + sName).c_str());

	if (!pData)
	{
		TAssert(!"Output missing.");
		TMsg(sprintf(tstring("Called nonexistant output %s of entity %s\n"), sName.c_str(), GetClassName()));
		return;
	}

	CEntityOutput* pOutput = (CEntityOutput*)((size_t)this + (size_t)pData->m_iOffset);
	pOutput->Clear();
}

void CBaseEntity::RemoveOutput(const tvector<tstring>& sArgs)
{
	if (sArgs.size() == 0)
	{
		TMsg("RemoveOutput called without a output name argument.\n");
		return;
	}

	RemoveOutputs(sArgs[0]);
}

CVar debug_entity_outputs("debug_entity_outputs", "off");

void CEntityOutput::Call()
{
	bool bTargetCalled = false;

	for (size_t j = 0; j < m_aTargets.size(); j++)
	{
		CEntityOutputTarget* pTarget = &m_aTargets[j];

		if (pTarget->m_sTargetName.length() == 0)
			continue;

		if (pTarget->m_sInput.length() == 0)
			continue;

		tvector<CBaseEntity*> apEntities;
		CBaseEntity::FindEntitiesByName(pTarget->m_sTargetName, apEntities);

		for (size_t i = 0; i < apEntities.size(); i++)
		{
			tstring sFormattedArgs = FormatArgs(pTarget->m_sArgs);
			CBaseEntity* pTargetEntity = apEntities[i];

			if (debug_entity_outputs.GetBool())
				TMsg(tstring(m_pEnt->GetClassName()) + "(\"" + m_pEnt->GetName() + "\")." + m_sOutputName + "() -> " + pTargetEntity->GetClassName() + "(\"" + pTargetEntity->GetName() + "\")." + pTarget->m_sInput + "(\"" + sFormattedArgs + "\")\n");

			pTargetEntity->CallInput(pTarget->m_sInput, sFormattedArgs);

			bTargetCalled = true;
		}

		if (!apEntities.size())
			TError("Couldn't find any entity with name '" + pTarget->m_sTargetName + "'\n");
	}

	if (!bTargetCalled)
	{
		if (debug_entity_outputs.GetInt() > 1)
			TMsg(tstring(m_pEnt->GetClassName()) + "(\"" + m_pEnt->GetName() + "\")." + m_sOutputName + "() -> none\n");

		return;
	}

	for (size_t i = 0; i < m_aTargets.size(); i++)
	{
		CEntityOutputTarget* pTarget = &m_aTargets[i];
		if (pTarget->m_bKill)
		{
			m_aTargets.erase(m_aTargets.begin()+i);
			i--;
		}
	}
}

void CEntityOutput::AddTarget(const tstring& sTargetName, const tstring& sInput, const tstring& sArgs, bool bKill)
{
	CEntityOutputTarget* pTarget = &m_aTargets.push_back();

	pTarget->m_sTargetName = sTargetName;
	pTarget->m_sInput = sInput;
	pTarget->m_sArgs = sArgs;
	pTarget->m_bKill = bKill;
}

void CEntityOutput::Clear()
{
	m_aTargets.clear();
}

const tstring CEntityOutput::FormatArgs(tstring sArgs)
{
	size_t iArg = 0;

	while (true)
	{
		tstring sArg = sprintf("[%d]", iArg);
		auto i = sArgs.find(sArg);
		if (i == tstring::npos)
			return sArgs;

		sArgs.replace(i, sArg.length(), m_pEnt->GetOutputValue(m_sOutputName, iArg));

		iArg++;
	}
}

SERVER_GAME_COMMAND(EmitSound)
{
	if (pCmd->GetNumArguments() < 4)
	{
		TMsg("EmitSound with less than 4 arguments.\n");
		return;
	}

	CSoundLibrary::PlaySound(CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(0)), pCmd->Arg(3), pCmd->ArgAsFloat(1), !!pCmd->ArgAsInt(2));
}

void CBaseEntity::EmitSound(const tstring& sFilename, float flVolume, bool bLoop)
{
	::EmitSound.RunCommand(sprintf(tstring("%d %.1f %d %s"), GetHandle(), flVolume, bLoop?1:0, sFilename.c_str()));
}

SERVER_GAME_COMMAND(StopSound)
{
	if (pCmd->GetNumArguments() < 2)
	{
		TMsg("StopSound with less than 2 arguments.\n");
		return;
	}

	CSoundLibrary::StopSound(CEntityHandle<CBaseEntity>(pCmd->ArgAsUInt(0)), pCmd->Arg(1));
}

void CBaseEntity::StopSound(const tstring& sFilename)
{
	::StopSound.RunCommand(sprintf(tstring("%d %s"), GetHandle(), sFilename.c_str()));
}

bool CBaseEntity::IsSoundPlaying(const tstring& sFilename)
{
	return CSoundLibrary::IsSoundPlaying(this, sFilename);
}

void CBaseEntity::SetSoundVolume(const tstring& sFilename, float flVolume)
{
	CSoundLibrary::SetSoundVolume(this, sFilename, flVolume);
}

const TFloat CBaseEntity::Distance(const TVector& vecSpot) const
{
	TFloat flDistance = (GetGlobalCenter() - vecSpot).Length();
	if (flDistance < GetBoundingRadius())
		return 0;

	return flDistance - GetBoundingRadius();
}

bool CBaseEntity::ShouldCollideWith(size_t iOtherHandle, const TVector& vecPoint) const
{
	CEntityHandle<CBaseEntity> hOther(iOtherHandle);
	TAssert(hOther);
	if (!hOther)
		return false;

	return ShouldCollideWith(hOther, vecPoint);
}

void CBaseEntity::SetSpawnSeed(size_t iSpawnSeed)
{
	m_iSpawnSeed = iSpawnSeed;

	mtsrand(iSpawnSeed);
}

SERVER_GAME_COMMAND(ClientSpawn)
{
	if (pCmd->GetNumArguments() < 1)
	{
		TMsg("ClientSpawn with no arguments.\n");
		return;
	}

	CEntityHandle<CBaseEntity> hEntity(pCmd->ArgAsUInt(0));

	if (!hEntity)
	{
		TMsg("ClientSpawn with invalid entity.\n");
		return;
	}

	hEntity->ClientSpawn();
}

void CBaseEntity::IssueClientSpawn()
{
	::ClientSpawn.RunCommand(sprintf(tstring("%d"), GetHandle()));
	m_bClientSpawn = true;
}

// ClientSpawn is always guaranteed to run after the client has received all initial data about a new entity.
void CBaseEntity::ClientSpawn()
{
	TAssert(!m_bClientSpawn);
	m_bClientSpawn = true;

	CallOutput("OnSpawn");
}

void CBaseEntity::SaveData_OverrideDefault(CEntityRegistration* pRegistration, CSaveData::datatype_t eDataType, const char* pszName, const char* pszHandle, const char* pszDefault)
{
	CSaveData* pSaveData = FindSaveDataByHandle(pszHandle, true);
	if (!pSaveData)
	{
		CSaveData* pHandleData = FindSaveDataByHandle(pRegistration->m_pszParentClass, pszHandle);
		TAssert(pHandleData);

		pSaveData = &pRegistration->m_aSaveData.push_back();
		pSaveData->m_eType = eDataType;
		pSaveData->m_pszVariableName = pszName;
		pSaveData->m_pszHandle = pszHandle;
		pSaveData->m_bOverride = true;
		pSaveData->m_bShowInEditor = false;
		pSaveData->m_pszType = "const char*";

		pSaveData->m_iOffset = pHandleData->m_iOffset;
		pSaveData->m_iSizeOfVariable = pHandleData->m_iSizeOfVariable;
		pSaveData->m_iSizeOfType = pHandleData->m_iSizeOfType;
	}

	strncpy(pSaveData->m_oDefault, pszDefault, sizeof(pSaveData->m_oDefault));
	pSaveData->m_bDefault = true;
}

void CBaseEntity::SaveData_OverrideDefault(CEntityRegistration* pRegistration, CSaveData::datatype_t eDataType, const char* pszName, const char* pszHandle, float flDefault)
{
	CSaveData* pSaveData = FindSaveDataByHandle(pszHandle, true);
	if (!pSaveData)
	{
		CSaveData* pHandleData = FindSaveDataByHandle(pRegistration->m_pszParentClass, pszHandle);
		TAssert(pHandleData);

		pSaveData = &pRegistration->m_aSaveData.push_back();
		pSaveData->m_eType = eDataType;
		pSaveData->m_pszVariableName = pszName;
		pSaveData->m_pszHandle = pszHandle;
		pSaveData->m_bOverride = true;
		pSaveData->m_bShowInEditor = false;
		pSaveData->m_pszType = "float";

		pSaveData->m_iOffset = pHandleData->m_iOffset;
		pSaveData->m_iSizeOfVariable = pHandleData->m_iSizeOfVariable;
		pSaveData->m_iSizeOfType = pHandleData->m_iSizeOfType;
	}

	memcpy(pSaveData->m_oDefault, &flDefault, sizeof(float));
	pSaveData->m_bDefault = true;
}

void CBaseEntity::SaveData_OverrideDefault(CEntityRegistration* pRegistration, CSaveData::datatype_t eDataType, const char* pszName, const char* pszHandle, int iDefault)
{
	CSaveData* pSaveData = FindSaveDataByHandle(pszHandle, true);
	if (!pSaveData)
	{
		CSaveData* pHandleData = FindSaveDataByHandle(pRegistration->m_pszParentClass, pszHandle);
		TAssert(pHandleData);

		pSaveData = &pRegistration->m_aSaveData.push_back();
		pSaveData->m_eType = eDataType;
		pSaveData->m_pszVariableName = pszName;
		pSaveData->m_pszHandle = pszHandle;
		pSaveData->m_bOverride = true;
		pSaveData->m_bShowInEditor = false;
		pSaveData->m_pszType = "int";

		pSaveData->m_iOffset = pHandleData->m_iOffset;
		pSaveData->m_iSizeOfVariable = pHandleData->m_iSizeOfVariable;
		pSaveData->m_iSizeOfType = pHandleData->m_iSizeOfType;
	}

	memcpy(pSaveData->m_oDefault, &iDefault, sizeof(int));
	pSaveData->m_bDefault = true;
}

void CBaseEntity::SaveData_OverrideDefault(CEntityRegistration* pRegistration, CSaveData::datatype_t eDataType, const char* pszName, const char* pszHandle, const AABB& aabbDefault)
{
	CSaveData* pSaveData = FindSaveDataByHandle(pszHandle, true);
	if (!pSaveData)
	{
		CSaveData* pHandleData = FindSaveDataByHandle(pRegistration->m_pszParentClass, pszHandle);
		TAssert(pHandleData);

		pSaveData = &pRegistration->m_aSaveData.push_back();
		pSaveData->m_eType = eDataType;
		pSaveData->m_pszVariableName = pszName;
		pSaveData->m_pszHandle = pszHandle;
		pSaveData->m_bOverride = true;
		pSaveData->m_bShowInEditor = false;
		pSaveData->m_pszType = "AABB";

		pSaveData->m_iOffset = pHandleData->m_iOffset;
		pSaveData->m_iSizeOfVariable = pHandleData->m_iSizeOfVariable;
		pSaveData->m_iSizeOfType = pHandleData->m_iSizeOfType;
	}

	memcpy(pSaveData->m_oDefault, &aabbDefault, sizeof(AABB));
	pSaveData->m_bDefault = true;
}

void CBaseEntity::SaveData_OverrideDefault(CEntityRegistration* pRegistration, CSaveData::datatype_t eDataType, const char* pszName, const char* pszHandle, bool bDefault)
{
	CSaveData* pSaveData = FindSaveDataByHandle(pszHandle, true);
	if (!pSaveData)
	{
		CSaveData* pHandleData = FindSaveDataByHandle(pRegistration->m_pszParentClass, pszHandle);
		TAssert(pHandleData);

		pSaveData = &pRegistration->m_aSaveData.push_back();
		pSaveData->m_eType = eDataType;
		pSaveData->m_pszVariableName = pszName;
		pSaveData->m_pszHandle = pszHandle;
		pSaveData->m_bOverride = true;
		pSaveData->m_bShowInEditor = false;
		pSaveData->m_pszType = "bool";

		pSaveData->m_iOffset = pHandleData->m_iOffset;
		pSaveData->m_iSizeOfVariable = pHandleData->m_iSizeOfVariable;
		pSaveData->m_iSizeOfType = pHandleData->m_iSizeOfType;
	}

	memcpy(pSaveData->m_oDefault, &bDefault, sizeof(bool));
	pSaveData->m_bDefault = true;
}

void CBaseEntity::SaveData_EditorVariable(CEntityRegistration* pRegistration, const char* pszHandle)
{
	CSaveData* pSaveData = FindSaveDataByHandle(pszHandle, true);
	if (pSaveData)
	{
		pSaveData->m_bShowInEditor = true;
	}
	else
	{
		CSaveData* pHandleData = FindSaveDataByHandle(pszHandle);
		TAssert(pHandleData);
		if (pHandleData)
		{
			pRegistration->m_aSaveData.push_back(*pHandleData);
			pSaveData = &pRegistration->m_aSaveData.back();
			pSaveData->m_bShowInEditor = true;
			pSaveData->m_bOverride = false;
		}
	}
}

CSaveData* CBaseEntity::FindSaveData(const char* pszName, bool bThisClassOnly)
{
	return FindSaveData(GetClassName(), pszName, bThisClassOnly);
}

CSaveData* CBaseEntity::FindSaveDataByHandle(const char* pszHandle, bool bThisClassOnly)
{
	return FindSaveDataByHandle(GetClassName(), pszHandle, bThisClassOnly);
}

CNetworkedVariableData* CBaseEntity::FindNetworkVariable(const char* pszName, bool bThisClassOnly)
{
	const tchar* pszClassName = GetClassName();
	CEntityRegistration* pRegistration = NULL;
	
	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		for (size_t i = 0; i < pRegistration->m_aNetworkVariables.size(); i++)
		{
			CNetworkedVariableData* pVarData = &pRegistration->m_aNetworkVariables[i];

			if (strcmp(pVarData->m_pszName, pszName) == 0)
				return pVarData;
		}

		if (bThisClassOnly)
			return nullptr;

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	return NULL;
}

CEntityInput* CBaseEntity::FindInput(const char* pszName, bool bThisClassOnly)
{
	const tchar* pszClassName = GetClassName();
	CEntityRegistration* pRegistration = NULL;
	
	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		tmap<tstring, CEntityInput>::iterator it = pRegistration->m_aInputs.find(pszName);

		if (it != pRegistration->m_aInputs.end())
			return &it->second;

		if (bThisClassOnly)
			return nullptr;

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	return NULL;
}

void CBaseEntity::CheckSaveDataSize(CEntityRegistration* pRegistration)
{
	// I don't give a fuck about this anymore. It never fucking works.
	// It's supposed to check my back and tell me when I forget to add shit but fuck if it can do that properly.
	return;

	size_t iSaveTableSize = 0;

	size_t iFirstOffset = 0;
	if (pRegistration->m_aSaveData.size())
		iFirstOffset = pRegistration->m_aSaveData[0].m_iOffset;

	for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
	{
		CSaveData* pData = &pRegistration->m_aSaveData[i];

		if (pData->m_bOverride)
			continue;

		// If bools have non-bools after them then the extra space is padded to retain four-byte alignment.
		// So, round everything up. Might mean adding bools doesn't trigger it, oh well.
		if (pData->m_iSizeOfVariable%4 == 0 && iSaveTableSize%4 != 0)
			iSaveTableSize += 4-iSaveTableSize%4;

		// This can help you find where missing stuff is, if all of the save data is in order.
		// On GCC there's also a problem where a boolean at the end of a parent class can make the beginning address of any child classes be not a multiple of 4, which can cause this to trip. Solution: Keep your booleans near the center of your class definitions. (Really should rewrite this function but meh.)
		TAssert(pData->m_iOffset - iFirstOffset == iSaveTableSize);

		iSaveTableSize += pData->m_iSizeOfVariable;

	//	TMsg(sprintf(tstring("%s::%s %d\n"), pRegistration->m_pszEntityClass, pData->m_pszVariableName, iSaveTableSize));
	}

	// In case a bool is at the end.
	if (iSaveTableSize%4)
		iSaveTableSize += 4-iSaveTableSize%4;

	size_t iSizeOfThis = SizeOfThis();

	// If you're getting this assert it probably means you forgot to add a savedata entry for some variable that you added to a class.
	if (iSaveTableSize != iSizeOfThis)
	{
		TMsg(sprintf(tstring("Save table for class '%s' doesn't match the class's size, %d != %d.\n"), GetClassName(), iSaveTableSize, iSizeOfThis));
//		TAssert(!"Save table size doesn't match class size.\n");
	}
}

void CBaseEntity::CheckTables(const char* pszEntity)
{
#ifndef _DEBUG
	return;
#endif

	CEntityRegistration* pRegistration = GetRegisteredEntity(pszEntity);

	tvector<CSaveData>& aSaveData = pRegistration->m_aSaveData;

	for (size_t i = 0; i < aSaveData.size(); i++)
	{
		CSaveData* pSaveData = &aSaveData[i];
		CNetworkedVariableData* pVariable = FindNetworkVariable(pSaveData->m_pszVariableName);
		if (pSaveData->m_eType == CSaveData::DATA_NETVAR)
			// I better be finding this in the network tables or yer gon have some 'splainin to do!
			TAssert(pVariable)
		else
		{
			// I better NOT be finding this in the network tables or yer gon have some 'splainin to do!
			TAssert(!pVariable);
		}
	}
}

void CBaseEntity::ClientEnterGame()
{
	SetModel(m_iModel);
}

void CBaseEntity::SerializeEntity(std::ostream& o, CBaseEntity* pEntity)
{
	writetstring(o, pEntity->GetClassName());

	size_t iHandle = pEntity->GetHandle();
	o.write((char*)&iHandle, sizeof(iHandle));

	size_t iSpawnSeed = pEntity->GetSpawnSeed();
	o.write((char*)&iSpawnSeed, sizeof(iSpawnSeed));

	pEntity->Serialize(o);
	pEntity->OnSerialize(o);
}

bool CBaseEntity::UnserializeEntity(std::istream& i)
{
	tstring sClassName = readtstring(i);

	size_t iHandle;
	i.read((char*)&iHandle, sizeof(iHandle));

	size_t iSpawnSeed;
	i.read((char*)&iSpawnSeed, sizeof(iSpawnSeed));

	size_t iNewHandle = GameServer()->CreateEntity(sClassName, iHandle, iSpawnSeed);
	TAssert(iNewHandle == iHandle);

	CEntityHandle<CBaseEntity> hEntity(iNewHandle);

	if (!hEntity->Unserialize(i))
		return false;

	return hEntity->OnUnserialize(i);
}

void CBaseEntity::Serialize(std::ostream& o, const char* pszClassName, void* pEntity)
{
	CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

	size_t iSaveDataSize = 0;
	for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
	{
		CSaveData* pSaveData = &pRegistration->m_aSaveData[i];
		if (pSaveData->m_eType != CSaveData::DATA_OMIT)
			iSaveDataSize++;
	}

	o.write((char*)&iSaveDataSize, sizeof(iSaveDataSize));

	for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
	{
		CSaveData* pSaveData = &pRegistration->m_aSaveData[i];

		if (pSaveData->m_eType == CSaveData::DATA_OMIT)
			continue;

		o.write((char*)&i, sizeof(i));

		char* pData = (char*)pEntity + pSaveData->m_iOffset;
		switch(pSaveData->m_eType)
		{
		case CSaveData::DATA_COPYTYPE:
			o.write(pData, pSaveData->m_iSizeOfType);
			break;

		case CSaveData::DATA_COPYARRAY:
			o.write(pData, pSaveData->m_iSizeOfVariable);
			break;

		case CSaveData::DATA_COPYVECTOR:
		{
			tvector<size_t>* pVector = (tvector<size_t>*)pData;
			size_t iSize = pVector->size();
			o.write((char*)&iSize, sizeof(iSize));
			if (iSize)
				o.write((char*)pVector->data(), pSaveData->m_iSizeOfType*iSize);
			break;
		}

		case CSaveData::DATA_NETVAR:
		{
			size_t iDataLength;
			CNetworkedVariableBase* pVariable = (CNetworkedVariableBase*)pData;
			char* pRealData = (char*)pVariable->Serialize(iDataLength);
			o.write((char*)&iDataLength, sizeof(iDataLength));
			o.write(pRealData, iDataLength);
			break;
		}

		case CSaveData::DATA_STRING:
			writestring(o, *(tstring*)pData);
			break;

		case CSaveData::DATA_OUTPUT:
		{
			CEntityOutput* pOutput = (CEntityOutput*)pData;
			size_t iTargets = pOutput->m_aTargets.size();
			o.write((char*)&iTargets, sizeof(iTargets));
			for (size_t i = 0; i < pOutput->m_aTargets.size(); i++)
			{
				CEntityOutput::CEntityOutputTarget* pTarget = &pOutput->m_aTargets[i];
				writestring(o, pTarget->m_sTargetName);
				writestring(o, pTarget->m_sInput);
				writestring(o, pTarget->m_sArgs);
				o.write((char*)&pTarget->m_bKill, sizeof(pTarget->m_bKill));
			}
			break;
		}
		}
	}
}

bool CBaseEntity::Unserialize(std::istream& i, const char* pszClassName, void* pEntity)
{
	CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

	size_t iSaveDataSize;
	i.read((char*)&iSaveDataSize, sizeof(iSaveDataSize));

	for (size_t j = 0; j < iSaveDataSize; j++)
	{
		size_t iSaveData;
		i.read((char*)&iSaveData, sizeof(iSaveData));

		CSaveData* pSaveData = &pRegistration->m_aSaveData[iSaveData];

		TAssert(pSaveData->m_eType != CSaveData::DATA_OMIT);

		char* pData = (char*)pEntity + pSaveData->m_iOffset;
		switch(pSaveData->m_eType)
		{
		case CSaveData::DATA_COPYTYPE:
			i.read(pData, pSaveData->m_iSizeOfType);
			break;

		case CSaveData::DATA_COPYARRAY:
			i.read(pData, pSaveData->m_iSizeOfVariable);
			break;

		case CSaveData::DATA_COPYVECTOR:
		{
			tvector<size_t>* pVector = (tvector<size_t>*)pData;
			size_t iSize;
			i.read((char*)&iSize, sizeof(iSize));
			if (iSize)
			{
				pSaveData->m_pfnResizeVector(pData, iSize);
				i.read((char*)pVector->data(), pSaveData->m_iSizeOfType*iSize);
			}
			break;
		}

		case CSaveData::DATA_NETVAR:
		{
			size_t iDataLength;
			i.read((char*)&iDataLength, sizeof(iDataLength));

			CNetworkedVariableBase* pVariable = (CNetworkedVariableBase*)pData;
			char* pRealData = new char[iDataLength];
			i.read(pRealData, iDataLength);
			pVariable->Unserialize(iDataLength, pRealData);
			delete[] pRealData;
			break;
		}

		case CSaveData::DATA_STRING:
			((tstring*)pData)->assign(readstring(i));
			break;

		case CSaveData::DATA_OUTPUT:
		{
			CEntityOutput* pOutput = (CEntityOutput*)pData;
			size_t iTargets = 0;
			i.read((char*)&iTargets, sizeof(iTargets));
			for (size_t j = 0; j < iTargets; j++)
			{
				bool bKill;

				tstring sTargetName = readstring(i);
				tstring sInput = readstring(i);
				tstring sArgs = readstring(i);
				i.read((char*)&bKill, sizeof(bool));

				pOutput->AddTarget(sTargetName, sInput, sArgs, bKill);
			}
			break;
		}
		}
	}

	return true;
}

void CBaseEntity::PrecacheModel(const tstring& sModel)
{
	CEntityRegistration* pReg = &GetEntityRegistration()[GetClassName()];
	for (size_t i = 0; i < pReg->m_asPrecaches.size(); i++)
	{
		if (pReg->m_asPrecaches[i] == sModel)
			return;
	}

	if (CModelLibrary::AddModel(sModel) == ~0)
	{
		TError("Model \"" + sModel + "\" could not be loaded.");
		return;
	}

	pReg->m_asPrecaches.push_back(sModel);
}

void CBaseEntity::PrecacheParticleSystem(const tstring& sSystem)
{
	CEntityRegistration* pReg = &GetEntityRegistration()[GetClassName()];
	for (size_t i = 0; i < pReg->m_asPrecaches.size(); i++)
	{
		if (pReg->m_asPrecaches[i] == sSystem)
			return;
	}

	size_t iSystem = CParticleSystemLibrary::Get()->FindParticleSystem(sSystem);
	CParticleSystemLibrary::Get()->LoadParticleSystem(iSystem);

	pReg->m_asPrecaches.push_back(sSystem);
}

void CBaseEntity::PrecacheSound(const tstring& sSound)
{
	CEntityRegistration* pReg = &GetEntityRegistration()[GetClassName()];
	for (size_t i = 0; i < pReg->m_asPrecaches.size(); i++)
	{
		if (pReg->m_asPrecaches[i] == sSound)
			return;
	}

	CSoundLibrary::Get()->AddSound(sSound);

	pReg->m_asPrecaches.push_back(sSound);
}

void CBaseEntity::PrecacheMaterial(const tstring& sMaterial)
{
	CEntityRegistration* pReg = &GetEntityRegistration()[GetClassName()];
	for (size_t i = 0; i < pReg->m_asPrecaches.size(); i++)
	{
		if (pReg->m_asPrecaches[i] == sMaterial)
			return;
	}

	CMaterialHandle hMaterial = CMaterialLibrary::AddMaterial(sMaterial);
	if (!hMaterial.IsValid())
	{
		TError("PrecacheMaterial(): Couldn't load material " + sMaterial + "\n");
		return;
	}

	pReg->m_ahMaterialPrecaches.push_back(hMaterial);
	pReg->m_asPrecaches.push_back(sMaterial);
}

tmap<tstring, CEntityRegistration>& CBaseEntity::GetEntityRegistration()
{
	static tmap<tstring, CEntityRegistration> aEntityRegistration;
	return aEntityRegistration;
}

void CBaseEntity::RegisterEntity(const char* pszClassName, const char* pszParentClass, EntityRegisterCallback pfnRegisterCallback, EntityPrecacheCallback pfnPrecacheCallback, EntityCreateCallback pfnCreateCallback)
{
	CEntityRegistration* pEntity = &GetEntityRegistration()[pszClassName];
	pEntity->m_pszEntityClass = pszClassName;
	pEntity->m_pszParentClass = pszParentClass;
	pEntity->m_pfnRegisterCallback = pfnRegisterCallback;
	pEntity->m_pfnPrecacheCallback = pfnPrecacheCallback;
	pEntity->m_pfnCreateCallback = pfnCreateCallback;
}

void CBaseEntity::Register(CBaseEntity* pEntity)
{
	pEntity->RegisterSaveData();
	pEntity->RegisterNetworkVariables();
	pEntity->RegisterInputData();
}

void CBaseEntity::PrecacheCallback(CBaseEntity* pEntity)
{
	pEntity->Precache();
}

size_t CBaseEntity::GetNumEntitiesRegistered()
{
	return GetEntityRegistration().size();
}

CEntityRegistration* CBaseEntity::GetEntityRegistration(size_t iEntity)
{
	if (iEntity >= GetNumEntitiesRegistered())
		return nullptr;

	// Not the fastest implementation but I don't think it needs to be.
	size_t i = 0;
	for (auto it = GetEntityRegistration().begin(); it != GetEntityRegistration().end(); it++, i++)
	{
		if (i == iEntity)
			return &it->second;
	}

	TAssert(false);	 // Dunno how this could happen.
	return nullptr;
}

CSaveData* CBaseEntity::FindSaveData(const char* pszClassName, const char* pszName, bool bThisClassOnly)
{
	CEntityRegistration* pRegistration;
	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		if (!pRegistration)
			return nullptr;

		for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
		{
			CSaveData* pVarData = &pRegistration->m_aSaveData[i];

			if (strcmp(pVarData->m_pszVariableName, pszName) == 0)
				return pVarData;
		}

		if (bThisClassOnly)
			return nullptr;

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	return NULL;
}

CSaveData* CBaseEntity::FindSaveDataByHandle(const char* pszClassName, const char* pszHandle, bool bThisClassOnly)
{
	CEntityRegistration* pRegistration;
	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		if (!pRegistration)
			return nullptr;

		for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
		{
			CSaveData* pVarData = &pRegistration->m_aSaveData[i];

			if (!pVarData->m_pszHandle)
				continue;

			if (strcmp(pVarData->m_pszHandle, pszHandle) == 0)
				return pVarData;
		}

		if (bThisClassOnly)
			return nullptr;

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	return NULL;
}

CSaveData* CBaseEntity::FindSaveDataValuesByHandle(const char* pszClassName, const char* pszHandle, CSaveData* pSaveData)
{
	tvector<tstring> asParents;
	CEntityRegistration* pRegistration;

	while (pszClassName)
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		if (!pRegistration)
			return nullptr;

		asParents.push_back(pszClassName);
		pszClassName = pRegistration->m_pszParentClass;
	}

	pSaveData->m_pszHandle = nullptr;

	for (size_t i = asParents.size()-1; i < asParents.size(); i--)
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(asParents[i]);

		for (size_t j = 0; j < pRegistration->m_aSaveData.size(); j++)
		{
			CSaveData* pVarData = &pRegistration->m_aSaveData[j];

			if (!pVarData->m_pszHandle)
				continue;

			if (strcmp(pVarData->m_pszHandle, pszHandle) == 0)
			{
				if (!pSaveData->m_pszHandle)
					memcpy(pSaveData, pVarData, sizeof(*pSaveData));
				else
				{
					TAssert(pVarData->m_bOverride || pVarData->m_bShowInEditor);
					if (pVarData->m_bShowInEditor)
						pSaveData->m_bShowInEditor = true;
					if (pVarData->m_bOverride && pVarData->m_bDefault)
					{
						pSaveData->m_bDefault = true;
						memcpy(pSaveData->m_oDefault, pVarData->m_oDefault, sizeof(pSaveData->m_oDefault));
					}
				}
			}
		}
	}

	return pSaveData;
}

CSaveData* CBaseEntity::FindOutput(const char* pszClassName, const tstring& sOutput, bool bThisClassOnly)
{
	return FindSaveData(pszClassName, ("m_Output_" + sOutput).c_str(), bThisClassOnly);
}

CEntityRegistration* CBaseEntity::GetRegisteredEntity(tstring sClassName)
{
	if (GetEntityRegistration().find(sClassName) == GetEntityRegistration().end())
		return NULL;

	return &GetEntityRegistration()[sClassName];
}

CBaseEntity* CBaseEntity::GetEntityByName(const tstring& sName)
{
	if (sName.length() == 0)
		return NULL;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		if (sName[0] == '*')
		{
			if (tstring(pEntity->GetClassName()+1) == sName.c_str()+1)
				return pEntity;
		}
		else
		{
			if (pEntity->GetName() == sName)
				return pEntity;
		}
	}

	return NULL;
}

void CBaseEntity::FindEntitiesByName(const tstring& sName, tvector<CBaseEntity*>& apEntities)
{
	if (sName.length() == 0)
		return;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		if (pEntity->IsDeleted())
			continue;

		if (sName[0] == '*')
		{
			if (tstring(pEntity->GetClassName()+1) != sName.c_str()+1)
				continue;
		}
		else
		{
			if (pEntity->GetName() != sName)
				continue;
		}

		apEntities.push_back(pEntity);
	}
}

bool CanUnserializeString_bool(const tstring& sData)
{
	return true;
}

bool UnserializeString_bool(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	return (sData.comparei("yes") == 0 || sData.comparei("true") == 0 || sData.comparei("on") == 0 || stoi(sData) != 0);
}

bool CanUnserializeString_size_t(const tstring& sData)
{
	return true;
}

size_t UnserializeString_size_t(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	return stoi(sData);
}

bool CanUnserializeString_TVector(const tstring& sData)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	return asTokens.size() == 3;
}

const TVector UnserializeString_TVector(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 3);
	if (asTokens.size() != 3)
	{
		TError("Entity '" + sName + "' (" + sClass + ":" + sHandle + ") wrong number of arguments for a vector (Format: \"x y z\")\n");
		return TVector();
	}

	return Vector((float)stof(asTokens[0]), (float)stof(asTokens[1]), (float)stof(asTokens[2]));
}

bool CanUnserializeString_Vector2D(const tstring& sData)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	return asTokens.size() == 2;
}

const Vector2D UnserializeString_Vector2D(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 2);
	if (asTokens.size() != 2)
	{
		TError("Entity '" + sName + "' (" + sClass + ":" + sHandle + ") wrong number of arguments for a 2D vector (Format: \"x y\")\n");
		return Vector2D();
	}

	return Vector2D((float)stof(asTokens[0]), (float)stof(asTokens[1]));
}

bool CanUnserializeString_EAngle(const tstring& sData)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	return asTokens.size() == 3;
}

const EAngle UnserializeString_EAngle(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 3);
	if (asTokens.size() != 3)
	{
		TError("Entity '" + sName + "' (" + sClass + ":" + sHandle + ") wrong number of arguments for an angle (Format: \"p y r\")\n");
		return EAngle();
	}

	return EAngle((float)stof(asTokens[0]), (float)stof(asTokens[1]), (float)stof(asTokens[2]));
}

bool CanUnserializeString_AABB(const tstring& sData)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	return asTokens.size() == 6;
}

const AABB UnserializeString_AABB(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 6);
	if (asTokens.size() != 6)
	{
		TError("Entity '" + sName + "' (" + sClass + ":" + sHandle + ") wrong number of arguments for an AABB (Format: \"x y z x y z\")\n");
		return AABB();
	}

	AABB aabbReturn(Vector((float)stof(asTokens[0]), (float)stof(asTokens[1]), (float)stof(asTokens[2])), Vector((float)stof(asTokens[3]), (float)stof(asTokens[4]), (float)stof(asTokens[5])));

	if (aabbReturn.m_vecMins.x > aabbReturn.m_vecMaxs.x)
		std::swap(aabbReturn.m_vecMins.x, aabbReturn.m_vecMaxs.x);
	if (aabbReturn.m_vecMins.y > aabbReturn.m_vecMaxs.y)
		std::swap(aabbReturn.m_vecMins.y, aabbReturn.m_vecMaxs.y);
	if (aabbReturn.m_vecMins.z > aabbReturn.m_vecMaxs.z)
		std::swap(aabbReturn.m_vecMins.z, aabbReturn.m_vecMaxs.z);

	return aabbReturn;
}

bool CanUnserializeString_Matrix4x4(const tstring& sData)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	return asTokens.size() == 6;
}

const Matrix4x4 UnserializeString_Matrix4x4(const tstring& sData, const tstring& sName, const tstring& sClass, const tstring& sHandle)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 6);
	if (asTokens.size() != 6)
	{
		TError("Entity '" + sName + "' (" + sClass + ":" + sHandle + ") wrong number of arguments for a matrix (Format: \"x y z p y r\")\n");
		return Matrix4x4();
	}

	Vector vecData((float)stof(asTokens[0]), (float)stof(asTokens[1]), (float)stof(asTokens[2]));
	EAngle angData((float)stof(asTokens[3]), (float)stof(asTokens[4]), (float)stof(asTokens[5]));

	return Matrix4x4(angData, vecData);
}

void UnserializeString_bool(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	bool bValue = UnserializeString_bool(sData);

	bool* pData = (bool*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = bValue;
		break;

	case CSaveData::DATA_NETVAR:
	{
		CNetworkedVariable<bool>* pVariable = (CNetworkedVariable<bool>*)pData;
		(*pVariable) = bValue;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_OUTPUT:
		TUnimplemented();
		break;
	}
}

void UnserializeString_int(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	TUnimplemented();
}

void UnserializeString_size_t(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	TUnimplemented();

	size_t i = UnserializeString_size_t(sData);

	size_t* pData = (size_t*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = i;
		break;

	case CSaveData::DATA_NETVAR:
	{
		CNetworkedVariable<size_t>* pVariable = (CNetworkedVariable<size_t>*)pData;
		(*pVariable) = i;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_OUTPUT:
		TUnimplemented();
		break;
	}
}

void UnserializeString_float(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	float f = (float)stof(sData);

	float* pData = (float*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = f;
		break;

	case CSaveData::DATA_NETVAR:
	{
		TUnimplemented();
		CNetworkedVariable<float>* pVariable = (CNetworkedVariable<float>*)pData;
		(*pVariable) = f;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_OUTPUT:
		TUnimplemented();
		break;
	}
}

void UnserializeString_double(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	double f = stof(sData);

	double* pData = (double*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = f;
		break;

	case CSaveData::DATA_NETVAR:
	{
		TUnimplemented();
		CNetworkedVariable<double>* pVariable = (CNetworkedVariable<double>*)pData;
		(*pVariable) = f;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_OUTPUT:
		TUnimplemented();
		break;
	}
}

void UnserializeString_tstring(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	tstring* psData = (tstring*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_STRING:
		*psData = sData;
		break;

	case CSaveData::DATA_NETVAR:
	{
		TUnimplemented();
		CNetworkedString* pVariable = (CNetworkedString*)psData;
		(*pVariable) = sData;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_COPYTYPE:
	case CSaveData::DATA_OUTPUT:
		TUnimplemented();
		break;
	}
}

void UnserializeString_TVector(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	Vector vecData = UnserializeString_TVector(sData, pEntity->GetName(), pEntity->GetClassName(), pSaveData->m_pszHandle);

	Vector* pData = (Vector*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		TUnimplemented();
		*pData = vecData;
		break;

	case CSaveData::DATA_NETVAR:
	{
		CNetworkedVector* pVariable = (CNetworkedVector*)pData;
		(*pVariable) = vecData;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_OUTPUT:
		TUnimplemented();
		break;
	}
}

void UnserializeString_Vector(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	UnserializeString_TVector(sData, pSaveData, pEntity);
}

void UnserializeString_Vector2D(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	Vector2D vecData = UnserializeString_Vector2D(sData, pEntity->GetName(), pEntity->GetClassName(), pSaveData->m_pszHandle);

	Vector2D* pData = (Vector2D*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		TUnimplemented();
		*pData = vecData;
		break;

	case CSaveData::DATA_NETVAR:
	{
		CNetworkedVariable<Vector2D>* pVariable = (CNetworkedVariable<Vector2D>*)pData;
		(*pVariable) = vecData;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_OUTPUT:
		TUnimplemented();
		break;
	}
}

void UnserializeString_EAngle(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	TUnimplemented();
}

void UnserializeString_Matrix4x4(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	Matrix4x4 mData = UnserializeString_Matrix4x4(sData, pEntity->GetName(), pEntity->GetClassName(), pSaveData->m_pszHandle);

	Matrix4x4* pData = (Matrix4x4*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = mData;
		break;

	case CSaveData::DATA_NETVAR:
	{
		TUnimplemented();
		CNetworkedVariable<Matrix4x4>* pVariable = (CNetworkedVariable<Matrix4x4>*)pData;
		(*pVariable) = mData;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_OUTPUT:
		TUnimplemented();
		break;
	}
}

void UnserializeString_AABB(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 6);
	if (asTokens.size() != 6)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") wrong number of arguments for an AABB (Format: \"x y z x y z\")\n");
		return;
	}

	AABB aabbData(Vector((float)stof(asTokens[0]), (float)stof(asTokens[1]), (float)stof(asTokens[2])), Vector((float)stof(asTokens[3]), (float)stof(asTokens[4]), (float)stof(asTokens[5])));

	if (aabbData.m_vecMins.x > aabbData.m_vecMaxs.x)
		std::swap(aabbData.m_vecMins.x, aabbData.m_vecMaxs.x);
	if (aabbData.m_vecMins.y > aabbData.m_vecMaxs.y)
		std::swap(aabbData.m_vecMins.y, aabbData.m_vecMaxs.y);
	if (aabbData.m_vecMins.z > aabbData.m_vecMaxs.z)
		std::swap(aabbData.m_vecMins.z, aabbData.m_vecMaxs.z);

	// Center the entity around this bounding box.
	Vector vecGlobalOrigin = aabbData.Center();
	aabbData.m_vecMins -= vecGlobalOrigin;
	aabbData.m_vecMaxs -= vecGlobalOrigin;
	pEntity->SetGlobalOrigin(pEntity->GetGlobalOrigin() + vecGlobalOrigin);

	AABB* pData = (AABB*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = aabbData;
		break;

	case CSaveData::DATA_NETVAR:
	{
		TUnimplemented();
		CNetworkedVariable<AABB>* pVariable = (CNetworkedVariable<AABB>*)pData;
		(*pVariable) = aabbData;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_OUTPUT:
		TUnimplemented();
		break;
	}
}

void UnserializeString_ModelID(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	CMaterialHandle hMaterial = CMaterialLibrary::AddMaterial(sData);

	if (hMaterial.IsValid())
	{
		pEntity->SetMaterialModel(hMaterial);
		return;
	}

	size_t iID = CModelLibrary::AddModel(sData);

	TAssert(iID != ~0);
	if (iID == ~0)
	{
		if (pSaveData->m_pszHandle)
			TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") couldn't find or load model '" + sData + "'\n");
		else
			TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszVariableName + ") couldn't find or load model '" + sData + "'\n");
		return;
	}

	pEntity->SetModel(iID);
}

void UnserializeString_EntityHandle(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	CBaseEntity* pNamedEntity = CBaseEntity::GetEntityByName(sData);

	TAssert(pNamedEntity);
	if (!pNamedEntity)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") couldn't find entity named '" + sData + "'\n");
		return;
	}

	CEntityHandle<CBaseEntity> hData(pNamedEntity);

	CEntityHandle<CBaseEntity>* pData = (CEntityHandle<CBaseEntity>*)((char*)pEntity + pSaveData->m_iOffset);
	switch(pSaveData->m_eType)
	{
	case CSaveData::DATA_COPYTYPE:
		*pData = hData;
		break;

	case CSaveData::DATA_NETVAR:
	{
		CNetworkedHandle<CBaseEntity>* pVariable = (CNetworkedHandle<CBaseEntity>*)pData;
		(*pVariable) = hData;
		break;
	}

	case CSaveData::DATA_COPYARRAY:
	case CSaveData::DATA_COPYVECTOR:
	case CSaveData::DATA_STRING:
	case CSaveData::DATA_OUTPUT:
		TUnimplemented();
		break;
	}
}

void UnserializeString_LocalOrigin(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	Vector vecData = UnserializeString_TVector(sData, pEntity->GetName(), pEntity->GetClassName(), pSaveData->m_pszHandle);
	pEntity->SetLocalOrigin(vecData);
}

void UnserializeString_LocalAngles(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	tvector<tstring> asTokens;
	tstrtok(sData, asTokens);

	TAssert(asTokens.size() == 3);
	if (asTokens.size() != 3)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") wrong number of arguments for a vector\n");
		return;
	}

	EAngle angData((float)stof(asTokens[0]), (float)stof(asTokens[1]), (float)stof(asTokens[2]));
	pEntity->SetLocalAngles(angData);
}

void UnserializeString_MoveParent(const tstring& sData, CSaveData* pSaveData, CBaseEntity* pEntity)
{
	CBaseEntity* pNamedEntity = CBaseEntity::GetEntityByName(sData);

	TAssert(pNamedEntity);
	if (!pNamedEntity)
	{
		TError("Entity '" + pEntity->GetName() + "' (" + pEntity->GetClassName() + ":" + pSaveData->m_pszHandle + ") couldn't find entity named '" + sData + "'\n");
		return;
	}

	pEntity->SetMoveParent(pNamedEntity);
}

CSaveData::CSaveData()
{
	m_eType = DATA_OMIT;
	m_pszVariableName = m_pszType = m_pszHandle = "";
	m_pfnUnserializeString = nullptr;
	m_pfnResizeVector = nullptr;
	m_bOverride = false;
	m_bShowInEditor = false;
	m_bDefault = false;
	m_iOffset = m_iSizeOfVariable = m_iSizeOfType = 0;
}
