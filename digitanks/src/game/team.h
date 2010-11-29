#ifndef DT_TEAM_H
#define DT_TEAM_H

#include "baseentity.h"

#include "color.h"
#include <EASTL/vector.h>

class CTeam : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CTeam, CBaseEntity);

public:
								CTeam();
	virtual						~CTeam();

public:
	virtual bool				OnUnserialize(std::istream& i);

	void						AddEntity(CBaseEntity* pEntity);
	virtual void				OnAddEntity(CBaseEntity* pEntity) {};

	void						RemoveEntity(CBaseEntity* pEntity);

	void						StartTurn();

	virtual void				ClientUpdate(int iClient);

	virtual void				OnDeleted(class CBaseEntity* pEntity);

	bool						IsPlayerControlled() { return m_bClientControlled; };
	void						SetClient(int iClient);
	void						SetBot();
	int							GetClient() { return m_iClient; };

	void						SetColor(Color clrTeam) { m_clrTeam = clrTeam; };
	Color						GetColor() { return m_clrTeam; };

	size_t						GetNumMembers() { return m_ahMembers.size(); };
	CBaseEntity*				GetMember(size_t i) { if (!m_ahMembers.size()) return NULL; return m_ahMembers[i]; };

	void						AddTeam(class CNetworkParameters* p);
	void						SetTeamColor(class CNetworkParameters* p);
	void						SetTeamClient(class CNetworkParameters* p);
	void						AddEntityToTeam(class CNetworkParameters* p);

	void						SetName(eastl::string16& szName) { m_szName = szName; };
	eastl::string16				GetName() { return m_szName; }

protected:
	Color						m_clrTeam;

	eastl::vector<CEntityHandle<CBaseEntity> >	m_ahMembers;

	bool						m_bClientControlled;
	int							m_iClient;

	eastl::string16				m_szName;
};

#endif