#include "hud.h"

#include <GL/glew.h>

#include <geometry.h>
#include <strutils.h>

#include <tinker/cvar.h>

#include <sound/sound.h>
#include <renderer/renderer.h>

#include "digitankswindow.h"
#include "digitanks/digitanksgame.h"
#include "debugdraw.h"
#include "instructor.h"
#include <game/digitanks/structures/cpu.h>
#include <game/digitanks/weapons/projectile.h>
#include <game/digitanks/structures/loader.h>
#include <game/digitanks/structures/props.h>
#include <game/digitanks/structures/collector.h>
#include <game/digitanks/structures/autoturret.h>
#include <game/digitanks/units/mobilecpu.h>
#include <game/digitanks/units/scout.h>
#include <game/digitanks/units/artillery.h>
#include <game/digitanks/units/mechinf.h>
#include <game/digitanks/units/maintank.h>
#include <game/digitanks/dt_camera.h>
#include <game/digitanks/weapons/cameraguided.h>
#include <game/digitanks/campaign/userfile.h>
#include "weaponpanel.h"
#include "scenetree.h"

using namespace glgui;

CVar hud_enable("hud_enable", "on");

CPowerBar::CPowerBar(powerbar_type_t ePowerbarType)
	: CLabel(0, 0, 100, 100, L"", L"text")
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
	if (m_ePowerbarType == POWERBAR_SHIELD)
	{
		CDigitank* pTank = dynamic_cast<CDigitank*>(pSelection);
		if (pTank && pTank->GetShieldMaxStrength())
		{
			sprintf(szLabel, "Shield Strength: %d/%d", (int)(pTank->GetShieldStrength() * pTank->GetShieldMaxStrength()), (int)(pTank->GetShieldMaxStrength() * pTank->GetDefenseScale(true)));
			SetText(szLabel);
		}
		else
			SetText("");
	}
	else if (m_ePowerbarType == POWERBAR_HEALTH)
	{
		if (pSelection->TakesDamage())
		{
			sprintf(szLabel, "Hull Strength: %d/%d", (int)pSelection->GetHealth(), (int)pSelection->GetTotalHealth());
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

	int iSize = 13;
	if (m_ePowerbarType == POWERBAR_SHIELD)
		iSize = 10;

	SetFont(L"text", iSize);
	while (iSize > 0 && GetTextWidth() > GetWidth()-1)
		SetFont(L"text", --iSize);
}

void CPowerBar::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelection())
		return;

	CSelectable* pSelection = DigitanksGame()->GetPrimarySelection();

	if (m_ePowerbarType == POWERBAR_SHIELD)
	{
		CDigitank* pTank = dynamic_cast<CDigitank*>(pSelection);
		if (pTank && pTank->GetShieldMaxStrength())
		{
			float flShield = pTank->GetShieldStrength() * pTank->GetShieldMaxStrength();
			float flShieldMax = pTank->GetShieldMaxStrength() * pTank->GetDefenseScale(true);
			CRootPanel::PaintRect(x+1, y+1, (int)(w * flShield / flShieldMax)-2, h-2, Color(80, 80, 80));
		}
	}
	else if (m_ePowerbarType == POWERBAR_HEALTH)
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
	: CPanel(0, 0, 100, 100),
	m_HUDSheet(L"textures/hud/hud-sheet.txt"),
	m_UnitsSheet(L"textures/hud/units-sheet.txt"),
	m_WeaponsSheet(L"textures/hud/hud-weapons-01.txt"),
	m_ButtonSheet(L"textures/hud/hud-menu-sheet-01.txt"),
	m_DownloadSheet(L"textures/hud/hud-download-sheet-01.txt")
{
	m_bNeedsUpdate = false;

	m_pShieldBar = new CPowerBar(POWERBAR_SHIELD);
	m_pHealthBar = new CPowerBar(POWERBAR_HEALTH);
	m_pAttackPower = new CPowerBar(POWERBAR_ATTACK);
	m_pDefensePower = new CPowerBar(POWERBAR_DEFENSE);
	m_pMovementPower = new CPowerBar(POWERBAR_MOVEMENT);

	AddControl(m_pShieldBar);
	AddControl(m_pHealthBar);
	AddControl(m_pAttackPower);
	AddControl(m_pDefensePower);
	AddControl(m_pMovementPower);

	m_iKeysSheet = CTextureLibrary::AddTexture(L"textures/hud/keys.png");
	m_iActionTanksSheet = CTextureLibrary::AddTexture(L"textures/hud/actionsigns/tanks.png");
	m_iActionSignsSheet = CTextureLibrary::AddTexture(L"textures/hud/actionsigns/signs.png");
	m_iPurchasePanel = CTextureLibrary::AddTexture(L"textures/purchasepanel.png");
	m_iShieldTexture = CTextureLibrary::AddTexture(L"textures/hud/hud-shield.png");

	m_eActionSign = ACTIONSIGN_NONE;

	m_pActionItem = new CLabel(0, 0, 10, 10, L"");
	m_pActionItem->SetFont(L"text");
	m_pCloseActionItems = new CButton(0, 0, 100, 50, L"Close");
	m_pCloseActionItems->SetFont(L"header");
	AddControl(m_pActionItem);
	AddControl(m_pCloseActionItems);
	m_flActionItemsLerp = m_flActionItemsLerpGoal = 0;
	m_flActionItemsWidth = 280;
	m_flSmallActionItemLerp = m_flSmallActionItemLerpGoal = 0;
	m_iCurrentSmallActionItem = ~0;

	m_pButtonPanel = new CMouseCapturePanel();
	AddControl(m_pButtonPanel);

	m_pHowToPlayPanel = new CHowToPlayPanel();
	AddControl(m_pHowToPlayPanel);

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

	m_pAttackInfo = new CLabel(0, 0, 100, 150, L"");
	m_pAttackInfo->SetWrap(false);
	m_pAttackInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pAttackInfo->SetFont(L"text");
	AddControl(m_pAttackInfo);

	m_pScoreboard = new CLabel(0, 0, 100, 150, L"");
	m_pScoreboard->SetWrap(false);
	m_pScoreboard->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pScoreboard->SetFont(L"text", 10);
	AddControl(m_pScoreboard);

	m_pTankInfo = new CLabel(0, 0, 100, 100, L"");
	m_pTankInfo->SetFont(L"text", 10);
	AddControl(m_pTankInfo);

	m_pTurnInfo = new CLabel(0, 0, 100, 100, L"");
	m_pTurnInfo->SetFont(L"text", 10);
	AddControl(m_pTurnInfo);

	m_pResearchInfo = new CLabel(0, 0, 100, 100, L"");
	m_pResearchInfo->SetFont(L"text");
	AddControl(m_pResearchInfo);

	m_pButtonInfo = new CLabel(0, 0, 100, 100, L"");
	m_pButtonInfo->SetFont(L"text");
	AddControl(m_pButtonInfo);

	m_pPressEnter = new CLabel(0, 0, 100, 100, L"");
	m_pPressEnter->SetFont(L"text");
	AddControl(m_pPressEnter);

	SetupMenu(MENUMODE_MAIN);

	m_pDemoNotice = new CLabel(0, 0, 100, 20, L"");
	m_pDemoNotice->SetFont(L"text");
	AddControl(m_pDemoNotice);

	m_pDemoNotice->SetAlign(CLabel::TA_TOPLEFT);
	m_pDemoNotice->SetPos(20, 20);
	m_pDemoNotice->SetText("");

	m_pPowerInfo = new CLabel(0, 0, 200, 20, L"");
	AddControl(m_pPowerInfo);

	m_pPowerInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pPowerInfo->SetPos(200, 20);
	m_pPowerInfo->SetFGColor(Color(220, 220, 255));
	m_pPowerInfo->SetFont(L"text");
	m_pPowerInfo->SetCursorInListener(this, ShowPowerInfo);
	m_pPowerInfo->SetCursorOutListener(this, HideTeamInfo);

	m_pFleetInfo = new CLabel(0, 0, 200, 20, L"");
	AddControl(m_pFleetInfo);

	m_pFleetInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pFleetInfo->SetPos(200, 20);
	m_pFleetInfo->SetFGColor(Color(220, 220, 255));
	m_pFleetInfo->SetFont(L"text");
	m_pFleetInfo->SetCursorInListener(this, ShowFleetInfo);
	m_pFleetInfo->SetCursorOutListener(this, HideTeamInfo);

	m_pBandwidthInfo = new CLabel(0, 0, 200, 20, L"");
	AddControl(m_pBandwidthInfo);

	m_pBandwidthInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pBandwidthInfo->SetPos(200, 20);
	m_pBandwidthInfo->SetFGColor(Color(220, 220, 255));
	m_pBandwidthInfo->SetFont(L"text");
	m_pBandwidthInfo->SetCursorInListener(this, ShowBandwidthInfo);
	m_pBandwidthInfo->SetCursorOutListener(this, HideTeamInfo);

	m_pTeamInfo = new CLabel(0, 0, 200, 20, L"");
	AddControl(m_pTeamInfo);

	m_pTeamInfo->SetAlign(CLabel::TA_TOPLEFT);
	m_pTeamInfo->SetPos(200, 20);
	m_pTeamInfo->SetFGColor(Color(255, 255, 255));
	m_pTeamInfo->SetFont(L"text");

	m_pUpdatesButton = new CPictureButton(L"Download Grid");
	m_pUpdatesButton->SetClickedListener(this, OpenUpdates);
	m_pUpdatesButton->ShowBackground(false);
	m_pUpdatesButton->SetTooltip(L"Download Grid");
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
	SetButtonSheetTexture(m_pTurnButton, &m_HUDSheet, "EndTurn");
	m_pTurnButton->SetClickedListener(this, EndTurn);
	m_pTurnButton->ShowBackground(false);
	AddControl(m_pTurnButton);

	m_flAttackInfoAlpha = m_flAttackInfoAlphaGoal = 0;

	m_flTurnInfoLerp = m_flTurnInfoLerpGoal = 0;
	m_flTurnInfoHeight = m_flTurnInfoHeightGoal = 0;

	m_iTurnSound = CSoundLibrary::Get()->AddSound(L"sound/turn.wav");

	m_pSpacebarHint = new CLabel(0, 0, 200, 20, L"");
	m_pSpacebarHint->SetAlign(CLabel::TA_MIDDLECENTER);
	m_pSpacebarHint->SetFont(L"text");
	AddControl(m_pSpacebarHint);

	//m_iCompetitionWatermark = CTextureLibrary::AddTexture(L"textures/competition.png");
}

