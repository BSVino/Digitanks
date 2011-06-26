#include "collector.h"

#include <renderer/renderer.h>
#include <ui/digitankswindow.h>
#include <ui/hud.h>

#include <digitanks/digitanksteam.h>
#include <digitanks/digitanksgame.h>

REGISTER_ENTITY(CCollector);

NETVAR_TABLE_BEGIN(CCollector);
	NETVAR_DEFINE(CEntityHandle<CResource>, m_hResource);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCollector);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CResource>, m_hResource);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CCollector);
INPUTS_TABLE_END();

void CCollector::Spawn()
{
	BaseClass::Spawn();

	SetModel(_T("models/structures/psu.obj"));
}

void CCollector::Precache()
{
	BaseClass::Precache();

	PrecacheModel(_T("models/structures/psu.obj"));
}

void CCollector::ClientSpawn()
{
	BaseClass::ClientSpawn();

	if (GameNetwork()->IsHost() && !GetResource())
	{
		SetResource(CResource::FindClosestResource(GetOrigin(), GetResourceType()));
		if (GetResource())
			GetResource()->SetCollector(this);
		else
			Delete();
	}
}

void CCollector::UpdateInfo(tstring& s)
{
	tstring p;
	s = _T("");

	s += _T("POWER SUPPLY UNIT\n");
	s += _T("Resource collector\n \n");

	if (GetTeam())
	{
		s += _T("Team: ") + GetTeam()->GetTeamName() + _T("\n");
		if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
			s += _T(" Friendly\n \n");
		else
			s += _T(" Hostile\n \n");
	}
	else
	{
		s += _T("Team: Neutral\n \n");
	}

	if (IsConstructing())
	{
		s += _T("(Constructing)\n");
		s += sprintf(tstring("Turns left: %d\n"), GetTurnsRemainingToConstruct());
		return;
	}

	if (GetSupplier() && m_hSupplyLine != NULL)
	{
		s += sprintf(tstring("Power: %.1f/turn\n"), GetPowerProduced());
		s += sprintf(tstring("Efficiency: %d\n"), (int)(m_hSupplier->GetChildEfficiency() * m_hSupplyLine->GetIntegrity() * 100));
		return;
	}
}

float CCollector::GetPowerProduced() const
{
	if (!m_hSupplyLine || GetSupplier() == NULL)
		return 0;

	return 1.2f * m_hSupplier->GetChildEfficiency() * m_hSupplyLine->GetIntegrity();
}

REGISTER_ENTITY(CBattery);

NETVAR_TABLE_BEGIN(CBattery);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBattery);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBattery);
INPUTS_TABLE_END();

void CBattery::Spawn()
{
	BaseClass::Spawn();

	SetModel(_T("models/structures/battery.obj"));
}

void CBattery::Precache()
{
	BaseClass::Precache();

	PrecacheModel(_T("models/structures/battery.obj"));
}

void CBattery::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = DigitanksWindow()->GetHUD();
	tstring p;

	if (!IsConstructing() && !IsUpgrading() && CanStructureUpgrade())
	{
		pHUD->SetButtonTexture(0, "PSU");

		if (UpgradeCost() <= GetDigitanksTeam()->GetPower())
		{
			pHUD->SetButtonListener(0, CHUD::BeginUpgrade);
			pHUD->SetButtonColor(0, Color(50, 50, 50));
		}

		tstring s;
		s += _T("UPGRADE TO POWER SUPPLY UNIT\n \n");
		s += _T("Power Supply Units provide 2 additional Power per turn. Upgrading will make this structure inactive until the upgrade is complete.\n \n");
		s += sprintf(tstring("Turns to upgrade: %d Turns\n \n"), GetTurnsToUpgrade());
		s += _T("Shortcut: Q");

		pHUD->SetButtonInfo(0, s);
		pHUD->SetButtonTooltip(0, _T("Upgrade To PSU"));
	}
}

void CBattery::UpdateInfo(tstring& s)
{
	tstring p;
	s = _T("");
	s += _T("CAPACITOR\n");
	s += _T("Resource collector\n \n");

	if (GetTeam())
	{
		s += _T("Team: ") + GetTeam()->GetTeamName() + _T("\n");
		if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
			s += _T(" Friendly\n \n");
		else
			s += _T(" Hostile\n \n");
	}
	else
	{
		s += _T("Team: Neutral\n \n");
	}

	if (IsConstructing())
	{
		s += _T("(Constructing)\n");
		s += sprintf(tstring("Turns left: %d\n"), GetTurnsRemainingToConstruct());
		return;
	}

	if (IsUpgrading())
	{
		s += _T("(Upgrading to Power Supply Unit)\n");
		s += sprintf(tstring("Turns left: %d\n"), GetTurnsRemainingToUpgrade());
		return;
	}

	if (m_hSupplier != NULL && m_hSupplyLine != NULL)
	{
		s += sprintf(tstring("Power Supplied: %.1f\n"), GetPowerProduced());
		s += sprintf(tstring("Efficiency: %d\n"), (int)(m_hSupplier->GetChildEfficiency() * m_hSupplyLine->GetIntegrity() * 100));
		return;
	}
}

bool CBattery::CanStructureUpgrade()
{
	if (!GetDigitanksTeam())
		return false;

	return GetDigitanksTeam()->CanBuildPSUs();
}

void CBattery::UpgradeComplete()
{
	if (!GameNetwork()->IsHost())
		return;

	CCollector* pCollector = GameServer()->Create<CCollector>("CCollector");
	pCollector->SetConstructing(false);
	pCollector->SetOrigin(GetOrigin());
	GetTeam()->AddEntity(pCollector);
	pCollector->SetSupplier(GetSupplier());
	pCollector->SetResource(GetResource());
	GetResource()->SetCollector(pCollector);
	pCollector->CalculateVisibility();

	Delete();

	GetDigitanksTeam()->AddActionItem(pCollector, ACTIONTYPE_UPGRADE);
}

float CBattery::GetPowerProduced() const
{
	if (!m_hSupplyLine || !m_hSupplier)
		return 0;

	return 0.5f * m_hSupplier->GetChildEfficiency() * m_hSupplyLine->GetIntegrity();
}
