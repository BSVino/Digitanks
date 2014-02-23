#include "structure.h"

#include <maths.h>
#include <mtrand.h>
#include <strutils.h>

#include <renderer/shaders.h>
#include <models/models.h>
#include <network/network.h>
#include <textures/texturelibrary.h>
#include <tinker/cvar.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/roperenderer.h>

#include <dt_renderer.h>
#include <digitanksgame.h>
#include <supplyline.h>
#include <dt_camera.h>
#include <ui/digitankswindow.h>
#include <ui/hud.h>
#include <ui/instructor.h>

#include "collector.h"
#include "loader.h"

REGISTER_ENTITY(CStructure);

NETVAR_TABLE_BEGIN(CStructure);
	NETVAR_DEFINE(bool, m_bConstructing);
	NETVAR_DEFINE(size_t, m_iTurnsToConstruct);

	NETVAR_DEFINE(bool, m_bUpgrading);
	NETVAR_DEFINE(size_t, m_iTurnsToUpgrade);

	NETVAR_DEFINE(CEntityHandle<CSupplyLine>, m_hSupplier);
	NETVAR_DEFINE(CEntityHandle<CSupplyLine>, m_hSupplyLine);

	NETVAR_DEFINE(size_t, m_iFleetSupply);
	NETVAR_DEFINE(float, m_flBandwidth);
	NETVAR_DEFINE(float, m_flPowerProduced);
	NETVAR_DEFINE(size_t, m_iEnergyBonus);
	NETVAR_DEFINE(float, m_flRechargeBonus);

	NETVAR_DEFINE(float, m_flScaffoldingSize);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CStructure);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bConstructing);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTurnsToConstruct);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bUpgrading);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTurnsToUpgrade);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CSupplier>, m_hSupplier);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CSupplyLine>, m_hSupplyLine);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iFleetSupply);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBandwidth);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flPowerProduced);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iEnergyBonus);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flRechargeBonus);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, size_t, m_iScaffolding);	// In Spawn()
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flScaffoldingSize);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flConstructionStartTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, defender_t, m_aoDefenders);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CStructure);
INPUTS_TABLE_END();

CStructure::CStructure()
{
	m_bConstructing = false;
	m_iTurnsToConstruct = 0;

	m_bUpgrading = false;
	m_iTurnsToUpgrade = 0;

	m_bConstructing = true;
}

void CStructure::Precache()
{
	BaseClass::Precache();

	PrecacheModel("models/structures/scaffolding.toy");
}

void CStructure::Spawn()
{
	BaseClass::Spawn();

	m_iFleetSupply = InitialFleetPoints();
	m_flBandwidth = InitialBandwidth();
	m_flPowerProduced = InitialPower();
	m_iEnergyBonus = InitialEnergyBonus();
	m_flRechargeBonus = InitialRechargeBonus();

	m_iScaffolding = CModelLibrary::Get()->FindModel("models/structures/scaffolding.toy");
	m_flScaffoldingSize = 10;

	m_flConstructionStartTime = 0;

	m_bConstructing = true;

	m_hSupplyLine = NULL;
	m_hSupplier = NULL;
}

void CStructure::Think()
{
	BaseClass::Think();

	if (IsAlive())
		FindGround();

	if (m_flConstructionStartTime && GameServer()->GetGameTime() - m_flConstructionStartTime > 3)
		m_flConstructionStartTime = 0;
}

void CStructure::OnTeamChange()
{
	BaseClass::OnTeamChange();

	SetSupplier(NULL);
}

void CStructure::StartTurn()
{
	BaseClass::StartTurn();

	FindGround();

	if (!GetSupplier() && !dynamic_cast<CCPU*>(this))
	{
		if (GetPlayerOwner())
			GetPlayerOwner()->RemoveUnit(this);
		SetSupplier(NULL);
	}

	if (GetSupplier() && !GetSupplier()->GetPlayerOwner())
	{
		GetSupplier()->RemoveChild(this);
		if (GetPlayerOwner())
			GetPlayerOwner()->RemoveUnit(this);
		SetSupplier(NULL);
	}

	if (GetPlayerOwner() == NULL)
		return;

	if (IsConstructing())
	{
		m_iTurnsToConstruct--;

		if (m_iTurnsToConstruct == (size_t)0)
		{
			GetDigitanksPlayer()->AppendTurnInfo(tstring("Construction finished on ") + GetEntityName());
			CompleteConstruction();

			GetDigitanksPlayer()->AddActionItem(this, ACTIONTYPE_NEWSTRUCTURE);
		}
		else
			GetDigitanksPlayer()->AppendTurnInfo(sprintf(tstring("Constructing ") + GetEntityName() + " (%d turns left)", m_iTurnsToConstruct.Get()));
	}

	if (IsUpgrading())
	{
		m_iTurnsToUpgrade--;

		if (m_iTurnsToUpgrade == (size_t)0)
		{
			GetDigitanksPlayer()->AppendTurnInfo(GetEntityName() + " finished upgrading.");

			UpgradeComplete();
		}
		else
			GetDigitanksPlayer()->AppendTurnInfo(sprintf(tstring("Upgrading ") + GetEntityName() + " (%d turns left)", GetTurnsToUpgrade()));
	}
}

