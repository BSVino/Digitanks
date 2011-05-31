#include "cpu.h"

#include <GL/glew.h>

#include <mtrand.h>
#include <strutils.h>

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
#include <digitanks/structures/autoturret.h>
#include <digitanks/units/scout.h>

REGISTER_ENTITY(CCPU);

NETVAR_TABLE_BEGIN(CCPU);
	NETVAR_DEFINE_CALLBACK(bool, m_bProducing, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(size_t, m_iTurnsToProduceRogue);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCPU);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecPreviewBuild);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, unittype_t, m_ePreviewStructure);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bProducing);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTurnsToProduceRogue);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, size_t, m_iFanModel);	// Spawn()
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flFanRotation);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CCPU);
INPUTS_TABLE_END();

void CCPU::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/cpu.obj");
	m_iFanModel = CModelLibrary::Get()->FindModel(L"models/structures/cpu-fan.obj");

	m_flFanRotation = RandomFloat(0, 360);

	m_bProducing = false;

	m_bConstructing = false;

	m_flConstructionStartTime = GameServer()->GetGameTime();
	m_flTendrilGrowthStartTime = m_flConstructionStartTime + 3;
}

void CCPU::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/cpu.obj");
	PrecacheModel(L"models/structures/cpu-fan.obj");
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

	if (!bDisableLoaders && eMenuMode == MENUMODE_LOADERS)
	{
		if (DigitanksGame()->GetControlMode() == MODE_BUILD && GetPreviewStructure() == STRUCTURE_INFANTRYLOADER)
		{
			pHUD->SetButtonTexture(0, "Cancel");

			pHUD->SetButtonListener(0, CHUD::CancelBuild);
			pHUD->SetButtonColor(0, Color(100, 100, 100));

			eastl::string16 s;
			s += L"CANCEL\n \n";
			s += L"Shortcut: Q";
			pHUD->SetButtonInfo(0, s);
			pHUD->SetButtonTooltip(0, L"Cancel");
		}
		else if (GetDigitanksTeam()->CanBuildInfantryLoaders())
		{
			pHUD->SetButtonTexture(0, "ResistorFactory");

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_INFANTRYLOADER))
			{
				pHUD->SetButtonListener(0, CHUD::BuildInfantryLoader);
				pHUD->SetButtonColor(0, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD RESISTOR FACTORY\n \n";
			s += L"This program lets you build Resistor, the main defensive force of your fleet. After fortifying them they gain energy bonuses.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_INFANTRYLOADER));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_INFANTRYLOADER))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: Q";
			pHUD->SetButtonInfo(0, s);
			pHUD->SetButtonTooltip(0, L"Build Resistor Factory");
		}

		if (DigitanksGame()->GetControlMode() == MODE_BUILD && GetPreviewStructure() == STRUCTURE_TANKLOADER)
		{
			pHUD->SetButtonTexture(1, "Cancel");

			pHUD->SetButtonListener(1, CHUD::CancelBuild);
			pHUD->SetButtonColor(1, Color(100, 100, 100));

			eastl::string16 s;
			s += L"CANCEL\n \n";
			s += L"Shortcut: W";
			pHUD->SetButtonInfo(1, s);
			pHUD->SetButtonTooltip(1, L"Cancel");
		}
		else if (GetDigitanksTeam()->CanBuildTankLoaders())
		{
			pHUD->SetButtonTexture(1, "DigitankFactory");

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_TANKLOADER))
			{
				pHUD->SetButtonListener(1, CHUD::BuildTankLoader);
				pHUD->SetButtonColor(1, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD DIGITANK FACTORY\n \n";
			s += L"This program lets you build Digitanks, the primary assault force in your fleet.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_TANKLOADER));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_TANKLOADER))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: W";
			pHUD->SetButtonInfo(1, s);
			pHUD->SetButtonTooltip(1, L"Build Digitank Factory");
		}

		if (DigitanksGame()->GetControlMode() == MODE_BUILD && GetPreviewStructure() == STRUCTURE_ARTILLERYLOADER)
		{
			pHUD->SetButtonTexture(2, "Cancel");

			pHUD->SetButtonListener(2, CHUD::CancelBuild);
			pHUD->SetButtonColor(2, Color(100, 100, 100));

			eastl::string16 s;
			s += L"CANCEL\n \n";
			s += L"Shortcut: E";
			pHUD->SetButtonInfo(2, s);
			pHUD->SetButtonTooltip(2, L"Cancel");
		}
		else if (GetDigitanksTeam()->CanBuildArtilleryLoaders())
		{
			pHUD->SetButtonTexture(2, "ArtilleryFactory");

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_ARTILLERYLOADER))
			{
				pHUD->SetButtonListener(2, CHUD::BuildArtilleryLoader);
				pHUD->SetButtonColor(2, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD ARTILLERY FACTORY\n \n";
			s += L"This program lets you build Artillery. Once deployed, these units have extreme range and can easily soften enemy defensive positions.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_ARTILLERYLOADER));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_ARTILLERYLOADER))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: E";
			pHUD->SetButtonInfo(2, s);
			pHUD->SetButtonTooltip(2, L"Build Artillery Factory");
		}

		pHUD->SetButtonListener(9, CHUD::GoToMain);
		pHUD->SetButtonTexture(9, "Cancel");
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, L"RETURN\n \nShortcut: G");
		pHUD->SetButtonTooltip(9, L"Return");
	}
	else
	{
		if (DigitanksGame()->GetControlMode() == MODE_BUILD && GetPreviewStructure() == STRUCTURE_MINIBUFFER)
		{
			pHUD->SetButtonTexture(5, "Cancel");

			pHUD->SetButtonListener(5, CHUD::CancelBuild);
			pHUD->SetButtonColor(5, Color(100, 100, 100));

			eastl::string16 s;
			s += L"CANCEL\n \n";
			s += L"Shortcut: A";
			pHUD->SetButtonInfo(5, s);
			pHUD->SetButtonTooltip(5, L"Cancel");
		}
		else if (!bDisableMiniBuffer)
		{
			pHUD->SetButtonTexture(5, "Buffer");

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_MINIBUFFER))
			{
				pHUD->SetButtonListener(5, CHUD::BuildMiniBuffer);
				pHUD->SetButtonColor(5, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD BUFFER\n \n";
			s += L"Buffers allow you to expand your Network, increasing the area under your control. All structures must be built on your Network. Buffers can be upgraded to Macro-Buffers after downloading them from the Downloads Grid.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_MINIBUFFER));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_MINIBUFFER))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: A";
			pHUD->SetButtonInfo(5, s);
			pHUD->SetButtonTooltip(5, L"Build Buffer");
		}

		if (DigitanksGame()->GetControlMode() == MODE_BUILD && GetPreviewStructure() == STRUCTURE_BUFFER)
		{
			pHUD->SetButtonTexture(0, "Cancel");

			pHUD->SetButtonListener(0, CHUD::CancelBuild);
			pHUD->SetButtonColor(0, Color(100, 100, 100));

			eastl::string16 s;
			s += L"CANCEL\n \n";
			s += L"Shortcut: Q";
			pHUD->SetButtonInfo(0, s);
			pHUD->SetButtonTooltip(0, L"Cancel");
		}
		else if (!bDisableBuffer)
		{
			pHUD->SetButtonTexture(0, "MacroBuffer");

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_BUFFER))
			{
				pHUD->SetButtonListener(0, CHUD::BuildBuffer);
				pHUD->SetButtonColor(0, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD MACRO-BUFFER\n \n";
			s += L"Macro-Buffers allow you to expand your Network, increasing the area under your control. All structures must be built on your Network. Macro-Buffers can be improved by downloading updates.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_BUFFER));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_BUFFER))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: Q";
			pHUD->SetButtonInfo(0, s);
			pHUD->SetButtonTooltip(0, L"Build Macro-Buffer");
		}

		if (DigitanksGame()->GetControlMode() == MODE_BUILD && GetPreviewStructure() == STRUCTURE_BATTERY)
		{
			pHUD->SetButtonTexture(6, "Cancel");

			pHUD->SetButtonListener(6, CHUD::CancelBuild);
			pHUD->SetButtonColor(6, Color(100, 100, 100));

			eastl::string16 s;
			s += L"CANCEL\n \n";
			s += L"Shortcut: S";
			pHUD->SetButtonInfo(6, s);
			pHUD->SetButtonTooltip(6, L"Cancel");
		}
		else if (!bDisableBattery)
		{
			pHUD->SetButtonTexture(6, "Capacitor");

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_BATTERY))
			{
				pHUD->SetButtonListener(6, CHUD::BuildBattery);
				pHUD->SetButtonColor(6, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD CAPACITOR\n \n";
			s += L"Capacitors allow you to harvest Power, which lets you build structures and units more quickly. Capacitors can upgraded to Power Supply Units once those have been downloaded from the Updates Grid.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_BATTERY));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_BATTERY))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: S";
			pHUD->SetButtonInfo(6, s);
			pHUD->SetButtonTooltip(6, L"Build Capacitor");
		}

		if (DigitanksGame()->GetControlMode() == MODE_BUILD && GetPreviewStructure() == STRUCTURE_PSU)
		{
			pHUD->SetButtonTexture(1, "Cancel");

			pHUD->SetButtonListener(1, CHUD::CancelBuild);
			pHUD->SetButtonColor(1, Color(100, 100, 100));

			eastl::string16 s;
			s += L"CANCEL\n \n";
			s += L"Shortcut: W";
			pHUD->SetButtonInfo(1, s);
			pHUD->SetButtonTooltip(1, L"Cancel");
		}
		else if (!bDisablePSU)
		{
			pHUD->SetButtonTexture(1, "PSU");

			if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_PSU))
			{
				pHUD->SetButtonListener(1, CHUD::BuildPSU);
				pHUD->SetButtonColor(1, Color(150, 150, 150));
			}

			eastl::string16 s;
			s += L"BUILD POWER SUPPLY UNIT\n \n";
			s += L"PSUs allow you to harvest Power, which lets you build structures and units more quickly.\n \n";
			s += p.sprintf(L"Power to construct: %d Power\n \n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_PSU));

			if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_PSU))
				s += L"NOT ENOUGH POWER\n \n";

			s += L"Shortcut: W";
			pHUD->SetButtonInfo(1, s);
			pHUD->SetButtonTooltip(1, L"Build PSU");
		}

		if (!bDisableLoaders)
		{
			pHUD->SetButtonListener(2, CHUD::BuildLoader);
			pHUD->SetButtonTexture(2, "Factory");
			pHUD->SetButtonColor(2, Color(150, 150, 150));
			pHUD->SetButtonInfo(2, L"OPEN FACTORY CONSTRUCTION MENU\n \nFactories allow you to produce more advanced units.\n \nShortcut: E");
			pHUD->SetButtonTooltip(2, L"Open Factory Menu");
		}

		pHUD->SetButtonTexture(7, "Rogue");
		if (GetDigitanksTeam()->GetUnusedFleetPoints() && GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(UNIT_SCOUT) && !IsProducing())
		{
			pHUD->SetButtonColor(7, Color(150, 150, 150));
			pHUD->SetButtonListener(7, CHUD::BuildScout);
		}

		eastl::string16 s;
		s += L"BUILD ROGUE\n \n";
		s += L"Rogues are a cheap reconnaisance unit with good speed but no shields. Their torpedo attack allows you to intercept enemy supply lines. Use them to find and slip behind enemy positions and harrass their support!\n \n";
		s += p.sprintf(L"Power to construct: %d Power\n", (int)DigitanksGame()->GetConstructionCost(UNIT_SCOUT));

		if (!GetDigitanksTeam()->GetUnusedFleetPoints())
			s += L"NOT ENOUGH FLEET POINTS\n \n";

		if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(UNIT_SCOUT))
			s += L"NOT ENOUGH POWER\n \n";

		s += L"Shortcut: D";
		pHUD->SetButtonInfo(7, s);
		pHUD->SetButtonTooltip(7, L"Build Rogue");

		pHUD->SetButtonTexture(8, "Firewall");
		if (GetDigitanksTeam()->GetPower() >= DigitanksGame()->GetConstructionCost(STRUCTURE_FIREWALL))
		{
			pHUD->SetButtonColor(8, Color(150, 150, 150));
			pHUD->SetButtonListener(8, CHUD::BuildTurret);
		}

		s = L"BUILD FIREWALL\n \n";
		s += L"Set them up Firewalls around your perimiter to defend your base. If you don't fire them manually, they will automatically fire at all enemies at the end of your turn.\n \nFirewalls get weaker when they need to fire at more targets, so be sure to build enough for the job.\n \n";
		s += p.sprintf(L"Power to construct: %d Power\n", (int)DigitanksGame()->GetConstructionCost(STRUCTURE_FIREWALL));

		if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(STRUCTURE_FIREWALL))
			s += L"NOT ENOUGH POWER\n \n";

		s += L"Shortcut: F";
		pHUD->SetButtonInfo(8, s);
		pHUD->SetButtonTooltip(8, L"Build Firewall");
	}
}

