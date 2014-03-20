#include "loader.h"

#include <mtrand.h>

#include <models/models.h>
#include <renderer/game_renderer.h>
#include <game/entities/game.h>
#include <game/gameserver.h>
#include <renderer/game_renderingcontext.h>
#include <glgui/rootpanel.h>

#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <ui/hud.h>

#include <units/mechinf.h>
#include <units/maintank.h>
#include <units/artillery.h>

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

INPUTS_TABLE_BEGIN(CLoader);
INPUTS_TABLE_END();

void CLoader::Precache()
{
	BaseClass::Precache();

	PrecacheModel("models/structures/loader-infantry.toy");
	PrecacheModel("models/structures/loader-main.toy");
	PrecacheModel("models/structures/loader-artillery.toy");
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
			tstring s;
			if (GetBuildUnit() == UNIT_INFANTRY)
				s = sprintf(tstring("Producing Resistor (%d turns left)"), GetTurnsRemainingToProduce());
			else if (GetBuildUnit() == UNIT_TANK)
				s = sprintf(tstring("Producing Digitank (%d turns left)"), GetTurnsRemainingToProduce());
			else if (GetBuildUnit() == UNIT_ARTILLERY)
				s = sprintf(tstring("Producing Artillery (%d turns left)"), GetTurnsRemainingToProduce());
			GetDigitanksPlayer()->AppendTurnInfo(s);
		}
	}
}

