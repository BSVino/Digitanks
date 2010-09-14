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
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_BANDWIDTH][iUpdate];

			std::wstringstream s;
			s << "INSTALL BANDWIDTH INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Bandwidth increase: " << pUpdate->m_flValue << " mbps\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(0, s.str().c_str());
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_FLEETSUPPLY) >= 0)
		{
			pHUD->SetButtonListener(1, CHUD::InstallFleetSupply);
			pHUD->SetButtonTexture(1, s_iInstallFleetSupplyIcon);
			pHUD->SetButtonColor(1, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_FLEETSUPPLY);
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_FLEETSUPPLY][iUpdate];

			std::wstringstream s;
			s << "INSTALL FLEET SUPPLY INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Fleet Supply increase: " << pUpdate->m_flValue << "\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(1, s.str().c_str());
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTENERGY) >= 0)
		{
			pHUD->SetButtonListener(2, CHUD::InstallEnergyBonus);
			pHUD->SetButtonTexture(2, s_iInstallEnergyBonusIcon);
			pHUD->SetButtonColor(2, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTENERGY);
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_SUPPORTENERGY][iUpdate];

			std::wstringstream s;
			s << "INSTALL SUPPORT ENERGY INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Attack Energy Buff increase: +" << pUpdate->m_flValue << "\n"
				<< "Defense Energy Buff increase: +" << pUpdate->m_flValue << "\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(2, s.str().c_str());
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTRECHARGE) >= 0)
		{
			pHUD->SetButtonListener(3, CHUD::InstallRechargeBonus);
			pHUD->SetButtonTexture(3, s_iInstallRechargeBonusIcon);
			pHUD->SetButtonColor(3, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_SUPPORTRECHARGE);
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_SUPPORTRECHARGE][iUpdate];

			std::wstringstream s;
			s << "INSTALL HEALTH ENERGY INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Health Recharge Buff increase: +" << pUpdate->m_flValue/5 << " per turn\n"
				<< "Shield Recharge Buff increase: +" << pUpdate->m_flValue << " per turn\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(3, s.str().c_str());
		}

		pHUD->SetButtonListener(4, CHUD::GoToMain);
		pHUD->SetButtonTexture(4, s_iCancelIcon);
		pHUD->SetButtonColor(4, Color(100, 0, 0));
	}
	else
	{
		if (HasUpdatesAvailable())
		{
			pHUD->SetButtonListener(3, CHUD::InstallMenu);
			pHUD->SetButtonTexture(3, s_iInstallIcon);
			pHUD->SetButtonColor(3, Color(150, 150, 150));
		}
	}
}

void CBuffer::UpdateInfo(std::wstring& sInfo)
{
	std::wstringstream s;

	s << L"BUFFER INFO\n";
	s << L"Network extender\n \n";

	if (IsConstructing())
	{
		s << L"(Constructing)\n";
		s << L"Power to build: " << GetProductionToConstruct() << "\n";
		s << L"Turns left: " << GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	s << L"Strength: " << m_iDataStrength << "\n";
	s << L"Growth: " << (int)GetDataFlowRate() << "\n";
	s << L"Size: " << (int)GetDataFlowRadius() << "\n";
	s << L"Efficiency: " << (int)(GetChildEfficiency()*100) << "%\n";

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

size_t CMiniBuffer::s_iCancelIcon = 0;

void CMiniBuffer::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/minibuffer.obj");
}

void CMiniBuffer::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/minibuffer.obj");
}

void CMiniBuffer::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	if (!IsConstructing() && CanStructureUpgrade())
	{
		if (IsUpgrading())
		{
			pHUD->SetButtonListener(4, CHUD::CancelUpgrade);
			pHUD->SetButtonTexture(4, s_iCancelIcon);
			pHUD->SetButtonColor(4, Color(100, 0, 0));
		}
		else
		{
			pHUD->SetButtonListener(0, CHUD::BeginUpgrade);
			pHUD->SetButtonTexture(0, 0);
			pHUD->SetButtonColor(0, Color(150, 150, 150));

			std::wstringstream s;
			s << "UPGRADE TO BUFFER\n \n"
				<< "Buffers provide larger Network radius and can be updated by installing downloaded updates. Upgrading will make this structure inactive until the upgrade is complete.\n \n"
				<< "Turns to upgrade: " << GetTurnsToUpgrade() << " Turns";

			pHUD->SetButtonInfo(0, s.str().c_str());
		}
	}
}

void CMiniBuffer::UpdateInfo(std::wstring& sInfo)
{
	std::wstringstream s;

	s << L"MINIBUFFER INFO\n";
	s << L"Network extender\n \n";

	if (IsConstructing())
	{
		s << L"(Constructing)\n";
		s << L"Power to build: " << GetProductionToConstruct() << "\n";
		s << L"Turns left: " << GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	if (IsUpgrading())
	{
		s << L"(Upgrading to Buffer)\n";
		s << L"Power to upgrade: " << GetProductionToUpgrade() << "\n";
		s << L"Turns left: " << GetTurnsToUpgrade() << "\n";
		sInfo = s.str();
		return;
	}

	s << L"Strength: " << m_iDataStrength << "\n";
	s << L"Growth: " << (int)GetDataFlowRate() << "\n";
	s << L"Size: " << (int)GetDataFlowRadius() << "\n";
	s << L"Efficiency: " << (int)(GetChildEfficiency()*100) << "%\n";

	sInfo = s.str();
}

bool CMiniBuffer::CanStructureUpgrade()
{
	return GetDigitanksTeam()->CanBuildBuffers();
}

void CMiniBuffer::UpgradeComplete()
{
	CBuffer* pBuffer = DigitanksGame()->Create<CBuffer>("CBuffer");
	pBuffer->SetOrigin(GetOrigin());
	pBuffer->SetSupplier(GetSupplier());
	GetTeam()->AddEntity(pBuffer);
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

	DigitanksGame()->GetCurrentTeam()->SetCurrentSelection(pBuffer);
}
