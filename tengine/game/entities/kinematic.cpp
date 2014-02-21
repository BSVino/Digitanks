#include "kinematic.h"

#include <physics/physics.h>

REGISTER_ENTITY(CKinematic);

NETVAR_TABLE_BEGIN(CKinematic);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CKinematic);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, double, m_flMoveTime, "MoveTime");
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, double, m_flMoveStart);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, double, m_flMoveEnd);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecOriginalPosition);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecMoveStart);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecMoveGoal);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_COPYTYPE, double, m_flRotateTime, "RotateTime");
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, double, m_flRotateStart);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, double, m_flRotateEnd);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, EAngle, m_angRotateStart);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, EAngle, m_angRotateGoal);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bMoving);
	SAVEDATA_EDITOR_VARIABLE("DisableBackCulling");
	SAVEDATA_EDITOR_VARIABLE("MoveTime");
	SAVEDATA_EDITOR_VARIABLE("RotateTime");
	SAVEDATA_EDITOR_VARIABLE("Visible");
	SAVEDATA_EDITOR_VARIABLE("Model");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CKinematic);
	INPUT_DEFINE(MoveTo);
	INPUT_DEFINE(MoveOffset);
	INPUT_DEFINE(RotateTo);
INPUTS_TABLE_END();

void CKinematic::Spawn()
{
	BaseClass::Spawn();

	m_flMoveTime = 1;
	m_flMoveStart = -1;
	m_flRotateTime = 1;
	m_flRotateStart = -1;
}

void CKinematic::PostLoad()
{
	BaseClass::PostLoad();

	m_vecOriginalPosition = GetGlobalOrigin();
}

void CKinematic::OnSetModel()
{
	BaseClass::OnSetModel();

	// In case the model has changed.
	if (IsInPhysics())
		RemoveFromPhysics();

	if (GetModelID() == ~0)
		return;

	AddToPhysics(CT_KINEMATIC);
}

void CKinematic::Think()
{
	BaseClass::Think();

	if (m_flMoveStart > 0)
	{
		double flTime = GameServer()->GetGameTime() - m_flMoveStart;
		float flMove = RemapVal((float)flTime, 0, (float)(m_flMoveEnd - m_flMoveStart), 0, 1);
		float flRamp = Gain(flMove, 0.2f);

		m_bMoving = true;
		SetGlobalOrigin(m_vecMoveStart * (1-flRamp) + m_vecMoveGoal * flRamp);
		m_bMoving = false;

		if (flRamp >= 1)
			m_flMoveStart = -1;
	}

	if (m_flRotateStart > 0)
	{
		double flTime = GameServer()->GetGameTime() - m_flRotateStart;
		float flRotate = RemapVal((float)flTime, 0, (float)(m_flRotateEnd - m_flRotateStart), 0, 1);
		float flRamp = Gain(flRotate, 0.2f);

		m_bMoving = true;
		SetGlobalAngles(LerpValue(m_angRotateStart, m_angRotateGoal, flRamp));
		m_bMoving = false;

		if (flRamp >= 1)
			m_flRotateStart = -1;
	}
}

void CKinematic::OnSetLocalTransform(TMatrix& m)
{
	BaseClass::OnSetLocalTransform(m);

	if (m_bMoving)
		return;

	m_flMoveStart = -1;
	m_flRotateStart = -1;
}

void CKinematic::MoveTo(const tvector<tstring>& sArgs)
{
	TAssert(sArgs.size() == 3);

	if (sArgs.size() < 3)
	{
		TMsg("Not enough arguments for MoveTo.\n");
		return;
	}

	m_flMoveStart = GameServer()->GetGameTime();
	m_flMoveEnd = m_flMoveStart + m_flMoveTime;

	m_vecMoveStart = GetGlobalOrigin();
	m_vecMoveGoal = Vector((float)stof(sArgs[0]), (float)stof(sArgs[1]), (float)stof(sArgs[2]));
}

void CKinematic::MoveOffset(const tvector<tstring>& sArgs)
{
	TAssert(sArgs.size() == 3);

	if (sArgs.size() < 3)
	{
		TMsg("Not enough arguments for MoveOffset.\n");
		return;
	}

	m_flMoveStart = GameServer()->GetGameTime();
	m_flMoveEnd = m_flMoveStart + m_flMoveTime;

	m_vecMoveStart = GetGlobalOrigin();
	m_vecMoveGoal = m_vecOriginalPosition + Vector((float)stof(sArgs[0]), (float)stof(sArgs[1]), (float)stof(sArgs[2]));
}

void CKinematic::RotateTo(const tvector<tstring>& sArgs)
{
	TAssert(sArgs.size() == 3);

	if (sArgs.size() < 3)
	{
		TMsg("Not enough arguments for RotateTo.\n");
		return;
	}

	m_flRotateStart = GameServer()->GetGameTime();
	m_flRotateEnd = m_flRotateStart + m_flRotateTime;

	m_angRotateStart = GetGlobalAngles();
	m_angRotateGoal = EAngle((float)stof(sArgs[0]), (float)stof(sArgs[1]), (float)stof(sArgs[2]));
}
