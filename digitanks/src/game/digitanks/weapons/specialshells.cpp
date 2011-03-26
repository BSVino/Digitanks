#include "specialshells.h"

#include <digitanks/digitanksgame.h>

REGISTER_ENTITY(CAirstrikeShell);

NETVAR_TABLE_BEGIN(CAirstrikeShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CAirstrikeShell);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CAirstrikeShell);
INPUTS_TABLE_END();


REGISTER_ENTITY(CFireworks);

NETVAR_TABLE_BEGIN(CFireworks);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CFireworks);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CFireworks);
INPUTS_TABLE_END();

bool CFireworks::ShouldTouch(CBaseEntity* pOther) const
{
	if (!pOther)
		return false;

	if (pOther->GetCollisionGroup() == CG_TERRAIN)
		return true;

	return false;
}
