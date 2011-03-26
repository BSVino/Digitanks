#include "userfile.h"

#include <digitanks/units/digitank.h>

REGISTER_ENTITY(CUserFile);

NETVAR_TABLE_BEGIN(CUserFile);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CUserFile);
	SAVEDATA_DEFINE_OUTPUT(OnPickup);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CUserFile);
INPUTS_TABLE_END();

void CUserFile::Spawn()
{
	SetModel(L"models/digitanks/digitank-body.obj");
}

EAngle CUserFile::GetRenderAngles() const
{
	float flRotate = fmod(GameServer()->GetGameTime(), 3.6f)*100.0f;
	return EAngle(0, flRotate, 0);
}

bool CUserFile::IsTouching(CBaseEntity* pOther, Vector& vecPoint) const
{
	if (!pOther)
		return false;

	if (!pOther->GetTeam())
		return false;

	if (!pOther->GetTeam()->IsPlayerControlled())
		return false;

	CDigitank* pTank = dynamic_cast<CDigitank*>(pOther);

	if (!pTank)
		return false;

	if (Distance(pTank->GetRealOrigin()) > 20)
		return false;

	return true;
}

void CUserFile::Pickup(CDigitank* pTank)
{
	CallOutput("OnPickup");

	Delete();
}
