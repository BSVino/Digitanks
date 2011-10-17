#include "loader.h"

#include <mtrand.h>

#include <models/models.h>
#include <renderer/renderer.h>
#include <game/game.h>

#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <ui/hud.h>

#include <units/mechinf.h>
#include <units/maintank.h>
#include <units/artillery.h>

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

INPUTS_TABLE_BEGIN(CLoader);
INPUTS_TABLE_END();

void CLoader::Precache()
{
	BaseClass::Precache();

	PrecacheModel(_T("models/structures/loader-infantry.obj"));
	PrecacheModel(_T("models/structures/loader-main.obj"));
	PrecacheModel(_T("models/structures/loader-artillery.obj"));
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
			GetDigitanksTeam()->AppendTurnInfo(s);
		}
	}
}

void CLoader::PostRender(bool bTransparent) const
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
		s += _T("BUILD RESISTOR\n \n");
		s += _T("Resistors can fortify, gaining attack and shield energy bonuses over time. They are fantastic defense platforms, but once fortified they can't be moved.\n \n");
		pHUD->SetButtonTooltip(0, _T("Build Resistor"));
	}
	else if (GetBuildUnit() == UNIT_TANK)
	{
		pHUD->SetButtonTexture(0, "Digitank");
		s += _T("BUILD DIGITANK\n \n");
		s += _T("Digitanks are the core of any digital tank fleet. Although expensive, they are the only real way of taking territory from your enemies.\n \n");
		pHUD->SetButtonTooltip(0, _T("Build Digitank"));
	}
	else
	{
		pHUD->SetButtonTexture(0, "Artillery");
		s += _T("BUILD ARTILLERY\n \n");
		s += _T("Artillery must be deployed before use and can only fire in front of themselves, but have ridiculous range and can pummel the enemy from afar. Artillery does double damage to shields, but only half damage to structures and tank hulls. Use them to soften enemy positions before moving in.\n \n");
		pHUD->SetButtonTooltip(0, _T("Build Artillery"));
	}

	s += sprintf(tstring("Fleet supply required: %d\n"), GetFleetPointsRequired());
	s += sprintf(tstring("Power required: %d\n"), (int)DigitanksGame()->GetConstructionCost(GetBuildUnit()));
	s += sprintf(tstring("Turns to produce: %d Turns\n \n"), GetTurnsToProduce());

	if (GetDigitanksTeam()->GetUnusedFleetPoints() < GetFleetPointsRequired())
		s += _T("NOT ENOUGH FLEET POINTS\n \n");

	if (GetDigitanksTeam()->GetPower() < DigitanksGame()->GetConstructionCost(GetBuildUnit()))
		s += _T("NOT ENOUGH POWER\n \n");

	s += _T("Shortcut: Q");
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

	if (GameNetwork()->IsHost())
		BeginProduction(&p);
	else
		GameNetwork()->CallFunctionParameters(NETWORK_TOSERVER, "BeginProduction", &p);
}

void CLoader::BeginProduction(class CNetworkParameters* p)
{
	if (!GameNetwork()->IsHost())
		return;

	if (GetDigitanksTeam()->GetPower() < GetUnitProductionCost())
		return;

	m_iTurnsToProduce = GetTurnsToProduce();
	m_bProducing = true;

	GetDigitanksTeam()->ConsumePower(GetUnitProductionCost());

	GetDigitanksTeam()->CountFleetPoints();
	GetDigitanksTeam()->CountProducers();
}

