#include "game.h"

CGame* CGame::s_pGame = NULL;

CGame::CGame()
{
	assert(!s_pGame);
	s_pGame = this;

	m_flGameTime = 0;
	m_flFrameTime = 0;
}

CGame::~CGame()
{
	assert(s_pGame == this);
	s_pGame = NULL;
}

void CGame::Think(float flGameTime)
{
	m_flFrameTime = flGameTime - m_flGameTime;
	m_flGameTime = flGameTime;

	// Erase anything deleted last frame.
	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		delete m_ahDeletedEntities[i];

	m_ahDeletedEntities.clear();

	Simulate();

	Think();
}

void CGame::Simulate()
{
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i));

		Vector vecNewOrigin = pEntity->GetOrigin() + pEntity->GetVelocity() * m_flFrameTime;
		if (vecNewOrigin.y < 0 && pEntity->GetOrigin() >= 0)
			pEntity->TouchedGround();
		pEntity->SetOrigin(vecNewOrigin);

		pEntity->SetVelocity(pEntity->GetVelocity() + pEntity->GetGravity() * m_flFrameTime);
	}
}

void CGame::Delete(class CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		if (m_ahDeletedEntities[i] == pEntity)
			return;

	OnDeleted(pEntity);
	pEntity->SetDeleted();
	m_ahDeletedEntities.push_back(pEntity);
}
