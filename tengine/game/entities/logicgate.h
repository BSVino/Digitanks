#pragma once

#include <game/entities/baseentity.h>

// This class is a logic gate for entity I/O
class CLogicGate : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CLogicGate, CBaseEntity);

public:
	void				Spawn();

	DECLARE_ENTITY_INPUT(InputLeft);
	DECLARE_ENTITY_INPUT(InputRight);

	DECLARE_ENTITY_OUTPUT(OnAndTrue);
	DECLARE_ENTITY_OUTPUT(OnAndFalse);

	DECLARE_ENTITY_OUTPUT(OnOrTrue);
	DECLARE_ENTITY_OUTPUT(OnOrFalse);

	DECLARE_ENTITY_OUTPUT(OnXOrTrue);
	DECLARE_ENTITY_OUTPUT(OnXOrFalse);

	void				SetAndGate(bool bAnd);
	void				SetOrGate(bool bOr);
	void				SetXOrGate(bool bXOr);

public:
	bool				m_bLeft;
	bool				m_bRight;

	bool				m_bAnd;
	bool				m_bOr;
	bool				m_bXOr;
};
