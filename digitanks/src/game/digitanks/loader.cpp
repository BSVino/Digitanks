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

size_t CLoader::s_iCancelIcon = 0;
size_t CLoader::s_iInstallIcon = 0;
size_t CLoader::s_iInstallAttackIcon = 0;
size_t CLoader::s_iInstallDefenseIcon = 0;
size_t CLoader::s_iInstallMovementIcon = 0;
size_t CLoader::s_iInstallRangeIcon = 0;
size_t CLoader::s_iInstallHealthIcon = 0;
size_t CLoader::s_iBuildInfantryIcon = 0;
size_t CLoader::s_iBuildTankIcon = 0;
size_t CLoader::s_iBuildArtilleryIcon = 0;

void CLoader::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/loader-infantry.obj");
	PrecacheModel(L"models/structures/loader-main.obj");
	PrecacheModel(L"models/structures/loader-artillery.obj");

	s_iCancelIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-cancel.png");
	s_iInstallIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install.png");
	s_iInstallAttackIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-attack.png");
	s_iInstallDefenseIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-defense.png");
	s_iInstallMovementIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-movement.png");
	s_iInstallRangeIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-range.png");
	s_iInstallHealthIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-health.png");
	s_iBuildInfantryIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-infantry.png");
	s_iBuildTankIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-tank.png");
	s_iBuildArtilleryIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-artillery.png");
}

