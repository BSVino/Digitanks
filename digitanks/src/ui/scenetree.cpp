#include "scenetree.h"

#include <game/gameserver.h>
#include <models/texturelibrary.h>
#include <renderer/renderer.h>

#include <game/digitanks/digitanksgame.h>
#include "digitankswindow.h"
#include "hud.h"
#include <game/digitanks/dt_camera.h>
#include "instructor.h"

CSceneTree::CSceneTree()
	: CTree(CTextureLibrary::AddTextureID(_T("textures/hud/arrow.png"), 0, 0)
{
}

void CSceneTree::Layout()
{
	SetSize(90, glgui::CRootPanel::Get()->GetHeight()-300);
	SetPos(15, 60);

	BaseClass::Layout();
}

void CSceneTree::BuildTree(bool bForce)
{
	if (!DigitanksGame())
		return;

	CDigitanksTeam* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	if (!pCurrentLocalTeam)
		return;

	if (pCurrentLocalTeam == m_hTeam && !bForce)
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

		if (eUnit == STRUCTURE_BUFFER)
			continue;

		if (eUnit == STRUCTURE_MINIBUFFER)
			continue;

		if (eUnit == STRUCTURE_BATTERY)
			continue;

		if (eUnit == STRUCTURE_PSU)
			continue;

		CSceneTreeGroup* pTreeGroup = GetUnitNode(eUnit);

		CSceneTreeUnit* pUnit = new CSceneTreeUnit(pEntity, pTreeGroup, this);
		if (pTreeGroup->GetUnitType() == STRUCTURE_CPU)
			pUnit->SetTooltip(tstring(_T("Structure: ") + pEntity->GetEntityName());
		else
			pUnit->SetTooltip(tstring(_T("Unit: ") + pEntity->GetEntityName());
		pTreeGroup->AddNode(pUnit);
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

		if (pTreeGroup->GetUnitType() == STRUCTURE_CPU)
		{
			if (eUnit == STRUCTURE_INFANTRYLOADER)
				return pTreeGroup;
			if (eUnit == STRUCTURE_TANKLOADER)
				return pTreeGroup;
			if (eUnit == STRUCTURE_ARTILLERYLOADER)
				return pTreeGroup;
		}
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

	if (pReturn->GetUnitType() == STRUCTURE_CPU)
		pReturn->SetTooltip(_T("Group: Structures");
	else
		pReturn->SetTooltip(_T("Group: Units");

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

	if (eUnit == STRUCTURE_BUFFER)
		return;

	if (eUnit == STRUCTURE_MINIBUFFER)
		return;

	if (eUnit == STRUCTURE_BATTERY)
		return;

	if (eUnit == STRUCTURE_PSU)
		return;

	if (eUnit == STRUCTURE_FIREWALL)
		return;

	CSceneTreeGroup* pTreeGroup = GetUnitNode(eUnit);

	CSceneTreeUnit* pUnit = new CSceneTreeUnit(pSelectable, pTreeGroup, this);

	if (pTreeGroup->GetUnitType() == STRUCTURE_CPU)
		pUnit->SetTooltip(tstring(_T("Structure: ") + pSelectable->GetEntityName());
	else
		pUnit->SetTooltip(tstring(_T("Unit: ") + pSelectable->GetEntityName());

	pTreeGroup->AddNode(pUnit);

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

	if (eUnit == STRUCTURE_BUFFER)
		return;

	if (eUnit == STRUCTURE_MINIBUFFER)
		return;

	if (eUnit == STRUCTURE_BATTERY)
		return;

	if (eUnit == STRUCTURE_PSU)
		return;

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

void CSceneTree::OnTeamMembersUpdated()
{
	BuildTree(true);
}

void CSceneTree::GetUnitDimensions(CDigitanksEntity* pEntity, int& x, int& y, int& w, int& h)
{
	if (!pEntity)
		return;

	CSceneTreeGroup* pGroup = GetUnitNode(pEntity->GetUnitType());

	if (!pGroup)
		return;

	pGroup->GetUnitDimensions(pEntity, x, y, w, h);
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

	CDigitanksTeam* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	CRenderingContext c(GameServer()->GetRenderer());
	c.SetBlend(BLEND_ALPHA);

	CHUD::PaintUnitSheet(m_eUnit, x+h, y, h, h, pCurrentLocalTeam?pCurrentLocalTeam->GetColor():Color(255,255,255,255));

	BaseClass::Paint(x, y, w, h, bFloating);
}

void CSceneTreeGroup::GetUnitDimensions(CDigitanksEntity* pEntity, int& x, int& y, int& w, int& h)
{
	for (size_t i = 0; i < m_apNodes.size(); i++)
	{
		CSceneTreeUnit* pUnit = dynamic_cast<CSceneTreeUnit*>(m_apNodes[i]);
		if (!pUnit)
			continue;

		if (pUnit->GetEntity() == pEntity)
		{
			pUnit->GetAbsDimensions(x, y, w, h);
			return;
		}
	}

	TAssert(!"Can't find dimensions for a unit");
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

	if (m_hEntity == NULL)
		return;

	CRenderingContext c(GameServer()->GetRenderer());
	c.SetBlend(BLEND_ALPHA);

	Color clrTeam = m_hEntity->GetTeam()?m_hEntity->GetTeam()->GetColor():Color(255,255,255,255);

	CDigitank* pTank = dynamic_cast<CDigitank*>(m_hEntity.GetPointer());
	if (pTank && !pTank->NeedsOrders())
	{
		clrTeam = (clrTeam/2+Color(128,128,128,255)/2);
		clrTeam.SetAlpha(100);
	}

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

	glgui::CRootPanel::PaintRect(x+h, y - 3, (int)(h*m_hEntity->GetHealth()/m_hEntity->GetTotalHealth()), 3, Color(100, 255, 100));

	if (pTank)
	{
		float flAttackPower = pTank->GetBaseAttackPower(true);
		float flDefensePower = pTank->GetBaseDefensePower(true);
		float flMovementPower = pTank->GetUsedMovementEnergy(true);
		float flTotalPower = pTank->GetStartingPower();
		flAttackPower = flAttackPower/flTotalPower;
		flDefensePower = flDefensePower/flTotalPower;
		flMovementPower = flMovementPower/pTank->GetMaxMovementEnergy();

		if (flMovementPower > 0.1f)
		{
			size_t iSheetWidth = CHUD::GetHUDSheet().GetSheetWidth("MoveIcon");
			size_t iSheetHeight = CHUD::GetHUDSheet().GetSheetHeight("MoveIcon");
			const Rect& rArea = CHUD::GetHUDSheet().GetArea("MoveIcon");
			glgui::CRootPanel::PaintSheet(CHUD::GetHUDSheet().GetSheet("MoveIcon"), x+h+h+2 - 18, y+h+2 - 14, 16, 12, rArea.x, rArea.y, rArea.w, rArea.h, iSheetWidth, iSheetHeight);
		}

		if (flAttackPower)
			glgui::CRootPanel::PaintRect(x+h, y, (int)(h*flAttackPower), 3, Color(255, 0, 0));
		glgui::CRootPanel::PaintRect(x+h+(int)(h*flAttackPower), y, (int)(h*flDefensePower), 3, Color(0, 0, 255));
		if (flMovementPower)
			glgui::CRootPanel::PaintRect(x+h, y + 3, (int)(h*flMovementPower), 3, Color(255, 255, 0));

		size_t iSize = 22;
		int iAlpha = 20;
		if (pTank->HasFiredWeapon())
			iAlpha = 255;

		CHUD::PaintWeaponSheet(pTank->GetCurrentWeapon(), x+h+h+4, y+4, iSize, iSize, Color(255, 255, 255, iAlpha));

		size_t iX = x+h+h+h+4;

		if (pTank->IsFortified() || pTank->IsFortifying())
		{
			size_t iSheetWidth = CHUD::GetButtonSheet().GetSheetWidth("Fortify");
			size_t iSheetHeight = CHUD::GetButtonSheet().GetSheetHeight("Fortify");
			const Rect& rArea = CHUD::GetButtonSheet().GetArea("Fortify");
			glgui::CRootPanel::PaintSheet(CHUD::GetButtonSheet().GetSheet("Fortify"), iX, y+4, iSize, iSize, rArea.x, rArea.y, rArea.w, rArea.h, iSheetWidth, iSheetHeight);
			iX += h;
		}

		if (pTank->IsSentried())
		{
			size_t iSheetWidth = CHUD::GetButtonSheet().GetSheetWidth("Sentry");
			size_t iSheetHeight = CHUD::GetButtonSheet().GetSheetHeight("Sentry");
			const Rect& rArea = CHUD::GetButtonSheet().GetArea("Sentry");
			glgui::CRootPanel::PaintSheet(CHUD::GetButtonSheet().GetSheet("Sentry"), iX, y+4, iSize, iSize, rArea.x, rArea.y, rArea.w, rArea.h, iSheetWidth, iSheetHeight);
			iX += h;
		}

		if (pTank->HasGoalMovePosition())
		{
			size_t iSheetWidth = CHUD::GetButtonSheet().GetSheetWidth("Move");
			size_t iSheetHeight = CHUD::GetButtonSheet().GetSheetHeight("Move");
			const Rect& rArea = CHUD::GetButtonSheet().GetArea("Move");
			glgui::CRootPanel::PaintSheet(CHUD::GetButtonSheet().GetSheet("Move"), iX, y+4, iSize, iSize, rArea.x, rArea.y, rArea.w, rArea.h, iSheetWidth, iSheetHeight);
			iX += h;
		}
	}

	CStructure* pStructure = dynamic_cast<CStructure*>(m_hEntity.GetPointer());
	if (pStructure && (pStructure->IsConstructing() || pStructure->IsUpgrading()))
	{
		int iTurnsProgressed = 0;
		int iTotalTurns = 0;
		if (pStructure->IsConstructing())
		{
			iTotalTurns = pStructure->GetTurnsToConstruct(pStructure->GetOrigin());
			iTurnsProgressed = iTotalTurns-pStructure->GetTurnsRemainingToConstruct();
		}
		else if (pStructure->IsUpgrading())
		{
			iTotalTurns = pStructure->GetTurnsToUpgrade();
			iTurnsProgressed = iTotalTurns-pStructure->GetTurnsRemainingToUpgrade();
		}

		iTurnsProgressed++;
		iTotalTurns++;

		glgui::CRootPanel::PaintRect(x+h, y+h-1, (int)((float)h*iTurnsProgressed/iTotalTurns), 3, Color(255, 255, 100));
	}

	BaseClass::Paint(x, y, w, h, bFloating);
}

void CSceneTreeUnit::Selected()
{
	if (DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_SELECT))
		return;

	CDigitanksTeam* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	if (m_hEntity != NULL && pCurrentLocalTeam)
	{

		if (DigitanksWindow()->IsShiftDown())
			pCurrentLocalTeam->AddToSelection(m_hEntity);
		else
		{
			pCurrentLocalTeam->SetPrimarySelection(m_hEntity);
			DigitanksGame()->GetDigitanksCamera()->SetTarget(m_hEntity->GetOrigin());
		}

		DigitanksWindow()->GetInstructor()->FinishedTutorial("artillery-select", true);
		if (m_hEntity->GetUnitType() == UNIT_MOBILECPU)
			DigitanksWindow()->GetInstructor()->FinishedTutorial("strategy-select", true);
	}

	BaseClass::Selected();
}

CSceneTreeNode::CSceneTreeNode(glgui::CTreeNode* pParent, glgui::CTree* pTree)
	: glgui::CTreeNode(pParent, pTree, _T("", _T("sans-serif")
{
}

void CSceneTreeNode::Paint(int x, int y, int w, int h, bool bFloating)
{
	BaseClass::Paint(x, y, w, h, bFloating);
}