void CHUD::Layout()
{
	SetSize(GetParent()->GetWidth(), GetParent()->GetHeight());

	int iWidth = DigitanksWindow()->GetWindowWidth();
	int iHeight = DigitanksWindow()->GetWindowHeight();

	m_pAttackInfo->SetPos(iWidth - 165, iHeight - 150 - 90 - 10);
	m_pAttackInfo->SetSize(165, 90);

	m_pShieldBar->SetPos(iWidth/2 - 720/2 + 170, iHeight - 142);
	m_pShieldBar->SetSize(200, 15);

	m_pHealthBar->SetPos(iWidth/2 - 720/2 + 170, iHeight - 124);
	m_pHealthBar->SetSize(200, 20);

	m_pAttackPower->SetPos(iWidth/2 - 720/2 + 170, iHeight - 90);
	m_pAttackPower->SetSize(200, 20);

	m_pDefensePower->SetPos(iWidth/2 - 720/2 + 170, iHeight - 60);
	m_pDefensePower->SetSize(200, 20);

	m_pMovementPower->SetPos(iWidth/2 - 720/2 + 170, iHeight - 30);
	m_pMovementPower->SetSize(200, 20);

	bool bShowCPUStuff = false;
	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() && DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetPrimaryCPU())
		bShowCPUStuff = true;

	m_pActionItem->SetPos(iWidth - 280, 70);
	m_pActionItem->SetSize(220, 250);
	m_pActionItem->SetAlign(CLabel::TA_TOPLEFT);
	m_pCloseActionItems->SetSize(130, 25);
	m_pCloseActionItems->SetPos(iWidth - 225, 398);
	m_pCloseActionItems->SetClickedListener(this, CloseActionItems);

	m_pActionItem->SetVisible(bShowCPUStuff);
	m_pCloseActionItems->SetVisible(bShowCPUStuff);

	for (size_t i = 0; i < m_apActionItemButtons.size(); i++)
	{
		RemoveControl(m_apActionItemButtons[i]);
		m_apActionItemButtons[i]->Destructor();
		m_apActionItemButtons[i]->Delete();
	}

	m_apActionItemButtons.clear();

	CDigitanksTeam* pLocalCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (pLocalCurrentTeam && bShowCPUStuff)
	{
		size_t iItemButtonSize = 30;
		for (size_t i = 0; i < pLocalCurrentTeam->GetNumActionItems(); i++)
		{
			CPictureButton* pButton = new CPictureButton(sprintf(L"%d", i));
			AddControl(pButton);
			pButton->SetSize(iItemButtonSize, iItemButtonSize);
			pButton->SetPos(iWidth - iItemButtonSize - 10, 120 + (iItemButtonSize+10)*i);
			pButton->SetClickedListener(this, ChooseActionItem);
			pButton->SetCursorInListener(this, ShowSmallActionItem);
			pButton->SetCursorOutListener(this, HideSmallActionItem);

			CEntityHandle<CDigitanksEntity> hUnit(pLocalCurrentTeam->GetActionItem(i)->iUnit);

			switch (pLocalCurrentTeam->GetActionItem(i)->eActionType)
			{
			case ACTIONTYPE_WELCOME:
				// Use the fleet logo, which is also the digitanks logo, for the welcome icon.
				SetButtonSheetTexture(pButton, &m_HUDSheet, "FleetPointsIcon");
				pButton->SetTooltip(L"Intro");
				break;

			case ACTIONTYPE_CONTROLS:
				// Use the fleet logo, which is also the digitanks logo, for the welcome icon.
				SetButtonSheetTexture(pButton, &m_HUDSheet, "FleetPointsIcon");
				pButton->SetTooltip(L"Controls");
				break;

			case ACTIONTYPE_NEWSTRUCTURE:
				if (hUnit != NULL)
				{
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_UnitsSheet.GetSheet(), sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(L"Structure Complete");
				}
				break;

			case ACTIONTYPE_AUTOMOVECANCELED:
				if (hUnit != NULL)
				{
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_UnitsSheet.GetSheet(), sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(L"Move Canceled");
				}
				break;

			case ACTIONTYPE_AUTOMOVEENEMY:
				if (hUnit != NULL)
				{
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_UnitsSheet.GetSheet(), sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(L"Enemy Sighted");
				}
				break;

			case ACTIONTYPE_UNITDAMAGED:
				if (hUnit != NULL)
				{
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_UnitsSheet.GetSheet(), sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(L"Unit Damaged");
				}
				break;

			case ACTIONTYPE_FORTIFIEDENEMY:
				if (hUnit != NULL)
				{
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_UnitsSheet.GetSheet(), sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(L"Enemy Sighted");
				}
				break;

			case ACTIONTYPE_UNITAUTOMOVE:
				if (hUnit != NULL)
				{
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_UnitsSheet.GetSheet(), sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(L"Move Completed");
				}
				break;

			case ACTIONTYPE_UNITORDERS:
				if (hUnit != NULL)
				{
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_UnitsSheet.GetSheet(), sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(L"Orders Needed");
				}
				break;

			case ACTIONTYPE_UPGRADE:
				if (hUnit != NULL)
				{
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_UnitsSheet.GetSheet(), sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(L"Upgrade Compeleted");
				}
				break;

			case ACTIONTYPE_UNITREADY:
				if (hUnit != NULL)
				{
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(DigitanksWindow()->GetHUD()->m_UnitsSheet.GetSheet(), sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(L"Unit Ready");
				}
				break;

			case ACTIONTYPE_DOWNLOADUPDATES:
				SetButtonSheetTexture(pButton, &m_HUDSheet, "BandwidthIcon");
				pButton->SetTooltip(L"Download Grid");
				break;

			case ACTIONTYPE_DOWNLOADCOMPLETE:
				SetButtonSheetTexture(pButton, &m_HUDSheet, "BandwidthIcon");
				pButton->SetTooltip(L"Download Complete");
				break;
			}

			m_apActionItemButtons.push_back(pButton);
		}
	}

	m_pButtonPanel->SetPos(iWidth/2 - 720/2 + 380, iHeight - 140);
	m_pButtonPanel->SetRight(m_pButtonPanel->GetLeft() + 330);
	m_pButtonPanel->SetBottom(iHeight - 10);

	m_pHowToPlayPanel->Layout();

	for (size_t i = 0; i < NUM_BUTTONS; i++)
	{
		m_apButtons[i]->SetSize(50, 50);
		m_apButtons[i]->SetPos(20 + 60*(i%5), 10 + 60*(i/5));
	}

	m_pTankInfo->SetSize(140, 240);
	m_pTankInfo->SetPos(10, iHeight - m_pTankInfo->GetHeight() + 10 + 7);
	m_pTankInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pTankInfo->SetWrap(true);

	m_pTurnInfo->SetSize(248, 150);
	m_pTurnInfo->SetPos(iWidth/2 - m_pTurnInfo->GetWidth()/2, 36);
	m_pTurnInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pTurnInfo->SetWrap(true);

	if (pLocalCurrentTeam)
		m_pTurnInfo->SetText(pLocalCurrentTeam->GetTurnInfo());
	else
		m_pTurnInfo->SetText(L"");

	m_pResearchInfo->SetSize(640, 25);
	m_pResearchInfo->SetPos(iWidth/2 - m_pResearchInfo->GetWidth()/2, 0);
	m_pResearchInfo->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pResearchInfo->SetWrap(false);

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

	m_pUpdatesButton->SetSize(35, 35);
	m_pUpdatesButton->SetPos(iWidth/2 - 617/2 - 35, 0);
	m_pUpdatesButton->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pUpdatesButton->SetWrap(false);
	m_pUpdatesButton->SetVisible(bShowCPUStuff);

	m_pUpdatesPanel->Layout();

	m_pWeaponPanel->Layout();

	m_pSceneTree->Layout();

	m_pPressEnter->SetDimensions(iWidth/2 - 100/2, iHeight*2/3, 100, 50);
	m_pPressEnter->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
	m_pPressEnter->SetWrap(false);

	m_pPowerInfo->SetAlign(CLabel::TA_LEFTCENTER);
	m_pPowerInfo->SetPos(iWidth - 160, 12);
	m_pPowerInfo->SetSize(80, 15);
	m_pPowerInfo->SetWrap(false);
	m_pPowerInfo->SetVisible(bShowCPUStuff);

	m_pFleetInfo->SetAlign(CLabel::TA_LEFTCENTER);
	m_pFleetInfo->SetPos(iWidth - 160, 42);
	m_pFleetInfo->SetSize(30, 15);
	m_pFleetInfo->SetWrap(false);
	m_pFleetInfo->SetVisible(bShowCPUStuff);

	m_pBandwidthInfo->SetAlign(CLabel::TA_LEFTCENTER);
	m_pBandwidthInfo->SetPos(iWidth - 160, 72);
	m_pBandwidthInfo->SetSize(150, 15);
	m_pBandwidthInfo->SetWrap(false);
	m_pBandwidthInfo->SetVisible(bShowCPUStuff);

	m_pTurnButton->SetPos(iWidth - 140, iHeight - 105);
	m_pTurnButton->SetSize(140, 105);

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam())
	{
		size_t iSheet;
		int sx, sy, sw, sh, tw, th;
		m_pUpdatesPanel->GetTextureForUpdateItem(DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetUpdateDownloading(), iSheet, sx, sy, sw, sh, tw, th);
		m_pUpdatesButton->SetSheetTexture(iSheet, sx, sy, sw, sh, tw, th);
	}

	if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
	{
		// Don't clear it in the start, we want dead tanks to remain in the list so we can mark them asploded.

		m_ahScoreboardTanks.resize(DigitanksGame()->GetNumTeams());

		for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
		{
			CDigitanksTeam* pTeam = DigitanksGame()->GetDigitanksTeam(i);
			eastl::map<size_t, CEntityHandle<CDigitank> >& ahTeamTanks = m_ahScoreboardTanks[i];

			for (size_t j = 0; j < pTeam->GetNumTanks(); j++)
			{
				CDigitank* pTank = pTeam->GetTank(j);
				ahTeamTanks[pTank->GetHandle()] = pTank;
			}
		}
	}

	UpdateTurnButton();
	UpdateScoreboard();
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

		bool bSpotVisible = false;
		if (DigitanksGame()->GetCurrentLocalDigitanksTeam())
			bSpotVisible = DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetVisibilityAtPoint(vecEntityPoint) > 0.2f;

		if (pHit && dynamic_cast<CSelectable*>(pHit) && bSpotVisible)
			DigitanksWindow()->SetMouseCursor(MOUSECURSOR_SELECT);

		if (pHit)
		{
			if (bSpotVisible)
			{
				CDigitanksEntity* pDTHit = dynamic_cast<CDigitanksEntity*>(pHit);
				if (pDTHit)
					SetTooltip(pDTHit->GetEntityName());
				else
				{
					if (DigitanksGame()->GetTerrain()->IsPointInTrees(vecEntityPoint))
						SetTooltip(L"Trees");
					else if (DigitanksGame()->GetTerrain()->IsPointOverWater(vecEntityPoint))
						SetTooltip(L"Interference");
					else if (DigitanksGame()->GetTerrain()->IsPointOverLava(vecEntityPoint))
						SetTooltip(L"Lava");
					else
						SetTooltip(L"");
				}
			}
			else
				SetTooltip(L"");
		}
	}
	else
	{
		bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecEntityPoint, &pHit);
		bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecTerrainPoint, NULL, CG_TERRAIN|CG_PROP);

		SetTooltip(L"");
	}

	if (pCurrentTank)
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		{
			float flMoveDistance = pCurrentTank->GetRemainingMovementDistance();
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
			{
				if (pDTHit->GetUnitType() == UNIT_SCOUT && pCurrentTank->GetCurrentWeapon() != WEAPON_INFANTRYLASER)
					DigitanksWindow()->SetMouseCursor(MOUSECURSOR_AIM);
				else if (pCurrentTank->IsInsideMaxRange(pDTHit->GetOrigin()))
					DigitanksWindow()->SetMouseCursor(MOUSECURSOR_AIMENEMY);
				else
					DigitanksWindow()->SetMouseCursor(MOUSECURSOR_AIM);
			}
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

	bool bShowCPUStuff = false;
	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() && DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetPrimaryCPU())
		bShowCPUStuff = true;

	if (!bShowCPUStuff)
		m_flActionItemsLerp = m_flActionItemsLerpGoal = 0;

	m_flActionItemsLerp = Approach(m_flActionItemsLerpGoal, m_flActionItemsLerp, GameServer()->GetFrameTime());
	m_flSmallActionItemLerp = Approach(m_flSmallActionItemLerpGoal, m_flSmallActionItemLerp, GameServer()->GetFrameTime() * 4);

	int iWidth = DigitanksWindow()->GetWindowWidth();
	m_pActionItem->SetPos(iWidth - 300 + (int)(Lerp(1-m_flActionItemsLerp, 0.2f) * m_flActionItemsWidth), 130);
	m_pActionItem->SetAlpha((int)(m_flActionItemsLerp*255));
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

			if (pDTHit && pDTHit->GetVisibility() <= 0.1f)
				vecPreviewAim = vecTerrainPoint;
			else if (pDigitankHit && pDigitankHit->IsScout())
			{
				// Scouts are hard to hit by projectiles because they float so far above the surface.
				if (pCurrentTank->GetCurrentWeapon() == WEAPON_INFANTRYLASER)
					vecPreviewAim = DigitanksGame()->GetTerrain()->SetPointHeight(pDigitankHit->GetOrigin());
				else
					vecPreviewAim = vecTerrainPoint;
			}
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

	if (m_bHUDActive && bMouseOnGrid && DigitanksGame()->GetCurrentLocalDigitanksTeam())
	{
		if (DigitanksGame()->GetControlMode() == MODE_BUILD)
		{
			if (DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetPrimaryCPU())
				DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetPrimaryCPU()->SetPreviewBuild(vecEntityPoint);
		}
	}

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() && m_apActionItemButtons.size())
	{
		for (size_t i = 0; i < DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetNumActionItems(); i++)
		{
			if (i >= m_apActionItemButtons.size())
				break;

			if (DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetActionItem(i)->bHandled)
				m_apActionItemButtons[i]->SetAlpha(100);
			else
				m_apActionItemButtons[i]->SetAlpha((int)RemapVal(Oscillate(GameServer()->GetGameTime(), 1), 0, 1, 100, 255));
		}
	}

	if (m_eMenuMode == MENUMODE_MAIN && m_bHUDActive)
	{
		eastl::string sTutorialName;
		if (DigitanksWindow()->GetInstructor() && DigitanksWindow()->GetInstructor()->GetCurrentTutorial())
			sTutorialName = DigitanksWindow()->GetInstructor()->GetCurrentTutorial()->m_sTutorialName;

		if (pCurrentTank && pCurrentTank->GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam() && sTutorialName == "artillery-aim")
		{
			float flRamp = Oscillate(GameServer()->GetGameTime(), 1);
			int iColor = (int)RemapVal(flRamp, 0, 1, 0, 150);
			m_apButtons[7]->SetButtonColor(Color(iColor, 0, 0));
		}
		else if (pCurrentTank && pCurrentTank->GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam() && sTutorialName == "strategy-deploy" && dynamic_cast<CMobileCPU*>(pCurrentTank))
		{
			float flRamp = Oscillate(GameServer()->GetGameTime(), 1);
			m_apButtons[8]->SetButtonColor(Color(0, 0, (int)RemapVal(flRamp, 0, 1, 0, 250)));
		}
		// Don't blink other buttons if we're trying to blink this one.
		else
		{
			if (pCurrentTank && pCurrentTank->GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam() && pCurrentTank->HasBonusPoints())
			{
				float flRamp = Oscillate(GameServer()->GetGameTime(), 1);
				m_apButtons[4]->SetButtonColor(Color((int)RemapVal(flRamp, 0, 1, 0, 250), (int)RemapVal(flRamp, 0, 1, 0, 200), 0));
			}

			if (pCurrentTank && pCurrentTank->GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam() && pCurrentTank->HasSpecialWeapons())
			{
				float flRamp = Oscillate(GameServer()->GetGameTime(), 1);
				m_apButtons[9]->SetButtonColor(Color((int)RemapVal(flRamp, 0, 1, 0, 250), (int)RemapVal(flRamp, 0, 1, 0, 200), 0));
			}
		}
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

	m_pUpdatesButton->SetVisible(bShowCPUStuff && !!DigitanksGame()->GetUpdateGrid());

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

	IControl* pMouseControl = glgui::CRootPanel::Get()->GetHasCursor();
	if (DigitanksGame()->GetDigitanksCamera()->HasCameraGuidedMissile())
	{
		DigitanksWindow()->SetMouseCursor(MOUSECURSOR_NONE);
		SetTooltip(L"");
	}
	else if (pMouseControl)
	{
		if (dynamic_cast<CHUD*>(pMouseControl))
		{
			// Nothing.
		}
		else if (pMouseControl->GetTooltip().length() > 0)
		{
			DigitanksWindow()->SetMouseCursor(MOUSECURSOR_NONE);
			SetTooltip(L"");
		}
		else if (dynamic_cast<glgui::CButton*>(pMouseControl))
		{
			DigitanksWindow()->SetMouseCursor(MOUSECURSOR_NONE);
			SetTooltip(L"");
		}
	}

	if (m_bBlinkTurnButton)
		m_pTurnButton->SetAlpha(Lerp(Oscillate(GameServer()->GetGameTime(), 1.0f), 0.8f));
	else
		m_pTurnButton->SetAlpha(1.0f);

	SetVisible(hud_enable.GetBool() || DigitanksGame()->GetDigitanksCamera()->HasCameraGuidedMissile());
}

