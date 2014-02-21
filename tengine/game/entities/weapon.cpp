#include "weapon.h"

#include <tinker/keys.h>

#include "character.h"

REGISTER_ENTITY(CBaseWeapon);

NETVAR_TABLE_BEGIN(CBaseWeapon);
	NETVAR_DEFINE(CEntityHandle, m_hOwner);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBaseWeapon);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CBaseEntity>, m_hOwner);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBaseWeapon);
INPUTS_TABLE_END();

const TVector CBaseWeapon::MeleeAttackSphereCenter() const
{
	if (!GetOwner())
		return GetGlobalCenter();

	return GetOwner()->MeleeAttackSphereCenter();
}

void CBaseWeapon::SetOwner(class CCharacter* pOwner)
{
	m_hOwner = pOwner;
}

CCharacter* CBaseWeapon::GetOwner() const
{
	return m_hOwner;
}

void CBaseWeapon::OwnerMouseInput(int iButton, int iState)
{
	if (iButton == TINKER_KEY_MOUSE_LEFT && iState == 1)
		MeleeAttack();
}
