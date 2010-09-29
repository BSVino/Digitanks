#include "team.h"

#include <network/network.h>

CTeam::CTeam()
{
	m_bClientControlled = false;
}

CTeam::~CTeam()
{
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] != NULL)
			m_ahMembers[i]->Delete();
	}
}

void CTeam::AddEntity(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		// If we're already on this team, forget it.
		// Calling the OnTeamChange() hooks just to stay on this team can be dangerous.
		if (pEntity == m_ahMembers[i])
			return;
	}

	if (pEntity->GetTeam())
		RemoveEntity(pEntity);

	pEntity->SetTeam(this);
	m_ahMembers.push_back(pEntity);

	OnAddEntity(pEntity);
}

void CTeam::RemoveEntity(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (pEntity == m_ahMembers[i])
		{
			m_ahMembers.erase(m_ahMembers.begin()+i);
			pEntity->SetTeam(NULL);
			return;
		}
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

	for (size_t i = 0; i < m_ahMembers.size(); i++)
		CNetwork::CallFunction(iClient, "AddEntityToTeam", GetHandle(), m_ahMembers[i]->GetHandle());
}

void CTeam::OnDeleted(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == pEntity)
			m_ahMembers.erase(m_ahMembers.begin()+i);
	}
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

void CTeam::SetTeamColor(CNetworkParameters* p)
{
	CEntityHandle<CTeam> hTeam(p->ui1);
	
	if (hTeam.GetPointer() != NULL)
		hTeam->m_clrTeam = Color(p->i2, p->i3, p->i4);
}

void CTeam::SetTeamClient(CNetworkParameters* p)
{
	CEntityHandle<CTeam> hTeam(p->ui1);
	
	if (hTeam.GetPointer() != NULL)
		hTeam->SetClient(p->i2);
}

void CTeam::AddEntityToTeam(CNetworkParameters* p)
{
	CEntityHandle<CTeam> hTeam(p->ui1);

	if (hTeam.GetPointer() != NULL)
		hTeam->AddEntity(CEntityHandle<CBaseEntity>(p->ui2));
}