#ifdef _DEBUG
#define SHOW_FPS "1"
#else
#define SHOW_FPS "0"
#endif

CVar show_fps("show_fps", SHOW_FPS);

void CHUD::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	if (GameServer()->IsLoading())
		return;

	if (DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

	if (show_fps.GetBool())
	{
		float flFontHeight = glgui::CLabel::GetFontHeight(L"text", 10);
		eastl::string16 sFPS = sprintf(L"Time: %.2f", GameServer()->GetGameTime());
		glgui::CLabel::PaintText(sFPS, sFPS.length(), L"text", 10, 5, flFontHeight + 5);
		sFPS = sprintf(L"FPS: %d", (int)(1/GameServer()->GetFrameTime()));
		glgui::CLabel::PaintText(sFPS, sFPS.length(), L"text", 10, 5, flFontHeight*2 + 5);

		Vector vecTerrainPoint;
		if (DigitanksWindow()->GetMouseGridPosition(vecTerrainPoint, NULL, CG_TERRAIN))
		{
			sFPS = sprintf(L"%.2f, %.2f", vecTerrainPoint.x, vecTerrainPoint.z);
			glgui::CLabel::PaintText(sFPS, sFPS.length(), L"text", 10, 5, flFontHeight*3 + 5);
		}
	}

	if (DigitanksGame()->GetDigitanksCamera()->HasCameraGuidedMissile())
	{
		PaintCameraGuidedMissile(x, y, w, h);
		return;
	}

	int iWidth = DigitanksWindow()->GetWindowWidth();
	int iHeight = DigitanksWindow()->GetWindowHeight();

	CDigitanksTeam* pCurrentLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	if ((DigitanksGame()->GetGameType() == GAMETYPE_STANDARD || DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN) && pCurrentLocalTeam && pCurrentLocalTeam->GetPrimaryCPU())
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);

		PaintHUDSheet("TeamInfoBackground", iWidth - 208, 0, 208, 341);
	}

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);

		if (!pDTEntity)
			continue;

		Vector vecOrigin = pEntity->GetOrigin();

		Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(vecOrigin);

		float flRadius = pDTEntity->GetBoundingRadius();

		Vector vecUp;
		Vector vecForward;
		GameServer()->GetRenderer()->GetCameraVectors(&vecForward, NULL, &vecUp);

		// Cull tanks behind the camera
		if (vecForward.Dot((vecOrigin-GameServer()->GetCamera()->GetCameraPosition()).Normalized()) < 0)
			continue;

		if (pDTEntity->IsObjective() && pDTEntity->IsActive())
		{
			eastl::string16 sText = sprintf(L"Objective: %s", pDTEntity->GetEntityName());
			float flWidth = CLabel::GetTextWidth(sText, sText.length(), L"text", 13);
			CLabel::PaintText(sText, sText.length(), L"text", 13, vecScreen.x - flWidth/2, vecScreen.y + 60);

			CRenderingContext c(GameServer()->GetRenderer());
			c.SetBlend(BLEND_ADDITIVE);
			PaintSheet(&m_HUDSheet, "Objective", (int)vecScreen.x - 50, (int)vecScreen.y - 50, 100, 100);
		}

		if (pDTEntity->GetVisibility() == 0)
			continue;

		CSelectable* pSelectable = dynamic_cast<CSelectable*>(pEntity);

		if (!pSelectable)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);

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

				if (pTank->HasGoalMovePosition())
				{
					float flDistance = (pTank->GetRealOrigin() - pTank->GetGoalMovePosition()).Length();
					int iTurns = (int)(flDistance/pTank->GetMaxMovementDistance())+1;

					eastl::string16 sTurns = sprintf(L":%d", iTurns);
					CLabel::PaintText(sTurns, sTurns.length(), L"text", 10, vecScreen.x + flWidth/2 - 10, vecScreen.y - flWidth/2 + CLabel::GetFontHeight(L"text", 10) - 2);
				}

				if (pTank->IsFortified() || pTank->IsFortifying())
				{
					eastl::string16 sTurns = sprintf(L"+%d", pTank->GetFortifyLevel());

					float flYPosition = vecScreen.y + flWidth/2;
					float flXPosition = vecScreen.x + flWidth/2;
					float flIconSize = 10;

					CRenderingContext c(GameServer()->GetRenderer());
					c.SetBlend(BLEND_ALPHA);

					Rect rArea = m_ButtonSheet.GetArea("Fortify");
					CBaseControl::PaintSheet(m_ButtonSheet.GetSheet(),
						(int)(flXPosition - 14 - flIconSize), (int)(flYPosition - flIconSize)-1, (int)flIconSize, (int)flIconSize,
						rArea.x, rArea.y, rArea.w, rArea.y, m_ButtonSheet.GetSheetWidth(), m_ButtonSheet.GetSheetHeight());
					CLabel::PaintText(sTurns, sTurns.length(), L"text", 10, flXPosition - 13, flYPosition - 3);
				}
			}

			CStructure* pStructure = dynamic_cast<CStructure*>(pSelectable);
			if (pStructure)
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

				if (pStructure->IsConstructing() || pStructure->IsUpgrading())
					CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y - flWidth/2 - 2), (int)(flWidth*iTurnsProgressed/iTotalTurns), 3, Color(255, 255, 0));

				if (pStructure->IsConstructing())
				{
					eastl::string16 sTurns = sprintf(L":%d", pStructure->GetTurnsRemainingToConstruct());
					CLabel::PaintText(sTurns, sTurns.length(), L"text", 10, vecScreen.x + flWidth/2 - 10, vecScreen.y - flWidth/2 + CLabel::GetFontHeight(L"text", 10) - 2);
				}
				else if (pStructure->IsUpgrading())
				{
					eastl::string16 sTurns = sprintf(L":%d", pStructure->GetTurnsRemainingToUpgrade());
					CLabel::PaintText(sTurns, sTurns.length(), L"text", 10, vecScreen.x + flWidth/2 - 10, vecScreen.y - flWidth/2 + CLabel::GetFontHeight(L"text", 10) - 2);
				}

				if (pStructure->GetUnitType() == STRUCTURE_CPU)
				{
					CCPU* pCPU = static_cast<CCPU*>(pStructure);
					if (pCPU->IsProducing())
					{
						eastl::string16 sTurns = L":1";	// It only ever takes one turn to make a rogue.
						CLabel::PaintText(sTurns, sTurns.length(), L"text", 10, vecScreen.x + flWidth/2 - 10, vecScreen.y - flWidth/2 + CLabel::GetFontHeight(L"text", 10) - 2);
					}
				}

				if (pStructure->GetUnitType() == STRUCTURE_TANKLOADER || pStructure->GetUnitType() == STRUCTURE_INFANTRYLOADER || pStructure->GetUnitType() == STRUCTURE_ARTILLERYLOADER)
				{
					CLoader* pLoader = static_cast<CLoader*>(pStructure);
					if (pLoader->IsProducing())
					{
						eastl::string16 sTurns = sprintf(L":%d", pLoader->GetTurnsRemainingToProduce());
						CLabel::PaintText(sTurns, sTurns.length(), L"text", 10, vecScreen.x + flWidth/2 - 10, vecScreen.y - flWidth/2 + CLabel::GetFontHeight(L"text", 10) - 2);
					}
				}
			}
		}

		if (pTank && pTank->IsDisabled())
		{
			CRenderingContext c(GameServer()->GetRenderer());
			c.SetBlend(BLEND_ALPHA);

			int iWidth = 77*2/3;
			int iHeight = 39*2/3;

			CHUD::PaintHUDSheet("TankDisabled", (int)(vecScreen.x + flWidth/2), (int)(vecScreen.y - flWidth + 2), iWidth, iHeight, Color(255, 255, 255, 255));

			int iFontSize = 17;

			eastl::string16 sZZZ = L"zzz";
			float flTextWidth = CLabel::GetTextWidth(sZZZ, sZZZ.length(), L"smileys", iFontSize);
			float flTextHeight = CLabel::GetFontHeight(L"smileys", iFontSize);

			float flLerp = fmod(GameServer()->GetGameTime(), 2.0f);
			if (flLerp < 0.5f)
				sZZZ = L"z";
			else if (flLerp < 1.0f)
				sZZZ = L"zz";

			CLabel::PaintText(sZZZ, sZZZ.length(), L"smileys", iFontSize, vecScreen.x + flWidth/2 + iWidth/2 - flTextWidth/2, vecScreen.y - flWidth + iHeight/2 + 3);
		}

		if (m_bHUDActive && pTank && DigitanksGame()->GetCurrentTeam()->IsPrimarySelection(pTank) && DigitanksGame()->GetControlMode() == MODE_AIM)
			UpdateInfo();
	}

	bool bShowCPUStuff = false;
	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() && DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetPrimaryCPU())
		bShowCPUStuff = true;

	if (bShowCPUStuff)
		m_pCloseActionItems->Paint();

	do {
		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);

		// Main control pannel
		PaintHUDSheet("MainControlPanel", iWidth/2 - 720/2, iHeight-160, 720, 160);

		if (bShowCPUStuff && (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD || DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN))
		{
			if (m_bUpdatesBlinking)
			{
				int iArrowWidth = 50;
				int iArrowHeight = 82;
				PaintHUDSheet("UpdatesArrow", m_pUpdatesButton->GetLeft() + m_pUpdatesButton->GetWidth()/2 - iArrowWidth/2, 40 + (int)(Lerp(Oscillate(GameServer()->GetGameTime(), 1), 0.8f)*20), iArrowWidth, iArrowHeight);
			}

			PaintHUDSheet("PowerIcon", iWidth - 190, 10, 20, 20);
			PaintHUDSheet("FleetPointsIcon", iWidth - 190, 40, 20, 20);
			PaintHUDSheet("BandwidthIcon", iWidth - 190, 70, 20, 20);

			PaintHUDSheet("TurnReport", m_pTurnInfo->GetLeft()-15, m_pTurnInfo->GetBottom()-585, 278, 600);

			int iResearchWidth = 617;
			PaintHUDSheet("ResearchBar", iWidth/2 - iResearchWidth/2, 0, iResearchWidth, 35);

			CRootPanel::PaintRect(iWidth/2 - iResearchWidth/2 - 34, 0, 34, 35, Color(0, 0, 0, 150));

			if (pCurrentLocalTeam && pCurrentLocalTeam->GetUpdateDownloading())
			{
				CUpdateItem* pItem = pCurrentLocalTeam->GetUpdateDownloading();

				int iDownloadedWidth = 580;

				float flUpdateDownloaded = pCurrentLocalTeam->GetUpdateDownloaded();
				if (flUpdateDownloaded > pItem->m_flSize)
					flUpdateDownloaded = pItem->m_flSize;
				int iResearchCompleted = (int)(iDownloadedWidth*(flUpdateDownloaded/pItem->m_flSize));
				int iResearchDue = (int)(iDownloadedWidth*(pCurrentLocalTeam->GetBandwidth()/pItem->m_flSize));
				if (iResearchCompleted + iResearchDue > iDownloadedWidth)
					iResearchDue -= (iResearchCompleted + iResearchDue) - iDownloadedWidth;

				CRootPanel::PaintRect(iWidth/2 - iDownloadedWidth/2, 3, iResearchCompleted, 20, Color(0, 200, 100, 255));
				CRootPanel::PaintRect(iWidth/2 - iDownloadedWidth/2 + iResearchCompleted, 3, iResearchDue, 20, Color(0, 200, 100, 100));

				size_t iItemSheet;
				int sx, sy, sw, sh, tw, th;
				m_pUpdatesPanel->GetTextureForUpdateItem(pItem, iItemSheet, sx, sy, sw, sh, tw, th);

				float flSlideTime = 0.8f;
				if (iItemSheet && GameServer()->GetGameTime() - m_flUpdateIconSlide < flSlideTime)
				{
					float flSlideLerp = Lerp(GameServer()->GetGameTime() - m_flUpdateIconSlide, flSlideTime);
					int iIconX = (int)RemapValClamped(flSlideLerp, 0.0f, 1.0f, (float)m_iUpdateIconSlideStartX, (float)(iWidth/2 - iResearchWidth/2 - 35));
					int iIconY = (int)RemapValClamped(flSlideLerp, 0.0f, 1.0f, (float)m_iUpdateIconSlideStartY, 0.0f);
					int iButtonSize = (int)RemapValClamped(flSlideLerp, 0.0f, 1.0f, (float)m_pUpdatesPanel->GetButtonSize(), 35.0f);

					CRootPanel::PaintSheet(iItemSheet, iIconX, iIconY, iButtonSize, iButtonSize, sx, sy, sw, sh, tw, th);
				}
			}
		}

		if (m_flAttackInfoAlpha > 0)
			PaintHUDSheet("AttackInfo", iWidth-175, m_pAttackInfo->GetTop()-15, 175, 110, Color(255, 255, 255, (int)(255*m_flAttackInfoAlpha)));

		if (bShowCPUStuff && m_flActionItemsLerp > 0)
			PaintHUDSheet("ActionItemPanel", m_pActionItem->GetLeft()-30, m_pActionItem->GetTop()-30, (int)m_flActionItemsWidth, 340, Color(255, 255, 255, (int)(255*m_flActionItemsLerp)));

		// Tank info
		PaintHUDSheet("TankInfo", 0, iHeight-250, 150, 250);
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

	CTeam* pLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (pLocalTeam && m_hHintWeapon == NULL)
	{
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

			if (pWeapon->HasFragmented())
				continue;

			if (!pWeapon->GetOwner())
				continue;
			
			if (pWeapon->GetOwner()->GetTeam() != pLocalTeam)
				continue;

			m_hHintWeapon = pWeapon;
			break;
		}
	}

	if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY && m_hHintWeapon != NULL)
	{
		int iX = GetWidth()/2 - 75;
		int iY = GetHeight()/2 + 50;

		CRootPanel::PaintRect(iX, iY, 150, 85, Color(0, 0, 0, 150));

		m_pSpacebarHint->SetVisible(true);
		m_pSpacebarHint->SetText(m_hHintWeapon->SpecialCommandHint());
		m_pSpacebarHint->SetSize(150, 25);
		m_pSpacebarHint->SetPos(iX, iY + 50);
		m_pSpacebarHint->SetWrap(false);

		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);

		iY -= (int)(Lerp(Oscillate(GameServer()->GetGameTime(), 0.5f), 0.2f) * 10);

		PaintSheet(m_iKeysSheet, iX+45, iY, 60, 60, 128, 64, 64, 64, 256, 256, Color(255, 255, 255, 255));
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

			weapon_t eWeapon = DigitanksGame()->GetPrimarySelectionTank()->GetCurrentWeapon();
			if (eWeapon == WEAPON_LASER || eWeapon == WEAPON_INFANTRYLASER || eWeapon == PROJECTILE_TORPEDO)
				bObstruction = false;

			if (bObstruction)
				PaintHUDSheet("Obstruction", iX, iY, 124, 68, Color(255, 255, 255, (int)RemapVal(Oscillate(GameServer()->GetGameTime(), 0.8f), 0, 1, 150, 255)));
		}
	}

	bool bCloseVisible = m_pCloseActionItems->IsVisible();
	m_pCloseActionItems->SetVisible(false);
	m_pButtonInfo->SetVisible(false);
	m_pTeamInfo->SetVisible(false);
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

		pSelection->DrawSchema(iWidth/2 - 720/2 + 10 + 150/2 - 100/2, iHeight - 150 + 10 + 130/2 - 100/2, 100, 100);
		pSelection->DrawQueue(iWidth/2 - 720/2 + 162, iHeight - 90, 216, 84);
	}

	CDigitank* pTank = DigitanksGame()->GetPrimarySelectionTank();

	if (pTank)
	{
		if (pTank->GetShieldValue() > 5)
		{
			CRenderingContext c(GameServer()->GetRenderer());

			c.SetBlend(BLEND_ADDITIVE);

			float flFlicker = 1;
	
			if (pTank->GetShieldValue() < pTank->GetShieldMaxStrength()*3/4)
				flFlicker = Flicker("zzzzmmzzztzzzzzznzzz", GameServer()->GetGameTime() + ((float)pTank->GetSpawnSeed()/100), 1.0f);

			int iShield = (int)(255*flFlicker*pTank->GetDefensePower(true)/pTank->GetStartingPower());
			if (iShield > 255)
				iShield = 255;

			int iSize = 100;

			glgui::CBaseControl::PaintTexture(m_iShieldTexture, iWidth/2 - 720/2 + 10 + 150/2 - iSize/2, iHeight - 150 + 10 + 130/2 - iSize/2, iSize, iSize, Color(255, 255, 255, iShield));
		}
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

	if (m_pTeamInfo->GetText().length() > 0)
		CRootPanel::PaintRect(m_pTeamInfo->GetLeft()-3, m_pTeamInfo->GetTop()-9, m_pTeamInfo->GetWidth()+6, m_pTeamInfo->GetHeight()+6, Color(0, 0, 0, 220));

	m_pTeamInfo->SetVisible(true);
	m_pTeamInfo->Paint();

	if (m_iCurrentSmallActionItem < m_apActionItemButtons.size() && m_flSmallActionItemLerp > 0)
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);

		CPictureButton* pActionItem = m_apActionItemButtons[m_iCurrentSmallActionItem];

		int iLeft = pActionItem->GetLeft();
		int iTop = pActionItem->GetTop();

		int iTextureLeft = iLeft - 243;

		PaintHUDSheet("SmallActionItem", iTextureLeft - 10, iTop, 243, 40, Color(255, 255, 255, (int)(255*m_flSmallActionItemLerp)));

		c.SetColor(Color(255, 255, 255, (int)(255*m_flSmallActionItemLerp)));
		float flWidth = CLabel::GetTextWidth(m_sSmallActionItem, m_sSmallActionItem.length(), L"text", 13);
		float flHeight = CLabel::GetFontHeight(L"text", 13);
		CLabel::PaintText(m_sSmallActionItem, m_sSmallActionItem.length(), L"text", 13, (float)(iLeft - flWidth) - 25, iTop + flHeight + 5);
	}

	if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
	{
		// Don't clear it in the start, we want dead tanks to remain in the list so we can mark them asploded.

		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);

		for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
		{
			CDigitanksTeam* pTeam = DigitanksGame()->GetDigitanksTeam(i);
			if (!pTeam)
				continue;

			eastl::map<size_t, CEntityHandle<CDigitank> >& ahTeamTanks = m_ahScoreboardTanks[i];

			Color clrTeam = pTeam->GetColor();
			if (DigitanksGame()->GetCurrentTeam() == pTeam)
				clrTeam.SetAlpha((int)(255 * Oscillate(GameServer()->GetGameTime(), 1)));
			c.SetColor(clrTeam);

			if (DigitanksGame()->GetCurrentTeam() == pTeam)
				CBaseControl::PaintRect(w - ahTeamTanks.size() * 30 - 20, 35 + i*40, ahTeamTanks.size()*30+10, 45, Color(0, 0, 0, 150));

			eastl::string16 sTeamName = pTeam->GetTeamName();
			if (pTeam->IsPlayerControlled())
				sTeamName = eastl::string16(L"[") + sTeamName + L"]";

			CLabel::PaintText(sTeamName, sTeamName.length(), L"text", 13, (float)w - CLabel::GetTextWidth(sTeamName, sTeamName.length(), L"text", 13) - 20, 50 + (float)i*40);

			int iTank = 0;
			for (eastl::map<size_t, CEntityHandle<CDigitank> >::iterator it = ahTeamTanks.begin(); it != ahTeamTanks.end(); it++)
			{
				CDigitank* pTank = it->second;

				Color clrTank = pTeam->GetColor();
				if (!pTank || !pTank->IsAlive())
					clrTank.SetAlpha(100);

				PaintUnitSheet(UNIT_TANK, w - (iTank+1)*30 - 10, 50 + i*40, 20, 20, clrTank);

				if (!pTank || !pTank->IsAlive())
					PaintHUDSheet("DeadTank", w - (iTank+1)*30 - 10, 50 + i*40, 20, 20);

				iTank++;
			}
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

void CHUD::PaintCameraGuidedMissile(int x, int y, int w, int h)
{
	CCameraGuidedMissile* pMissile = DigitanksGame()->GetDigitanksCamera()->GetCameraGuidedMissile();

	if (!pMissile->GetOwner())
		return;

	Color clrWhite = Color(255, 255, 255, 120);

	CRenderingContext c(GameServer()->GetRenderer());
	c.SetBlend(BLEND_ADDITIVE);
	c.SetColor(clrWhite);

	if (!pMissile->IsBoosting() && GameServer()->GetGameTime() - pMissile->GetSpawnTime() < 3)
	{
		eastl::string16 sLaunch = L"LAUNCH IN";
		float flLaunchWidth = glgui::CLabel::GetTextWidth(sLaunch, sLaunch.length(), L"cameramissile", 40);
		glgui::CLabel::PaintText(sLaunch, sLaunch.length(), L"cameramissile", 40, w/2-flLaunchWidth/2, 150);

		sLaunch = sprintf(L"%.1f", 3 - (GameServer()->GetGameTime() - pMissile->GetSpawnTime()));
		flLaunchWidth = glgui::CLabel::GetTextWidth(sLaunch, sLaunch.length(), L"cameramissile", 40);
		glgui::CLabel::PaintText(sLaunch, sLaunch.length(), L"cameramissile", 40, w/2-flLaunchWidth/2, 200);
	}

	CDigitank* pOwner = dynamic_cast<CDigitank*>(pMissile->GetOwner());
	if (!pOwner)
		return;

	eastl::string16 sRange = sprintf(L"RANGE %.2f", (pOwner->GetLastAim() - pMissile->GetOrigin()).Length());
	glgui::CLabel::PaintText(sRange, sRange.length(), L"cameramissile", 20, 10, 100);

	Vector vecHit;
	CBaseEntity* pHit;
	bool bHit = DigitanksGame()->TraceLine(pOwner->GetLastAim() + Vector(0, 1, 0), pMissile->GetOrigin(), vecHit, &pHit, CG_TERRAIN);

	eastl::string16 sClear;
	if (bHit)
		sClear = L"OBSTRUCTION";
	else
		sClear = L"CLEAR";
	glgui::CLabel::PaintText(sClear, sClear.length(), L"cameramissile", 20, 10, 200);

	eastl::string16 sAltitude = sprintf(L"ALT %.2f", pMissile->GetOrigin().y - DigitanksGame()->GetTerrain()->GetHeight(pMissile->GetOrigin().x, pMissile->GetOrigin().y));
	glgui::CLabel::PaintText(sAltitude, sAltitude.length(), L"cameramissile", 20, 10, 300);

	eastl::string16 sFuel = sprintf(L"FUEL %.2f", 13 - (GameServer()->GetGameTime() - pMissile->GetSpawnTime()));
	glgui::CLabel::PaintText(sFuel, sFuel.length(), L"cameramissile", 20, 10, 400);

	if (pMissile->IsBoosting() && Oscillate(GameServer()->GetGameTime(), 0.3f) > 0.1f)
	{
		eastl::string16 sBoost = L"BOOST";
		float flBoostWidth = glgui::CLabel::GetTextWidth(sBoost, sBoost.length(), L"cameramissile", 40);
		glgui::CLabel::PaintText(sBoost, sBoost.length(), L"cameramissile", 40, w/2-flBoostWidth/2, h - 200.0f);
	}

	int iBoxWidth = 300;

	glgui::CRootPanel::PaintRect(w/2-iBoxWidth/2, h/2-iBoxWidth/2, 1, iBoxWidth, clrWhite);
	glgui::CRootPanel::PaintRect(w/2-iBoxWidth/2, h/2-iBoxWidth/2, iBoxWidth, 1, clrWhite);
	glgui::CRootPanel::PaintRect(w/2+iBoxWidth/2, h/2-iBoxWidth/2, 1, iBoxWidth, clrWhite);
	glgui::CRootPanel::PaintRect(w/2-iBoxWidth/2, h/2+iBoxWidth/2, 300, 1, clrWhite);

	int iTrackingWidth = 500;

	glgui::CRootPanel::PaintRect(w/2-iTrackingWidth/2, h/2-iTrackingWidth/2-50, iTrackingWidth, 1, clrWhite);
	glgui::CRootPanel::PaintRect(w/2-iTrackingWidth/2, h/2+iTrackingWidth/2+50, iTrackingWidth, 1, clrWhite);

	EAngle angCamera = pMissile->GetAngles();
	int iTicks = 10;
	int iPixelsPerTick = iTrackingWidth/iTicks;

	for (int i = 0; i <= 10; i++)
	{
		int iOffset = iPixelsPerTick*i;
		int iXPosition = w/2-iTrackingWidth/2 + iOffset + (int)fmod(-angCamera.y*1.5f, (float)iPixelsPerTick);
		if (iXPosition < w/2-iTrackingWidth/2)
			continue;

		if (iXPosition > w/2+iTrackingWidth/2)
			continue;

		glgui::CRootPanel::PaintRect(iXPosition, h/2-iTrackingWidth/2-50+1, 1, 8, clrWhite);
		glgui::CRootPanel::PaintRect(iXPosition, h/2+iTrackingWidth/2+50-8, 1, 8, clrWhite);
	}

	glgui::CRootPanel::PaintRect(w/2-iTrackingWidth/2-50, h/2-iTrackingWidth/2, 1, iTrackingWidth, clrWhite);
	glgui::CRootPanel::PaintRect(w/2+iTrackingWidth/2+50, h/2-iTrackingWidth/2, 1, iTrackingWidth, clrWhite);

	for (int i = 0; i <= 10; i++)
	{
		int iOffset = iPixelsPerTick*i;
		int iYPosition = h/2-iTrackingWidth/2 + iOffset + (int)fmod(-angCamera.p*1.5f, (float)iPixelsPerTick);
		if (iYPosition < h/2-iTrackingWidth/2)
			continue;

		if (iYPosition > h/2+iTrackingWidth/2)
			continue;

		glgui::CRootPanel::PaintRect(w/2-iTrackingWidth/2-50+1, iYPosition, 8, 1, clrWhite);
		glgui::CRootPanel::PaintRect(w/2+iTrackingWidth/2+50-8, iYPosition, 8, 1, clrWhite);
	}

	Vector vecUp;
	Vector vecForward;
	GameServer()->GetRenderer()->GetCameraVectors(&vecForward, NULL, &vecUp);

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
		if (!pTank)
			continue;

		Vector vecOrigin = pEntity->GetOrigin();

		Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(vecOrigin);

		float flRadius = pDTEntity->GetBoundingRadius();

		// Cull tanks behind the camera
		if (vecForward.Dot((vecOrigin-GameServer()->GetCamera()->GetCameraPosition()).Normalized()) < 0)
			continue;

		Vector vecTop = GameServer()->GetRenderer()->ScreenPosition(vecOrigin + vecUp*flRadius);
		float flWidth = (vecTop - vecScreen).Length()*2 + 10;

		if (vecScreen.x < w/2-iBoxWidth)
			continue;

		if (vecScreen.y < h/2-iBoxWidth)
			continue;

		if (vecScreen.x > w/2+iBoxWidth)
			continue;

		if (vecScreen.y > h/2+iBoxWidth)
			continue;

		Color clrTeam = Color(255, 255, 255);
		if (pTank->GetTeam())
			clrTeam = pTank->GetTeam()->GetColor();
		clrTeam.SetAlpha(clrWhite.a());

		float flHole = 5;
		CRootPanel::PaintRect((int)(vecScreen.x - flWidth), (int)(vecScreen.y), (int)(flWidth - flHole), 1, clrTeam);
		CRootPanel::PaintRect((int)(vecScreen.x + flHole), (int)(vecScreen.y), (int)(flWidth - flHole), 1, clrTeam);
		CRootPanel::PaintRect((int)(vecScreen.x), (int)(vecScreen.y - flWidth), 1, (int)(flWidth - flHole), clrTeam);
		CRootPanel::PaintRect((int)(vecScreen.x), (int)(vecScreen.y + flHole), 1, (int)(flWidth - flHole), clrTeam);
	}

	if (Oscillate(GameServer()->GetGameTime(), 0.5f) > 0.3f)
	{
		if (vecForward.Dot((pOwner->GetLastAim()-GameServer()->GetCamera()->GetCameraPosition()).Normalized()) > 0)
		{
			Vector vecTarget = GameServer()->GetRenderer()->ScreenPosition(pOwner->GetLastAim());
			float flWidth = 40;

			CRootPanel::PaintRect((int)(vecTarget.x - flWidth/2), (int)(vecTarget.y - flWidth/2), (int)flWidth, 1, clrWhite);
			CRootPanel::PaintRect((int)(vecTarget.x - flWidth/2), (int)(vecTarget.y - flWidth/2), 1, (int)flWidth, clrWhite);
			CRootPanel::PaintRect((int)(vecTarget.x + flWidth/2), (int)(vecTarget.y - flWidth/2), 1, (int)flWidth, clrWhite);
			CRootPanel::PaintRect((int)(vecTarget.x - flWidth/2), (int)(vecTarget.y + flWidth/2), (int)flWidth, 1, clrWhite);

			c.SetBlend(BLEND_NONE);
			CRootPanel::PaintRect((int)(vecTarget.x - flWidth/2) - 1, (int)(vecTarget.y - flWidth/2) - 1, (int)flWidth + 2, 1, Color(0, 0, 0));
			CRootPanel::PaintRect((int)(vecTarget.x - flWidth/2) - 1, (int)(vecTarget.y - flWidth/2) - 1, 1, (int)flWidth + 2, Color(0, 0, 0));
			CRootPanel::PaintRect((int)(vecTarget.x + flWidth/2) + 1, (int)(vecTarget.y - flWidth/2) - 1, 1, (int)flWidth + 2, Color(0, 0, 0));
			CRootPanel::PaintRect((int)(vecTarget.x - flWidth/2) - 1, (int)(vecTarget.y + flWidth/2) + 1, (int)flWidth + 2, 1, Color(0, 0, 0));
		}
	}

	m_hHintWeapon = pMissile;
	if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY && m_hHintWeapon != NULL && !pMissile->IsBoosting() && hud_enable.GetBool())
	{
		int iX = GetWidth()/2 - 75;
		int iY = GetHeight()/2 + 50;

		CRootPanel::PaintRect(iX, iY, 150, 85, Color(0, 0, 0, 150));

		m_pSpacebarHint->SetVisible(true);
		m_pSpacebarHint->SetText(m_hHintWeapon->SpecialCommandHint());
		m_pSpacebarHint->SetSize(150, 25);
		m_pSpacebarHint->SetPos(iX, iY + 50);
		m_pSpacebarHint->SetWrap(false);

		CRenderingContext c(GameServer()->GetRenderer());
		c.SetBlend(BLEND_ALPHA);

		iY -= (int)(Lerp(Oscillate(GameServer()->GetGameTime(), 0.5f), 0.2f) * 10);

		PaintSheet(m_iKeysSheet, iX+45, iY, 60, 60, 128, 64, 64, 64, 256, 256, Color(255, 255, 255, 255));

		m_pSpacebarHint->Paint();
	}
	else
	{
		m_pSpacebarHint->SetVisible(false);
	}
}

