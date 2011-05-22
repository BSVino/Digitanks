#include "structure.h"

#include <maths.h>
#include <mtrand.h>
#include <strutils.h>

#include <tinker/cvar.h>

#include <digitanks/dt_renderer.h>
#include <digitanks/digitanksgame.h>
#include <digitanks/supplyline.h>
#include <digitanks/dt_camera.h>
#include <ui/digitankswindow.h>
#include <ui/hud.h>
#include <ui/instructor.h>
#include <shaders/shaders.h>
#include <models/models.h>
#include <network/network.h>
#include <models/texturelibrary.h>

#include "collector.h"
#include "loader.h"

#include <GL/glew.h>

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

	SetCollisionGroup(CG_ENTITY);

	m_bConstructing = true;
}

void CStructure::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/scaffolding.obj");
}

void CStructure::Spawn()
{
	BaseClass::Spawn();

	m_iFleetSupply = InitialFleetPoints();
	m_flBandwidth = InitialBandwidth();
	m_flPowerProduced = InitialPower();
	m_iEnergyBonus = InitialEnergyBonus();
	m_flRechargeBonus = InitialRechargeBonus();

	m_iScaffolding = CModelLibrary::Get()->FindModel(L"models/structures/scaffolding.obj");
	m_flScaffoldingSize = 10;

	m_flConstructionStartTime = 0;

	m_bConstructing = true;
}

void CStructure::Think()
{
	BaseClass::Think();

	if (IsAlive())
		FindGround();
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
		if (GetTeam())
			GetTeam()->RemoveEntity(this);
		SetSupplier(NULL);
	}

	if (GetSupplier() && !GetSupplier()->GetTeam())
	{
		GetSupplier()->RemoveChild(this);
		if (GetTeam())
			GetTeam()->RemoveEntity(this);
		SetSupplier(NULL);
	}

	if (GetTeam() == NULL)
		return;

	if (IsConstructing())
	{
		m_iTurnsToConstruct--;

		if (m_iTurnsToConstruct == (size_t)0)
		{
			GetDigitanksTeam()->AppendTurnInfo(eastl::string16(L"Construction finished on ") + GetEntityName());
			CompleteConstruction();

			GetDigitanksTeam()->AddActionItem(this, ACTIONTYPE_NEWSTRUCTURE);
		}
		else
			GetDigitanksTeam()->AppendTurnInfo(sprintf(eastl::string16(L"Constructing ") + GetEntityName() + L" (%d turns left)", m_iTurnsToConstruct.Get()));
	}

	if (IsUpgrading())
	{
		m_iTurnsToUpgrade--;

		if (m_iTurnsToUpgrade == (size_t)0)
		{
			GetDigitanksTeam()->AppendTurnInfo(GetEntityName() + L" finished upgrading.");

			UpgradeComplete();
		}
		else
			GetDigitanksTeam()->AppendTurnInfo(sprintf(eastl::string16(L"Upgrading ") + GetEntityName() + L" (%d turns left)", GetTurnsToUpgrade()));
	}
}

void CStructure::FindGround()
{
	float flHeight = DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x, GetOrigin().z);
	float flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x + GetBoundingRadius(), GetOrigin().z + GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x + GetBoundingRadius(), GetOrigin().z - GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x - GetBoundingRadius(), GetOrigin().z + GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x - GetBoundingRadius(), GetOrigin().z - GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	SetOrigin(Vector(GetOrigin().x, flHeight + GetBoundingRadius()/2 + 2, GetOrigin().z));
}

