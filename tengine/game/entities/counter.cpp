#include "counter.h"

REGISTER_ENTITY(CCounter);

NETVAR_TABLE_BEGIN(CCounter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CCounter);
	SAVEDATA_DEFINE_OUTPUT(OnCountReached);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, int, m_iGoalCount, "GoalCount");
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, int, m_iCurrentCount);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CCounter);
	INPUT_DEFINE(SetCount);
	INPUT_DEFINE(Increment);
	INPUT_DEFINE(Decrement);
INPUTS_TABLE_END();

void CCounter::Spawn()
{
	BaseClass::Spawn();

	SetGoalCount(0);
	SetCurrentCount(0);
}

void CCounter::SetCount(const tvector<tstring>& sArgs)
{
	TAssert(sArgs.size());

	if (sArgs.size() == 0)
		return;

	SetGoalCount(stoi(sArgs[0]));
}

void CCounter::Increment(const tvector<tstring>& sArgs)
{
	m_iCurrentCount++;

	if (m_iCurrentCount == m_iGoalCount)
		CallOutput("OnCountReached");
}

void CCounter::Decrement(const tvector<tstring>& sArgs)
{
	m_iCurrentCount--;

	if (m_iCurrentCount == m_iGoalCount)
		CallOutput("OnCountReached");
}
