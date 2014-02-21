#include "trigger.h"

#include <physics/physics.h>
#include <game/gameserver.h>
#include <renderer/game_renderer.h>

REGISTER_ENTITY(CTrigger);

NETVAR_TABLE_BEGIN(CTrigger);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CTrigger);
	SAVEDATA_DEFINE_OUTPUT(OnStartTouch);
	SAVEDATA_DEFINE_OUTPUT(OnEndTouch);
	SAVEDATA_DEFINE_OUTPUT(OnStartVisible);
	SAVEDATA_DEFINE_OUTPUT(OnEndVisible);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CBaseEntity>, m_ahTouching);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CBaseEntity>, m_ahLastTouching);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bVisible);
	SAVEDATA_EDITOR_VARIABLE("Scale");
	SAVEDATA_EDITOR_VARIABLE("Active");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTrigger);
INPUTS_TABLE_END();

void CTrigger::Spawn()
{
	BaseClass::Spawn();

	m_bVisible = false;
}

void CTrigger::OnSetModel()
{
	BaseClass::OnSetModel();

	// In case the model has changed.
	if (IsInPhysics())
		RemoveFromPhysics();

	AddToPhysics(CT_TRIGGER);
}

void CTrigger::ClientSpawn()
{
	if (IsInPhysics())
		RemoveFromPhysics();

	AddToPhysics(CT_TRIGGER);

	BaseClass::ClientSpawn();
}

void CTrigger::Think()
{
	BaseClass::Think();

	bool bVisible = false;
	if (IsActive())
		bVisible = GameServer()->GetRenderer()->IsSphereInFrustum(GetGlobalCenter(), (float)GetBoundingRadius());

	if (bVisible && !m_bVisible)
		StartVisible();
	else if (!bVisible && m_bVisible)
		EndVisible();
}

void CTrigger::Touching(size_t iOtherHandle)
{
	CBaseEntity* pBaseOther = CEntityHandle<CBaseEntity>(iOtherHandle);

	if (!IsActive())
		return;

	for (size_t i = 0; i < m_ahLastTouching.size(); i++)
	{
		if (m_ahLastTouching[i] == (const CBaseEntity*)pBaseOther)
		{
			// We were touching before and we still are. Great.
			m_ahTouching.push_back(pBaseOther);
			m_ahLastTouching.erase(m_ahLastTouching.begin()+i);
			return;
		}
	}

	// Not in the LastTouching list, so it must be a new touch.
	StartTouch(pBaseOther);
	m_ahTouching.push_back(pBaseOther);
}

void CTrigger::BeginTouchingList()
{
	m_ahLastTouching.clear();
	m_ahTouching.swap(m_ahLastTouching);
}

void CTrigger::EndTouchingList()
{
	// Anybody still in the LastTouching list is no longer touching.
	for (size_t i = 0; i < m_ahLastTouching.size(); i++)
	{
		if (!m_ahLastTouching[i])
			continue;

		EndTouch(m_ahLastTouching[i]);
	}
}

void CTrigger::StartTouch(CBaseEntity* pOther)
{
	CallOutput("OnStartTouch");
	OnStartTouch(pOther);
}

void CTrigger::EndTouch(CBaseEntity* pOther)
{
	CallOutput("OnEndTouch");
	OnEndTouch(pOther);
}

void CTrigger::StartVisible()
{
	m_bVisible = true;
	CallOutput("OnStartVisible");
}

void CTrigger::EndVisible()
{
	m_bVisible = false;
	CallOutput("OnEndVisible");
}
