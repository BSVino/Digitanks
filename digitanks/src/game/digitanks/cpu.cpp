#include "cpu.h"

#include <sstream>
#include <GL/glew.h>

#include <mtrand.h>
#include <renderer/renderer.h>
#include <renderer/dissolver.h>

#include <game/team.h>
#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <ui/hud.h>
#include <models/models.h>

#include "buffer.h"
#include "collector.h"
#include "loader.h"
#include "scout.h"

size_t CCPU::s_iCancelIcon = 0;
size_t CCPU::s_iBuildPSUIcon = 0;
size_t CCPU::s_iBuildBufferIcon = 0;
size_t CCPU::s_iBuildLoaderIcon = 0;
size_t CCPU::s_iBuildInfantryLoaderIcon = 0;
size_t CCPU::s_iBuildTankLoaderIcon = 0;
size_t CCPU::s_iBuildArtilleryLoaderIcon = 0;
size_t CCPU::s_iInstallIcon = 0;
size_t CCPU::s_iInstallPowerIcon = 0;
size_t CCPU::s_iInstallBandwidthIcon = 0;
size_t CCPU::s_iInstallFleetSupplyIcon = 0;

void CCPU::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/cpu.obj");
	m_iFanModel = CModelLibrary::Get()->FindModel(L"models/structures/cpu-fan.obj");

	m_flFanRotationSpeed = 0;
	m_flFanRotation = RandomFloat(0, 360);

	m_bProducing = false;
}

void CCPU::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/cpu.obj");
	PrecacheModel(L"models/structures/cpu-fan.obj");

	s_iCancelIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-cancel.png");
	s_iBuildPSUIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-psu.png");
	s_iBuildBufferIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-buffer.png");
	s_iBuildLoaderIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-loader.png");
	s_iBuildInfantryLoaderIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-infantry-loader.png");
	s_iBuildTankLoaderIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-tank-loader.png");
	s_iBuildArtilleryLoaderIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-artillery-loader.png");
	s_iInstallIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install.png");
	s_iInstallPowerIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-power.png");
	s_iInstallBandwidthIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-bandwidth.png");
	s_iInstallFleetSupplyIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-fleet.png");
}

