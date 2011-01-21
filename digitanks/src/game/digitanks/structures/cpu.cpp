#include "cpu.h"

#include <GL/glew.h>

#include <mtrand.h>
#include <renderer/renderer.h>
#include <renderer/dissolver.h>

#include <game/team.h>
#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <ui/hud.h>
#include <models/models.h>

#include <digitanks/digitanksgame.h>
#include <digitanks/structures/buffer.h>
#include <digitanks/structures/collector.h>
#include <digitanks/structures/loader.h>
#include <digitanks/units/scout.h>

size_t CCPU::s_iCancelIcon = 0;
size_t CCPU::s_iBuildPSUIcon = 0;
size_t CCPU::s_iBuildBufferIcon = 0;
size_t CCPU::s_iBuildLoaderIcon = 0;
size_t CCPU::s_iBuildInfantryLoaderIcon = 0;
size_t CCPU::s_iBuildTankLoaderIcon = 0;
size_t CCPU::s_iBuildArtilleryLoaderIcon = 0;
size_t CCPU::s_iBuildRogueIcon = 0;
size_t CCPU::s_iInstallIcon = 0;
size_t CCPU::s_iInstallPowerIcon = 0;
size_t CCPU::s_iInstallBandwidthIcon = 0;
size_t CCPU::s_iInstallFleetSupplyIcon = 0;

NETVAR_TABLE_BEGIN(CCPU);
	NETVAR_DEFINE_CALLBACK(CEntityHandle<CStructure>, m_hConstructing, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE_CALLBACK(bool, m_bProducing, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(size_t, m_iProduction);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCPU);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecPreviewBuild);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, unittype_t, m_ePreviewStructure);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CStructure>, m_hConstructing);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bProducing);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iProduction);
	//size_t						m_iFanModel;	// Spawn()
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flFanRotationSpeed);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flFanRotation);
SAVEDATA_TABLE_END();

void CCPU::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/cpu.obj");
	m_iFanModel = CModelLibrary::Get()->FindModel(L"models/structures/cpu-fan.obj");

	m_flFanRotationSpeed = 0;
	m_flFanRotation = RandomFloat(0, 360);

	m_bProducing = false;

	m_bConstructing = false;
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
	s_iBuildRogueIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-build-rogue.png");
	s_iInstallIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install.png");
	s_iInstallPowerIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-power.png");
	s_iInstallBandwidthIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-bandwidth.png");
	s_iInstallFleetSupplyIcon = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-install-fleet.png");
}

