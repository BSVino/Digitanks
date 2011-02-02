#include "hud.h"

#include <GL/glew.h>

#include <geometry.h>
#include <strutils.h>

#include <tinker/cvar.h>

#include "digitankswindow.h"
#include "digitanks/digitanksgame.h"
#include "debugdraw.h"
#include "instructor.h"
#include "renderer/renderer.h"
#include <game/digitanks/structures/cpu.h>
#include <game/digitanks/weapons/projectile.h>
#include <game/digitanks/structures/loader.h>
#include <game/digitanks/structures/props.h>
#include <game/digitanks/units/mobilecpu.h>
#include <game/digitanks/dt_camera.h>
#include <sound/sound.h>
#include "weaponpanel.h"
#include "scenetree.h"

using namespace glgui;

CPowerBar::CPowerBar(powerbar_type_t ePowerbarType)
	: CLabel(0, 0, 100, 100, L"")
{
	m_ePowerbarType = ePowerbarType;
}

void CPowerBar::Think()
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelection())
		return;

	CSelectable* pSelection = DigitanksGame()->GetPrimarySelection();

	char szLabel[100];
	if (m_ePowerbarType == POWERBAR_HEALTH)
	{
		if (pSelection->TakesDamage())
		{
			sprintf(szLabel, "Hull Strength: %.1f/%.1f", pSelection->GetHealth(), pSelection->GetTotalHealth());
			SetText(szLabel);
		}
		else
			SetText("");
	}
	else if (m_ePowerbarType == POWERBAR_ATTACK)
	{
		if (strlen(pSelection->GetPowerBar1Text()))
		{
			sprintf(szLabel, "%s: %d%%", pSelection->GetPowerBar1Text(), (int)(pSelection->GetPowerBar1Value()*100));
			SetText(szLabel);
		}
		else
			SetText("");
	}
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
	{
		if (strlen(pSelection->GetPowerBar2Text()))
		{
			sprintf(szLabel, "%s: %d%%", pSelection->GetPowerBar2Text(), (int)(pSelection->GetPowerBar2Value()*100));
			SetText(szLabel);
		}
		else
			SetText("");
	}
	else
	{
		if (strlen(pSelection->GetPowerBar3Text()))
		{
			sprintf(szLabel, "%s: %d%%", pSelection->GetPowerBar3Text(), (int)(pSelection->GetPowerBar3Value()*100));
			SetText(szLabel);
		}
		else
			SetText("");
	}
}

void CPowerBar::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelection())
		return;

	CSelectable* pSelection = DigitanksGame()->GetPrimarySelection();

	if (m_ePowerbarType == POWERBAR_HEALTH)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetHealth() / pSelection->GetTotalHealth())-2, h-2, Color(0, 150, 0));
	else if (m_ePowerbarType == POWERBAR_ATTACK)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetPowerBar1Size())-2, h-2, Color(150, 0, 0));
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetPowerBar2Size())-2, h-2, Color(0, 0, 150));
	else
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetPowerBar3Size())-2, h-2, Color(100, 100, 0));

	BaseClass::Paint(x, y, w, h);
}

CHUD::CHUD()
	: CPanel(0, 0, 100, 100)
{
	m_bNeedsUpdate = false;

	m_pHealthBar = new CPowerBar(POWERBAR_HEALTH);
	m_pAttackPower = new CPowerBar(POWERBAR_ATTACK);
	m_pDefensePower = new CPowerBar(POWERBAR_DEFENSE);
	m_pMovementPower = new CPowerBar(POWERBAR_MOVEMENT);

	AddControl(m_pHealthBar);
	AddControl(m_pAttackPower);
	AddControl(m_pDefensePower);
	AddControl(m_pMovementPower);

	m_iHUDSheet = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-sheet.png");
	m_iUnitsSheet = CRenderer::LoadTextureIntoGL(L"textures/hud/units-sheet.png");
	m_iWeaponsSheet = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-weapons-01.png");
	m_iKeysSheet = CRenderer::LoadTextureIntoGL(L"textures/hud/keys.png");
	m_iObstruction = CRenderer::LoadTextureIntoGL(L"textures/hud/hud-obstruction.png");
	m_iActionTanksSheet = CRenderer::LoadTextureIntoGL(L"textures/hud/actionsigns/tanks.png");
	m_iActionSignsSheet = CRenderer::LoadTextureIntoGL(L"textures/hud/actionsigns/signs.png");

	m_eActionSign = ACTIONSIGN_NONE;

	m_pActionItem = new CLabel(0, 0, 10, 10, L"");
	m_pCloseActionItems = new CButton(0, 0, 100, 50, L"Close");
	AddControl(m_pActionItem);
	AddControl(m_pCloseActionItems);
	m_flActionItemsLerp = m_flActionItemsLerpGoal = 0;
	m_flActionItemsWidth = 280;

	m_pButtonPanel = new CMouseCapturePanel();
	AddControl(m_pButtonPanel);

	for (size_t i = 0; i < NUM_BUTTONS; i++)
	{
		m_apButtons[i] = new CPictureButton(L"");
		m_apButtons[i]->SetCursorOutListener(this, ButtonCursorOut);
		m_pButtonPanel->AddControl(m_apButtons[i]);
	}

	m_apButtons[0]->SetCursorInListener(this, ButtonCursorIn0);
	m_apButtons[1]->SetCursorInListener(this, ButtonCursorIn1);
	m_apButtons[2]->SetCursorInListener(this, ButtonCursorIn2);
	m_apButtons[3]->SetCursorInListener(this, ButtonCursorIn3);
	m_apButtons[4]->SetCursorInListener(this, ButtonCursorIn4);
	m_apButtons[5]->SetCursorInListener(this, ButtonCursorIn5);
	m_apButtons[6]->SetCursorInListener(this, ButtonCursorIn6);
	m_apButtons[7]->SetCursorInListener(this, ButtonCursorIn7);
	m_apButtons[8]->SetCursorInListener(this, ButtonCursorIn8);
	m_apButtons[9]->SetCursorInListener(this, ButtonCursorIn9);

	m_pFireAttack = new CLabel(0, 0, 50, 50, L"");
	m_pFireDefend = new CLabel(0, 0, 50, 50, L"");
	AddControl(m_pFireAttack);
	AddControl(m_pFireDefend);

	m_pAttackInfo = new CLabel(0, 0, 100, 150, L"");
	m_pAttackInfo->SetWrap(false);
	m_pAttackInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pAttackInfo);

	m_pScoreboard = new CLabel(0, 0, 100, 150, L"");
	m_pScoreboard->SetWrap(false);
	m_pScoreboard->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pScoreboard->SetFontFaceSize(10);
	AddControl(m_pScoreboard);

	m_pFrontShieldInfo = new CLabel(0, 0, 100, 100, L"");
	AddControl(m_pFrontShieldInfo);

	m_pRearShieldInfo = new CLabel(0, 0, 100, 100, L"");
	AddControl(m_pRearShieldInfo);

	m_pLeftShieldInfo = new CLabel(0, 0, 100, 100, L"");
	AddControl(m_pLeftShieldInfo);

	m_pRightShieldInfo = new CLabel(0, 0, 100, 100, L"");
	AddControl(m_pRightShieldInfo);

	m_pTankInfo = new CLabel(0, 0, 100, 100, L"");
	AddControl(m_pTankInfo);

	m_pTurnInfo = new CLabel(0, 0, 100, 100, L"");
	AddControl(m_pTurnInfo);

	m_pResearchInfo = new CLabel(0, 0, 100, 100, L"");
	AddControl(m_pResearchInfo);

	m_pButtonInfo = new CLabel(0, 0, 100, 100, L"");
	AddControl(m_pButtonInfo);

	m_pPressEnter = new CLabel(0, 0, 100, 100, L"");
	AddControl(m_pPressEnter);

	SetupMenu(MENUMODE_MAIN);

	m_pDemoNotice = new CLabel(0, 0, 100, 20, L"");
	AddControl(m_pDemoNotice);

	m_pDemoNotice->SetAlign(CLabel::TA_TOPLEFT);
	m_pDemoNotice->SetPos(20, 20);
	m_pDemoNotice->SetText("");

	m_pPowerInfo = new CLabel(0, 0, 200, 20, L"");
	AddControl(m_pPowerInfo);

	m_pPowerInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pPowerInfo->SetPos(200, 20);

	m_pFleetInfo = new CLabel(0, 0, 200, 20, L"");
	AddControl(m_pFleetInfo);

	m_pFleetInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pFleetInfo->SetPos(200, 20);

	m_pBandwidthInfo = new CLabel(0, 0, 200, 20, L"");
	AddControl(m_pBandwidthInfo);

	m_pBandwidthInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pBandwidthInfo->SetPos(200, 20);

	m_pUpdatesButton = new CPictureButton(L"Download Updates");
	m_pUpdatesButton->SetSheetTexture(m_iHUDSheet, 500, 710, 175, 40, 1024, 1024);
	m_pUpdatesButton->SetClickedListener(this, OpenUpdates);
	m_pUpdatesButton->ShowBackground(false);
	AddControl(m_pUpdatesButton);

	m_pUpdatesPanel = new CUpdatesPanel();
	m_pUpdatesPanel->SetVisible(false);
	AddControl(m_pUpdatesPanel, true);

	m_flUpdateIconSlide = 0;

	m_pWeaponPanel = new CWeaponPanel();
	m_pWeaponPanel->SetVisible(false);
	AddControl(m_pWeaponPanel, true);

	m_pSceneTree = new CSceneTree();
	AddControl(m_pSceneTree, true);

	m_pTurnButton = new CPictureButton(L"TURN");
	m_pTurnButton->SetSheetTexture(m_iHUDSheet, 0, 730, 160, 120, 1024, 1024);
	m_pTurnButton->SetClickedListener(this, EndTurn);
	m_pTurnButton->ShowBackground(false);
	AddControl(m_pTurnButton);

	m_flAttackInfoAlpha = m_flAttackInfoAlphaGoal = 0;

	m_flTurnInfoLerp = m_flTurnInfoLerpGoal = 0;
	m_flTurnInfoHeight = m_flTurnInfoHeightGoal = 0;

	m_iTurnSound = CSoundLibrary::Get()->AddSound(L"sound/turn.wav");

	m_pSpacebarHint = new CLabel(0, 0, 200, 20, L"");
	m_pSpacebarHint->SetAlign(CLabel::TA_MIDDLECENTER);
	AddControl(m_pSpacebarHint);

	//m_iCompetitionWatermark = CRenderer::LoadTextureIntoGL(L"textures/competition.png");
}

