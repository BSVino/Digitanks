#include "team.h"

#include <network/network.h>

REGISTER_ENTITY(CTeam);

NETVAR_TABLE_BEGIN(CTeam);
	NETVAR_DEFINE(bool, m_bHumanPlayable);
	NETVAR_DEFINE(Color, m_clrTeam);
	NETVAR_DEFINE(CEntityHandle<CBaseEntity>, m_ahMembers);
	NETVAR_DEFINE(bool, m_bClientControlled);
	NETVAR_DEFINE(int, m_iClient);
	NETVAR_DEFINE(eastl::string16, m_sName);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTeam);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bHumanPlayable);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Color, m_clrTeam);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CBaseEntity>, m_ahMembers);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bClientControlled);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_iClient);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, eastl::string16, m_sName);
SAVEDATA_TABLE_END();

CTeam::CTeam()
{
	m_bClientControlled = false;
	m_bHumanPlayable = true;
	m_iClient = -1;
}

CTeam::~CTeam()
{
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] != NULL)
			m_ahMembers[i]->Delete();
	}
}

bool CTeam::OnUnserialize(std::istream& i)
{
	if (IsPlayerControlled() && m_iClient >= 0)
		SetClient(-2);

	return BaseClass::OnUnserialize(i);
}

void CTeam::AddEntity(CBaseEntity* pEntity)
{
	if (!pEntity)
		return;

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
			OnRemoveEntity(pEntity);
			return;
		}
	}
}

void CTeam::OnDeleted(CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == pEntity)
		{
			m_ahMembers.erase(m_ahMembers.begin()+i);
			OnRemoveEntity(pEntity);
			break;
		}
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
