#include "mechinf.h"

#include <maths.h>
#include <mtrand.h>

#include <models/models.h>
#include <network/network.h>
#include "dt_renderer.h"

#include "ui/digitankswindow.h"
#include "ui/hud.h"
#include "digitanksgame.h"
#include "projectile.h"

NETVAR_TABLE_BEGIN(CMechInfantry);
NETVAR_TABLE_END();

void CMechInfantry::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/digitanks/infantry-body.obj", true);
	PrecacheModel(L"models/digitanks/digitank-shield.obj", true);
	PrecacheModel(L"models/digitanks/infantry-fortify-base.obj", true);
	PrecacheModel(L"models/digitanks/infantry-fortify-shield.obj", true);
}

void CMechInfantry::Spawn()
{
	BaseClass::Spawn();

	m_iFortifyShieldModel = CModelLibrary::Get()->FindModel(L"models/digitanks/infantry-fortify-shield.obj");
	m_iFortifyWallModel = CModelLibrary::Get()->FindModel(L"models/digitanks/infantry-fortify-base.obj");

	SetModel(L"models/digitanks/infantry-body.obj");
	m_iShieldModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-shield.obj");

	m_flFrontMaxShieldStrength = m_flFrontShieldStrength = 10;
	m_flLeftMaxShieldStrength = m_flRightMaxShieldStrength = m_flLeftShieldStrength = m_flRightShieldStrength = 5;
	m_flRearMaxShieldStrength = m_flRearShieldStrength = 2;
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

CProjectile* CMechInfantry::CreateProjectile()
{
	return GameServer()->Create<CInfantryFlak>("CInfantryFlak");
}

float CMechInfantry::GetProjectileDamage()
{
	return GetAttackPower()/50;
}

void CMechInfantry::PostRender()
{
	BaseClass::PostRender();

	if ((IsFortifying() || IsFortified()) && GetVisibility() > 0)
	{
		float flTimeSinceFortify = GameServer()->GetGameTime() - m_flFortifyTime;
		CRenderingContext c(GameServer()->GetRenderer());
		c.Translate(GetOrigin() - Vector(0, RemapValClamped(flTimeSinceFortify, 0, 1, 5, 0), 0));
		c.Rotate(-GetAngles().y, Vector(0, 1, 0));
		float flAlpha = GetVisibility() * RemapValClamped(flTimeSinceFortify, 0, 2, 0.5f, 1);
		flAlpha *= RemapValClamped(GetFrontShieldStrength(), 0, 1, 0.5, 1);
		if (flAlpha < 1.0f)
		{
			c.SetBlend(BLEND_ALPHA);
			c.SetAlpha(flAlpha);
		}
		c.SetColorSwap(GetTeam()->GetColor());
		c.RenderModel(m_iFortifyWallModel);
	}

	if ((IsFortifying() || IsFortified()) && GetVisibility() > 0)
	{
		float flTimeSinceFortify = GameServer()->GetGameTime() - m_flFortifyTime;
		float flShieldScale = RemapValClamped(flTimeSinceFortify, 0, 1, 0, 1);
		CRenderingContext c(GameServer()->GetRenderer());
		c.Translate(GetOrigin());
		c.Rotate(-GetAngles().y, Vector(0, 1, 0));
		c.Scale(flShieldScale, flShieldScale, flShieldScale);
		c.SetBlend(BLEND_ADDITIVE);
		c.SetAlpha(GetVisibility() * RemapValClamped(flTimeSinceFortify, 0, 2, 0.5f, 1) * GetFrontShieldStrength());
		c.SetDepthMask(false);
		c.SetBackCulling(false);
		c.RenderModel(m_iFortifyShieldModel);
	}
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
		return 1.0f + ((float)m_iFortifyLevel.Get())/5 + GetSupportShieldRechargeBonus();

	return 1.0f + GetSupportShieldRechargeBonus();
}

float CMechInfantry::HealthRechargeRate() const
{
	if (IsFortified())
		return 0.2f + ((float)m_iFortifyLevel.Get())/25 + GetSupportHealthRechargeBonus();

	return 0.2f + GetSupportHealthRechargeBonus();
}

float CMechInfantry::FirstProjectileTime() const
{
	return RandomFloat(0.1f, 0.15f);
}

float CMechInfantry::FireProjectileTime() const
{
	return RandomFloat(0.05f, 0.1f);
}