void CStructure::PostRender(bool bTransparent)
{
	BaseClass::PostRender(bTransparent);

	if (bTransparent && (IsConstructing() || IsUpgrading()) && GetVisibility() > 0)
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.Translate(GetOrigin());
		c.Scale(m_flScaffoldingSize, m_flScaffoldingSize, m_flScaffoldingSize);
		c.SetBlend(BLEND_ADDITIVE);
		c.SetAlpha(GetVisibility() * 0.2f * RemapValClamped(GameServer()->GetGameTime() - m_flConstructionStartTime, 0, 3, 0, 1));
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

	if (!GetDigitanksTeam())
		return false;

	if (!GetDigitanksTeam()->GetPrimaryCPU())
		return false;

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() == GetDigitanksTeam())
		return false;

	if (DigitanksGame()->GetControlMode() != MODE_AIM)
		return false;

	CDigitank* pTank = DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetPrimarySelectionTank();

	if (!pTank)
		return false;

	if (!pTank->IsInsideMaxRange(GetOrigin()))
		return false;

	if (GetVisibility(pTank->GetDigitanksTeam()) < 0.1f)
		return false;

	if (pTank->FiringCone() < 360 && fabs(AngleDifference(pTank->GetAngles().y, VectorAngles((GetOrigin()-pTank->GetOrigin()).Normalized()).y)) > pTank->FiringCone())
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
	eastl::string16 sFont = L"text";
	float flIconFontHeight = glgui::CLabel::GetFontHeight(sFont, iIconFontSize) + 2;

	if (IsConstructing())
	{
		eastl::string16 sTurns = sprintf(L"Turns To Construct: %d", GetTurnsRemainingToConstruct());
		float flWidth = glgui::CLabel::GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);
		glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, (float)y);
		return;
	}

	if (IsUpgrading())
	{
		eastl::string16 sTurns = sprintf(L"Turns To Upgrade: %d", GetTurnsRemainingToUpgrade());
		float flWidth = glgui::CLabel::GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);
		glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, (float)y);
		return;
	}

	if (GetUnitType() == STRUCTURE_CPU)
	{
		CCPU* pCPU = static_cast<CCPU*>(this);
		if (pCPU->IsProducing())
		{
			eastl::string16 sTurns = L"Rogue: 1";	// It only ever takes one turn to make a rogue.
			float flWidth = glgui::CLabel::GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);
			glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, (float)y);
		}
	}
	else if (GetUnitType() == STRUCTURE_TANKLOADER || GetUnitType() == STRUCTURE_INFANTRYLOADER || GetUnitType() == STRUCTURE_ARTILLERYLOADER)
	{
		CLoader* pLoader = static_cast<CLoader*>(this);
		if (pLoader->IsProducing())
		{
			eastl::string16 sUnit;
			if (pLoader->GetBuildUnit() == UNIT_INFANTRY)
				sUnit = L"Resistor";
			else if (pLoader->GetBuildUnit() == UNIT_ARTILLERY)
				sUnit = L"Artillery";
			else
				sUnit = L"Digitank";

			eastl::string16 sTurns = sprintf(sUnit + L": %d", pLoader->GetTurnsRemainingToProduce());
			float flWidth = glgui::CLabel::GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);
			glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, (float)y);
		}
	}

	if (Bandwidth() > 0)
	{
		eastl::string16 sBandwidth = sprintf(L": %.1f", Bandwidth());
		float flWidth = glgui::CLabel::GetTextWidth(sBandwidth, sBandwidth.length(), sFont, iIconFontSize);

		DigitanksWindow()->GetHUD()->PaintHUDSheet("BandwidthIcon", (int)(flXPosition - flWidth - flIconFontHeight), (int)(flYPosition - flIconFontHeight) + 5, (int)flIconFontHeight, (int)flIconFontHeight);

		glgui::CLabel::PaintText(sBandwidth, sBandwidth.length(), sFont, iIconFontSize, flXPosition - flWidth, flYPosition);

		flYPosition -= flIconFontHeight;
	}

	if (FleetPoints() > 0)
	{
		eastl::string16 sFleetPoints = sprintf(L": %d", FleetPoints());
		float flWidth = glgui::CLabel::GetTextWidth(sFleetPoints, sFleetPoints.length(), sFont, iIconFontSize);

		DigitanksWindow()->GetHUD()->PaintHUDSheet("FleetPointsIcon", (int)(flXPosition - flWidth - flIconFontHeight), (int)(flYPosition - flIconFontHeight) + 5, (int)flIconFontHeight, (int)flIconFontHeight);

		glgui::CLabel::PaintText(sFleetPoints, sFleetPoints.length(), sFont, iIconFontSize, flXPosition - flWidth, flYPosition);

		flYPosition -= flIconFontHeight;
	}

	if (Power() > 0)
	{
		eastl::string16 sPower = sprintf(L": %.1f", Power());
		float flWidth = glgui::CLabel::GetTextWidth(sPower, sPower.length(), sFont, iIconFontSize);

		DigitanksWindow()->GetHUD()->PaintHUDSheet("PowerIcon", (int)(flXPosition - flWidth - flIconFontHeight), (int)(flYPosition - flIconFontHeight) + 5, (int)flIconFontHeight, (int)flIconFontHeight);

		glgui::CLabel::PaintText(sPower, sPower.length(), sFont, iIconFontSize, flXPosition - flWidth, flYPosition);

		flYPosition -= flIconFontHeight;
	}
}

