#include "scenetree.h"

#include <game/gameserver.h>
#include <textures/materiallibrary.h>
#include <renderer/game_renderer.h>
#include <ui/instructor.h>
#include <glgui/rootpanel.h>
#include <renderer/game_renderingcontext.h>

#include <digitanksgame.h>
#include "digitankswindow.h"
#include "hud.h"
#include <dt_camera.h>

using namespace glgui;

CSceneTree::CSceneTree()
	: CTree(CMaterialLibrary::AddMaterial("textures/hud/arrow.mat"))
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

	CDigitanksPlayer* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();

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

		if (pEntity->GetDigitanksPlayer() != pCurrentLocalTeam)
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
			pUnit->SetTooltip(tstring("Structure: ") + pEntity->GetEntityName());
		else
			pUnit->SetTooltip(tstring("Unit: ") + pEntity->GetEntityName());
		pTreeGroup->AddNode(pUnit);
	}

	Layout();
}

CSceneTreeGroup* CSceneTree::GetUnitNode(unittype_t eUnit)
{
	for (size_t i = 0; i < m_ahNodes.size(); i++)
	{
		CSceneTreeGroup* pTreeGroup = m_ahNodes[i].Downcast<CSceneTreeGroup>();
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
	for (size_t i = 0; i < m_ahNodes.size(); i++)
	{
		CSceneTreeGroup* pTreeGroup = m_ahNodes[i].Downcast<CSceneTreeGroup>();
		if (eUnit < pTreeGroup->GetUnitType())
		{
			CSceneTreeGroup* pReturn = new CSceneTreeGroup(eUnit, this);
			AddNode(pReturn, i);
			return pReturn;
		}
	}

	CSceneTreeGroup* pReturn = new CSceneTreeGroup(eUnit, this);

	if (pReturn->GetUnitType() == STRUCTURE_CPU)
		pReturn->SetTooltip("Group: Structures");
	else
		pReturn->SetTooltip("Group: Units");

	AddNode(pReturn);
	return pReturn;
}

void CSceneTree::Paint(float x, float y, float w, float h)
{
	glgui::CRootPanel::PaintRect(x, y, w, h, m_clrBackground);

	if (m_iHilighted != ~0)
	{
		CControlHandle pNode = m_apControls[m_iHilighted];
		float cx, cy, cw, ch;
		pNode->GetAbsDimensions(cx, cy, cw, ch);
		glgui::CRootPanel::PaintRect(cx+ch, cy, ch, ch, Color(255, 255, 255, 100));
	}

	// Skip CTree, we override its hilight and selection methods.
	CPanel::Paint(x, y, w, h);
}

void CSceneTree::OnAddUnitToTeam(CDigitanksPlayer* pTeam, CBaseEntity* pEntity)
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
		pUnit->SetTooltip(tstring("Structure: ") + pSelectable->GetEntityName());
	else
		pUnit->SetTooltip(tstring("Unit: ") + pSelectable->GetEntityName());

	pTreeGroup->AddNode(pUnit);

	Layout();
}

void CSceneTree::OnRemoveUnitFromTeam(CDigitanksPlayer* pTeam, CBaseEntity* pEntity)
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

	for (size_t i = 0; i < pTreeGroup->m_ahNodes.size(); i++)
	{
		CSceneTreeUnit* pUnit = pTreeGroup->m_ahNodes[i].Downcast<CSceneTreeUnit>();
		if (!pUnit)
			continue;

		if (pUnit->GetEntity() == pSelectable)
		{
			RemoveNode(pUnit);

			if (pTreeGroup->m_ahNodes.size() == 0)
				RemoveNode(pTreeGroup);

			break;
		}
	}

	Layout();
}

void CSceneTree::OnTeamMembersUpdated()
{
	BuildTree(true);
}

void CSceneTree::GetUnitDimensions(CDigitanksEntity* pEntity, float& x, float& y, float& w, float& h)
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

void CSceneTreeGroup::Paint(float x, float y, float w, float h, bool bFloating)
{
	if (!IsVisible())
		return;

	CDigitanksPlayer* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();

	CRenderingContext c(GameServer()->GetRenderer(), true);
	c.SetBlend(BLEND_ALPHA);

	CHUD::PaintUnitSheet(m_eUnit, x+h, y, h, h, pCurrentLocalTeam?pCurrentLocalTeam->GetColor():Color(255,255,255,255));

	BaseClass::Paint(x, y, w, h, bFloating);
}

void CSceneTreeGroup::GetUnitDimensions(CDigitanksEntity* pEntity, float& x, float& y, float& w, float& h)
{
	for (size_t i = 0; i < m_ahNodes.size(); i++)
	{
		CSceneTreeUnit* pUnit = m_ahNodes[i].Downcast<CSceneTreeUnit>();
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

void CSceneTreeUnit::Paint(float x, float y, float w, float h, bool bFloating)
{
	if (!IsVisible())
		return;

	if (!m_hEntity)
		return;

	CRenderingContext c(GameServer()->GetRenderer(), true);
	c.SetBlend(BLEND_ALPHA);

	Color clrTeam = m_hEntity->GetPlayerOwner()?m_hEntity->GetPlayerOwner()->GetColor():Color(255,255,255,255);

	CDigitank* pTank = dynamic_cast<CDigitank*>(m_hEntity.GetPointer());
	if (pTank && !pTank->NeedsOrders())
	{
		clrTeam = (clrTeam/2+Color(128,128,128,255)/2);
		clrTeam.SetAlpha(100);
	}

	CDigitanksPlayer* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();

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
			iTotalTurns = pStructure->GetTurnsToConstruct(pStructure->GetGlobalOrigin());
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
	if (DigitanksGame()->IsFeatureDisabled(DISABLE_SELECT))
		return;

	CDigitanksPlayer* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();

	if (m_hEntity != NULL && pCurrentLocalTeam)
	{

		if (DigitanksWindow()->IsShiftDown())
			pCurrentLocalTeam->AddToSelection(m_hEntity);
		else
		{
			pCurrentLocalTeam->SetPrimarySelection(m_hEntity);
			DigitanksGame()->GetOverheadCamera()->SetTarget(m_hEntity->GetGlobalOrigin());
		}

		DigitanksWindow()->GetInstructor()->FinishedLesson("artillery-select", true);
		if (m_hEntity->GetUnitType() == UNIT_MOBILECPU)
			DigitanksWindow()->GetInstructor()->FinishedLesson("strategy-select", true);
	}

	BaseClass::Selected();
}

CSceneTreeNode::CSceneTreeNode(glgui::CTreeNode* pParent, glgui::CTree* pTree)
	: glgui::CTreeNode(pParent, pTree, "", "sans-serif")
{
}

void CSceneTreeNode::Paint(float x, float y, float w, float h, bool bFloating)
{
	BaseClass::Paint(x, y, w, h, bFloating);
}