void CLoader::PostRender() const
{
	BaseClass::PostRender();

	if (GameServer()->GetRenderer()->IsRenderingTransparent() && IsProducing() && GetVisibility() > 0)
	{
		CGameRenderingContext c(GameServer()->GetRenderer());
		c.Translate(GetGlobalOrigin());
		c.SetAlpha(GetVisibility() * 0.3f);
		c.SetBlend(BLEND_ADDITIVE);
		if (GetPlayerOwner())
		{
			c.SetUniform("bColorSwapInAlpha", true);
			c.SetUniform("vecColorSwap", GetPlayerOwner()->GetColor());
		}
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
	tstring p;

	if (HasEnoughFleetPoints() && HasEnoughPower() && !IsProducing())
	{
		pHUD->SetButtonListener(0, CHUD::BuildUnit);
		pHUD->SetButtonColor(0, Color(50, 50, 50));
	}
	else
		pHUD->SetButtonColor(0, Color(100, 100, 100));

	tstring s;

	if (GetBuildUnit() == UNIT_INFANTRY)
	{
		pHUD->SetButtonTexture(0, "Resistor");
		s += "BUILD RESISTOR\n \n";
		s += "Resistors can fortify, gaining attack and shield energy bonuses over time. They are fantastic defense platforms, but once fortified they can't be moved.\n \n";
		pHUD->SetButtonTooltip(0, "Build Resistor");
	}
	else if (GetBuildUnit() == UNIT_TANK)
	{
		pHUD->SetButtonTexture(0, "Digitank");
		s += "BUILD DIGITANK\n \n";
		s += "Digitanks are the core of any digital tank fleet. Although expensive, they are the only real way of taking territory from your enemies.\n \n";
		pHUD->SetButtonTooltip(0, "Build Digitank");
	}
	else
	{
		pHUD->SetButtonTexture(0, "Artillery");
		s += "BUILD ARTILLERY\n \n";
		s += "Artillery must be deployed before use and can only fire in front of themselves, but have ridiculous range and can pummel the enemy from afar. Artillery does double damage to shields, but only half damage to structures and tank hulls. Use them to soften enemy positions before moving in.\n \n";
		pHUD->SetButtonTooltip(0, "Build Artillery");
	}

	s += sprintf(tstring("Fleet supply required: %d\n"), GetFleetPointsRequired());
	s += sprintf(tstring("Power required: %d\n"), (int)DigitanksGame()->GetConstructionCost(GetBuildUnit()));
	s += sprintf(tstring("Turns to produce: %d Turns\n \n"), GetTurnsToProduce());

	if (GetDigitanksPlayer()->GetUnusedFleetPoints() < GetFleetPointsRequired())
		s += "NOT ENOUGH FLEET POINTS\n \n";

	if (GetDigitanksPlayer()->GetPower() < DigitanksGame()->GetConstructionCost(GetBuildUnit()))
		s += "NOT ENOUGH POWER\n \n";

	s += "Shortcut: Q";
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

	if (GetDigitanksPlayer()->GetPower() < GetUnitProductionCost())
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();

	if (GameNetwork()->IsHost())
		BeginProduction(&p);
	else
		GameNetwork()->CallFunctionParameters(NETWORK_TOSERVER, "BeginProduction", &p);
}

void CLoader::BeginProduction(class CNetworkParameters* p)
{
	if (!GameNetwork()->IsHost())
		return;

	if (GetDigitanksPlayer()->GetPower() < GetUnitProductionCost())
		return;

	m_iTurnsToProduce = GetTurnsToProduce();
	m_bProducing = true;

	GetDigitanksPlayer()->ConsumePower(GetUnitProductionCost());

	GetDigitanksPlayer()->CountFleetPoints();
	GetDigitanksPlayer()->CountProducers();
}

void CLoader::CompleteProduction()
{
	if (GetBuildUnit() == UNIT_INFANTRY)
		GetDigitanksPlayer()->AppendTurnInfo("Production finished on Resistor");
	else if (GetBuildUnit() == UNIT_TANK)
		GetDigitanksPlayer()->AppendTurnInfo("Production finished on Digitank");
	else if (GetBuildUnit() == UNIT_ARTILLERY)
		GetDigitanksPlayer()->AppendTurnInfo("Production finished on Artillery");

	GetDigitanksPlayer()->AddActionItem(this, ACTIONTYPE_UNITREADY);

	if (GameNetwork()->IsHost())
	{
		CDigitank* pTank;
		if (GetBuildUnit() == UNIT_INFANTRY)
			pTank = GameServer()->Create<CMechInfantry>("CMechInfantry");
		else if (GetBuildUnit() == UNIT_TANK)
			pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		else if (GetBuildUnit() == UNIT_ARTILLERY)
			pTank = GameServer()->Create<CArtillery>("CArtillery");
			
		pTank->SetGlobalOrigin(GetGlobalOrigin());
		pTank->CalculateVisibility();

		GetPlayerOwner()->AddUnit(pTank);

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
				if (GetDigitanksPlayer()->HasDownloadedUpdate(x, y))
					pTank->DownloadComplete(x, y);
			}
		}

		m_bProducing = false;

		pTank->Move(pTank->GetGlobalOrigin() + AngleVector(pTank->GetGlobalAngles()) * 9);
		pTank->Turn(VectorAngles(-GetGlobalOrigin().Normalized()));

		if (!GetPlayerOwner()->IsHumanControlled() && GameNetwork()->IsHost())
			// It's not a real move but i dun care
			// We just need to get them spaced out around the loader, like a rally point
			pTank->Move(pTank->GetGlobalOrigin() + AngleVector(pTank->GetGlobalAngles()) * 9 + AngleVector(EAngle(0, RandomFloat(0, 360), 0)) * 10);

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
	if (!GetDigitanksPlayer())
		return false;

	return GetFleetPointsRequired() <= GetDigitanksPlayer()->GetUnusedFleetPoints();
}

bool CLoader::HasEnoughPower()
{
	if (!GetDigitanksPlayer())
		return false;

	return DigitanksGame()->GetConstructionCost(GetBuildUnit()) <= GetDigitanksPlayer()->GetPower();
}

size_t CLoader::InitialTurnsToConstruct()
{
	if (m_eBuildUnit == UNIT_INFANTRY)
		return 1;
	else if (m_eBuildUnit == UNIT_ARTILLERY)
		return 2;
	else if (m_eBuildUnit == UNIT_TANK)
		return 2;

	return 1;
}