void CCPU::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	bool bDisableMiniBuffer = !GetDigitanksTeam()->CanBuildMiniBuffers();
	bool bDisableBuffer = !GetDigitanksTeam()->CanBuildBuffers();
	bool bDisableBattery = !GetDigitanksTeam()->CanBuildBatteries();
	bool bDisablePSU = !GetDigitanksTeam()->CanBuildPSUs();
	bool bDisableLoaders = !GetDigitanksTeam()->CanBuildLoaders();

	if (m_hConstructing != NULL)
	{
		pHUD->SetButtonListener(4, CHUD::CancelBuild);
		pHUD->SetButtonTexture(4, s_iCancelIcon);
		pHUD->SetButtonColor(4, Color(100, 0, 0));
	}
	else if (IsInstalling())
	{
		pHUD->SetButtonListener(4, CHUD::CancelInstall);
		pHUD->SetButtonTexture(4, s_iCancelIcon);
		pHUD->SetButtonColor(4, Color(100, 0, 0));
	}
	else if (IsProducing())
	{
		pHUD->SetButtonListener(4, CHUD::CancelBuildScout);
		pHUD->SetButtonTexture(4, s_iCancelIcon);
		pHUD->SetButtonColor(4, Color(100, 0, 0));
	}
	else if (!bDisableLoaders && eMenuMode == MENUMODE_LOADERS)
	{
		if (GetDigitanksTeam()->CanBuildInfantryLoaders())
		{
			pHUD->SetButtonListener(0, CHUD::BuildInfantryLoader);
			pHUD->SetButtonTexture(0, s_iBuildInfantryLoaderIcon);
			pHUD->SetButtonColor(0, Color(150, 150, 150));

			std::wstringstream s;
			s << "BUILD INFANTRY LOADER\n \n"
				<< "This program lets you build Mechanized Infantry, the main defensive force of your fleet. After fortifying them they gain energy bonuses.\n \n"
				<< "Power to construct: " << CLoader::GetLoaderConstructionCost(BUILDUNIT_INFANTRY) << " Power\n"
				<< "Turns to install: " << GetTurnsToConstruct(CLoader::GetLoaderConstructionCost(BUILDUNIT_INFANTRY)) << " Turns";
			pHUD->SetButtonInfo(0, s.str().c_str());
		}

		if (GetDigitanksTeam()->CanBuildTankLoaders())
		{
			pHUD->SetButtonListener(1, CHUD::BuildTankLoader);
			pHUD->SetButtonTexture(1, s_iBuildTankLoaderIcon);
			pHUD->SetButtonColor(1, Color(150, 150, 150));

			std::wstringstream s;
			s << "BUILD MAIN BATTLE TANK LOADER\n \n"
				<< "This program lets you build Main Battle Tanks, the primary assault force in your fleet.\n \n"
				<< "Power to construct: " << CLoader::GetLoaderConstructionCost(BUILDUNIT_TANK) << " Power\n"
				<< "Turns to install: " << GetTurnsToConstruct(CLoader::GetLoaderConstructionCost(BUILDUNIT_TANK)) << " Turns";
			pHUD->SetButtonInfo(1, s.str().c_str());
		}

		if (GetDigitanksTeam()->CanBuildArtilleryLoaders())
		{
			pHUD->SetButtonListener(2, CHUD::BuildArtilleryLoader);
			pHUD->SetButtonTexture(2, s_iBuildArtilleryLoaderIcon);
			pHUD->SetButtonColor(2, Color(150, 150, 150));

			std::wstringstream s;
			s << "BUILD ARTILLERY LOADER\n \n"
				<< "This program lets you build Artillery. Once deployed, these units have extreme range and can easily soften enemy defensive positions.\n \n"
				<< "Power to construct: " << CLoader::GetLoaderConstructionCost(BUILDUNIT_ARTILLERY) << " Power\n"
				<< "Turns to install: " << GetTurnsToConstruct(CLoader::GetLoaderConstructionCost(BUILDUNIT_ARTILLERY)) << " Turns";
			pHUD->SetButtonInfo(2, s.str().c_str());
		}

		pHUD->SetButtonListener(4, CHUD::GoToMain);
		pHUD->SetButtonTexture(4, s_iCancelIcon);
		pHUD->SetButtonColor(4, Color(100, 0, 0));
	}
	else if (eMenuMode == MENUMODE_INSTALL)
	{
		if (GetFirstUninstalledUpdate(UPDATETYPE_PRODUCTION) >= 0)
		{
			pHUD->SetButtonListener(0, CHUD::InstallProduction);
			pHUD->SetButtonTexture(0, s_iInstallPowerIcon);
			pHUD->SetButtonColor(0, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_PRODUCTION);
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_PRODUCTION][iUpdate];

			std::wstringstream s;
			s << "INSTALL POWER INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Power increase: " << pUpdate->m_flValue << " Power\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(0, s.str().c_str());
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_BANDWIDTH) >= 0)
		{
			pHUD->SetButtonListener(1, CHUD::InstallBandwidth);
			pHUD->SetButtonTexture(1, s_iInstallBandwidthIcon);
			pHUD->SetButtonColor(1, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_BANDWIDTH);
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_BANDWIDTH][iUpdate];

			std::wstringstream s;
			s << "INSTALL BANDWIDTH INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Bandwidth increase: " << pUpdate->m_flValue << " Bandwidth\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(1, s.str().c_str());
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_FLEETSUPPLY) >= 0)
		{
			pHUD->SetButtonListener(2, CHUD::InstallFleetSupply);
			pHUD->SetButtonTexture(2, s_iInstallFleetSupplyIcon);
			pHUD->SetButtonColor(2, Color(150, 150, 150));

			int iUpdate = GetFirstUninstalledUpdate(UPDATETYPE_FLEETSUPPLY);
			CUpdateItem* pUpdate = m_apUpdates[UPDATETYPE_FLEETSUPPLY][iUpdate];

			std::wstringstream s;
			s << "INSTALL FLEET SUPPLY INCREASE\n \n"
				<< pUpdate->GetInfo() << "\n \n"
				<< "Fleet Supply increase: " << pUpdate->m_flValue << " Supply\n"
				<< "Turns to install: " << GetTurnsToInstall(pUpdate) << " Turns";
			pHUD->SetButtonInfo(2, s.str().c_str());
		}

		pHUD->SetButtonListener(4, CHUD::GoToMain);
		pHUD->SetButtonTexture(4, s_iCancelIcon);
		pHUD->SetButtonColor(4, Color(100, 0, 0));
	}
	else
	{
		if (bDisableBuffer)
		{
			if (!bDisableMiniBuffer)
			{
				pHUD->SetButtonListener(0, CHUD::BuildMiniBuffer);
				pHUD->SetButtonTexture(0, s_iBuildBufferIcon);
				pHUD->SetButtonColor(0, Color(150, 150, 150));

				std::wstringstream s;
				s << "BUILD MINIBUFFER\n \n"
					<< "MiniBuffers allow you to expand your Network, increasing the area under your control. All structures must be built on your Network. MiniBuffers can later be upgraded to Buffers.\n \n"
					<< "Power to construct: " << CMiniBuffer::GetMiniBufferConstructionCost() << " Power\n"
					<< "Turns to install: " << GetTurnsToConstruct(CMiniBuffer::GetMiniBufferConstructionCost()) << " Turns";
				pHUD->SetButtonInfo(0, s.str().c_str());
			}
		}
		else
		{
			pHUD->SetButtonListener(0, CHUD::BuildBuffer);
			pHUD->SetButtonTexture(0, s_iBuildBufferIcon);
			pHUD->SetButtonColor(0, Color(150, 150, 150));

			std::wstringstream s;
			s << "BUILD BUFFER\n \n"
				<< "Buffers allow you to expand your Network, increasing the area under your control. All structures must be built on your Network. Buffers can be improved by installing updates.\n \n"
				<< "Power to construct: " << CBuffer::GetBufferConstructionCost() << " Power\n"
				<< "Turns to install: " << GetTurnsToConstruct(CBuffer::GetBufferConstructionCost()) << " Turns";
			pHUD->SetButtonInfo(0, s.str().c_str());
		}

		if (bDisablePSU)
		{
			if (!bDisableBattery)
			{
				pHUD->SetButtonListener(1, CHUD::BuildBattery);
				pHUD->SetButtonTexture(1, s_iBuildPSUIcon);
				pHUD->SetButtonColor(1, Color(150, 150, 150));

				std::wstringstream s;
				s << "BUILD BATTERY\n \n"
					<< "Batteries allow you to harvest Power, which lets you build structures and units more quickly. Batteries can upgraded to Power Supply Units once those have been downloaded from the Updates Grid.\n \n"
					<< "Power to construct: " << CBattery::GetBatteryConstructionCost() << " Power\n"
					<< "Turns to install: " << GetTurnsToConstruct(CBattery::GetBatteryConstructionCost()) << " Turns";
				pHUD->SetButtonInfo(1, s.str().c_str());
			}
		}
		else
		{
			pHUD->SetButtonListener(1, CHUD::BuildPSU);
			pHUD->SetButtonTexture(1, s_iBuildPSUIcon);
			pHUD->SetButtonColor(1, Color(150, 150, 150));

			std::wstringstream s;
			s << "BUILD POWER SUPPLY UNIT\n \n"
				<< "PSUs allow you to harvest Power, which lets you build structures and units more quickly.\n \n"
				<< "Power to construct: " << CCollector::GetCollectorConstructionCost() << " Power\n"
				<< "Turns to install: " << GetTurnsToConstruct(CCollector::GetCollectorConstructionCost()) << " Turns";
			pHUD->SetButtonInfo(1, s.str().c_str());
		}

		if (!bDisableLoaders)
		{
			pHUD->SetButtonListener(2, CHUD::BuildLoader);
			pHUD->SetButtonTexture(2, s_iBuildLoaderIcon);
			pHUD->SetButtonColor(2, Color(150, 150, 150));
		}

		if (HasUpdatesAvailable())
		{
			pHUD->SetButtonListener(3, CHUD::InstallMenu);
			pHUD->SetButtonTexture(3, s_iInstallIcon);
			pHUD->SetButtonColor(3, Color(150, 150, 150));
		}

		pHUD->SetButtonListener(6, CHUD::BuildScout);
		pHUD->SetButtonTexture(6, 0);
		pHUD->SetButtonColor(6, Color(150, 150, 150));

		std::wstringstream s;
		s << "BUILD ROGUE\n \n"
			<< "Rogues are a cheap reconnaisance unit with good speed but shields. Their torpedo attack allows you to intercept enemy supply lines. Use them to find and slip behind enemy positions and harrass their support!\n \n"
			<< "Power to construct: " << g_aiTurnsToLoad[BUILDUNIT_SCOUT] << " Power\n"
			<< "Turns to install: " << GetTurnsToConstruct(g_aiTurnsToLoad[BUILDUNIT_SCOUT]) << " Turns";
		pHUD->SetButtonInfo(6, s.str().c_str());
	}
}