void CCPU::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = DigitanksWindow()->GetHUD();
	eastl::string16 p;

	bool bDisableMiniBuffer = !GetDigitanksTeam()->CanBuildMiniBuffers();
	bool bDisableBuffer = !GetDigitanksTeam()->CanBuildBuffers();
	bool bDisableBattery = !GetDigitanksTeam()->CanBuildBatteries();
	bool bDisablePSU = !GetDigitanksTeam()->CanBuildPSUs();
	bool bDisableLoaders = !GetDigitanksTeam()->CanBuildLoaders();

	if (m_hConstructing != NULL)
	{
		pHUD->SetButtonListener(9, CHUD::CancelBuild);
		pHUD->SetButtonTexture(9, s_iCancelIcon);
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, L"CANCEL CONSTRUCTION\n \nShortcut: G");
	}
	else if (IsProducing())
	{
		pHUD->SetButtonListener(9, CHUD::CancelBuildScout);
		pHUD->SetButtonTexture(9, s_iCancelIcon);
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, L"CANCEL UNIT PRODUCTION\n \nShortcut: G");
	}
	else if (!bDisableLoaders && eMenuMode == MENUMODE_LOADERS)
	{
		if (GetDigitanksTeam()->CanBuildInfantryLoaders())
		{
			pHUD->SetButtonListener(0, CHUD::BuildInfantryLoader);
			pHUD->SetButtonTexture(0, s_iBuildInfantryLoaderIcon);
			pHUD->SetButtonColor(0, Color(150, 150, 150));

			eastl::string16 s;
			s += L"BUILD RESISTOR LOADER\n \n";
			s += L"This program lets you build Resistor, the main defensive force of your fleet. After fortifying them they gain energy bonuses.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n", DigitanksGame()->GetConstructionCost(STRUCTURE_INFANTRYLOADER));
			s += p.sprintf(L"Turns to install: %d Turns\n \n", GetTurnsToConstruct(DigitanksGame()->GetConstructionCost(STRUCTURE_INFANTRYLOADER)));
			s += L"Shortcut: Q";
			pHUD->SetButtonInfo(0, s);
		}

		if (GetDigitanksTeam()->CanBuildTankLoaders())
		{
			pHUD->SetButtonListener(1, CHUD::BuildTankLoader);
			pHUD->SetButtonTexture(1, s_iBuildTankLoaderIcon);
			pHUD->SetButtonColor(1, Color(150, 150, 150));

			eastl::string16 s;
			s += L"BUILD DIGITANK LOADER\n \n";
			s += L"This program lets you build Digitanks, the primary assault force in your fleet.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n", DigitanksGame()->GetConstructionCost(STRUCTURE_TANKLOADER));
			s += p.sprintf(L"Turns to install: %d Turns\n \n", GetTurnsToConstruct(DigitanksGame()->GetConstructionCost(STRUCTURE_INFANTRYLOADER)));
			s += L"Shortcut: W";
			pHUD->SetButtonInfo(1, s);
		}

		if (GetDigitanksTeam()->CanBuildArtilleryLoaders())
		{
			pHUD->SetButtonListener(2, CHUD::BuildArtilleryLoader);
			pHUD->SetButtonTexture(2, s_iBuildArtilleryLoaderIcon);
			pHUD->SetButtonColor(2, Color(150, 150, 150));

			eastl::string16 s;
			s += L"BUILD ARTILLERY LOADER\n \n";
			s += L"This program lets you build Artillery. Once deployed, these units have extreme range and can easily soften enemy defensive positions.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n", DigitanksGame()->GetConstructionCost(STRUCTURE_ARTILLERYLOADER));
			s += p.sprintf(L"Turns to install: %d Turns\n \n", GetTurnsToConstruct(DigitanksGame()->GetConstructionCost(STRUCTURE_ARTILLERYLOADER)));
			s += L"Shortcut: E";
			pHUD->SetButtonInfo(2, s);
		}

		pHUD->SetButtonListener(9, CHUD::GoToMain);
		pHUD->SetButtonTexture(9, s_iCancelIcon);
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, L"RETURN\n \nShortcut: G");
	}
	else
	{
		if (!bDisableMiniBuffer)
		{
			pHUD->SetButtonListener(5, CHUD::BuildMiniBuffer);
			pHUD->SetButtonTexture(5, s_iBuildBufferIcon);
			pHUD->SetButtonColor(5, Color(150, 150, 150));

			eastl::string16 s;
			s += L"BUILD MINIBUFFER\n \n";
			s += L"MiniBuffers allow you to expand your Network, increasing the area under your control. All structures must be built on your Network. MiniBuffers can later be upgraded to Buffers.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n", DigitanksGame()->GetConstructionCost(STRUCTURE_MINIBUFFER));
			s += p.sprintf(L"Turns to install: %d Turns\n \n", GetTurnsToConstruct(DigitanksGame()->GetConstructionCost(STRUCTURE_MINIBUFFER)));
			s += L"Shortcut: A";
			pHUD->SetButtonInfo(5, s);
		}

		if (!bDisableBuffer)
		{
			pHUD->SetButtonListener(0, CHUD::BuildBuffer);
			pHUD->SetButtonTexture(0, s_iBuildBufferIcon);
			pHUD->SetButtonColor(0, Color(150, 150, 150));

			eastl::string16 s;
			s += L"BUILD BUFFER\n \n";
			s += L"Buffers allow you to expand your Network, increasing the area under your control. All structures must be built on your Network. Buffers can be improved by installing updates.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n", DigitanksGame()->GetConstructionCost(STRUCTURE_BUFFER));
			s += p.sprintf(L"Turns to install: %d Turns\n \n", GetTurnsToConstruct(DigitanksGame()->GetConstructionCost(STRUCTURE_BUFFER)));
			s += L"Shortcut: Q";
			pHUD->SetButtonInfo(0, s);
		}

		if (!bDisableBattery)
		{
			pHUD->SetButtonListener(6, CHUD::BuildBattery);
			pHUD->SetButtonTexture(6, s_iBuildPSUIcon);
			pHUD->SetButtonColor(6, Color(150, 150, 150));

			eastl::string16 s;
			s += L"BUILD BATTERY\n \n";
			s += L"Batteries allow you to harvest Power, which lets you build structures and units more quickly. Batteries can upgraded to Power Supply Units once those have been downloaded from the Updates Grid.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n", DigitanksGame()->GetConstructionCost(STRUCTURE_BATTERY));
			s += p.sprintf(L"Turns to install: %d Turns\n \n", GetTurnsToConstruct(DigitanksGame()->GetConstructionCost(STRUCTURE_BATTERY)));
			s += L"Shortcut: S";
			pHUD->SetButtonInfo(6, s);
		}

		if (!bDisablePSU)
		{
			pHUD->SetButtonListener(1, CHUD::BuildPSU);
			pHUD->SetButtonTexture(1, s_iBuildPSUIcon);
			pHUD->SetButtonColor(1, Color(150, 150, 150));

			eastl::string16 s;
			s += L"BUILD POWER SUPPLY UNIT\n \n";
			s += L"PSUs allow you to harvest Power, which lets you build structures and units more quickly.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n", DigitanksGame()->GetConstructionCost(STRUCTURE_PSU));
			s += p.sprintf(L"Turns to install: %d Turns\n \n", GetTurnsToConstruct(DigitanksGame()->GetConstructionCost(STRUCTURE_PSU)));
			s += L"Shortcut: W";
			pHUD->SetButtonInfo(1, s);
		}

		if (!bDisableLoaders)
		{
			pHUD->SetButtonListener(2, CHUD::BuildLoader);
			pHUD->SetButtonTexture(2, s_iBuildLoaderIcon);
			pHUD->SetButtonColor(2, Color(150, 150, 150));
			pHUD->SetButtonInfo(2, L"OPEN LOADER CONSTRUCTION MENU\n \nShortcut: E");
		}

		pHUD->SetButtonListener(7, CHUD::BuildScout);
		pHUD->SetButtonTexture(7, s_iBuildRogueIcon);
		pHUD->SetButtonColor(7, Color(150, 150, 150));

		eastl::string16 s;
		s += L"BUILD ROGUE\n \n";
		s += L"Rogues are a cheap reconnaisance unit with good speed but no shields. Their torpedo attack allows you to intercept enemy supply lines. Use them to find and slip behind enemy positions and harrass their support!\n \n";
		s += p.sprintf(L"Power to construct: %d Power\n", DigitanksGame()->GetConstructionCost(UNIT_SCOUT));
		s += p.sprintf(L"Turns to construct: %d Turns\n \n", GetTurnsToConstruct(DigitanksGame()->GetConstructionCost(UNIT_SCOUT)));
		s += L"Shortcut: D";
		pHUD->SetButtonInfo(7, s);
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

	if (!DigitanksGame()->GetTerrain()->IsPointOnMap(GetPreviewBuild()))
		return false;

	if (DigitanksGame()->GetTerrain()->IsPointOverHole(GetPreviewBuild()))
		return false;

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

	if (IsProducing())
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.i2 = m_ePreviewStructure;
	p.fl3 = GetPreviewBuild().x;
	p.fl4 = GetPreviewBuild().y;
	p.fl5 = GetPreviewBuild().z;

	if (CNetwork::IsHost())
		BeginConstruction(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "BeginConstruction", &p);
}