void CStructure::BeginConstruction(Vector vecConstructionOrigin)
{
	m_iTurnsToConstruct = GetTurnsToConstruct(vecConstructionOrigin);

	m_bConstructing = true;

	FindGround();

	if (GetModel() != ~0)
	{
		Vector vecScaffoldingSize = CModelLibrary::Get()->GetModel(GetModel())->m_pScene->m_oExtends.Size();
		m_flScaffoldingSize = vecScaffoldingSize.Length()/2;
	}

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "BeginStructureConstruction", GetHandle());

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
				if (GetDigitanksTeam()->HasDownloadedUpdate(x, y))
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
		iTurns = (size_t)(iTurns*1.5f);
	else if (DigitanksGame()->GetTerrain()->IsPointOverWater(vecSpot))
		iTurns = (size_t)(iTurns*2.0f);
	else if (DigitanksGame()->GetTerrain()->IsPointOverLava(vecSpot))
		iTurns = (size_t)(iTurns*2.5f);

	return iTurns;
}

void CStructure::InstallUpdate(size_t x, size_t y)
{
	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = x;
	p.ui3 = y;

	if (CNetwork::IsHost())
		InstallUpdate(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "InstallUpdate", &p);
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

	if (GetDigitanksTeam()->GetPower() < UpgradeCost())
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();

	if (CNetwork::IsHost())
		BeginUpgrade(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "BeginUpgrade", &p);
}

void CStructure::BeginUpgrade(CNetworkParameters* p)
{
	if (GetDigitanksTeam()->GetPower() < UpgradeCost())
		return;

	m_bUpgrading = true;

	m_iTurnsToUpgrade = GetTurnsToUpgrade();
	GetDigitanksTeam()->ConsumePower(UpgradeCost());

	GetDigitanksTeam()->CountProducers();
}

size_t CStructure::GetTurnsToUpgrade()
{
	size_t iTurns = 1;

	if (GetUpgradeType() == STRUCTURE_PSU)
		iTurns = 2;

	if (GetUpgradeType() == STRUCTURE_BUFFER)
		iTurns = 1;

	// Location location location!
	if (DigitanksGame()->GetTerrain()->IsPointInTrees(GetOrigin()))
		iTurns = (size_t)(iTurns*1.5f);
	else if (DigitanksGame()->GetTerrain()->IsPointOverWater(GetOrigin()))
		iTurns = (size_t)(iTurns*2.0f);
	else if (DigitanksGame()->GetTerrain()->IsPointOverLava(GetOrigin()))
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

void CStructure::SetSupplier(class CSupplier* pSupplier)
{
	if (!CNetwork::IsHost())
		return;

	m_hSupplier = pSupplier;

	if (m_hSupplyLine == NULL && m_hSupplier != NULL)
		m_hSupplyLine = GameServer()->Create<CSupplyLine>("CSupplyLine");

	if (m_hSupplyLine != NULL && m_hSupplier == NULL)
		GameServer()->Delete(m_hSupplyLine);

	if (m_hSupplyLine != NULL && m_hSupplier != NULL)
		m_hSupplyLine->SetEntities(m_hSupplier, this);
}

void CStructure::ModifyContext(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);

	if (IsConstructing() || IsUpgrading())
	{
		pContext->SetBlend(BLEND_ALPHA);
		pContext->SetColor(Color(255, 255, 255));
		pContext->SetAlpha(GetVisibility() * RemapValClamped(GameServer()->GetGameTime() - m_flConstructionStartTime, 0, 2, 0, 1));
		pContext->Translate(Vector(0, RemapValClamped(GameServer()->GetGameTime() - m_flConstructionStartTime, 0, 3, -3, 0), 0));
	}

	if (GetTeam())
		pContext->SetColorSwap(GetTeam()->GetColor());
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

size_t CSupplier::s_iTendrilBeam = 0;

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
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flTendrilGrowthStartTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bShouldRender);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CSupplier);
INPUTS_TABLE_END();

