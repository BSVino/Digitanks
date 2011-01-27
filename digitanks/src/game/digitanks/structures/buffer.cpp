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

REGISTER_ENTITY(CBuffer);

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
		s += p.sprintf(L"Turns left: %d\n", GetTurnsRemainingToConstruct());
		return;
	}

	s += p.sprintf(L"Strength: %d\n", m_iDataStrength.Get());
	s += p.sprintf(L"Growth: %d\n", (int)GetDataFlowRate());
	s += p.sprintf(L"Size: %d\n", (int)GetDataFlowRadius());
	s += p.sprintf(L"Efficiency: %d\n", (int)(GetChildEfficiency()*100));
}

size_t CBuffer::GetTurnsToConstruct()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_TUTORIAL)
		return 1;

	return 2;
}

size_t CMiniBuffer::s_iUpgradeIcon = 0;
size_t CMiniBuffer::s_iCancelIcon = 0;

REGISTER_ENTITY(CMiniBuffer);

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

	if (!IsConstructing() && !IsUpgrading() && CanStructureUpgrade())
	{
		pHUD->SetButtonListener(0, CHUD::BeginUpgrade);

		if (UpgradeCost() <= GetDigitanksTeam()->GetPower())
		{
			pHUD->SetButtonTexture(0, s_iUpgradeIcon);
			pHUD->SetButtonColor(0, Color(150, 150, 150));
		}

		eastl::string16 s;
		s += L"UPGRADE TO BUFFER\n \n";
		s += L"Buffers provide larger Network radius and can be updated by installing downloaded updates. Upgrading will make this structure inactive until the upgrade is complete.\n \n";
		s += p.sprintf(L"Turns to upgrade: %d Turns\n \n", GetTurnsToUpgrade());
		s += L"Shortcut: Q";

		pHUD->SetButtonInfo(0, s);
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
		s += p.sprintf(L"Turns left: %d\n", GetTurnsRemainingToConstruct());
		return;
	}

	if (IsUpgrading())
	{
		s += L"(Upgrading to Buffer)\n";
		s += p.sprintf(L"Turns left: %d\n", GetTurnsRemainingToUpgrade());
		return;
	}

	s += p.sprintf(L"Strength: %d\n", m_iDataStrength.Get());
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
	pBuffer->AddEnergyBonus(pBuffer->InitialEnergyBonus() - InitialEnergyBonus());
	pBuffer->AddRechargeBonus(pBuffer->InitialRechargeBonus() - InitialRechargeBonus());

	for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
	{
		for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
		{
			if (GetDigitanksTeam()->HasDownloadedUpdate(x, y))
				pBuffer->InstallUpdate(x, y);
		}
	}

	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		m_ahChildren[i]->SetSupplier(pBuffer);
		pBuffer->AddChild(m_ahChildren[i]);
	}

	Delete();

	DigitanksGame()->AddActionItem(pBuffer, ACTIONTYPE_UPGRADE);
}