void CCPU::BeginConstruction(CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	// Overload this so that things this function calls get replicated.
	CNetwork::SetRunningClientFunctions(false);

	unittype_t ePreviewStructure = (unittype_t)p->i2;
	Vector vecPreview(p->fl3, p->fl4, p->fl5);

	if (m_hConstructing != NULL)
		CancelConstruction();

	if (ePreviewStructure == STRUCTURE_MINIBUFFER)
	{
		m_hConstructing = GameServer()->Create<CMiniBuffer>("CMiniBuffer");
	}
	else if (ePreviewStructure == STRUCTURE_BUFFER)
	{
		if (!GetDigitanksTeam()->CanBuildBuffers())
			return;

		m_hConstructing = GameServer()->Create<CBuffer>("CBuffer");
	}
	else if (ePreviewStructure == STRUCTURE_BATTERY)
	{
		m_hConstructing = GameServer()->Create<CBattery>("CBattery");
	}
	else if (ePreviewStructure == STRUCTURE_PSU)
	{
		if (!GetDigitanksTeam()->CanBuildPSUs())
			return;

		m_hConstructing = GameServer()->Create<CCollector>("CCollector");
	}
	else if (ePreviewStructure == STRUCTURE_INFANTRYLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildInfantryLoaders())
			return;

		m_hConstructing = GameServer()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(m_hConstructing.GetPointer());
		if (pLoader)
			pLoader->SetBuildUnit(UNIT_INFANTRY);
	}
	else if (ePreviewStructure == STRUCTURE_TANKLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildTankLoaders())
			return;

		m_hConstructing = GameServer()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(m_hConstructing.GetPointer());
		if (pLoader)
			pLoader->SetBuildUnit(UNIT_TANK);
	}
	else if (ePreviewStructure == STRUCTURE_ARTILLERYLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildArtilleryLoaders())
			return;

		m_hConstructing = GameServer()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(m_hConstructing.GetPointer());
		if (pLoader)
			pLoader->SetBuildUnit(UNIT_ARTILLERY);
	}

	m_hConstructing->BeginConstruction(vecPreview);
	GetTeam()->AddEntity(m_hConstructing);
	m_hConstructing->SetSupplier(FindClosestSupplier(vecPreview, GetTeam()));
	m_hConstructing->GetSupplier()->AddChild(m_hConstructing);

	m_hConstructing->SetOrigin(vecPreview);
	if (ePreviewStructure == STRUCTURE_PSU || ePreviewStructure == STRUCTURE_BATTERY)
	{
		Vector vecPSU = vecPreview;

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
		pCollector->SetResource(CResource::FindClosestResource(vecPreview, pCollector->GetResourceType()));
		pCollector->GetResource()->SetCollector(pCollector);
	}

	GetDigitanksTeam()->CountProducers();

	size_t iTutorial = DigitanksWindow()->GetInstructor()->GetCurrentTutorial();

	if (ePreviewStructure == STRUCTURE_BUFFER && iTutorial == CInstructor::TUTORIAL_BUFFER)
	{
		DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_BUFFER);
		DigitanksWindow()->GetInstructor()->NextTutorial();

		// Make sure it's done next turn.
		m_hConstructing->AddProduction(m_hConstructing->GetProductionToConstruct());
	}

	if (ePreviewStructure == STRUCTURE_PSU && iTutorial == CInstructor::TUTORIAL_PSU)
	{
		// Make sure it's done next turn.
		m_hConstructing->AddProduction(m_hConstructing->GetProductionToConstruct());
	}

	if (iTutorial == CInstructor::TUTORIAL_LOADER && (ePreviewStructure == STRUCTURE_INFANTRYLOADER || ePreviewStructure == STRUCTURE_TANKLOADER || ePreviewStructure == STRUCTURE_ARTILLERYLOADER))
	{
		// Make sure it's done next turn.
		m_hConstructing->AddProduction(m_hConstructing->GetProductionToConstruct());
	}

	m_hConstructing->FindGround();

	for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
	{
		for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
		{
			if (GetDigitanksTeam()->HasDownloadedUpdate(x, y))
				m_hConstructing->InstallUpdate(x, y);
		}
	}

	DigitanksGame()->GetTerrain()->CalculateVisibility();
}

