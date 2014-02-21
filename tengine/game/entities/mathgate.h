#pragma once

#include <game/entities/baseentity.h>

// This class is a math gate for entity I/O
class CMathGate : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CMathGate, CBaseEntity);

public:
	void				Spawn();

	DECLARE_ENTITY_INPUT(InputLeft);
	DECLARE_ENTITY_INPUT(InputRight);

	void				SetBaseValue(float f) { m_flBase = f; }

	DECLARE_ENTITY_OUTPUT(OnResult);

	const tstring		GetOutputValue(const tstring& sOutput, size_t iValue);

public:
	float				m_flBase;
	float				m_flLeft;
	float				m_flRight;
};