void CLoader::Spawn()
{
	BaseClass::Spawn();

	m_bProducing = false;

	m_iTankAttack = m_iTankDefense = m_iTankMovement = m_iTankHealth = m_iTankRange = 0;
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

			for (size_t i = 0; i < m_iTankAttack; i++)
			{
				pTank->GiveBonusPoints(1, false);
				pTank->PromoteAttack();
			}

			for (size_t i = 0; i < m_iTankDefense; i++)
			{
				pTank->GiveBonusPoints(1, false);
				pTank->PromoteDefense();
			}

			for (size_t i = 0; i < m_iTankMovement; i++)
			{
				pTank->GiveBonusPoints(1, false);
				pTank->PromoteMovement();
			}

			pTank->SetTotalHealth(pTank->GetTotalHealth()+m_iTankHealth);
			pTank->AddRangeBonus((float)m_iTankRange);

			m_bProducing = false;

			if (GetBuildUnit() == BUILDUNIT_INFANTRY)
				DigitanksGame()->AppendTurnInfo(L"Production finished on Mechanized Infantry");
			else if (GetBuildUnit() == BUILDUNIT_TANK)
				DigitanksGame()->AppendTurnInfo(L"Production finished on Main Battle Tank");
			else if (GetBuildUnit() == BUILDUNIT_ARTILLERY)
				DigitanksGame()->AppendTurnInfo(L"Production finished on Artillery");

			GetDigitanksTeam()->SetCurrentSelection(pTank);
		}
		else
		{
			std::wstringstream s;
			if (GetBuildUnit() == BUILDUNIT_INFANTRY)
				s << L"Producing Mechanized Infantry (" << GetTurnsToProduce() << L" turns left)";
			else if (GetBuildUnit() == BUILDUNIT_TANK)
				s << L"Producing Main Battle Tank (" << GetTurnsToProduce() << L" turns left)";
			else if (GetBuildUnit() == BUILDUNIT_ARTILLERY)
				s << L"Producing Artillery (" << GetTurnsToProduce() << L" turns left)";
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

	if (IsInstalling())
	{
		pHUD->SetButton5Listener(CHUD::CancelInstall);
		pHUD->SetButton5Texture(s_iCancelIcon);
		pHUD->SetButton5Help("Cancel\nInstall");
		pHUD->SetButton5Color(Color(100, 0, 0));
	}
	else if (eMenuMode == MENUMODE_INSTALL)
	{
		if (GetFirstUninstalledUpdate(UPDATETYPE_TANKATTACK) >= 0)
		{
			pHUD->SetButton1Listener(CHUD::InstallTankAttack);
			pHUD->SetButton1Texture(s_iInstallAttackIcon);
			pHUD->SetButton1Help("Install\nAttack");
			pHUD->SetButton1Color(Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_TANKATTACK);
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_TANKATTACK][iUpdate];

			std::wstringstream s;
			s << "INSTALL ATTACK ENERGY INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Attack Energy increase: " << pUpdate->m_flValue << "0%\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(0, s.str().c_str());
		}

		if (GetUnitType() == STRUCTURE_ARTILLERYLOADER && GetFirstUninstalledUpdate(UPDATETYPE_TANKRANGE) >= 0)
		{
			pHUD->SetButton2Listener(CHUD::InstallTankRange);
			pHUD->SetButton2Texture(s_iInstallRangeIcon);
			pHUD->SetButton2Help("Install\nRange");
			pHUD->SetButton2Color(Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_TANKRANGE);
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_TANKRANGE][iUpdate];

			std::wstringstream s;
			s << "INSTALL ATTACK RANGE INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Attack Range increase: " << pUpdate->m_flValue << " Units\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(1, s.str().c_str());
		}
		else if (GetUnitType() != STRUCTURE_ARTILLERYLOADER && GetFirstUninstalledUpdate(UPDATETYPE_TANKDEFENSE) >= 0)
		{
			pHUD->SetButton2Listener(CHUD::InstallTankDefense);
			pHUD->SetButton2Texture(s_iInstallDefenseIcon);
			pHUD->SetButton2Help("Install\nDefense");
			pHUD->SetButton2Color(Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_TANKDEFENSE);
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_TANKDEFENSE][iUpdate];

			std::wstringstream s;
			s << "INSTALL DEFENSE ENERGY INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Defense Energy increase: " << pUpdate->m_flValue << "0%\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(1, s.str().c_str());
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_TANKMOVEMENT) >= 0)
		{
			pHUD->SetButton3Listener(CHUD::InstallTankMovement);
			pHUD->SetButton3Texture(s_iInstallMovementIcon);
			pHUD->SetButton3Help("Install\nMovement");
			pHUD->SetButton3Color(Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_TANKATTACK);
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_TANKATTACK][iUpdate];

			std::wstringstream s;
			s << "INSTALL MOVEMENT ENERGY INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Movement Energy increase: " << pUpdate->m_flValue << "0%\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(2, s.str().c_str());
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_TANKHEALTH) >= 0)
		{
			pHUD->SetButton4Listener(CHUD::InstallTankHealth);
			pHUD->SetButton4Texture(s_iInstallHealthIcon);
			pHUD->SetButton4Help("Install\nHealth");
			pHUD->SetButton4Color(Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_TANKATTACK);
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_TANKATTACK][iUpdate];

			std::wstringstream s;
			s << "INSTALL HEALTH INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Health increase: " << pUpdate->m_flValue << " Hull Strength\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(3, s.str().c_str());
		}

		pHUD->SetButton5Listener(CHUD::GoToMain);
		pHUD->SetButton5Texture(s_iCancelIcon);
		pHUD->SetButton5Help("Return");
		pHUD->SetButton5Color(Color(100, 0, 0));
	}
	else if (m_bProducing)
	{
		pHUD->SetButton5Listener(CHUD::CancelBuildUnit);
		pHUD->SetButton5Help("Cancel\nBuild");
		pHUD->SetButton5Texture(s_iCancelIcon);
		pHUD->SetButton5Color(Color(100, 0, 0));
	}
	else
	{
		if (HasEnoughFleetPoints())
			pHUD->SetButton1Listener(CHUD::BuildUnit);

		if (GetBuildUnit() == BUILDUNIT_INFANTRY)
		{
			pHUD->SetButton1Help("Build\nMech. Inf");
			pHUD->SetButton1Texture(s_iBuildInfantryIcon);
		}
		else if (GetBuildUnit() == BUILDUNIT_TANK)
		{
			pHUD->SetButton1Help("Build\nMain Tank");
			pHUD->SetButton1Texture(s_iBuildTankIcon);
		}
		else
		{
			pHUD->SetButton1Help("Build\nArtillery Tank");
			pHUD->SetButton1Texture(s_iBuildArtilleryIcon);
		}

		if (HasEnoughFleetPoints())
			pHUD->SetButton1Color(Color(150, 150, 150));

		if (HasUpdatesAvailable())
		{
			pHUD->SetButton4Listener(CHUD::InstallMenu);
			pHUD->SetButton4Help("Install\nUpdates");
			pHUD->SetButton4Texture(s_iInstallIcon);
			pHUD->SetButton4Color(Color(150, 150, 150));
		}
	}
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
	GetDigitanksTeam()->CountProducers();
}

void CLoader::CancelProduction()
{
	m_iProductionStored = 0;
	m_bProducing = false;

	GetDigitanksTeam()->CountFleetPoints();
}

