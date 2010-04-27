#include "baseentity.h"
#include "game.h"

std::map<size_t, CBaseEntity*> CBaseEntity::s_apEntityList;
size_t CBaseEntity::s_iNextEntityListIndex = 0;

CBaseEntity::CBaseEntity()
{
	m_iHandle = s_iNextEntityListIndex;
	s_apEntityList.insert(std::pair<size_t, CBaseEntity*>(s_iNextEntityListIndex++, this));

	m_flTotalHealth = 1;
	m_flHealth = 1;

	m_bDeleted = false;
}

CBaseEntity::~CBaseEntity()
{
	s_apEntityList.erase(s_apEntityList.find(m_iHandle));
}

CBaseEntity* CBaseEntity::GetEntity(size_t iHandle)
{
	if (s_apEntityList.find(iHandle) == s_apEntityList.end())
		return NULL;

	return s_apEntityList[iHandle];
}

void CBaseEntity::TakeDamage(CBaseEntity* pAttacker, float flDamage)
{
	m_flHealth -= flDamage;

	if (m_flHealth <= 0)
		Killed();
}

void CBaseEntity::Killed()
{
	Game()->OnKilled(this);
	Game()->Delete(this);
}

void CBaseEntity::Delete()
{
	Game()->Delete(this);
}