// Let's phase this one out in favor of the below.
void CHUD::PaintSheet(size_t iTexture, int x, int y, int w, int h, int sx, int sy, int sw, int sh, int tw, int th, const Color& c)
{
	glgui::CBaseControl::PaintSheet(iTexture, x, y, w, h, sx, sy, sw, sh, tw, th, c);
}

void CHUD::PaintSheet(const CTextureSheet* pSheet, const eastl::string& sArea, int x, int y, int w, int h, const Color& c)
{
	const Rect& rArea = pSheet->GetArea(sArea);
	glgui::CBaseControl::PaintSheet(pSheet->GetSheet(), x, y, w, h, rArea.x, rArea.y, rArea.w, rArea.h, pSheet->GetSheetWidth(), pSheet->GetSheetHeight(), c);
}

void CHUD::PaintHUDSheet(const eastl::string& sArea, int x, int y, int w, int h, const Color& c)
{
	CHUD* pHUD = DigitanksWindow()->GetHUD();
	const Rect* pRect = &pHUD->m_HUDSheet.GetArea(sArea);
	PaintSheet(pHUD->m_HUDSheet.GetSheet(), x, y, w, h, pRect->x, pRect->y, pRect->w, pRect->h, pHUD->m_HUDSheet.GetSheetWidth(), pHUD->m_HUDSheet.GetSheetHeight(), c);
}

