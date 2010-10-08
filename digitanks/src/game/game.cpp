#include "game.h"

#include <ui/digitankswindow.h>
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

void CGame::RegisterNetworkFunctions()
{
	CNetwork::RegisterFunction("SetAngles", this, SetAnglesCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("AddTeam", this, AddTeamCallback, 1, NET_HANDLE);
	CNetwork::RegisterFunction("SetTeamColor", this, SetTeamColorCallback, 4, NET_HANDLE, NET_INT, NET_INT, NET_INT);
	CNetwork::RegisterFunction("SetTeamClient", this, SetTeamClientCallback, 2, NET_HANDLE, NET_INT);
	CNetwork::RegisterFunction("AddEntityToTeam", this, AddEntityToTeamCallback, 2, NET_HANDLE, NET_HANDLE);
}

CRenderer* CGame::CreateRenderer()
{
	return new CRenderer(CDigitanksWindow::Get()->GetWindowWidth(), CDigitanksWindow::Get()->GetWindowHeight());
}

CCamera* CGame::CreateCamera()
{
	return new CCamera();
}

void CGame::SetAngles(CNetworkParameters* p)
{
	CEntityHandle<CBaseEntity> hEntity(p->ui1);

	if (hEntity != NULL)
		hEntity->SetAngles(EAngle(p->fl2, p->fl3, p->fl4));
}

void CGame::AddTeam(CNetworkParameters* p)
{
	m_ahTeams.push_back(CEntityHandle<CTeam>(p->ui1));
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