void CHUD::Layout()
{
	SetSize(GetParent()->GetWidth(), GetParent()->GetHeight());

	int iWidth = DigitanksWindow()->GetWindowWidth();
	int iHeight = DigitanksWindow()->GetWindowHeight();

	m_pAttackInfo->SetPos(iWidth - 165, iHeight - 150 - 90 - 10);
	m_pAttackInfo->SetSize(165, 90);

	m_pHealthBar->SetPos(iWidth/2 - 720/2 + 170, iHeight - 140);
	m_pHealthBar->SetSize(200, 20);

	m_pAttackPower->SetPos(iWidth/2 - 720/2 + 170, iHeight - 90);
	m_pAttackPower->SetSize(200, 20);

	m_pDefensePower->SetPos(iWidth/2 - 720/2 + 170, iHeight - 60);
	m_pDefensePower->SetSize(200, 20);

	m_pMovementPower->SetPos(iWidth/2 - 720/2 + 170, iHeight - 30);
	m_pMovementPower->SetSize(200, 20);

	m_pActionItem->SetPos(iWidth - 280, 70);
	m_pActionItem->SetSize(220, 250);
	m_pActionItem->SetAlign(CLabel::TA_TOPLEFT);
	m_pCloseActionItems->SetSize(130, 25);
	m_pCloseActionItems->SetPos(iWidth - 225, 341);
	m_pCloseActionItems->SetClickedListener(this, CloseActionItems);

	for (size_t i = 0; i < m_apActionItemButtons.size(); i++)
	{
		RemoveControl(m_apActionItemButtons[i]);
		m_apActionItemButtons[i]->Destructor();
		m_apActionItemButtons[i]->Delete();
	}

	m_apActionItemButtons.clear();

	size_t iItemButtonSize = 30;
	for (size_t i = 0; i < DigitanksGame()->GetActionItems().size(); i++)
	{
		CPictureButton* pButton = new CPictureButton(sprintf(L"%d", i));
		AddControl(pButton);
		pButton->SetSize(iItemButtonSize, iItemButtonSize);
		pButton->SetPos(iWidth - iItemButtonSize - 10, 50 + (iItemButtonSize+10)*i);
		pButton->SetClickedListener(this, ChooseActionItem);

		CEntityHandle<CDigitanksEntity> hUnit(DigitanksGame()->GetActionItems()[i].iUnit);

		switch (DigitanksGame()->GetActionItems()[i].eActionType)
		{
		case ACTIONTYPE_WELCOME:
			// Use the fleet logo, which is also the digitanks logo, for the welcome icon.
			pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iHUDSheet, 540, 530, 20, 20, 1024, 1024);
			break;

		case ACTIONTYPE_CONTROLS:
			// Use the fleet logo, which is also the digitanks logo, for the welcome icon.
			pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iHUDSheet, 540, 530, 20, 20, 1024, 1024);
			break;

		case ACTIONTYPE_NEWSTRUCTURE:
			if (hUnit != NULL)
			{
				int sx, sy, sw, sh, tw, th;
				GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
				pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iUnitsSheet, sx, sy, sw, sh, tw, th);
			}
			break;

		case ACTIONTYPE_AUTOMOVECANCELED:
			if (hUnit != NULL)
			{
				int sx, sy, sw, sh, tw, th;
				GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
				pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iUnitsSheet, sx, sy, sw, sh, tw, th);
			}
			break;

		case ACTIONTYPE_AUTOMOVEENEMY:
			if (hUnit != NULL)
			{
				int sx, sy, sw, sh, tw, th;
				GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
				pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iUnitsSheet, sx, sy, sw, sh, tw, th);
			}
			break;

		case ACTIONTYPE_UNITDAMAGED:
			if (hUnit != NULL)
			{
				int sx, sy, sw, sh, tw, th;
				GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
				pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iUnitsSheet, sx, sy, sw, sh, tw, th);
			}
			break;

		case ACTIONTYPE_FORTIFIEDENEMY:
			if (hUnit != NULL)
			{
				int sx, sy, sw, sh, tw, th;
				GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
				pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iUnitsSheet, sx, sy, sw, sh, tw, th);
			}
			break;

		case ACTIONTYPE_UNITAUTOMOVE:
			if (hUnit != NULL)
			{
				int sx, sy, sw, sh, tw, th;
				GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
				pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iUnitsSheet, sx, sy, sw, sh, tw, th);
			}
			break;

		case ACTIONTYPE_UNITORDERS:
			if (hUnit != NULL)
			{
				int sx, sy, sw, sh, tw, th;
				GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
				pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iUnitsSheet, sx, sy, sw, sh, tw, th);
			}
			break;

		case ACTIONTYPE_UPGRADE:
			if (hUnit != NULL)
			{
				int sx, sy, sw, sh, tw, th;
				GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
				pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iUnitsSheet, sx, sy, sw, sh, tw, th);
			}
			break;

		case ACTIONTYPE_UNITREADY:
			if (hUnit != NULL)
			{
				int sx, sy, sw, sh, tw, th;
				GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
				pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iUnitsSheet, sx, sy, sw, sh, tw, th);
			}
			break;

		case ACTIONTYPE_DOWNLOADUPDATES:
			pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iHUDSheet, 520, 530, 20, 20, 1024, 1024);
			break;

		case ACTIONTYPE_DOWNLOADCOMPLETE:
			pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_iHUDSheet, 520, 530, 20, 20, 1024, 1024);
			break;
		}

		m_apActionItemButtons.push_back(pButton);
	}

	m_pButtonPanel->SetPos(iWidth/2 - 720/2 + 380, iHeight - 140);
	m_pButtonPanel->SetRight(m_pButtonPanel->GetLeft() + 330);
	m_pButtonPanel->SetBottom(iHeight - 10);

	for (size_t i = 0; i < NUM_BUTTONS; i++)
	{
		m_apButtons[i]->SetSize(50, 50);
		m_apButtons[i]->SetPos(20 + 60*(i%5), 10 + 60*(i/5));
	}

	m_pLeftShieldInfo->SetDimensions(iWidth/2 - 720/2 + 10 + 150/2 - 50/2 - 40, iHeight - 150 + 10 + 130/2 - 50/2, 10, 50);
	m_pRightShieldInfo->SetDimensions(iWidth/2 - 720/2 + 10 + 150/2 + 50/2 + 30, iHeight - 150 + 10 + 130/2 - 50/2, 10, 50);
	m_pRearShieldInfo->SetDimensions(iWidth/2 - 720/2 + 10 + 150/2 - 50/2, iHeight - 150 + 10 + 130/2 + 50/2 + 25, 50, 10);
	m_pFrontShieldInfo->SetDimensions(iWidth/2 - 720/2 + 10 + 150/2 - 50/2, iHeight - 150 + 10 + 130/2 - 50/2 - 35, 50, 10);

	m_pLeftShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pRightShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pRearShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pFrontShieldInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);

	m_pLeftShieldInfo->SetWrap(false);
	m_pRightShieldInfo->SetWrap(false);
	m_pRearShieldInfo->SetWrap(false);
	m_pFrontShieldInfo->SetWrap(false);

	m_pTankInfo->SetSize(140, 240);
	m_pTankInfo->SetPos(10, iHeight - m_pTankInfo->GetHeight() + 10 + 7);
	m_pTankInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pTankInfo->SetWrap(true);
	m_pTankInfo->SetFontFaceSize(10);

	m_pTurnInfo->SetSize(248, 150);
	m_pTurnInfo->SetPos(iWidth/2 - m_pTurnInfo->GetWidth()/2, 36);
	m_pTurnInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pTurnInfo->SetWrap(true);
	m_pTurnInfo->SetFontFaceSize(10);

	m_pResearchInfo->SetSize(640, 35);
	m_pResearchInfo->SetPos(iWidth/2 - m_pResearchInfo->GetWidth()/2, 0);
	m_pResearchInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pResearchInfo->SetWrap(false);

	CDigitanksTeam* pLocalCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (pLocalCurrentTeam)
	{
		CUpdateItem* pItem = pLocalCurrentTeam->GetUpdateDownloading();
		if (pItem)
		{
			if (pLocalCurrentTeam->GetBandwidth() == 0)
				m_pResearchInfo->SetText(pItem->GetName());
			else
			{
				eastl::string16 s;
				s.sprintf((pItem->GetName() + L" (%d)").c_str(), pLocalCurrentTeam->GetTurnsToDownload());
				m_pResearchInfo->SetText(s);
			}
		}
		else
			m_pResearchInfo->SetText("");
	}

	m_pButtonInfo->SetSize(250, 250);
	m_pButtonInfo->SetPos(iWidth/2, iHeight - 400);
	m_pButtonInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pButtonInfo->SetWrap(true);

	m_pUpdatesButton->SetSize(175, 40);
	m_pUpdatesButton->SetPos(m_pTurnInfo->GetLeft() - 20 - m_pUpdatesButton->GetWidth(), 36);
	m_pUpdatesButton->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pUpdatesButton->SetWrap(false);

	m_pUpdatesPanel->Layout();

	m_pWeaponPanel->Layout();

	m_pSceneTree->Layout();

	m_pPressEnter->SetDimensions(iWidth/2 - 100/2, iHeight*2/3, 100, 50);
	m_pPressEnter->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pPressEnter->SetWrap(false);

	m_pPowerInfo->SetAlign(CLabel::TA_TOPLEFT);
	m_pPowerInfo->SetPos(iWidth - 320, 20);
	m_pPowerInfo->SetWrap(false);

	m_pFleetInfo->SetAlign(CLabel::TA_TOPLEFT);
	m_pFleetInfo->SetPos(iWidth - 220, 20);
	m_pFleetInfo->SetWrap(false);

	m_pBandwidthInfo->SetAlign(CLabel::TA_TOPLEFT);
	m_pBandwidthInfo->SetPos(iWidth - 150, 20);
	m_pBandwidthInfo->SetWrap(false);

	m_pTurnButton->SetPos(iWidth - 140, iHeight - 105);
	m_pTurnButton->SetSize(140, 105);

	UpdateTeamInfo();
	UpdateInfo();
	SetupMenu();
}

void CHUD::Think()
{
	if (GameServer()->IsLoading())
		return;

	BaseClass::Think();

	CDigitank* pCurrentTank = DigitanksGame()->GetPrimarySelectionTank();

	Vector vecTerrainPoint, vecEntityPoint;
	bool bMouseOnGrid = false;
	CBaseEntity* pHit = NULL;
	if (DigitanksGame()->GetControlMode() == MODE_NONE)
	{
		bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecEntityPoint, &pHit);

		if (pHit && dynamic_cast<CSelectable*>(pHit))
			DigitanksWindow()->SetMouseCursor(MOUSECURSOR_SELECT);
	}
	else
	{
		bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecEntityPoint, &pHit);
		bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecTerrainPoint, NULL, CG_TERRAIN|CG_PROP);
	}

	if (pCurrentTank)
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		{
			float flMoveDistance = pCurrentTank->GetMaxMovementDistance();
			if ((vecTerrainPoint - pCurrentTank->GetOrigin()).LengthSqr() > flMoveDistance*flMoveDistance)
				DigitanksWindow()->SetMouseCursor(MOUSECURSOR_MOVEAUTO);
			else
				DigitanksWindow()->SetMouseCursor(MOUSECURSOR_MOVE);
		}
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
		{
			CDigitanksEntity* pDTHit = dynamic_cast<CDigitanksEntity*>(pHit);
			CStaticProp* pSPHit = dynamic_cast<CStaticProp*>(pHit);
			if (pDTHit && pDTHit->TakesDamage() && pDTHit->GetTeam() != pCurrentTank->GetTeam() && pDTHit->GetVisibility() > 0.5f && !pSPHit)
				DigitanksWindow()->SetMouseCursor(MOUSECURSOR_AIMENEMY);
			else
				DigitanksWindow()->SetMouseCursor(MOUSECURSOR_AIM);
		}
		else if (DigitanksGame()->GetControlMode() == MODE_TURN)
		{
			DigitanksWindow()->SetMouseCursor(MOUSECURSOR_ROTATE);
		}
	}

	if (m_pUpdatesPanel->IsVisible())
	{
		DigitanksWindow()->SetMouseCursor(MOUSECURSOR_NONE);
	}

	m_flActionItemsLerp = Approach(m_flActionItemsLerpGoal, m_flActionItemsLerp, GameServer()->GetFrameTime());

	int iWidth = DigitanksWindow()->GetWindowWidth();
	m_pActionItem->SetPos(iWidth - 300 + (int)(Lerp(1-m_flActionItemsLerp, 0.2f) * m_flActionItemsWidth), 70);
	m_pCloseActionItems->SetPos(iWidth - 255 + (int)(Lerp(1-m_flActionItemsLerp, 0.2f) * m_flActionItemsWidth), m_pCloseActionItems->GetTop());

	if (m_bHUDActive && bMouseOnGrid && pCurrentTank)
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE || DigitanksGame()->GetControlMode() == MODE_TURN || DigitanksGame()->GetControlMode() == MODE_AIM)
			UpdateInfo();

		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		{
			Vector vecMove = vecTerrainPoint;
			vecMove.y = pCurrentTank->FindHoverHeight(vecMove);
			pCurrentTank->SetPreviewMove(vecMove);
		}

		if (DigitanksGame()->GetControlMode() == MODE_TURN)
		{
			if ((vecTerrainPoint - pCurrentTank->GetOrigin()).LengthSqr() > 4*4)
			{
				Vector vecTurn = vecTerrainPoint - pCurrentTank->GetOrigin();
				vecTurn.Normalize();
				float flTurn = atan2(vecTurn.z, vecTurn.x) * 180/M_PI;
				pCurrentTank->SetPreviewTurn(flTurn);
			}
			else
				pCurrentTank->SetPreviewTurn(pCurrentTank->GetAngles().y);
		}

		if (DigitanksGame()->GetControlMode() == MODE_AIM)
		{
			Vector vecPreviewAim;
			CDigitanksEntity* pDTHit = dynamic_cast<CDigitanksEntity*>(pHit);
			CStaticProp* pSPHit = dynamic_cast<CStaticProp*>(pHit);
			CDigitank* pDigitankHit = dynamic_cast<CDigitank*>(pHit);

			if (pDigitankHit && pDigitankHit->IsScout())
				// Scouts are hard to hit by projectiles because they float so far above the surface.
				vecPreviewAim = DigitanksGame()->GetTerrain()->SetPointHeight(pDigitankHit->GetOrigin());
			else if (pDTHit && !pSPHit && pDTHit->GetVisibility() > 0)
				vecPreviewAim = pDTHit->GetOrigin();
			else
				vecPreviewAim = vecEntityPoint;

			for (size_t i = 0; i < DigitanksGame()->GetCurrentTeam()->GetNumTanks(); i++)
			{
				CDigitank* pTank = DigitanksGame()->GetCurrentTeam()->GetTank(i);
				if (!pTank)
					continue;

				if (DigitanksGame()->GetCurrentTeam()->IsSelected(pTank))
				{
					if (pTank->GetCurrentWeapon() == WEAPON_CHARGERAM)
						pTank->SetPreviewCharge(pHit);
					else if (pTank->GetCurrentWeapon() == PROJECTILE_AIRSTRIKE)
						pTank->SetPreviewAim(vecTerrainPoint);
					else
						pTank->SetPreviewAim(vecPreviewAim);
				}
			}
		}
	}

	CStructure* pCurrentStructure = DigitanksGame()->GetPrimarySelectionStructure();
	if (m_bHUDActive && bMouseOnGrid && pCurrentStructure)
	{
		if (DigitanksGame()->GetControlMode() == MODE_BUILD)
		{
			CCPU* pCPU = dynamic_cast<CCPU*>(pCurrentStructure);
			if (pCPU)
				pCPU->SetPreviewBuild(vecTerrainPoint);
		}
	}

	for (size_t i = 0; i < DigitanksGame()->GetActionItems().size(); i++)
	{
		if (DigitanksGame()->GetActionItems()[i].bHandled)
			m_apActionItemButtons[i]->SetAlpha(100);
		else
			m_apActionItemButtons[i]->SetAlpha((int)RemapVal(Oscillate(GameServer()->GetGameTime(), 1), 0, 1, 100, 255));
	}

	if (m_eMenuMode == MENUMODE_MAIN)
	{
		if (m_bHUDActive && pCurrentTank && pCurrentTank->GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam() && DigitanksWindow()->GetInstructor()->GetCurrentTutorial() == CInstructor::TUTORIAL_INGAME_ARTILLERY_AIM)
		{
			float flRamp = Oscillate(GameServer()->GetGameTime(), 1);
			m_apButtons[2]->SetButtonColor(Color((int)RemapVal(flRamp, 0, 1, 0, 250), 0, 0));
		}
		else if (m_bHUDActive && pCurrentTank && pCurrentTank->GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam() && DigitanksWindow()->GetInstructor()->GetCurrentTutorial() == CInstructor::TUTORIAL_INGAME_STRATEGY_DEPLOY && dynamic_cast<CMobileCPU*>(pCurrentTank))
		{
			float flRamp = Oscillate(GameServer()->GetGameTime(), 1);
			m_apButtons[8]->SetButtonColor(Color(0, 0, (int)RemapVal(flRamp, 0, 1, 0, 250)));
		}
		// Don't blink other buttons if we're trying to blink this one.
		else
		{
			if (m_bHUDActive && pCurrentTank && pCurrentTank->GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam() && pCurrentTank->HasBonusPoints())
			{
				float flRamp = 1;
				if (!DigitanksWindow()->GetInstructor()->GetActive() || DigitanksWindow()->GetInstructor()->GetCurrentTutorial() >= CInstructor::TUTORIAL_UPGRADE)
					flRamp = Oscillate(GameServer()->GetGameTime(), 1);
				m_apButtons[4]->SetButtonColor(Color((int)RemapVal(flRamp, 0, 1, 0, 250), (int)RemapVal(flRamp, 0, 1, 0, 200), 0));
			}

			if (m_bHUDActive && pCurrentTank && pCurrentTank->GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam() && pCurrentTank->HasSpecialWeapons())
			{
				float flRamp = Oscillate(GameServer()->GetGameTime(), 1);
				m_apButtons[9]->SetButtonColor(Color((int)RemapVal(flRamp, 0, 1, 0, 250), (int)RemapVal(flRamp, 0, 1, 0, 200), 0));
			}
		}
	}

	if (pCurrentTank && !pCurrentTank->IsArtillery() && !pCurrentTank->IsScout())
	{
		m_pFireAttack->SetVisible(DigitanksGame()->GetControlMode() == MODE_AIM);
		m_pFireDefend->SetVisible(DigitanksGame()->GetControlMode() == MODE_AIM);
		m_pFireAttack->SetAlign(CLabel::TA_MIDDLECENTER);
		m_pFireDefend->SetAlign(CLabel::TA_MIDDLECENTER);
		m_pFireAttack->SetWrap(false);
		m_pFireDefend->SetWrap(false);
	}
	else
	{
		m_pFireAttack->SetVisible(false);
		m_pFireDefend->SetVisible(false);
	}

	if (m_pAttackInfo->GetText().length() && DigitanksGame()->GetControlMode() == MODE_AIM)
	{
		m_flAttackInfoAlphaGoal = 1.0f;
		m_pAttackInfo->SetVisible(true);
	}
	else
	{
		m_flAttackInfoAlphaGoal = 0.0f;
		m_pAttackInfo->SetVisible(false);
	}

	m_flAttackInfoAlpha = Approach(m_flAttackInfoAlphaGoal, m_flAttackInfoAlpha, GameServer()->GetFrameTime());

	m_flTurnInfoHeightGoal = m_pTurnInfo->GetTextHeight();
	m_flTurnInfoLerp = Approach(m_flTurnInfoLerpGoal, m_flTurnInfoLerp, GameServer()->GetFrameTime());
	m_flTurnInfoHeight = Approach(m_flTurnInfoHeightGoal, m_flTurnInfoHeight, GameServer()->GetFrameTime()*100);

	float flTurnInfoHeight = m_flTurnInfoHeight+10;
	m_pTurnInfo->SetSize(m_pTurnInfo->GetWidth(), (int)flTurnInfoHeight);
	m_pTurnInfo->SetPos(m_pTurnInfo->GetLeft(), 36 + 10 - (int)(Lerp(1.0f-m_flTurnInfoLerp, 0.2f)*flTurnInfoHeight));

	m_pUpdatesButton->SetVisible(!!DigitanksGame()->GetUpdateGrid());

	if (DigitanksWindow()->GetInstructor()->GetActive())
		m_bUpdatesBlinking = false;

	if (m_bUpdatesBlinking)
		m_pUpdatesButton->SetAlpha((int)(RemapVal(Oscillate(GameServer()->GetGameTime(), 1), 0, 1, 0.5f, 1)*255));
	else
		m_pUpdatesButton->SetAlpha(255);

	m_pScoreboard->SetVisible(DigitanksGame()->ShouldShowScores());

	if (m_bNeedsUpdate)
	{
		glgui::CRootPanel::Get()->Layout();
		m_bNeedsUpdate = false;
	}
}

