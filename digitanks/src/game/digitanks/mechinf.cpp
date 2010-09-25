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
	if (m_bFiredWeapon)
		return;

	m_bFiredWeapon = true;

	float flAttackPower = m_flTotalPower * m_flAttackSplit;
	m_flTotalPower -= flAttackPower;
	m_flAttackPower += flAttackPower;

	if (CNetwork::IsHost())
		m_iFireProjectiles = 20;

	m_flNextIdle = Game()->GetGameTime() + RandomFloat(10, 20);

	CDigitanksWindow::Get()->GetHUD()->UpdateTurnButton();
}

CProjectile* CMechInfantry::CreateProjectile()
{
	return Game()->Create<CInfantryFlak>("CInfantryFlak");
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
		float flTimeSinceFortify = Game()->GetGameTime() - m_flFortifyTime;
		CRenderingContext c(Game()->GetRenderer());
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
		float flTimeSinceFortify = Game()->GetGameTime() - m_flFortifyTime;
		float flShieldScale = RemapValClamped(flTimeSinceFortify, 0, 1, 0, 1);
		CRenderingContext c(Game()->GetRenderer());
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
		return 1.0f + ((float)m_iFortifyLevel)/5 + GetSupportShieldRechargeBonus();

	return 1.0f + GetSupportShieldRechargeBonus();
}

float CMechInfantry::HealthRechargeRate() const
{
	if (IsFortified())
		return 0.2f + ((float)m_iFortifyLevel)/25 + GetSupportHealthRechargeBonus();

	return 0.2f + GetSupportHealthRechargeBonus();
}
