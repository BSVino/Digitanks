#include "playerstart.h"

REGISTER_ENTITY(CPlayerStart);

NETVAR_TABLE_BEGIN(CPlayerStart);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CPlayerStart);
	SAVEDATA_OVERRIDE_DEFAULT(CSaveData::DATA_COPYTYPE, AABB, m_aabbBoundingBox, "BoundingBox", AABB(Vector(-0.35f, -0.35f, 0), Vector(0.35f, 0.35f, 2)));
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CPlayerStart);
INPUTS_TABLE_END();