void CSupplier::Precache()
{
	BaseClass::Precache();

	s_iTendrilBeam = CTextureLibrary::AddTextureID(L"textures/tendril.png");
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

float CSupplier::GetDataFlow(Vector vecPoint)
{
	return RemapValClamped((vecPoint - GetOrigin()).Length(), GetBoundingRadius(), GetDataFlowRadius()+GetBoundingRadius(), (float)m_iDataStrength, 0);
}

float CSupplier::GetDataFlow(Vector vecPoint, CTeam* pTeam, CSupplier* pIgnore)
{
	if (!pTeam)
		return 0;

	float flDataStrength = 0;
	size_t iNumMembers = pTeam->GetNumMembers();
	for (size_t i = 0; i < iNumMembers; i++)
	{
		CBaseEntity* pEntity = pTeam->GetMember(i);
		if (!pEntity)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
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
		m_flBonusDataFlow = sqrt(m_hSupplier->GetDataFlow(GetOrigin())/M_PI);
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

float CSupplier::GetChildEfficiency()
{
	size_t iEfficientChildren = EfficientChildren();

	if (iEfficientChildren == 0)
		return 1;

	size_t iConsumingChildren = 0;
	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		if (m_ahChildren[i] == NULL)
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
	if (!GetTeam())
	{
		for (size_t i = 0; i < m_ahChildren.size(); i++)
		{
			if (m_ahChildren[i] == NULL)
				continue;

			m_ahChildren[i]->SetSupplier(NULL);

			CStructure* pStructure = dynamic_cast<CStructure*>(m_ahChildren[i].GetPointer());
			if (!pStructure)
				continue;

			if (pStructure->GetTeam())
			{
				pStructure->GetTeam()->RemoveEntity(pStructure);
				DigitanksGame()->OnDisabled(pStructure, NULL, NULL);
			}
		}
	}

	BaseClass::OnTeamChange();
	UpdateTendrils();

	// This happens in UpdateTendrils() but do it again here anyway because UpdateTendrils only does it if there's a new tendril created.
	DigitanksGame()->GetTerrain()->DirtyChunkTexturesWithinDistance(GetOrigin(), GetDataFlowRadius() + GetBoundingRadius());
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
	CDigitanksTeam* pLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (!DigitanksGame()->ShouldRenderFogOfWar() || pLocalTeam == GetDigitanksTeam())
		m_bShouldRender = true;
	else if (!GetTeam())
		// Let regular culling happen
		m_bShouldRender = true;
	else if (pLocalTeam)
	{
		float flVisibleDistance = GetDataFlowRadius();

		m_bShouldRender = false;
		for (size_t i = 0; i < pLocalTeam->GetNumMembers(); i++)
		{
			CBaseEntity* pEntity = pLocalTeam->GetMember(i);
			if (!pEntity)
				continue;

			CDigitank* pDigitank = dynamic_cast<CDigitank*>(pEntity);
			if (pDigitank)
			{
				float flDistanceSqr = pDigitank->GetOrigin().DistanceSqr(GetOrigin());

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

			CDigitanksEntity* pDTEnt = dynamic_cast<CDigitanksEntity*>(pEntity);
			if (pDTEnt)
			{
				float flDistanceSqr = pDTEnt->GetOrigin().DistanceSqr(GetOrigin());

				float flTotalVisibleDistance = pDTEnt->VisibleRange() + flVisibleDistance;

				if (flDistanceSqr < flTotalVisibleDistance*flTotalVisibleDistance)
				{
					m_bShouldRender = true;
					break;
				}
			}
		}
	}

	if (GetDigitanksTeam() && !IsDataFlowSource())
	{
		if (!GetSupplyLine() || GetSupplyLine()->GetIntegrity() <= CSupplyLine::MinimumIntegrity())
		{
			GetDigitanksTeam()->RemoveEntity(this);
			return;
		}
	}

	UpdateTendrils();

	// This can happen if it was removed this same turn by lack of supplier.
	if (!GetDigitanksTeam())
		return;

	if (IsConstructing())
		return;

	if (CNetwork::IsHost() && GetSupplyLine() && GetSupplyLine()->GetIntegrity() >= 0.7f || IsDataFlowSource())
	{
		for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
			if (!pEntity)
				continue;

			CDigitanksEntity* pDTEnt = dynamic_cast<CDigitanksEntity*>(pEntity);
			if (pDTEnt && pDTEnt->IsImprisoned())
			{
				float flFlow = GetDataFlow(pDTEnt->GetOrigin());
				if (flFlow > 0)
					pDTEnt->Rescue(this);
			}

			CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);
			if (!pStructure)
				continue;

			// Just in case!
			if (this == pStructure)
				continue;

			if (dynamic_cast<CResource*>(pStructure))
				continue;

			if (pStructure->GetTeam())
				continue;

			float flFlow = GetDataFlow(pStructure->GetOrigin());

			if (flFlow < 10)
				continue;

			// You will join us... OR DIE
			GetDigitanksTeam()->AddEntity(pStructure);
			pStructure->SetSupplier(this);
			pStructure->GetSupplyLine()->SetIntegrity(0.5f);
		}
	}
}

void CSupplier::CompleteConstruction()
{
	m_flTendrilGrowthStartTime = GameServer()->GetGameTime();

	BaseClass::CompleteConstruction();
}

#define GROWTH_TIME 3

CVar tendril_fade_distance("perf_tendril_fade_distance", "200");

void CSupplier::PostRender(bool bTransparent)
{
	BaseClass::PostRender(bTransparent);

	if (!bTransparent)
		return;

	float flGrowthTime = GameServer()->GetGameTime() - m_flTendrilGrowthStartTime;

	if (flGrowthTime < 0)
		return;

	if (flGrowthTime >= GROWTH_TIME)
		m_flTendrilGrowthStartTime = 0;

	if (m_flTendrilGrowthStartTime == 0)
	{
		DigitanksGame()->GetDigitanksRenderer()->AddTendrilBatch(this);
		return;
	}

	CDigitanksCamera* pCamera = DigitanksGame()->GetDigitanksCamera();
	Vector vecCamera = pCamera->GetCameraPosition();
	float flDistanceSqr = GetOrigin().DistanceSqr(vecCamera);
	float flFadeDistance = tendril_fade_distance.GetFloat();

	float flFadeAlpha = RemapValClamped(flDistanceSqr, flFadeDistance*flFadeDistance, (flFadeDistance+20)*(flFadeDistance+20), 1, 0);

	if (flFadeAlpha <= 0)
		return;

	float flTreeAlpha = 1.0f;
	if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(GetOrigin().x), CTerrain::WorldToArraySpace(GetOrigin().z), TB_TREE))
		flTreeAlpha = 0.3f;

	Color clrTeam = GetTeam()?GetTeam()->GetColor():Color(255,255,255,255);

	CRenderingContext r(GameServer()->GetRenderer());
	if (DigitanksGame()->ShouldRenderFogOfWar() && DigitanksGame()->GetDigitanksRenderer()->ShouldUseFramebuffers())
		r.UseFrameBuffer(DigitanksGame()->GetDigitanksRenderer()->GetVisibilityMaskedBuffer());
	r.SetDepthMask(false);
	r.BindTexture(s_iTendrilBeam);

	GLuint iScrollingTextureProgram = (GLuint)CShaderLibrary::GetScrollingTextureProgram();

	if (GameServer()->GetRenderer()->ShouldUseShaders())
	{
		GameServer()->GetRenderer()->UseProgram(iScrollingTextureProgram);

		GLuint flTime = glGetUniformLocation(iScrollingTextureProgram, "flTime");
		glUniform1f(flTime, GameServer()->GetGameTime());

		GLuint iTexture = glGetUniformLocation(iScrollingTextureProgram, "iTexture");
		glUniform1f(iTexture, 0);

		GLuint flAlpha = glGetUniformLocation(iScrollingTextureProgram, "flAlpha");
		glUniform1f(flAlpha, flFadeAlpha * flTreeAlpha);
	}

	if (m_flTendrilGrowthStartTime > 0)
	{
		float flTotalSize = (float)m_aTendrils.size() + GetBoundingRadius();

		for (size_t i = 0; i < m_aTendrils.size(); i++)
		{
			if (i < m_aTendrils.size() - 15)
				continue;

			CTendril* pTendril = &m_aTendrils[i];

			float flGrowthLength = RemapVal(flGrowthTime, 0, GROWTH_TIME, pTendril->m_flLength-flTotalSize, pTendril->m_flLength);

			if (flGrowthLength < 0)
				continue;

			Vector vecDestination = GetOrigin() + (pTendril->m_vecEndPoint - GetOrigin()).Normalized() * flGrowthLength;

			Vector vecPath = vecDestination - GetOrigin();
			vecPath.y = 0;

			float flDistance = vecPath.Length2D();
			Vector vecDirection = vecPath.Normalized();
			size_t iSegments = (size_t)(flDistance/3);

			if (GameServer()->GetRenderer()->ShouldUseShaders())
			{
				GLuint flSpeed = glGetUniformLocation(iScrollingTextureProgram, "flSpeed");
				glUniform1f(flSpeed, pTendril->m_flSpeed);
			}

			clrTeam.SetAlpha(105);

			CRopeRenderer oRope(GameServer()->GetRenderer(), s_iTendrilBeam, DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin()) + Vector(0, 1, 0), 1.0f);
			oRope.SetColor(clrTeam);
			oRope.SetTextureScale(pTendril->m_flScale);
			oRope.SetTextureOffset(pTendril->m_flOffset);
			oRope.SetForward(Vector(0, -1, 0));

			for (size_t i = 1; i < iSegments; i++)
			{
				clrTeam.SetAlpha((int)RemapVal((float)i, 1, (float)iSegments, 100, 30));
				oRope.SetColor(clrTeam);

				float flCurrentDistance = ((float)i*flDistance)/iSegments;
				oRope.AddLink(DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + vecDirection*flCurrentDistance) + Vector(0, 1, 0));
			}

			oRope.Finish(DigitanksGame()->GetTerrain()->SetPointHeight(vecDestination) + Vector(0, 1, 0));
		}
	}

	if (GameServer()->GetRenderer()->ShouldUseShaders())
		GameServer()->GetRenderer()->ClearProgram();
}