void CLoader::InstallUpdate(updatetype_t eUpdate)
{
	if (m_bProducing)
		return;

	BaseClass::InstallUpdate(eUpdate);
}

void CLoader::InstallComplete()
{
	BaseClass::InstallComplete();

	CUpdateItem* pUpdate = m_apUpdates[m_eInstallingType][m_iInstallingUpdate];

	switch (pUpdate->m_eUpdateType)
	{
	case UPDATETYPE_TANKATTACK:
		m_iTankAttack += (size_t)pUpdate->m_flValue;
		break;

	case UPDATETYPE_TANKDEFENSE:
		m_iTankDefense += (size_t)pUpdate->m_flValue;
		break;

	case UPDATETYPE_TANKMOVEMENT:
		m_iTankMovement += (size_t)pUpdate->m_flValue;
		break;

	case UPDATETYPE_TANKHEALTH:
		m_iTankHealth += (size_t)pUpdate->m_flValue;
		break;

	case UPDATETYPE_TANKRANGE:
		m_iTankRange += (size_t)pUpdate->m_flValue;
		break;
	}

	GetDigitanksTeam()->SetCurrentSelection(this);
}

bool CLoader::HasUpdatesAvailable()
{
	if (GetFirstUninstalledUpdate(UPDATETYPE_TANKATTACK) >= 0)
		return true;

	if (GetFirstUninstalledUpdate(UPDATETYPE_TANKDEFENSE) >= 0)
		return true;

	if (GetFirstUninstalledUpdate(UPDATETYPE_TANKMOVEMENT) >= 0)
		return true;

	if (GetFirstUninstalledUpdate(UPDATETYPE_TANKHEALTH) >= 0)
		return true;

	if (m_eBuildUnit == BUILDUNIT_ARTILLERY && GetFirstUninstalledUpdate(UPDATETYPE_TANKRANGE) >= 0)
		return true;

	return false;
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

void CLoader::UpdateInfo(std::wstring& sInfo)
{
	std::wstringstream s;

	if (GetBuildUnit() == BUILDUNIT_INFANTRY)
		s << L"MECH. INFANTRY LOADER\n";
	else if (GetBuildUnit() == BUILDUNIT_TANK)
		s << L"MAIN BATTLE TANK LOADER\n";
	else
		s << L"ARTILLERY LOADER\n";

	s << L"Unit producer\n \n";

	if (IsConstructing())
	{
		s << L"(Constructing)\n";
		s << L"Power to build: " << GetProductionToConstruct() << L"\n";
		s << L"Turns left: " << GetTurnsToConstruct() << L"\n";
		sInfo = s.str();
		return;
	}

	if (IsProducing() && GetSupplier())
	{
		s << L"(Producing)\n";
		size_t iProduction = (size_t)(GetDigitanksTeam()->GetProductionPerLoader() * GetSupplier()->GetChildEfficiency());
		size_t iProductionLeft = g_aiTurnsToLoad[GetBuildUnit()] - m_iProductionStored;
		s << L"Power to build: " << iProductionLeft << L"\n";
		s << L"Turns left: " << (iProductionLeft/iProduction)+1 << L"\n \n";
	}

	if (IsInstalling())
	{
		s << L"[Installing update '" << GetUpdateInstalling()->GetName() << L"'...]\n";
		s << L"Power to install: " << GetProductionToInstall() << L"\n";
		s << L"Turns left: " << GetTurnsToInstall() << L"\n";
		sInfo = s.str();
	}

	s << L"Efficiency: " << (int)(m_hSupplier->GetChildEfficiency()*100) << L"%\n";

	sInfo = s.str();
}

const wchar_t* CLoader::GetName()
{
	if (GetBuildUnit() == BUILDUNIT_INFANTRY)
		return L"Mechanized Infantry Loader";
	else if (GetBuildUnit() == BUILDUNIT_TANK)
		return L"Main Battle Tank Loader";
	else
		return L"Artillery Loader";
}

unittype_t CLoader::GetUnitType()
{
	if (GetBuildUnit() == BUILDUNIT_INFANTRY)
		return STRUCTURE_INFANTRYLOADER;
	else if (GetBuildUnit() == BUILDUNIT_TANK)
		return STRUCTURE_TANKLOADER;
	else
		return STRUCTURE_ARTILLERYLOADER;
}

size_t CLoader::GetUnitProductionCost(buildunit_t eBuildUnit)
{
	return g_aiTurnsToLoad[eBuildUnit];
}