bool CCPU::NeedsOrders()
{
	if (HasConstruction() || IsProducing())
		return false;

	bool bDisableMiniBuffer = !GetDigitanksTeam()->CanBuildMiniBuffers();
	bool bDisableBuffer = !GetDigitanksTeam()->CanBuildBuffers();
	bool bDisableBattery = !GetDigitanksTeam()->CanBuildBatteries();
	bool bDisablePSU = !GetDigitanksTeam()->CanBuildPSUs();
	bool bDisableLoaders = !GetDigitanksTeam()->CanBuildLoaders();

	if (bDisableMiniBuffer && bDisableBuffer && bDisableBattery && bDisablePSU && bDisableLoaders)
		return BaseClass::NeedsOrders();

	return true;
}

bool CCPU::AllowControlMode(controlmode_t eMode) const
{
	if (eMode == MODE_BUILD)
		return true;

	return BaseClass::AllowControlMode(eMode);
}

bool CCPU::IsPreviewBuildValid() const
{
	CSupplier* pSupplier = FindClosestSupplier(GetPreviewBuild(), GetTeam());

	if (m_ePreviewStructure == STRUCTURE_PSU || m_ePreviewStructure == STRUCTURE_BATTERY)
	{
		CResource* pResource = CResource::FindClosestResource(GetPreviewBuild(), RESOURCE_ELECTRONODE);
		float flDistance = (pResource->GetOrigin() - GetPreviewBuild()).Length();
		if (flDistance > 5)
			return false;

		if (pResource->HasCollector())
			return false;
	}
	else
	{
		// Don't allow construction too close to other structures.
		CStructure* pClosestStructure = CBaseEntity::FindClosest<CStructure>(GetPreviewBuild());
		if ((pClosestStructure->GetOrigin() - GetPreviewBuild()).Length() < pClosestStructure->GetBoundingRadius()+5)
			return false;

		if (!pSupplier)
			return false;
	}

	return CSupplier::GetDataFlow(GetPreviewBuild(), GetTeam()) > 0;
}