bool CCPU::NeedsOrders()
{
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
		if (flDistance > 8)
			return false;

		if (pResource->HasCollector())
		{
			CCollector* pCollector = pResource->GetCollector();

			if (pCollector->GetTeam() != GetTeam())
				return false;

			if (m_ePreviewStructure == STRUCTURE_BATTERY)
				return false;

			if (m_ePreviewStructure == STRUCTURE_PSU && pCollector->GetUnitType() == STRUCTURE_PSU)
				return false;

			// If we're building a PSU and the structure is a battery, that's cool.
		}
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

	if (GetPreviewStructure() == STRUCTURE_BATTERY || GetPreviewStructure() == STRUCTURE_PSU)
	{
		CResource* pResource = CResource::FindClosestResource(vecPreviewBuild, RESOURCE_ELECTRONODE);

		if ((pResource->GetOrigin() - vecPreviewBuild).Length() <= 8)
			m_vecPreviewBuild = pResource->GetOrigin();
	}
}

void CCPU::ClearPreviewBuild()
{
	m_vecPreviewBuild = GetOrigin();
}

bool CCPU::BeginConstruction()
{
	if (!IsPreviewBuildValid())
		return false;

	if (GetPowerToConstruct(m_ePreviewStructure, GetPreviewBuild()) > GetDigitanksTeam()->GetPower())
		return false;

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

	bool bSuccess = false;

	// This is used for the bot to see if a build was successful.
	if (CNetwork::IsHost())
		bSuccess = (p.ui1 != ~0);
	else
		bSuccess = true;

	if (bSuccess && GetTeam())
	{
		int iRunners = RandomInt(15, 10);
		for (int i = 0; i < iRunners; i++)
			DigitanksGame()->GetTerrain()->AddRunner(GetPreviewBuild(), GetTeam()->GetColor(), 1);
	}

	return bSuccess;
}

