#include "buffer.h"

#include <tstring.h>

#include <ui/digitankswindow.h>
#include <ui/hud.h>
#include <game/entities/team.h>

#include <renderer/renderer.h>
#include <game/entities/game.h>

REGISTER_ENTITY(CBuffer);

NETVAR_TABLE_BEGIN(CBuffer);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBuffer);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBuffer);
INPUTS_TABLE_END();

void CBuffer::Spawn()
{
	BaseClass::Spawn();

	SetModel("models/structures/buffer.toy");
}

void CBuffer::Precache()
{
	BaseClass::Precache();

	PrecacheModel("models/structures/buffer.toy");
}

bool CBuffer::AllowControlMode(controlmode_t eMode) const
{
	if (eMode == MODE_BUILD)
		return true;

	return BaseClass::AllowControlMode(eMode);
}

void CBuffer::SetupMenu(menumode_t eMenuMode)
{
	if (GetDigitanksPlayer() && GetDigitanksPlayer()->GetPrimaryCPU() != NULL)
		GetDigitanksPlayer()->GetPrimaryCPU()->SetupMenu(eMenuMode);
}

void CBuffer::UpdateInfo(tstring& s)
{
	tstring p;

	s = "";
	s += "MACRO-BUFFER INFO\n";
	s += "Network extender\n \n";

	if (GetPlayerOwner())
	{
		s += "Team: " + GetPlayerOwner()->GetPlayerName() + "\n";
		if (GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
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
		s += tsprintf(tstring("Turns left: %d\n"), GetTurnsRemainingToConstruct());
		return;
	}

	s += tsprintf(tstring("Fleet Points: %d\n"), FleetPoints());
	s += tsprintf(tstring("Bandwidth: %.1f/turn\n"), Bandwidth());
	s += tsprintf(tstring("Network Size: %d\n"), (int)GetDataFlowRadius());
	s += tsprintf(tstring("Efficiency: %d\n"), (int)(GetChildEfficiency() * 100));
}

REGISTER_ENTITY(CMiniBuffer);

NETVAR_TABLE_BEGIN(CMiniBuffer);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMiniBuffer);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CMiniBuffer);
INPUTS_TABLE_END();

void CMiniBuffer::Spawn()
{
	BaseClass::Spawn();

	SetModel("models/structures/minibuffer.toy");
}

void CMiniBuffer::Precache()
{
	BaseClass::Precache();

	PrecacheModel("models/structures/minibuffer.toy");
}

bool CMiniBuffer::AllowControlMode(controlmode_t eMode) const
{
	if (eMode == MODE_BUILD)
		return true;

	return BaseClass::AllowControlMode(eMode);
}

void CMiniBuffer::SetupMenu(menumode_t eMenuMode)
{
	if (GetDigitanksPlayer()->GetPrimaryCPU() != NULL)
		GetDigitanksPlayer()->GetPrimaryCPU()->SetupMenu(eMenuMode);

	CHUD* pHUD = DigitanksWindow()->GetHUD();
	tstring p;

	if (!IsConstructing() && !IsUpgrading() && CanStructureUpgrade())
	{
		pHUD->SetButtonListener(0, CHUD::BeginUpgrade);

		if (UpgradeCost() <= GetDigitanksPlayer()->GetPower())
		{
			pHUD->SetButtonTexture(0, "MacroBuffer");
			pHUD->SetButtonColor(0, Color(50, 50, 50));
		}

		tstring s;
		s += "UPGRADE TO MACRO-BUFFER\n \n";
		s += "Macro-Buffers provide larger Network radius and can be updated by installing downloaded updates. Upgrading will make this structure inactive until the upgrade is complete.\n \n";
		s += tsprintf(tstring("Turns to upgrade: %d Turns\n \n"), GetTurnsToUpgrade());
		s += "Shortcut: Q";

		pHUD->SetButtonInfo(0, s);
		pHUD->SetButtonTooltip(0, "Upgrade To Macro-Buffer");
	}
}

void CMiniBuffer::UpdateInfo(tstring& s)
{
	tstring p;
	s = "";
	s += "BUFFER INFO\n";
	s += "Network extender\n \n";

	if (GetPlayerOwner())
	{
		s += "Team: " + GetPlayerOwner()->GetPlayerName() + "\n";
		if (GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
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
		s += tsprintf(tstring("Turns left: %d\n"), GetTurnsRemainingToConstruct());
		return;
	}

	if (IsUpgrading())
	{
		s += "(Upgrading to Macro-Buffer)\n";
		s += tsprintf(tstring("Turns left: %d\n"), GetTurnsRemainingToUpgrade());
		return;
	}

	s += tsprintf(tstring("Fleet Points: %d\n"), FleetPoints());
	s += tsprintf(tstring("Bandwidth: %.1f/turn\n"), Bandwidth());
	s += tsprintf(tstring("Network Size: %d\n"), (int)GetDataFlowRadius());
	s += tsprintf(tstring("Efficiency: %d\n"), (int)(GetChildEfficiency()*100));
}

bool CMiniBuffer::CanStructureUpgrade()
{
	if (!GetDigitanksPlayer())
		return false;

	return GetDigitanksPlayer()->CanBuildBuffers();
}

void CMiniBuffer::UpgradeComplete()
{
	if (!GameNetwork()->IsHost())
		return;

	CBuffer* pBuffer = GameServer()->Create<CBuffer>("CBuffer");
	pBuffer->SetConstructing(false);
	pBuffer->SetGlobalOrigin(GetGlobalOrigin());
	GetPlayerOwner()->AddUnit(pBuffer);
	pBuffer->SetSupplier(GetSupplier());
	GetSupplier()->AddChild(pBuffer);
	pBuffer->GiveDataStrength(pBuffer->InitialDataStrength() - InitialDataStrength());	// Give the difference
	pBuffer->GiveDataStrength(m_iDataStrength - InitialDataStrength());					// Give what I've earned so far
	pBuffer->AddFleetPoints(pBuffer->InitialFleetPoints() - InitialFleetPoints());
	pBuffer->AddBandwidth(pBuffer->InitialBandwidth() - InitialBandwidth());
	pBuffer->AddEnergyBonus(pBuffer->InitialEnergyBonus() - InitialEnergyBonus());
	pBuffer->AddRechargeBonus(pBuffer->InitialRechargeBonus() - InitialRechargeBonus());
	pBuffer->CalculateVisibility();

	for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
	{
		for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
		{
			if (GetDigitanksPlayer()->HasDownloadedUpdate(x, y))
				pBuffer->InstallUpdate(x, y);
		}
	}

	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		m_ahChildren[i]->SetSupplier(pBuffer);
		pBuffer->AddChild(m_ahChildren[i]);
	}

	Delete();

	GetDigitanksPlayer()->AddActionItem(pBuffer, ACTIONTYPE_UPGRADE);
}