void CCPU::SetPreviewBuild(Vector vecPreviewBuild)
{
	m_vecPreviewBuild = vecPreviewBuild;
}

void CCPU::ClearPreviewBuild()
{
	m_vecPreviewBuild = GetOrigin();
}

void CCPU::BeginConstruction()
{
	if (!IsPreviewBuildValid())
		return;

	if (IsInstalling())
		return;

	if (IsProducing())
		return;

	if (m_hConstructing != NULL)
		CancelConstruction();

	if (m_ePreviewStructure == STRUCTURE_MINIBUFFER)
	{
		m_hConstructing = Game()->Create<CMiniBuffer>("CMiniBuffer");
	}
	else if (m_ePreviewStructure == STRUCTURE_BUFFER)
	{
		if (!GetDigitanksTeam()->CanBuildBuffers())
			return;

		m_hConstructing = Game()->Create<CBuffer>("CBuffer");
	}
	else if (m_ePreviewStructure == STRUCTURE_BATTERY)
	{
		m_hConstructing = Game()->Create<CBattery>("CBattery");
	}
	else if (m_ePreviewStructure == STRUCTURE_PSU)
	{
		if (!GetDigitanksTeam()->CanBuildPSUs())
			return;

		m_hConstructing = Game()->Create<CCollector>("CCollector");
	}
	else if (m_ePreviewStructure == STRUCTURE_INFANTRYLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildInfantryLoaders())
			return;

		m_hConstructing = Game()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(m_hConstructing.GetPointer());
		if (pLoader)
			pLoader->SetBuildUnit(BUILDUNIT_INFANTRY);
	}
	else if (m_ePreviewStructure == STRUCTURE_TANKLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildTankLoaders())
			return;

		m_hConstructing = Game()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(m_hConstructing.GetPointer());
		if (pLoader)
			pLoader->SetBuildUnit(BUILDUNIT_TANK);
	}
	else if (m_ePreviewStructure == STRUCTURE_ARTILLERYLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildArtilleryLoaders())
			return;

		m_hConstructing = Game()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(m_hConstructing.GetPointer());
		if (pLoader)
			pLoader->SetBuildUnit(BUILDUNIT_ARTILLERY);
	}

	m_hConstructing->BeginConstruction();
	GetTeam()->AddEntity(m_hConstructing);
	m_hConstructing->SetSupplier(FindClosestSupplier(GetPreviewBuild(), GetTeam()));
	m_hConstructing->GetSupplier()->AddChild(m_hConstructing);

	m_hConstructing->SetOrigin(GetPreviewBuild());
	if (m_ePreviewStructure == STRUCTURE_PSU || m_ePreviewStructure == STRUCTURE_BATTERY)
	{
		Vector vecPSU = GetPreviewBuild();

		CResource* pResource = CBaseEntity::FindClosest<CResource>(vecPSU);

		if (pResource)
		{
			if ((pResource->GetOrigin() - vecPSU).Length() <= 6)
				m_hConstructing->SetOrigin(pResource->GetOrigin());
		}
	}

	CSupplier* pSupplier = dynamic_cast<CSupplier*>(m_hConstructing.GetPointer());
	if (pSupplier)
		pSupplier->GiveDataStrength((size_t)pSupplier->GetSupplier()->GetDataFlow(pSupplier->GetOrigin()));

	CCollector* pCollector = dynamic_cast<CCollector*>(m_hConstructing.GetPointer());
	if (pCollector)
	{
		pCollector->SetResource(CResource::FindClosestResource(GetPreviewBuild(), pCollector->GetResourceType()));
		pCollector->GetResource()->SetCollector(pCollector);
	}

	GetDigitanksTeam()->CountProducers();

	size_t iTutorial = CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial();

	if (m_ePreviewStructure == STRUCTURE_BUFFER && iTutorial == CInstructor::TUTORIAL_BUFFER)
	{
		CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_BUFFER);
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();

		// Make sure it's done next turn.
		m_hConstructing->AddProduction(m_hConstructing->GetProductionToConstruct());
	}

	if (m_ePreviewStructure == STRUCTURE_PSU && iTutorial == CInstructor::TUTORIAL_PSU)
	{
		// Make sure it's done next turn.
		m_hConstructing->AddProduction(m_hConstructing->GetProductionToConstruct());
	}

	if (iTutorial == CInstructor::TUTORIAL_LOADER && (m_ePreviewStructure == STRUCTURE_INFANTRYLOADER || m_ePreviewStructure == STRUCTURE_TANKLOADER || m_ePreviewStructure == STRUCTURE_ARTILLERYLOADER))
	{
		// Make sure it's done next turn.
		m_hConstructing->AddProduction(m_hConstructing->GetProductionToConstruct());
	}
}

