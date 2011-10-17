#include "baseentity.h"

#include <strutils.h>
#include <mtrand.h>

#include <raytracer/raytracer.h>
#include <models/models.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <sound/sound.h>
#include <tinker/application.h>
#include <tinker/profiler.h>
#include <network/commands.h>

#include "game.h"

#ifdef _WIN32
// The linker throws out objects in a .lib that aren't referenced by the .exe, so we need to force some of our registrations to be imported when we link.
#pragma comment(linker, "/include:?g_RegisterCCounter@@3VCRegisterCCounter@@A")
#endif

eastl::vector<CBaseEntity*> CBaseEntity::s_apEntityList;
size_t CBaseEntity::s_iEntities = 0;
size_t CBaseEntity::s_iOverrideEntityListIndex = ~0;
size_t CBaseEntity::s_iNextEntityListIndex = 0;

REGISTER_ENTITY(CBaseEntity);

NETVAR_TABLE_BEGIN(CBaseEntity);
	NETVAR_DEFINE_INTERVAL(Vector, m_vecOrigin, 0.15f);
	NETVAR_DEFINE_INTERVAL(EAngle, m_angAngles, 0.15f);
	NETVAR_DEFINE_INTERVAL(Vector, m_vecVelocity, 0.15f);
	NETVAR_DEFINE(Vector, m_vecGravity);
	NETVAR_DEFINE(bool, m_bTakeDamage);
	NETVAR_DEFINE(float, m_flTotalHealth);
	NETVAR_DEFINE(float, m_flHealth);
	NETVAR_DEFINE(bool, m_bActive);
	NETVAR_DEFINE(CEntityHandle<CBaseEntity>, m_hTeam);
	NETVAR_DEFINE(int, m_iCollisionGroup);
	NETVAR_DEFINE(size_t, m_iModel);
	NETVAR_DEFINE(float, m_flSpawnTime);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBaseEntity);
	SAVEDATA_DEFINE_OUTPUT(OnSpawn);
	SAVEDATA_DEFINE_OUTPUT(OnTakeDamage);
	SAVEDATA_DEFINE_OUTPUT(OnKilled);
	SAVEDATA_DEFINE_OUTPUT(OnActivated);
	SAVEDATA_DEFINE_OUTPUT(OnDeactivated);
	SAVEDATA_DEFINE(CSaveData::DATA_STRING, eastl::string, m_sName);
	SAVEDATA_DEFINE(CSaveData::DATA_STRING, tstring, m_sClassName);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecOrigin);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecLastOrigin);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, EAngle, m_angAngles);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecVelocity);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecGravity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iHandle);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bTakeDamage);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flTotalHealth);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flHealth);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flTimeKilled);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bActive);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CTeam>, m_hTeam);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bSimulated);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, bool, m_bDeleted);	// Deleted entities are not saved.
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bClientSpawn);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CBaseEntity>, m_ahTouching);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_iCollisionGroup);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, class raytrace::CRaytracer*, m_pTracer);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iModel);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iSpawnSeed);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flSpawnTime);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBaseEntity);
	INPUT_DEFINE(Activate);
	INPUT_DEFINE(Deactivate);
	INPUT_DEFINE(ToggleActive);
	INPUT_DEFINE(RemoveOutput);
	INPUT_DEFINE(Delete);
INPUTS_TABLE_END();

CBaseEntity::CBaseEntity()
{
	if (s_iOverrideEntityListIndex == ~0)
		m_iHandle = s_iNextEntityListIndex;
	else
		m_iHandle = s_iOverrideEntityListIndex;

	s_iNextEntityListIndex = (m_iHandle+1)%s_apEntityList.size();
	while (s_apEntityList[s_iNextEntityListIndex] != NULL)
	{
		s_iNextEntityListIndex = (s_iNextEntityListIndex+1)%s_apEntityList.size();
	}

	s_apEntityList[m_iHandle] = this;

	s_iEntities++;

	m_iCollisionGroup = 0;
	m_pTracer = NULL;

	m_bTakeDamage = false;
	m_flTotalHealth = 1;
	m_flHealth = 1;
	m_flTimeKilled = 0;

	m_bDeleted = false;
	m_bActive = true;
	m_bSimulated = false;

	m_iModel = ~0;

	m_iSpawnSeed = 0;

	m_bClientSpawn = false;
}

