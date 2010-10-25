#include "baseentity.h"

#include <strutils.h>
#include <mtrand.h>

#include <raytracer/raytracer.h>
#include <models/models.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <sound/sound.h>

#include "game.h"

std::map<size_t, CBaseEntity*> CBaseEntity::s_apEntityList;
size_t CBaseEntity::s_iOverrideEntityListIndex = ~0;
size_t CBaseEntity::s_iNextEntityListIndex = 0;

NETVAR_TABLE_BEGIN_NOBASE(CBaseEntity);
	NETVAR_DEFINE(Vector, m_vecOrigin);
	NETVAR_DEFINE(EAngle, m_angAngles);
	NETVAR_DEFINE(Vector, m_vecVelocity);
	NETVAR_DEFINE(Vector, m_vecGravity);
	NETVAR_DEFINE(bool, m_bTakeDamage);
	NETVAR_DEFINE(float, m_flTotalHealth);
	NETVAR_DEFINE(float, m_flHealth);
	NETVAR_DEFINE(int, m_iCollisionGroup);
	NETVAR_DEFINE(size_t, m_iModel);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBaseEntity);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecOrigin);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecLastOrigin);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, EAngle, m_angAngles);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecVelocity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecGravity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bSimulated);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bTakeDamage);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flTotalHealth);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flHealth);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flTimeKilled);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CTeam>, m_hTeam);
	//SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bDeleted);	// Deleted entities are not saved.
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CBaseEntity>, m_ahTouching);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, int, m_iCollisionGroup);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iModel);
SAVEDATA_TABLE_END();

CBaseEntity::CBaseEntity()
{
	if (s_iOverrideEntityListIndex == ~0)
		m_iHandle = s_iNextEntityListIndex++;
	else
	{
		m_iHandle = s_iOverrideEntityListIndex;

		if (s_iNextEntityListIndex < m_iHandle+1)
			s_iNextEntityListIndex = m_iHandle+1;
	}

	s_apEntityList[m_iHandle] = this;

	m_iCollisionGroup = 0;
	m_pTracer = NULL;

	m_bTakeDamage = false;
	m_flTotalHealth = 1;
	m_flHealth = 1;
	m_flTimeKilled = 0;

	m_bDeleted = false;

	m_bSimulated = false;

	m_iModel = ~0;

	m_iSpawnSeed = 0;
}

CBaseEntity::~CBaseEntity()
{
	s_apEntityList.erase(s_apEntityList.find(m_iHandle));

	if (m_pTracer)
		delete m_pTracer;
}

void CBaseEntity::SetModel(const wchar_t* pszModel)
{
	SetModel(CModelLibrary::Get()->FindModel(pszModel));
}

void CBaseEntity::SetModel(size_t iModel)
{
	m_iModel = iModel;

	if (m_iModel.Get() == ~0)
		return;

	if (UsesRaytracedCollision())
	{
		if (m_pTracer)
			delete m_pTracer;

		m_pTracer = new raytrace::CRaytracer(CModelLibrary::Get()->GetModel(m_iModel)->m_pScene);
		m_pTracer->AddMeshesFromNode(CModelLibrary::Get()->GetModel(m_iModel)->m_pScene->GetScene(0));
		m_pTracer->BuildTree();
	}
}

CBaseEntity* CBaseEntity::GetEntity(size_t iHandle)
{
	if (s_apEntityList.find(iHandle) == s_apEntityList.end())
		return NULL;

	return s_apEntityList[iHandle];
}

size_t CBaseEntity::GetEntityHandle(size_t i)
{
	if (!s_apEntityList.size())
		return ~0;

	if (i > s_apEntityList.size())
		return ~0;

	static std::map<size_t, CBaseEntity*>::iterator it;
	static size_t iLastRequest = ~0;

	size_t iThisRequest = i;

	// Perf. Most GetEntityHandle requests come in sequentially,
	// so saving the last iterator can save O(n!) search time.
	if (iLastRequest == ~0 || iLastRequest != i-1)
	{
		it = s_apEntityList.begin();

		while (i--)
			it++;
	}
	else
		it++;

	iLastRequest = iThisRequest;

	return (*it).first;
}

CBaseEntity* CBaseEntity::GetEntityNumber(size_t i)
{
	return GetEntity(GetEntityHandle(i));
}

size_t CBaseEntity::GetNumEntities()
{
	return s_apEntityList.size();
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
	CNetwork::CallFunction(iClient, "SetAngles", GetHandle(), GetAngles().p, GetAngles().y, GetAngles().r);
}

