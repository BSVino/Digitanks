#include "loader.h"

#include <mtrand.h>

#include <models/models.h>
#include <renderer/renderer.h>
#include <game/game.h>

#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <ui/hud.h>

#include <digitanks/units/mechinf.h>
#include <digitanks/units/maintank.h>
#include <digitanks/units/artillery.h>

#include <GL/glew.h>

REGISTER_ENTITY(CLoader);

NETVAR_TABLE_BEGIN(CLoader);
	NETVAR_DEFINE(unittype_t, m_eBuildUnit);
	NETVAR_DEFINE(size_t, m_iBuildUnitModel);
	NETVAR_DEFINE(bool, m_bProducing);
	NETVAR_DEFINE(size_t, m_iTurnsToProduce);
	NETVAR_DEFINE(size_t, m_iTankAttack);
	NETVAR_DEFINE(size_t, m_iTankDefense);
	NETVAR_DEFINE(size_t, m_iTankMovement);
	NETVAR_DEFINE(size_t, m_iTankHealth);
	NETVAR_DEFINE(size_t, m_iTankRange);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CLoader);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, unittype_t, m_eBuildUnit);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iBuildUnitModel);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bProducing);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTurnsToProduce);
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

	m_bProducing = false;
}

void CLoader::StartTurn()
{
	BaseClass::StartTurn();

	if (m_bProducing && m_hSupplier != NULL && m_hSupplyLine != NULL)
	{
		m_iTurnsToProduce--;
		if (m_iTurnsToProduce == (size_t)0)
		{
			CompleteProduction();
		}
		else
		{
			eastl::string16 s;
			if (GetBuildUnit() == UNIT_INFANTRY)
				s.sprintf(L"Producing Resistor (%d turns left)", GetTurnsRemainingToProduce());
			else if (GetBuildUnit() == UNIT_TANK)
				s.sprintf(L"Producing Digitank (%d turns left)", GetTurnsRemainingToProduce());
			else if (GetBuildUnit() == UNIT_ARTILLERY)
				s.sprintf(L"Producing Artillery (%d turns left)", GetTurnsRemainingToProduce());
			DigitanksGame()->AppendTurnInfo(s);
		}
	}
}

void CLoader::PostRender(bool bTransparent)
{
	BaseClass::PostRender(bTransparent);

	if (bTransparent && IsProducing() && GetVisibility() > 0)
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

	CHUD* pHUD = DigitanksWindow()->GetHUD();
	eastl::string16 p;

	if (HasEnoughFleetPoints() && HasEnoughPower() && !IsProducing())
	{
		pHUD->SetButtonListener(0, CHUD::BuildUnit);
		pHUD->SetButtonColor(0, Color(150, 150, 150));
	}

	eastl::string16 s;

	if (GetBuildUnit() == UNIT_INFANTRY)
	{
		pHUD->SetButtonTexture(0, 256, 0);
		s += L"BUILD RESISTOR\n \n";
		s += L"Resistors can fortify, gaining attack and shield energy bonuses over time. They are fantastic defense platforms, but once fortified they can't be moved.\n \n";
		pHUD->SetButtonTooltip(0, L"Build Resistor");
	}
	else if (GetBuildUnit() == UNIT_TANK)
	{
		pHUD->SetButtonTexture(0, 64, 64);
		s += L"BUILD DIGITANK\n \n";
		s += L"Digitanks are the core of any digital tank fleet. Although expensive, they are the only real way of taking territory from your enemies.\n \n";
		pHUD->SetButtonTooltip(0, L"Build Digitank");
	}
	else
	{
		pHUD->SetButtonTexture(0, 64, 0);
		s += L"BUILD ARTILLERY\n \n";
		s += L"Artillery must be deployed before use and can only fire in front of themselves, but have ridiculous range and can pummel the enemy from afar. Artillery does double damage to shields, but only half damage to structures and tank hulls. Use them to soften enemy positions before moving in.\n \n";
		pHUD->SetButtonTooltip(0, L"Build Artillery");
	}

	s += p.sprintf(L"Fleet supply required: %d\n", GetFleetPointsRequired());
	s += p.sprintf(L"Power required: %d\n", (int)DigitanksGame()->GetConstructionCost(GetBuildUnit()));
	s += p.sprintf(L"Turns to produce: %d Turns\n \n", GetTurnsToProduce());

	if (GetDigitanksTeam()->GetUnusedFleetPoints() < GetFleetPointsRequired())
		s += L"NOT ENOUGH FLEET POINTS\n \n";

	if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(GetBuildUnit()))
		s += L"NOT ENOUGH POWER\n \n";

	s += L"Shortcut: Q";
	pHUD->SetButtonInfo(0, s);
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

	if (IsProducing())
		return;

	if (!HasEnoughFleetPoints())
		return;

	if (GetDigitanksTeam()->GetPower() < GetUnitProductionCost())
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

	if (GetDigitanksTeam()->GetPower() < GetUnitProductionCost())
		return;

	m_iTurnsToProduce = GetTurnsToProduce();
	m_bProducing = true;

	GetDigitanksTeam()->ConsumePower(GetUnitProductionCost());

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_PRODUCING_UNITS);

	size_t iTutorial = DigitanksWindow()->GetInstructor()->GetCurrentTutorial();
	if (iTutorial == CInstructor::TUTORIAL_PRODUCING_UNITS)
		DigitanksWindow()->GetInstructor()->NextTutorial();

	GetDigitanksTeam()->CountFleetPoints();
	GetDigitanksTeam()->CountProducers();
}

