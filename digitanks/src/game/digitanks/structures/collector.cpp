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

void CCollector::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/psu.obj");
}

void CCollector::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/psu.obj");
}

void CCollector::UpdateInfo(eastl::string16& s)
{
	eastl::string16 p;
	s = L"";

	s += L"POWER SUPPLY UNIT\n";
	s += L"Resource collector\n \n";

	if (IsConstructing())
	{
		s += L"(Constructing)\n";
		s += p.sprintf(L"Turns left: %d\n", GetTurnsToConstruct());
		return;
	}

	if (GetSupplier())
	{
		s += p.sprintf(L"Power supplied: %d\n", (size_t)(GetProduction() * GetSupplier()->GetChildEfficiency()));
		s += p.sprintf(L"Efficiency: %d\n", (int)(m_hSupplier->GetChildEfficiency()*100));
		return;
	}
}

size_t CCollector::GetProduction()
{
	if (m_hSupplyLine == NULL)
		return 0;

	return (size_t)(4 * m_hSupplyLine->GetIntegrity());
}

size_t CBattery::s_iUpgradeIcon = 0;
size_t CBattery::s_iCancelIcon = 0;

REGISTER_ENTITY(CBattery);

NETVAR_TABLE_BEGIN(CBattery);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBattery);
SAVEDATA_TABLE_END();

void CBattery::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/battery.obj");
}

void CBattery::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/battery.obj");

	s_iCancelIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-cancel.png");
	s_iUpgradeIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-psu.png");
}

void CBattery::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = DigitanksWindow()->GetHUD();
	eastl::string16 p;

	if (!IsConstructing() && CanStructureUpgrade())
	{
		if (IsUpgrading())
		{
			pHUD->SetButtonListener(9, CHUD::CancelUpgrade);
			pHUD->SetButtonTexture(9, s_iCancelIcon);
			pHUD->SetButtonColor(9, Color(100, 0, 0));
			pHUD->SetButtonInfo(9, L"CANCEL UPGRADE\n \nShortcut: G");
		}
		else
		{
			pHUD->SetButtonListener(0, CHUD::BeginUpgrade);
			pHUD->SetButtonTexture(0, s_iUpgradeIcon);
			pHUD->SetButtonColor(0, Color(150, 150, 150));

			eastl::string16 s;
			s += L"UPGRADE TO POWER SUPPLY UNIT\n \n";
			s += L"Power Supply Units provide 2 additional Power per turn. Upgrading will make this structure inactive until the upgrade is complete.\n \n";
			s += p.sprintf(L"Turns to upgrade: %d Turns\n \n", GetTurnsToUpgrade());
			s += L"Shortcut: Q";

			pHUD->SetButtonInfo(0, s);
		}
	}
}

void CBattery::UpdateInfo(eastl::string16& s)
{
	eastl::string16 p;
	s = L"";
	s += L"BATTERY\n";
	s += L"Resource collector\n \n";

	if (IsConstructing())
	{
		s += L"(Constructing)\n";
		s += p.sprintf(L"Turns left: %d\n", GetTurnsToConstruct());
		return;
	}

	if (IsUpgrading())
	{
		s += L"(Upgrading to Power Supply Unit)\n";
		s += p.sprintf(L"Power to upgrade: %d\n", GetProductionToUpgrade());
		s += p.sprintf(L"Turns left: %d\n", GetTurnsToUpgrade());
		return;
	}

	if (m_hSupplier != NULL)
	{
		s += p.sprintf(L"Power supplied: %d\n", (size_t)(GetProduction() * m_hSupplier->GetChildEfficiency()));
		s += p.sprintf(L"Efficiency: %d%\n", (int)(m_hSupplier->GetChildEfficiency()*100));
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
	if (!CNetwork::IsHost())
		return;

	CCollector* pCollector = GameServer()->Create<CCollector>("CCollector");
	pCollector->SetConstructing(false);
	pCollector->SetOrigin(GetOrigin());
	GetTeam()->AddEntity(pCollector);
	pCollector->SetSupplier(GetSupplier());
	pCollector->SetResource(GetResource());
	GetResource()->SetCollector(pCollector);

	Delete();

	DigitanksGame()->AddActionItem(pCollector, ACTIONTYPE_UPGRADE);
}

size_t CBattery::GetProduction()
{
	if (m_hSupplyLine == NULL)
		return 0;

	return (size_t)(2 * m_hSupplyLine->GetIntegrity());
}