void CBaseEntity::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit)
{
	if (!m_bTakeDamage)
		return;

	bool bWasAlive = IsAlive();

	if (CNetwork::IsHost())
		m_flHealth -= flDamage;

	Game()->OnTakeDamage(this, pAttacker, pInflictor, flDamage, bDirectHit, !IsAlive() && bWasAlive);

	if (bWasAlive && m_flHealth <= 0)
		Killed(pAttacker);
}

void CBaseEntity::Killed(CBaseEntity* pKilledBy)
{
	m_flTimeKilled = GameServer()->GetGameTime();

	OnKilled(pKilledBy);
	Game()->OnKilled(this);
}

void CBaseEntity::Render()
{
	PreRender();

	do {
		CRenderingContext r(GameServer()->GetRenderer());
		r.Translate(GetRenderOrigin());

		EAngle angRender = GetRenderAngles();
		Vector vecForward, vecRight, vecUp;

		AngleVectors(angRender, &vecForward, &vecRight, &vecUp);

		// These first two aren't tested.
		//r.Rotate(-angRender.r, vecForward);
		//r.Rotate(-angRender.p, vecRight);
		r.Rotate(-angRender.y, Vector(0, 1, 0));

		ModifyContext(&r);

		if (r.GetAlpha() > 0)
		{
			if (m_iModel != (size_t)~0)
				r.RenderModel(GetModel());

			OnRender();
		}
	} while (false);

	PostRender();
}

void CBaseEntity::Delete()
{
	GameServer()->Delete(this);
}

void CBaseEntity::EmitSound(const char* pszFilename)
{
	CSoundLibrary::PlaySound(this, pszFilename);
}

void CBaseEntity::StopSound(const char* pszFilename)
{
	CSoundLibrary::StopSound(this, pszFilename);
}

bool CBaseEntity::IsSoundPlaying(const char* pszFilename)
{
	return CSoundLibrary::IsSoundPlaying(this, pszFilename);
}

void CBaseEntity::SetSoundVolume(const char* pszFilename, float flVolume)
{
	CSoundLibrary::SetSoundVolume(this, pszFilename, flVolume);
}

float CBaseEntity::Distance(Vector vecSpot)
{
	float flDistance = (GetOrigin() - vecSpot).Length();
	if (flDistance < GetBoundingRadius())
		return 0;

	return flDistance - GetBoundingRadius();
}

bool CBaseEntity::Collide(const Vector& v1, const Vector& v2, Vector& vecPoint)
{
	if (m_pTracer)
	{
		// Got to translate from world space to object space and back again. The collision mesh is in object space!
		Matrix4x4 m;
		m.SetTranslation(GetOrigin());
		m.SetRotation(GetAngles());

		Matrix4x4 i(m);
		i.InvertTR();

		raytrace::CTraceResult tr;
		bool bHit = m_pTracer->Raytrace(i*v1, i*v2, &tr);
		if (bHit)
			vecPoint = m*tr.m_vecHit;
		return bHit;
	}

	if (GetBoundingRadius() == 0)
		return false;

	return LineSegmentIntersectsSphere(v1, v2, GetOrigin(), GetBoundingRadius(), vecPoint);
}

void CBaseEntity::SetSpawnSeed(size_t iSpawnSeed)
{
	m_iSpawnSeed = iSpawnSeed;

	mtsrand(iSpawnSeed);
}

void CBaseEntity::RegisterNetworkVariable(CNetworkedVariableBase* pVariable)
{
	assert(m_apNetworkVariables.find(pVariable->GetName()) == m_apNetworkVariables.end());

	CNetwork::RegisterNetworkVariable(pVariable);

	m_apNetworkVariables[pVariable->GetName()] = pVariable;
}

void CBaseEntity::DeregisterNetworkVariables()
{
	static std::map<std::string, CNetworkedVariableBase*>::iterator it;

	it = m_apNetworkVariables.begin();

	while (it != m_apNetworkVariables.end())
	{
		DeregisterNetworkVariable(it->second);

		it++;
	}
}

void CBaseEntity::DeregisterNetworkVariable(CNetworkedVariableBase* pVariable)
{
	CNetwork::DeregisterNetworkVariable(pVariable);

	// Don't bother removing it from the entity list, this entity is about to die anyway.
}

CNetworkedVariableBase* CBaseEntity::GetNetworkVariable(const char* pszName)
{
	return m_apNetworkVariables[pszName];
}

void CBaseEntity::GameLoaded()
{
	SetModel(m_iModel);
}