void CSupplier::UpdateTendrils()
{
	if (IsConstructing())
		return;

	if (!GetTeam())
	{
		if (m_iTendrilsCallList)
			glDeleteLists((GLuint)m_iTendrilsCallList, 1);

		m_iTendrilsCallList = 0;

		DigitanksGame()->GetTerrain()->DirtyChunkTexturesWithinDistance(GetOrigin(), GetDataFlowRadius() + GetBoundingRadius());
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
		pTendril->m_vecEndPoint = DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + AngleVector(EAngle(0, RandomFloat(0, 360), 0)) * pTendril->m_flLength);
		pTendril->m_flScale = RandomFloat(3, 7);
		pTendril->m_flOffset = RandomFloat(0, 1);
		pTendril->m_flSpeed = RandomFloat(0.5f, 2);

		bUpdateTerrain = true;
	}

	if (bUpdateTerrain)
		DigitanksGame()->GetTerrain()->DirtyChunkTexturesWithinDistance(GetOrigin(), GetDataFlowRadius() + GetBoundingRadius());

	if (m_iTendrilsCallList)
		glDeleteLists((GLuint)m_iTendrilsCallList, 1);

	m_iTendrilsCallList = glGenLists(1);

	glNewList((GLuint)m_iTendrilsCallList, GL_COMPILE);
	for (size_t i = 0; i < m_aTendrils.size(); i++)
	{
		// Only show the longest tendrils, for perf reasons.
		if (i < iRadius - 15)
			continue;

		CTendril* pTendril = &m_aTendrils[i];

		Vector vecDestination = pTendril->m_vecEndPoint;

		Vector vecPath = vecDestination - GetOrigin();
		vecPath.y = 0;

		float flDistance = vecPath.Length2D();
		Vector vecDirection = vecPath.Normalized();
		size_t iSegments = (size_t)(flDistance/3);

		Color clrTeam = GetTeam()->GetColor();

		GLuint iScrollingTextureProgram = (GLuint)CShaderLibrary::GetScrollingTextureProgram();

		GLuint flSpeed = glGetUniformLocation(iScrollingTextureProgram, "flSpeed");
		glUniform1f(flSpeed, pTendril->m_flSpeed);

		clrTeam.SetAlpha(105);

		CRopeRenderer oRope(GameServer()->GetRenderer(), s_iTendrilBeam, DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin()) + Vector(0, 1, 0), 1.0f);
		oRope.SetColor(clrTeam);
		oRope.SetTextureScale(pTendril->m_flScale);
		oRope.SetTextureOffset(pTendril->m_flOffset);
		oRope.SetForward(Vector(0, -1, 0));

		for (size_t i = 1; i < iSegments; i++)
		{
			clrTeam.SetAlpha((int)RemapVal((float)i, 1, (float)iSegments, 100, 30));
			oRope.SetColor(clrTeam);

			float flCurrentDistance = ((float)i*flDistance)/iSegments;
			oRope.AddLink(DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + vecDirection*flCurrentDistance) + Vector(0, 1, 0));
		}

		oRope.Finish(DigitanksGame()->GetTerrain()->SetPointHeight(vecDestination) + Vector(0, 1, 0));
	}
	glEndList();
}

