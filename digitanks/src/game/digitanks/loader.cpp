#include "loader.h"

#include <sstream>

#include <mtrand.h>

#include <renderer/renderer.h>
#include <game/game.h>

#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <ui/hud.h>

#include "mechinf.h"
#include "maintank.h"
#include "artillery.h"

#include <GL/glew.h>

size_t g_aiTurnsToLoad[] = {
	25, // BUILDUNIT_INFANTRY,
	50, // BUILDUNIT_TANK,
	60, // BUILDUNIT_ARTILLERY,
};

void CLoader::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/loader-infantry.obj");
	PrecacheModel(L"models/structures/loader-main.obj");
	PrecacheModel(L"models/structures/loader-artillery.obj");
}

void CLoader::Spawn()
{
	BaseClass::Spawn();

	m_bProducing = false;
}

void CLoader::StartTurn()
{
	BaseClass::StartTurn();

	if (m_bProducing && m_hSupplier != NULL)
	{
		m_iProductionStored += (size_t)(GetDigitanksTeam()->GetProductionPerLoader() * m_hSupplier->GetChildEfficiency());
		if (m_iProductionStored > g_aiTurnsToLoad[GetBuildUnit()])
		{
			CDigitank* pTank;
			if (GetBuildUnit() == BUILDUNIT_INFANTRY)
				pTank = Game()->Create<CMechInfantry>("CMechInfantry");
			else if (GetBuildUnit() == BUILDUNIT_TANK)
				pTank = Game()->Create<CMainBattleTank>("CMainBattleTank");
			else if (GetBuildUnit() == BUILDUNIT_ARTILLERY)
				pTank = Game()->Create<CArtillery>("CArtillery");
			
			float y = RandomFloat(0, 360);
			pTank->SetOrigin(DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + AngleVector(EAngle(0, y, 0)) * 10));

			// Face him toward the center.
			pTank->SetAngles(EAngle(0, VectorAngles(-GetOrigin().Normalized()).y, 0));

			GetTeam()->AddEntity(pTank);

			m_bProducing = false;

			if (GetBuildUnit() == BUILDUNIT_INFANTRY)
				DigitanksGame()->AppendTurnInfo("Production finished on Mechanized Infantry");
			else if (GetBuildUnit() == BUILDUNIT_TANK)
				DigitanksGame()->AppendTurnInfo("Production finished on Main Battle Tank");
			else if (GetBuildUnit() == BUILDUNIT_ARTILLERY)
				DigitanksGame()->AppendTurnInfo("Production finished on Artillery");

			GetDigitanksTeam()->SetCurrentSelection(pTank);
		}
		else
		{
			std::stringstream s;
			if (GetBuildUnit() == BUILDUNIT_INFANTRY)
				s << "Producing Mechanized Infantry (" << GetTurnsToProduce() << " turns left)";
			else if (GetBuildUnit() == BUILDUNIT_TANK)
				s << "Producing Main Battle Tank (" << GetTurnsToProduce() << " turns left)";
			else if (GetBuildUnit() == BUILDUNIT_ARTILLERY)
				s << "Producing Artillery (" << GetTurnsToProduce() << " turns left)";
			DigitanksGame()->AppendTurnInfo(s.str().c_str());
		}
	}
}

void CLoader::SetupMenu(menumode_t eMenuMode)
{
	if (IsConstructing())
	{
		// Base class is empty.
		BaseClass::SetupMenu(eMenuMode);
		return;
	}

	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	if (m_bProducing)
		pHUD->SetButton1Listener(CHUD::CancelBuildUnit);
	else if (HasEnoughFleetPoints())
		pHUD->SetButton1Listener(CHUD::BuildUnit);
	else
		pHUD->SetButton1Listener(NULL);
	pHUD->SetButton2Listener(NULL);
	pHUD->SetButton3Listener(NULL);
	pHUD->SetButton4Listener(NULL);
	pHUD->SetButton5Listener(NULL);

	if (m_bProducing)
		pHUD->SetButton1Help("Cancel\nBuild");
	else
	{
		if (GetBuildUnit() == BUILDUNIT_INFANTRY)
			pHUD->SetButton1Help("Build\nMech. Inf");
		else if (GetBuildUnit() == BUILDUNIT_TANK)
			pHUD->SetButton1Help("Build\nMain Tank");
		else
			pHUD->SetButton1Help("Build\nArtillery Tank");
	}
	pHUD->SetButton2Help("");
	pHUD->SetButton3Help("");
	pHUD->SetButton4Help("");
	pHUD->SetButton5Help("");

	pHUD->SetButton1Texture(0);
	pHUD->SetButton2Texture(0);
	pHUD->SetButton3Texture(0);
	pHUD->SetButton4Texture(0);
	pHUD->SetButton5Texture(0);

	if (HasEnoughFleetPoints())
		pHUD->SetButton1Color(Color(150, 150, 150));
	else
		pHUD->SetButton1Color(glgui::g_clrBox);
	pHUD->SetButton2Color(glgui::g_clrBox);
	pHUD->SetButton3Color(glgui::g_clrBox);
	pHUD->SetButton4Color(glgui::g_clrBox);
	pHUD->SetButton5Color(glgui::g_clrBox);
}