void CCPU::BeginConstruction(CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	// Overload this so that things this function calls get replicated.
	CNetwork::SetRunningClientFunctions(false);

	unittype_t ePreviewStructure = (unittype_t)p->i2;
	Vector vecPreview(p->fl3, p->fl4, p->fl5);

	if (GetPowerToConstruct(ePreviewStructure, vecPreview) > GetDigitanksTeam()->GetPower())
		return;

	if (ePreviewStructure == STRUCTURE_PSU)
	{
		CResource* pResource = CResource::FindClosestResource(vecPreview, RESOURCE_ELECTRONODE);
		if (pResource->HasCollector())
		{
			CCollector* pCollector = pResource->GetCollector();
			if (pCollector->GetTeam() == GetTeam() && pCollector->GetUnitType() == STRUCTURE_BATTERY)
			{
				pCollector->BeginUpgrade();
				return;
			}
		}
	}

	CStructure* pConstructing = NULL;

	if (ePreviewStructure == STRUCTURE_FIREWALL)
	{
		pConstructing = GameServer()->Create<CAutoTurret>("CAutoTurret");
	}
	else if (ePreviewStructure == STRUCTURE_MINIBUFFER)
	{
		pConstructing = GameServer()->Create<CMiniBuffer>("CMiniBuffer");
	}
	else if (ePreviewStructure == STRUCTURE_BUFFER)
	{
		if (!GetDigitanksTeam()->CanBuildBuffers())
			return;

		pConstructing = GameServer()->Create<CBuffer>("CBuffer");
	}
	else if (ePreviewStructure == STRUCTURE_BATTERY)
	{
		pConstructing = GameServer()->Create<CBattery>("CBattery");
	}
	else if (ePreviewStructure == STRUCTURE_PSU)
	{
		if (!GetDigitanksTeam()->CanBuildPSUs())
			return;

		pConstructing = GameServer()->Create<CCollector>("CCollector");
	}
	else if (ePreviewStructure == STRUCTURE_INFANTRYLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildInfantryLoaders())
			return;

		pConstructing = GameServer()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(pConstructing);
		if (pLoader)
			pLoader->SetBuildUnit(UNIT_INFANTRY);
	}
	else if (ePreviewStructure == STRUCTURE_TANKLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildTankLoaders())
			return;

		pConstructing = GameServer()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(pConstructing);
		if (pLoader)
			pLoader->SetBuildUnit(UNIT_TANK);
	}
	else if (ePreviewStructure == STRUCTURE_ARTILLERYLOADER)
	{
		if (!GetDigitanksTeam()->CanBuildArtilleryLoaders())
			return;

		pConstructing = GameServer()->Create<CLoader>("CLoader");

		CLoader* pLoader = dynamic_cast<CLoader*>(pConstructing);
		if (pLoader)
			pLoader->SetBuildUnit(UNIT_ARTILLERY);
	}

	if (CNetwork::IsHost())
		p->ui1 = pConstructing->GetHandle();

	GetDigitanksTeam()->ConsumePower(GetPowerToConstruct(ePreviewStructure, vecPreview));

	pConstructing->BeginConstruction(vecPreview);
	GetTeam()->AddEntity(pConstructing);
	pConstructing->SetSupplier(FindClosestSupplier(vecPreview, GetTeam()));
	pConstructing->GetSupplier()->AddChild(pConstructing);

	pConstructing->SetOrigin(vecPreview);
	pConstructing->CalculateVisibility();

	if (ePreviewStructure == STRUCTURE_PSU || ePreviewStructure == STRUCTURE_BATTERY)
	{
		Vector vecPSU = vecPreview;

		CResource* pResource = CBaseEntity::FindClosest<CResource>(vecPSU);

		if (pResource)
		{
			if ((pResource->GetOrigin() - vecPSU).Length() <= 8)
				pConstructing->SetOrigin(pResource->GetOrigin());
		}
	}

	CSupplier* pSupplier = dynamic_cast<CSupplier*>(pConstructing);
	if (pSupplier)
		pSupplier->GiveDataStrength((size_t)pSupplier->GetSupplier()->GetDataFlow(pSupplier->GetOrigin()));

	CCollector* pCollector = dynamic_cast<CCollector*>(pConstructing);
	if (pCollector)
	{
		pCollector->SetResource(CResource::FindClosestResource(vecPreview, pCollector->GetResourceType()));
		pCollector->GetResource()->SetCollector(pCollector);
	}

	if (CNetwork::IsHost() && pConstructing->GetTurnsRemainingToConstruct() == 0)
		pConstructing->CompleteConstruction();

	GetDigitanksTeam()->CountProducers();

	DigitanksWindow()->GetInstructor()->FinishedTutorial("strategy-placebuffer", true);

	pConstructing->FindGround();

	for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
	{
		for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
		{
			if (GetDigitanksTeam()->HasDownloadedUpdate(x, y))
				pConstructing->InstallUpdate(x, y);
		}
	}

	DigitanksGame()->GetTerrain()->CalculateVisibility();

	if (DigitanksGame()->GetTurn() == 0 && GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
	{
		// Let's see our action items.
		GetDigitanksTeam()->ClearActionItems();
		GetDigitanksTeam()->AddActionItem(NULL, ACTIONTYPE_WELCOME);
		GetDigitanksTeam()->AddActionItem(NULL, ACTIONTYPE_CONTROLS);
		GetDigitanksTeam()->AddActionItem(NULL, ACTIONTYPE_DOWNLOADUPDATES);

		for (size_t i = 0; i < GetDigitanksTeam()->GetNumTanks(); i++)
			GetDigitanksTeam()->AddActionItem(GetDigitanksTeam()->GetTank(i), ACTIONTYPE_UNITORDERS);

		DigitanksWindow()->GetHUD()->ShowActionItem(ACTIONTYPE_WELCOME);

		CInstructor* pInstructor = DigitanksWindow()->GetInstructor();
		pInstructor->SetActive(false);
	}

	DigitanksWindow()->GetHUD()->Layout();
}

