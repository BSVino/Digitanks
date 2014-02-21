#ifndef DT_TEAM_H
#define DT_TEAM_H

#include "baseentity.h"

#include "color.h"
#include <tvector.h>

class CTeam : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CTeam, CBaseEntity);

public:
								CTeam();
	virtual						~CTeam();

public:
	bool						IsHumanPlayable() { return m_bHumanPlayable; }
	void						SetNotHumanPlayable() { m_bHumanPlayable = false; }

	virtual bool				OnUnserialize(std::istream& i);

	void						AddEntity(CBaseEntity* pEntity);
	virtual void				OnAddEntity(CBaseEntity* pEntity) {};

	void						RemoveEntity(CBaseEntity* pEntity);
	virtual void				OnRemoveEntity(const CBaseEntity* pEntity) {};

	void						StartTurn();

	virtual void				OnDeleted(const CBaseEntity* pEntity);

	bool						IsPlayerControlled() const { return m_bClientControlled; };

	void						SetColor(Color clrTeam) { m_clrTeam = clrTeam; };
	Color						GetColor() const { return m_clrTeam; };

	size_t						GetNumMembers() const { return m_ahMembers.size(); };
	CBaseEntity*				GetMember(size_t i) const;

	void						SetTeamName(const tstring& sName) { m_sName = sName; };
	const tstring&				GetTeamName() const { return m_sName; }

protected:
	CNetworkedVariable<bool>	m_bHumanPlayable;

	CNetworkedColor				m_clrTeam;

	CNetworkedSTLVector<CEntityHandle<CBaseEntity> >	m_ahMembers;

	CNetworkedVariable<bool>	m_bClientControlled;

	CNetworkedString			m_sName;
};

#endif