const CTextureSheet& CHUD::GetHUDSheet()
{
	return DigitanksWindow()->GetHUD()->m_HUDSheet;
}

void CHUD::GetUnitSheet(unittype_t eUnit, int& sx, int& sy, int& sw, int& sh, int& tw, int& th)
{
	CTextureSheet* pUnits = &DigitanksWindow()->GetHUD()->m_UnitsSheet;
	tw = pUnits->GetSheetWidth();
	th = pUnits->GetSheetHeight();

	Rect rUnit;
	if (eUnit == UNIT_TANK)
		rUnit = pUnits->GetArea("Digitank");
	else if (eUnit == UNIT_SCOUT)
		rUnit = pUnits->GetArea("Rogue");
	else if (eUnit == UNIT_ARTILLERY)
		rUnit = pUnits->GetArea("Artillery");
	else if (eUnit == UNIT_INFANTRY)
		rUnit = pUnits->GetArea("Resistor");
	else if (eUnit == UNIT_MOBILECPU)
		rUnit = pUnits->GetArea("MobileCPU");
	else if (eUnit == UNIT_GRIDBUG)
		rUnit = pUnits->GetArea("GridBug");
	else if (eUnit == UNIT_BUGTURRET)
		rUnit = pUnits->GetArea("BugTurret");
	else if (eUnit == STRUCTURE_MINIBUFFER)
		rUnit = pUnits->GetArea("Buffer");
	else if (eUnit == STRUCTURE_BUFFER)
		rUnit = pUnits->GetArea("MacroBuffer");
	else if (eUnit == STRUCTURE_BATTERY)
		rUnit = pUnits->GetArea("Capacitor");
	else if (eUnit == STRUCTURE_PSU)
		rUnit = pUnits->GetArea("PSU");
	else if (eUnit == STRUCTURE_INFANTRYLOADER)
		rUnit = pUnits->GetArea("ResistorFactory");
	else if (eUnit == STRUCTURE_TANKLOADER)
		rUnit = pUnits->GetArea("DigitankFactory");
	else if (eUnit == STRUCTURE_ARTILLERYLOADER)
		rUnit = pUnits->GetArea("ArtilleryFactory");
	else if (eUnit == STRUCTURE_CPU)
		rUnit = pUnits->GetArea("CPU");
	else if (eUnit == STRUCTURE_ELECTRONODE)
		rUnit = pUnits->GetArea("Electronode");
	else
		rUnit = Rect(0, 0, 100, 100);

	sx = rUnit.x;
	sy = rUnit.y;
	sw = rUnit.w;
	sh = rUnit.h;
}

