#include "collector.h"

#include <sstream>

#include <renderer/renderer.h>
#include <ui/digitankswindow.h>
#include <ui/hud.h>

#include "digitanksteam.h"
#include "digitanksgame.h"

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

void CCollector::UpdateInfo(std::wstring& sInfo)
{
	std::wstringstream s;

	s << L"POWER SUPPLY UNIT\n";
	s << L"Resource collector\n \n";

	if (IsConstructing())
	{
		s << L"(Constructing)\n";
		s << L"Turns left: " << GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	s << L"Power supplied: " << (size_t)(GetProduction() * m_hSupplier->GetChildEfficiency()) << L"\n";
	s << L"Efficiency: " << (int)(m_hSupplier->GetChildEfficiency()*100) << L"%\n";

	sInfo = s.str();
}

size_t CCollector::GetProduction()
{
	if (m_hSupplyLine == NULL)
		return 0;

	return (size_t)(4 * m_hSupplyLine->GetIntegrity());
}

size_t CBattery::s_iCancelIcon = 0;

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
}

void CBattery::SetupMenu(menumode_t eMenuMode)
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
			s << "UPGRADE TO POWER SUPPLY UNIT\n \n"
				<< "Power Supply Units provide 2 additional Power per turn. Upgrading will make this structure inactive until the upgrade is complete.\n \n"
				<< "Turns to upgrade: " << GetTurnsToUpgrade() << " Turns";

			pHUD->SetButtonInfo(0, s.str().c_str());
		}
	}
}

void CBattery::UpdateInfo(std::wstring& sInfo)
{
	std::wstringstream s;

	s << L"BATTERY\n";
	s << L"Resource collector\n \n";

	if (IsConstructing())
	{
		s << L"(Constructing)\n";
		s << L"Turns left: " << GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	if (IsUpgrading())
	{
		s << L"(Upgrading to Power Supply Unit)\n";
		s << L"Power to upgrade: " << GetProductionToUpgrade() << "\n";
		s << L"Turns left: " << GetTurnsToUpgrade() << "\n";
		sInfo = s.str();
		return;
	}

	if (m_hSupplier != NULL)
	{
		s << L"Power supplied: " << (size_t)(GetProduction() * m_hSupplier->GetChildEfficiency()) << L"\n";
		s << L"Efficiency: " << (int)(m_hSupplier->GetChildEfficiency()*100) << L"%\n";
	}

	sInfo = s.str();
}

bool CBattery::CanStructureUpgrade()
{
	return GetDigitanksTeam()->CanBuildPSUs();
}

void CBattery::UpgradeComplete()
{
	CCollector* pCollector = DigitanksGame()->Create<CCollector>("CCollector");
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