void CLoader::CompleteProduction()
{
	if (GetBuildUnit() == UNIT_INFANTRY)
		DigitanksGame()->AppendTurnInfo(L"Production finished on Resistor");
	else if (GetBuildUnit() == UNIT_TANK)
		DigitanksGame()->AppendTurnInfo(L"Production finished on Digitank");
	else if (GetBuildUnit() == UNIT_ARTILLERY)
		DigitanksGame()->AppendTurnInfo(L"Production finished on Artillery");

	DigitanksGame()->AddActionItem(this, ACTIONTYPE_UNITREADY);

	if (CNetwork::IsHost())
	{
		CDigitank* pTank;
		if (GetBuildUnit() == UNIT_INFANTRY)
			pTank = GameServer()->Create<CMechInfantry>("CMechInfantry");
		else if (GetBuildUnit() == UNIT_TANK)
			pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		else if (GetBuildUnit() == UNIT_ARTILLERY)
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

		for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
		{
			for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
			{
				if (GetDigitanksTeam()->HasDownloadedUpdate(x, y))
					pTank->DownloadComplete(x, y);
			}
		}

		m_bProducing = false;

		pTank->Move(pTank->GetOrigin() + AngleVector(pTank->GetAngles())*9);
		pTank->Turn(VectorAngles(-GetOrigin().Normalized()));

		if (!GetTeam()->IsPlayerControlled() && CNetwork::IsHost())
			// It's not a real move but i dun care
			// We just need to get them spaced out around the loader, like a rally point
			pTank->Move(pTank->GetOrigin() + AngleVector(pTank->GetAngles())*9 + AngleVector(EAngle(0, RandomFloat(0, 360), 0))*10);

		pTank->StartTurn();
	}
}

float CLoader::GetUnitProductionCost()
{
	return DigitanksGame()->GetConstructionCost(m_eBuildUnit);
}

void CLoader::InstallUpdate(size_t x, size_t y)
{
	BaseClass::InstallUpdate(x, y);

	CUpdateItem* pUpdate = &DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y];

	if (pUpdate->m_eStructure != GetUnitType())
		return;

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
}