void CHUD::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	if (GameServer()->IsLoading())
		return;

	if (DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	int iWidth = DigitanksWindow()->GetWindowWidth();
	int iHeight = DigitanksWindow()->GetWindowHeight();

	CDigitanksTeam* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);

		if (!pDTEntity)
			continue;

		if (pDTEntity->GetVisibility() == 0)
			continue;

		CSelectable* pSelectable = dynamic_cast<CSelectable*>(pEntity);

		if (!pSelectable)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);

		Vector vecOrigin = pEntity->GetOrigin();
		if (pTank)
			vecOrigin = pTank->GetOrigin();

		Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(vecOrigin);

		float flRadius = pDTEntity->GetBoundingRadius();

		Vector vecUp;
		Vector vecForward;
		GameServer()->GetRenderer()->GetCameraVectors(&vecForward, NULL, &vecUp);

		// Cull tanks behind the camera
		if (vecForward.Dot((vecOrigin-GameServer()->GetCamera()->GetCameraPosition()).Normalized()) < 0)
			continue;

		Vector vecTop = GameServer()->GetRenderer()->ScreenPosition(vecOrigin + vecUp*flRadius);
		float flWidth = (vecTop - vecScreen).Length()*2 + 10;

		if (pCurrentLocalTeam && pCurrentLocalTeam->IsSelected(pSelectable) && !IsUpdatesPanelOpen())
		{
			Color clrSelection(255, 255, 255, 128);
			if (pCurrentLocalTeam->IsPrimarySelection(pSelectable))
				clrSelection = Color(255, 255, 255, 255);

			CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y - flWidth/2), (int)flWidth, 1, clrSelection);
			CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y - flWidth/2), 1, (int)flWidth, clrSelection);
			CRootPanel::PaintRect((int)(vecScreen.x + flWidth/2), (int)(vecScreen.y - flWidth/2), 1, (int)flWidth, clrSelection);
			CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y + flWidth/2), (int)flWidth, 1, clrSelection);

			if (pTank)
			{
				CRenderingContext c(GameServer()->GetRenderer());
				c.SetBlend(BLEND_ALPHA);

				int iSize = 20;
				PaintWeaponSheet(pTank->GetCurrentWeapon(), (int)(vecScreen.x - flWidth/2) - 2, (int)(vecScreen.y + flWidth/2) - iSize + 2, iSize, iSize, Color(255, 255, 255, 255));
			}
		}

		if (pCurrentLocalTeam && (DigitanksWindow()->IsAltDown() || pEntity->GetTeam() == pCurrentLocalTeam || pCurrentLocalTeam->IsSelected(pSelectable)))
		{
			if (pSelectable->ShowHealthBar())
			{
				CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2 - 1), (int)(vecScreen.y - flWidth/2 - 11), (int)flWidth + 2, 5, Color(255, 255, 255, 128));
				CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y - flWidth/2 - 10), (int)(flWidth*pEntity->GetHealth()/pEntity->GetTotalHealth()), 3, Color(100, 255, 100));
			}

			if (pTank)
			{
				float flAttackPower = pTank->GetBaseAttackPower(true);
				float flDefensePower = pTank->GetBaseDefensePower(true);
				float flMovementPower = pTank->GetUsedMovementEnergy(true);
				float flTotalPower = pTank->GetStartingPower();
				flAttackPower = flAttackPower/flTotalPower;
				flDefensePower = flDefensePower/flTotalPower;
				flMovementPower = flMovementPower/pTank->GetMaxMovementEnergy();
				CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2 - 1), (int)(vecScreen.y - flWidth/2 - 7), (int)(flWidth + 2), 5, Color(255, 255, 255, 128));
				CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y - flWidth/2 - 6), (int)(flWidth*flAttackPower), 3, Color(255, 0, 0));
				CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2 + flWidth*flAttackPower), (int)(vecScreen.y - flWidth/2 - 6), (int)(flWidth*flDefensePower), 3, Color(0, 0, 255));
				CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2 - 1), (int)(vecScreen.y - flWidth/2 - 3), (int)(flWidth + 2), 5, Color(255, 255, 255, 128));
				CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y - flWidth/2 - 2), (int)(flWidth*flMovementPower), 3, Color(255, 255, 0));
			}
		}

		bool bShowEnergy = false;
		if (DigitanksGame()->GetControlMode() == MODE_AIM)
		{
			if (pTank && !pTank->IsArtillery() && !pTank->IsScout())
				bShowEnergy = true;
		}

		if (m_bHUDActive && pTank && DigitanksGame()->GetCurrentTeam()->IsPrimarySelection(pTank) && bShowEnergy)
		{
			int iHeight = (int)(200 * pTank->GetTotalPower()/pTank->GetStartingPower());

			if (iHeight < 20)
				iHeight = 20;

			int iTop = (int)vecScreen.y - iHeight/2;
			int iBottom = (int)vecScreen.y + iHeight/2;

			m_pFireAttack->SetSize(0, 20);
			m_pFireDefend->SetSize(0, 20);

			char szLabel[100];
			sprintf(szLabel, "Damage: %d%%", (int)(pTank->GetAttackPower(true)/pTank->GetStartingPower()*100));
			m_pFireAttack->SetText(szLabel);
			sprintf(szLabel, "Defense: %d%%", (int)(pTank->GetDefensePower(true)/pTank->GetStartingPower()*100));
			m_pFireDefend->SetText(szLabel);

			m_pFireAttack->EnsureTextFits();
			m_pFireDefend->EnsureTextFits();

			m_pFireAttack->SetPos((int)vecScreen.x + 70 - m_pFireAttack->GetWidth()/2, iTop-20);
			m_pFireDefend->SetPos((int)vecScreen.x + 70 - m_pFireDefend->GetWidth()/2, iBottom);

			float flAttackPercentage;
			flAttackPercentage = DigitanksGame()->GetPrimarySelectionTank()->GetWeaponEnergy() / DigitanksGame()->GetPrimarySelectionTank()->GetTotalPower();

			CRootPanel::PaintRect((int)vecScreen.x + 60, iTop, 20, iHeight, Color(255, 255, 255, 128));

			CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + 1, 18, (int)((1-flAttackPercentage)*(iHeight-2)), Color(0, 0, 255, 255));
			CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + 1 + (int)((1-flAttackPercentage)*(iHeight-2)), 18, (int)(flAttackPercentage*(iHeight-2)), Color(255, 0, 0, 255));
			CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + (int)((1-flAttackPercentage)*(iHeight-2)) - 2, 18, 6, Color(128, 128, 128, 255));

			UpdateInfo();
		}
	}

	m_pCloseActionItems->Paint();

	do {
		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);
		PaintHUDSheet(iWidth/2 - 720/2, iHeight-160, 720, 160, 0, 864, 720, 160);

		if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		{
			PaintHUDSheet(iWidth - 350, 10, 20, 20, 500, 530, 20, 20);
			PaintHUDSheet(iWidth - 250, 10, 20, 20, 540, 530, 20, 20);
			PaintHUDSheet(iWidth - 180, 10, 20, 20, 520, 530, 20, 20);

			PaintHUDSheet(m_pTurnInfo->GetLeft()-15, m_pTurnInfo->GetBottom()-585, 278, 600, 600, 0, 278, 600);

			int iResearchWidth = 570;
			CRootPanel::PaintRect(iWidth/2 - iResearchWidth/2, 0, iResearchWidth, 35, Color(0, 0, 0, 150));

			if (pCurrentLocalTeam && pCurrentLocalTeam->GetUpdateDownloading())
			{
				CUpdateItem* pItem = pCurrentLocalTeam->GetUpdateDownloading();

				float flUpdateDownloaded = pCurrentLocalTeam->GetUpdateDownloaded();
				if (flUpdateDownloaded > pItem->m_flSize)
					flUpdateDownloaded = pItem->m_flSize;
				int iResearchCompleted = (int)(iResearchWidth*(flUpdateDownloaded/pItem->m_flSize));

				CRootPanel::PaintRect(iWidth/2 - iResearchWidth/2 + 6, 6, iResearchCompleted - 12, 35 - 12, Color(0, 200, 100, 255));

				size_t iItemIcon = m_pUpdatesPanel->GetTextureForUpdateItem(pItem);

				if (iItemIcon)
				{
					float flSlideTime = Lerp(GameServer()->GetGameTime() - m_flUpdateIconSlide, 0.8f);
					int iIconX = (int)RemapValClamped(flSlideTime, 0.0f, 1.0f, (float)m_iUpdateIconSlideStartX, (float)(iWidth/2 - iResearchWidth/2 - 35));
					int iIconY = (int)RemapValClamped(flSlideTime, 0.0f, 1.0f, (float)m_iUpdateIconSlideStartY, 0.0f);
					int iButtonSize = (int)RemapValClamped(flSlideTime, 0.0f, 1.0f, (float)m_pUpdatesPanel->GetButtonSize(), 35.0f);

					CRootPanel::PaintTexture(iItemIcon, iIconX, iIconY, iButtonSize, iButtonSize);
				}
			}
		}

		if (m_flAttackInfoAlpha > 0)
			PaintHUDSheet(iWidth-175, m_pAttackInfo->GetTop()-15, 175, 110, 500, 600, 175, 110, Color(255, 255, 255, (int)(255*m_flAttackInfoAlpha)));

		if (m_flActionItemsLerp > 0)
			PaintHUDSheet(m_pActionItem->GetLeft()-30, m_pActionItem->GetTop()-30, (int)m_flActionItemsWidth, 340, 350, 0, 250, 310, Color(255, 255, 255, (int)(255*m_flActionItemsLerp)));

		PaintHUDSheet(0, iHeight-250, 150, 250, 350, 510, 150, 250);
	} while (false);

	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		CRootPanel::PaintRect(m_pScoreboard->GetLeft()-3, m_pScoreboard->GetTop()-9, m_pScoreboard->GetWidth()+6, m_pScoreboard->GetHeight()+6, Color(0, 0, 0, 100));

	size_t iX, iY, iX2, iY2;
	if (DigitanksWindow()->GetBoxSelection(iX, iY, iX2, iY2))
	{
		Color clrSelection(255, 255, 255, 255);

		size_t iWidth = iX2 - iX;
		size_t iHeight = iY2 - iY;
		CRootPanel::PaintRect(iX, iY, iWidth, 1, clrSelection);
		CRootPanel::PaintRect(iX, iY, 1, iHeight, clrSelection);
		CRootPanel::PaintRect(iX + iWidth, iY, 1, iHeight, clrSelection);
		CRootPanel::PaintRect(iX, iY + iHeight, iWidth, 1, clrSelection);
	}

	if (m_hHintWeapon == NULL)
	{
		CTeam* pLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
		for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
			if (!pEntity)
				continue;

			CBaseWeapon* pWeapon = dynamic_cast<CBaseWeapon*>(pEntity);
			if (!pWeapon)
				continue;

			if (pWeapon->HasExploded())
				continue;

			if (!pWeapon->UsesSpecialCommand())
				continue;

			if (pLocalTeam && pWeapon->GetOwner() && pWeapon->GetOwner()->GetTeam() == pLocalTeam)
			{
				m_hHintWeapon = pWeapon;
				break;
			}
		}
	}

	if (m_hHintWeapon != NULL)
	{
		int iX = GetWidth()/2 + 50;
		int iY = GetHeight()/2 + 50;

		CRootPanel::PaintRect(iX-6, iY-1, 60, 75, Color(0, 0, 0, 150));

		m_pSpacebarHint->SetVisible(true);
		m_pSpacebarHint->SetText(m_hHintWeapon->SpecialCommandHint());
		m_pSpacebarHint->SetSize(60, 25);
		m_pSpacebarHint->SetPos(iX-6, iY + 50);
		m_pSpacebarHint->SetWrap(false);

		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);

		if (fmod(GameServer()->GetGameTime(), 0.5f) > 0.25f)
			iY -= 10;

		PaintSheet(m_iKeysSheet, iX, iY, 48, 48, 128, 64, 64, 64, 256, 256, Color(255, 255, 255, 255));
	}
	else
	{
		m_pSpacebarHint->SetVisible(false);
	}

	if (DigitanksGame()->GetPrimarySelectionTank() && DigitanksGame()->GetControlMode() == MODE_AIM)
	{
		if (DigitanksGame()->GetAimType() == AIM_NORMAL)
		{
			CRenderingContext c(GameServer()->GetRenderer());
			c.SetBlend(BLEND_ALPHA);

			int iX = GetWidth()/2 - 150;
			int iY = GetHeight()/2 - 150;

			bool bObstruction = false;
			CDigitank* pDigitank = DigitanksGame()->GetPrimarySelectionTank();
			Vector vecTankOrigin = pDigitank->GetOrigin() + Vector(0, 1, 0);

			float flGravity = DigitanksGame()->GetGravity();
			float flTime;
			Vector vecForce;
			FindLaunchVelocity(vecTankOrigin, pDigitank->GetPreviewAim(), flGravity, vecForce, flTime, pDigitank->ProjectileCurve());

			size_t iLinks = 20;
			float flTimePerLink = flTime/iLinks;
			Vector vecLastOrigin = vecTankOrigin;
			Vector vecPoint;
			for (size_t i = 1; i < iLinks; i++)
			{
				float flCurrentTime = flTimePerLink*i;
				Vector vecCurrentOrigin = vecTankOrigin + vecForce*flCurrentTime + Vector(0, flGravity*flCurrentTime*flCurrentTime/2, 0);
				if (DigitanksGame()->GetTerrain()->Collide(vecLastOrigin, vecCurrentOrigin, vecPoint))
				{
					bObstruction = true;
					break;
				}

				vecLastOrigin = vecCurrentOrigin;
			}

			if (bObstruction)
				CRootPanel::PaintTexture(m_iObstruction, iX, iY, 100, 100, Color(255, 255, 255, (int)RemapVal(Oscillate(GameServer()->GetGameTime(), 0.8f), 0, 1, 150, 255)));
		}
	}

	bool bCloseVisible = m_pCloseActionItems->IsVisible();
	m_pCloseActionItems->SetVisible(false);
	m_pButtonInfo->SetVisible(false);
	CPanel::Paint(x, y, w, h);
	m_pCloseActionItems->SetVisible(bCloseVisible);

	if (m_pButtonInfo->GetText()[0] != L'\0')
	{
		CRootPanel::PaintRect(m_pButtonInfo->GetLeft()-3, m_pButtonInfo->GetTop()-9, m_pButtonInfo->GetWidth()+6, m_pButtonInfo->GetHeight()+6, Color(0, 0, 0));
		m_pButtonInfo->SetVisible(true);
		m_pButtonInfo->Paint();
	}

	CSelectable* pSelection = DigitanksGame()->GetPrimarySelection();

	if (pSelection)
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);

		int iSize = 100;
		CDigitank* pTank = DigitanksGame()->GetPrimarySelectionTank();
		if (pTank)
			iSize = 50;

		Color clrTeam(255, 255, 255, 255);
		if (pSelection->GetTeam())
			clrTeam = pSelection->GetTeam()->GetColor();
		PaintUnitSheet(pSelection->GetUnitType(), iWidth/2 - 720/2 + 10 + 150/2 - iSize/2, iHeight - 150 + 10 + 130/2 - iSize/2, iSize, iSize, clrTeam);
	}

	CDigitank* pTank = DigitanksGame()->GetPrimarySelectionTank();

	if (pTank)
	{
		CRenderingContext c(GameServer()->GetRenderer());

		c.SetBlend(BLEND_ALPHA);

		int iSize = 36;
		PaintWeaponSheet(pTank->GetCurrentWeapon(), iWidth/2 - 720/2 + 10 + 2, iHeight - 150 + 10 + 130 - iSize - 2, iSize, iSize, Color(255, 255, 255, 255));

		c.SetBlend(BLEND_ADDITIVE);
		c.Translate(Vector((float)(iWidth/2 - 720/2 + 10 + 150/2), (float)(iHeight - 150 + 10 + 130/2), 0));

		do
		{
			int iShield = (int)(255*pTank->GetFrontShieldStrength());
			if (iShield > 255)
				iShield = 255;
			PaintHUDSheet(-50/2, -50/2 - 20, 50, 10, 0, 850, 50, 14, Color(255, 255, 255, iShield));
		}
		while (false);

		do
		{
			CRenderingContext c(GameServer()->GetRenderer());
			c.Rotate(90, Vector(0, 0, 1));

			int iShield = (int)(255*pTank->GetRightShieldStrength());
			if (iShield > 255)
				iShield = 255;
			PaintHUDSheet(-50/2, -50/2 - 20, 50, 10, 0, 850, 50, 14, Color(255, 255, 255, iShield));
		}
		while (false);

		do
		{
			CRenderingContext c(GameServer()->GetRenderer());
			c.Rotate(180, Vector(0, 0, 1));

			int iShield = (int)(255*pTank->GetRearShieldStrength());
			if (iShield > 255)
				iShield = 255;
			PaintHUDSheet(-50/2, -50/2 - 20, 50, 10, 0, 850, 50, 14, Color(255, 255, 255, iShield));
		}
		while (false);

		do
		{
			CRenderingContext c(GameServer()->GetRenderer());
			c.Rotate(270, Vector(0, 0, 1));

			int iShield = (int)(255*pTank->GetLeftShieldStrength());
			if (iShield > 255)
				iShield = 255;
			PaintHUDSheet(-50/2, -50/2 - 20, 50, 10, 0, 850, 50, 14, Color(255, 255, 255, iShield));
		}
		while (false);
	}

	if (m_eActionSign)
	{
		float flAnimationTime = GameServer()->GetGameTime() - m_flActionSignStart;
		if (flAnimationTime > 2.0f)
			m_eActionSign = ACTIONSIGN_NONE;
		else
		{
			float flWarpIn = RemapValClamped(flAnimationTime, 0, 0.5f, 0, 1);
			float flAlpha = RemapValClamped(flAnimationTime, 1.3f, 1.8f, 1, 0);

			size_t iTotalWidth = DigitanksWindow()->GetWindowWidth();
			size_t iTotalHeight = DigitanksWindow()->GetWindowHeight();

			size_t iTankWidth = iTotalWidth/4;
			size_t iTankHeight = iTankWidth/2;

			size_t iTankOffset = (size_t)(Lerp(1-flWarpIn, 0.2f) * iTankWidth*2);

			CRenderingContext c(GameServer()->GetRenderer());
			c.SetBlend(BLEND_ALPHA);

			PaintSheet(m_iActionTanksSheet, iTotalWidth-iTankWidth*3/2 + iTankOffset, iTotalHeight/2-iTankHeight/2, iTankWidth, iTankHeight, 0, 0, 512, 256, 512, 512, Color(255, 255, 255, (int)(flAlpha*255)));
			PaintSheet(m_iActionTanksSheet, iTankWidth/2 - iTankOffset, iTotalHeight/2-iTankHeight/2, iTankWidth, iTankHeight, 0, 256, 512, 256, 512, 512, Color(255, 255, 255, (int)(flAlpha*255)));

			size_t iSignWidth = (size_t)(iTotalWidth/2 * Lerp(flWarpIn, 0.2f));
			size_t iSignHeight = (size_t)(iSignWidth*0.32f);
			if (m_eActionSign == ACTIONSIGN_FIGHT)
				PaintSheet(m_iActionSignsSheet, iTotalWidth/2-iSignWidth/2, iTotalHeight/2-iSignHeight/2, iSignWidth, iSignHeight, 0, 0, 1024, 330, 1024, 1024, Color(255, 255, 255, (int)(flAlpha*255)));
			else if (m_eActionSign == ACTIONSIGN_SHOWDOWN)
				PaintSheet(m_iActionSignsSheet, iTotalWidth/2-iSignWidth/2, iTotalHeight/2-iSignHeight/2, iSignWidth, iSignHeight, 0, 330, 1024, 330, 1024, 1024, Color(255, 255, 255, (int)(flAlpha*255)));
		}
	}