void CHUD::PaintUnitSheet(unittype_t eUnit, int x, int y, int w, int h, const Color& c)
{
	int sx, sy, sw, sh, tw, th;
	GetUnitSheet(eUnit, sx, sy, sw, sh, tw, th);
	PaintSheet(DigitanksWindow()->GetHUD()->m_UnitsSheet.GetSheet(), x, y, w, h, sx, sy, sw, sh, tw, th, c);
}

void CHUD::GetWeaponSheet(weapon_t eWeapon, int& sx, int& sy, int& sw, int& sh, int& tw, int& th)
{
	CTextureSheet* pWeapons = &DigitanksWindow()->GetHUD()->m_WeaponsSheet;
	tw = pWeapons->GetSheetWidth();
	th = pWeapons->GetSheetHeight();

	Rect rUnit;
	switch (eWeapon)
	{
	default:
	case WEAPON_NONE:
	case PROJECTILE_SMALL:
		rUnit = pWeapons->GetArea("LittleBoy");
		break;

	case PROJECTILE_MEDIUM:
		rUnit = pWeapons->GetArea("FatMan");
		break;

	case PROJECTILE_LARGE:
		rUnit = pWeapons->GetArea("BigMama");
		break;

	case PROJECTILE_AOE:
	case PROJECTILE_ARTILLERY_AOE:
		rUnit = pWeapons->GetArea("AOE");
		break;

	case PROJECTILE_EMP:
		rUnit = pWeapons->GetArea("EMP");
		break;

	case PROJECTILE_ICBM:
	case PROJECTILE_ARTILLERY_ICBM:
		rUnit = pWeapons->GetArea("ICBM");
		break;

	case PROJECTILE_GRENADE:
		rUnit = pWeapons->GetArea("Grenade");
		break;

	case PROJECTILE_DAISYCHAIN:
		rUnit = pWeapons->GetArea("DaisyChain");
		break;

	case PROJECTILE_CLUSTERBOMB:
		rUnit = pWeapons->GetArea("ClusterBomb");
		break;

	case PROJECTILE_SPLOOGE:
		rUnit = pWeapons->GetArea("Grapeshot");
		break;

	case PROJECTILE_TRACTORBOMB:
		rUnit = pWeapons->GetArea("RepulsorBomb");
		break;

	case PROJECTILE_EARTHSHAKER:
		rUnit = pWeapons->GetArea("Earthshaker");
		break;

	case PROJECTILE_CAMERAGUIDED:
		rUnit = pWeapons->GetArea("CameraGuidedMissile");
		break;

	case WEAPON_LASER:
	case WEAPON_INFANTRYLASER:
		rUnit = pWeapons->GetArea("FragmentationRay");
		break;

	case WEAPON_CHARGERAM:
		rUnit = pWeapons->GetArea("ChargingRAM");
		break;

	case PROJECTILE_TREECUTTER:
		rUnit = pWeapons->GetArea("TreeCutter");
		break;

	case PROJECTILE_FLAK:
		rUnit = pWeapons->GetArea("MachineGun");
		break;

	case PROJECTILE_DEVASTATOR:
		rUnit = pWeapons->GetArea("Devastator");
		break;

	case PROJECTILE_TORPEDO:
		rUnit = pWeapons->GetArea("Torpedo");
		break;
	}

	sx = rUnit.x;
	sy = rUnit.y;
	sw = rUnit.w;
	sh = rUnit.h;
}

void CHUD::PaintWeaponSheet(weapon_t eWeapon, int x, int y, int w, int h, const Color& c)
{
	int sx, sy, sw, sh, tw, th;
	GetWeaponSheet(eWeapon, sx, sy, sw, sh, tw, th);
	PaintSheet(DigitanksWindow()->GetHUD()->m_WeaponsSheet.GetSheet(), x, y, w, h, sx, sy, sw, sh, tw, th, c);
}

const CTextureSheet& CHUD::GetWeaponSheet()
{
	return DigitanksWindow()->GetHUD()->m_WeaponsSheet;
}

const CTextureSheet& CHUD::GetButtonSheet()
{
	return DigitanksWindow()->GetHUD()->m_ButtonSheet;
}

const CTextureSheet& CHUD::GetDownloadSheet()
{
	return DigitanksWindow()->GetHUD()->m_DownloadSheet;
}

size_t CHUD::GetActionTanksSheet()
{
	return DigitanksWindow()->GetHUD()->m_iActionTanksSheet;
}

size_t CHUD::GetPurchasePanel()
{
	return DigitanksWindow()->GetHUD()->m_iPurchasePanel;
}

size_t CHUD::GetShieldTexture()
{
	return DigitanksWindow()->GetHUD()->m_iShieldTexture;
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

	m_ahScoreboardTanks.clear();
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
	else
		m_pTankInfo->SetText(L"No selection.");
}

void CHUD::UpdateTankInfo(CDigitank* pTank)
{
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

	float flShieldStrength = pClosestTarget->GetShieldValue();
	float flDamageBlocked = flShieldStrength * pClosestTarget->GetDefenseScale(true);
	float flAttackDamage = CBaseWeapon::GetWeaponDamage(pTank->GetCurrentWeapon());

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
		"Shield Damage: %d/%d\n"
		"Hull Damage: %d/%d\n",
		(int)flShieldDamage, (int)(flShieldStrength * pClosestTarget->GetDefenseScale(true)),
		(int)flTankDamage, (int)pClosestTarget->GetHealth()
	);

	m_pAttackInfo->SetText(szAttackInfo);
}

void CHUD::UpdateStructureInfo(CStructure* pStructure)
{
	m_pAttackInfo->SetText(L"");
}

void CHUD::UpdateTeamInfo()
{
	if (!DigitanksGame())
		return;

	bool bShow = false;
	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD || DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN)
		bShow = true;
	
	if (!bShow)
	{
		m_pPowerInfo->SetText("");
		m_pFleetInfo->SetText("");
		m_pBandwidthInfo->SetText("");
		return;
	}

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	if (!pTeam)
		return;

	if (!pTeam->GetPrimaryCPU())
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
		s += pTeam->GetTeamName();
		if (DigitanksGame()->IsTeamControlledByMe(pTeam))
			s += L"]";

		s += p.sprintf(L": %d\n", pTeam->GetScore());
	}

	m_pScoreboard->SetText(L"Score:\n \n");
	m_pScoreboard->AppendText(s);

	m_pScoreboard->SetSize(100, 9999);
	m_pScoreboard->SetSize(m_pScoreboard->GetWidth(), (int)m_pScoreboard->GetTextHeight());

	int iWidth = DigitanksWindow()->GetWindowWidth();

	m_pScoreboard->SetPos(iWidth - m_pScoreboard->GetWidth() - 10, m_pAttackInfo->GetTop() - m_pScoreboard->GetHeight() - 20);
}