void CLoader::CompleteProduction()
{
	if (GetBuildUnit() == UNIT_INFANTRY)
		GetDigitanksTeam()->AppendTurnInfo(_T("Production finished on Resistor"));
	else if (GetBuildUnit() == UNIT_TANK)
		GetDigitanksTeam()->AppendTurnInfo(_T("Production finished on Digitank"));
	else if (GetBuildUnit() == UNIT_ARTILLERY)
		GetDigitanksTeam()->AppendTurnInfo(_T("Production finished on Artillery"));

	GetDigitanksTeam()->AddActionItem(this, ACTIONTYPE_UNITREADY);

	if (GameNetwork()->IsHost())
	{
		CDigitank* pTank;
		if (GetBuildUnit() == UNIT_INFANTRY)
			pTank = GameServer()->Create<CMechInfantry>("CMechInfantry");
		else if (GetBuildUnit() == UNIT_TANK)
			pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
		else if (GetBuildUnit() == UNIT_ARTILLERY)
			pTank = GameServer()->Create<CArtillery>("CArtillery");
			
		pTank->SetOrigin(GetOrigin());
		pTank->CalculateVisibility();

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

		if (!GetTeam()->IsPlayerControlled() && GameNetwork()->IsHost())
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
		SetModel(_T("models/structures/loader-infantry.obj"));
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/infantry-body.obj"));
		break;

	case UNIT_TANK:
		SetModel(_T("models/structures/loader-main.obj"));
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/digitank-body.obj"));
		break;

	case UNIT_ARTILLERY:
		SetModel(_T("models/structures/loader-artillery.obj"));
		m_iBuildUnitModel = CModelLibrary::Get()->FindModel(_T("models/digitanks/artillery.obj"));
		break;
	}
}

void CLoader::UpdateInfo(tstring& s)
{
	s = _T("");
	tstring p;

	if (GetTeam())
	{
		s += _T("Team: ") + GetTeam()->GetTeamName() + _T("\n");
		if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
			s += _T(" Friendly\n \n");
		else
			s += _T(" Hostile\n \n");
	}
	else
	{
		s += _T("Team: Neutral\n \n");
	}

	if (GetBuildUnit() == UNIT_INFANTRY)
		s += _T("RESISTOR FACTORY\n");
	else if (GetBuildUnit() == UNIT_TANK)
		s += _T("DIGITANK FACTORY\n");
	else
		s += _T("ARTILLERY FACTORY\n");

	s += _T("Unit producer\n \n");

	if (IsConstructing())
	{
		s += _T("(Constructing)\n");
		s += sprintf(tstring("Turns left: %d\n"), GetTurnsRemainingToConstruct());
		return;
	}

	if (IsProducing() && GetSupplier() && GetSupplyLine())
	{
		s += _T("(Producing)\n");
		s += sprintf(tstring("Turns left: %d\n \n"), GetTurnsRemainingToProduce());
	}

	if (GetSupplier() && GetSupplyLine())
		s += sprintf(tstring("Efficiency: %d%\n"), (int)(GetSupplier()->GetChildEfficiency()*m_hSupplyLine->GetIntegrity()*100));
}

void CLoader::DrawQueue(int x, int y, int w, int h)
{
	glgui::CBaseControl::PaintRect(x, y, w, h, Color(0, 0, 0));

	if (!GetTeam())
		return;

	if (!IsProducing())
		return;

	int iSize = h*2/3;

	CHUD::PaintUnitSheet(GetBuildUnit(), x + w/2 - iSize/2, y + h/2 - iSize/2, iSize, iSize, GetTeam()->GetColor());

	int iTotalTurns = GetTurnsToProduce();
	int iTurnsProgressed = iTotalTurns-GetTurnsRemainingToProduce();
	iTurnsProgressed++;
	iTotalTurns++;

	glgui::CRootPanel::PaintRect(x + w/2 - iSize/2, y + h/2 + iSize/2, (int)(iSize*iTurnsProgressed/iTotalTurns), 3, Color(255, 255, 0));

	CRenderingContext c(GameServer()->GetRenderer());
	c.SetColor(Color(255,255,255));
	tstring sTurns = sprintf(tstring(":%d"), GetTurnsRemainingToProduce());
	glgui::CLabel::PaintText(sTurns, sTurns.length(), _T("text"), 10, (float)(x + w/2 + iSize/2), (float)(y + h/2 - iSize/2 - 2));
}

tstring CLoader::GetEntityName() const
{
	if (GetBuildUnit() == UNIT_INFANTRY)
		return _T("Resistor Factory");
	else if (GetBuildUnit() == UNIT_TANK)
		return _T("Digitank Factory");
	else
		return _T("Artillery Factory");
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
