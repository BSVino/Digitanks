#include "loader.h"

#include <sstream>

#include <mtrand.h>

#include <models/models.h>
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
	8, // BUILDUNIT_SCOUT,
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

NETVAR_TABLE_BEGIN(CLoader);
	NETVAR_DEFINE(buildunit_t, m_eBuildUnit);
	NETVAR_DEFINE(size_t, m_iBuildUnitModel);
	NETVAR_DEFINE(bool, m_bProducing);
	NETVAR_DEFINE(size_t, m_iProductionStored);
	NETVAR_DEFINE(size_t, m_iTankAttack);
	NETVAR_DEFINE(size_t, m_iTankDefense);
	NETVAR_DEFINE(size_t, m_iTankMovement);
	NETVAR_DEFINE(size_t, m_iTankHealth);
	NETVAR_DEFINE(size_t, m_iTankRange);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CLoader);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, buildunit_t, m_eBuildUnit);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iBuildUnitModel);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bProducing);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iProductionStored);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTankAttack);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTankDefense);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTankMovement);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTankHealth);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTankRange);
SAVEDATA_TABLE_END();

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

	m_iBuildUnitModel = ~0;
}

void CLoader::OnTeamChange()
{
	BaseClass::OnTeamChange();

	CancelProduction();
}

void CLoader::StartTurn()
{
	BaseClass::StartTurn();

	if (!m_bProducing)
		m_iProductionStored = 0;

	if (m_bProducing && m_hSupplier != NULL && m_hSupplyLine != NULL)
	{
		m_iProductionStored += (size_t)(GetDigitanksTeam()->GetProductionPerLoader() * m_hSupplier->GetChildEfficiency() * m_hSupplyLine->GetIntegrity());
		if (m_iProductionStored > g_aiTurnsToLoad[GetBuildUnit()])
		{
			if (GetBuildUnit() == BUILDUNIT_INFANTRY)
				DigitanksGame()->AppendTurnInfo(L"Production finished on Mechanized Infantry");
			else if (GetBuildUnit() == BUILDUNIT_TANK)
				DigitanksGame()->AppendTurnInfo(L"Production finished on Main Battle Tank");
			else if (GetBuildUnit() == BUILDUNIT_ARTILLERY)
				DigitanksGame()->AppendTurnInfo(L"Production finished on Artillery");

			//DigitanksGame()->AddActionItem(pTank, ACTIONTYPE_NEWUNIT);
			DigitanksGame()->AddActionItem(this, ACTIONTYPE_UNITREADY);

			if (CNetwork::IsHost())
			{
				CDigitank* pTank;
				if (GetBuildUnit() == BUILDUNIT_INFANTRY)
					pTank = GameServer()->Create<CMechInfantry>("CMechInfantry");
				else if (GetBuildUnit() == BUILDUNIT_TANK)
					pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
				else if (GetBuildUnit() == BUILDUNIT_ARTILLERY)
					pTank = GameServer()->Create<CArtillery>("CArtillery");
			
				pTank->SetOrigin(GetOrigin());

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

				// All of these StartTurn calls will probably cause problems later but for now they're
				// the only way to refresh the tank's energy so it has enough to leave the loader.
				pTank->StartTurn();

				pTank->SetPreviewMove(pTank->GetOrigin() + AngleVector(pTank->GetAngles())*9);
				pTank->Move();

				pTank->StartTurn();

				// Face him toward the center.
				pTank->SetPreviewTurn(VectorAngles(-GetOrigin().Normalized()).y);
				pTank->Turn();

				pTank->StartTurn();
			}
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

void CLoader::PostRender()
{
	BaseClass::PostRender();

	if (IsProducing() && GetVisibility() > 0)
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.Translate(GetOrigin());
		c.SetAlpha(GetVisibility() * 0.3f);
		c.SetBlend(BLEND_ADDITIVE);
		if (GetTeam())
			c.SetColorSwap(GetTeam()->GetColor());
		c.RenderModel(m_iBuildUnitModel);
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
		pHUD->SetButtonListener(9, CHUD::CancelInstall);
		pHUD->SetButtonTexture(9, s_iCancelIcon);
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, L"CANCEL INSTALLATION\n \nShortcut: G");
	}
	else if (eMenuMode == MENUMODE_INSTALL)
	{
		if (GetFirstUninstalledUpdate(UPDATETYPE_TANKATTACK) >= 0)
		{
			pHUD->SetButtonListener(0, CHUD::InstallTankAttack);
			pHUD->SetButtonTexture(0, s_iInstallAttackIcon);
			pHUD->SetButtonColor(0, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_TANKATTACK);
			CUpdateItem* pUpdate = GetUpdate(UPDATETYPE_TANKATTACK, iUpdate);

			std::wstringstream s;
			s << "INSTALL ATTACK ENERGY INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Attack Energy increase: " << pUpdate->m_flValue << "0%\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns\n \n"
				<< "Shortcut: Q";
			pHUD->SetButtonInfo(0, s.str().c_str());
		}

		if (GetUnitType() == STRUCTURE_ARTILLERYLOADER && GetFirstUninstalledUpdate(UPDATETYPE_TANKRANGE) >= 0)
		{
			pHUD->SetButtonListener(1, CHUD::InstallTankRange);
			pHUD->SetButtonTexture(1, s_iInstallRangeIcon);
			pHUD->SetButtonColor(1, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_TANKRANGE);
			CUpdateItem* pUpdate = GetUpdate(UPDATETYPE_TANKRANGE, iUpdate);

			std::wstringstream s;
			s << "INSTALL ATTACK RANGE INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Attack Range increase: " << pUpdate->m_flValue << " Units\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns\n \n"
				<< "Shortcut: W";
			pHUD->SetButtonInfo(1, s.str().c_str());
		}

		if (GetUnitType() != STRUCTURE_ARTILLERYLOADER && GetFirstUninstalledUpdate(UPDATETYPE_TANKDEFENSE) >= 0)
		{
			pHUD->SetButtonListener(2, CHUD::InstallTankDefense);
			pHUD->SetButtonTexture(2, s_iInstallDefenseIcon);
			pHUD->SetButtonColor(2, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_TANKDEFENSE);
			CUpdateItem* pUpdate = GetUpdate(UPDATETYPE_TANKDEFENSE, iUpdate);

			std::wstringstream s;
			s << "INSTALL DEFENSE ENERGY INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Defense Energy increase: " << pUpdate->m_flValue << "0%\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns\n \n"
				<< "Shortcut: E";
			pHUD->SetButtonInfo(2, s.str().c_str());
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_TANKMOVEMENT) >= 0)
		{
			pHUD->SetButtonListener(3, CHUD::InstallTankMovement);
			pHUD->SetButtonTexture(3, s_iInstallMovementIcon);
			pHUD->SetButtonColor(3, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_TANKMOVEMENT);
			CUpdateItem* pUpdate = GetUpdate(UPDATETYPE_TANKMOVEMENT, iUpdate);

			std::wstringstream s;
			s << "INSTALL MOVEMENT ENERGY INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Movement Energy increase: " << pUpdate->m_flValue << "0%\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns\n \n"
				<< "Shortcut: R";
			pHUD->SetButtonInfo(3, s.str().c_str());
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_TANKHEALTH) >= 0)
		{
			pHUD->SetButtonListener(4, CHUD::InstallTankHealth);
			pHUD->SetButtonTexture(4, s_iInstallHealthIcon);
			pHUD->SetButtonColor(4, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_TANKHEALTH);
			CUpdateItem* pUpdate = GetUpdate(UPDATETYPE_TANKHEALTH, iUpdate);

			std::wstringstream s;
			s << "INSTALL HEALTH INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Health increase: " << pUpdate->m_flValue << " Hull Strength\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns\n \n"
				<< "Shortcut: T";
			pHUD->SetButtonInfo(4, s.str().c_str());
		}

		pHUD->SetButtonListener(9, CHUD::GoToMain);
		pHUD->SetButtonTexture(9, s_iCancelIcon);
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, L"RETURN\n \nShortcut: G");
	}
	else if (m_bProducing)
	{
		pHUD->SetButtonListener(9, CHUD::CancelBuildUnit);
		pHUD->SetButtonTexture(9, s_iCancelIcon);
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, L"CANCEL UNIT PRODUCTION\n \nShortcut: G");
	}
	else
	{
		if (HasEnoughFleetPoints())
			pHUD->SetButtonListener(0, CHUD::BuildUnit);

		std::wstringstream s;

		if (GetBuildUnit() == BUILDUNIT_INFANTRY)
		{
			pHUD->SetButtonTexture(0, s_iBuildInfantryIcon);
			s << "BUILD MECHANIZED INFANTRY\n \n";
			s << "Mechanized infantry can fortify, gaining defense and attack energy bonuses over time. They are fantastic defense platforms, but once fortified they can't be moved.\n \n";
		}
		else if (GetBuildUnit() == BUILDUNIT_TANK)
		{
			pHUD->SetButtonTexture(0, s_iBuildTankIcon);
			s << "BUILD MAIN BATTLE TANK\n \n";
			s << "Main Battle Tanks are the core of any digital tank fleet. Although expensive, they are the only real way of taking territory from your enemies.\n \n";
		}
		else
		{
			pHUD->SetButtonTexture(0, s_iBuildArtilleryIcon);
			s << "BUILD ARTILLERY\n \n";
			s << "Artillery must be deployed before use and can only fire in front of themselves, but have ridiculous range and can pummel the enemy from afar. Artillery does double damage to shields, but only half damage to structures and tank hulls. Use them to soften enemy positions before moving in.\n \n";
		}

		s << "Fleet supply required: " << GetFleetPointsRequired() << "\n";
		s << "Turns to produce: " << GetTurnsToProduce() << " Turns\n \n";
		if (GetDigitanksTeam()->GetUnusedFleetPoints() < GetFleetPointsRequired())
			s << "NOT ENOUGH FLEET POINTS\n \n";
		s << "Shortcut: Q";
		pHUD->SetButtonInfo(0, s.str().c_str());

		if (HasEnoughFleetPoints())
			pHUD->SetButtonColor(0, Color(150, 150, 150));

		if (HasUpdatesAvailable())
		{
			pHUD->SetButtonListener(3, CHUD::InstallMenu);
			pHUD->SetButtonTexture(3, s_iInstallIcon);
			pHUD->SetButtonColor(3, Color(150, 150, 150));
			pHUD->SetButtonInfo(3, L"OPEN UPDATE INSTALL MENU\n \nShortcut: R");
		}
	}
}