void CCPU::CancelConstruction()
{
	CNetworkParameters p;
	p.ui1 = GetHandle();

	if (CNetwork::IsHost())
		CancelConstruction(NULL);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "CancelConstruction", &p);
}

void CCPU::CancelConstruction(class CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	if (m_hConstructing != NULL)
	{
		GameServer()->Delete(m_hConstructing);
		m_hConstructing = NULL;
	}

	DigitanksGame()->GetTerrain()->CalculateVisibility();
}

void CCPU::BeginRogueProduction()
{
	if (HasConstruction())
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();

	if (CNetwork::IsHost())
		BeginRogueProduction(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "BeginRogueProduction", &p);
}

void CCPU::BeginRogueProduction(class CNetworkParameters* p)
{
	m_iProduction = 0;
	m_bProducing = true;

	GetDigitanksTeam()->CountFleetPoints();
	GetDigitanksTeam()->CountProducers();
}

void CCPU::CancelRogueProduction()
{
	CNetworkParameters p;
	p.ui1 = GetHandle();

	if (CNetwork::IsHost())
		CancelRogueProduction(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "CancelRogueProduction", &p);
}

void CCPU::CancelRogueProduction(class CNetworkParameters* p)
{
	m_iProduction = 0;
	m_bProducing = false;

	GetDigitanksTeam()->CountFleetPoints();
	GetDigitanksTeam()->CountProducers();
}

