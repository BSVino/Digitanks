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
			pHUD->SetButton5Listener(CHUD::CancelUpgrade);
			pHUD->SetButton5Texture(s_iCancelIcon);
			pHUD->SetButton5Help("Cancel\nUpgrade");
			pHUD->SetButton5Color(Color(100, 0, 0));
		}
		else
		{
			pHUD->SetButton1Listener(CHUD::BeginUpgrade);
			pHUD->SetButton1Texture(0);
			pHUD->SetButton1Help("Upgrade to\nPSU");
			pHUD->SetButton1Color(Color(150, 150, 150));

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

	DigitanksGame()->GetCurrentTeam()->SetCurrentSelection(pCollector);
}
