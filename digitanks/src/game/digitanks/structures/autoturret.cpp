#include "autoturret.h"

#include <models/models.h>
#include <renderer/renderer.h>

#include <ui/digitankswindow.h>
#include <ui/hud.h>

#include "../weapons/turretmissile.h"

REGISTER_ENTITY(CAutoTurret);

NETVAR_TABLE_BEGIN(CAutoTurret);
	NETVAR_DEFINE_CALLBACK(bool, m_bHasFired, &CDigitanksGame::UpdateHUD);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CAutoTurret);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bHasFired);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CAutoTurret);
INPUTS_TABLE_END();

void CAutoTurret::Precache()
{
	PrecacheModel(L"models/digitanks/autoturret.obj", true);
}

void CAutoTurret::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/autoturret.obj");
}

void CAutoTurret::ModifyContext(CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);

	pContext->Scale(1.5f, 1.5f, 1.5f);

	if (!GetTeam())
		return;

	pContext->SetColorSwap(GetTeam()->GetColor());
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

eastl::vector<CDigitank*> CAutoTurret::GetTargets()
{
	eastl::vector<CDigitank*> apTargets;
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

	if (!CNetwork::IsHost())
		return;

	eastl::vector<CDigitank*> apTargets = GetTargets();

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
	eastl::string16 p;

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

		eastl::string16 s;
		s += L"DEFEND YOUR TERRITORY\n \n";
		s += L"Activate this Firewall to automatically fire at all enemies within range.\n \n";

		if (!bHasTargets)
			s += L"NO TARGETS IN RANGE\n \n";

		s += L"Shortcut: e";

		pHUD->SetButtonInfo(2, s);
		pHUD->SetButtonTooltip(2, L"Activate Firewall");
	}
}

void CAutoTurret::UpdateInfo(eastl::string16& s)
{
	eastl::string16 p;

	s = L"";
	s += L"FIREWALL INFO\n";
	s += L"Network defender\n \n";

	if (GetTeam())
	{
		s += L"Team: " + GetTeam()->GetTeamName() + L"\n";
		if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
			s += L" Friendly\n \n";
		else
			s += L" Hostile\n \n";
	}
	else
	{
		s += L"Team: Neutral\n \n";
	}

	if (IsConstructing())
	{
		s += L"(Constructing)\n";
		s += p.sprintf(L"Turns left: %d\n", GetTurnsRemainingToConstruct());
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
	if (GetDigitanksTeam()->GetPrimarySelection()->GetUnitType() == STRUCTURE_AUTOTURRET)
		return true;

	return false;
}
