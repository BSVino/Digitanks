#ifndef TINKER_TRIGGER_H
#define TINKER_TRIGGER_H

#include <game/entities/baseentity.h>

// This class is a kinematic physics object that is controllable with entity I/O
class CTrigger : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CTrigger, CBaseEntity);

public:
	void				Spawn();
	virtual void		OnSetModel();
	virtual void		ClientSpawn();

	virtual void		Think();

	virtual void		Touching(size_t iOtherHandle);
	virtual void		BeginTouchingList();
	virtual void		EndTouchingList();

	void				StartTouch(CBaseEntity* pOther);
	void				EndTouch(CBaseEntity* pOther);

	virtual void		OnStartTouch(CBaseEntity* pOther) {};
	virtual void		OnEndTouch(CBaseEntity* pOther) {};

	DECLARE_ENTITY_OUTPUT(OnStartTouch);
	DECLARE_ENTITY_OUTPUT(OnEndTouch);

	virtual void		StartVisible();
	virtual void		EndVisible();

	DECLARE_ENTITY_OUTPUT(OnStartVisible);
	DECLARE_ENTITY_OUTPUT(OnEndVisible);

	virtual bool		ShouldRender() const { return false; };

	collision_group_t   GetCollisionGroup() const { return CG_TRIGGER; }

protected:
	tvector<CEntityHandle<CBaseEntity> >	m_ahTouching;
	tvector<CEntityHandle<CBaseEntity> >	m_ahLastTouching;

	bool				m_bVisible;
};

#endif
