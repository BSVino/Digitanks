#include "structure.h"

#include <sstream>

#include <maths.h>
#include <mtrand.h>

#include "dt_renderer.h"
#include "digitanksgame.h"
#include "supplyline.h"

#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <shaders/shaders.h>
#include <models/models.h>
#include <network/network.h>

#include "collector.h"
#include "loader.h"

#include <GL/glew.h>

NETVAR_TABLE_BEGIN(CStructure);
	NETVAR_DEFINE(bool, m_bConstructing);
	NETVAR_DEFINE(size_t, m_iProductionToConstruct);

	NETVAR_DEFINE(bool, m_bInstalling);
	NETVAR_DEFINE(updatetype_t, m_eInstallingType);
	NETVAR_DEFINE(int, m_iInstallingUpdate);
	NETVAR_DEFINE(size_t, m_iProductionToInstall);

	NETVAR_DEFINE(bool, m_bUpgrading);
	NETVAR_DEFINE(size_t, m_iProductionToUpgrade);

	NETVAR_DEFINE(CEntityHandle<CSupplyLine>, m_hSupplier);
	NETVAR_DEFINE(CEntityHandle<CSupplyLine>, m_hSupplyLine);

	NETVAR_DEFINE(size_t, m_iFleetSupply);
	NETVAR_DEFINE(size_t, m_iBandwidth);
	NETVAR_DEFINE(size_t, m_iPower);
	NETVAR_DEFINE(size_t, m_iEnergyBonus);
	NETVAR_DEFINE(float, m_flRechargeBonus);

	NETVAR_DEFINE(float, m_flScaffoldingSize);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CStructure);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bConstructing);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iProductionToConstruct);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bInstalling);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, updatetype_t, m_eInstallingType);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, int, m_iInstallingUpdate);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iProductionToInstall);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bUpgrading);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iProductionToUpgrade);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CSupplier>, m_hSupplier);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CSupplyLine>, m_hSupplyLine);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iFleetSupply);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iBandwidth);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iEnergyBonus);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flRechargeBonus);

	//std::map<size_t, std::vector<whateveritisnow> >	m_aUpdates;	// OnSerialize()
	//std::map<size_t, size_t>		m_aiUpdatesInstalled;	// OnSerialize()

	//size_t						m_iScaffolding;	// In Spawn()
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flScaffoldingSize);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flConstructionStartTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, defender_t, m_aoDefenders);
SAVEDATA_TABLE_END();

CStructure::CStructure()
{
	m_bConstructing = false;
	m_iProductionToConstruct = 0;

	m_bInstalling = false;
	m_iProductionToInstall = 0;

	m_bUpgrading = false;
	m_iProductionToUpgrade = 0;

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
	m_iBandwidth = InitialBandwidth();
	m_iPower = InitialPower();
	m_iEnergyBonus = InitialEnergyBonus();
	m_flRechargeBonus = InitialRechargeBonus();

	m_iScaffolding = CModelLibrary::Get()->FindModel(L"models/structures/scaffolding.obj");
	m_flScaffoldingSize = 10;

	m_flConstructionStartTime = 0;

	m_bConstructing = true;
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

	if (IsInstalling() && GetUpdateInstalling())
	{
		if (GetDigitanksTeam()->GetProductionPerLoader() >= GetProductionToInstall())
		{
			std::wstringstream s;
			s << L"'" << GetUpdateInstalling()->GetName() << L"' finished installing on " << GetName();
			DigitanksGame()->AppendTurnInfo(s.str().c_str());

			InstallComplete();
		}
		else
		{
			AddProduction((size_t)GetDigitanksTeam()->GetProductionPerLoader());

			std::wstringstream s;
			s << L"Installing '" << GetUpdateInstalling()->GetName() << L"' on " << GetName() << L" (" << GetTurnsToInstall() << L" turns left)";
			DigitanksGame()->AppendTurnInfo(s.str().c_str());
		}
	}

	if (IsUpgrading())
	{
		if (GetDigitanksTeam()->GetProductionPerLoader() >= GetProductionToUpgrade())
		{
			std::wstringstream s;
			s << L"" << GetName() << L" finished upgrading.";
			DigitanksGame()->AppendTurnInfo(s.str().c_str());

			UpgradeComplete();
		}
		else
		{
			AddProduction((size_t)GetDigitanksTeam()->GetProductionPerLoader());

			std::wstringstream s;
			s << L"Upgrading " << GetName() << L" (" << GetTurnsToUpgrade() << L" turns left)";
			DigitanksGame()->AppendTurnInfo(s.str().c_str());
		}
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

	SetOrigin(Vector(GetOrigin().x, flHeight, GetOrigin().z));
}

void CStructure::PostRender()
{
	BaseClass::PostRender();

	if ((IsConstructing() || IsUpgrading()) && GetVisibility() > 0)
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.Translate(GetOrigin() - Vector(0, 10, 0));
		c.Scale(m_flScaffoldingSize, m_flScaffoldingSize, m_flScaffoldingSize);
		c.SetBlend(BLEND_ADDITIVE);
		c.SetAlpha(GetVisibility() * 0.2f * RemapValClamped(GameServer()->GetGameTime() - m_flConstructionStartTime, 0, 3, 0, 1));
		c.SetDepthMask(false);
		c.SetBackCulling(false);
		c.RenderModel(m_iScaffolding);
	}
}