void CStructure::FindGround()
{
	float flHeight = DigitanksGame()->GetTerrain()->GetHeight(GetGlobalOrigin().x, GetGlobalOrigin().y);
	float flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetGlobalOrigin().x + GetBoundingRadius(), GetGlobalOrigin().y + GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetGlobalOrigin().x + GetBoundingRadius(), GetGlobalOrigin().y - GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetGlobalOrigin().x - GetBoundingRadius(), GetGlobalOrigin().y + GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetGlobalOrigin().x - GetBoundingRadius(), GetGlobalOrigin().y - GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	SetGlobalOrigin(Vector(GetGlobalOrigin().x, GetGlobalOrigin().y, flHeight + GetBoundingRadius()/2 + 2));
}

void CStructure::PostRender() const
{
	BaseClass::PostRender();

	if (GameServer()->GetRenderer()->IsRenderingTransparent() && (m_flConstructionStartTime > 0) && GetVisibility() > 0)
	{
		CGameRenderingContext c(GameServer()->GetRenderer());
		c.Translate(GetGlobalOrigin());
		c.Scale(m_flScaffoldingSize, m_flScaffoldingSize, m_flScaffoldingSize);
		c.SetBlend(BLEND_ADDITIVE);
		c.SetAlpha(GetVisibility() * 0.2f * RemapValClamped((float)(GameServer()->GetGameTime() - m_flConstructionStartTime), 0.0f, 3.0f, 0.0f, 1.0f));
		c.SetDepthMask(false);
		c.SetBackCulling(false);
		c.RenderModel(m_iScaffolding);
	}
}

float CStructure::AvailableArea(int iArea) const
{
	return GetBoundingRadius();
}

bool CStructure::IsAvailableAreaActive(int iArea) const
{
	if (iArea != 0)
		return BaseClass::IsAvailableAreaActive(iArea);

	if (!GetDigitanksPlayer())
		return false;

	if (!GetDigitanksPlayer()->GetPrimaryCPU())
		return false;

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() == GetDigitanksPlayer())
		return false;

	if (DigitanksGame()->GetControlMode() != MODE_AIM)
		return false;

	CDigitank* pTank = DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetPrimarySelectionTank();

	if (!pTank)
		return false;

	if (!pTank->IsInsideMaxRange(GetGlobalOrigin()))
		return false;

	if (GetVisibility(pTank->GetDigitanksPlayer()) < 0.1f)
		return false;

	if (pTank->FiringCone() < 360 && fabs(AngleDifference(pTank->GetAngles().y, VectorAngles((GetGlobalOrigin()-pTank->GetGlobalOrigin()).Normalized()).y)) > pTank->FiringCone())
		return false;

	if (pTank->GetCurrentWeapon() == PROJECTILE_TREECUTTER)
		return false;

	return true;
}

void CStructure::DrawSchema(int x, int y, int w, int h)
{
	CRenderingContext c(GameServer()->GetRenderer());
	c.SetBlend(BLEND_ALPHA);
	c.SetColor(Color(255, 255, 255));

	float flYPosition = (float)y + h;
	float flXPosition = (float)x + w + 20;

	int iIconFontSize = 11;
	tstring sFont = "text";
	float flIconFontHeight = glgui::CLabel::GetFontHeight(sFont, iIconFontSize) + 2;

	if (IsConstructing())
	{
		tstring sTurns = sprintf(tstring("Turns To Construct: %d"), GetTurnsRemainingToConstruct());
		float flWidth = glgui::CLabel::GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);
		glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, (float)y);
		return;
	}

	if (IsUpgrading())
	{
		tstring sTurns = sprintf(tstring("Turns To Upgrade: %d"), GetTurnsRemainingToUpgrade());
		float flWidth = glgui::CLabel::GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);
		glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, (float)y);
		return;
	}

	if (GetUnitType() == STRUCTURE_CPU)
	{
		CCPU* pCPU = static_cast<CCPU*>(this);
		if (pCPU->IsProducing())
		{
			tstring sTurns = "Rogue: 1";	// It only ever takes one turn to make a rogue.
			float flWidth = glgui::CLabel::GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);
			glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, (float)y);
		}
	}
	else if (GetUnitType() == STRUCTURE_TANKLOADER || GetUnitType() == STRUCTURE_INFANTRYLOADER || GetUnitType() == STRUCTURE_ARTILLERYLOADER)
	{
		CLoader* pLoader = static_cast<CLoader*>(this);
		if (pLoader->IsProducing())
		{
			tstring sUnit;
			if (pLoader->GetBuildUnit() == UNIT_INFANTRY)
				sUnit = "Resistor";
			else if (pLoader->GetBuildUnit() == UNIT_ARTILLERY)
				sUnit = "Artillery";
			else
				sUnit = "Digitank";

			tstring sTurns = sprintf(sUnit + ": %d", pLoader->GetTurnsRemainingToProduce());
			float flWidth = glgui::CLabel::GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);
			glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, (float)y);
		}
	}

	if (Bandwidth() > 0)
	{
		tstring sBandwidth = sprintf(tstring(": %.1f"), Bandwidth());
		float flWidth = glgui::CLabel::GetTextWidth(sBandwidth, sBandwidth.length(), sFont, iIconFontSize);

		DigitanksWindow()->GetHUD()->PaintHUDSheet("BandwidthIcon", flXPosition - flWidth - flIconFontHeight, flYPosition - flIconFontHeight + 5, flIconFontHeight, flIconFontHeight);

		glgui::CLabel::PaintText(sBandwidth, sBandwidth.length(), sFont, iIconFontSize, flXPosition - flWidth, flYPosition);

		flYPosition -= flIconFontHeight;
	}

	if (FleetPoints() > 0)
	{
		tstring sFleetPoints = sprintf(tstring(": %d"), FleetPoints());
		float flWidth = glgui::CLabel::GetTextWidth(sFleetPoints, sFleetPoints.length(), sFont, iIconFontSize);

		DigitanksWindow()->GetHUD()->PaintHUDSheet("FleetPointsIcon", flXPosition - flWidth - flIconFontHeight, flYPosition - flIconFontHeight + 5, flIconFontHeight, flIconFontHeight);

		glgui::CLabel::PaintText(sFleetPoints, sFleetPoints.length(), sFont, iIconFontSize, flXPosition - flWidth, flYPosition);

		flYPosition -= flIconFontHeight;
	}

	if (Power() > 0)
	{
		tstring sPower = sprintf(tstring(": %.1f"), Power());
		float flWidth = glgui::CLabel::GetTextWidth(sPower, sPower.length(), sFont, iIconFontSize);

		DigitanksWindow()->GetHUD()->PaintHUDSheet("PowerIcon", (flXPosition - flWidth - flIconFontHeight), (flYPosition - flIconFontHeight) + 5, flIconFontHeight, flIconFontHeight);

		glgui::CLabel::PaintText(sPower, sPower.length(), sFont, iIconFontSize, flXPosition - flWidth, flYPosition);

		flYPosition -= flIconFontHeight;
	}
}