bool CLoader::NeedsOrders()
{
	if (IsProducing())
		return false;

	if (!HasEnoughFleetPoints())
		return BaseClass::NeedsOrders();

	return true;
}

void CLoader::BeginProduction()
{
	if (IsConstructing())
		return;

	if (!HasEnoughFleetPoints())
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();

	if (CNetwork::IsHost())
		BeginProduction(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "BeginProduction", &p);
}

void CLoader::BeginProduction(class CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
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
	CNetworkParameters p;
	p.ui1 = GetHandle();

	if (CNetwork::IsHost())
		CancelProduction(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "CancelProduction", &p);
}

void CLoader::CancelProduction(class CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	m_iProductionStored = 0;
	m_bProducing = false;

	if (GetDigitanksTeam())
	{
		GetDigitanksTeam()->CountFleetPoints();
		GetDigitanksTeam()->CountProducers();
	}
}

size_t CLoader::GetUnitProductionCost()
{
	size_t iUpdates = 0;
	for (size_t i = 0; i < m_aiUpdatesInstalled.size(); i++)
		iUpdates += m_aiUpdatesInstalled[i];

	return g_aiTurnsToLoad[m_eBuildUnit] + iUpdates*4;
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

	CUpdateItem* pUpdate = GetUpdate(m_eInstallingType, m_iInstallingUpdate);

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

	DigitanksGame()->AddActionItem(this, ACTIONTYPE_INSTALLATION);
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
	if (!GetDigitanksTeam())
		return false;

	return GetFleetPointsRequired() <= GetDigitanksTeam()->GetUnusedFleetPoints();
}

