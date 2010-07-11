#include "loader.h"

#include <sstream>

#include <renderer/renderer.h>
#include <game/game.h>

#include <ui/digitankswindow.h>
#include <ui/hud.h>

#include "mechinf.h"
#include "maintank.h"
#include "artillery.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

REGISTER_ENTITY(CLoader);

size_t g_aiTurnsToLoad[] = {
	40, // BUILDUNIT_INFANTRY,
	80, // BUILDUNIT_TANK,
	100, // BUILDUNIT_ARTILLERY,
};

void CLoader::Spawn()
{
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
			
			float y = RemapVal((float)(rand()%1000), 0, 1000, 0, 360);
			pTank->SetOrigin(DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + AngleVector(EAngle(0, y, 0)) * 10));

			GetTeam()->AddEntity(pTank);

			m_bProducing = false;

			if (GetBuildUnit() == BUILDUNIT_INFANTRY)
				DigitanksGame()->AppendTurnInfo("Production finished on Mechanized Infantry");
			else if (GetBuildUnit() == BUILDUNIT_TANK)
				DigitanksGame()->AppendTurnInfo("Production finished on Main Battle Tank");
			else if (GetBuildUnit() == BUILDUNIT_ARTILLERY)
				DigitanksGame()->AppendTurnInfo("Production finished on Artillery");
		}
		else
		{
			size_t iPerTurn = (size_t)(GetDigitanksTeam()->GetProductionPerLoader() * m_hSupplier->GetChildEfficiency());
			std::stringstream s;
			if (GetBuildUnit() == BUILDUNIT_INFANTRY)
				s << "Producing Mechanized Infantry (" << (m_iProductionStored/iPerTurn) << " turns left)";
			else if (GetBuildUnit() == BUILDUNIT_TANK)
				s << "Producing Main Battle Tank (" << (m_iProductionStored/iPerTurn) << " turns left)";
			else if (GetBuildUnit() == BUILDUNIT_ARTILLERY)
				s << "Producing Artillery (" << (m_iProductionStored/iPerTurn) << " turns left)";
		}
	}
}

void CLoader::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	if (m_bProducing)
		pHUD->SetButton1Listener(CHUD::CancelBuildUnit);
	else
		pHUD->SetButton1Listener(CHUD::BuildUnit);
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
		else
			pHUD->SetButton1Help("Build\nMain Tank");
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

	pHUD->SetButton1Color(Color(150, 150, 150));
	pHUD->SetButton2Color(glgui::g_clrBox);
	pHUD->SetButton3Color(glgui::g_clrBox);
	pHUD->SetButton4Color(glgui::g_clrBox);
	pHUD->SetButton5Color(glgui::g_clrBox);
}

void CLoader::BeginProduction()
{
	if (IsConstructing())
		return;

	m_iProductionStored = 0;
	m_bProducing = true;
}

void CLoader::CancelProduction()
{
	m_iProductionStored = 0;
	m_bProducing = false;
}

void CLoader::OnRender()
{
	if (GetVisibility() == 0)
		return;

	glutSolidCube(6);
}

void CLoader::UpdateInfo(std::string& sInfo)
{
	std::stringstream s;

	if (GetBuildUnit() == BUILDUNIT_INFANTRY)
		s << "MECH. INFANTRY LOADER\n";
	else
		s << "MAIN BATTLE TANK LOADER\n";

	s << "Unit producer\n \n";

	if (IsConstructing())
	{
		s << "(Constructing)\n";
		s << "Production left: " << GetProductionRemaining() << "\n";
		s << "Turns left: " << GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	if (IsProducing())
	{
		s << "(Producing)\n";
		size_t iProduction = (size_t)(GetDigitanksTeam()->GetProductionPerLoader() * m_hSupplier->GetChildEfficiency());
		size_t iProductionLeft = g_aiTurnsToLoad[GetBuildUnit()] - m_iProductionStored;
		s << "Production left: " << iProductionLeft << "\n";
		s << "Turns left: " << (iProductionLeft/iProduction)+1 << "\n \n";
	}

	s << "Efficiency: " << (int)(m_hSupplier->GetChildEfficiency()*100) << "%\n";

	sInfo = s.str();
}

const char* CLoader::GetName()
{
	if (GetBuildUnit() == BUILDUNIT_INFANTRY)
		return "Mechanized Infantry Loader";
	else
		return "Main Battle Tank Loader";
}