void CCPU::StartTurn()
{
	BaseClass::StartTurn();

	if (!m_bProducing)
		m_iProduction = 0;

	if (m_bProducing)
	{
		m_iProduction += (size_t)(GetDigitanksTeam()->GetProductionPerLoader());
		if (m_iProduction > DigitanksGame()->GetConstructionCost(UNIT_SCOUT))
		{
			if (CNetwork::IsHost())
			{
				CDigitank* pTank = GameServer()->Create<CScout>("CScout");
				pTank->SetOrigin(GetOrigin());
				GetTeam()->AddEntity(pTank);

				for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
				{
					for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
					{
						if (GetDigitanksTeam()->HasDownloadedUpdate(x, y))
							pTank->DownloadComplete(x, y);
					}
				}

				// All of these StartTurn calls will probably cause problems later but for now they're
				// the only way to refresh the tank's energy so it has enough to leave the loader.
				pTank->StartTurn();

				pTank->SetPreviewMove(pTank->GetOrigin() + -GetOrigin().Normalized()*15);
				pTank->Move();

				pTank->StartTurn();

				// Face him toward the center.
				pTank->SetPreviewTurn(VectorAngles(-GetOrigin().Normalized()).y);
				pTank->Turn();

				pTank->StartTurn();
			}

			m_bProducing = false;

			DigitanksGame()->AppendTurnInfo(L"Production finished on Rogue");

//			DigitanksGame()->AddActionItem(pTank, ACTIONTYPE_NEWUNIT);
			DigitanksGame()->AddActionItem(this, ACTIONTYPE_UNITREADY);
		}
		else
		{
			eastl::string16 s;
			s.sprintf(L"Producing Rogue (%d turns left)", GetTurnsToConstruct(DigitanksGame()->GetConstructionCost(UNIT_SCOUT)-m_iProduction));
			DigitanksGame()->AppendTurnInfo(s);
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
			eastl::string16 s;
			s += L"Construction finished on " + m_hConstructing->GetName();
			DigitanksGame()->AppendTurnInfo(s);

			CCollector* pCollector = dynamic_cast<CCollector*>(m_hConstructing.GetPointer());
			if (pCollector && pCollector->GetSupplier())
				GetDigitanksTeam()->AddProduction((size_t)(pCollector->GetProduction() * pCollector->GetSupplier()->GetChildEfficiency()));

			m_hConstructing->CompleteConstruction();

			DigitanksGame()->AddActionItem(m_hConstructing, ACTIONTYPE_NEWSTRUCTURE);
			DigitanksGame()->AddActionItem(this, ACTIONTYPE_CONSTRUCTION);

			m_hConstructing = NULL;
		}
		else
		{
			m_hConstructing->AddProduction((size_t)GetDigitanksTeam()->GetProductionPerLoader());

			eastl::string16 s;
			s.sprintf((L"Constructing " + m_hConstructing->GetName() + L" (%d turns left)").c_str(), m_hConstructing->GetTurnsToConstruct());
			DigitanksGame()->AppendTurnInfo(s);
		}
	}
}

