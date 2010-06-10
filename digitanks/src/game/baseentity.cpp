#include "baseentity.h"

#include <models/models.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <sound/sound.h>

#include "game.h"

std::map<size_t, CBaseEntity*> CBaseEntity::s_apEntityList;
size_t CBaseEntity::s_iNextEntityListIndex = 0;
std::vector<EntityRegisterCallback> CBaseEntity::s_apfnEntityRegisterCallbacks;

REGISTER_ENTITY(CBaseEntity);

CBaseEntity::CBaseEntity()
{
	m_iHandle = s_iNextEntityListIndex;
	s_apEntityList.insert(std::pair<size_t, CBaseEntity*>(s_iNextEntityListIndex++, this));

	m_iCollisionGroup = 0;

	m_bTakeDamage = false;
	m_flTotalHealth = 1;
	m_flHealth = 1;
	m_flTimeKilled = 0;

	m_bDeleted = false;

	m_bSimulated = false;

	m_iModel = ~0;
}

CBaseEntity::~CBaseEntity()
{
	s_apEntityList.erase(s_apEntityList.find(m_iHandle));
}

void CBaseEntity::SetModel(const wchar_t* pszModel)
{
	m_iModel = CModelLibrary::Get()->FindModel(pszModel);
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

	std::map<size_t, CBaseEntity*>::iterator it = s_apEntityList.begin();
	while (i--)
		it++;

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

void CBaseEntity::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit)
{
	if (!m_bTakeDamage)
		return;

	bool bWasAlive = IsAlive();

	m_flHealth -= flDamage;

	Game()->OnTakeDamage(this, pAttacker, pInflictor, flDamage, bDirectHit, !IsAlive() && bWasAlive);

	if (bWasAlive && m_flHealth <= 0)
		Killed(pAttacker);
}

void CBaseEntity::Killed(CBaseEntity* pKilledBy)
{
	m_flTimeKilled = Game()->GetGameTime();

	OnKilled(pKilledBy);
	Game()->OnKilled(this);
}

void CBaseEntity::Render()
{
	PreRender();

	do {
		CRenderingContext r(Game()->GetRenderer());
		r.Translate(GetRenderOrigin());

		EAngle angRender = GetRenderAngles();
		Vector vecForward, vecRight, vecUp;

		AngleVectors(angRender, &vecForward, &vecRight, &vecUp);

		// These first two aren't tested.
		//r.Rotate(-angRender.r, vecForward);
		//r.Rotate(-angRender.p, vecRight);
		r.Rotate(-angRender.y, Vector(0, 1, 0));

		ModifyContext(&r);

		if (m_iModel != ~0)
			r.RenderModel(GetModel());

		OnRender();
	} while (false);

	PostRender();
}

void CBaseEntity::Delete()
{
	Game()->Delete(this);
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

void CBaseEntity::RegisterEntity(EntityRegisterCallback pfnCallback)
{
	s_apfnEntityRegisterCallbacks.push_back(pfnCallback);
}

void CBaseEntity::RegisterEntity(CBaseEntity* pEntity)
{
	pEntity->Precache();
}