void CCPU::CancelConstruction()
{
	if (m_hConstructing != NULL)
	{
		Game()->Delete(m_hConstructing);
		m_hConstructing = NULL;
	}
}

void CCPU::BeginProduction()
{
	if (IsInstalling())
		return;

	if (HasConstruction())
		return;

	m_iProduction = 0;
	m_bProducing = true;

	GetDigitanksTeam()->CountFleetPoints();
	GetDigitanksTeam()->CountProducers();
}

void CCPU::CancelProduction()
{
	m_iProduction = 0;
	m_bProducing = false;

	GetDigitanksTeam()->CountFleetPoints();
}

bool CCPU::HasUpdatesAvailable()
{
	if (GetFirstUninstalledUpdate(UPDATETYPE_BANDWIDTH) >= 0)
		return true;

	if (GetFirstUninstalledUpdate(UPDATETYPE_FLEETSUPPLY) >= 0)
		return true;

	if (GetFirstUninstalledUpdate(UPDATETYPE_PRODUCTION) >= 0)
		return true;

	return false;
}

void CCPU::InstallUpdate(updatetype_t eUpdate)
{
	if (HasConstruction())
		return;

	if (IsProducing())
		return;

	BaseClass::InstallUpdate(eUpdate);
}

void CCPU::StartTurn()
{
	BaseClass::StartTurn();

	if (!m_bProducing)
		m_iProduction = 0;

	if (m_bProducing)
	{
		m_iProduction += (size_t)(GetDigitanksTeam()->GetProductionPerLoader());
		if (m_iProduction > g_aiTurnsToLoad[BUILDUNIT_SCOUT])
		{
			CDigitank* pTank = Game()->Create<CScout>("CScout");
			
			pTank->SetOrigin(GetOrigin());

			GetTeam()->AddEntity(pTank);

			m_bProducing = false;

			DigitanksGame()->AppendTurnInfo(L"Production finished on Rogue");

			// All of these StartTurn calls will probably cause problems later but for now they're
			// the only way to refresh the tank's energy so it has enough to leave the loader.
			pTank->StartTurn();

			pTank->SetPreviewMove(pTank->GetOrigin() + -GetOrigin().Normalized()*15);
			pTank->SetDesiredMove();
			pTank->Move();

			pTank->StartTurn();

			// Face him toward the center.
			pTank->SetPreviewTurn(VectorAngles(-GetOrigin().Normalized()).y);
			pTank->SetDesiredTurn();
			pTank->Turn();

			pTank->StartTurn();

			DigitanksGame()->AddActionItem(pTank, ACTIONTYPE_UNITREADY);
		}
		else
		{
			std::wstringstream s;
			s << L"Producing Rogue (" << GetTurnsToConstruct(g_aiTurnsToLoad[BUILDUNIT_SCOUT]-m_iProduction) << L" turns left)";
			DigitanksGame()->AppendTurnInfo(s.str().c_str());
		}
	}

	if (m_hConstructing != NULL && m_hConstructing->IsConstructing())
	{
		if (!m_hConstructing->GetSupplier() || m_hConstructing->GetSupplier()->IsDeleted())
		{
			CancelConstruction();
		}
	}

	if (m_hConstructing != NULL && m_hConstructing->IsConstructing())
	{
		if (GetDigitanksTeam()->GetProductionPerLoader() > m_hConstructing->GetProductionToConstruct())
		{
			std::wstringstream s;
			s << L"Construction finished on " << m_hConstructing->GetName();
			DigitanksGame()->AppendTurnInfo(s.str().c_str());

			CCollector* pCollector = dynamic_cast<CCollector*>(m_hConstructing.GetPointer());
			if (pCollector && pCollector->GetSupplier())
				GetDigitanksTeam()->AddProduction((size_t)(pCollector->GetProduction() * pCollector->GetSupplier()->GetChildEfficiency()));

			m_hConstructing->CompleteConstruction();
			m_hConstructing = NULL;
		}
		else
		{
			m_hConstructing->AddProduction((size_t)GetDigitanksTeam()->GetProductionPerLoader());

			std::wstringstream s;
			s << L"Constructing " << m_hConstructing->GetName() << L" (" << m_hConstructing->GetTurnsToConstruct() << L" turns left)";
			DigitanksGame()->AppendTurnInfo(s.str().c_str());
		}
	}
}