void CStructure::BeginConstruction(Vector vecConstructionOrigin)
{
	m_iTurnsToConstruct = GetTurnsToConstruct(vecConstructionOrigin);

	m_bConstructing = true;

	FindGround();

	if (GetModel())
	{
		Vector vecScaffoldingSize = GetModel()->m_aabbVisBoundingBox.Size();
		m_flScaffoldingSize = vecScaffoldingSize.Length()/2;
	}

	GameNetwork()->CallFunction(NETWORK_TOCLIENTS, "BeginStructureConstruction", GetHandle());

	BeginStructureConstruction(NULL);
}

void CStructure::BeginStructureConstruction(CNetworkParameters* p)
{
	m_flConstructionStartTime = GameServer()->GetGameTime();
}

void CStructure::CompleteConstruction()
{
	m_bConstructing = false;

	if (DigitanksGame()->GetUpdateGrid())
	{
		for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
		{
			for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
			{
				if (GetDigitanksPlayer()->HasDownloadedUpdate(x, y))
					DownloadComplete(x, y);
			}
		}
	}
}

size_t CStructure::GetTurnsToConstruct(Vector vecSpot)
{
	size_t iTurns = InitialTurnsToConstruct();

	// Location location location!
	if (DigitanksGame()->GetTerrain()->IsPointInTrees(vecSpot))
	{
		iTurns = (size_t)(iTurns*1.5f);
		if (iTurns < 1)
			iTurns = 1;
	}
	else if (DigitanksGame()->GetTerrain()->IsPointOverWater(vecSpot))
	{
		iTurns = (size_t)(iTurns*2.0f);
		if (iTurns < 2)
			iTurns = 2;
	}
	else if (DigitanksGame()->GetTerrain()->IsPointOverLava(vecSpot))
	{
		iTurns = (size_t)(iTurns*2.5f);
		if (iTurns < 2)
			iTurns = 2;
	}

	return iTurns;
}

void CStructure::InstallUpdate(size_t x, size_t y)
{
	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = x;
	p.ui3 = y;

	if (GameNetwork()->IsHost())
		InstallUpdate(&p);
	else
		GameNetwork()->CallFunctionParameters(NETWORK_TOSERVER, "InstallUpdate", &p);
}

void CStructure::InstallUpdate(CNetworkParameters* p)
{
	size_t x = p->ui2;
	size_t y = p->ui3;

	CUpdateItem* pUpdate = &DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y];

	if (pUpdate->m_eStructure != GetUnitType())
		return;

	switch (pUpdate->m_eUpdateType)
	{
	case UPDATETYPE_BANDWIDTH:
		m_flBandwidth += pUpdate->m_flValue;
		break;

	case UPDATETYPE_PRODUCTION:
		m_flPowerProduced += pUpdate->m_flValue;
		break;

	case UPDATETYPE_FLEETSUPPLY:
		m_iFleetSupply += (size_t)pUpdate->m_flValue;
		break;

	case UPDATETYPE_SUPPORTENERGY:
		m_iEnergyBonus += (size_t)pUpdate->m_flValue;
		break;

	case UPDATETYPE_SUPPORTRECHARGE:
		m_flRechargeBonus += (float)pUpdate->m_flValue;
		break;
	}
}

void CStructure::DownloadComplete(size_t x, size_t y)
{
	CUpdateItem* pItem = &DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y];

	if (pItem->m_eStructure != GetUnitType())
		return;

	if (pItem->m_eUpdateClass != UPDATECLASS_STRUCTUREUPDATE)
		return;

	InstallUpdate(x, y);
}

void CStructure::BeginUpgrade()
{
	if (!CanStructureUpgrade())
		return;

	if (GetDigitanksPlayer()->GetPower() < UpgradeCost())
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();

	if (GameNetwork()->IsHost())
		BeginUpgrade(&p);
	else
		GameNetwork()->CallFunctionParameters(NETWORK_TOSERVER, "BeginUpgrade", &p);
}

