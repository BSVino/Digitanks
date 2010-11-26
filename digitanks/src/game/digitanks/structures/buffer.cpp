#include "buffer.h"

#include <EASTL/string.h>

#include <ui/digitankswindow.h>
#include <ui/hud.h>
#include <game/team.h>

#include <renderer/renderer.h>
#include <game/game.h>

size_t CBuffer::s_iCancelIcon = 0;
size_t CBuffer::s_iInstallIcon = 0;
size_t CBuffer::s_iInstallBandwidthIcon = 0;
size_t CBuffer::s_iInstallFleetSupplyIcon = 0;
size_t CBuffer::s_iInstallEnergyBonusIcon = 0;
size_t CBuffer::s_iInstallRechargeBonusIcon = 0;

NETVAR_TABLE_BEGIN(CBuffer);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBuffer);
SAVEDATA_TABLE_END();

void CBuffer::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/buffer.obj");
}

void CBuffer::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/buffer.obj");

	s_iCancelIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-cancel.png");
	s_iInstallIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install.png");
	s_iInstallBandwidthIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-bandwidth.png");
	s_iInstallFleetSupplyIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-fleet.png");
	s_iInstallEnergyBonusIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-energy.png");
	s_iInstallRechargeBonusIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-recharge.png");
}

void CBuffer::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = DigitanksWindow()->GetHUD();
	eastl::string16 p;

	if (IsInstalling())
	{
		pHUD->SetButtonListener(4, CHUD::CancelInstall);
		pHUD->SetButtonTexture(4, s_iCancelIcon);
		pHUD->SetButtonColor(4, Color(100, 0, 0));
	}
	else if (eMenuMode == MENUMODE_INSTALL)
	{
		if (GetFirstUninstalledUpdate(UPDATETYPE_BANDWIDTH) >= 0)
		{
			pHUD->SetButtonListener(0, CHUD::InstallBandwidth);
			pHUD->SetButtonTexture(0, s_iInstallBandwidthIcon);
			pHUD->SetButtonColor(0, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_BANDWIDTH);
			CUpdateItem* pUpdate = GetUpdate(UPDATETYPE_BANDWIDTH, iUpdate);

			eastl::string16 s;
			s += L"INSTALL BANDWIDTH INCREASE\n \n";
			s += pUpdate->GetInfo() + L"\n \n";
			s += p.sprintf(L"Bandwidth increase: %f\n", pUpdate->m_flValue);
			s += p.sprintf(L"Turns to install: %d Turns\n \n", GetTurnsToInstall(pUpdate));
			s += L"Shortcut: Q";
			pHUD->SetButtonInfo(0, s);
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_FLEETSUPPLY) >= 0)
		{
			pHUD->SetButtonListener(1, CHUD::InstallFleetSupply);
			pHUD->SetButtonTexture(1, s_iInstallFleetSupplyIcon);
			pHUD->SetButtonColor(1, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_FLEETSUPPLY);
			CUpdateItem* pUpdate = GetUpdate(UPDATETYPE_FLEETSUPPLY, iUpdate);

			eastl::string16 s;
			s += L"INSTALL FLEET SUPPLY INCREASE\n \n";
			s += pUpdate->GetInfo() + L"\n \n";
			s += p.sprintf(L"Fleet Supply increase: %f\n", pUpdate->m_flValue);
			s += p.sprintf(L"Turns to install: %d Turns\n \n", GetTurnsToInstall(pUpdate));
			s += L"Shortcut: W";
			pHUD->SetButtonInfo(1, s);
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTENERGY) >= 0)
		{
			pHUD->SetButtonListener(2, CHUD::InstallEnergyBonus);
			pHUD->SetButtonTexture(2, s_iInstallEnergyBonusIcon);
			pHUD->SetButtonColor(2, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTENERGY);
			CUpdateItem* pUpdate = GetUpdate(UPDATETYPE_SUPPORTENERGY, iUpdate);

			eastl::string16 s;
			s += L"INSTALL SUPPORT ENERGY INCREASE\n \n";
			s += pUpdate->GetInfo() + L"\n \n";
			s += p.sprintf(L"Attack Energy Buff increase: %f\n", pUpdate->m_flValue);
			s += p.sprintf(L"Defense Energy Buff increase: %f\n", pUpdate->m_flValue);
			s += p.sprintf(L"Turns to install: %d Turns\n \n", GetTurnsToInstall(pUpdate));
			s += L"Shortcut: E";
			pHUD->SetButtonInfo(2, s);
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTRECHARGE) >= 0)
		{
			pHUD->SetButtonListener(3, CHUD::InstallRechargeBonus);
			pHUD->SetButtonTexture(3, s_iInstallRechargeBonusIcon);
			pHUD->SetButtonColor(3, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTRECHARGE);
			CUpdateItem* pUpdate = GetUpdate(UPDATETYPE_SUPPORTRECHARGE, iUpdate);

			eastl::string16 s;
			s += L"INSTALL HEALTH ENERGY INCREASE\n \n";
			s += pUpdate->GetInfo() + L"\n \n";
			s += p.sprintf(L"Health Recharge Buff increase: %f per turn\n", pUpdate->m_flValue/5);
			s += p.sprintf(L"Shield Recharge Buff increase: %f per turn\n", pUpdate->m_flValue);
			s += p.sprintf(L"Turns to install: %d Turns\n \n", GetTurnsToInstall(pUpdate));
			s += L"Shortcut: R";
			pHUD->SetButtonInfo(3, s);
		}

		pHUD->SetButtonListener(9, CHUD::GoToMain);
		pHUD->SetButtonTexture(9, s_iCancelIcon);
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, L"RETURN\n \nShortcut: G");
	}
	else
	{
		if (HasUpdatesAvailable())
		{
			pHUD->SetButtonListener(3, CHUD::InstallMenu);
			pHUD->SetButtonTexture(3, s_iInstallIcon);
			pHUD->SetButtonColor(3, Color(150, 150, 150));
			pHUD->SetButtonInfo(3, L"OPEN UPDATE INSTALL MENU\n \nShortcut: E");
		}
	}
}