float CCPU::GetPowerToConstruct(unittype_t eStructure, Vector vecLocation)
{
	float flPowerToConstruct = DigitanksGame()->GetConstructionCost(eStructure);

	return flPowerToConstruct;
}

void CCPU::BeginRogueProduction()
{
	if (IsProducing())
		return;

	if (DigitanksGame()->GetConstructionCost(UNIT_SCOUT) > GetDigitanksTeam()->GetPower())
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
	if (IsProducing())
		return;

	if (DigitanksGame()->GetConstructionCost(UNIT_SCOUT) > GetDigitanksTeam()->GetPower())
		return;

	if (!GetDigitanksTeam()->GetUnusedFleetPoints())
		return;

	m_iTurnsToProduceRogue = 1;
	m_bProducing = true;
	GetDigitanksTeam()->ConsumePower(DigitanksGame()->GetConstructionCost(UNIT_SCOUT));

	GetDigitanksTeam()->CountFleetPoints();
	GetDigitanksTeam()->CountProducers();
}

void CCPU::StartTurn()
{
	BaseClass::StartTurn();

	if (m_bProducing)
	{
		m_iTurnsToProduceRogue -= (size_t)1;
		if (m_iTurnsToProduceRogue == (size_t)0)
		{
			if (CNetwork::IsHost())
			{
				CDigitank* pTank = GameServer()->Create<CScout>("CScout");
				pTank->SetOrigin(GetOrigin());
				pTank->CalculateVisibility();
				GetTeam()->AddEntity(pTank);

				for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
				{
					for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
					{
						if (GetDigitanksTeam()->HasDownloadedUpdate(x, y))
							pTank->DownloadComplete(x, y);
					}
				}

				// Face him toward the center.
				pTank->Move(pTank->GetOrigin() + -GetOrigin().Normalized()*15);
				pTank->Turn(VectorAngles(-GetOrigin().Normalized()));

				pTank->StartTurn();
			}

			m_bProducing = false;

			GetDigitanksTeam()->AppendTurnInfo(L"Production finished on Rogue");

			GetDigitanksTeam()->AddActionItem(this, ACTIONTYPE_UNITREADY);
		}
		else
		{
			GetDigitanksTeam()->AppendTurnInfo(sprintf(L"Producing Rogue (%d turns left)", m_iTurnsToProduceRogue.Get()));
		}
	}
}