void CStructure::BeginConstruction()
{
	m_iProductionToConstruct = ConstructionCost();
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

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_POWER);
	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_PSU);
	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_LOADER);

	size_t iTutorial = CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial();
	if (iTutorial == CInstructor::TUTORIAL_POWER)
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();

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

	if (dynamic_cast<CCollector*>(this) && iTutorial == CInstructor::TUTORIAL_PSU)
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();

	if (dynamic_cast<CLoader*>(this) && iTutorial == CInstructor::TUTORIAL_LOADER)
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();
}

size_t CStructure::GetTurnsToConstruct()
{
	if (!GetDigitanksTeam())
		return 9999;

	return (size_t)(m_iProductionToConstruct/GetDigitanksTeam()->GetProductionPerLoader())+1;
}

void CStructure::AddProduction(size_t iProduction)
{
	if (IsConstructing())
	{
		if (iProduction > m_iProductionToConstruct)
			m_iProductionToConstruct = 0;
		else
			m_iProductionToConstruct -= iProduction;
	}
	else if (IsInstalling())
	{
		if (iProduction > m_iProductionToInstall)
			m_iProductionToInstall = 0;
		else
			m_iProductionToInstall -= iProduction;
	}
	else if (IsUpgrading())
	{
		if (iProduction > m_iProductionToUpgrade)
			m_iProductionToUpgrade = 0;
		else
			m_iProductionToUpgrade -= iProduction;
	}
}

size_t CStructure::GetTurnsToConstruct(size_t iPower)
{
	return (size_t)(iPower/GetDigitanksTeam()->GetProductionPerLoader())+1;
}

void CStructure::InstallUpdate(updatetype_t eUpdate)
{
	if (IsUpgrading())
		return;

	int iUninstalled = GetFirstUninstalledUpdate(eUpdate);
	if (iUninstalled < 0)
		return;

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.i2 = eUpdate;

	if (CNetwork::IsHost())
		InstallUpdate(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "InstallUpdate", &p);
}

void CStructure::InstallUpdate(CNetworkParameters* p)
{
	updatetype_t eUpdate = (updatetype_t)p->i2;

	m_bInstalling = true;

	int iUninstalled = GetFirstUninstalledUpdate(eUpdate);
	m_eInstallingType = eUpdate;
	m_iInstallingUpdate = iUninstalled;

	m_iProductionToInstall = GetUpdate(eUpdate, iUninstalled)->m_iProductionToInstall;

	GetDigitanksTeam()->CountProducers();
}