//	while (true)
//	{
//		CRenderingContext c(GameServer()->GetRenderer());
//		c.SetBlend(BLEND_ALPHA);
//		CRootPanel::PaintTexture(m_iCompetitionWatermark, 20, 20, 125/2, 184/2);
//		break;
//	}
}

void CHUD::PaintSheet(size_t iTexture, int x, int y, int w, int h, int sx, int sy, int sw, int sh, int tw, int th, const Color& c)
{
	glgui::CBaseControl::PaintSheet(iTexture, x, y, w, h, sx, sy, sw, sh, tw, th, c);
}

void CHUD::PaintHUDSheet(int x, int y, int w, int h, int sx, int sy, int sw, int sh, const Color& c)
{
	PaintSheet(DigitanksWindow()->GetHUD()->m_iHUDSheet, x, y, w, h, sx, sy, sw, sh, 1024, 1024, c);
}

void CHUD::GetUnitSheet(unittype_t eUnit, int& sx, int& sy, int& sw, int& sh, int& tw, int& th)
{
	tw = th = 512;
	if (eUnit == UNIT_TANK)
	{
		sx = 0;
		sy = 0;
		sw = 100;
		sh = 100;
		return;
	}
	else if (eUnit == UNIT_SCOUT)
	{
		sx = 100;
		sy = 0;
		sw = 100;
		sh = 100;
		return;
	}
	else if (eUnit == UNIT_ARTILLERY)
	{
		sx = 0;
		sy = 100;
		sw = 100;
		sh = 100;
		return;
	}
	else if (eUnit == UNIT_INFANTRY)
	{
		sx = 100;
		sy = 100;
		sw = 100;
		sh = 100;
		return;
	}
	else if (eUnit == STRUCTURE_MINIBUFFER)
	{
		sx = 300;
		sy = 0;
		sw = 100;
		sh = 100;
		return;
	}
	else if (eUnit == STRUCTURE_BUFFER)
	{
		sx = 200;
		sy = 0;
		sw = 100;
		sh = 100;
		return;
	}
	else if (eUnit == STRUCTURE_BATTERY)
	{
		sx = 300;
		sy = 100;
		sw = 100;
		sh = 100;
		return;
	}
	else if (eUnit == STRUCTURE_PSU)
	{
		sx = 200;
		sy = 100;
		sw = 100;
		sh = 100;
		return;
	}
	else if (eUnit == STRUCTURE_INFANTRYLOADER)
	{
		sx = 400;
		sy = 0;
		sw = 100;
		sh = 100;
		return;
	}
	else if (eUnit == STRUCTURE_TANKLOADER)
	{
		sx = 400;
		sy = 100;
		sw = 100;
		sh = 100;
		return;
	}
	else if (eUnit == STRUCTURE_ARTILLERYLOADER)
	{
		sx = 400;
		sy = 200;
		sw = 100;
		sh = 100;
		return;
	}
	else if (eUnit == STRUCTURE_CPU)
	{
		sx = 000;
		sy = 200;
		sw = 100;
		sh = 100;
		return;
	}
	else
	{
		sx = 0;
		sy = 0;
		sw = 100;
		sh = 100;
		return;
	}
}