void CCPU::ModifyContext(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);

	if (GameServer()->GetGameTime() - m_flConstructionStartTime < 3)
	{
		pContext->SetBlend(BLEND_ALPHA);
		pContext->SetColor(Color(255, 255, 255));
		pContext->SetAlpha(GetVisibility() * RemapValClamped(GameServer()->GetGameTime() - m_flConstructionStartTime, 0, 2, 0, 1));
		pContext->Translate(Vector(0, RemapValClamped(GameServer()->GetGameTime() - m_flConstructionStartTime, 0, 3, -3, 0), 0));
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

	if (GetTeam())
		m_flFanRotation -= 100 * GameServer()->GetFrameTime();

	r.Rotate(m_flFanRotation, Vector(0, 1, 0));

	r.RenderModel(m_iFanModel);
}

bool CCPU::IsAvailableAreaActive(int iArea) const
{
	if (iArea <= 1)
		return BaseClass::IsAvailableAreaActive(iArea);

	if (DigitanksGame()->GetControlMode() != MODE_BUILD)
		return false;

	if (m_ePreviewStructure != STRUCTURE_BUFFER && m_ePreviewStructure != STRUCTURE_MINIBUFFER && m_ePreviewStructure != STRUCTURE_FIREWALL)
		return false;

	if (!IsPreviewBuildValid())
		return false;

	return true;
}

