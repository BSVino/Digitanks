#include "mechinf.h"

#include <maths.h>
#include <mtrand.h>

#include <models/models.h>
#include <network/network.h>
#include <renderer/game_renderingcontext.h>

#include <dt_renderer.h>
#include "ui/digitankswindow.h"
#include "ui/hud.h"
#include <digitanksgame.h>
#include <weapons/projectile.h>

REGISTER_ENTITY(CMechInfantry);

NETVAR_TABLE_BEGIN(CMechInfantry);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMechInfantry);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, size_t, m_iFortifyShieldModel);	// Set in Spawn()
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, size_t, m_iFortifyWallModel);		// Set in Spawn()
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CMechInfantry);
INPUTS_TABLE_END();

#define _T(x) x

void CMechInfantry::Precache()
{
	BaseClass::Precache();

	PrecacheModel(_T("models/digitanks/infantry-body.toy"));
	PrecacheModel(_T("models/digitanks/digitank-shield.toy"));
	PrecacheModel(_T("models/digitanks/infantry-fortify-base.toy"));
	PrecacheModel(_T("models/digitanks/infantry-fortify-shield.toy"));
}

void CMechInfantry::Spawn()
{
	BaseClass::Spawn();

	m_iFortifyShieldModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/infantry-fortify-shield.toy"));
	m_iFortifyWallModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/infantry-fortify-base.toy"));

	SetModel(_T("models/digitanks/infantry-body.toy"));
	m_iShieldModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/digitank-shield.toy"));

	m_flMaxShieldStrength = m_flShieldStrength = 100;

	m_aeWeapons.push_back(PROJECTILE_FLAK);
	m_aeWeapons.push_back(PROJECTILE_TREECUTTER);
	m_aeWeapons.push_back(WEAPON_INFANTRYLASER);

	m_eWeapon = WEAPON_INFANTRYLASER;
}

void CMechInfantry::PostRender() const
{
	BaseClass::PostRender();

	if (GameServer()->GetRenderer()->IsRenderingTransparent() && (IsFortifying() || IsFortified()) && GetVisibility() > 0)
	{
		float flTimeSinceFortify = (float)(GameServer()->GetGameTime() - m_flFortifyTime);
		CGameRenderingContext c(GameServer()->GetRenderer());
		c.Translate(GetGlobalOrigin() - Vector(0, 0, RemapValClamped(flTimeSinceFortify, 0, 1, 5.0f, 0)));
		c.Rotate(-GetGlobalAngles().y, Vector(0, 0, 1));
		float flAlpha = GetVisibility() * RemapValClamped(flTimeSinceFortify, 0, 2, 0.5f, 1);
		flAlpha *= RemapValClamped(GetShieldStrength(), 0.0f, 1.0f, 0.5f, 1);
		if (flAlpha < 1.0f)
		{
			c.SetBlend(BLEND_ALPHA);
			c.SetAlpha(flAlpha);
		}

		c.SetUniform("bColorSwapInAlpha", true);

		if (GetDigitanksPlayer())
			c.SetUniform("vecColorSwap", GetPlayerOwner()->GetColor());
		else
			c.SetUniform("vecColorSwap", Color(150, 150, 150));

		c.RenderModel(m_iFortifyWallModel);
	}

	if (GameServer()->GetRenderer()->IsRenderingTransparent() && (IsFortifying() || IsFortified()) && GetVisibility() > 0)
	{
		float flTimeSinceFortify = (float)(GameServer()->GetGameTime() - m_flFortifyTime);
		float flShieldScale = RemapValClamped(flTimeSinceFortify, 0, 1, 0.0f, 1);
		CGameRenderingContext c(GameServer()->GetRenderer());
		c.Translate(GetGlobalOrigin());
		c.Rotate(-GetGlobalAngles().y, Vector(0, 0, 1));
		c.Scale(flShieldScale, flShieldScale, flShieldScale);
		c.SetBlend(BLEND_ADDITIVE);
		c.SetAlpha(GetVisibility() * RemapValClamped(flTimeSinceFortify, 0, 2, 0.5f, 1) * GetShieldStrength());
		c.SetDepthMask(false);
		c.SetBackCulling(false);
		c.RenderModel(m_iFortifyShieldModel);
	}
}

bool CMechInfantry::CanFortify()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN && GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
		return DigitanksGame()->IsInfantryFortifyAllowed();

	else
		return true;
}

float CMechInfantry::BaseShieldRechargeRate() const
{
	float flRate = 20.0f;

	if (IsFortified())
		return flRate + ((float)m_iFortifyLevel)*10;

	return flRate;
}

float CMechInfantry::BaseHealthRechargeRate() const
{
	float flRate = 5.0f;

	if (IsFortified())
		return flRate + ((float)m_iFortifyLevel)*2;

	return flRate;
}

float CMechInfantry::ProjectileCurve() const
{
	if (GetCurrentWeapon() == PROJECTILE_FLAK)
		return -0.01f;
	else
		return -0.025f;
}

double CMechInfantry::FirstProjectileTime() const
{
	return RandomFloat(0.1f, 0.15f);
}