size_t CLoader::GetTurnsToProduce()
{
	if (m_eBuildUnit == UNIT_INFANTRY)
		return 1;

	if (m_eBuildUnit == UNIT_TANK)
		return 2;

	if (m_eBuildUnit == UNIT_ARTILLERY)
		return 2;

	return 2;
}

void CLoader::SetBuildUnit(unittype_t eBuildUnit)
{
	m_eBuildUnit = eBuildUnit;

	switch (m_eBuildUnit)
	{
	case UNIT_INFANTRY:
		SetModel("models/structures/loader-infantry.toy");
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel("models/digitanks/infantry-body.toy");
		break;

	case UNIT_TANK:
		SetModel("models/structures/loader-main.toy");
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel("models/digitanks/digitank-body.toy");
		break;

	case UNIT_ARTILLERY:
		SetModel("models/structures/loader-artillery.toy");
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel("models/digitanks/artillery.toy");
		break;
	}
}

void CLoader::UpdateInfo(tstring& s)
{
	s = "";
	tstring p;

	if (GetPlayerOwner())
	{
		s += "Team: " + GetPlayerOwner()->GetPlayerName() + "\n";
		if (GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
			s += " Friendly\n \n";
		else
			s += " Hostile\n \n";
	}
	else
	{
		s += "Team: Neutral\n \n";
	}

	if (GetBuildUnit() == UNIT_INFANTRY)
		s += "RESISTOR FACTORY\n";
	else if (GetBuildUnit() == UNIT_TANK)
		s += "DIGITANK FACTORY\n";
	else
		s += "ARTILLERY FACTORY\n";

	s += "Unit producer\n \n";

	if (IsConstructing())
	{
		s += "(Constructing)\n";
		s += sprintf(tstring("Turns left: %d\n"), GetTurnsRemainingToConstruct());
		return;
	}

	if (IsProducing() && GetSupplier() && GetSupplyLine())
	{
		s += "(Producing)\n";
		s += sprintf(tstring("Turns left: %d\n \n"), GetTurnsRemainingToProduce());
	}

	if (GetSupplier() && GetSupplyLine())
		s += sprintf(tstring("Efficiency: %d%\n"), (int)(GetSupplier()->GetChildEfficiency()*m_hSupplyLine->GetIntegrity()*100));
}

void CLoader::DrawQueue(float x, float y, float w, float h)
{
	glgui::CBaseControl::PaintRect(x, y, w, h, Color(0, 0, 0));

	if (!GetPlayerOwner())
		return;

	if (!IsProducing())
		return;

	float flSize = h*2/3;

	CHUD::PaintUnitSheet(GetBuildUnit(), x + w/2 - flSize/2, y + h/2 - flSize/2, flSize, flSize, GetPlayerOwner()->GetColor());

	int iTotalTurns = GetTurnsToProduce();
	int iTurnsProgressed = iTotalTurns-GetTurnsRemainingToProduce();
	iTurnsProgressed++;
	iTotalTurns++;

	glgui::CRootPanel::PaintRect(x + w/2 - flSize/2, y + h/2 + flSize/2, flSize*iTurnsProgressed/iTotalTurns, 3, Color(255, 255, 0));

	CRenderingContext c(GameServer()->GetRenderer());
	c.SetColor(Color(255,255,255));
	tstring sTurns = sprintf(tstring(":%d"), GetTurnsRemainingToProduce());
	glgui::CLabel::PaintText(sTurns, sTurns.length(), "text", 10, (float)(x + w/2 + flSize/2), (float)(y + h/2 - flSize/2 - 2));
}

tstring CLoader::GetEntityName() const
{
	if (GetBuildUnit() == UNIT_INFANTRY)
		return "Resistor Factory";
	else if (GetBuildUnit() == UNIT_TANK)
		return "Digitank Factory";
	else
		return "Artillery Factory";
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
