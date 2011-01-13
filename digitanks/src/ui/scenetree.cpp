#include "scenetree.h"

#include <game/gameserver.h>
#include <game/digitanks/digitanksgame.h>
#include "digitankswindow.h"
#include "hud.h"
#include <renderer\renderer.h>
#include <game/digitanks/dt_camera.h>

CSceneTree::CSceneTree()
	: CTree(CRenderer::LoadTextureIntoGL(L"textures/hud/arrow.png"), 0, 0)
{
}

void CSceneTree::Layout()
{
	SetSize(200, glgui::CRootPanel::Get()->GetHeight()-300);
	SetPos(15, 60);

	BaseClass::Layout();
}

void CSceneTree::BuildTree(bool bForce)
{
	CDigitanksTeam* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	if (!pCurrentLocalTeam)
		return;

	if (m_hTeam == pCurrentLocalTeam && !bForce)
		return;

	ClearTree();

	m_hTeam = pCurrentLocalTeam;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CSelectable* pEntity = CBaseEntity::GetEntityType<CSelectable>(i);
		if (!pEntity)
			continue;

		if (pEntity->GetDigitanksTeam() != pCurrentLocalTeam)
			continue;

		unittype_t eUnit = pEntity->GetUnitType();

		CSceneTreeGroup* pTreeGroup = GetUnitNode(eUnit);

		pTreeGroup->AddNode(new CSceneTreeUnit(pEntity, pTreeGroup, this));
	}

	Layout();
}

CSceneTreeGroup* CSceneTree::GetUnitNode(unittype_t eUnit)
{
	for (size_t i = 0; i < m_apNodes.size(); i++)
	{
		CSceneTreeGroup* pTreeGroup = dynamic_cast<CSceneTreeGroup*>(m_apNodes[i]);
		if (eUnit == pTreeGroup->GetUnitType())
			return pTreeGroup;
	}

	// It's not there. Insert it.
	for (size_t i = 0; i < m_apNodes.size(); i++)
	{
		CSceneTreeGroup* pTreeGroup = dynamic_cast<CSceneTreeGroup*>(m_apNodes[i]);
		if (eUnit < pTreeGroup->GetUnitType())
		{
			CSceneTreeGroup* pReturn = new CSceneTreeGroup(eUnit, this);
			AddNode(pReturn, i);
			return pReturn;
		}
	}

	CSceneTreeGroup* pReturn = new CSceneTreeGroup(eUnit, this);
	AddNode(pReturn);
	return pReturn;
}

void CSceneTree::Paint(int x, int y, int w, int h)
{
	glgui::CRootPanel::PaintRect(x, y, w, h, m_clrBackground);

	if (m_iHilighted != ~0)
	{
		IControl* pNode = m_apControls[m_iHilighted];
		int cx, cy, cw, ch;
		pNode->GetAbsDimensions(cx, cy, cw, ch);
		glgui::CRootPanel::PaintRect(cx+ch, cy, ch, ch, Color(255, 255, 255, 100));
	}

	// Skip CTree, we override its hilight and selection methods.
	CPanel::Paint(x, y, w, h);
}

void CSceneTree::OnAddEntityToTeam(CDigitanksTeam* pTeam, CBaseEntity* pEntity)
{
	CSelectable* pSelectable = dynamic_cast<CSelectable*>(pEntity);
	if (!pSelectable)
		return;

	if (pTeam != m_hTeam)
		return;

	unittype_t eUnit = pSelectable->GetUnitType();
	CSceneTreeGroup* pTreeGroup = GetUnitNode(eUnit);

	pTreeGroup->AddNode(new CSceneTreeUnit(pSelectable, pTreeGroup, this));

	Layout();
}

void CSceneTree::OnRemoveEntityFromTeam(CDigitanksTeam* pTeam, CBaseEntity* pEntity)
{
	CSelectable* pSelectable = dynamic_cast<CSelectable*>(pEntity);
	if (!pSelectable)
		return;

	if (pTeam != m_hTeam)
		return;

	unittype_t eUnit = pSelectable->GetUnitType();
	CSceneTreeGroup* pTreeGroup = GetUnitNode(eUnit);

	for (size_t i = 0; i < pTreeGroup->m_apNodes.size(); i++)
	{
		CSceneTreeUnit* pUnit = dynamic_cast<CSceneTreeUnit*>(pTreeGroup->m_apNodes[i]);
		if (!pUnit)
			continue;

		if (pUnit->GetEntity() == pSelectable)
		{
			RemoveNode(pUnit);
			pUnit->Destructor();
			pUnit->Delete();

			if (pTreeGroup->m_apNodes.size() == 0)
			{
				RemoveNode(pTreeGroup);
				pTreeGroup->Destructor();
				pTreeGroup->Delete();
			}

			break;
		}
	}

	Layout();
}