void CSupplier::BeginTendrilGrowth()
{
	m_flTendrilGrowthStartTime = GameServer()->GetGameTime();
}

void CSupplier::AddChild(CStructure* pChild)
{
	if (!CNetwork::IsHost())
		return;

	if (!pChild)
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = pChild->GetHandle();

	AddChild(&p);

	CNetwork::CallFunctionParameters(NETWORK_TOCLIENTS, "AddChild", &p);
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
	if (!CNetwork::IsHost())
		return;

	if (!pChild)
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = pChild->GetHandle();

	AddChild(&p);

	CNetwork::CallFunctionParameters(NETWORK_TOCLIENTS, "RemoveChild", &p);
}

void CSupplier::RemoveChild(CNetworkParameters* p)
{
	CEntityHandle<CStructure> hChild(p->ui2);

	if (hChild == NULL)
		return;

	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		if (m_ahChildren[i] == NULL)
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

		if (pSupplier->GetTeam() != pUnit->GetTeam())
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (!pClosest)
		{
			pClosest = pSupplier;
			continue;
		}

		if ((pSupplier->GetOrigin() - pUnit->GetOrigin()).Length() < (pClosest->GetOrigin() - pUnit->GetOrigin()).Length())
			pClosest = pSupplier;
	}

	return pClosest;
}

