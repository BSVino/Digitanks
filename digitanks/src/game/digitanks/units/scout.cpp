#include "scout.h"

#include <network/network.h>
#include <mtrand.h>

#include "ui/digitankswindow.h"
#include "ui/hud.h"
#include <digitanks/digitanksgame.h>
#include <digitanks/weapons/projectile.h>

REGISTER_ENTITY(CScout);

NETVAR_TABLE_BEGIN(CScout);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CScout);
SAVEDATA_TABLE_END();

void CScout::Precache()
{
	PrecacheModel(L"models/digitanks/scout.obj", true);
	PrecacheSound(L"sound/torpedo-drop.wav");
}

void CScout::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/scout.obj");

	m_flMaxShieldStrength = m_flShieldStrength = 0;

	m_bFortified = false;

	m_aeWeapons.push_back(PROJECTILE_TORPEDO);

	m_eWeapon = PROJECTILE_TORPEDO;
}

bool CScout::AllowControlMode(controlmode_t eMode) const
{
	return BaseClass::AllowControlMode(eMode);
}

void CScout::Move()
{
	BaseClass::Move();
}

CSupplyLine* CScout::FindClosestEnemySupplyLine(bool bInRange)
{
	CSupplyLine* pClosest = NULL;
	while (true)
	{
		pClosest = CBaseEntity::FindClosest<CSupplyLine>(GetOrigin(), pClosest);

		if (!pClosest)
			return NULL;

		if (pClosest->GetTeam() == GetTeam())
			continue;

		if (!pClosest->GetTeam())
			continue;

		if (!pClosest->GetSupplier() || !pClosest->GetEntity())
			continue;

		if (pClosest->GetIntegrity() < 0.25f)
			continue;

		if (bInRange)
		{
			if (!IsInsideMaxRange(pClosest->GetOrigin()))
				return NULL;
		}
		else
		{
			if (pClosest->Distance(GetOrigin()) > VisibleRange())
				return NULL;
		}

		// This one will do.
		return pClosest;
	}
}

void CScout::Fire()
{
	BaseClass::Fire();
}

void CScout::FireWeapon(CNetworkParameters* p)
{
	CProjectile* pTorpedo = CEntityHandle<CProjectile>(p->ui2);
	if (!pTorpedo)
		return;

	Vector vecLandingSpot = Vector(p->fl3, p->fl4, p->fl5);

	float flGravity = DigitanksGame()->GetGravity();

	// FIRE PROTON TORPEDO NUMBER ONE NUMBER TWO
	pTorpedo->SetOwner(this);
	pTorpedo->SetVelocity(Vector(0,0,0));
	pTorpedo->SetGravity(Vector(0, flGravity, 0));
	pTorpedo->SetLandingSpot(vecLandingSpot);

	if (GetVisibility() > 0)
		EmitSound(L"sound/torpedo-drop.wav");

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
}

float CScout::FindHoverHeight(Vector vecPosition) const
{
	float flHeight = BaseClass::FindHoverHeight(vecPosition);

	return flHeight + 9;
}

float CScout::BaseShieldRechargeRate() const
{
	return 0;
}

float CScout::BaseHealthRechargeRate() const
{
	return 7.0f;
}