void CStructure::BeginUpgrade(CNetworkParameters* p)
{
	if (GetDigitanksPlayer()->GetPower() < UpgradeCost())
		return;

	m_bUpgrading = true;

	m_iTurnsToUpgrade = GetTurnsToUpgrade();
	GetDigitanksPlayer()->ConsumePower(UpgradeCost());

	GetDigitanksPlayer()->CountProducers();
}

size_t CStructure::GetTurnsToUpgrade()
{
	size_t iTurns = 1;

	if (GetUpgradeType() == STRUCTURE_PSU)
		iTurns = 2;

	if (GetUpgradeType() == STRUCTURE_BUFFER)
		iTurns = 1;

	// Location location location!
	if (DigitanksGame()->GetTerrain()->IsPointInTrees(GetGlobalOrigin()))
		iTurns = (size_t)(iTurns*1.5f);
	else if (DigitanksGame()->GetTerrain()->IsPointOverWater(GetGlobalOrigin()))
		iTurns = (size_t)(iTurns*2.0f);
	else if (DigitanksGame()->GetTerrain()->IsPointOverLava(GetGlobalOrigin()))
		iTurns = (size_t)(iTurns*2.5f);

	return iTurns;
}

bool CStructure::NeedsOrders()
{
	if (IsUpgrading() || IsConstructing())
		return false;

	if (CanStructureUpgrade())
		return true;

	return BaseClass::NeedsOrders();
}

void CStructure::SetSupplier(const class CSupplier* pSupplier)
{
	if (!GameNetwork()->IsHost())
		return;

	m_hSupplier = pSupplier;

	if (!m_hSupplyLine && m_hSupplier != NULL)
		m_hSupplyLine = GameServer()->Create<CSupplyLine>("CSupplyLine");

	if (m_hSupplyLine != NULL && !m_hSupplier)
		GameServer()->Delete(m_hSupplyLine);

	if (m_hSupplyLine != NULL && m_hSupplier != NULL)
		m_hSupplyLine->SetEntities(m_hSupplier, this);
}

CSupplier* CStructure::GetSupplier() const
{
	if (!m_hSupplier)
		return NULL;

	return m_hSupplier;
}

CSupplyLine* CStructure::GetSupplyLine() const
{
	if (!m_hSupplyLine)
		return NULL;

	return m_hSupplyLine;
}

void CStructure::ModifyContext(class CRenderingContext* pContext) const
{
	BaseClass::ModifyContext(pContext);

	if (IsConstructing() || IsUpgrading())
	{
		pContext->SetBlend(BLEND_ALPHA);
		pContext->SetColor(Color(255, 255, 255));
		pContext->SetAlpha(GetVisibility() * RemapValClamped((float)(GameServer()->GetGameTime() - m_flConstructionStartTime), 0.0f, 2.0f, 0.0f, 1.0f));
		pContext->Translate(Vector(0, 0, RemapValClamped((float)(GameServer()->GetGameTime() - m_flConstructionStartTime), 0.0f, 3.0f, -3.0f, 0.0f)));
	}

	if (GetPlayerOwner())
	{
		pContext->SetUniform("bColorSwapInAlpha", true);
		pContext->SetUniform("vecColorSwap", GetPlayerOwner()->GetColor());
	}
}

void CStructure::OnDeleted()
{
	BaseClass::OnDeleted();

	SetSupplier(NULL);
}

void CStructure::ClientUpdate(int iClient)
{
	BaseClass::ClientUpdate(iClient);
}

void CStructure::OnSerialize(std::ostream& o)
{
	BaseClass::OnSerialize(o);
}

bool CStructure::OnUnserialize(std::istream& i)
{
	return BaseClass::OnUnserialize(i);
}

float CStructure::VisibleRange() const
{
	if (IsConstructing())
		return GetBoundingRadius()*2;

	return BaseClass::VisibleRange();
}

float CStructure::ConstructionCost() const
{
	return DigitanksGame()->GetConstructionCost(GetUnitType());
}

float CStructure::UpgradeCost() const
{
	float flPowerToUpgrade = DigitanksGame()->GetConstructionCost(GetUpgradeType());

	return flPowerToUpgrade;
}

CMaterialHandle CSupplier::s_hTendrilBeam = 0;

REGISTER_ENTITY(CSupplier);

NETVAR_TABLE_BEGIN(CSupplier);
	NETVAR_DEFINE(size_t, m_iDataStrength);
	NETVAR_DEFINE(float, m_flBonusDataFlow);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CSupplier);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iDataStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBonusDataFlow);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CTendril, m_aTendrils);	// Generated
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CStructure>, m_ahChildren);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, size_t, m_iTendrilsCallList);	// Generated
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, double, m_flTendrilGrowthStartTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bShouldRender);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CSupplier);
INPUTS_TABLE_END();

void CSupplier::Precache()
{
	BaseClass::Precache();

	s_hTendrilBeam = CMaterialLibrary::AddMaterial("textures/tendril.mat");
}

void CSupplier::Spawn()
{
	BaseClass::Spawn();

	m_iDataStrength = InitialDataStrength();
	m_flBonusDataFlow = 0;

	m_iTendrilsCallList = 0;

	m_flTendrilGrowthStartTime = 0;

	m_bShouldRender = true;
}