void CHUD::PaintUnitSheet(unittype_t eUnit, int x, int y, int w, int h, const Color& c)
{
	int sx, sy, sw, sh, tw, th;
	GetUnitSheet(eUnit, sx, sy, sw, sh, tw, th);
	PaintSheet(DigitanksWindow()->GetHUD()->m_iUnitsSheet, x, y, w, h, sx, sy, sw, sh, tw, th, c);
}

void CHUD::GetWeaponSheet(weapon_t eWeapon, int& sx, int& sy, int& sw, int& sh, int& tw, int& th)
{
	tw = 512;
	th = 256;

	sw = 64;
	sh = 64;

	switch (eWeapon)
	{
	default:
	case WEAPON_NONE:
	case PROJECTILE_SMALL:
		sx = 0;
		sy = 0;
		break;

	case PROJECTILE_MEDIUM:
		sx = 64;
		sy = 0;
		break;

	case PROJECTILE_LARGE:
		sx = 128;
		sy = 0;
		break;

	case PROJECTILE_AOE:
	case PROJECTILE_ARTILLERY_AOE:
		sx = 192;
		sy = 0;
		break;

	case PROJECTILE_EMP:
		sx = 256;
		sy = 0;
		break;

	case PROJECTILE_ICBM:
	case PROJECTILE_ARTILLERY_ICBM:
		sx = 320;
		sy = 0;
		break;

	case PROJECTILE_GRENADE:
		sx = 384;
		sy = 0;
		break;

	case PROJECTILE_DAISYCHAIN:
		sx = 448;
		sy = 0;
		break;

	case PROJECTILE_CLUSTERBOMB:
		sx = 0;
		sy = 64;
		break;

	case PROJECTILE_SPLOOGE:
		sx = 64;
		sy = 64;
		break;

	case PROJECTILE_TRACTORBOMB:
		sx = 128;
		sy = 64;
		break;

	case PROJECTILE_EARTHSHAKER:
		sx = 192;
		sy = 64;
		break;

	case PROJECTILE_CAMERAGUIDED:
		sx = 256;
		sy = 64;
		break;

	case WEAPON_LASER:
	case WEAPON_INFANTRYLASER:
		sx = 320;
		sy = 64;
		break;

	case WEAPON_CHARGERAM:
		sx = 384;
		sy = 64;
		break;
	}
}

void CHUD::PaintWeaponSheet(weapon_t eWeapon, int x, int y, int w, int h, const Color& c)
{
	int sx, sy, sw, sh, tw, th;
	GetWeaponSheet(eWeapon, sx, sy, sw, sh, tw, th);
	PaintSheet(DigitanksWindow()->GetHUD()->m_iWeaponsSheet, x, y, w, h, sx, sy, sw, sh, tw, th, c);
}

size_t CHUD::GetWeaponSheet()
{
	return DigitanksWindow()->GetHUD()->m_iWeaponsSheet;
}

void CHUD::ClientEnterGame()
{
	UpdateInfo();
	UpdateTeamInfo();
	UpdateScoreboard();
	UpdateTurnButton();
	SetupMenu();

	m_pPressEnter->SetText("");

	m_pSceneTree->BuildTree(true);
}

void CHUD::UpdateInfo()
{
	if (!DigitanksGame())
		return;

	m_pAttackInfo->SetText("");

	CDigitank* pCurrentTank = DigitanksGame()->GetPrimarySelectionTank();

	if (pCurrentTank)
		UpdateTankInfo(pCurrentTank);

	CStructure* pCurrentStructure = DigitanksGame()->GetPrimarySelectionStructure();

	if (pCurrentStructure)
		UpdateStructureInfo(pCurrentStructure);

	CSelectable* pCurrentSelection = DigitanksGame()->GetPrimarySelection();

	if (pCurrentSelection)
	{
		eastl::string16 sInfo;
		pCurrentSelection->UpdateInfo(sInfo);
		m_pTankInfo->SetText(sInfo.c_str());
	}
}

void CHUD::UpdateTankInfo(CDigitank* pTank)
{
	char szShieldInfo[1024];

	if (pTank->GetFrontShieldMaxStrength() > 0)
	{
		sprintf(szShieldInfo, "%.1f/%.1f",
			pTank->GetFrontShieldStrength() * pTank->GetFrontShieldMaxStrength(),
			pTank->GetFrontShieldMaxStrength() * pTank->GetDefenseScale(true));
		m_pFrontShieldInfo->SetText(szShieldInfo);
		m_pFrontShieldInfo->SetVisible(true);
	}
	else
		m_pFrontShieldInfo->SetVisible(false);

	if (pTank->GetRearShieldMaxStrength() > 0)
	{
		sprintf(szShieldInfo, "%.1f/%.1f",
			pTank->GetRearShieldStrength() * pTank->GetRearShieldMaxStrength(),
			pTank->GetRearShieldMaxStrength() * pTank->GetDefenseScale(true));
		m_pRearShieldInfo->SetText(szShieldInfo);
		m_pRearShieldInfo->SetVisible(true);
	}
	else
		m_pRearShieldInfo->SetVisible(false);

	if (pTank->GetLeftShieldMaxStrength() > 0)
	{
		sprintf(szShieldInfo, "%.1f/\n%.1f",
			pTank->GetLeftShieldStrength() * pTank->GetLeftShieldMaxStrength(),
			pTank->GetLeftShieldMaxStrength() * pTank->GetDefenseScale(true));
		m_pLeftShieldInfo->SetText(szShieldInfo);
		m_pLeftShieldInfo->SetVisible(true);
	}
	else
		m_pLeftShieldInfo->SetVisible(false);

	if (pTank->GetRightShieldMaxStrength() > 0)
	{
		sprintf(szShieldInfo, "%.1f/\n%.1f",
			pTank->GetRightShieldStrength() * pTank->GetRightShieldMaxStrength(),
			pTank->GetRightShieldMaxStrength() * pTank->GetDefenseScale(true));
		m_pRightShieldInfo->SetText(szShieldInfo);
		m_pRightShieldInfo->SetVisible(true);
	}
	else
		m_pRightShieldInfo->SetVisible(false);

	m_pAttackInfo->SetText(L"");

	Vector vecOrigin;
	if (DigitanksGame()->GetControlMode() == MODE_MOVE && pTank->GetPreviewMoveTurnPower() <= pTank->GetRemainingMovementEnergy())
		vecOrigin = pTank->GetPreviewMove();
	else
		vecOrigin = pTank->GetOrigin();

	Vector vecAim;
	if (DigitanksGame()->GetControlMode() == MODE_AIM)
		vecAim = pTank->GetPreviewAim();
	else
		vecAim = pTank->GetLastAim();

	Vector vecAttack = vecOrigin - vecAim;
	float flAttackDistance = vecAttack.Length();

	if (!pTank->IsInsideMaxRange(vecAim))
		return;

	if (flAttackDistance < pTank->GetMinRange())
		return;

	if (!pTank->IsPreviewAimValid())
		return;

	float flRadius = pTank->FindAimRadius(vecAim);

	CDigitank* pClosestTarget = NULL;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		if (!pEntity)
			continue;

		CDigitank* pTargetTank = dynamic_cast<CDigitank*>(pEntity);

		if (!pTargetTank)
			continue;

		if (pTargetTank == pTank)
			continue;

		if (pTargetTank->GetTeam() == pTank->GetTeam())
			continue;

		Vector vecOrigin2D = pTargetTank->GetOrigin();
		Vector vecAim2D = vecAim;
		vecOrigin2D.y = vecAim2D.y = 0;
		if ((vecOrigin2D - vecAim2D).LengthSqr() > flRadius*flRadius)
			continue;

		if (pTargetTank->GetVisibility() == 0)
			continue;

		if (!pClosestTarget)
		{
			pClosestTarget = pTargetTank;
			continue;
		}

		if ((pTargetTank->GetOrigin() - vecAim).LengthSqr() < (pClosestTarget->GetOrigin() - vecAim).LengthSqr())
			pClosestTarget = pTargetTank;
	}

	if (!pClosestTarget)
		return;

	vecAttack = vecOrigin - pClosestTarget->GetOrigin();

	Vector vecOrigin2D = pClosestTarget->GetOrigin();
	Vector vecAim2D = vecAim;
	vecOrigin2D.y = vecAim2D.y = 0;
	float flTargetDistance = (vecAim2D - vecOrigin2D).Length();

	if (flTargetDistance > flRadius)
		return;

	float flShieldStrength = pClosestTarget->GetShieldValueForAttackDirection(vecAttack.Normalized());
	float flDamageBlocked = flShieldStrength * pClosestTarget->GetDefenseScale(true);
	float flAttackDamage = pTank->GetAttackPower(true);

	float flShieldDamage;
	float flTankDamage = 0;
	if (flAttackDamage - flDamageBlocked <= 0)
		flShieldDamage = flAttackDamage;
	else
	{
		flShieldDamage = flDamageBlocked;
		flTankDamage = flAttackDamage - flDamageBlocked;
	}

	char szAttackInfo[1024];
	sprintf(szAttackInfo,
		"ATTACK REPORT\n \n"
		"Shield Damage: %.1f/%.1f\n"
		"Digitank Damage: %.1f/%.1f\n",
		flShieldDamage, flShieldStrength * pClosestTarget->GetDefenseScale(true),
		flTankDamage, pClosestTarget->GetHealth()
	);

	m_pAttackInfo->SetText(szAttackInfo);
}

void CHUD::UpdateStructureInfo(CStructure* pStructure)
{
	m_pFrontShieldInfo->SetText("");
	m_pRearShieldInfo->SetText("");
	m_pLeftShieldInfo->SetText("");
	m_pRightShieldInfo->SetText("");

	m_pAttackInfo->SetText(L"");
}

void CHUD::UpdateTeamInfo()
{
	if (!DigitanksGame())
		return;

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
	{
		m_pPowerInfo->SetText("");
		m_pFleetInfo->SetText("");
		m_pBandwidthInfo->SetText("");
		return;
	}

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	if (!pTeam)
		return;

	eastl::string16 s1;
	s1.sprintf(L"%d +%.1f/turn\n", (int)pTeam->GetPower(), pTeam->GetPowerPerTurn());
	m_pPowerInfo->SetText(s1);

	eastl::string16 s2;
	s2.sprintf(L"%d/%d\n", pTeam->GetUsedFleetPoints(), pTeam->GetTotalFleetPoints());
	m_pFleetInfo->SetText(s2);

	eastl::string16 s3;
	s3.sprintf(L"%d/%dmb +%.1fmb/turn\n", (int)pTeam->GetUpdateDownloaded(), (int)pTeam->GetUpdateSize(), pTeam->GetBandwidth());
	m_pBandwidthInfo->SetText(s3);
}

