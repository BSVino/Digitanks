#include "mathgate.h"

REGISTER_ENTITY(CMathGate);

NETVAR_TABLE_BEGIN(CMathGate);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CMathGate);
	SAVEDATA_DEFINE_OUTPUT(OnResult);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, float, m_flBase, "Base");
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, float, m_flLeft, "Left");
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, float, m_flRight, "Right");
	SAVEDATA_EDITOR_VARIABLE("Base");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CMathGate);
	INPUT_DEFINE(InputLeft);
	INPUT_DEFINE(InputRight);
INPUTS_TABLE_END();

void CMathGate::Spawn()
{
	BaseClass::Spawn();

	m_flBase = 0;
	m_flLeft = 0;
	m_flRight = 0;
}

void CMathGate::InputLeft(const tvector<tstring>& sArgs)
{
	TAssert(sArgs.size() == 1);

	if (sArgs.size() < 1)
	{
		TMsg("Not enough arguments for InputLeft.\n");
		return;
	}

	float flNewLeft = (float)stof(sArgs[0]);

	m_flLeft = flNewLeft;

	CallOutput("OnResult");
}

void CMathGate::InputRight(const tvector<tstring>& sArgs)
{
	TAssert(sArgs.size() == 1);

	if (sArgs.size() < 1)
	{
		TMsg("Not enough arguments for InputRight.\n");
		return;
	}

	float flNewRight = (float)stof(sArgs[0]);

	m_flRight = flNewRight;

	CallOutput("OnResult");
}

const tstring CMathGate::GetOutputValue(const tstring& sOutput, size_t iValue)
{
	TAssert(sOutput == "OnResult" && iValue == 0);

	return sprintf("%f", m_flBase + m_flLeft + m_flRight);
}