void CStructure::InstallComplete()
{
	m_bInstalling = false;

	m_aiUpdatesInstalled[m_eInstallingType] = m_iInstallingUpdate+1;

	CUpdateItem* pUpdate = GetUpdate(m_eInstallingType, m_iInstallingUpdate);

	switch (pUpdate->m_eUpdateType)
	{
	case UPDATETYPE_BANDWIDTH:
		m_iBandwidth += (size_t)pUpdate->m_flValue;
		break;

	case UPDATETYPE_PRODUCTION:
		m_iPower += (size_t)pUpdate->m_flValue;
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

	DigitanksGame()->AddActionItem(this, ACTIONTYPE_INSTALLATION);
}

void CStructure::CancelInstall()
{
	CNetworkParameters p;
	p.ui1 = GetHandle();

	if (CNetwork::IsHost())
		CancelInstall(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "CancelInstall", &p);
}

void CStructure::CancelInstall(CNetworkParameters* p)
{
	m_bInstalling = false;

	m_iProductionToInstall = 0;
}

size_t CStructure::GetTurnsToInstall(CUpdateItem* pItem)
{
	return (size_t)(pItem->m_iProductionToInstall/GetDigitanksTeam()->GetProductionPerLoader())+1;
}

size_t CStructure::GetTurnsToInstall()
{
	return (size_t)(m_iProductionToInstall/GetDigitanksTeam()->GetProductionPerLoader())+1;
}

int CStructure::GetFirstUninstalledUpdate(updatetype_t eUpdate)
{
	std::vector<CUpdateCoordinate>& aUpdates = m_aUpdates[eUpdate];
	size_t iUpdatesInstalled = m_aiUpdatesInstalled[eUpdate];

	if (iUpdatesInstalled >= aUpdates.size())
		return -1;

	if (aUpdates.size() == 0)
		return -1;

	return (int)iUpdatesInstalled;
}

CUpdateItem* CStructure::GetUpdateInstalling()
{
	if (m_aUpdates.find(m_eInstallingType) == m_aUpdates.end())
		return NULL;

	if (m_iInstallingUpdate >= (int)m_aUpdates[m_eInstallingType].size())
		return NULL;

	if (IsInstalling())
		return GetUpdate(m_eInstallingType, m_iInstallingUpdate);

	return NULL;
}

CUpdateItem* CStructure::GetUpdate(size_t iType, size_t iUpdate)
{
	CUpdateCoordinate* pCoordinates = &m_aUpdates[iType][iUpdate];

	return &DigitanksGame()->GetUpdateGrid()->m_aUpdates[pCoordinates->x][pCoordinates->y];
}

void CStructure::DownloadComplete(size_t x, size_t y)
{
	CUpdateItem* pItem = &DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y];

	if (IsConstructing())
		return;

	if (pItem->m_eStructure != GetUnitType())
		return;

	if (pItem->m_eUpdateClass != UPDATECLASS_STRUCTUREUPDATE)
		return;

	m_aUpdates[pItem->m_eUpdateType].push_back(CUpdateCoordinate());
	CUpdateCoordinate* pC = &m_aUpdates[pItem->m_eUpdateType][m_aUpdates[pItem->m_eUpdateType].size()-1];
	pC->x = x;
	pC->y = y;
}

size_t CStructure::GetUpdatesScore()
{
	size_t iScore = 0;
	for (size_t i = 0; i < UPDATETYPE_SIZE; i++)
	{
		for (size_t j = 0; j < m_aUpdates[i].size(); j++)
		{
			if (m_aiUpdatesInstalled[i] > j)
				iScore += GetUpdate(i, j)->m_iProductionToInstall;
		}
	}

	return iScore;
}

void CStructure::BeginUpgrade()
{
	if (!CanStructureUpgrade())
		return;

	if (IsInstalling())
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
	m_bUpgrading = true;

	m_iProductionToUpgrade = UpgradeCost();

	GetDigitanksTeam()->CountProducers();
}

void CStructure::CancelUpgrade()
{
	CNetworkParameters p;
	p.ui1 = GetHandle();

	if (CNetwork::IsHost())
		CancelUpgrade(&p);
	else
		CNetwork::CallFunctionParameters(NETWORK_TOSERVER, "CancelUpgrade", &p);
}