void CHUD::UpdateScoreboard()
{
	if (!DigitanksGame())
		return;

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
		return;

	eastl::vector<CDigitanksTeam*> apSortedTeams;

	// Prob not the fastest sorting algorithm but it doesn't need to be.
	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
	{
		CDigitanksTeam* pTeam = DigitanksGame()->GetDigitanksTeam(i);

		if (!pTeam->ShouldIncludeInScoreboard())
			continue;

		if (apSortedTeams.size() == 0)
		{
			apSortedTeams.push_back(pTeam);
			continue;
		}

		bool bFound = false;
		for (size_t j = 0; j < apSortedTeams.size(); j++)
		{
			if (pTeam->GetScore() > apSortedTeams[j]->GetScore())
			{
				apSortedTeams.insert(apSortedTeams.begin()+j, pTeam);
				bFound = true;
				break;
			}
		}

		if (!bFound)
			apSortedTeams.push_back(pTeam);
	}

	eastl::string16 s;
	eastl::string16 p;
	for (size_t i = 0; i < apSortedTeams.size(); i++)
	{
		CDigitanksTeam* pTeam = apSortedTeams[i];

		if (DigitanksGame()->IsTeamControlledByMe(pTeam))
			s += L"[";
		s += pTeam->GetName();
		if (DigitanksGame()->IsTeamControlledByMe(pTeam))
			s += L"]";

		s += p.sprintf(L": %d\n", pTeam->GetScore());
	}

	m_pScoreboard->SetText(s);

	m_pScoreboard->SetSize(100, 9999);
	m_pScoreboard->SetSize(m_pScoreboard->GetWidth(), (int)m_pScoreboard->GetTextHeight());

	int iWidth = DigitanksWindow()->GetWindowWidth();

	m_pScoreboard->SetPos(iWidth - m_pScoreboard->GetWidth() - 10, m_pAttackInfo->GetTop() - m_pScoreboard->GetHeight() - 20);
}

void CHUD::UpdateTurnButton()
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTeam())
		return;

	bool bTurnComplete = true;

	for (size_t i = 0; i < DigitanksGame()->GetCurrentTeam()->GetNumMembers(); i++)
	{
		CDigitank* pTank = dynamic_cast<CDigitank*>(DigitanksGame()->GetCurrentTeam()->GetMember(i));

		if (pTank)
		{
			if (pTank->NeedsOrders())
				bTurnComplete = false;
		}
	}

	if (!DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentTeam()))
	{
		m_pTurnButton->SetSheetTexture(m_iHUDSheet, 320, 760, 120, 90, 1024, 1024);
		m_pPressEnter->SetVisible(true);
	}
	else if (bTurnComplete)
	{
		m_pTurnButton->SetSheetTexture(m_iHUDSheet, 160, 730, 160, 120, 1024, 1024);
		m_pPressEnter->SetVisible(true);
	}
	else
	{
		m_pTurnButton->SetSheetTexture(m_iHUDSheet, 0, 730, 160, 120, 1024, 1024);
		m_pPressEnter->SetVisible(false);
	}

	if (DigitanksGame()->GetCurrentTeam() != DigitanksGame()->GetCurrentLocalDigitanksTeam())
		m_pPressEnter->SetVisible(true);
}

void CHUD::SetupMenu()
{
	SetupMenu(m_eMenuMode);
}

void CHUD::SetupMenu(menumode_t eMenuMode)
{
	if (!DigitanksGame())
		return;

	Color clrBox = glgui::g_clrBox;
	clrBox.SetAlpha(100);

	for (size_t i = 0; i < NUM_BUTTONS; i++)
	{
		m_apButtons[i]->SetClickedListener(NULL, NULL);
		m_apButtons[i]->SetEnabled(false);
		SetButtonColor(i, clrBox);
		SetButtonTexture(i, 0);
		SetButtonInfo(i, L"");
	}

	if (!IsActive() || !DigitanksGame()->GetPrimarySelection() || DigitanksGame()->GetPrimarySelection()->GetTeam() != DigitanksGame()->GetCurrentLocalDigitanksTeam())
		return;

	DigitanksGame()->GetPrimarySelection()->SetupMenu(eMenuMode);

	m_eMenuMode = eMenuMode;
}

void CHUD::SetButtonListener(int i, IEventListener::Callback pfnCallback)
{
	if (pfnCallback)
	{
		m_apButtons[i]->SetClickedListener(this, pfnCallback);
		m_apButtons[i]->SetEnabled(true);
	}
	else
	{
		m_apButtons[i]->SetClickedListener(NULL, NULL);
		m_apButtons[i]->SetEnabled(false);
	}
}

void CHUD::SetButtonTexture(int i, size_t iTexture)
{
	m_apButtons[i]->SetTexture(iTexture);
}

void CHUD::SetButtonColor(int i, Color clrButton)
{
	m_apButtons[i]->SetButtonColor(clrButton);
}

void CHUD::SetButtonInfo(int iButton, const eastl::string16& pszInfo)
{
	m_aszButtonInfos[iButton] = pszInfo;
}

void CHUD::ButtonCallback(int iButton)
{
	if (m_apButtons[iButton]->GetClickedListener())
		m_apButtons[iButton]->GetClickedListenerCallback()(m_apButtons[iButton]->GetClickedListener());
}

void CHUD::GameStart()
{
	DigitanksGame()->SetControlMode(MODE_NONE);
	DigitanksWindow()->GetInstructor()->Initialize();
	ClearTurnInfo();
}

void CHUD::GameOver(bool bPlayerWon)
{
	DigitanksWindow()->GameOver(bPlayerWon);
}

void CHUD::NewCurrentTeam()
{
	if (m_bHUDActive && DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentTeam()) &&
			DigitanksGame()->GetPrimarySelectionTank() && !DigitanksGame()->GetPrimarySelectionTank()->HasGoalMovePosition() &&
			DigitanksWindow()->GetInstructor()->GetCurrentTutorial() != CInstructor::TUTORIAL_THEEND_BASICS)	// Don't set control mode if it's the end so we can open the menu.
		DigitanksGame()->SetControlMode(MODE_MOVE);
	else
		DigitanksGame()->SetControlMode(MODE_NONE);

	UpdateTurnButton();
	UpdateTeamInfo();

	if (DigitanksGame()->GetCurrentTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
		m_pPressEnter->SetText("Press <ENTER> to end your turn...");
	else
		m_pPressEnter->SetText("Other players are taking their turns...");

	if (DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentTeam()))
		m_flTurnInfoLerpGoal = 1;
	else
		m_flTurnInfoLerpGoal = 0;

	if (DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentTeam()))
	{
		// Don't open the research window on the first turn, give the player a chance to see the game grid first.
		if (DigitanksGame()->GetTurn() >= 1 && DigitanksGame()->GetUpdateGrid() && !DigitanksGame()->GetCurrentTeam()->GetUpdateDownloading())
		{
			bool bShouldOpen = false;
			for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
			{
				for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
				{
					if (DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y].m_eUpdateClass != UPDATECLASS_EMPTY && !DigitanksGame()->GetCurrentTeam()->HasDownloadedUpdate(x, y))
					{
						bShouldOpen = true;
						break;
					}
				}

				if (bShouldOpen)
					break;
			}

			m_bUpdatesBlinking = bShouldOpen;

			// If we've got to the third turn and the player isn't downloading anything, pop it up so he knows it's there.
			if (DigitanksGame()->GetTurn() == 3 && bShouldOpen)
			{
				m_pUpdatesPanel->SetVisible(true);
				DigitanksGame()->SetControlMode(MODE_NONE);
				m_bUpdatesBlinking = false;
			}
		}

		if (DigitanksWindow()->GetInstructor()->GetActive())
			m_bUpdatesBlinking = false;

		if (!DigitanksWindow()->IsRegistered() && DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		{
			eastl::string16 s;
			s.sprintf(L"Demo turns left: %d", DigitanksGame()->GetDemoTurns() - DigitanksGame()->GetTurn());
			m_pDemoNotice->SetText(s);
		}
		else
			m_pDemoNotice->SetText("");

		// If we have local hotseat multiplayer, update for the new team.
		m_pSceneTree->BuildTree();
	}

	UpdateScoreboard();

	if (DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentTeam()))
	{
		bool bShow = true;

		// Don't show the action items until we have a CPU because we have the story and the tutorials to think about first.
		if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD && !DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU())
			bShow = false;

		if (m_pUpdatesPanel->IsVisible())
			bShow = false;

		if (bShow)
			ShowFirstActionItem();
	}

	if (CNetwork::IsConnected() && DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentTeam()))
		CSoundLibrary::PlaySound(NULL, L"sound/lesson-learned.wav");	// No time to make a new sound.

	CRootPanel::Get()->Layout();
}

void CHUD::NewCurrentSelection()
{
	UpdateTurnButton();
	UpdateInfo();

	SetupMenu(MENUMODE_MAIN);

	ShowActionItem(DigitanksGame()->GetPrimarySelection());
}

void CHUD::ShowFirstActionItem()
{
	m_bAllActionItemsHandled = false;
	eastl::vector<actionitem_t>& aActionItems = DigitanksGame()->GetActionItems();

	if (aActionItems.size())
	{
		m_iCurrentActionItem = -1;
		ShowNextActionItem();
	}
	else
	{
		m_pActionItem->SetText("");
		m_bAllActionItemsHandled = true;
		m_flActionItemsLerpGoal = 0;
	}
}

void CHUD::ShowNextActionItem()
{
	if (m_bAllActionItemsHandled)
		return;

	size_t iOriginalActionItem = m_iCurrentActionItem;
	eastl::vector<actionitem_t>& aActionItems = DigitanksGame()->GetActionItems();

	do
	{
		m_iCurrentActionItem = (m_iCurrentActionItem+1)%aActionItems.size();

		if (m_iCurrentActionItem == iOriginalActionItem)
		{
			// We're done!
			m_bAllActionItemsHandled = true;
			m_pActionItem->SetText("");

			m_flActionItemsLerpGoal = 0;
			return;
		}

		// Bit of a hack. If m_iCurrentActionItem was -1 (or ~0 unsigned) then it'll now be 0.
		// Once it loops back around to 0 again the second time we'll consider it done.
		if (iOriginalActionItem == -1)
			iOriginalActionItem = 0;

		actionitem_t* pItem = &aActionItems[m_iCurrentActionItem];

		if (pItem->eActionType == ACTIONTYPE_DOWNLOADCOMPLETE && DigitanksGame()->GetCurrentTeam()->GetUpdateDownloading())
			pItem->bHandled = true;

		if (pItem->bHandled)
			continue;

		// Only perform this check with tanks. Structures always return false for NeedsOrders()
		CEntityHandle<CDigitank> hDigitank(pItem->iUnit);

		if (hDigitank != NULL && !hDigitank->NeedsOrders())
		{
			// If must have been handled already.
			pItem->bHandled = true;
			continue;
		}

		ShowActionItem(m_iCurrentActionItem);
		return;
	}
	while (true);
}

void CHUD::ShowActionItem(CSelectable* pSelectable)
{
	if (m_bAllActionItemsHandled)
		return;

	eastl::vector<actionitem_t>& aActionItems = DigitanksGame()->GetActionItems();

	if (pSelectable)
	{
		for (size_t i = 0; i < aActionItems.size(); i++)
		{
			if (aActionItems[i].iUnit == pSelectable->GetHandle())
			{
				if (i == m_iCurrentActionItem)
					return;

				m_iCurrentActionItem = i;
				ShowActionItem(m_iCurrentActionItem);
				return;
			}
		}
	}

	// There is no action item for the current unit. If the current action item has no unit, don't clobber it just to show the "press next" message.
	if (m_iCurrentActionItem >= 0 && m_iCurrentActionItem < aActionItems.size() && aActionItems[m_iCurrentActionItem].iUnit == ~0)
		return;

	ShowActionItem(~0);
}

void CHUD::ShowActionItem(actiontype_t eActionItem)
{
	if (m_bAllActionItemsHandled)
		return;

	eastl::vector<actionitem_t>& aActionItems = DigitanksGame()->GetActionItems();
	for (size_t i = 0; i < aActionItems.size(); i++)
	{
		if (aActionItems[i].eActionType == eActionItem)
		{
			if (i == m_iCurrentActionItem)
				return;

			m_iCurrentActionItem = i;
			ShowActionItem(m_iCurrentActionItem);
			return;
		}
	}
}

