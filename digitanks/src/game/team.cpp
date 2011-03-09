#include "team.h"

#include <network/network.h>

REGISTER_ENTITY(CTeam);

NETVAR_TABLE_BEGIN(CTeam);
	NETVAR_DEFINE(bool, m_bHumanPlayable);
	NETVAR_DEFINE(eastl::string16, m_sName);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTeam);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bHumanPlayable);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Color, m_clrTeam);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CBaseEntity>, m_ahMembers);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bClientControlled);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, int, m_iClient);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, eastl::string16, m_sName);
SAVEDATA_TABLE_END();

CTeam::CTeam()
{
	m_bClientControlled = false;
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

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = pEntity->GetHandle();

	AddEntityToTeam(&p);

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunctionParameters(NETWORK_TOCLIENTS, "AddEntityToTeam", &p);
}

void CTeam::AddEntityToTeam(CNetworkParameters* p)
{
	CEntityHandle<CTeam> hTeam(p->ui1);
	CEntityHandle<CBaseEntity> hEntity(p->ui2);

	if (hTeam.GetPointer() == NULL)
		return;

	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		// If we're already on this team, forget it.
		// Calling the OnTeamChange() hooks just to stay on this team can be dangerous.
		if (hEntity == m_ahMembers[i])
			return;
	}

	if (hEntity->GetTeam())
		RemoveEntity(hEntity);

	hEntity->SetTeam(this);
	m_ahMembers.push_back(hEntity);

	OnAddEntity(hEntity);
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

void CTeam::ClientUpdate(int iClient)
{
	BaseClass::ClientUpdate(iClient);

	CNetwork::CallFunction(iClient, "SetTeamColor", GetHandle(), GetColor().r(), GetColor().g(), GetColor().b());

	if (IsPlayerControlled())
		CNetwork::CallFunction(iClient, "SetTeamClient", GetHandle(), GetClient());
	else
		CNetwork::CallFunction(iClient, "SetTeamClient", GetHandle(), -2);	// Bot

	for (size_t i = 0; i < m_ahMembers.size(); i++)
	{
		if (m_ahMembers[i] == NULL)
			continue;

		CNetwork::CallFunction(iClient, "AddEntityToTeam", GetHandle(), m_ahMembers[i]->GetHandle());
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