void CCPU::RenderAvailableArea(int iArea)
{
	if (iArea < 2)
	{
		BaseClass::RenderAvailableArea(iArea);
		return;
	}

	if (DigitanksGame()->GetControlMode() != MODE_BUILD)
		return;

	if (m_ePreviewStructure != STRUCTURE_BUFFER && m_ePreviewStructure != STRUCTURE_MINIBUFFER && m_ePreviewStructure != STRUCTURE_FIREWALL)
		return;

	if (!IsPreviewBuildValid())
		return;

	size_t iAreaSize;
	if (m_ePreviewStructure == STRUCTURE_BUFFER)
		iAreaSize = CBuffer::InitialBufferDataStrength();
	else
		iAreaSize = CMiniBuffer::InitialMiniBufferDataStrength();

	float flRadius = sqrt((float)iAreaSize/M_PI) + 5;	// 5 is the bounding radius for both structures

	if (m_ePreviewStructure == STRUCTURE_FIREWALL)
		flRadius = CAutoTurret::DefenseRadius();

	CRenderingContext c(GameServer()->GetRenderer());
	c.Translate(GetPreviewBuild());
	c.Scale(flRadius, flRadius, flRadius);
	c.RenderSphere();
}

void CCPU::UpdateInfo(eastl::string16& s)
{
	eastl::string16 p;
	s = L"";
	s += L"CENTRAL PROCESSING UNIT\n";
	s += L"Command center\n \n";

	if (GetTeam())
	{
		s += L"Team: " + GetTeam()->GetTeamName() + L"\n";
		if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
			s += L" Friendly\n \n";
		else
			s += L" Hostile\n \n";
	}
	else
	{
		s += L"Team: Neutral\n \n";
	}

	if (IsProducing())
	{
		s += L"[Producing Rogue]\n";
		s += sprintf(L"Turns left: %d\n \n", m_iTurnsToProduceRogue.Get());
	}

	s += p.sprintf(L"Power: %.1f/turn\n", Power());
	s += p.sprintf(L"Fleet Points: %d\n", FleetPoints());
	s += p.sprintf(L"Bandwidth: %.1f/turn\n", Bandwidth());
	s += p.sprintf(L"Network Size: %d\n", (int)GetDataFlowRadius());
	s += p.sprintf(L"Efficiency: %d\n", (int)(GetChildEfficiency()*100));
}

void CCPU::OnDeleted()
{
	eastl::vector<CBaseEntity*> apDeleteThese;

	if (!GetTeam())
		return;

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