void CCPU::OnRender(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::OnRender(pContext, bTransparent);

	if (m_iFanModel == ~0)
		return;

	if (GetVisibility() == 0)
		return;

	CRenderingContext r(GameServer()->GetRenderer());
	if (GetTeam())
		r.SetColorSwap(GetTeam()->GetColor());
	else
		r.SetColorSwap(Color(255, 255, 255, 255));

	float flVisibility = GetVisibility();

	if (flVisibility < 1 && !bTransparent)
		return;

	if (flVisibility == 1 && bTransparent)
		return;

	if (bTransparent)
	{
		r.SetAlpha(GetVisibility());
		if (r.GetAlpha() < 1)
			r.SetBlend(BLEND_ALPHA);
	}

	float flSlowSpeed = 50.0f;
	float flFastSpeed = 200.0f;

	m_flFanRotationSpeed = Approach(HasConstruction()?flFastSpeed:flSlowSpeed, m_flFanRotationSpeed, GameServer()->GetFrameTime()*(flFastSpeed-flSlowSpeed));
	m_flFanRotation -= RemapVal(GameServer()->GetFrameTime(), 0, 1, 0, m_flFanRotationSpeed);

	r.Rotate(m_flFanRotation, Vector(0, 1, 0));

	r.RenderModel(m_iFanModel);
}

void CCPU::PostRender(bool bTransparent)
{
	BaseClass::PostRender(bTransparent);

	if (bTransparent && DigitanksGame()->GetControlMode() == MODE_BUILD)
	{
		CRenderingContext r(GameServer()->GetRenderer());
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

void CCPU::UpdateInfo(eastl::string16& s)
{
	eastl::string16 p;
	s = L"";
	s += L"CENTRAL PROCESSING UNIT\n";
	s += L"Command center\n \n";

	if (IsConstructing())
	{
		s += L"(Constructing)\n";
		s += p.sprintf(L"Power to build: %d\n", GetProductionToConstruct());
		s += p.sprintf(L"Turns left: %d\n", GetTurnsToConstruct());
		return;
	}

	if (HasConstruction())
	{
		s += L"[Constructing " + m_hConstructing->GetName() + L"...]\n";
		s += p.sprintf(L"Power to build: %d\n", m_hConstructing->GetProductionToConstruct());
		s += p.sprintf(L"Turns left: %d\n", m_hConstructing->GetTurnsToConstruct());
		return;
	}

	if (IsProducing())
	{
		s += L"[Producing Rogue]\n";
		s += p.sprintf(L"Power to build: %d\n", m_iProduction);
		s += p.sprintf(L"Turns left: %d\n \n", GetTurnsToConstruct(DigitanksGame()->GetConstructionCost(UNIT_SCOUT)-m_iProduction));
	}

	s += p.sprintf(L"Strength: %d\n", m_iDataStrength);
	s += p.sprintf(L"Growth: %d\n", (int)GetDataFlowRate());
	s += p.sprintf(L"Size: %d\n", (int)GetDataFlowRadius());
	s += p.sprintf(L"Efficiency: %d%\n", (int)(GetChildEfficiency()*100));
}

void CCPU::OnDeleted()
{
	eastl::vector<CBaseEntity*> apDeleteThese;

	for (size_t i = 0; i < GetTeam()->GetNumMembers(); i++)
	{
		CBaseEntity* pMember = GetTeam()->GetMember(i);
		if (pMember == this)
			continue;

		apDeleteThese.push_back(pMember);
	}

	if (GameServer()->IsLoading())
		return;

	for (size_t i = 0; i < apDeleteThese.size(); i++)
	{
		CBaseEntity* pMember = apDeleteThese[i];

		if (!pMember)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(pMember);
		// Delete? I meant... repurpose.
		if (pStructure && !pStructure->IsConstructing())
		{
			GetTeam()->RemoveEntity(pStructure);
		}
		else
		{
			bool bColorSwap = (pStructure || dynamic_cast<CDigitank*>(pMember));
			CModelDissolver::AddModel(pMember, bColorSwap?&GetTeam()->GetColor():NULL);
			pMember->Delete();
		}
	}
}