void CCPU::OnRender()
{
	BaseClass::OnRender();

	if (m_iFanModel == ~0)
		return;

	if (GetVisibility() == 0)
		return;

	CRenderingContext r(Game()->GetRenderer());
	if (GetTeam())
		r.SetColorSwap(GetTeam()->GetColor());
	else
		r.SetColorSwap(Color(255, 255, 255, 255));

	r.SetAlpha(GetVisibility());
	if (r.GetAlpha() < 1)
		r.SetBlend(BLEND_ALPHA);

	float flSlowSpeed = 50.0f;
	float flFastSpeed = 200.0f;

	m_flFanRotationSpeed = Approach(HasConstruction()?flFastSpeed:flSlowSpeed, m_flFanRotationSpeed, Game()->GetFrameTime()*(flFastSpeed-flSlowSpeed));
	m_flFanRotation -= RemapVal(Game()->GetFrameTime(), 0, 1, 0, m_flFanRotationSpeed);

	r.Rotate(m_flFanRotation, Vector(0, 1, 0));

	r.RenderModel(m_iFanModel);
}

void CCPU::PostRender()
{
	BaseClass::PostRender();

	if (DigitanksGame()->GetControlMode() == MODE_BUILD)
	{
		CRenderingContext r(Game()->GetRenderer());
		r.Translate(GetPreviewBuild() + Vector(0, 1, 0));
		r.Rotate(-GetAngles().y, Vector(0, 1, 0));

		if (IsPreviewBuildValid())
		{
			r.SetColorSwap(Color(255, 255, 255));
			r.SetAlpha(0.5f);
			r.SetBlend(BLEND_ALPHA);
		}
		else
		{
			r.SetColorSwap(Color(255, 0, 0));
			r.SetAlpha(0.3f);
			r.SetBlend(BLEND_ADDITIVE);
		}

		size_t iModel = 0;
		switch (m_ePreviewStructure)
		{
		case STRUCTURE_MINIBUFFER:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/minibuffer.obj");
			break;

		case STRUCTURE_BUFFER:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/buffer.obj");
			break;

		case STRUCTURE_BATTERY:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/battery.obj");
			break;

		case STRUCTURE_PSU:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/psu.obj");
			break;

		case STRUCTURE_INFANTRYLOADER:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/loader-infantry.obj");
			break;

		case STRUCTURE_TANKLOADER:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/loader-main.obj");
			break;

		case STRUCTURE_ARTILLERYLOADER:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/loader-artillery.obj");
			break;
		}

		r.RenderModel(iModel);
	}
}

