#include "team.h"

#include <network/network.h>

REGISTER_ENTITY(CTeam);

CTeam::CTeam()
{
	m_bClientControlled = false;
}

CTeam::~CTeam()
{
	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if (m_ahTanks[i] != NULL)
			m_ahTanks[i]->Delete();
	}
}

void CTeam::AddTank(CDigitank* pTank)
{
	pTank->SetTeam(this);

	m_ahTanks.push_back(pTank);
}

void CTeam::StartTurn()
{
	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if (m_ahTanks[i] == NULL)
			continue;

		m_ahTanks[i]->StartTurn();
	}
}

void CTeam::MoveTanks()
{
	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if (m_ahTanks[i] != NULL)
			m_ahTanks[i]->Move();
	}
}

void CTeam::FireTanks()
{
	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if (m_ahTanks[i] != NULL)
			m_ahTanks[i]->Fire();
	}
}

void CTeam::ClientUpdate(int iClient)
{
	BaseClass::ClientUpdate(iClient);

	CNetwork::CallFunction(iClient, "SetTeamColor", GetHandle(), GetColor().r(), GetColor().g(), GetColor().b());

	if (IsPlayerControlled())
		CNetwork::CallFunction(iClient, "SetTeamClient", GetHandle(), GetClient());
	else
		CNetwork::CallFunction(iClient, "SetTeamClient", GetHandle(), -2);	// Bot

	for (size_t i = 0; i < GetNumTanks(); i++)
		CNetwork::CallFunction(iClient, "AddTankToTeam", GetHandle(), GetTank(i)->GetHandle());
}

void CTeam::OnKilled(CBaseEntity* pEntity)
{
}

void CTeam::OnDeleted(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if ((CBaseEntity*)m_ahTanks[i] == pEntity)
			m_ahTanks.erase(m_ahTanks.begin()+i);
	}
}

size_t CTeam::GetNumTanksAlive()
{
	size_t iTanksAlive = 0;
	for (size_t i = 0; i < m_ahTanks.size(); i++)
	{
		if (m_ahTanks[i]->IsAlive())
			iTanksAlive++;
	}

	return iTanksAlive;
}

void CTeam::SetClient(int iClient)
{
	if (iClient < -1)
	{
		SetBot();
		return;
	}

	m_bClientControlled = true;
	m_iClient = iClient;
}

void CTeam::SetBot()
{
	m_bClientControlled = false;
}
