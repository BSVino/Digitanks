#include "autoturret.h"

#include <GL/glew.h>

#include <models/models.h>
#include <renderer/renderer.h>
#include <shaders/shaders.h>

#include <ui/digitankswindow.h>
#include <ui/hud.h>

#include "../weapons/turretmissile.h"

REGISTER_ENTITY(CAutoTurret);

NETVAR_TABLE_BEGIN(CAutoTurret);
	NETVAR_DEFINE_CALLBACK(bool, m_bHasFired, &CDigitanksGame::UpdateHUD);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CAutoTurret);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bHasFired);
	SAVEDATA_OMIT(m_iShieldModel);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CAutoTurret);
INPUTS_TABLE_END();

void CAutoTurret::Precache()
{
	PrecacheModel("models/structures/firewall.toy", true);
	PrecacheModel("models/structures/firewall-shield.toy", true);
}

void CAutoTurret::Spawn()
{
	BaseClass::Spawn();

	SetModel("models/structures/firewall.toy");

	m_iShieldModel = CModelLibrary::Get()->FindModel("models/structures/firewall-shield.toy");

	m_bHasFired = false;
}

void CAutoTurret::ModifyContext(CRenderingContext* pContext) const
{
	BaseClass::ModifyContext(pContext);

	if (!GetTeam())
		return;

	pContext->SetColorSwap(GetTeam()->GetColor());
}

void CAutoTurret::OnRender(CRenderingContext* pContext) const
{
	if (m_iShieldModel == ~0)
		return;

	if (GetVisibility() == 0)
		return;

	if (!GetTeam())
		return;

	if (!bTransparent)
		return;

	float flFlicker = 1;
	
	if (GetHealth() < GetTotalHealth()/2)
		flFlicker = Flicker("zzzzmmzzztzzzzzznzzz", GameServer()->GetGameTime() + ((float)GetSpawnSeed()/100), 1.0f);

	CRenderingContext r(GameServer()->GetRenderer());

	float flFinalAlpha = flFlicker*GetVisibility()*0.4f;

	if (flFinalAlpha <= 0)
		return;

	if (GameServer()->GetRenderer()->ShouldUseShaders())
	{
		r.UseProgram(CShaderLibrary::GetScrollingTextureProgram());
		r.SetUniform("iTexture", 0);
		r.SetUniform("flAlpha", flFinalAlpha);
		r.SetUniform("flTime", GameServer()->GetGameTime());
		r.SetUniform("flSpeed", 1.0f);
	}
	else
		r.SetAlpha(flFinalAlpha);

	r.SetBlend(BLEND_ADDITIVE);
	r.SetDepthTest(false);
	r.SetBackCulling(false);

	// If you just call r.RenderModel() it overrides the shader
	CModel* pModel = CModelLibrary::Get()->GetModel(m_iShieldModel);
	if (pModel)
	{
		r.BindTexture(pModel->m_iCallListTexture);
		glCallList((GLuint)pModel->m_iCallList);

		r.Scale(1.1f, 1.1f, 1.1f);
		r.Rotate(90, Vector(0, 1, 0));
		r.SetUniform("flTime", GameServer()->GetGameTime()*1.2f);

		glCallList((GLuint)pModel->m_iCallList);
	}
}

void CAutoTurret::StartTurn()
{
	BaseClass::StartTurn();

	m_bHasFired = false;

	if (GetDigitanksTeam() && GetTargets().size() > 0)
		GetDigitanksTeam()->AddActionItem(this, ACTIONTYPE_FORTIFIEDENEMY);
}

void CAutoTurret::EndTurn()
{
	BaseClass::EndTurn();

	if (!m_bHasFired)
		Fire();
}

tvector<CDigitank*> CAutoTurret::GetTargets()
{
	tvector<CDigitank*> apTargets;
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		if (pEntity->GetTeam() == GetTeam())
			continue;

		if (!pEntity->GetTeam())
			continue;

		if (pEntity->Distance(GetOrigin()) > VisibleRange())
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		if (!pTank)
			continue;

		apTargets.push_back(pTank);
	}

	return apTargets;
}

void CAutoTurret::Fire()
{
	if (m_bHasFired)
		return;

	if (!GameNetwork()->IsHost())
		return;

	tvector<CDigitank*> apTargets = GetTargets();

	for (size_t i = 0; i < apTargets.size(); i++)
	{
		CTurretMissile* pMissile = GameServer()->Create<CTurretMissile>("CTurretMissile");
		pMissile->SetOwner(this);
		pMissile->SetTarget(apTargets[i]);

		// Firing more missiles dilutes each missile.
		pMissile->SetDamage(CBaseWeapon::GetWeaponDamage(WEAPON_TURRETMISSILE)/apTargets.size());
	}

	m_bHasFired = true;

	DigitanksWindow()->GetHUD()->Layout();
}

void CAutoTurret::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = DigitanksWindow()->GetHUD();
	tstring p;

	if (!IsConstructing() && !IsUpgrading() && !m_bHasFired)
	{
		pHUD->SetButtonTexture(2, "Aim");

		bool bHasTargets = GetTargets().size() > 0;
		if (bHasTargets && (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_AIM))
		{
			pHUD->SetButtonColor(2, Color(150, 0, 0));
			pHUD->SetButtonListener(2, CHUD::FireTurret);
		}
		else
		{
			pHUD->SetButtonColor(2, Color(100, 100, 100));
			pHUD->SetButtonListener(2, NULL);
		}

		tstring s;
		s += "DEFEND YOUR TERRITORY\n \n";
		s += "Activate this Firewall to automatically fire at all enemies within range.\n \n";

		if (!bHasTargets)
			s += "NO TARGETS IN RANGE\n \n";

		s += "Shortcut: e";

		pHUD->SetButtonInfo(2, s);
		pHUD->SetButtonTooltip(2, "Activate Firewall");
	}
}

void CAutoTurret::UpdateInfo(tstring& s)
{
	tstring p;

	s = "";
	s += "FIREWALL INFO\n";
	s += "Network defender\n \n";

	if (GetTeam())
	{
		s += "Team: " + GetTeam()->GetTeamName() + "\n";
		if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
			s += " Friendly\n \n";
		else
			s += " Hostile\n \n";
	}
	else
	{
		s += "Team: Neutral\n \n";
	}

	if (IsConstructing())
	{
		s += "(Constructing)\n";
		s += sprintf(tstring("Turns left: %d\n"), GetTurnsRemainingToConstruct());
		return;
	}
}

float CAutoTurret::AvailableArea(int iArea) const
{
	if (iArea == 1)
		return DefenseRadius();

	return BaseClass::AvailableArea(iArea);
}

bool CAutoTurret::IsAvailableAreaActive(int iArea) const
{
	if (iArea != 1)
		return BaseClass::IsAvailableAreaActive(iArea);

	if (!GetDigitanksTeam())
		return false;

	if (!GetDigitanksTeam()->GetPrimaryCPU())
		return false;

	if (GetTeam() != DigitanksGame()->GetCurrentLocalDigitanksTeam())
		return false;

	if (!GetDigitanksTeam()->GetPrimarySelection())
		return false;

	// If we're selecting an auto-turret, show all auto-turret areas. Otherwise don't!
	if (GetDigitanksTeam()->GetPrimarySelection()->GetUnitType() == STRUCTURE_FIREWALL)
		return true;

	return false;
}