void CBuffer::UpdateInfo(eastl::string16& s)
{
	eastl::string16 p;

	s = L"";
	s += L"BUFFER INFO\n";
	s += L"Network extender\n \n";

	if (IsConstructing())
	{
		s += L"(Constructing)\n";
		s += p.sprintf(L"Power to build: %d\n", GetProductionToConstruct());
		s += p.sprintf(L"Turns left: %d\n", GetTurnsToConstruct());
		return;
	}

	s += p.sprintf(L"Strength: %d\n", m_iDataStrength);
	s += p.sprintf(L"Growth: %d\n", (int)GetDataFlowRate());
	s += p.sprintf(L"Size: %d\n", (int)GetDataFlowRadius());
	s += p.sprintf(L"Efficiency: %d\n", (int)(GetChildEfficiency()*100));
}

bool CBuffer::HasUpdatesAvailable()
{
	if (GetFirstUninstalledUpdate(UPDATETYPE_BANDWIDTH) >= 0)
		return true;

	if (GetFirstUninstalledUpdate(UPDATETYPE_FLEETSUPPLY) >= 0)
		return true;

	if (GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTENERGY) >= 0)
		return true;

	if (GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTRECHARGE) >= 0)
		return true;

	return false;
}

size_t CMiniBuffer::s_iUpgradeIcon = 0;
size_t CMiniBuffer::s_iCancelIcon = 0;

NETVAR_TABLE_BEGIN(CMiniBuffer);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMiniBuffer);
SAVEDATA_TABLE_END();

void CMiniBuffer::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/minibuffer.obj");
}

void CMiniBuffer::Precache()
{
	BaseClass::Precache();

	s_iCancelIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-cancel.png");
	s_iUpgradeIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-buffer.png");

	PrecacheModel(L"models/structures/minibuffer.obj");
}

void CMiniBuffer::SetupMenu(menumode_t eMenuMode)
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
			s += L"UPGRADE TO BUFFER\n \n";
			s += L"Buffers provide larger Network radius and can be updated by installing downloaded updates. Upgrading will make this structure inactive until the upgrade is complete.\n \n";
			s += p.sprintf(L"Turns to upgrade: %d Turns\n \n", GetTurnsToUpgrade());
			s += L"Shortcut: Q";

			pHUD->SetButtonInfo(0, s);
		}
	}
}

void CMiniBuffer::UpdateInfo(eastl::string16& s)
{
	eastl::string16 p;
	s = L"";
	s += L"MINIBUFFER INFO\n";
	s += L"Network extender\n \n";

	if (IsConstructing())
	{
		s += L"(Constructing)\n";
		s += p.sprintf(L"Power to build: %d\n", GetProductionToConstruct());
		s += p.sprintf(L"Turns left: %d\n", GetTurnsToConstruct());
		return;
	}

	if (IsUpgrading())
	{
		s += L"(Upgrading to Buffer)\n";
		s += p.sprintf(L"Power to upgrade: %d\n", GetProductionToUpgrade());
		s += p.sprintf(L"Turns left: %d\n", GetTurnsToConstruct());
		return;
	}

	s += p.sprintf(L"Strength: %d\n", m_iDataStrength);
	s += p.sprintf(L"Growth: %d\n", (int)GetDataFlowRate());
	s += p.sprintf(L"Size: %d\n", (int)GetDataFlowRadius());
	s += p.sprintf(L"Efficiency: %d\n", (int)(GetChildEfficiency()*100));
}

bool CMiniBuffer::CanStructureUpgrade()
{
	if (!GetDigitanksTeam())
		return false;

	return GetDigitanksTeam()->CanBuildBuffers();
}

void CMiniBuffer::UpgradeComplete()
{
	if (!CNetwork::IsHost())
		return;

	CBuffer* pBuffer = GameServer()->Create<CBuffer>("CBuffer");
	pBuffer->SetConstructing(false);
	pBuffer->SetOrigin(GetOrigin());
	GetTeam()->AddEntity(pBuffer);
	pBuffer->SetSupplier(GetSupplier());
	GetSupplier()->AddChild(pBuffer);
	pBuffer->GiveDataStrength(pBuffer->InitialDataStrength() - InitialDataStrength());	// Give the difference
	pBuffer->GiveDataStrength(m_iDataStrength - InitialDataStrength());					// Give what I've earned so far
	pBuffer->AddFleetPoints(pBuffer->InitialFleetPoints() - InitialFleetPoints());
	pBuffer->AddBandwidth(pBuffer->InitialBandwidth() - InitialBandwidth());

	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		m_ahChildren[i]->SetSupplier(pBuffer);
		pBuffer->AddChild(m_ahChildren[i]);
	}

	Delete();

	DigitanksGame()->AddActionItem(pBuffer, ACTIONTYPE_UPGRADE);
}