void CCPU::UpdateInfo(std::wstring& sInfo)
{
	std::wstringstream s;

	s << L"CENTRAL PROCESSING UNIT\n";
	s << L"Command center\n \n";

	if (IsConstructing())
	{
		s << L"(Constructing)\n";
		s << L"Power to build: " << GetProductionToConstruct() << L"\n";
		s << L"Turns left: " << GetTurnsToConstruct() << L"\n";
		sInfo = s.str();
		return;
	}

	if (HasConstruction())
	{
		s << L"[Constructing " << m_hConstructing->GetName() << L"...]\n";
		s << L"Power to build: " << m_hConstructing->GetProductionToConstruct() << L"\n";
		s << L"Turns left: " << m_hConstructing->GetTurnsToConstruct() << L"\n";
		sInfo = s.str();
		return;
	}

	if (IsInstalling())
	{
		s << L"[Installing update '" << GetUpdateInstalling()->GetName() << L"'...]\n";
		s << L"Power to install: " << GetProductionToInstall() << L"\n";
		s << L"Turns left: " << GetTurnsToInstall() << L"\n";
		sInfo = s.str();
		return;
	}

	if (IsProducing())
	{
		s << L"(Producing Rogue)\n";
		s << L"Power to build: " << m_iProduction << L"\n";
		s << L"Turns left: " << GetTurnsToConstruct(g_aiTurnsToLoad[BUILDUNIT_SCOUT]-m_iProduction) << L"\n \n";
	}

	s << L"Strength: " << m_iDataStrength << L"\n";
	s << L"Growth: " << (int)GetDataFlowRate() << L"\n";
	s << L"Size: " << (int)GetDataFlowRadius() << L"\n";
	s << L"Efficiency: " << (int)(GetChildEfficiency()*100) << L"%\n";

	sInfo = s.str();
}

void CCPU::OnDeleted()
{
	std::vector<CBaseEntity*> apDeleteThese;

	for (size_t i = 0; i < GetTeam()->GetNumMembers(); i++)
	{
		CBaseEntity* pMember = GetTeam()->GetMember(i);
		if (pMember == this)
			continue;

		apDeleteThese.push_back(pMember);
	}

	for (size_t i = 0; i < apDeleteThese.size(); i++)
	{
		CBaseEntity* pMember = apDeleteThese[i];

		CStructure* pStructure = dynamic_cast<CStructure*>(pMember);
		// Delete? I meant... repurpose.
		if (pStructure && !pStructure->IsConstructing())
		{
			GetTeam()->RemoveEntity(pStructure);
		}
		else
		{
			CModelDissolver::AddModel(pMember);
			pMember->Delete();
		}
	}
}