CSupplier* CSupplier::FindClosestSupplier(Vector vecPoint, CTeam* pTeam)
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

		if (pSupplier->GetTeam() != pTeam)
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (pSupplier->GetDataFlow(vecPoint) > 0)
		{
			if (!pClosestInNetwork || (pSupplier->GetOrigin() - vecPoint).Length() < (pClosestInNetwork->GetOrigin() - vecPoint).Length())
				pClosestInNetwork = pSupplier;
		}

		if (!pClosest)
		{
			pClosest = pSupplier;
			continue;
		}

		if ((pSupplier->GetOrigin() - vecPoint).Length() < (pClosest->GetOrigin() - vecPoint).Length())
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

	if (!GetDigitanksTeam())
		return false;

	if (!GetDigitanksTeam()->GetPrimaryCPU())
		return false;

	unittype_t ePreviewStructure = GetDigitanksTeam()->GetPrimaryCPU()->GetPreviewStructure();
	if (ePreviewStructure == STRUCTURE_PSU || ePreviewStructure == STRUCTURE_BATTERY)
		return false;

	if (GetTeam() != DigitanksGame()->GetCurrentLocalDigitanksTeam())
		return false;

	if (IsConstructing())
		return false;

	// In build mode show everybody, otherwise only show the selected structure.
	if (DigitanksGame()->GetControlMode() == MODE_BUILD || GetDigitanksTeam()->IsSelected(this))
		return true;

	return false;
}