void CStructure::CancelUpgrade(CNetworkParameters* p)
{
	m_bUpgrading = false;

	m_iProductionToUpgrade = 0;

	GetDigitanksTeam()->CountProducers();
}

size_t CStructure::GetTurnsToUpgrade()
{
	if (IsUpgrading())
		return (size_t)(m_iProductionToUpgrade/GetDigitanksTeam()->GetProductionPerLoader())+1;
	else
		return (size_t)(UpgradeCost()/GetDigitanksTeam()->GetProductionPerLoader())+1;
}

bool CStructure::NeedsOrders()
{
	if (IsUpgrading() || IsConstructing() || IsInstalling())
		return false;

	if (CanStructureUpgrade())
		return true;

	if (HasUpdatesAvailable())
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

void CStructure::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

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

	for (std::map<size_t, std::vector<CUpdateCoordinate> >::iterator i = m_aUpdates.begin(); i != m_aUpdates.end(); i++)
	{
		CNetworkParameters p;
		p.ui1 = GetHandle();
		p.ui2 = i->first;
		p.ui3 = i->second.size();
		p.ui4 = m_aiUpdatesInstalled[i->first];
		p.CreateExtraData(sizeof(size_t) * i->second.size() * 2);

		size_t* pCoordinates = (size_t*)p.m_pExtraData;
		for (size_t j = 0; j < i->second.size(); j++)
		{
			pCoordinates[j*2] = i->second[j].x;
			pCoordinates[j*2+1] = i->second[j].y;
		}

		CNetwork::CallFunctionParameters(iClient, "AddStructureUpdate", &p);
	}
}

void CStructure::AddStructureUpdate(CNetworkParameters* p)
{
	size_t iKey = p->ui2;
	size_t iCoordinates = p->ui3;
	size_t iInstalled = p->ui4;

	m_aiUpdatesInstalled[iKey] = iInstalled;

	m_aUpdates[iKey].clear();
	for (size_t i = 0; i < iCoordinates; i++)
	{
		size_t* pCoordinates = (size_t*)p->m_pExtraData;
		m_aUpdates[iKey].push_back(CUpdateCoordinate());
		CUpdateCoordinate* pCoordinate = &m_aUpdates[iKey][m_aUpdates[iKey].size()-1];
		pCoordinate->x = pCoordinates[i*2];
		pCoordinate->y = pCoordinates[i*2+1];
	}
}

void CStructure::OnSerialize(std::ostream& o)
{
	size_t iUpdates = m_aUpdates.size();
	o.write((char*)&iUpdates, sizeof(iUpdates));

	std::map<size_t, std::vector<CUpdateCoordinate> >::iterator it;
	for (it = m_aUpdates.begin(); it != m_aUpdates.end(); it++)
	{
		iUpdates--;

		size_t iCategory = it->first;
		o.write((char*)&iCategory, sizeof(iCategory));

		size_t iItemsInstalled = m_aiUpdatesInstalled[it->first];
		o.write((char*)&iItemsInstalled, sizeof(iItemsInstalled));

		size_t iItemsInCategory = it->second.size();
		o.write((char*)&iItemsInCategory, sizeof(iItemsInCategory));

		for (size_t k = 0; k < iItemsInCategory; k++)
		{
			size_t x = m_aUpdates[it->first][k].x;
			size_t y = m_aUpdates[it->first][k].y;

			o.write((char*)&x, sizeof(x));
			o.write((char*)&y, sizeof(y));
		}
	}

	assert(iUpdates == 0);

	BaseClass::OnSerialize(o);
}

