#include "mechinf.h"

#include <maths.h>
#include <mtrand.h>

#include <models/models.h>
#include <network/network.h>
#include <digitanks/dt_renderer.h>

#include "ui/digitankswindow.h"
#include "ui/hud.h"
#include <digitanks/digitanksgame.h>
#include <digitanks/weapons/projectile.h>

NETVAR_TABLE_BEGIN(CMechInfantry);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMechInfantry);
SAVEDATA_TABLE_END();

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

	m_aeWeapons.push_back(PROJECTILE_FLAK);
	m_aeWeapons.push_back(PROJECTILE_TREECUTTER);
	m_aeWeapons.push_back(WEAPON_LASER);

	m_eWeapon = PROJECTILE_FLAK;
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

void CMechInfantry::PostRender(bool bTransparent)
{
	BaseClass::PostRender(bTransparent);

	if (bTransparent && (IsFortifying() || IsFortified()) && GetVisibility() > 0)
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

	if (bTransparent && (IsFortifying() || IsFortified()) && GetVisibility() > 0)
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

float CMechInfantry::ProjectileCurve() const
{
	if (GetCurrentWeapon() == PROJECTILE_FLAK)
		return -0.01f;
	else
		return -0.025f;
}

float CMechInfantry::FirstProjectileTime() const
{
	return RandomFloat(0.1f, 0.15f);
}