void CSupplier::ClientSpawn()
{
	BaseClass::ClientSpawn();

	UpdateTendrils();
}

#define GROWTH_TIME 3

void CSupplier::Think()
{
	BaseClass::Think();

	float flGrowthTime = (float)(GameServer()->GetGameTime() - m_flTendrilGrowthStartTime);
	if (flGrowthTime >= GROWTH_TIME)
		m_flTendrilGrowthStartTime = 0;
}

void CSupplier::ClientEnterGame()
{
	BaseClass::ClientEnterGame();

	if (HasIssuedClientSpawn())
		UpdateTendrils();
}

float CSupplier::GetDataFlowRate()
{
	return BaseDataFlowPerTurn() + m_flBonusDataFlow;
}

float CSupplier::GetDataFlowRadius() const
{
	// Opposite of formula for area of a circle.
	return sqrt((float)m_iDataStrength.Get()/M_PI) + GetBoundingRadius();
}

float CSupplier::GetDataFlow(Vector vecPoint) const
{
	return RemapValClamped((vecPoint - GetGlobalOrigin()).Length(), GetBoundingRadius(), GetDataFlowRadius()+GetBoundingRadius(), (float)m_iDataStrength, 0);
}

float CSupplier::GetDataFlow(Vector vecPoint, const CDigitanksPlayer* pPlayer, const CSupplier* pIgnore)
{
	if (!pPlayer)
		return 0;

	float flDataStrength = 0;
	size_t iNumMembers = pPlayer->GetNumUnits();
	for (size_t i = 0; i < iNumMembers; i++)
	{
		const CBaseEntity* pEntity = pPlayer->GetUnit(i);
		if (!pEntity)
			continue;

		const CSupplier* pSupplier = dynamic_cast<const CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (pSupplier == pIgnore)
			continue;

		flDataStrength += pSupplier->GetDataFlow(vecPoint);
	}

	return flDataStrength;
}

void CSupplier::CalculateDataFlow()
{
	if (IsConstructing())
		return;

	if (m_hSupplier != NULL)
	{
		// Use the radius of a circle with the area of the given data flow
		// so the flow doesn't get huge when you're close to a source.
		m_flBonusDataFlow = sqrt(m_hSupplier->GetDataFlow(GetGlobalOrigin())/M_PI);
	}
	else
		m_flBonusDataFlow = 0;

	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		CSupplier* pSupplier = dynamic_cast<CSupplier*>(m_ahChildren[i].GetPointer());
		if (pSupplier)
			pSupplier->CalculateDataFlow();
	}
}

float CSupplier::GetChildEfficiency() const
{
	size_t iEfficientChildren = EfficientChildren();

	if (iEfficientChildren == 0)
		return 1;

	size_t iConsumingChildren = 0;
	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		if (!m_ahChildren[i])
			continue;

		if (!dynamic_cast<CStructure*>(m_ahChildren[i].GetPointer()))
			continue;

		if (dynamic_cast<CSupplier*>(m_ahChildren[i].GetPointer()))
			continue;

		iConsumingChildren++;
	}

	if (iConsumingChildren <= 2)
		return 1;

	// 3 == 0.5, 4 == 0.25, 5 == .2, and so on.
	return 1/((float)iConsumingChildren-1);
}

void CSupplier::OnTeamChange()
{
	if (!GetPlayerOwner())
	{
		for (size_t i = 0; i < m_ahChildren.size(); i++)
		{
			if (!m_ahChildren[i])
				continue;

			m_ahChildren[i]->SetSupplier(NULL);

			CStructure* pStructure = dynamic_cast<CStructure*>(m_ahChildren[i].GetPointer());
			if (!pStructure)
				continue;

			if (pStructure->GetPlayerOwner())
			{
				pStructure->GetPlayerOwner()->RemoveUnit(pStructure);
				DigitanksGame()->OnDisabled(pStructure, NULL, NULL);
			}
		}
	}

	BaseClass::OnTeamChange();
	UpdateTendrils();

	// This happens in UpdateTendrils() but do it again here anyway because UpdateTendrils only does it if there's a new tendril created.
	DigitanksGame()->GetTerrain()->DirtyChunkTexturesWithinDistance(GetGlobalOrigin(), GetDataFlowRadius() + GetBoundingRadius());
}

