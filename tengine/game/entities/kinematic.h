#ifndef TINKER_KINEMATIC_H
#define TINKER_KINEMATIC_H

#include <game/entities/baseentity.h>

// This class is a kinematic physics object that is controllable with entity I/O
class CKinematic : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CKinematic, CBaseEntity);

public:
	void				Spawn();
	void                PostLoad();
	virtual void		OnSetModel();
	virtual void		Think();

	virtual void		OnSetLocalTransform(TMatrix& m);

	virtual void        SetMoveTime(float flMoveTime) { m_flMoveTime = flMoveTime; }
	virtual void        SetRotateTime(float flRotateTime) { m_flRotateTime = flRotateTime; }

	DECLARE_ENTITY_INPUT(MoveTo);
	DECLARE_ENTITY_INPUT(MoveOffset);
	DECLARE_ENTITY_INPUT(RotateTo);

	collision_group_t   GetCollisionGroup() const { return CG_STATIC; }

protected:
	double              m_flMoveTime;
	double              m_flMoveStart;
	double              m_flMoveEnd;

	Vector              m_vecOriginalPosition;
	Vector              m_vecMoveStart;
	Vector              m_vecMoveGoal;

	double              m_flRotateTime;
	double              m_flRotateStart;
	double              m_flRotateEnd;

	EAngle              m_angRotateStart;
	EAngle              m_angRotateGoal;

	bool                m_bMoving;
};

#endif
