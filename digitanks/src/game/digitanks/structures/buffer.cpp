#include "buffer.h"

#include <EASTL/string.h>

#include <ui/digitankswindow.h>
#include <ui/hud.h>
#include <game/team.h>

#include <renderer/renderer.h>
#include <game/game.h>

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

	SetModel(_T("models/structures/buffer.obj"));
}

void CBuffer::Precache()
{
	BaseClass::Precache();

	PrecacheModel(_T("models/structures/buffer.obj"));
}

bool CBuffer::AllowControlMode(controlmode_t eMode) const
{
	if (eMode == MODE_BUILD)
		return true;

	return BaseClass::AllowControlMode(eMode);
}

void CBuffer::SetupMenu(menumode_t eMenuMode)
{
	if (GetDigitanksTeam() && GetDigitanksTeam()->GetPrimaryCPU() != NULL)
		GetDigitanksTeam()->GetPrimaryCPU()->SetupMenu(eMenuMode);
}

void CBuffer::UpdateInfo(tstring& s)
{
	tstring p;

	s = _T("");
	s += _T("MACRO-BUFFER INFO\n");
	s += _T("Network extender\n \n");

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

	s += sprintf(tstring("Fleet Points: %d\n"), FleetPoints());
	s += sprintf(tstring("Bandwidth: %.1f/turn\n"), Bandwidth());
	s += sprintf(tstring("Network Size: %d\n"), (int)GetDataFlowRadius());
	s += sprintf(tstring("Efficiency: %d\n"), (int)(GetChildEfficiency()*100));
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

	SetModel(_T("models/structures/minibuffer.obj"));
}

void CMiniBuffer::Precache()
{
	BaseClass::Precache();

	PrecacheModel(_T("models/structures/minibuffer.obj"));
}

bool CMiniBuffer::AllowControlMode(controlmode_t eMode) const
{
	if (eMode == MODE_BUILD)
		return true;

	return BaseClass::AllowControlMode(eMode);
}

void CMiniBuffer::SetupMenu(menumode_t eMenuMode)
{
	if (GetDigitanksTeam()->GetPrimaryCPU() != NULL)
		GetDigitanksTeam()->GetPrimaryCPU()->SetupMenu(eMenuMode);

	CHUD* pHUD = DigitanksWindow()->GetHUD();
	tstring p;

	if (!IsConstructing() && !IsUpgrading() && CanStructureUpgrade())
	{
		pHUD->SetButtonListener(0, CHUD::BeginUpgrade);

		if (UpgradeCost() <= GetDigitanksTeam()->GetPower())
		{
			pHUD->SetButtonTexture(0, "MacroBuffer");
			pHUD->SetButtonColor(0, Color(150, 150, 150));
		}

		tstring s;
		s += _T("UPGRADE TO MACRO-BUFFER\n \n");
		s += _T("Macro-Buffers provide larger Network radius and can be updated by installing downloaded updates. Upgrading will make this structure inactive until the upgrade is complete.\n \n");
		s += sprintf(tstring("Turns to upgrade: %d Turns\n \n"), GetTurnsToUpgrade());
		s += _T("Shortcut: Q");

		pHUD->SetButtonInfo(0, s);
		pHUD->SetButtonTooltip(0, _T("Upgrade To Macro-Buffer"));
	}
}

void CMiniBuffer::UpdateInfo(tstring& s)
{
	tstring p;
	s = _T("");
	s += _T("BUFFER INFO\n");
	s += _T("Network extender\n \n");

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
		s += _T("(Upgrading to Macro-Buffer)\n");
		s += sprintf(tstring("Turns left: %d\n"), GetTurnsRemainingToUpgrade());
		return;
	}

	s += sprintf(tstring("Fleet Points: %d\n"), FleetPoints());
	s += sprintf(tstring("Bandwidth: %.1f/turn\n"), Bandwidth());
	s += sprintf(tstring("Network Size: %d\n"), (int)GetDataFlowRadius());
	s += sprintf(tstring("Efficiency: %d\n"), (int)(GetChildEfficiency()*100));
}

bool CMiniBuffer::CanStructureUpgrade()
{
	if (!GetDigitanksTeam())
		return false;

	return GetDigitanksTeam()->CanBuildBuffers();
}

void CMiniBuffer::UpgradeComplete()
{
	if (!GameNetwork()->IsHost())
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
	pBuffer->CalculateVisibility();

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

	GetDigitanksTeam()->AddActionItem(pBuffer, ACTIONTYPE_UPGRADE);
}
