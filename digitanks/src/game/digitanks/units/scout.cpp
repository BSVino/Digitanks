#include "scout.h"

#include <network/network.h>
#include <mtrand.h>

#include "ui/digitankswindow.h"
#include "ui/hud.h"
#include <digitanks/digitanksgame.h>
#include <digitanks/projectiles/projectile.h>

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

	m_flFrontMaxShieldStrength = m_flFrontShieldStrength = m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flRearMaxShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flRearShieldStrength = 0;

	m_bFortified = false;
}

bool CScout::AllowControlMode(controlmode_t eMode) const
{
	if (eMode == MODE_FIRE)
		return false;

	return BaseClass::AllowControlMode(eMode);
}

void CScout::Move()
{
	Vector vecStart = GetOrigin();
	Vector vecEnd = m_vecPreviewMove;

	BaseClass::Move();

	SetPreviewTurn(VectorAngles(vecEnd-vecStart).y);
	Turn();
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
	if (m_flTotalPower < TorpedoAttackPower())
		return;

	BaseClass::Fire();
}

CProjectile* CScout::CreateProjectile()
{
	return GameServer()->Create<CTorpedo>("CTorpedo");
}

void CScout::FireProjectile(CNetworkParameters* p)
{
	m_hProjectile = CEntityHandle<CProjectile>(p->ui2);

	Vector vecLandingSpot = Vector(p->fl3, p->fl4, p->fl5);

	float flGravity = DigitanksGame()->GetGravity();

	// FIRE PROTON TORPEDO NUMBER ONE NUMBER TWO
	m_hProjectile->SetOwner(this);
	m_hProjectile->SetForce(Vector(0,0,0));
	m_hProjectile->SetGravity(Vector(0, flGravity, 0));
	m_hProjectile->SetLandingSpot(vecLandingSpot);

	if (GetVisibility() > 0)
		EmitSound(L"sound/torpedo-drop.wav");

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
}

float CScout::ShieldRechargeRate() const
{
	return 0;
}

float CScout::HealthRechargeRate() const
{
	return 0.2f + GetSupportHealthRechargeBonus();
}