CSceneTreeGroup::CSceneTreeGroup(unittype_t eUnit, glgui::CTree* pTree)
	: CSceneTreeNode(NULL, pTree)
{
	m_eUnit = eUnit;
}

void CSceneTreeGroup::Paint(int x, int y, int w, int h, bool bFloating)
{
	if (!IsVisible())
		return;

	BaseClass::Paint(x, y, w, h, bFloating);

	CDigitanksTeam* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	CRenderingContext c(GameServer()->GetRenderer());
	c.SetBlend(BLEND_ALPHA);

	CHUD::PaintUnitSheet(m_eUnit, x+h, y, h, h, pCurrentLocalTeam?pCurrentLocalTeam->GetColor():Color(255,255,255,255));
}

CSceneTreeUnit::CSceneTreeUnit(CEntityHandle<CSelectable> hEntity, glgui::CTreeNode* pParent, glgui::CTree* pTree)
	: CSceneTreeNode(pParent, pTree)
{
	m_hEntity = hEntity;
}

void CSceneTreeUnit::Paint(int x, int y, int w, int h, bool bFloating)
{
	if (!IsVisible())
		return;

	BaseClass::Paint(x, y, w, h, bFloating);

	if (m_hEntity == NULL)
		return;

	CRenderingContext c(GameServer()->GetRenderer());
	c.SetBlend(BLEND_ALPHA);

	Color clrTeam = m_hEntity->GetTeam()?m_hEntity->GetTeam()->GetColor():Color(255,255,255,255);

	CDigitank* pTank = dynamic_cast<CDigitank*>(m_hEntity.GetPointer());
	if (pTank && !pTank->NeedsOrders())
		clrTeam.SetAlpha(100);

	CDigitanksTeam* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	if (pCurrentLocalTeam && pCurrentLocalTeam->IsSelected(m_hEntity))
	{
		Color clrSelection(255, 255, 255, 64);
		if (pCurrentLocalTeam->IsPrimarySelection(m_hEntity))
			clrSelection = Color(255, 255, 255, 128);

		glgui::CRootPanel::PaintRect(x+h, y, h, h, clrSelection);

		clrSelection = Color(255, 255, 255, 128);
		if (pCurrentLocalTeam->IsPrimarySelection(m_hEntity))
			clrSelection = Color(255, 255, 255, 255);

		glgui::CRootPanel::PaintRect(x+h, y, h, 1, clrSelection);
		glgui::CRootPanel::PaintRect(x+h, y, 1, h, clrSelection);
		glgui::CRootPanel::PaintRect(x+h+h, y, 1, h, clrSelection);
		glgui::CRootPanel::PaintRect(x+h, y+h, h, 1, clrSelection);
	}

	CHUD::PaintUnitSheet(m_hEntity->GetUnitType(), x+h+2, y+2, h-4, h-4, clrTeam);

	glgui::CRootPanel::PaintRect(x+h, y, (int)(h*m_hEntity->GetHealth()/m_hEntity->GetTotalHealth()), 3, Color(100, 255, 100));

	CStructure* pStructure = dynamic_cast<CStructure*>(m_hEntity.GetPointer());
	if (pStructure && (pStructure->IsConstructing() || pStructure->IsInstalling() || pStructure->IsUpgrading()))
	{
		int iProductionRemaining = 0;
		int iTotalProduction = 0;
		if (pStructure->IsConstructing())
		{
			iProductionRemaining = pStructure->GetProductionToConstruct();
			iTotalProduction = pStructure->ConstructionCost();
		}
		else if (pStructure->IsInstalling())
		{
			iProductionRemaining = pStructure->GetProductionToInstall();
			iTotalProduction = pStructure->GetUpdateInstalling()->m_iProductionToInstall;
		}
		else if (pStructure->IsUpgrading())
		{
			iProductionRemaining = pStructure->GetProductionToUpgrade();
			iTotalProduction = pStructure->UpgradeCost();
		}

		int iProduction = iTotalProduction-iProductionRemaining;
		glgui::CRootPanel::PaintRect(x+h, y+h-1, (int)((float)h*iProduction/iTotalProduction), 3, Color(255, 255, 100));
	}
}

void CSceneTreeUnit::Selected()
{
	if (m_hEntity != NULL)
	{
		CDigitanksTeam* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

		if (DigitanksWindow()->IsShiftDown())
			pCurrentLocalTeam->AddToSelection(m_hEntity);
		else
		{
			pCurrentLocalTeam->SetPrimarySelection(m_hEntity);
			DigitanksGame()->GetDigitanksCamera()->SetTarget(m_hEntity->GetOrigin());
		}
	}

	BaseClass::Selected();
}

CSceneTreeNode::CSceneTreeNode(glgui::CTreeNode* pParent, glgui::CTree* pTree)
	: glgui::CTreeNode(pParent, pTree, L"")
{
}

void CSceneTreeNode::Paint(int x, int y, int w, int h, bool bFloating)
{
	BaseClass::Paint(x, y, w, h, bFloating);
}