CBaseEntity::~CBaseEntity()
{
	s_apEntityList[m_iHandle] = NULL;

	TAssert(s_iEntities > 0);
	s_iEntities--;

	if (m_pTracer)
		delete m_pTracer;
}

void CBaseEntity::Spawn()
{
}

void CBaseEntity::SetModel(const tstring& sModel)
{
	SetModel(CModelLibrary::Get()->FindModel(sModel));
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

void CBaseEntity::SetOrigin(const Vector& vecOrigin)
{
	if ((vecOrigin - m_vecOrigin).LengthSqr() == 0)
		return;

	OnSetOrigin(vecOrigin);
	m_vecOrigin = vecOrigin;
};

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

	bool bWasAlive = IsAlive();

	if (GameNetwork()->IsHost())
		m_flHealth -= flDamage;

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
	Game()->OnKilled(this);

	CallOutput("OnKilled");
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

void CBaseEntity::Activate(const eastl::vector<tstring>& sArgs)
{
	SetActive(true);
}

void CBaseEntity::Deactivate(const eastl::vector<tstring>& sArgs)
{
	SetActive(false);
}

void CBaseEntity::ToggleActive(const eastl::vector<tstring>& sArgs)
{
	SetActive(!IsActive());
}

void CBaseEntity::Render(bool bTransparent) const
{
	TPROF("CBaseEntity::Render");

	PreRender(bTransparent);

	do {
		CRenderingContext r(GameServer()->GetRenderer());
		r.Translate(GetRenderOrigin());

		EAngle angRender = GetRenderAngles();

		r.Rotate(-angRender.y, Vector(0, 1, 0));
		r.Rotate(angRender.p, Vector(0, 0, 1));
		r.Rotate(angRender.r, Vector(1, 0, 0));

		ModifyContext(&r, bTransparent);

		if (r.GetAlpha() > 0)
		{
			if (ShouldRenderModel() && m_iModel != (size_t)~0)
			{
				if (r.GetBlend() == BLEND_NONE && !bTransparent)
				{
					TPROF("CRenderingContext::RenderModel(Opaque)");
					r.RenderModel(GetModel());
				}
				if (r.GetBlend() != BLEND_NONE && bTransparent)
				{
					TPROF("CRenderingContext::RenderModel(Transparent)");
					r.RenderModel(GetModel());
				}
			}

			TPROF("CBaseEntity::OnRender");
			OnRender(&r, bTransparent);
		}
	} while (false);

	PostRender(bTransparent);
}

void CBaseEntity::Delete()
{
	GameServer()->Delete(this);
}

void CBaseEntity::Delete(const eastl::vector<tstring>& sArgs)
{
	Delete();
}

void CBaseEntity::CallInput(const eastl::string& sName, const tstring& sArgs)
{
	CEntityInput* pInput = GetInput(sName.c_str());

	if (!pInput)
	{
		TAssert(!"Input missing.");
		TMsg(sprintf(tstring("Input %s not found in %s\n"), convertstring<char, tchar>(sName).c_str(), convertstring<char, tchar>(GetClassName()).c_str()));
		return;
	}

	eastl::vector<tstring> asArgs;
	tstrtok(sArgs, asArgs);
	pInput->m_pfnCallback(this, asArgs);
}

void CBaseEntity::CallOutput(const eastl::string& sName)
{
	CSaveData* pData = GetSaveData((eastl::string("m_Output_") + sName).c_str());

	if (!pData)
	{
		TAssert(!"Output missing.");
		TMsg(sprintf(tstring("Called nonexistant output %s of entity %s\n"), convertstring<char, tchar>(sName).c_str(), convertstring<char, tchar>(GetClassName()).c_str()));
		return;
	}

	CEntityOutput* pOutput = (CEntityOutput*)((size_t)this + (size_t)pData->m_iOffset);
	pOutput->Call();
}

void CBaseEntity::AddOutputTarget(const eastl::string& sName, const eastl::string& sTargetName, const eastl::string& sInput, const eastl::string& sArgs, bool bKill)
{
	CSaveData* pData = GetSaveData((eastl::string("m_Output_") + sName).c_str());

	if (!pData)
	{
		TAssert(!"Output missing.");
		TMsg(sprintf(tstring("Called nonexistant output %s of entity %s\n"), convertstring<char, tchar>(sName).c_str(), convertstring<char, tchar>(GetClassName()).c_str()));
		return;
	}

	CEntityOutput* pOutput = (CEntityOutput*)((size_t)this + (size_t)pData->m_iOffset);
	pOutput->AddTarget(sTargetName, sInput, sArgs, bKill);
}

void CBaseEntity::RemoveOutputs(const eastl::string& sName)
{
	CSaveData* pData = GetSaveData((eastl::string("m_Output_") + sName).c_str());

	if (!pData)
	{
		TAssert(!"Output missing.");
		TMsg(sprintf(tstring("Called nonexistant output %s of entity %s\n"), convertstring<char, tchar>(sName).c_str(), convertstring<char, tchar>(GetClassName()).c_str()));
		return;
	}

	CEntityOutput* pOutput = (CEntityOutput*)((size_t)this + (size_t)pData->m_iOffset);
	pOutput->Clear();
}

void CBaseEntity::RemoveOutput(const eastl::vector<tstring>& sArgs)
{
	if (sArgs.size() == 0)
	{
		TMsg("RemoveOutput called without a output name argument.\n");
		return;
	}

	RemoveOutputs(convertstring<tchar, char>(sArgs[0]));
}

void CEntityOutput::Call()
{
	if (m_aTargets.size() == 0)
		return;

	for (size_t j = 0; j < m_aTargets.size(); j++)
	{
		CEntityOutputTarget* pTarget = &m_aTargets[j];

		if (pTarget->m_sTargetName.length() == 0)
			continue;

		if (pTarget->m_sInput.length() == 0)
			continue;

		eastl::vector<CBaseEntity*> apEntities;
		CBaseEntity::FindEntitiesByName(pTarget->m_sTargetName, apEntities);

		for (size_t i = 0; i < apEntities.size(); i++)
			apEntities[i]->CallInput(pTarget->m_sInput, convertstring<char, tchar>(pTarget->m_sArgs));
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

void CEntityOutput::AddTarget(const eastl::string& sTargetName, const eastl::string& sInput, const eastl::string& sArgs, bool bKill)
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

float CBaseEntity::Distance(Vector vecSpot) const
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

SERVER_GAME_COMMAND(ClientSpawn)
{
	if (pCmd->GetNumArguments() < 1)
	{
		TMsg("ClientSpawn with no arguments.\n");
		return;
	}

	CEntityHandle<CBaseEntity> hEntity(pCmd->ArgAsUInt(0));

	if (hEntity == NULL)
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

CSaveData* CBaseEntity::GetSaveData(const char* pszName)
{
	const tchar* pszClassName = GetClassName();
	CEntityRegistration* pRegistration = NULL;
	
	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
		{
			CSaveData* pVarData = &pRegistration->m_aSaveData[i];

			if (strcmp(pVarData->m_pszVariableName, pszName) == 0)
				return pVarData;
		}

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	return NULL;
}

CNetworkedVariableData* CBaseEntity::GetNetworkVariable(const char* pszName)
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

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	return NULL;
}

CEntityInput* CBaseEntity::GetInput(const char* pszName)
{
	const tchar* pszClassName = GetClassName();
	CEntityRegistration* pRegistration = NULL;
	
	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		eastl::map<eastl::string, CEntityInput>::iterator it = pRegistration->m_aInputs.find(pszName);

		if (it != pRegistration->m_aInputs.end())
			return &it->second;

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	return NULL;
}

void CBaseEntity::CheckSaveDataSize(CEntityRegistration* pRegistration)
{
#ifndef _DEBUG
	return;
#endif

	size_t iSaveTableSize = 0;

	size_t iFirstOffset = 0;
	if (pRegistration->m_aSaveData.size())
		iFirstOffset = pRegistration->m_aSaveData[0].m_iOffset;

	for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
	{
		CSaveData* pData = &pRegistration->m_aSaveData[i];

		// If bools have non-bools after them then the extra space is padded to retain four-byte alignment.
		// So, round everything up. Might mean adding bools doesn't trigger it, oh well.
		if (pData->m_iSizeOfVariable%4 == 0 && iSaveTableSize%4 != 0)
			iSaveTableSize += 4-iSaveTableSize%4;

		// This can help you find where missing stuff is, if all of the save data is in order.
		// On GCC there's also a problem where a boolean at the end of a parent class can make the beginning address of any child classes be not a multiple of 4, which can cause this to trip. Solution: Keep your booleans near the center of your class definitions. (Really should rewrite this function but meh.)
		TAssert(pData->m_iOffset - iFirstOffset == iSaveTableSize);

		iSaveTableSize += pData->m_iSizeOfVariable;
	}

	// In case a bool is at the end.
	if (iSaveTableSize%4)
		iSaveTableSize += 4-iSaveTableSize%4;

	size_t iSizeOfThis = SizeOfThis();

	// If you're getting this assert it probably means you forgot to add a savedata entry for some variable that you added to a class.
	if (iSaveTableSize != iSizeOfThis)
	{
		TMsg(sprintf(tstring("Save table for class '%s' doesn't match the class's size.\n"), convertstring<char, tchar>(GetClassName()).c_str()));
		TAssert(!_T("Save table size doesn't match class size.\n"));
	}
}

void CBaseEntity::CheckTables(const char* pszEntity)
{
#ifndef _DEBUG
	return;
#endif

	CEntityRegistration* pRegistration = GetRegisteredEntity(pszEntity);

	eastl::vector<CSaveData>& aSaveData = pRegistration->m_aSaveData;

	for (size_t i = 0; i < aSaveData.size(); i++)
	{
		CSaveData* pSaveData = &aSaveData[i];
		CNetworkedVariableData* pVariable = GetNetworkVariable(pSaveData->m_pszVariableName);
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
			eastl::vector<size_t>* pVector = (eastl::vector<size_t>*)pData;
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
			writestring(o, *(eastl::string*)pData);
			break;

		case CSaveData::DATA_STRING16:
			writetstring(o, *(tstring*)pData);
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
			eastl::vector<size_t>* pVector = (eastl::vector<size_t>*)pData;
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
			((eastl::string*)pData)->assign(readstring(i));
			break;

		case CSaveData::DATA_STRING16:
			((tstring*)pData)->assign(readtstring(i));
			break;

		case CSaveData::DATA_OUTPUT:
		{
			CEntityOutput* pOutput = (CEntityOutput*)pData;
			size_t iTargets = 0;
			i.read((char*)&iTargets, sizeof(iTargets));
			for (size_t j = 0; j < iTargets; j++)
			{
				bool bKill;

				eastl::string sTargetName = readstring(i);
				eastl::string sInput = readstring(i);
				eastl::string sArgs = readstring(i);
				i.read((char*)&bKill, sizeof(bool));

				pOutput->AddTarget(sTargetName, sInput, sArgs, bKill);
			}
			break;
		}
		}
	}

	return true;
}

void CBaseEntity::PrecacheModel(const tstring& sModel, bool bStatic)
{
	CModelLibrary::Get()->AddModel(sModel, bStatic);
}

void CBaseEntity::PrecacheParticleSystem(const tstring& sSystem)
{
	size_t iSystem = CParticleSystemLibrary::Get()->FindParticleSystem(sSystem);
	CParticleSystemLibrary::Get()->LoadParticleSystem(iSystem);
}

void CBaseEntity::PrecacheSound(const tstring& sSound)
{
	CSoundLibrary::Get()->AddSound(sSound);
}

eastl::map<tstring, CEntityRegistration>& CBaseEntity::GetEntityRegistration()
{
	static eastl::map<tstring, CEntityRegistration> aEntityRegistration;
	return aEntityRegistration;
}

void CBaseEntity::RegisterEntity(const char* pszClassName, const char* pszParentClass, EntityCreateCallback pfnCreateCallback, EntityRegisterCallback pfnRegisterCallback)
{
	CEntityRegistration* pEntity = &GetEntityRegistration()[pszClassName];
	pEntity->m_pszEntityName = pszClassName;
	pEntity->m_pszParentClass = pszParentClass;
	pEntity->m_pfnCreateCallback = pfnCreateCallback;
	pEntity->m_pfnRegisterCallback = pfnRegisterCallback;
}

void CBaseEntity::Register(CBaseEntity* pEntity)
{
	pEntity->Precache();
	pEntity->RegisterSaveData();
	pEntity->RegisterNetworkVariables();
	pEntity->RegisterInputData();
}

CEntityRegistration* CBaseEntity::GetRegisteredEntity(tstring sClassName)
{
	if (GetEntityRegistration().find(sClassName) == GetEntityRegistration().end())
		return NULL;

	return &GetEntityRegistration()[sClassName];
}

void CBaseEntity::FindEntitiesByName(const eastl::string& sName, eastl::vector<CBaseEntity*>& apEntities)
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
			if (eastl::string(pEntity->GetClassName()) != sName.c_str()+1)
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