void CHUD::UpdateTurnButton()
{
	bool bWasBlinking = m_bBlinkTurnButton;
	m_bBlinkTurnButton = false;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTeam())
		return;

	if (!DigitanksGame()->GetCurrentLocalDigitanksTeam())
		return;

	if (!DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentTeam()))
	{
		SetButtonSheetTexture(m_pTurnButton, &m_HUDSheet, "EnemyTurnButton");
		m_pPressEnter->SetVisible(true);
		return;
	}

	bool bTurnComplete = true;

	for (size_t i = 0; i < DigitanksGame()->GetCurrentTeam()->GetNumMembers(); i++)
	{
		CDigitank* pTank = dynamic_cast<CDigitank*>(DigitanksGame()->GetCurrentTeam()->GetMember(i));

		if (pTank)
		{
			if (pTank->NeedsOrders())
			{
				bTurnComplete = false;
				break;
			}
		}
	}

	for (size_t i = 0; i < DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetNumActionItems(); i++)
	{
		const actionitem_t* pItem = DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetActionItem(i);

		if (!pItem->bHandled)
		{
			bTurnComplete = false;
			break;
		}
	}
	
	if (bTurnComplete)
	{
		if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY && !bWasBlinking && DigitanksGame()->GetTurn() == 0 && !DigitanksWindow()->GetInstructor()->GetCurrentPanel())
			DigitanksWindow()->GetInstructor()->DisplayTutorial("artillery-endturn");

		m_bBlinkTurnButton = true;
		SetButtonSheetTexture(m_pTurnButton, &m_HUDSheet, "TurnCompleteButton");
		m_pPressEnter->SetVisible(true);
	}
	else
	{
		SetButtonSheetTexture(m_pTurnButton, &m_HUDSheet, "TurnButton");
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
		m_apButtons[i]->SetTexture(0);
		SetButtonInfo(i, L"");
		SetButtonTooltip(i, L"");
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

void CHUD::SetButtonTexture(int i, const eastl::string& sArea)
{
	SetButtonSheetTexture(m_apButtons[i], &m_ButtonSheet, sArea);
}

void CHUD::SetButtonColor(int i, Color clrButton)
{
	m_apButtons[i]->SetButtonColor(clrButton);
}

void CHUD::SetButtonInfo(int iButton, const eastl::string16& pszInfo)
{
	m_aszButtonInfos[iButton] = pszInfo;
}

void CHUD::SetButtonTooltip(int iButton, const eastl::string16& sTip)
{
	m_apButtons[iButton]->SetTooltip(sTip);
}

void CHUD::ButtonCallback(int iButton)
{
	if (m_apButtons[iButton]->GetClickedListener())
		m_apButtons[iButton]->GetClickedListenerCallback()(m_apButtons[iButton]->GetClickedListener());
}

void CHUD::GameStart()
{
	DigitanksGame()->SetControlMode(MODE_NONE);
	ClearTurnInfo();
}

void CHUD::GameOver(bool bPlayerWon)
{
	DigitanksWindow()->GameOver(bPlayerWon);
}

void CHUD::NewCurrentTeam()
{
	m_bAllActionItemsHandled = false;

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

	if (m_pWeaponPanel->IsVisible())
		m_pWeaponPanel->SetVisible(false);

	if (DigitanksGame()->GetPrimarySelection())
	{
		Vector vecTarget = DigitanksGame()->GetPrimarySelection()->GetOrigin();
		EAngle angCamera = DigitanksGame()->GetDigitanksCamera()->GetAngles();
		float flDistance = DigitanksGame()->GetDigitanksCamera()->GetDistance();
		int iRotations = 0;
		while (iRotations++ < 4)
		{
			Vector vecCamera = AngleVector(angCamera) * flDistance + vecTarget;

			Vector vecHit;
			CBaseEntity* pHit;
			Game()->TraceLine(vecCamera, vecTarget, vecHit, &pHit);
			if (pHit == DigitanksGame()->GetPrimarySelection())
				break;

			angCamera.y += 90;
		}

		DigitanksGame()->GetDigitanksCamera()->SetAngle(angCamera);
	}
}

void CHUD::ShowFirstActionItem()
{
	m_bAllActionItemsHandled = false;

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetNumActionItems())
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

	do
	{
		m_iCurrentActionItem = (m_iCurrentActionItem+1)%DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetNumActionItems();

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

		const actionitem_t* pItem = DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetActionItem(m_iCurrentActionItem);

		if (pItem->eActionType == ACTIONTYPE_DOWNLOADCOMPLETE && DigitanksGame()->GetCurrentTeam()->GetUpdateDownloading())
		{
			DigitanksGame()->GetCurrentLocalDigitanksTeam()->HandledActionItem(ACTIONTYPE_DOWNLOADCOMPLETE);
			UpdateTurnButton();
		}

		if (pItem->bHandled)
			continue;

		// Only perform this check with tanks. Structures always return false for NeedsOrders()
		CEntityHandle<CDigitank> hDigitank(pItem->iUnit);

		if (hDigitank != NULL && !hDigitank->NeedsOrders())
		{
			// If must have been handled already.
			DigitanksGame()->GetCurrentLocalDigitanksTeam()->HandledActionItem(hDigitank);
			UpdateTurnButton();
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

	CDigitanksTeam* pLocalTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (!pLocalTeam)
		return;

	if (pSelectable)
	{
		for (size_t i = 0; i < pLocalTeam->GetNumActionItems(); i++)
		{
			if (pLocalTeam->GetActionItem(i)->iUnit == pSelectable->GetHandle())
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
	if (m_iCurrentActionItem >= 0 && m_iCurrentActionItem < pLocalTeam->GetNumActionItems() && pLocalTeam->GetActionItem(m_iCurrentActionItem)->iUnit == ~0)
		return;

	ShowActionItem(~0);
}

void CHUD::ShowActionItem(actiontype_t eActionItem)
{
	if (m_bAllActionItemsHandled)
		return;

	for (size_t i = 0; i < DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetNumActionItems(); i++)
	{
		if (DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetActionItem(i)->eActionType == eActionItem)
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

	if (iActionItem >= DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetNumActionItems())
	{
		m_pActionItem->SetText("");
		m_flActionItemsLerpGoal = 0;
		return;
	}

	const actionitem_t* pItem = DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetActionItem(iActionItem);

	// Some action items are handled just by looking at them.
	if (pItem->eActionType == ACTIONTYPE_WELCOME || pItem->eActionType == ACTIONTYPE_CONTROLS || pItem->eActionType == ACTIONTYPE_NEWSTRUCTURE ||
		pItem->eActionType == ACTIONTYPE_AUTOMOVEENEMY || pItem->eActionType == ACTIONTYPE_UPGRADE || pItem->eActionType == ACTIONTYPE_UNITREADY ||
		pItem->eActionType == ACTIONTYPE_FORTIFIEDENEMY)
	{
		DigitanksGame()->GetCurrentLocalDigitanksTeam()->HandledActionItem(iActionItem);
		UpdateTurnButton();
	}

	switch (pItem->eActionType)
	{
	case ACTIONTYPE_WELCOME:
		m_pActionItem->SetText(
			"WELCOME TO DIGITANKS\n \n"
			"On the right is a list of tasks for you for each turn.\n \n"
			"Click an icon on the right to view that task. Tasks that are blinking need handling.\n \n"
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
		m_pActionItem->SetText("");
		m_flActionItemsLerpGoal = 0;
		return;

	case ACTIONTYPE_UNITORDERS:
		m_pActionItem->SetText("");
		m_flActionItemsLerpGoal = 0;
		return;

	case ACTIONTYPE_UNITAUTOMOVE:
		m_pActionItem->SetText("");
		m_flActionItemsLerpGoal = 0;
		return;

	case ACTIONTYPE_AUTOMOVECANCELED:
		m_pActionItem->SetText(
			"UNIT AUTO-MOVE INTERRUPTED\n \n"
			"This unit's auto-move has been canceled, due to it taking damage from enemy fire. Please assign it new orders.\n");
		break;

	case ACTIONTYPE_AUTOMOVEENEMY:
		m_pActionItem->SetText(
			"UNIT AUTO-MOVE THREAT\n \n"
			"This unit is auto-moving to a new location, but an enemy is visible. You may wish to cancel this auto move.\n");
		break;

	case ACTIONTYPE_UNITDAMAGED:
		m_pActionItem->SetText("");
		m_flActionItemsLerpGoal = 0;
		return;

	case ACTIONTYPE_FORTIFIEDENEMY:
		m_pActionItem->SetText(
			"ENEMY SIGHTED\n \n"
			"An enemy has been sighted in range of this fortified unit. Strike while the iron is hot.\n");
		break;

	case ACTIONTYPE_UPGRADE:
		m_pActionItem->SetText("");
		m_flActionItemsLerpGoal = 0;
		return;

	case ACTIONTYPE_UNITREADY:
		m_pActionItem->SetText("");
		m_flActionItemsLerpGoal = 0;
		return;

	case ACTIONTYPE_DOWNLOADCOMPLETE:
		m_pActionItem->SetText("");
		m_flActionItemsLerpGoal = 0;
		OpenUpdatesCallback();
		return;

	case ACTIONTYPE_DOWNLOADUPDATES:
		m_pActionItem->SetText(
			"DOWNLOAD UPDATES\n \n"
			"You can download updates for your structures. Press the 'Download' button at the top of the screen to choose an update.\n");
		break;
	}

	// Just in case it was turned off because of the tutorials.
	if (pItem->eActionType == ACTIONTYPE_DOWNLOADUPDATES)
		m_bUpdatesBlinking = true;

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

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() == NULL)
		return;

	if (pVictim && DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetVisibilityAtPoint(pVictim->GetOrigin()) < 0.1f)
		return;

	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, true);

	if (!pVictim->IsAlive() && bDirectHit && flDamage > 0)
		new CHitIndicator(pVictim, L"OVERKILL!");
}

void CHUD::OnTakeDamage(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() == NULL)
		return;

	if (pVictim && DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetVisibilityAtPoint(pVictim->GetOrigin()) < 0.1f)
		return;

	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, false);

	if ((pVictim && pVictim->IsAlive() || bKilled) && bDirectHit && flDamage > 0)
		new CHitIndicator(pVictim, L"DIRECT HIT!");
}

void CHUD::OnDisabled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() == NULL)
		return;

	if (pVictim && DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetVisibilityAtPoint(pVictim->GetOrigin()) < 0.1f)
		return;

	if (pVictim && pVictim->IsAlive())
	{
		if (dynamic_cast<CDigitank*>(pVictim))
			new CHitIndicator(pVictim, L"DISABLED!");
		else
			new CHitIndicator(pVictim, L"DISCONNECTED!");
	}
}

void CHUD::OnMiss(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	if (pVictim == pAttacker)
		return;

	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() == NULL)
		return;

	if (pVictim && DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetVisibilityAtPoint(pVictim->GetOrigin()) < 0.1f)
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

void ActionSignCallback(CCommand* pCommand, eastl::vector<eastl::string16>& asTokens, const eastl::string16& sCommand)
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

	if (iActionItem < DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetNumActionItems())
	{
		const actionitem_t* pItem = DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetActionItem(iActionItem);

		CEntityHandle<CSelectable> hSelection(pItem->iUnit);

		if (hSelection != NULL)
		{
			DigitanksGame()->GetCurrentTeam()->SetPrimarySelection(hSelection);
			DigitanksGame()->GetDigitanksCamera()->SetTarget(hSelection->GetOrigin());
			if (DigitanksGame()->GetDigitanksCamera()->GetDistance() > 250)
				DigitanksGame()->GetDigitanksCamera()->SetDistance(250);
		}
	}

	m_flSmallActionItemLerpGoal = 0;
}

void CHUD::ShowSmallActionItemCallback()
{
	m_iCurrentSmallActionItem = ~0;
	m_flSmallActionItemLerpGoal = 0;

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
			m_iCurrentSmallActionItem = i;
			break;
		}
	}

	if (m_iCurrentSmallActionItem == ~0)
		return;

	m_flSmallActionItemLerpGoal = 1;

	if (!DigitanksGame()->GetCurrentLocalDigitanksTeam())
		return;

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetNumActionItems() == 0)
		return;

	const actionitem_t* pItem = DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetActionItem(m_iCurrentSmallActionItem);

	switch (pItem->eActionType)
	{
	case ACTIONTYPE_WELCOME:
		m_sSmallActionItem = L"WELCOME TO DIGITANKS";
		break;

	case ACTIONTYPE_CONTROLS:
		m_sSmallActionItem = L"QUICK CONTROLS";
		break;

	case ACTIONTYPE_NEWSTRUCTURE:
		m_sSmallActionItem = L"STRUCTURE COMPLETED";
		break;

	case ACTIONTYPE_UNITORDERS:
		m_sSmallActionItem = L"ORDERS NEEDED";
		break;

	case ACTIONTYPE_UNITAUTOMOVE:
		m_sSmallActionItem = L"AUTO-MOVE COMPLETED";
		break;

	case ACTIONTYPE_AUTOMOVECANCELED:
		m_sSmallActionItem = L"AUTO-MOVE INTERRUPTED";
		break;

	case ACTIONTYPE_AUTOMOVEENEMY:
		m_sSmallActionItem = L"AUTO-MOVE THREAT";
		break;

	case ACTIONTYPE_UNITDAMAGED:
		m_sSmallActionItem = L"UNIT DAMAGED";
		break;

	case ACTIONTYPE_FORTIFIEDENEMY:
		m_sSmallActionItem = L"ENEMY IN RANGE";
		break;

	case ACTIONTYPE_UPGRADE:
		m_sSmallActionItem = L"UPRGADE COMPLETED";
		break;

	case ACTIONTYPE_UNITREADY:
		m_sSmallActionItem = L"UNIT COMPLETED";
		break;

	case ACTIONTYPE_DOWNLOADCOMPLETE:
		m_sSmallActionItem = L"DOWNLOAD COMPLETED";
		break;

	case ACTIONTYPE_DOWNLOADUPDATES:
		m_sSmallActionItem = L"DOWNLOAD UPDATES";
		break;
	}
}

