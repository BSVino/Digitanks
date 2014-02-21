#ifndef _COUNTER_H
#define _COUNTER_H

#include <game/entities/baseentity.h>

class CCounter : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CCounter, CBaseEntity);

public:
	void				Spawn();

	void				SetGoalCount(int iCount) { m_iGoalCount = iCount; };
	int					GetGoalCount() { return m_iGoalCount; };

	void				SetCurrentCount(int iCount) { m_iCurrentCount = iCount; };
	int					GetCurrentCount() { return m_iCurrentCount; };

public:
	DECLARE_ENTITY_INPUT(SetCount);

	DECLARE_ENTITY_INPUT(Increment);
	DECLARE_ENTITY_INPUT(Decrement);

	DECLARE_ENTITY_OUTPUT(OnCountReached);

protected:
	int					m_iGoalCount;
	int					m_iCurrentCount;
};

#endif
