#include "specialshells.h"

#include <digitanks/digitanksgame.h>

NETVAR_TABLE_BEGIN(CAirstrikeShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CAirstrikeShell);
SAVEDATA_TABLE_END();


NETVAR_TABLE_BEGIN(CFireworks);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CFireworks);
SAVEDATA_TABLE_END();

bool CFireworks::ShouldTouch(CBaseEntity* pOther) const
{
	if (!pOther)
		return false;

	if (pOther->GetCollisionGroup() == CG_TERRAIN)
		return true;

	return false;
}
