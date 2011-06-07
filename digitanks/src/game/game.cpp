#include "game.h"

#include <strutils.h>

#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <renderer/dissolver.h>
#include <sound/sound.h>
#include <network/network.h>
#include <tinker/application.h>
#include <tinker/cvar.h>

#include "camera.h"

REGISTER_ENTITY(CGame);

NETVAR_TABLE_BEGIN(CGame);
	NETVAR_DEFINE_CALLBACK(CEntityHandle<CTeam>, m_ahTeams, &CGame::ClearLocalTeams);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGame);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CTeam>, m_ahTeams);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CEntityHandle<CTeam>, m_ahLocalTeams);	// Detected on the fly.
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CGame);
INPUTS_TABLE_END();

CVar game_level("game_level", "");

CGame::CGame()
{
}

CGame::~CGame()
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
		m_ahTeams[i]->Delete();
}

void CGame::Spawn()
{
	BaseClass::Spawn();

	RegisterNetworkFunctions();
}

void CGame::RegisterNetworkFunctions()
{
}

void CGame::OnClientConnect(int iClient)
{
}

void CGame::OnClientEnterGame(int iClient)
{
}

void CGame::OnClientDisconnect(int iClient)
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (m_ahTeams[i]->GetClient() == iClient)
		{
			m_ahTeams[i]->SetClient(-2);
			return;
		}
	}

	TAssert(!"Couldn't find the guy who just quit!");
}

void CGame::EnterGame()
{
}

void CGame::AddTeam(CTeam* pTeam)
{
	if (!pTeam)
		return;

	// Prevent dupes
	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (m_ahTeams[i] == pTeam)
			return;
	}

	m_ahTeams.push_back(pTeam);
	m_ahLocalTeams.clear();
}

void CGame::RemoveTeam(CTeam* pTeam)
{
	if (!pTeam)
		return;

	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (m_ahTeams[i] == pTeam)
		{
			m_ahTeams.erase(m_ahTeams.begin()+i);
			break;
		}
	}

	m_ahLocalTeams.clear();
}

void CGame::OnDeleted()
{
	m_ahLocalTeams.clear();
}

void CGame::OnDeleted(CBaseEntity* pEntity)
{
	RemoveTeam(dynamic_cast<CTeam*>(pEntity));
}

bool CGame::TraceLine(const Vector& s1, const Vector& s2, Vector& vecHit, CBaseEntity** pHit, int iCollisionGroup)
{
	Vector vecClosest = s2;
	bool bHit = false;
	size_t iMaxEntities = GameServer()->GetMaxEntities();
	for (size_t i = 0; i < iMaxEntities; i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		if (iCollisionGroup && !(pEntity->GetCollisionGroup() & iCollisionGroup))
			continue;

		Vector vecPoint;
		if (pEntity->Collide(s1, s2, vecPoint))
		{
			if (!bHit || (vecPoint - s1).LengthSqr() < (vecClosest - s1).LengthSqr())
			{
				vecClosest = vecPoint;
				bHit = true;
				if (pHit)
					*pHit = pEntity;
			}
		}
	}

	if (bHit)
		vecHit = vecClosest;

	return bHit;
}

bool CGame::IsTeamControlledByMe(CTeam* pTeam)
{
	if (!pTeam)
		return false;

	if (pTeam->IsPlayerControlled() && pTeam->GetClient() == GameServer()->GetClientIndex())
		return true;

	return false;
}

const eastl::vector<CEntityHandle<CTeam> >& CGame::GetLocalTeams()
{
	if (m_ahLocalTeams.size() == 0)
	{
		// Force the const version so that accessing doesn't send it over the wire
		const CNetworkedSTLVector<CEntityHandle<CTeam> >& ahTeams = m_ahTeams;

		CGameServer* pGameServer = GameServer();
		for (size_t i = 0; i < ahTeams.size(); i++)
		{
			if (!ahTeams[i])
				continue;

			if (!ahTeams[i]->IsPlayerControlled())
				continue;

			if (ahTeams[i]->GetClient() == pGameServer->GetClientIndex())
				m_ahLocalTeams.push_back(ahTeams[i]);
		}
	}

	return m_ahLocalTeams;
}

size_t CGame::GetNumLocalTeams()
{
	if (m_ahLocalTeams.size() == 0)
		GetLocalTeams();

	return m_ahLocalTeams.size();
}

CTeam* CGame::GetLocalTeam(size_t i)
{
	if (m_ahLocalTeams.size() == 0)
		GetLocalTeams();

	return m_ahLocalTeams[i];
}

void CGame::ClearLocalTeams(CNetworkedVariableBase* pVariable)
{
	CGame* pGame = Game();
	if (!pGame)
		return;

	pGame->m_ahLocalTeams.clear();
}

CVar cheats("cheats", "off");

bool CGame::AllowCheats()
{
	return cheats.GetBool();
}
