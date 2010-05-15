#include "baseentity.h"
#include "game.h"

std::map<size_t, CBaseEntity*> CBaseEntity::s_apEntityList;
size_t CBaseEntity::s_iNextEntityListIndex = 0;

CBaseEntity::CBaseEntity()
{
	m_iHandle = s_iNextEntityListIndex;
	s_apEntityList.insert(std::pair<size_t, CBaseEntity*>(s_iNextEntityListIndex++, this));

	m_iCollisionGroup = 0;

	m_bTakeDamage = false;
	m_flTotalHealth = 1;
	m_flHealth = 1;

	m_bDeleted = false;

	m_bSimulated = false;
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

void CBaseEntity::TakeDamage(CBaseEntity* pAttacker, float flDamage)
{
	if (!m_bTakeDamage)
		return;

	m_flHealth -= flDamage;

	Game()->OnTakeDamage(this, pAttacker, flDamage);

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