size_t CLoader::GetTurnsToProduce()
{
	if (m_hSupplier == NULL)
		return -1;

	if (m_hSupplyLine == NULL)
		return -1;

	size_t iPerTurn = (size_t)(GetDigitanksTeam()->GetProductionPerLoader() * m_hSupplier->GetChildEfficiency() * m_hSupplyLine->GetIntegrity());
	if (iPerTurn == 0)
		return 9999;

	return (size_t)((GetUnitProductionCost()-m_iProductionStored)/iPerTurn)+1;
}

void CLoader::SetBuildUnit(buildunit_t eBuildUnit)
{
	m_eBuildUnit = eBuildUnit;

	switch (m_eBuildUnit)
	{
	case BUILDUNIT_INFANTRY:
		SetModel(L"models/structures/loader-infantry.obj");
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel(L"models/digitanks/infantry-body.obj");
		break;

	case BUILDUNIT_TANK:
		SetModel(L"models/structures/loader-main.obj");
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-body.obj");
		break;

	case BUILDUNIT_ARTILLERY:
		SetModel(L"models/structures/loader-artillery.obj");
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel(L"models/digitanks/artillery.obj");
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

	if (IsProducing() && GetSupplier() && GetSupplyLine())
	{
		s << L"(Producing)\n";
		size_t iProduction = (size_t)(GetDigitanksTeam()->GetProductionPerLoader() * GetSupplier()->GetChildEfficiency() * m_hSupplyLine->GetIntegrity());
		size_t iProductionLeft = GetUnitProductionCost() - m_iProductionStored;
		s << L"Power to build: " << iProductionLeft << L"\n";
		if (iProduction == 0)
			s << L"Turns left: " << 9999 << L"\n \n";
		else
			s << L"Turns left: " << (iProductionLeft/iProduction)+1 << L"\n \n";
	}

	if (IsInstalling())
	{
		s << L"[Installing update '" << GetUpdateInstalling()->GetName() << L"'...]\n";
		s << L"Power to install: " << GetProductionToInstall() << L"\n";
		s << L"Turns left: " << GetTurnsToInstall() << L"\n";
		sInfo = s.str();
	}

	if (GetSupplier() && GetSupplyLine())
		s << L"Efficiency: " << (int)(GetSupplier()->GetChildEfficiency()*m_hSupplyLine->GetIntegrity()*100) << L"%\n";

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

size_t CLoader::GetLoaderConstructionCost(buildunit_t eBuildUnit)
{
	if (eBuildUnit == BUILDUNIT_INFANTRY)
		return 20;
	if (eBuildUnit == BUILDUNIT_TANK)
		return 80;
	if (eBuildUnit == BUILDUNIT_ARTILLERY)
		return 100;
	else
		return 100;
}
