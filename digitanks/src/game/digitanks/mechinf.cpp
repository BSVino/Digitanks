#include "mechinf.h"

#include <maths.h>
#include <mtrand.h>

#include <models/models.h>
#include <network/network.h>

#include "digitanksgame.h"
#include "projectile.h"

CMechInfantry::CMechInfantry()
{
	SetModel(L"models/digitanks/infantry-body.obj");
	m_iShieldModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-shield.obj");

	m_flFrontMaxShieldStrength = m_flFrontShieldStrength = 10;
	m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = 5;
	m_flRearMaxShieldStrength = m_flRearShieldStrength = 2;

	m_iFireProjectiles = 0;
	m_flLastProjectileFire = 0;
}

void CMechInfantry::Precache()
{
	PrecacheModel(L"models/digitanks/infantry-body.obj");
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

	if (!pClosest)
		return;

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
	return GetAttackPower()/80;
}

bool CMechInfantry::AllowControlMode(controlmode_t eMode)
{
	if (eMode == MODE_AIM)
		return false;

	return BaseClass::AllowControlMode(eMode);
}

float CMechInfantry::GetBonusAttackPower(bool bPreview)
{
	return BaseClass::GetBonusAttackPower(bPreview) + GetFortifyAttackPowerBonus()*GetBonusAttackScale(bPreview);
}

float CMechInfantry::GetBonusDefensePower(bool bPreview)
{
	return BaseClass::GetBonusDefensePower(bPreview) + GetFortifyDefensePowerBonus()*GetBonusDefenseScale(bPreview);
}

float CMechInfantry::GetFortifyAttackPowerBonus()
{
	if (m_bFortified)
		return (float)m_iFortifyLevel;
	else
		return 0;
}

float CMechInfantry::GetFortifyDefensePowerBonus()
{
	if (m_bFortified)
		return (float)m_iFortifyLevel;
	else
		return 0;
}

float CMechInfantry::ShieldRechargeRate() const
{
	if (IsFortified())
		return 1.0f + ((float)m_iFortifyLevel)/5 + GetSupportShieldRechargeBonus();

	return 1.0f + GetSupportShieldRechargeBonus();
}

float CMechInfantry::HealthRechargeRate() const
{
	if (IsFortified())
		return 0.2f + ((float)m_iFortifyLevel)/25 + GetSupportHealthRechargeBonus();

	return 0.2f + GetSupportHealthRechargeBonus();
}