void CSupplier::StartTurn()
{
	BaseClass::StartTurn();

	if (!IsConstructing())
	{
		if (IsDataFlowSource())
			m_iDataStrength += (size_t)GetDataFlowRate();
		else if (GetSupplyLine())
			m_iDataStrength += (size_t)(GetDataFlowRate() * GetSupplyLine()->GetIntegrity());
	}

	// Figure out whether we're too far to bother rendering.
	CDigitanksPlayer* pLocalPlayer = DigitanksGame()->GetCurrentLocalDigitanksPlayer();
	if (!DigitanksGame()->ShouldRenderFogOfWar() || pLocalPlayer == GetDigitanksPlayer())
		m_bShouldRender = true;
	else if (!GetPlayerOwner())
		// Let regular culling happen
		m_bShouldRender = true;
	else if (pLocalPlayer)
	{
		float flVisibleDistance = GetDataFlowRadius();

		m_bShouldRender = false;
		for (size_t i = 0; i < pLocalPlayer->GetNumUnits(); i++)
		{
			const CBaseEntity* pEntity = pLocalPlayer->GetUnit(i);
			if (!pEntity)
				continue;

			const CDigitank* pDigitank = dynamic_cast<const CDigitank*>(pEntity);
			if (pDigitank)
			{
				float flDistanceSqr = pDigitank->GetGlobalOrigin().DistanceSqr(GetGlobalOrigin());

				// Look at the maximum that the tank could ever go in his next turn.
				float flTankVisibleDistance = pDigitank->GetMaxMovementEnergy() * pDigitank->GetTankSpeed() + pDigitank->VisibleRange();

				float flTotalVisibleDistance = flTankVisibleDistance + flVisibleDistance;

				if (flDistanceSqr < flTotalVisibleDistance*flTotalVisibleDistance)
				{
					m_bShouldRender = true;
					break;
				}

				continue;
			}

			const CDigitanksEntity* pDTEnt = dynamic_cast<const CDigitanksEntity*>(pEntity);
			if (pDTEnt)
			{
				float flDistanceSqr = pDTEnt->GetGlobalOrigin().DistanceSqr(GetGlobalOrigin());

				float flTotalVisibleDistance = pDTEnt->VisibleRange() + flVisibleDistance;

				if (flDistanceSqr < flTotalVisibleDistance*flTotalVisibleDistance)
				{
					m_bShouldRender = true;
					break;
				}
			}
		}
	}

	if (GetDigitanksPlayer() && !IsDataFlowSource())
	{
		if (!GetSupplyLine() || GetSupplyLine()->GetIntegrity() <= CSupplyLine::MinimumIntegrity())
		{
			GetDigitanksPlayer()->RemoveUnit(this);
			return;
		}
	}

	UpdateTendrils();

	// This can happen if it was removed this same turn by lack of supplier.
	if (!GetDigitanksPlayer())
		return;

	if (IsConstructing())
		return;

	if (GameNetwork()->IsHost() && GetSupplyLine() && GetSupplyLine()->GetIntegrity() >= 0.7f || IsDataFlowSource())
	{
		for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
			if (!pEntity)
				continue;

			CDigitanksEntity* pDTEnt = dynamic_cast<CDigitanksEntity*>(pEntity);
			if (pDTEnt && pDTEnt->IsImprisoned())
			{
				float flFlow = GetDataFlow(pDTEnt->GetGlobalOrigin());
				if (flFlow > 0)
					pDTEnt->Rescue(this);
			}

			CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);
			if (!pStructure)
				continue;

			// Just in case!
			if (this == pStructure)
				continue;

			if (dynamic_cast<CResourceNode*>(pStructure))
				continue;

			if (pStructure->GetPlayerOwner())
				continue;

			float flFlow = GetDataFlow(pStructure->GetGlobalOrigin());

			if (flFlow < 10)
				continue;

			// You will join us... OR DIE
			GetDigitanksPlayer()->AddUnit(pStructure);
			pStructure->SetSupplier(this);
			pStructure->GetSupplyLine()->SetIntegrity(0.5f);

			pStructure->DirtyArea();
		}
	}
}

void CSupplier::CompleteConstruction()
{
	m_flTendrilGrowthStartTime = GameServer()->GetGameTime();

	BaseClass::CompleteConstruction();
}

CVar tendril_fade_distance("perf_tendril_fade_distance", "200");

