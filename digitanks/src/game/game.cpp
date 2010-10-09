#include "game.h"

#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <renderer/dissolver.h>
#include <sound/sound.h>
#include <network/network.h>

#include "camera.h"

NETVAR_TABLE_BEGIN(CGame);
NETVAR_TABLE_END();

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
	RegisterNetworkFunctions();
}

void CGame::RegisterNetworkFunctions()
{
	CNetwork::RegisterFunction("SetAngles", this, SetAnglesCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("AddTeam", this, AddTeamCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("RemoveTeam", this, RemoveTeamCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("SetTeamColor", this, SetTeamColorCallback, 4, NET_HANDLE, NET_INT, NET_INT, NET_INT);
	CNetwork::RegisterFunction("SetTeamClient", this, SetTeamClientCallback, 2, NET_HANDLE, NET_INT);
	CNetwork::RegisterFunction("AddEntityToTeam", this, AddEntityToTeamCallback, 2, NET_HANDLE, NET_HANDLE);
}

void CGame::OnClientConnect(CNetworkParameters* p)
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
		CNetwork::CallFunction(p->i2, "AddTeam", GetTeam(i)->GetHandle());

	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (!m_ahTeams[i]->IsPlayerControlled())
		{
			p->p1 = (void*)i;
			m_ahTeams[i]->SetClient(p->i2);
			break;
		}
	}
}

void CGame::OnClientDisconnect(CNetworkParameters* p)
{
	m_ahTeams[p->i1]->SetClient(-1);
}

void CGame::SetAngles(CNetworkParameters* p)
{
	CEntityHandle<CBaseEntity> hEntity(p->ui1);

	if (hEntity != NULL)
		hEntity->SetAngles(EAngle(p->fl2, p->fl3, p->fl4));
}

void CGame::AddTeamToList(CTeam* pTeam)
{
	if (!pTeam)
		return;

	CNetworkParameters p;
	p.ui1 = pTeam->GetHandle();

	CNetwork::CallFunctionParameters(NETWORK_TOCLIENTS, "AddTeam",  &p);

	AddTeam(&p);
}

void CGame::AddTeam(CNetworkParameters* p)
{
	// Prevent dupes
	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (m_ahTeams[i] == CEntityHandle<CTeam>(p->ui1))
			return;
	}

	m_ahTeams.push_back(CEntityHandle<CTeam>(p->ui1));
}

void CGame::RemoveTeamFromList(CTeam* pTeam)
{
	if (!pTeam)
		return;

	CNetworkParameters p;
	p.ui1 = pTeam->GetHandle();

	CNetwork::CallFunctionParameters(NETWORK_TOCLIENTS, "RemoveTeam",  &p);

	RemoveTeam(&p);
}

void CGame::RemoveTeam(CNetworkParameters* p)
{
	for (size_t i = 0; i < m_ahTeams.size(); i++)
	{
		if (m_ahTeams[i] == CEntityHandle<CTeam>(p->ui1))
		{
			m_ahTeams.erase(m_ahTeams.begin()+i);
			return;
		}
	}
}

void CGame::OnDeleted()
{
}

void CGame::OnDeleted(CBaseEntity* pEntity)
{
}

bool CGame::TraceLine(const Vector& s1, const Vector& s2, Vector& vecHit, CBaseEntity** pHit)
{
	Vector vecClosest = s2;
	bool bHit = false;
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
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

CTeam* CGame::GetLocalTeam()
{
	if (m_hLocalTeam == NULL || m_hLocalTeam->IsDeleted())
	{
		CGameServer* pGameServer = GameServer();
		for (size_t i = 0; i < m_ahTeams.size(); i++)
		{
			if (!m_ahTeams[i]->IsPlayerControlled())
				continue;

			if (m_ahTeams[i]->GetClient() == pGameServer->GetClientIndex())
			{
				m_hLocalTeam = m_ahTeams[i];
				return m_hLocalTeam;
			}
		}

		return NULL;
	}

	return m_hLocalTeam;
}
