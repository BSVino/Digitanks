#include "buffer.h"

#include <sstream>

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
	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	if (IsInstalling())
	{
		pHUD->SetButton1Listener(NULL);
		pHUD->SetButton2Listener(NULL);
		pHUD->SetButton3Listener(NULL);
		pHUD->SetButton4Listener(NULL);
		pHUD->SetButton5Listener(CHUD::CancelInstall);

		pHUD->SetButton1Texture(0);
		pHUD->SetButton2Texture(0);
		pHUD->SetButton3Texture(0);
		pHUD->SetButton4Texture(0);
		pHUD->SetButton5Texture(s_iCancelIcon);

		pHUD->SetButton1Help("");
		pHUD->SetButton2Help("");
		pHUD->SetButton3Help("");
		pHUD->SetButton4Help("");
		pHUD->SetButton5Help("Cancel\nInstall");

		pHUD->SetButton1Color(glgui::g_clrBox);
		pHUD->SetButton2Color(glgui::g_clrBox);
		pHUD->SetButton3Color(glgui::g_clrBox);
		pHUD->SetButton4Color(glgui::g_clrBox);
		pHUD->SetButton5Color(Color(100, 0, 0));
	}
	else if (eMenuMode == MENUMODE_INSTALL)
	{
		if (GetFirstUninstalledUpdate(UPDATETYPE_BANDWIDTH) >= 0)
		{
			pHUD->SetButton1Listener(CHUD::InstallBandwidth);
			pHUD->SetButton1Texture(s_iInstallBandwidthIcon);
			pHUD->SetButton1Help("Install\nBandwidth");
			pHUD->SetButton1Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton1Listener(NULL);
			pHUD->SetButton1Texture(0);
			pHUD->SetButton1Help("");
			pHUD->SetButton1Color(glgui::g_clrBox);
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_FLEETSUPPLY) >= 0)
		{
			pHUD->SetButton2Listener(CHUD::InstallFleetSupply);
			pHUD->SetButton2Texture(s_iInstallFleetSupplyIcon);
			pHUD->SetButton2Help("Install\nFleet Sply");
			pHUD->SetButton2Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton2Listener(NULL);
			pHUD->SetButton2Texture(0);
			pHUD->SetButton2Help("");
			pHUD->SetButton2Color(glgui::g_clrBox);
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTENERGY) >= 0)
		{
			pHUD->SetButton3Listener(CHUD::InstallEnergyBonus);
			pHUD->SetButton3Texture(s_iInstallEnergyBonusIcon);
			pHUD->SetButton3Help("Install\nEnrg Bonus");
			pHUD->SetButton3Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton3Listener(NULL);
			pHUD->SetButton3Texture(0);
			pHUD->SetButton3Help("");
			pHUD->SetButton3Color(glgui::g_clrBox);
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTRECHARGE) >= 0)
		{
			pHUD->SetButton4Listener(CHUD::InstallRechargeBonus);
			pHUD->SetButton4Texture(s_iInstallRechargeBonusIcon);
			pHUD->SetButton4Help("Install\nRchg Bonus");
			pHUD->SetButton4Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton4Listener(NULL);
			pHUD->SetButton4Texture(0);
			pHUD->SetButton4Help("");
			pHUD->SetButton4Color(glgui::g_clrBox);
		}

		pHUD->SetButton5Listener(CHUD::GoToMain);
		pHUD->SetButton5Texture(s_iCancelIcon);
		pHUD->SetButton5Help("Return");
		pHUD->SetButton5Color(Color(100, 0, 0));
	}
	else
	{
		pHUD->SetButton1Listener(NULL);
		pHUD->SetButton1Help("");
		pHUD->SetButton1Texture(0);
		pHUD->SetButton1Color(glgui::g_clrBox);

		pHUD->SetButton2Listener(NULL);
		pHUD->SetButton2Help("");
		pHUD->SetButton2Texture(0);
		pHUD->SetButton2Color(glgui::g_clrBox);

		pHUD->SetButton3Listener(NULL);
		pHUD->SetButton3Help("");
		pHUD->SetButton3Texture(0);
		pHUD->SetButton3Color(glgui::g_clrBox);

		if (HasUpdatesAvailable())
		{
			pHUD->SetButton4Listener(CHUD::InstallMenu);
			pHUD->SetButton4Help("Install\nUpdates");
			pHUD->SetButton4Texture(s_iInstallIcon);
			pHUD->SetButton4Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton4Listener(NULL);
			pHUD->SetButton4Help("");
			pHUD->SetButton4Texture(0);
			pHUD->SetButton4Color(glgui::g_clrBox);
		}

		pHUD->SetButton5Listener(NULL);
		pHUD->SetButton5Help("");
		pHUD->SetButton5Texture(0);
		pHUD->SetButton5Color(glgui::g_clrBox);
	}
}

void CBuffer::UpdateInfo(std::string& sInfo)
{
	std::stringstream s;

	s << "BUFFER INFO\n";
	s << "Network extender\n \n";

	if (IsConstructing())
	{
		s << "(Constructing)\n";
		s << "Power to build: " << GetProductionToConstruct() << "\n";
		s << "Turns left: " << GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	s << "Strength: " << m_iDataStrength << "\n";
	s << "Growth: " << (int)GetDataFlowRate() << "\n";
	s << "Size: " << (int)GetDataFlowRadius() << "\n";
	s << "Efficiency: " << (int)(GetChildEfficiency()*100) << "%\n";

	sInfo = s.str();
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
