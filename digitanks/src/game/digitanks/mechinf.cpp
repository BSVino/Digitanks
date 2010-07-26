#include "mechinf.h"

#include <maths.h>
#include <mtrand.h>

#include <models/models.h>
#include <network/network.h>

#include "digitanksgame.h"
#include "projectile.h"

REGISTER_ENTITY(CMechInfantry);

CMechInfantry::CMechInfantry()
{
	SetModel(L"models/digitanks/infantry-body.obj");
	m_iShieldModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-shield.obj");

	m_flFrontMaxShieldStrength = m_flFrontShieldStrength = 15;
	m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flRearMaxShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = m_flRearShieldStrength = 2;

	m_iFireProjectiles = 0;
	m_flLastProjectileFire = 0;
}

void CMechInfantry::Precache()
{
	PrecacheModel(L"models/digitanks/infantry-body.obj", false);
	PrecacheModel(L"models/digitanks/digitank-shield.obj");
}

float CMechInfantry::GetLeftShieldMaxStrength()
{
	return GetFrontShieldMaxStrength();
}

float CMechInfantry::GetRightShieldMaxStrength()
{
	return GetFrontShieldMaxStrength();
}

float CMechInfantry::GetRearShieldMaxStrength()
{
	return GetFrontShieldMaxStrength();
}

float* CMechInfantry::GetShieldForAttackDirection(Vector vecAttack)
{
	Vector vecForward;
	AngleVectors(GetAngles(), &vecForward, NULL, NULL);

	float flForwardDot = vecForward.Dot(vecAttack);

	if (flForwardDot > 0.0f)
		return &m_flFrontShieldStrength;
	else
		return &m_flRearShieldStrength;
}

void CMechInfantry::Think()
{
	BaseClass::Think();

	if (m_iFireProjectiles && Game()->GetGameTime() > m_flLastProjectileFire + 0.1f)
	{
		m_iFireProjectiles--;
		FireProjectile();
		m_flLastProjectileFire = Game()->GetGameTime();
	}
}

void CMechInfantry::Fire()
{
	CDigitank* pClosest = NULL;

	// Fire at the closest enemy.
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CDigitank* pDigitank = dynamic_cast<CDigitank*>(pEntity);
		if (!pDigitank)
			continue;

		if (pDigitank == this)
			continue;

		if (pDigitank->GetTeam() == GetTeam())
			continue;

		if (!pClosest)
		{
			pClosest = pDigitank;
			continue;
		}

		if ((pDigitank->GetOrigin() - GetOrigin()).LengthSqr() < (pClosest->GetOrigin() - GetOrigin()).LengthSqr())
			pClosest = pDigitank;
	}

	float flDistanceSqr = (pClosest->GetOrigin() - GetOrigin()).LengthSqr();
	if (flDistanceSqr > GetMaxRange()*GetMaxRange())
		return;

	if (flDistanceSqr < GetMinRange()*GetMinRange())
		return;

	SetPreviewAim(pClosest->GetOrigin());
	SetDesiredAim();

	if (CNetwork::IsHost())
		m_iFireProjectiles = 20;

	for (size_t i = 0; i < m_iFireProjectiles; i++)
		DigitanksGame()->AddProjectileToWaitFor();

	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);
}

CProjectile* CMechInfantry::CreateProjectile()
{
	return Game()->Create<CInfantryFlak>("CInfantryFlak");
}

float CMechInfantry::GetProjectileDamage()
{
	return GetAttackPower()/20;
}

bool CMechInfantry::AllowControlMode(controlmode_t eMode)
{
	if (eMode == MODE_AIM)
		return false;

	return BaseClass::AllowControlMode(eMode);
}

float CMechInfantry::GetBonusAttackPower()
{
	if (!m_bFortified)
		return BaseClass::GetBonusAttackPower();

	return BaseClass::GetBonusAttackPower() + GetFortifyAttackPowerBonus();
}

float CMechInfantry::GetBonusDefensePower()
{
	if (!m_bFortified)
		return BaseClass::GetBonusDefensePower();

	return BaseClass::GetBonusDefensePower() + GetFortifyDefensePowerBonus();
}

float CMechInfantry::GetFortifyAttackPowerBonus()
{
	return (float)m_iFortifyLevel;
}

float CMechInfantry::GetFortifyDefensePowerBonus()
{
	return (float)m_iFortifyLevel*2;
}

float CMechInfantry::ShieldRechargeRate() const
{
	if (IsFortified())
		return 1.0f + (float)m_iFortifyLevel + GetSupportShieldRechargeBonus();

	return 1.0f + GetSupportShieldRechargeBonus();
}

float CMechInfantry::HealthRechargeRate() const
{
	if (IsFortified())
		return 0.2f + ((float)m_iFortifyLevel)/5 + GetSupportHealthRechargeBonus();

	return 0.2f + GetSupportHealthRechargeBonus();
}