bool CStructure::OnUnserialize(std::istream& i)
{
	size_t iUpdates;
	i.read((char*)&iUpdates, sizeof(iUpdates));

	for (size_t j = 0; j < iUpdates; j++)
	{
		size_t iCategory;
		i.read((char*)&iCategory, sizeof(iCategory));

		size_t iItemsInstalled;
		i.read((char*)&iItemsInstalled, sizeof(iItemsInstalled));
		m_aiUpdatesInstalled[iCategory] = iItemsInstalled;

		size_t iItemsInCategory;
		i.read((char*)&iItemsInCategory, sizeof(iItemsInCategory));

		for (size_t k = 0; k < iItemsInCategory; k++)
		{
			size_t x, y;
			i.read((char*)&x, sizeof(x));
			i.read((char*)&y, sizeof(y));

			m_aUpdates[iCategory].push_back(CUpdateCoordinate());
			CUpdateCoordinate* pC = &m_aUpdates[iCategory][m_aUpdates[iCategory].size()-1];
			pC->x = x;
			pC->y = y;
		}
	}

	return BaseClass::OnUnserialize(i);
}

size_t CSupplier::s_iTendrilBeam = 0;

NETVAR_TABLE_BEGIN(CSupplier);
	NETVAR_DEFINE(size_t, m_iDataStrength);
	NETVAR_DEFINE(float, m_flBonusDataFlow);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CSupplier);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iDataStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBonusDataFlow);
	//std::vector<CTendril>		m_aTendrils;	// Generated
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<CStructure>, m_ahChildren);
	//size_t						m_iTendrilsCallList;	// Generated
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flTendrilGrowthStartTime);
SAVEDATA_TABLE_END();

void CSupplier::Precache()
{
	BaseClass::Precache();

	s_iTendrilBeam = CRenderer::LoadTextureIntoGL(L"textures/tendril.png");
}

void CSupplier::Spawn()
{
	BaseClass::Spawn();

	m_iDataStrength = InitialDataStrength();
	m_flBonusDataFlow = 0;

	m_iTendrilsCallList = 0;

	m_flTendrilGrowthStartTime = 0;
}

void CSupplier::ClientEnterGame()
{
	BaseClass::ClientEnterGame();

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
	return RemapValClamped((vecPoint - GetOrigin()).Length2D(), GetBoundingRadius(), GetDataFlowRadius()+GetBoundingRadius(), (float)m_iDataStrength, 0);
}

float CSupplier::GetDataFlow(Vector vecPoint, CTeam* pTeam, CSupplier* pIgnore)
{
	float flDataStrength = 0;
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (pSupplier->GetTeam() != pTeam)
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
				pStructure->GetTeam()->RemoveEntity(pStructure);
		}
	}

	BaseClass::OnTeamChange();
	UpdateTendrils();
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
		for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
			if (!pEntity)
				continue;

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