void CHUD::ShowActionItem(size_t iActionItem)
{
	if (m_bAllActionItemsHandled)
		return;

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
		return;

	eastl::vector<actionitem_t>& aActionItems = DigitanksGame()->GetActionItems();
	if (iActionItem >= aActionItems.size())
	{
		m_pActionItem->SetText("");
		m_flActionItemsLerpGoal = 0;
		return;
	}

	actionitem_t* pItem = &aActionItems[iActionItem];

	switch (pItem->eActionType)
	{
	case ACTIONTYPE_WELCOME:
		m_pActionItem->SetText(
			"WELCOME TO DIGITANKS\n \n"
			"Here on the right is the 'Action Items' list. It will help guide you through the tasks you need to complete each turn.\n \n"
			"Click an icon on the right to view that action item. Items that are blinking need handling. When you deal with all items you can end your turn confidently!\n \n"
			"When you're done, press the 'End Turn' button to continue.\n");
		break;

	case ACTIONTYPE_CONTROLS:
		m_pActionItem->SetText(
			"QUICK CONTROLS\n \n"
			"Rotate view - Drag right mouse button\n"
			"Move view - Hold space, move mouse\n"
			"Zoom - Scrollwheel or pgup/pgdn\n");
		break;

	case ACTIONTYPE_NEWSTRUCTURE:
		m_pActionItem->SetText(
			"NEW STRUCTURE COMPLETED\n \n"
			"This structure has just been completed.\n");
		break;

	case ACTIONTYPE_UNITORDERS:
		m_pActionItem->SetText(
			"UNIT NEEDS ORDERS\n \n"
			"This unit needs new orders. You can move it and fire on enemies in range.\n");
		break;

	case ACTIONTYPE_UNITAUTOMOVE:
		m_pActionItem->SetText(
			"UNIT AUTO-MOVE COMPLETE\n \n"
			"This unit has finished its auto-move task. Please assign it new orders.\n");
		break;

	case ACTIONTYPE_AUTOMOVECANCELED:
		m_pActionItem->SetText(
			"UNIT AUTO-MOVE CANCELED\n \n"
			"This unit's auto-move has been canceled, due to it taking damage from enemy fire. Please assign it new orders.\n");
		break;

	case ACTIONTYPE_AUTOMOVEENEMY:
		m_pActionItem->SetText(
			"UNIT AUTO-MOVE THREAT\n \n"
			"This unit is auto-moving to a new location, but an enemy is visible. You may wish to cancel this auto move.\n");
		break;

	case ACTIONTYPE_UNITDAMAGED:
		m_pActionItem->SetText(
			"UNIT DAMAGED\n \n"
			"This unit has been damaged by enemy fire. Take evasive action!\n");
		break;

	case ACTIONTYPE_FORTIFIEDENEMY:
		m_pActionItem->SetText(
			"ENEMY SIGHTED\n \n"
			"An enemy has been sighted in range of this fortified unit. Strike while the iron is hot.\n");
		break;

	case ACTIONTYPE_UPGRADE:
		m_pActionItem->SetText(
			"UPRGADE COMPLETE\n \n"
			"This structure has completed its upgrade.\n");
		break;

	case ACTIONTYPE_UNITREADY:
		m_pActionItem->SetText(
			"UNIT COMPLETED\n \n"
			"A new unit was just constructed here and is now finish. Please choose the next construction task.\n");
		break;

	case ACTIONTYPE_DOWNLOADCOMPLETE:
		m_pActionItem->SetText(
			"DOWNLOAD COMPLETE\n \n"
			"A download has just been completed in your updates grid. Press the 'Download Updates' button to download more updates.\n");
		break;

	case ACTIONTYPE_DOWNLOADUPDATES:
		m_pActionItem->SetText(
			"DOWNLOAD UPDATES\n \n"
			"You can download updates for your structures. Press the 'Download Updates' button to choose an update.\n");
		break;
	}

	// Some action items are handled just by looking at them.
	if (pItem->eActionType == ACTIONTYPE_WELCOME || pItem->eActionType == ACTIONTYPE_CONTROLS || pItem->eActionType == ACTIONTYPE_NEWSTRUCTURE ||
		pItem->eActionType == ACTIONTYPE_AUTOMOVEENEMY || pItem->eActionType == ACTIONTYPE_UPGRADE || pItem->eActionType == ACTIONTYPE_UNITREADY)
		pItem->bHandled = true;

	// Just in case it was turned off because of the tutorials.
	if (pItem->eActionType == ACTIONTYPE_DOWNLOADUPDATES)
		m_bUpdatesBlinking = true;

	CEntityHandle<CSelectable> hSelection(pItem->iUnit);

	if (hSelection != NULL)
	{
		DigitanksGame()->GetCurrentTeam()->SetPrimarySelection(hSelection);
		DigitanksGame()->GetDigitanksCamera()->SetTarget(hSelection->GetOrigin());
	}

	m_flActionItemsLerpGoal = 1;
}

void CHUD::OnAddNewActionItem()
{
	m_bAllActionItemsHandled = false;
}

void CHUD::OnTakeShieldDamage(CDigitank* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, true);

	if (!pVictim->IsAlive() && bDirectHit && flDamage > 0)
		new CHitIndicator(pVictim, L"OVERKILL!");

	else if (bShieldOnly && bDirectHit && flDamage > 0)
		new CHitIndicator(pVictim, L"DIRECT HIT!");
}

void CHUD::OnTakeDamage(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, false);

	if ((pVictim->IsAlive() || bKilled) && bDirectHit && flDamage > 0)
		new CHitIndicator(pVictim, L"DIRECT HIT!");
}

void CHUD::OnDisabled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	if (pVictim->IsAlive())
		new CHitIndicator(pVictim, L"DISABLED!");
}

void CHUD::OnMiss(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	if (pVictim == pAttacker)
		return;

	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	if (pVictim->IsAlive())
		new CHitIndicator(pVictim, L"MISS!");
}

void CHUD::OnAddEntityToTeam(CDigitanksTeam* pTeam, CBaseEntity* pEntity)
{
	m_pSceneTree->OnAddEntityToTeam(pTeam, pEntity);
}

void CHUD::OnRemoveEntityFromTeam(CDigitanksTeam* pTeam, CBaseEntity* pEntity)
{
	m_pSceneTree->OnRemoveEntityFromTeam(pTeam, pEntity);
}

void CHUD::TankSpeak(class CBaseEntity* pTank, const eastl::string& sSpeech)
{
	CDigitank* pDigitank = dynamic_cast<CDigitank*>(pTank);
	if (pDigitank && pDigitank->GetVisibility() == 0)
		return;

	// Cleans itself up.
	new CSpeechBubble(pTank, sSpeech);
}

void CHUD::ClearTurnInfo()
{
	m_pTurnInfo->SetText("");

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() != DigitanksGame()->GetCurrentTeam())
		return;

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
		return;

	m_pTurnInfo->SetText("TURN REPORT\n \n");
}

void CHUD::AppendTurnInfo(const eastl::string16& pszInfo)
{
	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() != DigitanksGame()->GetCurrentTeam())
		return;

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
		return;

	m_pTurnInfo->AppendText("* ");
	m_pTurnInfo->AppendText(pszInfo);
	m_pTurnInfo->AppendText("\n");
}

void CHUD::SetHUDActive(bool bActive)
{
	m_bHUDActive = bActive;

	SetupMenu(m_eMenuMode);

	if (!bActive)
		DigitanksGame()->SetControlMode(MODE_NONE);
}

bool CHUD::IsVisible()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return false;

	return BaseClass::IsVisible();
}

void CHUD::ShowButtonInfo(int iButton)
{
	if (iButton < 0 || iButton >= NUM_BUTTONS)
		return;

	m_pButtonInfo->SetText(m_aszButtonInfos[iButton].c_str());
	m_pButtonInfo->SetSize(m_pButtonInfo->GetWidth(), 9999);
	m_pButtonInfo->SetSize(m_pButtonInfo->GetWidth(), (int)m_pButtonInfo->GetTextHeight());

	int iWidth = DigitanksWindow()->GetWindowWidth();
	int iHeight = DigitanksWindow()->GetWindowHeight();

	m_pButtonInfo->SetPos(iWidth/2 + 720/2 - m_pButtonInfo->GetWidth() - 50, iHeight - 160 - m_pButtonInfo->GetHeight());
}

void CHUD::HideButtonInfo()
{
	m_pButtonInfo->SetText("");
}

bool CHUD::IsButtonInfoVisible()
{
	return m_pButtonInfo->GetText().length() > 0;
}

bool CHUD::IsUpdatesPanelOpen()
{
	if (!m_pUpdatesPanel)
		return false;

	return m_pUpdatesPanel->IsVisible();
}

void CHUD::SlideUpdateIcon(int x, int y)
{
	m_iUpdateIconSlideStartX = x;
	m_iUpdateIconSlideStartY = y;
	m_flUpdateIconSlide = GameServer()->GetGameTime();
}

void ActionSignCallback(CCommand* pCommand, eastl::vector<eastl::string16>& asTokens)
{
	if (asTokens.size() < 2)
		return;

	if (asTokens[1] == L"fight")
		DigitanksWindow()->GetHUD()->ShowFightSign();
	else if (asTokens[1] == L"showdown")
		DigitanksWindow()->GetHUD()->ShowShowdownSign();
}

CCommand actionsign("actionsign", ActionSignCallback);

void CHUD::ShowFightSign()
{
	m_eActionSign = ACTIONSIGN_FIGHT;
	m_flActionSignStart = GameServer()->GetGameTime();
	CSoundLibrary::PlaySound(NULL, L"sound/actionsign.wav");
}

void CHUD::ShowShowdownSign()
{
	m_eActionSign = ACTIONSIGN_SHOWDOWN;
	m_flActionSignStart = GameServer()->GetGameTime();
	CSoundLibrary::PlaySound(NULL, L"sound/actionsign.wav");
}

void CHUD::ChooseActionItemCallback()
{
	eastl::vector<actionitem_t>& aActionItems = DigitanksGame()->GetActionItems();

	size_t iActionItem = ~0;

	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	// Bit of a hack since we don't know what button was pressed we have to look for it.
	for (size_t i = 0; i < m_apActionItemButtons.size(); i++)
	{
		int x, y, w, h;
		m_apActionItemButtons[i]->GetAbsDimensions(x, y, w, h);
		if (mx >= x &&
			my >= y &&
			mx < x + w &&
			my < y + h)
		{
			iActionItem = i;
			break;
		}
	}

	if (iActionItem == ~0)
		return;

	ShowActionItem(iActionItem);
}

void CHUD::CloseActionItemsCallback()
{
	ShowActionItem(~0);
}

void CHUD::ButtonCursorIn0Callback()
{
	ShowButtonInfo(0);
}

void CHUD::ButtonCursorIn1Callback()
{
	ShowButtonInfo(1);
}

void CHUD::ButtonCursorIn2Callback()
{
	ShowButtonInfo(2);
}

void CHUD::ButtonCursorIn3Callback()
{
	ShowButtonInfo(3);
}

void CHUD::ButtonCursorIn4Callback()
{
	ShowButtonInfo(4);
}

void CHUD::ButtonCursorIn5Callback()
{
	ShowButtonInfo(5);
}

void CHUD::ButtonCursorIn6Callback()
{
	ShowButtonInfo(6);
}

void CHUD::ButtonCursorIn7Callback()
{
	ShowButtonInfo(7);
}

void CHUD::ButtonCursorIn8Callback()
{
	ShowButtonInfo(8);
}

void CHUD::ButtonCursorIn9Callback()
{
	ShowButtonInfo(9);
}

void CHUD::ButtonCursorOutCallback()
{
	HideButtonInfo();
}

void CHUD::EndTurnCallback()
{
	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() != DigitanksGame()->GetCurrentTeam())
		return;

	CSoundLibrary::PlaySound(NULL, L"sound/turn.wav");
	DigitanksGame()->EndTurn();
}

void CHUD::OpenUpdatesCallback()
{
	if (m_pUpdatesPanel)
		m_pUpdatesPanel->SetVisible(true);

	m_bUpdatesBlinking = false;

	DigitanksGame()->SetControlMode(MODE_NONE);
}

void CHUD::MoveCallback()
{
	if (!m_bHUDActive)
		return;

	if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		DigitanksGame()->SetControlMode(MODE_NONE);
	else
		DigitanksGame()->SetControlMode(MODE_MOVE);

	SetupMenu();
}

void CHUD::CancelAutoMoveCallback()
{
	if (!m_bHUDActive)
		return;

	DigitanksGame()->SetControlMode(MODE_NONE);

	for (size_t i = 0; i < DigitanksGame()->GetCurrentTeam()->GetNumTanks(); i++)
	{
		if (DigitanksGame()->GetCurrentTeam()->IsSelected(DigitanksGame()->GetCurrentTeam()->GetTank(i)))
			DigitanksGame()->GetCurrentTeam()->GetTank(i)->CancelGoalMovePosition();
	}

	SetupMenu();
}

void CHUD::TurnCallback()
{
	if (!m_bHUDActive)
		return;

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
		DigitanksGame()->SetControlMode(MODE_NONE);
	else
		DigitanksGame()->SetControlMode(MODE_TURN);

	SetupMenu();
}

void CHUD::AimCallback()
{
	if (!m_bHUDActive)
		return;

	if (DigitanksGame()->GetControlMode() == MODE_AIM)
		DigitanksGame()->SetControlMode(MODE_NONE);
	else if (DigitanksGame()->GetPrimarySelectionTank())
	{
		CDigitank* pTank = DigitanksGame()->GetPrimarySelectionTank();
		if (pTank->GetCurrentWeapon() == PROJECTILE_AIRSTRIKE)
		{
			if (pTank->GetNumWeapons())
				pTank->SetCurrentWeapon(pTank->GetWeapon(0));
			else
				pTank->SetCurrentWeapon(WEAPON_NONE);
		}

		DigitanksGame()->SetControlMode(MODE_AIM);
		DigitanksGame()->SetAimTypeByWeapon(pTank->GetCurrentWeapon());
		DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_ARTILLERY_AIM, true);
	}

	SetupMenu();
}

void CHUD::FortifyCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame()->GetPrimarySelectionTank())
		return;

	DigitanksGame()->SetControlMode(MODE_NONE);

	for (size_t i = 0; i < DigitanksGame()->GetCurrentTeam()->GetNumTanks(); i++)
	{
		if (DigitanksGame()->GetCurrentTeam()->IsSelected(DigitanksGame()->GetCurrentTeam()->GetTank(i)))
		{
			DigitanksGame()->GetCurrentTeam()->GetTank(i)->Fortify();
			DigitanksGame()->HandledActionItem(DigitanksGame()->GetCurrentTeam()->GetTank(i));
		}
	}

	SetupMenu(MENUMODE_MAIN);
	UpdateInfo();
}

