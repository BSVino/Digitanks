#include "baseentity.h"

#include <models/models.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <sound/sound.h>

#include "game.h"

std::map<size_t, CBaseEntity*> CBaseEntity::s_apEntityList;
size_t CBaseEntity::s_iOverrideEntityListIndex = ~0;
size_t CBaseEntity::s_iNextEntityListIndex = 0;
std::vector<CEntityRegistration> CBaseEntity::s_aEntityRegistration;

REGISTER_ENTITY(CBaseEntity);

CBaseEntity::CBaseEntity()
{
	if (s_iOverrideEntityListIndex == ~0)
		m_iHandle = s_iNextEntityListIndex++;
	else
		m_iHandle = s_iOverrideEntityListIndex;

	s_apEntityList[m_iHandle] = this;

	m_iCollisionGroup = 0;

	m_bTakeDamage = false;
	m_flTotalHealth = 1;
	m_flHealth = 1;
	m_flTimeKilled = 0;

	m_bDeleted = false;

	m_bSimulated = false;

	m_iModel = ~0;

	m_iSpawnSeed = 0;

	m_pTeam = NULL;
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

void CBaseEntity::ClientUpdate(int iClient)
{
	CNetwork::CallFunction(iClient, "SetOrigin", GetHandle(), GetOrigin().x, GetOrigin().y, GetOrigin().z);
	CNetwork::CallFunction(iClient, "SetAngles", GetHandle(), GetAngles().p, GetAngles().y, GetAngles().r);
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

		if (r.GetAlpha() > 0)
		{
			if (m_iModel != ~0)
				r.RenderModel(GetModel());

			OnRender();
		}
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

bool CBaseEntity::Collide(const Vector& s1, const Vector& s2, Vector& vecPoint)
{
	if (GetBoundingRadius() == 0)
		return false;

	float flDistance = DistanceToLineSegment(GetOrigin(), s1, s2);

	if (flDistance > GetBoundingRadius())
		return false;

	Vector vecLine = s2 - s1;
	Vector vecSphere = s1 - GetOrigin();

	float flA = vecLine.LengthSqr();
	float flB = 2 * vecSphere.Dot(vecLine);
	float flC = vecSphere.LengthSqr() - GetBoundingRadius()*GetBoundingRadius();

	float flBB4AC = flB*flB - 4*flA*flC;
	if (flBB4AC < 0)
		return false;

	float flSqrt = sqrt(flBB4AC);
	float flPlus = (-flB + flSqrt)/(2*flA);
	float flMinus = (-flB - flSqrt)/(2*flA);

	flDistance = vecLine.Length();

	Vector vecDirection = vecLine / flDistance;
	Vector vecPlus = s1 + vecDirection * (flPlus * flDistance);
	Vector vecMinus = s1 + vecDirection * (flMinus * flDistance);

	if ((vecPlus - s1).LengthSqr() < (vecMinus - s1).LengthSqr())
		vecPoint = vecPlus;
	else
		vecPoint = vecMinus;

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