void CBaseEntity::SerializeEntity(std::ostream& o, CBaseEntity* pEntity)
{
	size_t iRegistration = FindRegisteredEntity(pEntity->GetClassName());
	o.write((char*)&iRegistration, sizeof(iRegistration));

	size_t iHandle = pEntity->GetHandle();
	o.write((char*)&iHandle, sizeof(iHandle));

	size_t iSpawnSeed = pEntity->GetSpawnSeed();
	o.write((char*)&iSpawnSeed, sizeof(iSpawnSeed));

	pEntity->Serialize(o);
	pEntity->OnSerialize(o);
}

bool CBaseEntity::UnserializeEntity(std::istream& i)
{
	size_t iRegistration;
	i.read((char*)&iRegistration, sizeof(iRegistration));

	size_t iHandle;
	i.read((char*)&iHandle, sizeof(iHandle));

	size_t iSpawnSeed;
	i.read((char*)&iSpawnSeed, sizeof(iSpawnSeed));

	size_t iNewHandle = GameServer()->CreateEntity(iRegistration, iHandle, iSpawnSeed);
	assert(iNewHandle == iHandle);

	CEntityHandle<CBaseEntity> hEntity(iNewHandle);

	if (!hEntity->Unserialize(i))
		return false;

	return hEntity->OnUnserialize(i);
}

void CBaseEntity::Serialize(std::ostream& o, const char* pszClassName, void* pEntity)
{
	size_t iEntity = CBaseEntity::FindRegisteredEntity(pszClassName);
	CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity(iEntity);

	size_t iSaveDataSize = pRegistration->m_aSaveData.size();
	o.write((char*)&iSaveDataSize, sizeof(iSaveDataSize));

	for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
	{
		CSaveData* pSaveData = &pRegistration->m_aSaveData[i];

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
			std::vector<size_t>* pVector = (std::vector<size_t>*)pData;
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

		case CSaveData::DATA_WSTRING:
			writewstring(o, *(std::wstring*)pData);
			break;
		}
	}
}

bool CBaseEntity::Unserialize(std::istream& i, const char* pszClassName, void* pEntity)
{
	size_t iEntity = CBaseEntity::FindRegisteredEntity(pszClassName);
	CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity(iEntity);

	size_t iSaveDataSize;
	i.read((char*)&iSaveDataSize, sizeof(iSaveDataSize));

	for (size_t j = 0; j < iSaveDataSize; j++)
	{
		size_t iSaveData;
		i.read((char*)&iSaveData, sizeof(iSaveData));

		CSaveData* pSaveData = &pRegistration->m_aSaveData[iSaveData];

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
			std::vector<size_t>* pVector = (std::vector<size_t>*)pData;
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
			pVariable->Unserialize(pRealData);
			delete[] pRealData;
			break;
		}

		case CSaveData::DATA_WSTRING:
			((std::wstring*)pData)->assign(readwstring(i));
			break;
		}
	}

	return true;
}

void CBaseEntity::PrecacheModel(const wchar_t* pszModel, bool bStatic)
{
	CModelLibrary::Get()->AddModel(pszModel, bStatic);
}

void CBaseEntity::PrecacheParticleSystem(const wchar_t* pszParticleSystem)
{
	size_t iSystem = CParticleSystemLibrary::Get()->FindParticleSystem(pszParticleSystem);
	CParticleSystemLibrary::Get()->LoadParticleSystem(iSystem);
}

void CBaseEntity::PrecacheSound(const char* pszSound)
{
	CSoundLibrary::Get()->AddSound(pszSound);
}

void CBaseEntity::RegisterEntity(const char* pszEntityName, EntityCreateCallback pfnCreateCallback, EntityRegisterCallback pfnRegisterCallback)
{
	s_aEntityRegistration.push_back(CEntityRegistration());
	CEntityRegistration* pEntity = &s_aEntityRegistration[s_aEntityRegistration.size()-1];
	pEntity->m_pszEntityName = pszEntityName;
	pEntity->m_pfnCreateCallback = pfnCreateCallback;
	pEntity->m_pfnRegisterCallback = pfnRegisterCallback;
}

void CBaseEntity::Register(CBaseEntity* pEntity)
{
	pEntity->Precache();
	pEntity->RegisterSaveData();
}

size_t CBaseEntity::FindRegisteredEntity(const char* pszEntityName)
{
	for (size_t i = 0; i < CBaseEntity::s_aEntityRegistration.size(); i++)
	{
		if (strcmp(CBaseEntity::s_aEntityRegistration[i].m_pszEntityName, pszEntityName) == 0)
		{
			return i;
		}
	}
	return ~0;
}

CEntityRegistration* CBaseEntity::GetRegisteredEntity(size_t iEntity)
{
	return &s_aEntityRegistration[iEntity];
}