void CHUD::SentryCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame()->GetPrimarySelectionTank())
		return;

	DigitanksGame()->SetControlMode(MODE_NONE);

	for (size_t i = 0; i < DigitanksGame()->GetCurrentTeam()->GetNumTanks(); i++)
	{
		if (DigitanksGame()->GetCurrentTeam()->IsSelected(DigitanksGame()->GetCurrentTeam()->GetTank(i)))
		{
			DigitanksGame()->GetCurrentTeam()->GetTank(i)->Sentry();
			DigitanksGame()->HandledActionItem(DigitanksGame()->GetCurrentTeam()->GetTank(i));
		}
	}

	SetupMenu(MENUMODE_MAIN);
	UpdateInfo();
}

void CHUD::PromoteCallback()
{
	if (!m_bHUDActive)
		return;

	SetupMenu(MENUMODE_PROMOTE);
}

void CHUD::PromoteAttackCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetPrimarySelectionTank();

	pTank->PromoteAttack();

	SetupMenu(MENUMODE_MAIN);

	UpdateInfo();

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
}

void CHUD::PromoteDefenseCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetPrimarySelectionTank();

	pTank->PromoteDefense();

	SetupMenu(MENUMODE_MAIN);

	UpdateInfo();

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
}

void CHUD::PromoteMovementCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetPrimarySelectionTank();

	pTank->PromoteMovement();

	SetupMenu(MENUMODE_MAIN);

	UpdateInfo();

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
}

void CHUD::FireSpecialCallback()
{
	if (!m_bHUDActive)
		return;

	if (DigitanksGame()->GetControlMode() == MODE_AIM)
		DigitanksGame()->SetControlMode(MODE_NONE);
	else if (DigitanksGame()->GetCurrentTeam() && DigitanksGame()->GetCurrentTeam()->GetPrimarySelectionTank())
	{
		DigitanksGame()->GetCurrentTeam()->GetPrimarySelectionTank()->SetCurrentWeapon(PROJECTILE_AIRSTRIKE);
		DigitanksGame()->SetControlMode(MODE_AIM);
		DigitanksGame()->SetAimTypeByWeapon(PROJECTILE_AIRSTRIKE);
	}

	SetupMenu();
}

void CHUD::BuildMiniBufferCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetPrimarySelectionStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_MINIBUFFER);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_STRATEGY_BUILDBUFFER, true);
}

void CHUD::BuildBufferCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetPrimarySelectionStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_BUFFER);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::BuildBatteryCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetPrimarySelectionStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_BATTERY);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_INGAME_STRATEGY_BUILDBUFFER, true);
}

void CHUD::BuildPSUCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetPrimarySelectionStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_PSU);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::BuildLoaderCallback()
{
	if (!m_bHUDActive)
		return;

	SetupMenu(MENUMODE_LOADERS);
}

void CHUD::BuildInfantryLoaderCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetPrimarySelectionStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_INFANTRYLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::BuildTankLoaderCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetPrimarySelectionStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_TANKLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::BuildArtilleryLoaderCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetPrimarySelectionStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_ARTILLERYLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);
}

void CHUD::BuildUnitCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetPrimarySelectionStructure();

	CLoader* pLoader = dynamic_cast<CLoader*>(pStructure);
	if (!pLoader)
		return;

	pLoader->BeginProduction();
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::BuildScoutCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetPrimarySelectionStructure();

	CCPU* pCPU = dynamic_cast<CCPU*>(pStructure);
	if (!pCPU)
		return;

	pCPU->BeginRogueProduction();
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::BeginUpgradeCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelectionStructure())
		return;

	CStructure* pStructure = DigitanksGame()->GetPrimarySelectionStructure();
	pStructure->BeginUpgrade();

	DigitanksGame()->SetControlMode(MODE_NONE);

	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::CloakCallback()
{
	CDigitank* pDigitank = DigitanksGame()->GetPrimarySelectionTank();

	if (!pDigitank)
		return;

	if (!pDigitank->HasCloak())
		return;

	if (pDigitank->IsCloaked())
		pDigitank->Uncloak();
	else
		pDigitank->Cloak();
}

void CHUD::ChooseWeaponCallback()
{
	if (!m_bHUDActive)
		return;

	DigitanksGame()->SetControlMode(MODE_NONE);

	if (m_pWeaponPanel->IsVisible())
		m_pWeaponPanel->SetVisible(false);
	else
	{
		m_pWeaponPanel->Layout();
		m_pWeaponPanel->SetVisible(true);
	}
}

void CHUD::GoToMainCallback()
{
	SetupMenu(MENUMODE_MAIN);
}

void CHUD::SetNeedsUpdate()
{
	DigitanksWindow()->GetHUD()->m_bNeedsUpdate = true;
}

CDamageIndicator::CDamageIndicator(CBaseEntity* pVictim, float flDamage, bool bShield)
	: CLabel(0, 0, 100, 100, L"")
{
	m_hVictim = pVictim;
	m_flDamage = flDamage;
	m_bShield = bShield;
	m_flTime = GameServer()->GetGameTime();
	m_vecLastOrigin = pVictim->GetOrigin();

	glgui::CRootPanel::Get()->AddControl(this, true);

	Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(pVictim->GetOrigin());
	if (bShield)
		vecScreen += Vector(10, 10, 0);
	SetPos((int)vecScreen.x, (int)vecScreen.y);

	if (m_bShield)
		SetFGColor(Color(255, 255, 255));
	else if (flDamage < 0)
		SetFGColor(Color(0, 255, 0));
	else
		SetFGColor(Color(255, 0, 0));

	int iDamage = (int)flDamage;

	// Don't let it say 0
	if (flDamage > 0 && iDamage < 1)
		iDamage = 1;
	if (flDamage < 0 && iDamage > -1)
		iDamage = -1;

	if (flDamage < 0.5f && flDamage > -0.1f)
	{
		m_flTime = 0;
		SetVisible(false);
	}

	char szDamage[100];
	if (iDamage < 0)
		sprintf(szDamage, "+%d", -iDamage);
	else
		sprintf(szDamage, "-%d", iDamage);
	SetText(szDamage);

	SetFontFaceSize(18);
	SetAlign(CLabel::TA_TOPLEFT);
}

void CDamageIndicator::Destructor()
{
	glgui::CRootPanel::Get()->RemoveControl(this);
}

void CDamageIndicator::Think()
{
	float flFadeTime = 1.0f;

	if (GameServer()->GetGameTime() - m_flTime > flFadeTime)
	{
		Destructor();
		Delete();
		return;
	}

	if (m_hVictim != NULL)
		m_vecLastOrigin = m_hVictim->GetOrigin();

	float flOffset = RemapVal(GameServer()->GetGameTime() - m_flTime, 0, flFadeTime, 10, 20);

	Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(m_vecLastOrigin);
	if (m_bShield)
		vecScreen += Vector(10, 10, 0);

	SetPos((int)(vecScreen.x+flOffset), (int)(vecScreen.y-flOffset));

	SetAlpha((int)RemapVal(GameServer()->GetGameTime() - m_flTime, 0, flFadeTime, 255, 0));

	// Cull tanks behind the camera
	if (GameServer()->GetRenderer()->GetCameraVector().Dot((m_vecLastOrigin-GameServer()->GetCamera()->GetCameraPosition()).Normalized()) < 0)
		SetAlpha(0);

	BaseClass::Think();
}

void CDamageIndicator::Paint(int x, int y, int w, int h)
{
	if (m_bShield)
	{
		int iWidth = GetTextWidth();
		int iHeight = (int)GetTextHeight();
		CRootPanel::PaintRect(x, y-iHeight/2, iWidth, iHeight, Color(0, 0, 0, GetAlpha()/2));
	}

	BaseClass::Paint(x, y, w, h);
}

CHitIndicator::CHitIndicator(CBaseEntity* pVictim, eastl::string16 sMessage)
	: CLabel(0, 0, 200, 100, L"")
{
	m_hVictim = pVictim;
	m_flTime = GameServer()->GetGameTime();
	m_vecLastOrigin = pVictim->GetOrigin();

	glgui::CRootPanel::Get()->AddControl(this, true);

	Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(pVictim->GetOrigin());
	vecScreen.x -= 20;
	vecScreen.y -= 20;
	SetPos((int)vecScreen.x, (int)vecScreen.y);

	SetFGColor(Color(255, 255, 255));

	SetText(sMessage.c_str());
	SetWrap(false);

	SetFontFaceSize(20);
	SetAlign(CLabel::TA_TOPLEFT);
}

void CHitIndicator::Destructor()
{
	glgui::CRootPanel::Get()->RemoveControl(this);
}

void CHitIndicator::Think()
{
	float flFadeTime = 1.0f;

	if (GameServer()->GetGameTime() - m_flTime > flFadeTime)
	{
		Destructor();
		Delete();
		return;
	}

	if (m_hVictim != NULL)
		m_vecLastOrigin = m_hVictim->GetOrigin();

	float flOffset = RemapVal(GameServer()->GetGameTime() - m_flTime, 0, flFadeTime, 10, 20);

	Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(m_vecLastOrigin);
	vecScreen.x -= 20;
	vecScreen.y -= 20;

	SetPos((int)(vecScreen.x+flOffset), (int)(vecScreen.y-flOffset));

	SetAlpha((int)RemapVal(GameServer()->GetGameTime() - m_flTime, 0, flFadeTime, 255, 0));

	// Cull tanks behind the camera
	if (GameServer()->GetRenderer()->GetCameraVector().Dot((m_vecLastOrigin-GameServer()->GetCamera()->GetCameraPosition()).Normalized()) < 0)
		SetAlpha(0);

	BaseClass::Think();
}

void CHitIndicator::Paint(int x, int y, int w, int h)
{
	int iWidth = GetTextWidth();
	int iHeight = (int)GetTextHeight();
	CRootPanel::PaintRect(x, y-iHeight/2, iWidth, iHeight, Color(0, 0, 0, GetAlpha()/2));

	BaseClass::Paint(x, y, w, h);
}

CSpeechBubble::CSpeechBubble(CBaseEntity* pSpeaker, eastl::string sSpeech)
	: CLabel(0, 0, 83*2/3, 47*2/3, L"")
{
	m_hSpeaker = pSpeaker;
	m_flTime = GameServer()->GetGameTime();
	m_vecLastOrigin = pSpeaker->GetOrigin();

	if (pSpeaker)
		m_flRadius = pSpeaker->GetBoundingRadius();
	else
		m_flRadius = 10;

	glgui::CRootPanel::Get()->AddControl(this, (DigitanksGame()->GetGameType() == GAMETYPE_MENU)?false:true);

	SetFGColor(Color(255, 255, 255));

	SetText(sSpeech.c_str());
	SetWrap(false);

	SetFontFaceSize(18);
	SetAlign(CLabel::TA_MIDDLECENTER);
}

void CSpeechBubble::Destructor()
{
	glgui::CRootPanel::Get()->RemoveControl(this);
}

void CSpeechBubble::Think()
{
	float flFadeTime = 3.0f;

	if (!GameServer())
		return;

	if (GameServer()->GetGameTime() - m_flTime > flFadeTime)
	{
		Destructor();
		Delete();
		return;
	}

	if (m_hSpeaker != NULL)
		m_vecLastOrigin = m_hSpeaker->GetOrigin();

	Vector vecUp;
	GameServer()->GetRenderer()->GetCameraVectors(NULL, NULL, &vecUp);

	Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(m_vecLastOrigin);
	Vector vecTop = GameServer()->GetRenderer()->ScreenPosition(m_vecLastOrigin + vecUp*m_flRadius);
	float flWidth = (vecTop - vecScreen).Length()*2 + 10;

	vecScreen.x -= (flWidth/2 + 50);
	vecScreen.y -= flWidth/2;

	SetPos((int)(vecScreen.x), (int)(vecScreen.y));

	SetAlpha((int)RemapValClamped(GameServer()->GetGameTime() - m_flTime, flFadeTime-1, flFadeTime, 255, 0));

	// Cull tanks behind the camera
	if (GameServer()->GetRenderer()->GetCameraVector().Dot((m_vecLastOrigin-GameServer()->GetCamera()->GetCameraPosition()).Normalized()) < 0)
		SetAlpha(0);

	BaseClass::Think();
}

void CSpeechBubble::Paint(int x, int y, int w, int h)
{
	if (!GameServer())
		return;

	if (DigitanksWindow()->GetHUD()->IsUpdatesPanelOpen())
		return;

	do {
		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);
		CHUD::PaintHUDSheet(x, y, w, h, 500, 550, 83, 47, Color(255, 255, 255, GetAlpha()));
	} while (false);

	BaseClass::Paint(x, y, w, h);
}

bool CMouseCapturePanel::MousePressed(int code, int mx, int my)
{
	BaseClass::MousePressed(code, mx, my);
	return true;
}

bool CMouseCapturePanel::MouseReleased(int code, int mx, int my)
{
	BaseClass::MouseReleased(code, mx, my);

	if (DigitanksWindow()->IsMouseDragging())
		return false;

	return true;
}