void CLoader::BeginProduction()
{
	if (IsConstructing())
		return;

	if (!HasEnoughFleetPoints())
		return;

	m_iProductionStored = 0;
	m_bProducing = true;

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_PRODUCING_UNITS);

	size_t iTutorial = CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial();
	if (iTutorial == CInstructor::TUTORIAL_PRODUCING_UNITS)
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();

	GetDigitanksTeam()->CountFleetPoints();
}

void CLoader::CancelProduction()
{
	m_iProductionStored = 0;
	m_bProducing = false;

	GetDigitanksTeam()->CountFleetPoints();
}

size_t CLoader::GetFleetPointsRequired()
{
	if (m_eBuildUnit == BUILDUNIT_INFANTRY)
		return CMechInfantry::InfantryFleetPoints();
	else if (m_eBuildUnit == BUILDUNIT_ARTILLERY)
		return CArtillery::ArtilleryFleetPoints();
	else if (m_eBuildUnit == BUILDUNIT_TANK)
		return CMainBattleTank::MainTankFleetPoints();

	return 0;
}

bool CLoader::HasEnoughFleetPoints()
{
	return GetFleetPointsRequired() <= GetDigitanksTeam()->GetUnusedFleetPoints();
}

size_t CLoader::GetTurnsToProduce()
{
	if (m_hSupplier == NULL)
		return -1;

	size_t iPerTurn = (size_t)(GetDigitanksTeam()->GetProductionPerLoader() * m_hSupplier->GetChildEfficiency());
	return (size_t)((g_aiTurnsToLoad[GetBuildUnit()]-m_iProductionStored)/iPerTurn)+1;
}

void CLoader::SetBuildUnit(buildunit_t eBuildUnit)
{
	m_eBuildUnit = eBuildUnit;

	switch (m_eBuildUnit)
	{
	case BUILDUNIT_INFANTRY:
		SetModel(L"models/structures/loader-infantry.obj");
		break;

	case BUILDUNIT_TANK:
		SetModel(L"models/structures/loader-main.obj");
		break;

	case BUILDUNIT_ARTILLERY:
		SetModel(L"models/structures/loader-artillery.obj");
		break;
	}
}

void CLoader::UpdateInfo(std::string& sInfo)
{
	std::stringstream s;

	if (GetBuildUnit() == BUILDUNIT_INFANTRY)
		s << "MECH. INFANTRY LOADER\n";
	else if (GetBuildUnit() == BUILDUNIT_TANK)
		s << "MAIN BATTLE TANK LOADER\n";
	else
		s << "ARTILLERY LOADER\n";

	s << "Unit producer\n \n";

	if (IsConstructing())
	{
		s << "(Constructing)\n";
		s << "Power to build: " << GetProductionToConstruct() << "\n";
		s << "Turns left: " << GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	if (IsProducing() && GetSupplier())
	{
		s << "(Producing)\n";
		size_t iProduction = (size_t)(GetDigitanksTeam()->GetProductionPerLoader() * GetSupplier()->GetChildEfficiency());
		size_t iProductionLeft = g_aiTurnsToLoad[GetBuildUnit()] - m_iProductionStored;
		s << "Power to build: " << iProductionLeft << "\n";
		s << "Turns left: " << (iProductionLeft/iProduction)+1 << "\n \n";
	}

	s << "Efficiency: " << (int)(m_hSupplier->GetChildEfficiency()*100) << "%\n";

	sInfo = s.str();
}

const char* CLoader::GetName()
{
	if (GetBuildUnit() == BUILDUNIT_INFANTRY)
		return "Mechanized Infantry Loader";
	else if (GetBuildUnit() == BUILDUNIT_TANK)
		return "Main Battle Tank Loader";
	else
		return "Artillery Loader";
}

unittype_t CLoader::GetUnitType()
{
	if (GetBuildUnit() == BUILDUNIT_INFANTRY)
		return UNIT_INFANTRY;
	else if (GetBuildUnit() == BUILDUNIT_TANK)
		return UNIT_TANK;
	else
		return UNIT_ARTILLERY;
}