void CSupplier::PostRender() const
{
	BaseClass::PostRender();

	if (!GameServer()->GetRenderer()->IsRenderingTransparent())
		return;

	float flGrowthTime = (float)(GameServer()->GetGameTime() - m_flTendrilGrowthStartTime);

	if (flGrowthTime < 0)
		return;

	if (m_flTendrilGrowthStartTime == 0)
	{
		DigitanksGame()->GetDigitanksRenderer()->AddTendrilBatch(this);
		return;
	}

	CDigitanksCamera* pCamera = DigitanksGame()->GetDigitanksCamera();
	Vector vecCamera = pCamera->GetCameraPosition();
	float flDistanceSqr = GetGlobalOrigin().DistanceSqr(vecCamera);
	float flFadeDistance = tendril_fade_distance.GetFloat();

	float flFadeAlpha = RemapValClamped(flDistanceSqr, flFadeDistance*flFadeDistance, (flFadeDistance+20)*(flFadeDistance+20), 1.0f, 0.0f);

	if (flFadeAlpha <= 0)
		return;

	float flTreeAlpha = 1.0f;
	if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(GetGlobalOrigin().x), CTerrain::WorldToArraySpace(GetGlobalOrigin().y), TB_TREE))
		flTreeAlpha = 0.3f;

	CRenderingContext r(GameServer()->GetRenderer());
	if (DigitanksGame()->ShouldRenderFogOfWar())
		r.UseFrameBuffer(DigitanksGame()->GetDigitanksRenderer()->GetVisibilityMaskedBuffer());
	r.SetDepthMask(false);
	r.UseMaterial(s_hTendrilBeam);

	r.SetUniform("flTime", (float)GameServer()->GetGameTime());
	r.SetUniform("iTexture", 0);
	r.SetUniform("flAlpha", flFadeAlpha * flTreeAlpha);

	Color clrTeam = GetPlayerOwner()?GetPlayerOwner()->GetColor():Color(255,255,255,255);
	clrTeam = (Vector(clrTeam) + Vector(1,1,1))/2;

	if (m_flTendrilGrowthStartTime > 0)
	{
		float flTotalSize = (float)m_aTendrils.size() + GetBoundingRadius();

		for (size_t i = 0; i < m_aTendrils.size(); i++)
		{
			if (i < m_aTendrils.size() - 15)
				continue;

			const CTendril* pTendril = &m_aTendrils[i];

			float flGrowthLength = RemapVal(flGrowthTime, 0, GROWTH_TIME, pTendril->m_flLength-flTotalSize, pTendril->m_flLength);

			if (flGrowthLength < 0)
				continue;

			Vector vecDestination = GetGlobalOrigin() + (pTendril->m_vecEndPoint - GetGlobalOrigin()).Normalized() * flGrowthLength;

			Vector vecPath = vecDestination - GetGlobalOrigin();
			vecPath.y = 0;

			float flDistance = vecPath.Length2D();
			Vector vecDirection = vecPath.Normalized();
			size_t iSegments = (size_t)(flDistance/3);

			r.SetUniform("flSpeed", pTendril->m_flSpeed);

			clrTeam.SetAlpha(105);

			CRopeRenderer oRope(GameServer()->GetRenderer(), s_hTendrilBeam, DigitanksGame()->GetTerrain()->GetPointHeight(GetGlobalOrigin()) + Vector(0, 0, 1), 1.0f);
			oRope.SetColor(clrTeam);
			oRope.SetTextureScale(pTendril->m_flScale);
			oRope.SetTextureOffset(pTendril->m_flOffset);
			oRope.SetForward(Vector(0, 0, -1));

			for (size_t i = 1; i < iSegments; i++)
			{
				clrTeam.SetAlpha((int)RemapVal((float)i, 1, (float)iSegments, 100, 30));
				oRope.SetColor(clrTeam);

				float flCurrentDistance = ((float)i*flDistance)/iSegments;
				oRope.AddLink(DigitanksGame()->GetTerrain()->GetPointHeight(GetGlobalOrigin() + vecDirection*flCurrentDistance) + Vector(0, 0, 1));
			}

			oRope.Finish(DigitanksGame()->GetTerrain()->GetPointHeight(vecDestination) + Vector(0, 0, 1));
		}
	}
}

void CSupplier::UpdateTendrils()
{
	if (IsConstructing())
		return;

#if 0
	if (!GetPlayerOwner())
	{
		if (m_iTendrilsCallList)
			glDeleteLists((GLuint)m_iTendrilsCallList, 1);

		m_iTendrilsCallList = 0;

		DigitanksGame()->GetTerrain()->DirtyChunkTexturesWithinDistance(GetGlobalOrigin(), GetDataFlowRadius() + GetBoundingRadius());
		return;
	}

	if (GameServer()->IsLoading())
		return;

	bool bUpdateTerrain = false;
	size_t iRadius = (size_t)GetDataFlowRadius();
	while (m_aTendrils.size() < iRadius)
	{
		m_aTendrils.push_back(CTendril());
		CTendril* pTendril = &m_aTendrils[m_aTendrils.size()-1];
		pTendril->m_flLength = (float)m_aTendrils.size() + GetBoundingRadius();
		pTendril->m_vecEndPoint = DigitanksGame()->GetTerrain()->GetPointHeight(GetGlobalOrigin() + AngleVector(EAngle(0, RandomFloat(0, 360), 0)) * pTendril->m_flLength);
		pTendril->m_flScale = RandomFloat(3, 7);
		pTendril->m_flOffset = RandomFloat(0, 1);
		pTendril->m_flSpeed = RandomFloat(0.5f, 2);

		bUpdateTerrain = true;
	}

	if (bUpdateTerrain)
		DigitanksGame()->GetTerrain()->DirtyChunkTexturesWithinDistance(GetGlobalOrigin(), GetDataFlowRadius() + GetBoundingRadius());

	if (m_iTendrilsCallList)
		glDeleteLists((GLuint)m_iTendrilsCallList, 1);

	m_iTendrilsCallList = glGenLists(1);

	Color clrTeam = GetPlayerOwner()->GetColor();
	clrTeam = (Vector(clrTeam) + Vector(1,1,1))/2;

	glNewList((GLuint)m_iTendrilsCallList, GL_COMPILE);
	for (size_t i = 0; i < m_aTendrils.size(); i++)
	{
		// Only show the longest tendrils, for perf reasons.
		if (i < iRadius - 15)
			continue;

		CTendril* pTendril = &m_aTendrils[i];

		Vector vecDestination = pTendril->m_vecEndPoint;

		Vector vecPath = vecDestination - GetGlobalOrigin();
		vecPath.y = 0;

		float flDistance = vecPath.Length2D();
		Vector vecDirection = vecPath.Normalized();
		size_t iSegments = (size_t)(flDistance/3);

		GLuint iScrollingTextureProgram = (GLuint)CShaderLibrary::GetScrollingTextureProgram();

		GLuint flSpeed = glGetUniformLocation(iScrollingTextureProgram, "flSpeed");
		glUniform1f(flSpeed, pTendril->m_flSpeed);

		clrTeam.SetAlpha(105);

		CRopeRenderer oRope(GameServer()->GetRenderer(), s_iTendrilBeam, DigitanksGame()->GetTerrain()->GetPointHeight(GetGlobalOrigin()) + Vector(0, 0, 1), 1.0f);
		oRope.SetColor(clrTeam);
		oRope.SetTextureScale(pTendril->m_flScale);
		oRope.SetTextureOffset(pTendril->m_flOffset);
		oRope.SetForward(Vector(0, 0, -1));

		for (size_t i = 1; i < iSegments; i++)
		{
			clrTeam.SetAlpha((int)RemapVal((float)i, 1, (float)iSegments, 100, 30));
			oRope.SetColor(clrTeam);

			float flCurrentDistance = ((float)i*flDistance)/iSegments;
			oRope.AddLink(DigitanksGame()->GetTerrain()->GetPointHeight(GetGlobalOrigin() + vecDirection*flCurrentDistance) + Vector(0, 0, 1));
		}

		oRope.Finish(DigitanksGame()->GetTerrain()->GetPointHeight(vecDestination) + Vector(0, 0, 1));
	}
	glEndList();
#endif
}