size_t CLoader::GetFleetPointsRequired()
{
	if (m_eBuildUnit == UNIT_INFANTRY)
		return CMechInfantry::InfantryFleetPoints();
	else if (m_eBuildUnit == UNIT_ARTILLERY)
		return CArtillery::ArtilleryFleetPoints();
	else if (m_eBuildUnit == UNIT_TANK)
		return CMainBattleTank::MainTankFleetPoints();

	return 0;
}

bool CLoader::HasEnoughFleetPoints()
{
	if (!GetDigitanksTeam())
		return false;

	return GetFleetPointsRequired() <= GetDigitanksTeam()->GetUnusedFleetPoints();
}

bool CLoader::HasEnoughPower()
{
	if (!GetDigitanksTeam())
		return false;

	return DigitanksGame()->GetConstructionCost(GetBuildUnit()) <= GetDigitanksTeam()->GetPower();
}

size_t CLoader::InitialTurnsToConstruct()
{
	if (m_eBuildUnit == UNIT_INFANTRY)
		return 2;
	else if (m_eBuildUnit == UNIT_ARTILLERY)
		return 4;
	else if (m_eBuildUnit == UNIT_TANK)
		return 5;

	return 2;
}

size_t CLoader::GetTurnsToProduce()
{
	if (m_eBuildUnit == UNIT_INFANTRY)
		return 2;

	if (m_eBuildUnit == UNIT_TANK)
		return 4;

	if (m_eBuildUnit == UNIT_ARTILLERY)
		return 5;

	return 2;
}

void CLoader::SetBuildUnit(unittype_t eBuildUnit)
{
	m_eBuildUnit = eBuildUnit;

	switch (m_eBuildUnit)
	{
	case UNIT_INFANTRY:
		SetModel(L"models/structures/loader-infantry.obj");
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel(L"models/digitanks/infantry-body.obj");
		break;

	case UNIT_TANK:
		SetModel(L"models/structures/loader-main.obj");
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-body.obj");
		break;

	case UNIT_ARTILLERY:
		SetModel(L"models/structures/loader-artillery.obj");
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel(L"models/digitanks/artillery.obj");
		break;
	}
}

void CLoader::UpdateInfo(eastl::string16& s)
{
	s = L"";
	eastl::string16 p;

	if (GetTeam())
	{
		s += L"Team: " + GetTeam()->GetName() + L"\n";
		if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
			s += L" Friendly\n \n";
		else
			s += L" Hostile\n \n";
	}
	else
	{
		s += L"Team: Neutral\n \n";
	}

	if (GetBuildUnit() == UNIT_INFANTRY)
		s += L"RESISTOR FACTORY\n";
	else if (GetBuildUnit() == UNIT_TANK)
		s += L"DIGITANK FACTORY\n";
	else
		s += L"ARTILLERY FACTORY\n";

	s += L"Unit producer\n \n";

	if (IsConstructing())
	{
		s += L"(Constructing)\n";
		s += p.sprintf(L"Turns left: %d\n", GetTurnsRemainingToConstruct());
		return;
	}

	if (IsProducing() && GetSupplier() && GetSupplyLine())
	{
		s += L"(Producing)\n";
		s += p.sprintf(L"Turns left: %d\n \n", GetTurnsRemainingToProduce());
	}

	if (GetSupplier() && GetSupplyLine())
		s += p.sprintf(L"Efficiency: %d%\n", (int)(GetSupplier()->GetChildEfficiency()*m_hSupplyLine->GetIntegrity()*100));
}

eastl::string16 CLoader::GetName()
{
	if (GetBuildUnit() == UNIT_INFANTRY)
		return L"Resistor Factory";
	else if (GetBuildUnit() == UNIT_TANK)
		return L"Digitank Factory";
	else
		return L"Artillery Factory";
}

unittype_t CLoader::GetUnitType() const
{
	if (GetBuildUnit() == UNIT_INFANTRY)
		return STRUCTURE_INFANTRYLOADER;
	else if (GetBuildUnit() == UNIT_TANK)
		return STRUCTURE_TANKLOADER;
	else
		return STRUCTURE_ARTILLERYLOADER;
}