void CSupplier::PostRender()
{
	BaseClass::PostRender();

	CRenderingContext r(GameServer()->GetRenderer());
	if (DigitanksGame()->ShouldRenderFogOfWar())
		r.UseFrameBuffer(DigitanksGame()->GetDigitanksRenderer()->GetVisibilityMaskedBuffer());
	r.SetDepthMask(false);
	r.BindTexture(s_iTendrilBeam);

	GLuint iScrollingTextureProgram = (GLuint)CShaderLibrary::GetScrollingTextureProgram();
	glUseProgram(iScrollingTextureProgram);

	GLuint flTime = glGetUniformLocation(iScrollingTextureProgram, "flTime");
	glUniform1f(flTime, GameServer()->GetGameTime());

	GLuint iTexture = glGetUniformLocation(iScrollingTextureProgram, "iTexture");
	glUniform1f(iTexture, 0);

	float flGrowthTime = GameServer()->GetGameTime() - m_flTendrilGrowthStartTime;
	if (flGrowthTime >= GROWTH_TIME)
		m_flTendrilGrowthStartTime = 0;

	if (m_flTendrilGrowthStartTime > 0)
	{
		float flTotalSize = (float)m_aTendrils.size() + GetBoundingRadius();

		for (size_t i = 0; i < m_aTendrils.size(); i++)
		{
			CTendril* pTendril = &m_aTendrils[i];

			float flGrowthLength = RemapVal(flGrowthTime, 0, GROWTH_TIME, pTendril->m_flLength-flTotalSize, pTendril->m_flLength);

			if (flGrowthLength < 0)
				continue;

			Vector vecDestination = GetOrigin() + (pTendril->m_vecEndPoint - GetOrigin()).Normalized() * flGrowthLength;

			Vector vecPath = vecDestination - GetOrigin();
			vecPath.y = 0;

			float flDistance = vecPath.Length2D();
			Vector vecDirection = vecPath.Normalized();
			size_t iSegments = (size_t)(flDistance/2);

			Color clrTeam = GetTeam()->GetColor();

			GLuint iScrollingTextureProgram = (GLuint)CShaderLibrary::GetScrollingTextureProgram();

			GLuint flSpeed = glGetUniformLocation(iScrollingTextureProgram, "flSpeed");
			glUniform1f(flSpeed, pTendril->m_flSpeed);

			CRopeRenderer oRope(GameServer()->GetRenderer(), s_iTendrilBeam, DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin()) + Vector(0, 1, 0));
			oRope.SetColor(clrTeam);
			oRope.SetTextureScale(pTendril->m_flScale);
			oRope.SetTextureOffset(pTendril->m_flOffset);
			oRope.SetForward(Vector(0, -1, 0));

			for (size_t i = 1; i < iSegments; i++)
			{
				oRope.SetColor(Color(clrTeam.r(), clrTeam.g(), clrTeam.b(), (int)RemapVal((float)i, 1, (float)iSegments, 155, 50)));

				float flCurrentDistance = ((float)i*flDistance)/iSegments;
				oRope.AddLink(DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + vecDirection*flCurrentDistance) + Vector(0, 1, 0));
			}

			oRope.Finish(DigitanksGame()->GetTerrain()->SetPointHeight(vecDestination) + Vector(0, 1, 0));
		}
	}
	else if (m_iTendrilsCallList)
		glCallList((GLuint)m_iTendrilsCallList);

	glUseProgram(0);
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
		return;
	}

	if (GameServer()->IsLoading())
		return;

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
	}

	if (m_iTendrilsCallList)
		glDeleteLists((GLuint)m_iTendrilsCallList, 1);

	m_iTendrilsCallList = glGenLists(1);

	glNewList((GLuint)m_iTendrilsCallList, GL_COMPILE);
	for (size_t i = 0; i < m_aTendrils.size(); i++)
	{
		CTendril* pTendril = &m_aTendrils[i];

		Vector vecDestination = pTendril->m_vecEndPoint;

		Vector vecPath = vecDestination - GetOrigin();
		vecPath.y = 0;

		float flDistance = vecPath.Length2D();
		Vector vecDirection = vecPath.Normalized();
		size_t iSegments = (size_t)(flDistance/2);

		Color clrTeam = GetTeam()->GetColor();

		GLuint iScrollingTextureProgram = (GLuint)CShaderLibrary::GetScrollingTextureProgram();

		GLuint flSpeed = glGetUniformLocation(iScrollingTextureProgram, "flSpeed");
		glUniform1f(flSpeed, pTendril->m_flSpeed);

		CRopeRenderer oRope(GameServer()->GetRenderer(), s_iTendrilBeam, DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin()) + Vector(0, 1, 0));
		oRope.SetColor(clrTeam);
		oRope.SetTextureScale(pTendril->m_flScale);
		oRope.SetTextureOffset(pTendril->m_flOffset);
		oRope.SetForward(Vector(0, -1, 0));

		for (size_t i = 1; i < iSegments; i++)
		{
			oRope.SetColor(Color(clrTeam.r(), clrTeam.g(), clrTeam.b(), (int)RemapVal((float)i, 1, (float)iSegments, 155, 50)));

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

	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
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

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
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

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (pSupplier->GetTeam() != pTeam)
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (!pClosest)
		{
			pClosest = pSupplier;
			continue;
		}

		if ((pSupplier->GetOrigin() - vecPoint).Length() < (pClosest->GetOrigin() - vecPoint).Length())
			pClosest = pSupplier;
	}

	return pClosest;
}

float CSupplier::VisibleRange() const
{
	return GetDataFlowRadius() + 5;
}