void CSupplier::BeginTendrilGrowth()
{
	m_flTendrilGrowthStartTime = GameServer()->GetGameTime();
}

void CSupplier::AddChild(CStructure* pChild)
{
	if (!GameNetwork()->IsHost())
		return;

	if (!pChild)
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = pChild->GetHandle();

	AddChild(&p);

	GameNetwork()->CallFunctionParameters(NETWORK_TOCLIENTS, "AddChild", &p);
}

void CSupplier::AddChild(CNetworkParameters* p)
{
	CEntityHandle<CStructure> hChild(p->ui2);

	// Just in case of dupes.
	RemoveChild(p);

	m_ahChildren.push_back(hChild.GetPointer());
}

void CSupplier::RemoveChild(CStructure* pChild)
{
	if (!GameNetwork()->IsHost())
		return;

	if (!pChild)
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = pChild->GetHandle();

	AddChild(&p);

	GameNetwork()->CallFunctionParameters(NETWORK_TOCLIENTS, "RemoveChild", &p);
}

void CSupplier::RemoveChild(CNetworkParameters* p)
{
	CEntityHandle<CStructure> hChild(p->ui2);

	if (!hChild)
		return;

	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		if (!m_ahChildren[i])
			continue;

		if (m_ahChildren[i] == hChild)
			m_ahChildren.erase(m_ahChildren.begin()+i);
		// Don't return as soon as we find it in case of dupes.
	}
}

void CSupplier::OnDeleted(CBaseEntity* pEntity)
{
	BaseClass::OnDeleted(pEntity);

	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		if (m_ahChildren[i] == pEntity)
			m_ahChildren.erase(m_ahChildren.begin()+i);
	}
}

CSupplier* CSupplier::FindClosestSupplier(CBaseEntity* pUnit)
{
	CSupplier* pClosest = NULL;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		if (pEntity == pUnit)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (pSupplier->GetPlayerOwner() != pUnit->GetOwner())
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (!pClosest)
		{
			pClosest = pSupplier;
			continue;
		}

		if ((pSupplier->GetGlobalOrigin() - pUnit->GetGlobalOrigin()).Length() < (pClosest->GetGlobalOrigin() - pUnit->GetGlobalOrigin()).Length())
			pClosest = pSupplier;
	}

	return pClosest;
}

CSupplier* CSupplier::FindClosestSupplier(Vector vecPoint, const CDigitanksPlayer* pTeam)
{
	CSupplier* pClosest = NULL;
	CSupplier* pClosestInNetwork = NULL;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (pSupplier->GetPlayerOwner() != pTeam)
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (pSupplier->GetDataFlow(vecPoint) > 0)
		{
			if (!pClosestInNetwork || (pSupplier->GetGlobalOrigin() - vecPoint).Length() < (pClosestInNetwork->GetGlobalOrigin() - vecPoint).Length())
				pClosestInNetwork = pSupplier;
		}

		if (!pClosest)
		{
			pClosest = pSupplier;
			continue;
		}

		if ((pSupplier->GetGlobalOrigin() - vecPoint).Length() < (pClosest->GetGlobalOrigin() - vecPoint).Length())
			pClosest = pSupplier;
	}

	if (pClosestInNetwork)
		return pClosestInNetwork;

	return pClosest;
}

float CSupplier::BaseVisibleRange() const
{
	return GetDataFlowRadius() + 15;
}

float CSupplier::AvailableArea(int iArea) const
{
	if (iArea == 1)
		return GetDataFlowRadius() + GetBoundingRadius();

	return BaseClass::AvailableArea(iArea);
}

bool CSupplier::IsAvailableAreaActive(int iArea) const
{
	if (iArea != 1)
		return BaseClass::IsAvailableAreaActive(iArea);

	if (!GetDigitanksPlayer())
		return false;

	if (!GetDigitanksPlayer()->GetPrimaryCPU())
		return false;

	unittype_t ePreviewStructure = GetDigitanksPlayer()->GetPrimaryCPU()->GetPreviewStructure();
	if (ePreviewStructure == STRUCTURE_PSU || ePreviewStructure == STRUCTURE_BATTERY)
		return false;

	if (GetPlayerOwner() != DigitanksGame()->GetCurrentLocalDigitanksPlayer())
		return false;

	if (IsConstructing())
		return false;

	// In build mode show everybody, otherwise only show the selected structure.
	if (DigitanksGame()->GetControlMode() == MODE_BUILD || GetDigitanksPlayer()->IsSelected(this))
		return true;

	return false;
}