void CHUD::HideSmallActionItemCallback()
{
	m_flSmallActionItemLerpGoal = 0;
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

	m_flActionItemsLerpGoal = 0;

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

	DigitanksWindow()->GetInstructor()->FinishedTutorial("mission-1-move-mode");

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
			DigitanksGame()->GetCurrentLocalDigitanksTeam()->HandledActionItem(DigitanksGame()->GetCurrentTeam()->GetTank(i));
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
			DigitanksGame()->GetCurrentLocalDigitanksTeam()->HandledActionItem(DigitanksGame()->GetCurrentTeam()->GetTank(i));
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

//	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
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

//	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
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

//	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_UPGRADE);
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

	if (!DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_MINIBUFFER);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	DigitanksWindow()->GetInstructor()->FinishedTutorial("strategy-buildbuffer", true);

	SetupMenu();
}

void CHUD::BuildBufferCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_BUFFER);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
}

void CHUD::BuildBatteryCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_BATTERY);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	DigitanksWindow()->GetInstructor()->FinishedTutorial("strategy-buildbuffer", true);

	SetupMenu();
}

void CHUD::BuildPSUCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_PSU);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
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

	if (!DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_INFANTRYLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
}

void CHUD::BuildTankLoaderCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_TANKLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
}

void CHUD::BuildArtilleryLoaderCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_ARTILLERYLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
}

void CHUD::CancelBuildCallback()
{
	DigitanksGame()->SetControlMode(MODE_NONE);
	SetupMenu();
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

	if (!DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->BeginRogueProduction();
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::BuildTurretCallback()
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentTeam()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_AUTOTURRET);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
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
		DigitanksWindow()->GetInstructor()->FinishedTutorial("artillery-aim", true);
	}
}

void CHUD::GoToMainCallback()
{
	SetupMenu(MENUMODE_MAIN);
}

void CHUD::ShowPowerInfoCallback()
{
	m_pTeamInfo->SetText("");

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
		return;

	CDigitanksTeam* pCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (!pCurrentTeam)
		return;

	m_pTeamInfo->AppendText("POWER INFO\n \n");

	m_pTeamInfo->AppendText(sprintf(L"Power stored: %.1f\n", pCurrentTeam->GetPower()));
	m_pTeamInfo->AppendText(sprintf(L"Power per turn: +%.1f\n \n", pCurrentTeam->GetPowerPerTurn()));

	for (size_t i = 0; i < pCurrentTeam->GetNumMembers(); i++)
	{
		CBaseEntity* pEntity = pCurrentTeam->GetMember(i);
		if (!pEntity)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);
		if (!pStructure)
			continue;

		if (pStructure->Power() > 0)
			m_pTeamInfo->AppendText(sprintf(pStructure->GetEntityName() + L": +%.1f\n", pStructure->Power()));

		CCollector* pCollector = dynamic_cast<CCollector*>(pEntity);
		if (!pCollector)
			continue;

		if (pCollector->IsConstructing())
			continue;

		float flEfficiency;
		if (!pCollector->GetSupplier() || !pCollector->GetSupplyLine())
			flEfficiency = 0;
		else
			flEfficiency = pCollector->GetSupplier()->GetChildEfficiency() * pCollector->GetSupplyLine()->GetIntegrity();

		m_pTeamInfo->AppendText(sprintf(pCollector->GetEntityName() + L": +%.1f (%d%%)\n", pCollector->GetPowerProduced(), (int)(flEfficiency*100)));
	}

	LayoutTeamInfo();
}

void CHUD::ShowFleetInfoCallback()
{
	m_pTeamInfo->SetText("");

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
		return;

	CDigitanksTeam* pCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (!pCurrentTeam)
		return;

	m_pTeamInfo->AppendText("FLEET INFO\n \n");

	m_pTeamInfo->AppendText(sprintf(L"Fleet points available: %d\n", pCurrentTeam->GetTotalFleetPoints()));
	m_pTeamInfo->AppendText(sprintf(L"Fleet points in use: %d\n \n", pCurrentTeam->GetUsedFleetPoints()));

	int iScouts = 0;
	int iTanks = 0;
	int iArtillery = 0;
	int iInfantry = 0;

	m_pTeamInfo->AppendText(L"Suppliers:\n");
	for (size_t i = 0; i < pCurrentTeam->GetNumMembers(); i++)
	{
		CBaseEntity* pEntity = pCurrentTeam->GetMember(i);
		if (!pEntity)
			continue;

		CDigitanksEntity* pDTEnt = dynamic_cast<CDigitanksEntity*>(pEntity);
		if (pDTEnt)
		{
			if (pDTEnt->GetUnitType() == UNIT_SCOUT)
				iScouts++;
			else if (pDTEnt->GetUnitType() == UNIT_TANK)
				iTanks++;
			else if (pDTEnt->GetUnitType() == UNIT_INFANTRY)
				iInfantry++;
			else if (pDTEnt->GetUnitType() == UNIT_ARTILLERY)
				iArtillery++;
		}

		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);
		if (!pStructure)
			continue;

		if (pStructure->IsConstructing())
			continue;

		if (pStructure->FleetPoints() > 0)
			m_pTeamInfo->AppendText(sprintf(pStructure->GetEntityName() + L": +%d\n", pStructure->FleetPoints()));
	}

	m_pTeamInfo->AppendText(L" \nFleet:\n");
	if (iScouts)
		m_pTeamInfo->AppendText(sprintf(L"%d Rogues (%d): %d\n", iScouts, CScout::ScoutFleetPoints(), iScouts*CScout::ScoutFleetPoints()));
	if (iInfantry)
		m_pTeamInfo->AppendText(sprintf(L"%d Resistors (%d): %d\n", iInfantry, CMechInfantry::InfantryFleetPoints(), iInfantry*CMechInfantry::InfantryFleetPoints()));
	if (iTanks)
		m_pTeamInfo->AppendText(sprintf(L"%d Digitanks (%d): %d\n", iTanks, CMainBattleTank::MainTankFleetPoints(), iTanks*CMainBattleTank::MainTankFleetPoints()));
	if (iArtillery)
		m_pTeamInfo->AppendText(sprintf(L"%d Artillery (%d): %d\n", iArtillery, CArtillery::ArtilleryFleetPoints(), iArtillery*CArtillery::ArtilleryFleetPoints()));

	LayoutTeamInfo();
}

void CHUD::ShowBandwidthInfoCallback()
{
	m_pTeamInfo->SetText("");

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
		return;

	CDigitanksTeam* pCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (!pCurrentTeam)
		return;

	m_pTeamInfo->AppendText("BANDWIDTH INFO\n \n");

	if (pCurrentTeam->GetUpdateDownloading())
	{
		m_pTeamInfo->AppendText(sprintf(L"Downloading: %s\n", pCurrentTeam->GetUpdateDownloading()->GetName()));
		m_pTeamInfo->AppendText(sprintf(L"Progress: %.1f/%dmb (%d turns)\n", pCurrentTeam->GetUpdateDownloaded(), (int)pCurrentTeam->GetUpdateSize(), pCurrentTeam->GetTurnsToDownload()));
		m_pTeamInfo->AppendText(sprintf(L"Download rate: %.1fmb/turn\n \n", pCurrentTeam->GetBandwidth()));
	}

	for (size_t i = 0; i < pCurrentTeam->GetNumMembers(); i++)
	{
		CBaseEntity* pEntity = pCurrentTeam->GetMember(i);
		if (!pEntity)
			continue;

		CStructure* pStructure = dynamic_cast<CStructure*>(pEntity);
		if (!pStructure)
			continue;

		if (pStructure->IsConstructing())
			continue;

		if (pStructure->Bandwidth() > 0)
			m_pTeamInfo->AppendText(sprintf(pStructure->GetEntityName() + L": +%.1fmb\n", pStructure->Bandwidth()));
	}

	LayoutTeamInfo();
}

void CHUD::HideTeamInfoCallback()
{
	m_pTeamInfo->SetText("");
}

void CHUD::FireTurretCallback()
{
	CAutoTurret* pTurret = dynamic_cast<CAutoTurret*>(DigitanksGame()->GetPrimarySelection());

	if (!pTurret)
		return;

	pTurret->Fire();
}

void CHUD::LayoutTeamInfo()
{
	if (m_pTeamInfo->GetText().length() == 0)
		return;

	m_pTeamInfo->ComputeLines();
	m_pTeamInfo->SetSize(300, (int)m_pTeamInfo->GetTextHeight());
	m_pTeamInfo->SetPos(GetWidth() - m_pTeamInfo->GetWidth() - 50, 50);
}

void CHUD::SetNeedsUpdate()
{
	DigitanksWindow()->GetHUD()->m_bNeedsUpdate = true;
}

void CHUD::SetTeamMembersUpdated()
{
	DigitanksWindow()->GetHUD()->m_pSceneTree->OnTeamMembersUpdated();
}

CDamageIndicator::CDamageIndicator(CBaseEntity* pVictim, float flDamage, bool bShield)
	: CLabel(0, 0, 100, 100, L"")
{
	m_hVictim = pVictim;
	m_flDamage = flDamage;
	m_bShield = bShield;
	m_flTime = GameServer()->GetGameTime();

	if (pVictim)
		m_vecLastOrigin = pVictim->GetOrigin();

	glgui::CRootPanel::Get()->AddControl(this, true);

	Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(m_vecLastOrigin);
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

	SetFont(L"header", 18);
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

	SetFont(L"header", 18);
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

	SetFont(L"smileys", 18);
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
	vecScreen.y -= flWidth;

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
		CHUD::PaintHUDSheet("SpeechBubble", x, y, w, h, Color(255, 255, 255, GetAlpha()));
	} while (false);

	BaseClass::Paint(x, y, w, h);
}

CHowToPlayPanel::CHowToPlayPanel()
	: CPanel(0, 0, 100, 100)
{
	m_pOpen = new CLabel(0, 0, 100, 100, L"", L"header", 18);
	AddControl(m_pOpen);

	m_bOpen = false;

	m_flGoalLerp = 0;
	m_flCurLerp = 0;

	m_pControls = new CLabel(0, 0, 100, 100, L"", L"text", 13);
	AddControl(m_pControls);
}

void CHowToPlayPanel::Layout()
{
	BaseClass::Layout();

	if (IsOpen())
		m_pOpen->SetText(L"Click to close");
	else
		m_pOpen->SetText(L"How to play");

	m_pOpen->SetSize(150, 50);
	m_pOpen->SetPos(0, 100);
	m_pOpen->SetAlign(CLabel::TA_MIDDLECENTER);

	m_pControls->SetSize(280, 250);
	m_pControls->SetPos(10, 30);
	m_pControls->SetAlign(CLabel::TA_TOPLEFT);

	m_pControls->SetText(
		"Scroll view: Hold spacebar\n"
		"Rotate view: Hold right click\n"
		"Zoom view: Scrollwheel or pgup/pgdn\n"
		"Select similar units: Double click\n"
	);
}

void CHowToPlayPanel::Think()
{
	BaseClass::Think();

	m_flCurLerp = Approach(m_flGoalLerp, m_flCurLerp, GameServer()->GetFrameTime()*2);

	SetPos(150, (int)(RemapVal(Lerp(m_flCurLerp, 0.6f), 0, 1, -100, 0)));
	SetSize((int)(RemapVal(Lerp(m_flCurLerp, 0.6f), 0, 1, 150, 300)), 150);
	m_pOpen->SetSize((int)(RemapVal(Lerp(m_flCurLerp, 0.6f), 0, 1, 150, 300)), 50);
	m_pControls->SetAlpha(m_flCurLerp);
}

void CHowToPlayPanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h);

	BaseClass::Paint(x, y, w, h);
}

bool CHowToPlayPanel::IsOpen()
{
	return m_bOpen;
}

void CHowToPlayPanel::Open()
{
	m_bOpen = true;
	m_flGoalLerp = 1;

	Layout();
}

void CHowToPlayPanel::Close()
{
	m_bOpen = false;
	m_flGoalLerp = 0;

	Layout();
}

bool CHowToPlayPanel::MousePressed(int code, int mx, int my)
{
	if (IsOpen())
		Close();
	else
		Open();

	return true;
}

bool CHowToPlayPanel::MouseReleased(int code, int mx, int my)
{
	return true;
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
