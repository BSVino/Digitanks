#include "hud.h"

#include <geometry.h>
#include <strutils.h>

#include <tinker/cvar.h>
#include <glgui/rootpanel.h>
#include <tinker/profiler.h>

#include <sound/sound.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>
#include <game/cameramanager.h>

#include "digitankswindow.h"
#include "digitanksgame.h"
#include <structures/cpu.h>
#include <weapons/projectile.h>
#include <structures/loader.h>
#include <structures/props.h>
#include <structures/collector.h>
#include <structures/autoturret.h>
#include <units/mobilecpu.h>
#include <units/scout.h>
#include <units/artillery.h>
#include <units/mechinf.h>
#include <units/maintank.h>
#include <dt_camera.h>
#include <weapons/cameraguided.h>
#include <campaign/userfile.h>
#include "weaponpanel.h"
#include "scenetree.h"

using namespace glgui;

#define _T(x) x

CVar hud_enable("hud_enable", "on");

CPowerBar::CPowerBar(powerbar_type_t ePowerbarType)
	: CLabel(0, 0, 100, 100, "", "text")
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
		if (pTank && pTank->GetShieldMaxStrength() && pTank->TakesDamage())
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
		if (strlen(pSelection->GetPowerBar1Text()) && pSelection->GetPlayerOwner() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
		{
			sprintf(szLabel, "%s: %d%%", pSelection->GetPowerBar1Text(), (int)(pSelection->GetPowerBar1Value()*100));
			SetText(szLabel);
		}
		else
			SetText("");
	}
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
	{
		if (strlen(pSelection->GetPowerBar2Text()) && pSelection->GetPlayerOwner() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
		{
			sprintf(szLabel, "%s: %d%%", pSelection->GetPowerBar2Text(), (int)(pSelection->GetPowerBar2Value()*100));
			SetText(szLabel);
		}
		else
			SetText("");
	}
	else
	{
		if (strlen(pSelection->GetPowerBar3Text()) && pSelection->GetPlayerOwner() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
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

	SetFont("text", iSize);
	while (iSize > 0 && GetTextWidth() > GetWidth()-1)
		SetFont("text", --iSize);
}

void CPowerBar::Paint(float x, float y, float w, float h)
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetPrimarySelection())
		return;

	CSelectable* pSelection = DigitanksGame()->GetPrimarySelection();

	if (m_ePowerbarType == POWERBAR_SHIELD)
	{
		CDigitank* pTank = dynamic_cast<CDigitank*>(pSelection);
		if (pTank && pTank->GetShieldMaxStrength() && pTank->TakesDamage())
		{
			float flShield = pTank->GetShieldStrength() * pTank->GetShieldMaxStrength();
			float flShieldMax = pTank->GetShieldMaxStrength() * pTank->GetDefenseScale(true);
			CRootPanel::PaintRect(x+1, y+1, (int)(w * flShield / flShieldMax)-2, h-2, Color(80, 80, 80));
		}
	}
	else if (m_ePowerbarType == POWERBAR_HEALTH)
	{
		CDigitank* pTank = dynamic_cast<CDigitank*>(pSelection);
		if (!pTank || pTank->TakesDamage())
			CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetHealth() / pSelection->GetTotalHealth())-2, h-2, Color(0, 150, 0));
	}
	else if (m_ePowerbarType == POWERBAR_ATTACK)
	{
		if (pSelection->GetPlayerOwner() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
			CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetPowerBar1Size())-2, h-2, Color(150, 0, 0));
	}
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
	{
		if (pSelection->GetPlayerOwner() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
			CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetPowerBar2Size())-2, h-2, Color(0, 0, 150));
	}
	else
	{
		if (pSelection->GetPlayerOwner() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
			CRootPanel::PaintRect(x+1, y+1, (int)(w * pSelection->GetPowerBar3Size())-2, h-2, Color(100, 100, 0));
	}

	BaseClass::Paint(x, y, w, h);
}

CHUD::CHUD()
	: CHUDViewport(),
	m_HUDSheet("textures/hud/hud-sheet.txt"),
	m_UnitsSheet("textures/hud/units-sheet.txt"),
	m_WeaponsSheet("textures/hud/hud-weapons-01.txt"),
	m_ButtonSheet("textures/hud/hud-menu-sheet-01.txt"),
	m_DownloadSheet("textures/hud/hud-download-sheet-01.txt"),
	m_KeysSheet("textures/hud/keys.txt"),
	m_ActionSignsSheet("textures/hud/actionsigns/signs.txt"),
	m_PowerupsSheet("textures/hud/powerups-sheet.txt")
{
	m_bNeedsUpdate = false;

	m_pShieldBar = AddControl(new CPowerBar(POWERBAR_SHIELD));
	m_pHealthBar = AddControl(new CPowerBar(POWERBAR_HEALTH));
	m_pAttackPower = AddControl(new CPowerBar(POWERBAR_ATTACK));
	m_pDefensePower = AddControl(new CPowerBar(POWERBAR_DEFENSE));
	m_pMovementPower = AddControl(new CPowerBar(POWERBAR_MOVEMENT));

	m_hActionTanksSheet = CMaterialLibrary::AddMaterial(_T("textures/hud/actionsigns/tanks.mat"));
	m_hPurchasePanel = CMaterialLibrary::AddMaterial(_T("textures/purchasepanel.mat"));
	m_hShieldTexture = CMaterialLibrary::AddMaterial(_T("textures/hud/hud-shield.mat"));
	m_hSelectorMedalTexture = CMaterialLibrary::AddMaterial(_T("textures/hud/selector-medal.mat"));

	m_eActionSign = ACTIONSIGN_NONE;

	m_pActionItem = AddControl(new CLabel(0, 0, 10, 10, _T("")));
	m_pActionItem->SetFont(_T("text"));
	m_pCloseActionItems = AddControl(new CButton(0, 0, 100, 50, _T("Close")));
	m_pCloseActionItems->SetFont(_T("header"));
	m_flActionItemsLerp = m_flActionItemsLerpGoal = 0;
	m_flActionItemsWidth = 280;
	m_flSmallActionItemLerp = m_flSmallActionItemLerpGoal = 0;
	m_iCurrentSmallActionItem = ~0;

	m_pButtonPanel = AddControl(new CMouseCapturePanel());

	for (size_t i = 0; i < NUM_BUTTONS; i++)
	{
		m_apButtons[i] = m_pButtonPanel->AddControl(new CPictureButton(_T("")));
		m_apButtons[i]->SetCursorOutListener(this, ButtonCursorOut);
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

	m_pAttackInfo = AddControl(new CLabel(0, 0, 100, 150, _T("")));
	m_pAttackInfo->SetWrap(false);
	m_pAttackInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pAttackInfo->SetFont(_T("text"));

	m_pScoreboard = AddControl(new CLabel(0, 0, 100, 150, _T("")));
	m_pScoreboard->SetWrap(false);
	m_pScoreboard->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_pScoreboard->SetFont(_T("text"), 10);

	m_pTankInfo = AddControl(new CLabel(0, 0, 100, 100, _T("")));
	m_pTankInfo->SetFont(_T("text"), 10);

	m_pTurnInfo = AddControl(new CLabel(0, 0, 100, 100, _T("")));
	m_pTurnInfo->SetFont(_T("text"), 10);

	m_pResearchInfo = AddControl(new CLabel(0, 0, 100, 100, _T("")));
	m_pResearchInfo->SetFont(_T("text"));

	m_pButtonInfo = AddControl(new CLabel(0, 0, 100, 100, _T("")));
	m_pButtonInfo->SetFont(_T("text"));

	m_pPressEnter = AddControl(new CLabel(0, 0, 100, 100, _T("")));
	m_pPressEnter->SetFont(_T("text"));

	SetupMenu(MENUMODE_MAIN);

	m_pDemoNotice = AddControl(new CLabel(0, 0, 100, 20, _T("")));
	m_pDemoNotice->SetFont(_T("text"));

	m_pDemoNotice->SetAlign(CLabel::TA_TOPLEFT);
	m_pDemoNotice->SetPos(20, 20);
	m_pDemoNotice->SetText("");

	m_pPowerInfo = AddControl(new CLabel(0, 0, 200, 20, _T("")));

	m_pPowerInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pPowerInfo->SetPos(200, 20);
	m_pPowerInfo->SetTextColor(Color(220, 220, 255));
	m_pPowerInfo->SetFont(_T("text"));
	m_pPowerInfo->SetCursorInListener(this, ShowPowerInfo);
	m_pPowerInfo->SetCursorOutListener(this, HideTeamInfo);

	m_pFleetInfo = AddControl(new CLabel(0, 0, 200, 20, _T("")));

	m_pFleetInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pFleetInfo->SetPos(200, 20);
	m_pFleetInfo->SetTextColor(Color(220, 220, 255));
	m_pFleetInfo->SetFont(_T("text"));
	m_pFleetInfo->SetCursorInListener(this, ShowFleetInfo);
	m_pFleetInfo->SetCursorOutListener(this, HideTeamInfo);

	m_pBandwidthInfo = AddControl(new CLabel(0, 0, 200, 20, _T("")));

	m_pBandwidthInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pBandwidthInfo->SetPos(200, 20);
	m_pBandwidthInfo->SetTextColor(Color(220, 220, 255));
	m_pBandwidthInfo->SetFont(_T("text"));
	m_pBandwidthInfo->SetCursorInListener(this, ShowBandwidthInfo);
	m_pBandwidthInfo->SetCursorOutListener(this, HideTeamInfo);

	m_pTeamInfo = AddControl(new CLabel(0, 0, 200, 20, _T("")));

	m_pTeamInfo->SetAlign(CLabel::TA_TOPLEFT);
	m_pTeamInfo->SetPos(200, 20);
	m_pTeamInfo->SetTextColor(Color(255, 255, 255));
	m_pTeamInfo->SetFont(_T("text"));

	m_pUpdatesButton = AddControl(new CPictureButton(_T("Download Grid")));
	m_pUpdatesButton->SetClickedListener(this, OpenUpdates);
	m_pUpdatesButton->ShowBackground(false);
	m_pUpdatesButton->SetTooltip(_T("Download Grid"));

	m_pUpdatesPanel = AddControl(new CUpdatesPanel());
	m_pUpdatesPanel->SetVisible(false);

	m_flUpdateIconSlide = 0;

	m_pWeaponPanel = AddControl(new CWeaponPanel());
	m_pWeaponPanel->SetVisible(false);

	m_pSceneTree = AddControl(new CSceneTree());

	m_pTurnButton = AddControl(new CPictureButton(_T("TURN")));
	SetButtonSheetTexture(m_pTurnButton, &m_HUDSheet, "EndTurn");
	m_pTurnButton->SetClickedListener(this, EndTurn);
	m_pTurnButton->ShowBackground(false);
	m_pTurnButton->SetBorder(BT_NONE);
	m_pTurnButton->SetCursorInListener(this, CursorInTurnButton);
	m_pTurnButton->SetCursorOutListener(this, CursorOutTurnButton);

	m_pTurnWarning = AddControl(new CLabel(0, 0, 100, 100, _T("")));
	m_pTurnWarning->SetAlign(CLabel::TA_TOPLEFT);
	m_pTurnWarning->SetTextColor(Color(255, 255, 255));
	m_pTurnWarning->SetFont(_T("text"));

	m_flTurnWarningGoal = 0;
	m_flTurnWarningLerp = 0;

	m_flAttackInfoAlpha = m_flAttackInfoAlphaGoal = 0;

	m_flTurnInfoLerp = m_flTurnInfoLerpGoal = 0;
	m_flTurnInfoHeight = m_flTurnInfoHeightGoal = 0;

	m_flSelectorMedalStart = 0;

	m_flFileRescueStart = 0;

	m_iTurnSound = CSoundLibrary::Get()->AddSound(_T("sound/turn.wav"));

	m_pSpacebarHint = AddControl(new CLabel(0, 0, 200, 20, _T("")));
	m_pSpacebarHint->SetAlign(CLabel::TA_MIDDLECENTER);
	m_pSpacebarHint->SetFont(_T("text"));

	m_pHowToPlayPanel = AddControl(new CHowToPlayPanel());

#ifdef DT_COMPETITION
	m_iCompetitionWatermark = CTextureLibrary::AddTextureID(_T("textures/competition.png"));
#endif
}

void CHUD::Layout()
{
	if (!DigitanksGame())
		return;

	BaseClass::Layout();

	int iWidth = RootPanel()->GetWidth();
	int iHeight = RootPanel()->GetHeight();

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
	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() && DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetPrimaryCPU())
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
		RemoveControl(m_apActionItemButtons[i]);

	m_apActionItemButtons.clear();

	CDigitanksPlayer* pLocalCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();
	if (pLocalCurrentTeam && bShowCPUStuff)
	{
		size_t iItemButtonSize = 30;
		for (size_t i = 0; i < pLocalCurrentTeam->GetNumActionItems(); i++)
		{
			CControl<CPictureButton> pButton = AddControl(new CPictureButton(tsprintf(tstring("%d"), i)));
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
				pButton->SetTooltip(_T("Intro"));
				break;

			case ACTIONTYPE_CONTROLS:
				// Use the fleet logo, which is also the digitanks logo, for the welcome icon.
				SetButtonSheetTexture(pButton, &m_HUDSheet, "FleetPointsIcon");
				pButton->SetTooltip(_T("Controls"));
				break;

			case ACTIONTYPE_NEWSTRUCTURE:
				if (hUnit != NULL)
				{
					CMaterialHandle hSheet;
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(_T("Structure Complete"));
				}
				break;

			case ACTIONTYPE_AUTOMOVECANCELED:
				if (hUnit != NULL)
				{
					CMaterialHandle hSheet;
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(_T("Move Canceled"));
				}
				break;

			case ACTIONTYPE_AUTOMOVEENEMY:
				if (hUnit != NULL)
				{
					CMaterialHandle hSheet;
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(_T("Enemy Sighted"));
				}
				break;

			case ACTIONTYPE_UNITDAMAGED:
				if (hUnit != NULL)
				{
					CMaterialHandle hSheet;
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(_T("Unit Damaged"));
				}
				break;

			case ACTIONTYPE_FORTIFIEDENEMY:
				if (hUnit != NULL)
				{
					CMaterialHandle hSheet;
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(_T("Enemy Sighted"));
				}
				break;

			case ACTIONTYPE_UNITAUTOMOVE:
				if (hUnit != NULL)
				{
					CMaterialHandle hSheet;
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(_T("Move Completed"));
				}
				break;

			case ACTIONTYPE_UNITORDERS:
				if (hUnit != NULL)
				{
					CMaterialHandle hSheet;
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(_T("Orders Needed"));
				}
				break;

			case ACTIONTYPE_UPGRADE:
				if (hUnit != NULL)
				{
					CMaterialHandle hSheet;
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(_T("Upgrade Compeleted"));
				}
				break;

			case ACTIONTYPE_UNITREADY:
				if (hUnit != NULL)
				{
					CMaterialHandle hSheet;
					int sx, sy, sw, sh, tw, th;
					GetUnitSheet(hUnit->GetUnitType(), hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetSheetTexture(hSheet, sx, sy, sw, sh, tw, th);
					pButton->SetTooltip(_T("Unit Ready"));
				}
				break;

			case ACTIONTYPE_DOWNLOADUPDATES:
				SetButtonSheetTexture(pButton, &m_HUDSheet, "BandwidthIcon");
				pButton->SetTooltip(_T("Download Grid"));
				break;

			case ACTIONTYPE_DOWNLOADCOMPLETE:
				SetButtonSheetTexture(pButton, &m_HUDSheet, "BandwidthIcon");
				pButton->SetTooltip(_T("Download Complete"));
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
		m_pTurnInfo->SetText(_T(""));

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
				tstring s;
				s = tsprintf((pItem->GetName() + _T(" (%d)")).c_str(), pLocalCurrentTeam->GetTurnsToDownload());
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

	m_pTurnWarning->SetPos(iWidth - 165, iHeight - 150 - 90 - 10);
	m_pTurnWarning->SetSize(165, 90);

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

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer())
	{
		CMaterialHandle hSheet;
		int sx, sy, sw, sh, tw, th;
		m_pUpdatesPanel->GetTextureForUpdateItem(DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetUpdateDownloading(), hSheet, sx, sy, sw, sh, tw, th);
		m_pUpdatesButton->SetSheetTexture(hSheet, sx, sy, sw, sh, tw, th);
	}

	if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
	{
		// Don't clear it in the start, we want dead tanks to remain in the list so we can mark them asploded.

		m_ahScoreboardTanks.resize(DigitanksGame()->GetNumPlayers());

		for (size_t i = 0; i < DigitanksGame()->GetNumPlayers(); i++)
		{
			const CDigitanksPlayer* pTeam = DigitanksGame()->GetDigitanksPlayer(i);
			tmap<size_t, CEntityHandle<CDigitank> >& ahTeamTanks = m_ahScoreboardTanks[i];

			for (size_t j = 0; j < pTeam->GetNumTanks(); j++)
			{
				const CDigitank* pTank = pTeam->GetTank(j);
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

	if (!DigitanksGame())
		return;

	if (!IsVisible())
		return;

	BaseClass::Think();

	CDigitank* pCurrentTank = DigitanksGame()->GetPrimarySelectionTank();

	Vector vecTerrainPoint, vecEntityPoint;
	bool bMouseOnGrid = false;
	CBaseEntity* pHit = NULL;

#ifndef T_PLATFORM_TOUCH
	if (DigitanksGame()->GetControlMode() == MODE_NONE)
	{
		if (!CVar::GetCVarBool("m_emulate_touch"))
			bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecEntityPoint, &pHit);

		bool bSpotVisible = false;
		if (DigitanksGame()->GetCurrentLocalDigitanksPlayer())
			bSpotVisible = DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetVisibilityAtPoint(vecEntityPoint) > 0.2f;

		if (pHit && dynamic_cast<CSelectable*>(pHit) && bSpotVisible && !DigitanksGame()->IsFeatureDisabled(DISABLE_SELECT))
			DigitanksWindow()->SetMouseCursor(MOUSECURSOR_SELECT);

		if (bSpotVisible)
		{
			CDigitanksEntity* pDTHit = dynamic_cast<CDigitanksEntity*>(pHit);
			if (pDTHit)
				SetTooltip(pDTHit->GetEntityName());
			else
			{
				if (DigitanksGame()->GetTerrain()->IsPointInTrees(vecEntityPoint))
					SetTooltip(_T("Trees"));
				else if (DigitanksGame()->GetTerrain()->IsPointOverWater(vecEntityPoint))
					SetTooltip(_T("Interference"));
				else if (DigitanksGame()->GetTerrain()->IsPointOverLava(vecEntityPoint))
					SetTooltip(_T("Lava"));
				else
					SetTooltip(_T(""));
			}
		}
		else
			SetTooltip(_T(""));
	}
	else if (!CVar::GetCVarBool("m_emulate_touch"))
	{
		bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecEntityPoint, &pHit);
		bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecTerrainPoint, NULL, true);

		SetTooltip(_T(""));
	}
#endif

	if (pCurrentTank)
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		{
			float flMoveDistance = pCurrentTank->GetRemainingMovementDistance();
			if ((vecTerrainPoint - pCurrentTank->GetGlobalOrigin()).LengthSqr() > flMoveDistance*flMoveDistance)
				DigitanksWindow()->SetMouseCursor(MOUSECURSOR_MOVEAUTO);
			else
				DigitanksWindow()->SetMouseCursor(MOUSECURSOR_MOVE);
		}
		else if (DigitanksGame()->GetControlMode() == MODE_AIM)
		{
			CDigitanksEntity* pDTHit = dynamic_cast<CDigitanksEntity*>(pHit);
			CStaticProp* pSPHit = dynamic_cast<CStaticProp*>(pHit);
			if (pDTHit && pDTHit->TakesDamage() && pDTHit->GetPlayerOwner() != pCurrentTank->GetPlayerOwner() && pDTHit->GetVisibility() > 0.5f && !pSPHit)
			{
				if (pDTHit->GetUnitType() == UNIT_SCOUT && pCurrentTank->GetCurrentWeapon() != WEAPON_INFANTRYLASER)
					DigitanksWindow()->SetMouseCursor(MOUSECURSOR_AIM);
				else if (pCurrentTank->IsInsideMaxRange(pDTHit->GetGlobalOrigin()))
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
	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() && DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetPrimaryCPU())
		bShowCPUStuff = true;

	if (!bShowCPUStuff)
		m_flActionItemsLerp = m_flActionItemsLerpGoal = 0;

	m_flActionItemsLerp = Approach(m_flActionItemsLerpGoal, m_flActionItemsLerp, GameServer()->GetFrameTime());
	m_flSmallActionItemLerp = Approach(m_flSmallActionItemLerpGoal, m_flSmallActionItemLerp, GameServer()->GetFrameTime() * 4);

	int iWidth = RootPanel()->GetWidth();
	m_pActionItem->SetPos(iWidth - 300 + (int)(Bias(1-m_flActionItemsLerp, 0.2f) * m_flActionItemsWidth), 130);
	m_pActionItem->SetAlpha((int)(m_flActionItemsLerp*255));
	m_pCloseActionItems->SetPos(iWidth - 255 + (int)(Bias(1-m_flActionItemsLerp, 0.2f) * m_flActionItemsWidth), m_pCloseActionItems->GetTop());

	if (m_bHUDActive && bMouseOnGrid && pCurrentTank)
	{
		if (DigitanksGame()->GetControlMode() == MODE_MOVE || DigitanksGame()->GetControlMode() == MODE_TURN || DigitanksGame()->GetControlMode() == MODE_AIM)
			UpdateInfo();

		if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		{
			Vector vecMove = vecTerrainPoint;
			vecMove.z = pCurrentTank->FindHoverHeight(vecMove);
			pCurrentTank->SetPreviewMove(vecMove);
		}

		if (DigitanksGame()->GetControlMode() == MODE_TURN)
		{
			if ((vecTerrainPoint - pCurrentTank->GetGlobalOrigin()).LengthSqr() > 4*4)
			{
				Vector vecTurn = vecTerrainPoint - pCurrentTank->GetGlobalOrigin();
				vecTurn.Normalize();
				float flTurn = atan2(vecTurn.y, vecTurn.x) * 180/M_PI;
				pCurrentTank->SetPreviewTurn(flTurn);
			}
			else
				pCurrentTank->SetPreviewTurn(pCurrentTank->GetGlobalAngles().y);
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
					vecPreviewAim = DigitanksGame()->GetTerrain()->GetPointHeight(pDigitankHit->GetGlobalOrigin());
				else
					vecPreviewAim = vecTerrainPoint;
			}
			else if (pDTHit && !pSPHit && pDTHit->GetVisibility() > 0)
				vecPreviewAim = pDTHit->GetGlobalOrigin();
			else
				vecPreviewAim = vecEntityPoint;

			for (size_t i = 0; i < DigitanksGame()->GetCurrentPlayer()->GetNumTanks(); i++)
			{
				CDigitank* pTank = DigitanksGame()->GetCurrentPlayer()->GetTank(i);
				if (!pTank)
					continue;

				if (DigitanksGame()->GetCurrentPlayer()->IsSelected(pTank))
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

	if (m_bHUDActive && bMouseOnGrid && DigitanksGame()->GetCurrentLocalDigitanksPlayer())
	{
		if (DigitanksGame()->GetControlMode() == MODE_BUILD)
		{
			if (DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetPrimaryCPU())
				DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetPrimaryCPU()->SetPreviewBuild(vecEntityPoint);
		}
	}

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() && m_apActionItemButtons.size())
	{
		for (size_t i = 0; i < DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumActionItems(); i++)
		{
			if (i >= m_apActionItemButtons.size())
				break;

			if (DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetActionItem(i)->bHandled)
				m_apActionItemButtons[i]->SetAlpha(50);
			else
				m_apActionItemButtons[i]->SetAlpha((int)RemapVal(Oscillate(GameServer()->GetGameTime(), 1), 0, 1, 100, 255));
		}
	}

	if (m_eMenuMode == MENUMODE_MAIN && m_bHUDActive)
	{
		CLesson* pLesson = NULL;
		if (DigitanksWindow()->GetInstructor() && DigitanksWindow()->GetInstructor()->GetCurrentLesson())
			pLesson = DigitanksWindow()->GetInstructor()->GetCurrentLesson();

		tstring sLessonName;
		if (DigitanksWindow()->GetInstructor() && DigitanksWindow()->GetInstructor()->GetCurrentLesson())
			sLessonName = DigitanksWindow()->GetInstructor()->GetCurrentLesson()->m_sLessonName;

		if (pLesson && !!pLesson->m_hPointAt.Downcast<CPictureButton>() && DigitanksWindow()->GetInstructor() && DigitanksWindow()->GetInstructor()->GetCurrentPanel() && DigitanksWindow()->GetInstructor()->GetCurrentPanel()->IsVisible())
		{
			float flRamp = Oscillate(GameServer()->GetGameTime(), 1);
			int iColor = (int)RemapVal(flRamp, 0, 1, 0, 150);
			pLesson->m_hPointAt.DowncastStatic<CPictureButton>()->SetButtonColor(Color(iColor, 0, 0));
		}
		else if (pCurrentTank && pCurrentTank->GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer() && sLessonName == "artillery-aim")
		{
			float flRamp = Oscillate(GameServer()->GetGameTime(), 1);
			int iColor = (int)RemapVal(flRamp, 0, 1, 0, 150);
			m_apButtons[7]->SetButtonColor(Color(iColor, 0, 0));
		}
		else if (pCurrentTank && pCurrentTank->GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer() && sLessonName == "strategy-deploy" && dynamic_cast<CMobileCPU*>(pCurrentTank))
		{
			float flRamp = Oscillate(GameServer()->GetGameTime(), 1);
			m_apButtons[8]->SetButtonColor(Color(0, 0, (int)RemapVal(flRamp, 0, 1, 0, 250)));
		}
		// Don't blink other buttons if we're trying to blink this one.
		else
		{
			if (pCurrentTank && pCurrentTank->GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer() && pCurrentTank->HasBonusPoints())
			{
				float flRamp = Oscillate(GameServer()->GetGameTime(), 1);
				m_apButtons[4]->SetButtonColor(Color((int)RemapVal(flRamp, 0, 1, 0, 250), (int)RemapVal(flRamp, 0, 1, 0, 200), 0));
			}

			if (pCurrentTank && pCurrentTank->GetDigitanksPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer() && pCurrentTank->HasSpecialWeapons())
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
	m_flTurnWarningLerp = Approach(m_flTurnWarningGoal, m_flTurnWarningLerp, GameServer()->GetFrameTime()*5);

	m_flTurnInfoHeightGoal = m_pTurnInfo->GetTextHeight();
	m_flTurnInfoLerp = Approach(m_flTurnInfoLerpGoal, m_flTurnInfoLerp, GameServer()->GetFrameTime());
	m_flTurnInfoHeight = Approach(m_flTurnInfoHeightGoal, m_flTurnInfoHeight, GameServer()->GetFrameTime()*100);

	float flTurnInfoHeight = m_flTurnInfoHeight+10;
	m_pTurnInfo->SetSize(m_pTurnInfo->GetWidth(), (int)flTurnInfoHeight);
	m_pTurnInfo->SetPos(m_pTurnInfo->GetLeft(), 36 + 10 - (int)(Bias(1.0f-m_flTurnInfoLerp, 0.2f)*flTurnInfoHeight));

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

	CControlHandle pMouseControl = glgui::CRootPanel::Get()->GetHasCursor();
	if (DigitanksGame()->GetOverheadCamera()->HasCameraGuidedMissile())
	{
		DigitanksWindow()->SetMouseCursor(MOUSECURSOR_NONE);
		SetTooltip(_T(""));
	}
	else if (pMouseControl)
	{
		if (pMouseControl.Downcast<CHUD>())
		{
			// Nothing.
		}
		else if (pMouseControl->GetTooltip().length() > 0)
		{
			DigitanksWindow()->SetMouseCursor(MOUSECURSOR_NONE);
			SetTooltip(_T(""));
		}
		else if (pMouseControl.Downcast<CButton>())
		{
			DigitanksWindow()->SetMouseCursor(MOUSECURSOR_NONE);
			SetTooltip(_T(""));
		}
	}

	if (m_bBlinkTurnButton)
		m_pTurnButton->SetAlpha(Bias(Oscillate(GameServer()->GetGameTime(), 1.0f), 0.8f));
	else
		m_pTurnButton->SetAlpha(1.0f);

	SetVisible(hud_enable.GetBool() || DigitanksGame()->GetOverheadCamera()->HasCameraGuidedMissile());
}

#ifdef _DEBUG
#define SHOW_FPS "1"
#else
#define SHOW_FPS "0"
#endif

CVar show_fps("show_fps", SHOW_FPS);

void CHUD::Paint(float x, float y, float w, float h)
{
	TPROF("CHUD::Paint()");

	if (!DigitanksGame())
		return;

	if (GameServer()->IsLoading())
		return;

	if (DigitanksGame()->GetGameType() == GAMETYPE_MENU)
		return;

#ifdef DT_COMPETITION
	if (true)
	{
		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);
		CRootPanel::PaintTexture(m_iCompetitionWatermark, 70, 20, 128/2, 128/2);
	}
#endif

	if (show_fps.GetBool())
	{
		float flFontHeight = glgui::RootPanel()->GetFontHeight(_T("text"), 10);
		tstring sFPS = tsprintf(tstring("Time: %.2f"), GameServer()->GetGameTime());
		glgui::CLabel::PaintText(sFPS, sFPS.length(), _T("text"), 10, 125, flFontHeight + 50);
		sFPS = tsprintf(tstring("FPS: %d"), (int)(1 / GameServer()->GetFrameTime()));
		glgui::CLabel::PaintText(sFPS, sFPS.length(), _T("text"), 10, 125, flFontHeight*2 + 50);

#ifndef T_PLATFORM_TOUCH
		Vector vecTerrainPoint;
		if (DigitanksWindow()->GetMouseGridPosition(vecTerrainPoint, NULL, true))
		{
			sFPS = tsprintf(tstring("%.2f, %.2f"), vecTerrainPoint.x, vecTerrainPoint.y);
			glgui::CLabel::PaintText(sFPS, sFPS.length(), _T("text"), 10, 125, flFontHeight*3 + 50);
		}
#endif
	}

	if (DigitanksGame()->GetOverheadCamera()->HasCameraGuidedMissile())
	{
		PaintCameraGuidedMissile(x, y, w, h);
		return;
	}

	int iWidth = RootPanel()->GetWidth();
	int iHeight = RootPanel()->GetHeight();

	CDigitanksPlayer* pCurrentLocalPlayer = DigitanksGame()->GetCurrentLocalDigitanksPlayer();

	int iMouseX = 0, iMouseY = 0;
	
	if (pCurrentLocalPlayer)
	{
		iMouseX = pCurrentLocalPlayer->GetMouseCurrentX();
		iMouseY = pCurrentLocalPlayer->GetMouseCurrentY();
	}

	if ((DigitanksGame()->GetGameType() == GAMETYPE_STANDARD || DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN) && pCurrentLocalPlayer && pCurrentLocalPlayer->GetPrimaryCPU())
	{
		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);

		PaintHUDSheet("TeamInfoBackground", iWidth - 208, 0, 208, 341);
	}

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

		CDigitanksEntity* pDTEntity = dynamic_cast<CDigitanksEntity*>(pEntity);

		if (!pDTEntity)
			continue;

		Vector vecOrigin = pEntity->GetGlobalOrigin();

		Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(vecOrigin);

		float flRadius = pDTEntity->GetBoundingRadius();

		Vector vecUp;
		Vector vecForward;
		GameServer()->GetRenderer()->GetCameraVectors(&vecForward, NULL, &vecUp);

		// Cull tanks behind the camera
		if (vecForward.Dot((vecOrigin-GameServer()->GetCameraManager()->GetCameraPosition()).Normalized()) < 0)
			continue;

		if (pDTEntity->IsObjective() && pDTEntity->IsActive())
		{
			tstring sText = tsprintf(tstring("Objective: %s"), pDTEntity->GetEntityName().c_str());
			float flWidth = RootPanel()->GetTextWidth(sText, sText.length(), _T("text"), 13);
			CLabel::PaintText(sText, sText.length(), _T("text"), 13, vecScreen.x - flWidth/2, vecScreen.y + 60);

			CRenderingContext c(GameServer()->GetRenderer(), true);
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

		bool bMouseOver = false;
		if (fabs(vecScreen.x - iMouseX) < flWidth/2 && fabs(vecScreen.y - iMouseY) < flWidth/2)
			bMouseOver = true;

		bool bShowBox = false;
		if (pCurrentLocalPlayer && pCurrentLocalPlayer->IsSelected(pSelectable))
			bShowBox = true;
		if (bMouseOver)
			bShowBox = true;
		if (IsUpdatesPanelOpen())
			bShowBox = false;

		if (bShowBox)
		{
			Color clrSelection(255, 255, 255, 128);
			if (pCurrentLocalPlayer && pCurrentLocalPlayer->IsPrimarySelection(pSelectable))
				clrSelection = Color(255, 255, 255, 255);
			else if (pCurrentLocalPlayer && !pCurrentLocalPlayer->IsSelected(pSelectable) && bMouseOver)
				clrSelection = Color(255, 255, 255, 50);

			CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y - flWidth/2), (int)flWidth, 1, clrSelection);
			CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y - flWidth/2), 1, (int)flWidth, clrSelection);
			CRootPanel::PaintRect((int)(vecScreen.x + flWidth/2), (int)(vecScreen.y - flWidth/2), 1, (int)flWidth, clrSelection);
			CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y + flWidth/2), (int)flWidth, 1, clrSelection);

			if (pTank && pTank->GetPlayerOwner() == pCurrentLocalPlayer)
			{
				CRenderingContext c(GameServer()->GetRenderer(), true);
				c.SetBlend(BLEND_ALPHA);

				int iSize = 20;
				PaintWeaponSheet(pTank->GetCurrentWeapon(), (int)(vecScreen.x - flWidth/2) - 2, (int)(vecScreen.y + flWidth/2) - iSize + 2, iSize, iSize, Color(255, 255, 255, 255));
			}
		}

		bool bShowHealth = false;
		if (pCurrentLocalPlayer && (pEntity->GetOwner() == pCurrentLocalPlayer || pCurrentLocalPlayer->IsSelected(pSelectable)))
			bShowHealth = true;
		if (bMouseOver)
			bShowHealth = true;
		if (DigitanksWindow()->IsAltDown())
			bShowHealth = true;
		if (!pSelectable->ShowHealthBar())
			bShowHealth = false;

		if (bShowHealth)
		{
			CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2 - 1), (int)(vecScreen.y - flWidth/2 - 11), (int)flWidth + 2, 5, Color(255, 255, 255, 128));
			CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y - flWidth/2 - 10), (int)(flWidth*pEntity->GetHealth()/pEntity->GetTotalHealth()), 3, Color(100, 255, 100));
		}

		bool bShowPower = bMouseOver;
		if (DigitanksWindow()->IsAltDown())
			bShowPower = true;
		if (pEntity->GetOwner() != pCurrentLocalPlayer)
			bShowPower = false;
		if (!pTank)
			bShowPower = false;

		if (bShowPower)
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

		bool bShowGoalMove = pTank && pTank->GetPlayerOwner() == pCurrentLocalPlayer && pTank->HasGoalMovePosition();

		if (bShowGoalMove)
		{
			float flDistance = (pTank->GetRealOrigin() - pTank->GetGoalMovePosition()).Length();
			int iTurns = (int)(flDistance/pTank->GetMaxMovementDistance())+1;
				
			tstring sTurns = tsprintf(tstring(":%d"), iTurns);
			CLabel::PaintText(sTurns, sTurns.length(), _T("text"), 10, vecScreen.x + flWidth / 2 - 10, vecScreen.y - flWidth / 2 + RootPanel()->GetFontHeight(_T("text"), 10) - 2);
		}

		bool bShowFortify = pTank && (pTank->IsFortified() || pTank->IsFortifying());
		if (pEntity->GetOwner() != pCurrentLocalPlayer)
			bShowFortify = false;

		if (bShowFortify)
		{
			tstring sTurns = tsprintf(tstring("+%d"), pTank->GetFortifyLevel());

			float flYPosition = vecScreen.y + flWidth/2;
			float flXPosition = vecScreen.x + flWidth/2;
			float flIconSize = 10;

			CRenderingContext c(GameServer()->GetRenderer(), true);
			c.SetBlend(BLEND_ALPHA);

			Rect rArea = m_ButtonSheet.GetArea("Fortify");
			CBaseControl::PaintSheet(m_ButtonSheet.GetSheet("Fortify"),
				(int)(flXPosition - 14 - flIconSize), (int)(flYPosition - flIconSize)-1, (int)flIconSize, (int)flIconSize,
				rArea.x, rArea.y, rArea.w, rArea.y, m_ButtonSheet.GetSheetWidth("Fortify"), m_ButtonSheet.GetSheetHeight("Fortify"));
			CLabel::PaintText(sTurns, sTurns.length(), _T("text"), 10, flXPosition - 13, flYPosition - 3);
		}

		if (pStructure)
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

			if (pStructure->IsConstructing() || pStructure->IsUpgrading())
				CRootPanel::PaintRect((int)(vecScreen.x - flWidth/2), (int)(vecScreen.y - flWidth/2 - 2), (int)(flWidth*iTurnsProgressed/iTotalTurns), 3, Color(255, 255, 0));

			if (pStructure->IsConstructing())
			{
				tstring sTurns = tsprintf(tstring(":%d"), pStructure->GetTurnsRemainingToConstruct());
				CLabel::PaintText(sTurns, sTurns.length(), _T("text"), 10, vecScreen.x + flWidth / 2 - 10, vecScreen.y - flWidth / 2 + RootPanel()->GetFontHeight(_T("text"), 10) - 2);
			}
			else if (pStructure->IsUpgrading())
			{
				tstring sTurns = tsprintf(tstring(":%d"), pStructure->GetTurnsRemainingToUpgrade());
				CLabel::PaintText(sTurns, sTurns.length(), _T("text"), 10, vecScreen.x + flWidth / 2 - 10, vecScreen.y - flWidth / 2 + RootPanel()->GetFontHeight(_T("text"), 10) - 2);
			}

			if (pStructure->GetUnitType() == STRUCTURE_CPU)
			{
				CCPU* pCPU = static_cast<CCPU*>(pStructure);
				if (pCPU->IsProducing())
				{
					tstring sTurns = _T(":1");	// It only ever takes one turn to make a rogue.
					CLabel::PaintText(sTurns, sTurns.length(), _T("text"), 10, vecScreen.x + flWidth / 2 - 10, vecScreen.y - flWidth / 2 + RootPanel()->GetFontHeight(_T("text"), 10) - 2);
				}
			}

			if (pStructure->GetUnitType() == STRUCTURE_TANKLOADER || pStructure->GetUnitType() == STRUCTURE_INFANTRYLOADER || pStructure->GetUnitType() == STRUCTURE_ARTILLERYLOADER)
			{
				CLoader* pLoader = static_cast<CLoader*>(pStructure);
				if (pLoader->IsProducing())
				{
					tstring sTurns = tsprintf(tstring(":%d"), pLoader->GetTurnsRemainingToProduce());
					CLabel::PaintText(sTurns, sTurns.length(), _T("text"), 10, vecScreen.x + flWidth / 2 - 10, vecScreen.y - flWidth / 2 + RootPanel()->GetFontHeight(_T("text"), 10) - 2);
				}
			}
		}

		if (pTank && pTank->IsDisabled())
		{
			CRenderingContext c(GameServer()->GetRenderer(), true);
			c.SetBlend(BLEND_ALPHA);

			int iWidth = 77*2/3;
			int iHeight = 39*2/3;

			CHUD::PaintHUDSheet("TankDisabled", (int)(vecScreen.x + flWidth/2), (int)(vecScreen.y - flWidth + 2), iWidth, iHeight, Color(255, 255, 255, 255));

			int iFontSize = 17;

			tstring sZZZ = _T("zzz");
			float flTextWidth = RootPanel()->GetTextWidth(sZZZ, sZZZ.length(), _T("smileys"), iFontSize);
			float flTextHeight = RootPanel()->GetFontHeight(_T("smileys"), iFontSize);

			float flLerp = fmod(GameServer()->GetGameTime(), 2.0);
			if (flLerp < 0.5f)
				sZZZ = _T("z");
			else if (flLerp < 1.0f)
				sZZZ = _T("zz");

			CLabel::PaintText(sZZZ, sZZZ.length(), _T("smileys"), iFontSize, vecScreen.x + flWidth/2 + iWidth/2 - flTextWidth/2, vecScreen.y - flWidth + iHeight/2 + 3);
		}

		if (m_bHUDActive && pTank && DigitanksGame()->GetCurrentPlayer()->IsPrimarySelection(pTank) && DigitanksGame()->GetControlMode() == MODE_AIM)
			UpdateInfo();
	}

	bool bShowCPUStuff = false;
	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() && DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetPrimaryCPU())
		bShowCPUStuff = true;

	if (bShowCPUStuff)
		m_pCloseActionItems->Paint();

	do {
		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);

		// Main control pannel
		PaintHUDSheet("MainControlPanel", iWidth/2 - 720/2, iHeight-160, 720, 160);

		if (bShowCPUStuff && (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD || DigitanksGame()->GetGameType() == GAMETYPE_CAMPAIGN))
		{
			if (m_bUpdatesBlinking)
			{
				int iArrowWidth = 50;
				int iArrowHeight = 82;
				PaintHUDSheet("HintArrow", m_pUpdatesButton->GetLeft() + m_pUpdatesButton->GetWidth()/2 - iArrowWidth/2, 40 + (int)(Bias(Oscillate(GameServer()->GetGameTime(), 1), 0.8f)*20), iArrowWidth, iArrowHeight);
			}

			PaintHUDSheet("PowerIcon", iWidth - 190, 10, 20, 20);
			PaintHUDSheet("FleetPointsIcon", iWidth - 190, 40, 20, 20);
			PaintHUDSheet("BandwidthIcon", iWidth - 190, 70, 20, 20);

			PaintHUDSheet("TurnReport", m_pTurnInfo->GetLeft()-15, m_pTurnInfo->GetBottom()-585, 278, 600);

			int iResearchWidth = 617;
			PaintHUDSheet("ResearchBar", iWidth/2 - iResearchWidth/2, 0, iResearchWidth, 35);

			CRootPanel::PaintRect(iWidth/2 - iResearchWidth/2 - 34, 0, 34, 35, Color(0, 0, 0, 150));

			if (pCurrentLocalPlayer && pCurrentLocalPlayer->GetUpdateDownloading())
			{
				CUpdateItem* pItem = pCurrentLocalPlayer->GetUpdateDownloading();

				int iDownloadedWidth = 580;

				float flUpdateDownloaded = pCurrentLocalPlayer->GetUpdateDownloaded();
				if (flUpdateDownloaded > pItem->m_flSize)
					flUpdateDownloaded = pItem->m_flSize;
				int iResearchCompleted = (int)(iDownloadedWidth*(flUpdateDownloaded/pItem->m_flSize));
				int iResearchDue = (int)(iDownloadedWidth*(pCurrentLocalPlayer->GetBandwidth()/pItem->m_flSize));
				if (iResearchCompleted + iResearchDue > iDownloadedWidth)
					iResearchDue -= (iResearchCompleted + iResearchDue) - iDownloadedWidth;

				CRootPanel::PaintRect(iWidth/2 - iDownloadedWidth/2, 3, iResearchCompleted, 20, Color(0, 200, 100, 255));
				CRootPanel::PaintRect(iWidth/2 - iDownloadedWidth/2 + iResearchCompleted, 3, iResearchDue, 20, Color(0, 200, 100, 100));

				CMaterialHandle hItemSheet;
				int sx, sy, sw, sh, tw, th;
				m_pUpdatesPanel->GetTextureForUpdateItem(pItem, hItemSheet, sx, sy, sw, sh, tw, th);

				float flSlideTime = 0.8f;
				if (hItemSheet && GameServer()->GetGameTime() - m_flUpdateIconSlide < flSlideTime)
				{
					float flSlideLerp = Bias((GameServer()->GetGameTime() - m_flUpdateIconSlide)/flSlideTime, 0.7f);
					int iIconX = (int)RemapValClamped(flSlideLerp, 0.0f, 1.0f, (float)m_iUpdateIconSlideStartX, (float)(iWidth/2 - iResearchWidth/2 - 35));
					int iIconY = (int)RemapValClamped(flSlideLerp, 0.0f, 1.0f, (float)m_iUpdateIconSlideStartY, 0.0f);
					int iButtonSize = (int)RemapValClamped(flSlideLerp, 0.0f, 1.0f, (float)m_pUpdatesPanel->GetButtonSize(), 35.0f);

					CRootPanel::PaintSheet(hItemSheet, iIconX, iIconY, iButtonSize, iButtonSize, sx, sy, sw, sh, tw, th);
				}
			}
		}

		if (m_flAttackInfoAlpha > 0)
			PaintHUDSheet("AttackInfo", iWidth-175, m_pAttackInfo->GetTop()-15, 175, 110, Color(255, 255, 255, (int)(255*m_flAttackInfoAlpha)));

		if (m_flTurnWarningLerp > 0)
		{
			float flWarningLerp = Bias(m_flTurnWarningLerp, 0.7f);
			m_pTurnWarning->SetText(_T("WARNING!\n \nSome tanks still need orders. Ending the turn will forfeit their moves or attacks."));
			m_pTurnWarning->SetSize(220, 75);
			m_pTurnWarning->SetPos(iWidth - (int)(220*flWarningLerp), iHeight - 75 - 90 - 10);
			m_pTurnWarning->SetAlpha(m_flTurnWarningLerp);
			CRootPanel::PaintRect(m_pTurnWarning->GetLeft()-3, m_pTurnWarning->GetTop()-9, m_pTurnWarning->GetWidth()+6, m_pTurnWarning->GetHeight()+6);
		}
		else
		{
			m_pTurnWarning->SetAlpha(0);
		}

		if (bShowCPUStuff && m_flActionItemsLerp > 0)
			PaintHUDSheet("ActionItemPanel", m_pActionItem->GetLeft()-30, m_pActionItem->GetTop()-30, (int)m_flActionItemsWidth, 340, Color(255, 255, 255, (int)(255*m_flActionItemsLerp)));

		// Tank info
		PaintHUDSheet("TankInfo", 0, iHeight-250, 150, 250);
	} while (false);

	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		CRootPanel::PaintRect(m_pScoreboard->GetLeft()-3, m_pScoreboard->GetTop()-9, m_pScoreboard->GetWidth()+6, m_pScoreboard->GetHeight()+6, Color(0, 0, 0, 100));

	size_t iX, iY, iX2, iY2;
	if (pCurrentLocalPlayer && pCurrentLocalPlayer->GetBoxSelection(iX, iY, iX2, iY2))
	{
		Color clrSelection(255, 255, 255, 255);

		size_t iWidth = iX2 - iX;
		size_t iHeight = iY2 - iY;
		CRootPanel::PaintRect(iX, iY, iWidth, 1, clrSelection);
		CRootPanel::PaintRect(iX, iY, 1, iHeight, clrSelection);
		CRootPanel::PaintRect(iX + iWidth, iY, 1, iHeight, clrSelection);
		CRootPanel::PaintRect(iX, iY + iHeight, iWidth, 1, clrSelection);
	}

	if (pCurrentLocalPlayer && !m_hHintWeapon)
	{
		for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
			if (!pEntity)
				continue;

			CDigitanksWeapon* pWeapon = dynamic_cast<CDigitanksWeapon*>(pEntity);
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
			
			if (pWeapon->GetOwner()->GetPlayerOwner() != pCurrentLocalPlayer)
				continue;

			if (pWeapon->GetBonusDamage() > 0)
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

		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);

		iY -= (int)(Bias(Oscillate(GameServer()->GetGameTime(), 0.5f), 0.2f) * 10);

		PaintSheet(&m_KeysSheet, "SpaceBar", iX+45, iY, 60, 60);
	}
	else
	{
		m_pSpacebarHint->SetVisible(false);
	}

	if (DigitanksGame()->GetPrimarySelectionTank() && DigitanksGame()->GetControlMode() == MODE_AIM)
	{
		if (DigitanksGame()->GetAimType() == AIM_NORMAL)
		{
			CRenderingContext c(GameServer()->GetRenderer(), true);
			c.SetBlend(BLEND_ALPHA);

			int iX = GetWidth()/2 - 150;
			int iY = GetHeight()/2 - 150;

			bool bObstruction = false;
			CDigitank* pDigitank = DigitanksGame()->GetPrimarySelectionTank();
			Vector vecTankOrigin = pDigitank->GetGlobalOrigin() + Vector(0, 0, 1);

			float flGravity = DigitanksGame()->GetGravity();
			float flTime;
			Vector vecForce;
			FindLaunchVelocity(vecTankOrigin, pDigitank->GetPreviewAim(), flGravity, vecForce, flTime, pDigitank->ProjectileCurve());

			size_t iLinks = 10;
			float flTimePerLink = flTime/iLinks;
			Vector vecLastOrigin = vecTankOrigin;
			Vector vecPoint;
			for (size_t i = 1; i < iLinks; i++)
			{
				float flCurrentTime = flTimePerLink*i;
				Vector vecCurrentOrigin = vecTankOrigin + vecForce*flCurrentTime + Vector(0, 0, flGravity*flCurrentTime*flCurrentTime/2);
				if (DigitanksGame()->TraceLine(vecLastOrigin, vecCurrentOrigin, vecPoint, nullptr, true))
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

	if (m_pButtonInfo->GetText()[0] != _T('\0'))
	{
		CRootPanel::PaintRect(m_pButtonInfo->GetLeft()-3, m_pButtonInfo->GetTop()-9, m_pButtonInfo->GetWidth()+6, m_pButtonInfo->GetHeight()+6, Color(0, 0, 0));
		m_pButtonInfo->SetVisible(true);
		m_pButtonInfo->Paint();
	}

	CSelectable* pSelection = DigitanksGame()->GetPrimarySelection();

	if (pSelection)
	{
		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);

		int iSize = 100;
		CDigitank* pTank = DigitanksGame()->GetPrimarySelectionTank();
		if (pTank)
			iSize = 50;

		Color clrTeam(255, 255, 255, 255);
		if (pSelection->GetPlayerOwner())
			clrTeam = pSelection->GetPlayerOwner()->GetColor();
		PaintUnitSheet(pSelection->GetUnitType(), iWidth/2 - 720/2 + 10 + 150/2 - iSize/2, iHeight - 150 + 10 + 130/2 - iSize/2, iSize, iSize, clrTeam);

		pSelection->DrawSchema(iWidth/2 - 720/2 + 10 + 150/2 - 100/2, iHeight - 150 + 10 + 130/2 - 100/2, 100, 100);
		pSelection->DrawQueue(iWidth/2 - 720/2 + 162, iHeight - 90, 216, 84);
	}

	CDigitank* pTank = DigitanksGame()->GetPrimarySelectionTank();

	if (pTank)
	{
		if (pTank->GetShieldValue() > 5)
		{
			CRenderingContext c(GameServer()->GetRenderer(), true);

			c.SetBlend(BLEND_ADDITIVE);

			float flFlicker = 1;
	
			if (pTank->GetShieldValue() < pTank->GetShieldMaxStrength()*3/4)
				flFlicker = Flicker("zzzzmmzzztzzzzzznzzz", GameServer()->GetGameTime() + ((float)pTank->GetSpawnSeed()/100), 1.0f);

			int iShield = (int)(255*flFlicker*pTank->GetDefensePower(true)/pTank->GetStartingPower());
			if (iShield > 255)
				iShield = 255;

			int iSize = 100;

			glgui::CBaseControl::PaintTexture(m_hShieldTexture, iWidth/2 - 720/2 + 10 + 150/2 - iSize/2, iHeight - 150 + 10 + 130/2 - iSize/2, iSize, iSize, Color(255, 255, 255, iShield));
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

			size_t iTotalWidth = RootPanel()->GetWidth();
			size_t iTotalHeight = RootPanel()->GetHeight();

			size_t iTankWidth = iTotalWidth/4;
			size_t iTankHeight = iTankWidth/2;

			size_t iTankOffset = (size_t)(Bias(1-flWarpIn, 0.2f) * iTankWidth*2);

			CRenderingContext c(GameServer()->GetRenderer(), true);
			c.SetBlend(BLEND_ALPHA);

			PaintSheet(m_hActionTanksSheet, iTotalWidth-iTankWidth*3/2 + iTankOffset, iTotalHeight/2-iTankHeight/2, iTankWidth, iTankHeight, 0, 0, 512, 256, 512, 512, Color(255, 255, 255, (int)(flAlpha*255)));
			PaintSheet(m_hActionTanksSheet, iTankWidth/2 - iTankOffset, iTotalHeight/2-iTankHeight/2, iTankWidth, iTankHeight, 0, 256, 512, 256, 512, 512, Color(255, 255, 255, (int)(flAlpha*255)));

			size_t iSignWidth = (size_t)(iTotalWidth/2 * Bias(flWarpIn, 0.2f));
			size_t iSignHeight = (size_t)(iSignWidth*0.32f);
			if (m_eActionSign == ACTIONSIGN_FIGHT)
				PaintSheet(&m_ActionSignsSheet, "Fight", iTotalWidth/2-iSignWidth/2, iTotalHeight/2-iSignHeight/2, iSignWidth, iSignHeight, Color(255, 255, 255, (int)(flAlpha*255)));
			else if (m_eActionSign == ACTIONSIGN_SHOWDOWN)
				PaintSheet(&m_ActionSignsSheet, "Showdown", iTotalWidth/2-iSignWidth/2, iTotalHeight/2-iSignHeight/2, iSignWidth, iSignHeight, Color(255, 255, 255, (int)(flAlpha*255)));
			else if (m_eActionSign == ACTIONSIGN_NEWTURN)
				PaintSheet(&m_ActionSignsSheet, "NewTurn", iTotalWidth/2-iSignWidth/2, iTotalHeight/2-iSignHeight/2, iSignWidth, iSignHeight, Color(255, 255, 255, (int)(flAlpha*255)));
		}
	}

	if (m_pTeamInfo->GetText().length() > 0)
		CRootPanel::PaintRect(m_pTeamInfo->GetLeft()-3, m_pTeamInfo->GetTop()-9, m_pTeamInfo->GetWidth()+6, m_pTeamInfo->GetHeight()+6, Color(0, 0, 0, 220));

	m_pTeamInfo->SetVisible(true);
	m_pTeamInfo->Paint();

	if (m_iCurrentSmallActionItem < m_apActionItemButtons.size() && m_flSmallActionItemLerp > 0)
	{
		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);

		CPictureButton* pActionItem = m_apActionItemButtons[m_iCurrentSmallActionItem];

		int iLeft = pActionItem->GetLeft();
		int iTop = pActionItem->GetTop();

		int iTextureLeft = iLeft - 243;

		PaintHUDSheet("SmallActionItem", iTextureLeft - 10, iTop, 243, 40, Color(255, 255, 255, (int)(255*m_flSmallActionItemLerp)));

		c.SetColor(Color(255, 255, 255, (int)(255*m_flSmallActionItemLerp)));
		float flWidth = RootPanel()->GetTextWidth(m_sSmallActionItem, m_sSmallActionItem.length(), _T("text"), 13);
		float flHeight = RootPanel()->GetFontHeight(_T("text"), 13);
		CLabel::PaintText(m_sSmallActionItem, m_sSmallActionItem.length(), _T("text"), 13, (float)(iLeft - flWidth) - 25, iTop + flHeight + 5);
	}

	for (size_t i = 0; i < m_apActionItemButtons.size(); i++)
	{
		if (i >= m_apActionItemButtons.size() || i >= DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumActionItems())
			break;

		CPictureButton* pButton = m_apActionItemButtons[i];

		if (!pButton)
			continue;

		if (!DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetActionItem(i))
			continue;

		if (DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetActionItem(i)->bHandled)
		{
			CRenderingContext c(GameServer()->GetRenderer(), true);
			c.SetBlend(BLEND_ALPHA);
			PaintHUDSheet("CheckMark", pButton->GetLeft(), pButton->GetTop(), pButton->GetWidth(), pButton->GetHeight(), Color(255, 255, 255));
		}
	}

	if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
	{
		// Don't clear it in the start, we want dead tanks to remain in the list so we can mark them asploded.

		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);

		for (size_t i = 0; i < DigitanksGame()->GetNumPlayers(); i++)
		{
			const CDigitanksPlayer* pTeam = DigitanksGame()->GetDigitanksPlayer(i);
			if (!pTeam)
				continue;

			if (i >= m_ahScoreboardTanks.size())
				continue;

			tmap<size_t, CEntityHandle<CDigitank> >& ahTeamTanks = m_ahScoreboardTanks[i];

			Color clrTeam = pTeam->GetColor();
			if (DigitanksGame()->GetCurrentPlayer() == pTeam)
				clrTeam.SetAlpha((int)(255 * Oscillate(GameServer()->GetGameTime(), 1)));
			c.SetColor(clrTeam);

			if (DigitanksGame()->GetCurrentPlayer() == pTeam)
				CBaseControl::PaintRect(w - ahTeamTanks.size() * 30 - 20, 35 + i*40, ahTeamTanks.size()*30+10, 45, Color(0, 0, 0, 150));

			tstring sTeamName = pTeam->GetPlayerName();
			if (pTeam->IsHumanControlled())
				sTeamName = tstring(_T("[")) + sTeamName + _T("]");

			CLabel::PaintText(sTeamName, sTeamName.length(), _T("text"), 13, (float)w - RootPanel()->GetTextWidth(sTeamName, sTeamName.length(), _T("text"), 13) - 20, 50 - RootPanel()->GetFontHeight("text", 13) + (float)i * 40, clrTeam);

			int iTank = 0;
			for (tmap<size_t, CEntityHandle<CDigitank> >::iterator it = ahTeamTanks.begin(); it != ahTeamTanks.end(); it++)
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

	int iNotificationHeight = 40;
	int iFirstNotificationHeight = glgui::CRootPanel::Get()->GetHeight()/3 - 50;
	int iNotificationX = glgui::CRootPanel::Get()->GetWidth()/6;

	for (size_t i = 0; i < m_aPowerupNotifications.size(); i++)
	{
		powerup_notification_t* pNotification = &m_aPowerupNotifications[i];

		if (!pNotification->bActive)
			continue;

		if (!pNotification->hEntity || GameServer()->GetGameTime() - pNotification->flTime > 3)
		{
			pNotification->bActive = false;
			continue;
		}

		if (pNotification->hEntity->GetPlayerOwner() != DigitanksGame()->GetCurrentLocalDigitanksPlayer())
			continue;

		float flLerpIn = Bias(RemapValClamped(GameServer()->GetGameTime() - pNotification->flTime, 0, 0.5, 0.0, 1), 0.2f);
		int iX = (int)RemapValClamped(flLerpIn, 0, 1, -350, (float)iNotificationX);

		int iY = iFirstNotificationHeight - (iNotificationHeight+10)*i;

		Color clrBox = glgui::g_clrBox;
		clrBox.SetAlpha((float)RemapValClamped(GameServer()->GetGameTime() - pNotification->flTime, 1.5, 2, 1.0, 0));
		glgui::CRootPanel::PaintRect(iX, iY, 350, iNotificationHeight, clrBox);

		if (GameServer()->GetGameTime() - pNotification->flTime < 2)
		{
			tstring sText;
			if (pNotification->ePowerupType == POWERUP_TANK)
			{
				if (dynamic_cast<CDigitank*>(pNotification->hEntity.GetPointer()))
					sText = _T("Bonus Tank Received!");
				else
					sText = _T("Structure Rescued!");
			}
			else if (pNotification->ePowerupType == POWERUP_MISSILEDEFENSE)
				sText = _T("Missile Defense Received!");
			else if (pNotification->ePowerupType == POWERUP_AIRSTRIKE)
				sText = _T("Airstrike Received!");
			else
				sText = _T("Tank Upgrade Received!");

			CRenderingContext c(GameServer()->GetRenderer(), true);
			c.SetAlpha(RemapValClamped(GameServer()->GetGameTime() - pNotification->flTime, 1.5, 2, 1.0, 0));
			c.SetColor(Color(255, 255, 255, 255));
			glgui::CLabel::PaintText(sText, sText.length(), _T("header"), 18, (float)(iX + iNotificationHeight + 20), (float)(iY + iNotificationHeight - 15));
		}

		float iSceneX, iSceneY, iSceneW, iSceneH;
		m_pSceneTree->GetUnitDimensions(pNotification->hEntity, iSceneX, iSceneY, iSceneW, iSceneH);

		int iIconX, iIconY, iIconW, iIconH;

		int iSpacing = 3;
		float flMoveLerp = Gain(RemapValClamped(GameServer()->GetGameTime() - pNotification->flTime, 2.0, 2.5, 0.0, 1), 0.1f);
		iIconX = (int)RemapValClamped(flMoveLerp, 0, 1, (float)iX+iSpacing, (float)iSceneX);
		iIconY = (int)RemapValClamped(flMoveLerp, 0, 1, (float)iY+iSpacing, (float)iSceneY);
		iIconW = (int)RemapValClamped(flMoveLerp, 0, 1, (float)iNotificationHeight-iSpacing*2, (float)iSceneH);
		iIconH = (int)RemapValClamped(flMoveLerp, 0, 1, (float)iNotificationHeight-iSpacing*2, (float)iSceneH);

		tstring sArea;
		if (pNotification->ePowerupType == POWERUP_AIRSTRIKE)
			sArea = "Airstrike";
		else if (pNotification->ePowerupType == POWERUP_TANK)
		{
			if (pNotification->hEntity->GetUnitType() == STRUCTURE_CPU)
				sArea = "NewCPU";
			else if (pNotification->hEntity->GetUnitType() == UNIT_SCOUT)
				sArea = "NewRogue";
			else if (pNotification->hEntity->GetUnitType() == UNIT_INFANTRY)
				sArea = "NewResistor";
			else
				sArea = "NewDigitank";
		}
		else
			sArea = "Upgrade";

		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ADDITIVE);
		Color clrPowerup(255, 255, 255, 255);
		clrPowerup.SetAlpha((float)RemapValClamped(GameServer()->GetGameTime() - pNotification->flTime, 2.5, 3.0, 1.0, 0));
		PaintSheet(&m_PowerupsSheet, sArea, iIconX, iIconY, iIconW, iIconH, clrPowerup);
	}

	if (m_flSelectorMedalStart > 0 && GameServer()->GetGameTime() > m_flSelectorMedalStart)
	{
		double flMedalDisplayTime = 5;

		if (GameServer()->GetGameTime() > m_flSelectorMedalStart + flMedalDisplayTime)
			m_flSelectorMedalStart = 0;

		float flAlpha = RemapValClamped(GameServer()->GetGameTime(), m_flSelectorMedalStart, m_flSelectorMedalStart+0.5, 0.0, 1);
		if (GameServer()->GetGameTime() - m_flSelectorMedalStart > flMedalDisplayTime-1)
			flAlpha = RemapValClamped(GameServer()->GetGameTime(), m_flSelectorMedalStart+flMedalDisplayTime-1, m_flSelectorMedalStart+flMedalDisplayTime, 1.0, 0);

		Color clrMedal = Color(255, 255, 255, (int)(255*flAlpha));

		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);
		glgui::CBaseControl::PaintTexture(m_hSelectorMedalTexture, w/2-128, h/2-128, 256, 256, clrMedal);

		c.SetColor(clrMedal);

		tstring sMedal = _T("YOU RECEIVED A MEDAL");
		float flMedalWidth = glgui::RootPanel()->GetTextWidth(sMedal, sMedal.length(), _T("header"), 18);
		glgui::CLabel::PaintText(sMedal, sMedal.length(), _T("header"), 18, w/2-flMedalWidth/2, h/2 + (float)120);
	}

	if (m_flFileRescueStart > 0 && GameServer()->GetGameTime() > m_flFileRescueStart)
	{
		double flFileRescueTime = 5;

		if (GameServer()->GetGameTime() > m_flFileRescueStart + flFileRescueTime)
			m_flFileRescueStart = 0;

		float flAlpha = RemapValClamped(GameServer()->GetGameTime(), m_flFileRescueStart, m_flFileRescueStart+0.5, 0.0, 1);
		if (GameServer()->GetGameTime() - m_flFileRescueStart > flFileRescueTime-1)
			flAlpha = RemapValClamped(GameServer()->GetGameTime(), m_flFileRescueStart+flFileRescueTime-1, m_flFileRescueStart+flFileRescueTime, 1.0, 0);

		Color clrFile = Color(255, 255, 255, (int)(255*flAlpha));

		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);
		glgui::CBaseControl::PaintTexture(CMaterialLibrary::FindAsset(m_sFileRescueTexture), w/2-128, h/2-128, 256, 256, clrFile);

		c.SetColor(clrFile);

		tstring sText = _T("FILE RETRIEVED");
		float flTextWidth = glgui::RootPanel()->GetTextWidth(sText, sText.length(), _T("header"), 18);
		glgui::CLabel::PaintText(sText, sText.length(), _T("header"), 18, w/2-flTextWidth/2, h/2 + (float)150);
	}

	if (m_eMenuMode == MENUMODE_PROMOTE)
	{
		tstring sChooseHint = _T("Choose a promotion for this unit below.");

		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);
		c.SetColor(Color(255, 255, 255, (int)(255*RemapVal(Oscillate(GameServer()->GetGameTime(), 1), 0, 1, 0.5f, 1))));

		float flHintWidth = glgui::RootPanel()->GetTextWidth(sChooseHint, sChooseHint.length(), _T("text"), 9);
		glgui::CLabel::PaintText(sChooseHint, sChooseHint.length(), _T("text"), 9, (float)(m_pButtonPanel->GetLeft() + m_pButtonPanel->GetWidth()/2) - flHintWidth/2, (float)(m_pButtonPanel->GetTop() - 12));
	}

	CRenderingContext::DebugFinish();
}

void CHUD::PaintCameraGuidedMissile(float x, float y, float w, float h)
{
	CCameraGuidedMissile* pMissile = DigitanksGame()->GetOverheadCamera()->GetCameraGuidedMissile();

	if (!pMissile->GetOwner())
		return;

	Color clrWhite = Color(255, 255, 255, 120);

	CRenderingContext c(GameServer()->GetRenderer(), true);
	c.SetBlend(BLEND_ADDITIVE);
	c.SetColor(clrWhite);

	if (!pMissile->IsBoosting() && GameServer()->GetGameTime() - pMissile->GetSpawnTime() < 3)
	{
		tstring sLaunch = _T("LAUNCH IN");
		float flLaunchWidth = glgui::RootPanel()->GetTextWidth(sLaunch, sLaunch.length(), _T("cameramissile"), 40);
		glgui::CLabel::PaintText(sLaunch, sLaunch.length(), _T("cameramissile"), 40, w/2-flLaunchWidth/2, 150);

		sLaunch = tsprintf(tstring("%.1f"), 3 - (GameServer()->GetGameTime() - pMissile->GetSpawnTime()));
		flLaunchWidth = glgui::RootPanel()->GetTextWidth(sLaunch, sLaunch.length(), _T("cameramissile"), 40);
		glgui::CLabel::PaintText(sLaunch, sLaunch.length(), _T("cameramissile"), 40, w/2-flLaunchWidth/2, 200);
	}

	CDigitank* pOwner = dynamic_cast<CDigitank*>(pMissile->GetOwner());
	if (!pOwner)
		return;

	tstring sRange = tsprintf(tstring("RANGE %.2f"), (pOwner->GetLastAim() - pMissile->GetGlobalOrigin()).Length());
	glgui::CLabel::PaintText(sRange, sRange.length(), _T("cameramissile"), 20, 10, 100);

	Vector vecHit;
	CBaseEntity* pHit;
	bool bHit = DigitanksGame()->TraceLine(pOwner->GetLastAim() + Vector(0, 0, 1), pMissile->GetGlobalOrigin(), vecHit, &pHit, true);

	tstring sClear;
	if (bHit)
		sClear = _T("OBSTRUCTION");
	else
		sClear = _T("CLEAR");
	glgui::CLabel::PaintText(sClear, sClear.length(), _T("cameramissile"), 20, 10, 200);

	tstring sAltitude = tsprintf(tstring("ALT %.2f"), pMissile->GetGlobalOrigin().z - DigitanksGame()->GetTerrain()->GetHeight(pMissile->GetGlobalOrigin().x, pMissile->GetGlobalOrigin().y));
	glgui::CLabel::PaintText(sAltitude, sAltitude.length(), _T("cameramissile"), 20, 10, 300);

	tstring sFuel = tsprintf(tstring("FUEL %.2f"), 13 - (GameServer()->GetGameTime() - pMissile->GetSpawnTime()));
	glgui::CLabel::PaintText(sFuel, sFuel.length(), _T("cameramissile"), 20, 10, 400);

	if (pMissile->IsBoosting() && Oscillate(GameServer()->GetGameTime(), 0.3f) > 0.1f)
	{
		tstring sBoost = _T("BOOST");
		float flBoostWidth = glgui::RootPanel()->GetTextWidth(sBoost, sBoost.length(), _T("cameramissile"), 40);
		glgui::CLabel::PaintText(sBoost, sBoost.length(), _T("cameramissile"), 40, w/2-flBoostWidth/2, h - 200.0f);
	}

	int iBoxWidth = 300;

	glgui::CRootPanel::PaintRect(w/2-iBoxWidth/2, h/2-iBoxWidth/2, 1, iBoxWidth, clrWhite);
	glgui::CRootPanel::PaintRect(w/2-iBoxWidth/2, h/2-iBoxWidth/2, iBoxWidth, 1, clrWhite);
	glgui::CRootPanel::PaintRect(w/2+iBoxWidth/2, h/2-iBoxWidth/2, 1, iBoxWidth, clrWhite);
	glgui::CRootPanel::PaintRect(w/2-iBoxWidth/2, h/2+iBoxWidth/2, 300, 1, clrWhite);

	int iTrackingWidth = 500;

	glgui::CRootPanel::PaintRect(w/2-iTrackingWidth/2, h/2-iTrackingWidth/2-50, iTrackingWidth, 1, clrWhite);
	glgui::CRootPanel::PaintRect(w/2-iTrackingWidth/2, h/2+iTrackingWidth/2+50, iTrackingWidth, 1, clrWhite);

	EAngle angCamera = pMissile->GetGlobalAngles();
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

		Vector vecOrigin = pEntity->GetGlobalOrigin();

		Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(vecOrigin);

		float flRadius = pDTEntity->GetBoundingRadius();

		// Cull tanks behind the camera
		if (vecForward.Dot((vecOrigin-GameServer()->GetCameraManager()->GetCameraPosition()).Normalized()) < 0)
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
		if (pTank->GetPlayerOwner())
			clrTeam = pTank->GetPlayerOwner()->GetColor();
		clrTeam.SetAlpha(clrWhite.a());

		float flHole = 5;
		CRootPanel::PaintRect((int)(vecScreen.x - flWidth), (int)(vecScreen.y), (int)(flWidth - flHole), 1, clrTeam);
		CRootPanel::PaintRect((int)(vecScreen.x + flHole), (int)(vecScreen.y), (int)(flWidth - flHole), 1, clrTeam);
		CRootPanel::PaintRect((int)(vecScreen.x), (int)(vecScreen.y - flWidth), 1, (int)(flWidth - flHole), clrTeam);
		CRootPanel::PaintRect((int)(vecScreen.x), (int)(vecScreen.y + flHole), 1, (int)(flWidth - flHole), clrTeam);
	}

	if (Oscillate(GameServer()->GetGameTime(), 0.5f) > 0.3f)
	{
		if (vecForward.Dot((pOwner->GetLastAim()-GameServer()->GetCameraManager()->GetCameraPosition()).Normalized()) > 0)
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

		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);

		iY -= (int)(Bias(Oscillate(GameServer()->GetGameTime(), 0.5f), 0.2f) * 10);

		PaintSheet(&m_KeysSheet, "SpaceBar", iX+45, iY, 60, 60);

		m_pSpacebarHint->Paint();
	}
	else
	{
		m_pSpacebarHint->SetVisible(false);
	}
}

// Let's phase this one out in favor of the below.
void CHUD::PaintSheet(CMaterialHandle& hSheet, float x, float y, float w, float h, int sx, int sy, int sw, int sh, int tw, int th, const Color& c)
{
	glgui::CBaseControl::PaintSheet(hSheet, x, y, w, h, sx, sy, sw, sh, tw, th, c);
}

void CHUD::PaintSheet(const CTextureSheet* pSheet, const tstring& sArea, float x, float y, float w, float h, const Color& c)
{
	const Rect& rArea = pSheet->GetArea(sArea);
	glgui::CBaseControl::PaintSheet(pSheet->GetSheet(sArea), x, y, w, h, rArea.x, rArea.y, rArea.w, rArea.h, pSheet->GetSheetWidth(sArea), pSheet->GetSheetHeight(sArea), c);
}

void CHUD::PaintHUDSheet(const tstring& sArea, float x, float y, float w, float h, const Color& c)
{
	CHUD* pHUD = DigitanksWindow()->GetHUD();

	TAssert(pHUD);
	if (!pHUD)
		return;

	const Rect* pRect = &pHUD->m_HUDSheet.GetArea(sArea);
	CMaterialHandle hSheet = pHUD->m_HUDSheet.GetSheet(sArea);
	PaintSheet(hSheet, x, y, w, h, pRect->x, pRect->y, pRect->w, pRect->h, pHUD->m_HUDSheet.GetSheetWidth(sArea), pHUD->m_HUDSheet.GetSheetHeight(sArea), c);
}

const CTextureSheet& CHUD::GetHUDSheet()
{
	return DigitanksWindow()->GetHUD()->m_HUDSheet;
}

void CHUD::GetUnitSheet(unittype_t eUnit, CMaterialHandle& hSheet, int& sx, int& sy, int& sw, int& sh, int& tw, int& th)
{
	CTextureSheet* pUnits = &DigitanksWindow()->GetHUD()->m_UnitsSheet;

	tstring sArea;

	if (eUnit == UNIT_TANK)
		sArea = "Digitank";
	else if (eUnit == UNIT_SCOUT)
		sArea = "Rogue";
	else if (eUnit == UNIT_ARTILLERY)
		sArea = "Artillery";
	else if (eUnit == UNIT_INFANTRY)
		sArea = "Resistor";
	else if (eUnit == UNIT_MOBILECPU)
		sArea = "MobileCPU";
	else if (eUnit == UNIT_GRIDBUG)
		sArea = "GridBug";
	else if (eUnit == UNIT_BUGTURRET)
		sArea = "BugTurret";
	else if (eUnit == STRUCTURE_MINIBUFFER)
		sArea = "Buffer";
	else if (eUnit == STRUCTURE_BUFFER)
		sArea = "MacroBuffer";
	else if (eUnit == STRUCTURE_BATTERY)
		sArea = "Capacitor";
	else if (eUnit == STRUCTURE_PSU)
		sArea = "PSU";
	else if (eUnit == STRUCTURE_INFANTRYLOADER)
		sArea = "ResistorFactory";
	else if (eUnit == STRUCTURE_TANKLOADER)
		sArea = "DigitankFactory";
	else if (eUnit == STRUCTURE_ARTILLERYLOADER)
		sArea = "ArtilleryFactory";
	else if (eUnit == STRUCTURE_CPU)
		sArea = "CPU";
	else if (eUnit == STRUCTURE_ELECTRONODE)
		sArea = "Electronode";
	else if (eUnit == STRUCTURE_FIREWALL)
		sArea = "Firewall";

	Rect rUnit = pUnits->GetArea(sArea);
	hSheet = pUnits->GetSheet(sArea);
	tw = pUnits->GetSheetWidth(sArea);
	th = pUnits->GetSheetHeight(sArea);
	sx = rUnit.x;
	sy = rUnit.y;
	sw = rUnit.w;
	sh = rUnit.h;
}

void CHUD::PaintUnitSheet(unittype_t eUnit, float x, float y, float w, float h, const Color& c)
{
	CMaterialHandle hSheet;
	int sx, sy, sw, sh, tw, th;
	GetUnitSheet(eUnit, hSheet, sx, sy, sw, sh, tw, th);
	PaintSheet(hSheet, x, y, w, h, sx, sy, sw, sh, tw, th, c);
}

void CHUD::GetWeaponSheet(weapon_t eWeapon, CMaterialHandle& hSheet, int& sx, int& sy, int& sw, int& sh, int& tw, int& th)
{
	CTextureSheet* pWeapons = &DigitanksWindow()->GetHUD()->m_WeaponsSheet;

	tstring sArea;

	switch (eWeapon)
	{
	default:
	case WEAPON_NONE:
	case PROJECTILE_SMALL:
		sArea = "LittleBoy";
		break;

	case PROJECTILE_MEDIUM:
		sArea = "FatMan";
		break;

	case PROJECTILE_LARGE:
		sArea = "BigMama";
		break;

	case PROJECTILE_AOE:
	case PROJECTILE_ARTILLERY_AOE:
		sArea = "AOE";
		break;

	case PROJECTILE_EMP:
		sArea = "EMP";
		break;

	case PROJECTILE_ICBM:
	case PROJECTILE_ARTILLERY_ICBM:
		sArea = "ICBM";
		break;

	case PROJECTILE_GRENADE:
		sArea = "Grenade";
		break;

	case PROJECTILE_DAISYCHAIN:
		sArea = "DaisyChain";
		break;

	case PROJECTILE_CLUSTERBOMB:
		sArea = "ClusterBomb";
		break;

	case PROJECTILE_SPLOOGE:
		sArea = "Grapeshot";
		break;

	case PROJECTILE_TRACTORBOMB:
		sArea = "RepulsorBomb";
		break;

	case PROJECTILE_EARTHSHAKER:
		sArea = "Earthshaker";
		break;

	case PROJECTILE_CAMERAGUIDED:
		sArea = "CameraGuidedMissile";
		break;

	case WEAPON_LASER:
	case WEAPON_INFANTRYLASER:
		sArea = "FragmentationRay";
		break;

	case WEAPON_CHARGERAM:
		sArea = "ChargingRAM";
		break;

	case PROJECTILE_TREECUTTER:
		sArea = "TreeCutter";
		break;

	case PROJECTILE_FLAK:
		sArea = "MachineGun";
		break;

	case PROJECTILE_DEVASTATOR:
		sArea = "Devastator";
		break;

	case PROJECTILE_TORPEDO:
		sArea = "Torpedo";
		break;
	}

	Rect rUnit = pWeapons->GetArea(sArea);
	hSheet = pWeapons->GetSheet(sArea);
	tw = pWeapons->GetSheetWidth(sArea);
	th = pWeapons->GetSheetHeight(sArea);
	sx = rUnit.x;
	sy = rUnit.y;
	sw = rUnit.w;
	sh = rUnit.h;
}

void CHUD::PaintWeaponSheet(weapon_t eWeapon, float x, float y, float w, float h, const Color& c)
{
	CMaterialHandle hSheet;
	int sx, sy, sw, sh, tw, th;
	GetWeaponSheet(eWeapon, hSheet, sx, sy, sw, sh, tw, th);
	PaintSheet(hSheet, x, y, w, h, sx, sy, sw, sh, tw, th, c);
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

const CTextureSheet& CHUD::GetKeysSheet()
{
	return DigitanksWindow()->GetHUD()->m_KeysSheet;
}

CMaterialHandle CHUD::GetActionTanksSheet()
{
	return DigitanksWindow()->GetHUD()->m_hActionTanksSheet;
}

CMaterialHandle CHUD::GetPurchasePanel()
{
	return DigitanksWindow()->GetHUD()->m_hPurchasePanel;
}

CMaterialHandle CHUD::GetShieldTexture()
{
	return DigitanksWindow()->GetHUD()->m_hShieldTexture;
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

	Layout();
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
		tstring sInfo;
		pCurrentSelection->UpdateInfo(sInfo);
		m_pTankInfo->SetText(sInfo.c_str());
	}
	else
		m_pTankInfo->SetText(_T("No selection."));
}

void CHUD::UpdateTankInfo(CDigitank* pTank)
{
	m_pAttackInfo->SetText(_T(""));

	Vector vecOrigin;
	if (DigitanksGame()->GetControlMode() == MODE_MOVE && pTank->GetPreviewMoveTurnPower() <= pTank->GetRemainingMovementEnergy())
		vecOrigin = pTank->GetPreviewMove();
	else
		vecOrigin = pTank->GetGlobalOrigin();

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

		if (pTargetTank->GetPlayerOwner() == pTank->GetPlayerOwner())
			continue;

		Vector vecOrigin2D = pTargetTank->GetGlobalOrigin().Flattened();
		Vector vecAim2D = vecAim.Flattened();

		if ((vecOrigin2D - vecAim2D).LengthSqr() > flRadius*flRadius)
			continue;

		if (pTargetTank->GetVisibility() == 0)
			continue;

		if (!pClosestTarget)
		{
			pClosestTarget = pTargetTank;
			continue;
		}

		if ((pTargetTank->GetGlobalOrigin() - vecAim).LengthSqr() < (pClosestTarget->GetGlobalOrigin() - vecAim).LengthSqr())
			pClosestTarget = pTargetTank;
	}

	if (!pClosestTarget)
		return;

	vecAttack = vecOrigin - pClosestTarget->GetGlobalOrigin();

	Vector vecOrigin2D = pClosestTarget->GetGlobalOrigin().Flattened();
	Vector vecAim2D = vecAim.Flattened();

	float flTargetDistance = (vecAim2D - vecOrigin2D).Length();

	if (flTargetDistance > flRadius)
		return;

	float flShieldStrength = pClosestTarget->GetShieldValue();
	float flDamageBlocked = flShieldStrength * pClosestTarget->GetDefenseScale(true);
	float flAttackDamage = CDigitanksWeapon::GetWeaponDamage(pTank->GetCurrentWeapon());

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
	m_pAttackInfo->SetText(_T(""));
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

	CDigitanksPlayer* pTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();

	if (!pTeam)
		return;

	if (!pTeam->GetPrimaryCPU())
		return;

	tstring s1;
	s1 = tsprintf(tstring("%d +%.1f/turn\n"), (int)pTeam->GetPower(), pTeam->GetPowerPerTurn());
	m_pPowerInfo->SetText(s1);

	tstring s2;
	s2 = tsprintf(tstring("%d/%d\n"), pTeam->GetUsedFleetPoints(), pTeam->GetTotalFleetPoints());
	m_pFleetInfo->SetText(s2);

	if (pTeam->GetUpdateDownloading())
	{
		tstring s3;
		s3 = tsprintf(tstring("%d/%dmb +%.1fmb/turn\n"), (int)pTeam->GetUpdateDownloaded(), (int)pTeam->GetUpdateSize(), pTeam->GetBandwidth());
		m_pBandwidthInfo->SetText(s3);
	}
	else
		m_pBandwidthInfo->SetText(tsprintf(tstring("%dmb +%.1fmb/turn\n"), (int)pTeam->GetMegabytes(), pTeam->GetBandwidth()));
}

void CHUD::UpdateScoreboard()
{
	if (!DigitanksGame())
		return;

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
		return;

	tvector<const CDigitanksPlayer*> apSortedTeams;

	// Prob not the fastest sorting algorithm but it doesn't need to be.
	for (size_t i = 0; i < DigitanksGame()->GetNumPlayers(); i++)
	{
		const CDigitanksPlayer* pTeam = DigitanksGame()->GetDigitanksPlayer(i);

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

	tstring s;
	tstring p;
	for (size_t i = 0; i < apSortedTeams.size(); i++)
	{
		const CDigitanksPlayer* pTeam = apSortedTeams[i];

		if (DigitanksGame()->IsTeamControlledByMe(pTeam))
			s += _T("[");
		s += pTeam->GetPlayerName();
		if (DigitanksGame()->IsTeamControlledByMe(pTeam))
			s += _T("]");

		s += tsprintf(tstring(": %d\n"), pTeam->GetScore());
	}

	m_pScoreboard->SetText(_T("Score:\n \n"));
	m_pScoreboard->AppendText(s);

	m_pScoreboard->SetSize(100, 9999);
	m_pScoreboard->SetSize(m_pScoreboard->GetWidth(), (int)m_pScoreboard->GetTextHeight());

	int iWidth = RootPanel()->GetWidth();

	m_pScoreboard->SetPos(iWidth - m_pScoreboard->GetWidth() - 10, m_pAttackInfo->GetTop() - m_pScoreboard->GetHeight() - 20);
}

void CHUD::UpdateTurnButton()
{
	if (!IsVisible())
		return;

	bool bWasBlinking = m_bBlinkTurnButton;
	m_bBlinkTurnButton = false;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentPlayer())
		return;

	if (!DigitanksGame()->GetCurrentLocalDigitanksPlayer())
		return;

	if (!DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentPlayer()))
	{
		SetButtonSheetTexture(m_pTurnButton, &m_HUDSheet, "EnemyTurnButton");
		m_pPressEnter->SetVisible(true);
		return;
	}

	bool bTurnComplete = true;

	for (size_t i = 0; i < DigitanksGame()->GetCurrentPlayer()->GetNumUnits(); i++)
	{
		CDigitank* pTank = dynamic_cast<CDigitank*>(DigitanksGame()->GetCurrentPlayer()->GetUnit(i));

		if (pTank)
		{
			if (pTank->NeedsOrders())
			{
				bTurnComplete = false;
				break;
			}
		}
	}

	for (size_t i = 0; i < DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumActionItems(); i++)
	{
		const actionitem_t* pItem = DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetActionItem(i);

		if (!pItem->bHandled)
		{
			bTurnComplete = false;
			break;
		}
	}
	
	if (bTurnComplete)
	{
		if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY && !bWasBlinking && DigitanksGame()->GetTurn() == 0 && !DigitanksWindow()->GetInstructor()->GetCurrentPanel())
			DigitanksWindow()->GetInstructor()->DisplayLesson("artillery-endturn");

		m_bBlinkTurnButton = true;
		SetButtonSheetTexture(m_pTurnButton, &m_HUDSheet, "TurnCompleteButton");
		m_pPressEnter->SetVisible(true);
	}
	else
	{
		SetButtonSheetTexture(m_pTurnButton, &m_HUDSheet, "TurnButton");
		m_pPressEnter->SetVisible(false);
	}

	if (DigitanksGame()->GetCurrentPlayer() != DigitanksGame()->GetCurrentLocalDigitanksPlayer())
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
		m_apButtons[i]->SetTexture(CMaterialHandle());
		SetButtonInfo(i, _T(""));
		SetButtonTooltip(i, _T(""));
	}

	if (!IsActive() || !DigitanksGame()->GetPrimarySelection() || DigitanksGame()->GetPrimarySelection()->GetPlayerOwner() != DigitanksGame()->GetCurrentLocalDigitanksPlayer())
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
		m_apButtons[i]->SetAlpha(1.0f);
	}
	else
	{
		m_apButtons[i]->SetClickedListener(NULL, NULL);
		m_apButtons[i]->SetEnabled(false);
		m_apButtons[i]->SetAlpha(0.5f);
	}
}

void CHUD::SetButtonTexture(int i, const tstring& sArea)
{
	SetButtonSheetTexture(m_apButtons[i], &m_ButtonSheet, sArea);
}

void CHUD::SetButtonColor(int i, Color clrButton)
{
	m_apButtons[i]->SetButtonColor(clrButton);
}

void CHUD::SetButtonInfo(int iButton, const tstring& pszInfo)
{
	m_aszButtonInfos[iButton] = pszInfo;
}

void CHUD::SetButtonTooltip(int iButton, const tstring& sTip)
{
	m_apButtons[iButton]->SetTooltip(sTip);
}

void CHUD::ButtonCallback(int iButton)
{
	if (m_apButtons[iButton]->GetClickedListener())
		m_apButtons[iButton]->GetClickedListenerCallback()(m_apButtons[iButton]->GetClickedListener(), "");
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

	if (DigitanksGame()->GetCurrentPlayer() == DigitanksGame()->GetCurrentLocalDigitanksPlayer())
		m_pPressEnter->SetText("Press <ENTER> to end your turn...");
	else
		m_pPressEnter->SetText("Other players are taking their turns...");

	if (DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentPlayer()))
		m_flTurnInfoLerpGoal = 1;
	else
		m_flTurnInfoLerpGoal = 0;

	if (DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentPlayer()))
	{
		// Don't open the research window on the first turn, give the player a chance to see the game grid first.
		if (DigitanksGame()->GetTurn() >= 1 && DigitanksGame()->GetUpdateGrid() && !DigitanksGame()->GetCurrentPlayer()->GetUpdateDownloading())
		{
			bool bShouldOpen = false;
			for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
			{
				for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
				{
					if (DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y].m_eUpdateClass != UPDATECLASS_EMPTY && !DigitanksGame()->GetCurrentPlayer()->HasDownloadedUpdate(x, y))
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

		m_pDemoNotice->SetText("");

		// If we have local hotseat multiplayer, update for the new team.
		m_pSceneTree->BuildTree();

		if (DigitanksGame()->GetPlayer(0) == DigitanksGame()->GetCurrentPlayer() && DigitanksGame()->GetTurn() == 0)
		{
			// Don't show for the first team on the first turn, show fight instead.
		}
		else
			DigitanksWindow()->GetHUD()->ShowNewTurnSign();
	}

	UpdateScoreboard();

	if (DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentPlayer()))
	{
		bool bShow = true;

		// Don't show the action items until we have a CPU because we have the story and the tutorials to think about first.
		if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD && !DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU())
			bShow = false;

		if (m_pUpdatesPanel->IsVisible())
			bShow = false;

		if (bShow)
			ShowFirstActionItem();
	}

	if (GameNetwork()->IsConnected() && DigitanksGame()->IsTeamControlledByMe(DigitanksGame()->GetCurrentPlayer()))
		CSoundLibrary::PlaySound(NULL, _T("sound/lesson-learned.wav"));	// No time to make a new sound.

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
		Vector vecTarget = DigitanksGame()->GetPrimarySelection()->GetGlobalOrigin();
		EAngle angCamera = DigitanksGame()->GetOverheadCamera()->GetAngles();
		float flDistance = DigitanksGame()->GetOverheadCamera()->GetDistance();
		int iRotations = 0;
		while (iRotations++ < 4)
		{
			Vector vecCamera = AngleVector(angCamera) * flDistance + vecTarget;

			Vector vecHit;
			CBaseEntity* pHit = nullptr;
			DigitanksGame()->TraceLine(vecCamera, vecTarget, vecHit, &pHit);
			if (pHit == DigitanksGame()->GetPrimarySelection())
				break;

			angCamera.y += 90;
		}

		DigitanksGame()->GetOverheadCamera()->SetAngle(angCamera);
	}
}

void CHUD::ShowFirstActionItem()
{
	m_bAllActionItemsHandled = false;

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumActionItems())
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
		m_iCurrentActionItem = (m_iCurrentActionItem+1)%DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumActionItems();

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

		const actionitem_t* pItem = DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetActionItem(m_iCurrentActionItem);

		if (pItem->eActionType == ACTIONTYPE_DOWNLOADCOMPLETE && DigitanksGame()->GetCurrentPlayer()->GetUpdateDownloading())
		{
			DigitanksGame()->GetCurrentLocalDigitanksPlayer()->HandledActionItem(ACTIONTYPE_DOWNLOADCOMPLETE);
			UpdateTurnButton();
		}

		if (pItem->bHandled)
			continue;

		// Only perform this check with tanks. Structures always return false for NeedsOrders()
		CEntityHandle<CDigitank> hDigitank(pItem->iUnit);

		if (hDigitank != NULL && !hDigitank->NeedsOrders())
		{
			// If must have been handled already.
			DigitanksGame()->GetCurrentLocalDigitanksPlayer()->HandledActionItem(hDigitank);
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

	CDigitanksPlayer* pLocalPlayer = DigitanksGame()->GetCurrentLocalDigitanksPlayer();
	if (!pLocalPlayer)
		return;

	if (pSelectable)
	{
		for (size_t i = 0; i < pLocalPlayer->GetNumActionItems(); i++)
		{
			if (pLocalPlayer->GetActionItem(i)->iUnit == pSelectable->GetHandle())
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
	if (m_iCurrentActionItem >= 0 && m_iCurrentActionItem < pLocalPlayer->GetNumActionItems() && pLocalPlayer->GetActionItem(m_iCurrentActionItem)->iUnit == ~0)
		return;

	ShowActionItem(~0);
}

void CHUD::ShowActionItem(actiontype_t eActionItem)
{
	if (m_bAllActionItemsHandled)
		return;

	for (size_t i = 0; i < DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumActionItems(); i++)
	{
		if (DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetActionItem(i)->eActionType == eActionItem)
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

	if (iActionItem >= DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumActionItems())
	{
		m_pActionItem->SetText("");
		m_flActionItemsLerpGoal = 0;
		return;
	}

	const actionitem_t* pItem = DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetActionItem(iActionItem);

	// Some action items are handled just by looking at them.
	if (pItem->eActionType == ACTIONTYPE_WELCOME || pItem->eActionType == ACTIONTYPE_CONTROLS || pItem->eActionType == ACTIONTYPE_NEWSTRUCTURE ||
		pItem->eActionType == ACTIONTYPE_AUTOMOVEENEMY || pItem->eActionType == ACTIONTYPE_UPGRADE || pItem->eActionType == ACTIONTYPE_UNITREADY ||
		pItem->eActionType == ACTIONTYPE_FORTIFIEDENEMY)
	{
		DigitanksGame()->GetCurrentLocalDigitanksPlayer()->HandledActionItem(iActionItem);
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
		OpenUpdatesCallback("");
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

void CHUD::ShowTankSelectionMedal()
{
	m_flSelectorMedalStart = GameServer()->GetGameTime();

	CSoundLibrary::PlaySound(NULL, _T("sound/lesson-learned.wav"));
}

void CHUD::OnTakeShieldDamage(CDigitank* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() == NULL)
		return;

	if (pVictim && DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetVisibilityAtPoint(pVictim->GetGlobalOrigin()) < 0.1f)
		return;

	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, true);

	if (pVictim && !pVictim->IsAlive() && bDirectHit && flDamage > 0)
		new CHitIndicator(pVictim, _T("OVERKILL!"));
}

void CHUD::OnTakeDamage(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() == NULL)
		return;

	if (pVictim && DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetVisibilityAtPoint(pVictim->GetGlobalOrigin()) < 0.1f)
		return;

	// Cleans itself up.
	new CDamageIndicator(pVictim, flDamage, false);

	if ((pVictim && pVictim->IsAlive() || bKilled) && bDirectHit && flDamage > 0)
		new CHitIndicator(pVictim, _T("DIRECT HIT!"));
}

void CHUD::OnDisabled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() == NULL)
		return;

	if (pVictim && DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetVisibilityAtPoint(pVictim->GetGlobalOrigin()) < 0.1f)
		return;

	if (pVictim && pVictim->IsAlive())
	{
		if (dynamic_cast<CDigitank*>(pVictim))
			new CHitIndicator(pVictim, _T("DISABLED!"));
		else
			new CHitIndicator(pVictim, _T("DISCONNECTED!"));
	}
}

void CHUD::OnMiss(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	if (pVictim == pAttacker)
		return;

	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() == NULL)
		return;

	if (pVictim && DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetVisibilityAtPoint(pVictim->GetGlobalOrigin()) < 0.1f)
		return;

	if (pVictim && pVictim->IsAlive())
		new CHitIndicator(pVictim, _T("MISS!"));
}

void CHUD::OnCritical(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor)
{
	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);
	if (pProjectile && !pProjectile->SendsNotifications())
		return;

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() == NULL)
		return;

	if (pVictim && DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetVisibilityAtPoint(pVictim->GetGlobalOrigin()) < 0.1f)
		return;

	if (pVictim)
		new CHitIndicator(pVictim, _T("CRITICAL HIT!"));
}

void CHUD::OnAddUnitToTeam(CDigitanksPlayer* pTeam, CBaseEntity* pEntity)
{
	m_pSceneTree->OnAddUnitToTeam(pTeam, pEntity);
}

void CHUD::OnRemoveUnitFromTeam(CDigitanksPlayer* pTeam, CBaseEntity* pEntity)
{
	m_pSceneTree->OnRemoveUnitFromTeam(pTeam, pEntity);
}

void CHUD::TankSpeak(class CBaseEntity* pTank, const tstring& sSpeech)
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
	if (!GameServer())
		return false;

	if (GameServer()->IsLoading())
		return false;

	if (!DigitanksGame())
		return false;

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

	int iWidth = RootPanel()->GetWidth();
	int iHeight = RootPanel()->GetHeight();

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

void ActionSignCallback(CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	if (asTokens.size() < 2)
		return;

	if (asTokens[1] == _T("fight"))
		DigitanksWindow()->GetHUD()->ShowFightSign();
	else if (asTokens[1] == _T("showdown"))
		DigitanksWindow()->GetHUD()->ShowShowdownSign();
	else if (asTokens[1] == _T("newturn"))
		DigitanksWindow()->GetHUD()->ShowNewTurnSign();
}

CCommand actionsign("actionsign", ActionSignCallback);

void CHUD::ShowFightSign()
{
	m_eActionSign = ACTIONSIGN_FIGHT;
	m_flActionSignStart = GameServer()->GetGameTime();
	CSoundLibrary::PlaySound(NULL, _T("sound/actionsign.wav"));
}

void CHUD::ShowShowdownSign()
{
	m_eActionSign = ACTIONSIGN_SHOWDOWN;
	m_flActionSignStart = GameServer()->GetGameTime();
	CSoundLibrary::PlaySound(NULL, _T("sound/actionsign.wav"));
}

void CHUD::ShowNewTurnSign()
{
	m_eActionSign = ACTIONSIGN_NEWTURN;
	m_flActionSignStart = GameServer()->GetGameTime();
	CSoundLibrary::PlaySound(NULL, _T("sound/actionsign.wav"));
}

void PowerupNotifyCallback(CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	if (!DigitanksGame()->GetCurrentLocalDigitanksPlayer())
		return;

	DigitanksWindow()->GetHUD()->AddPowerupNotification(DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetTank(0), POWERUP_BONUS);
}

CCommand powerup_notify("powerup_notify", PowerupNotifyCallback);

void CHUD::AddPowerupNotification(const CDigitanksEntity* pEntity, powerup_type_t ePowerup)
{
	powerup_notification_t* pNewNotification = NULL;

	for (size_t i = 0; i < m_aPowerupNotifications.size(); i++)
	{
		powerup_notification_t* pNotification = &m_aPowerupNotifications[i];

		if (pNotification->bActive)
			continue;

		pNewNotification = pNotification;
		break;
	}

	if (!pNewNotification)
		pNewNotification = &m_aPowerupNotifications.push_back();

	pNewNotification->bActive = true;
	pNewNotification->ePowerupType = ePowerup;
	pNewNotification->flTime = GameServer()->GetGameTime();
	pNewNotification->hEntity = pEntity;
}

void CHUD::ShowFileRescue(const tstring& sTexture)
{
	m_sFileRescueTexture = sTexture;
	m_flFileRescueStart = GameServer()->GetGameTime();
	CSoundLibrary::PlaySound(NULL, _T("lesson-learned.wav"));	// No time to make a new sound.
}

void CHUD::CloseWeaponPanel()
{
	m_pWeaponPanel->SetVisible(false);
}

FRect CHUD::GetButtonDimensions(size_t i)
{
	FRect r;
	m_apButtons[i]->GetAbsDimensions(r.x, r.y, r.w, r.h);
	return r;
}

void CHUD::ChooseActionItemCallback(const tstring& sArgs)
{
	size_t iActionItem = ~0;

	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	// Bit of a hack since we don't know what button was pressed we have to look for it.
	for (size_t i = 0; i < m_apActionItemButtons.size(); i++)
	{
		float x, y, w, h;
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

	if (iActionItem < DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumActionItems())
	{
		const actionitem_t* pItem = DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetActionItem(iActionItem);

		CEntityHandle<CSelectable> hSelection(pItem->iUnit);

		if (hSelection != NULL)
		{
			DigitanksGame()->GetCurrentPlayer()->SetPrimarySelection(hSelection);
			DigitanksGame()->GetOverheadCamera()->SetTarget(hSelection->GetGlobalOrigin());
			if (DigitanksGame()->GetOverheadCamera()->GetDistance() > 250)
				DigitanksGame()->GetOverheadCamera()->SetDistance(250);
		}
	}

	m_flSmallActionItemLerpGoal = 0;
}

void CHUD::ShowSmallActionItemCallback(const tstring& sArgs)
{
	m_iCurrentSmallActionItem = ~0;
	m_flSmallActionItemLerpGoal = 0;

	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	// Bit of a hack since we don't know what button was pressed we have to look for it.
	for (size_t i = 0; i < m_apActionItemButtons.size(); i++)
	{
		float x, y, w, h;
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

	if (!DigitanksGame()->GetCurrentLocalDigitanksPlayer())
		return;

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetNumActionItems() == 0)
		return;

	const actionitem_t* pItem = DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetActionItem(m_iCurrentSmallActionItem);

	switch (pItem->eActionType)
	{
	case ACTIONTYPE_WELCOME:
		m_sSmallActionItem = _T("WELCOME TO DIGITANKS");
		break;

	case ACTIONTYPE_CONTROLS:
		m_sSmallActionItem = _T("QUICK CONTROLS");
		break;

	case ACTIONTYPE_NEWSTRUCTURE:
		m_sSmallActionItem = _T("STRUCTURE COMPLETED");
		break;

	case ACTIONTYPE_UNITORDERS:
		m_sSmallActionItem = _T("ORDERS NEEDED");
		break;

	case ACTIONTYPE_UNITAUTOMOVE:
		m_sSmallActionItem = _T("AUTO-MOVE COMPLETED");
		break;

	case ACTIONTYPE_AUTOMOVECANCELED:
		m_sSmallActionItem = _T("AUTO-MOVE INTERRUPTED");
		break;

	case ACTIONTYPE_AUTOMOVEENEMY:
		m_sSmallActionItem = _T("AUTO-MOVE THREAT");
		break;

	case ACTIONTYPE_UNITDAMAGED:
		m_sSmallActionItem = _T("UNIT DAMAGED");
		break;

	case ACTIONTYPE_FORTIFIEDENEMY:
		m_sSmallActionItem = _T("ENEMY IN RANGE");
		break;

	case ACTIONTYPE_UPGRADE:
		m_sSmallActionItem = _T("UPRGADE COMPLETED");
		break;

	case ACTIONTYPE_UNITREADY:
		m_sSmallActionItem = _T("UNIT COMPLETED");
		break;

	case ACTIONTYPE_DOWNLOADCOMPLETE:
		m_sSmallActionItem = _T("DOWNLOAD COMPLETED");
		break;

	case ACTIONTYPE_DOWNLOADUPDATES:
		m_sSmallActionItem = _T("DOWNLOAD UPDATES");
		break;
	}
}

void CHUD::HideSmallActionItemCallback(const tstring& sArgs)
{
	m_flSmallActionItemLerpGoal = 0;
}

void CHUD::CloseActionItemsCallback(const tstring& sArgs)
{
	ShowActionItem(~0);
}

void CHUD::CursorInTurnButtonCallback(const tstring& sArgs)
{
	if (!DigitanksGame()->GetCurrentPlayer())
		return;

	for (size_t i = 0; i < DigitanksGame()->GetCurrentPlayer()->GetNumTanks(); i++)
	{
		CDigitank* pTank = DigitanksGame()->GetCurrentPlayer()->GetTank(i);
		if (!pTank)
			continue;

		if (pTank->NeedsOrders())
		{
			m_flTurnWarningGoal = 1;
			return;
		}
	}

	m_flTurnWarningGoal = 0;
}

void CHUD::CursorOutTurnButtonCallback(const tstring& sArgs)
{
	m_flTurnWarningGoal = 0;
}

void CHUD::ButtonCursorIn0Callback(const tstring& sArgs)
{
	ShowButtonInfo(0);
}

void CHUD::ButtonCursorIn1Callback(const tstring& sArgs)
{
	ShowButtonInfo(1);
}

void CHUD::ButtonCursorIn2Callback(const tstring& sArgs)
{
	ShowButtonInfo(2);
}

void CHUD::ButtonCursorIn3Callback(const tstring& sArgs)
{
	ShowButtonInfo(3);
}

void CHUD::ButtonCursorIn4Callback(const tstring& sArgs)
{
	ShowButtonInfo(4);
}

void CHUD::ButtonCursorIn5Callback(const tstring& sArgs)
{
	ShowButtonInfo(5);
}

void CHUD::ButtonCursorIn6Callback(const tstring& sArgs)
{
	ShowButtonInfo(6);
}

void CHUD::ButtonCursorIn7Callback(const tstring& sArgs)
{
	ShowButtonInfo(7);
}

void CHUD::ButtonCursorIn8Callback(const tstring& sArgs)
{
	ShowButtonInfo(8);
}

void CHUD::ButtonCursorIn9Callback(const tstring& sArgs)
{
	ShowButtonInfo(9);
}

void CHUD::ButtonCursorOutCallback(const tstring& sArgs)
{
	HideButtonInfo();
}

void CHUD::EndTurnCallback(const tstring& sArgs)
{
	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer() != DigitanksGame()->GetCurrentPlayer())
		return;

	m_flActionItemsLerpGoal = 0;

	CSoundLibrary::PlaySound(NULL, _T("sound/turn.wav"));
	DigitanksGame()->EndTurn();
}

void CHUD::OpenUpdatesCallback(const tstring& sArgs)
{
	if (m_pUpdatesPanel)
		m_pUpdatesPanel->SetVisible(true);

	m_bUpdatesBlinking = false;

	DigitanksGame()->SetControlMode(MODE_NONE);
}

void CHUD::MoveCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		DigitanksGame()->SetControlMode(MODE_NONE);
	else
		DigitanksGame()->SetControlMode(MODE_MOVE);

	DigitanksWindow()->GetInstructor()->FinishedLesson("mission-1-move-mode");

	DigitanksWindow()->SetContextualCommandsOverride(true);

	SetupMenu();
}

void CHUD::CancelAutoMoveCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	DigitanksGame()->SetControlMode(MODE_NONE);

	for (size_t i = 0; i < DigitanksGame()->GetCurrentPlayer()->GetNumTanks(); i++)
	{
		if (DigitanksGame()->GetCurrentPlayer()->IsSelected(DigitanksGame()->GetCurrentPlayer()->GetTank(i)))
			DigitanksGame()->GetCurrentPlayer()->GetTank(i)->CancelGoalMovePosition();
	}

	SetupMenu();
}

void CHUD::TurnCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
		DigitanksGame()->SetControlMode(MODE_NONE);
	else
		DigitanksGame()->SetControlMode(MODE_TURN);

	DigitanksWindow()->SetContextualCommandsOverride(true);

	SetupMenu();
}

void CHUD::AimCallback(const tstring& sArgs)
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

	DigitanksWindow()->SetContextualCommandsOverride(true);

	SetupMenu();
}

void CHUD::FortifyCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame()->GetPrimarySelectionTank())
		return;

	DigitanksGame()->SetControlMode(MODE_NONE);

	for (size_t i = 0; i < DigitanksGame()->GetCurrentPlayer()->GetNumTanks(); i++)
	{
		if (DigitanksGame()->GetCurrentPlayer()->IsSelected(DigitanksGame()->GetCurrentPlayer()->GetTank(i)))
		{
			DigitanksGame()->GetCurrentPlayer()->GetTank(i)->Fortify();
			DigitanksGame()->GetCurrentLocalDigitanksPlayer()->HandledActionItem(DigitanksGame()->GetCurrentPlayer()->GetTank(i));
		}
	}

	SetupMenu(MENUMODE_MAIN);
	UpdateInfo();
}

void CHUD::SentryCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame()->GetPrimarySelectionTank())
		return;

	DigitanksGame()->SetControlMode(MODE_NONE);

	for (size_t i = 0; i < DigitanksGame()->GetCurrentPlayer()->GetNumTanks(); i++)
	{
		if (DigitanksGame()->GetCurrentPlayer()->IsSelected(DigitanksGame()->GetCurrentPlayer()->GetTank(i)))
		{
			DigitanksGame()->GetCurrentPlayer()->GetTank(i)->Sentry();
			DigitanksGame()->GetCurrentLocalDigitanksPlayer()->HandledActionItem(DigitanksGame()->GetCurrentPlayer()->GetTank(i));
		}
	}

	SetupMenu(MENUMODE_MAIN);
	UpdateInfo();
}

void CHUD::PromoteCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	SetupMenu(MENUMODE_PROMOTE);
}

void CHUD::PromoteAttackCallback(const tstring& sArgs)
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

//	DigitanksWindow()->GetInstructor()->FinishedLesson(CInstructor::TUTORIAL_UPGRADE);
}

void CHUD::PromoteDefenseCallback(const tstring& sArgs)
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

//	DigitanksWindow()->GetInstructor()->FinishedLesson(CInstructor::TUTORIAL_UPGRADE);
}

void CHUD::PromoteMovementCallback(const tstring& sArgs)
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

//	DigitanksWindow()->GetInstructor()->FinishedLesson(CInstructor::TUTORIAL_UPGRADE);
}

void CHUD::FireSpecialCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (DigitanksGame()->GetControlMode() == MODE_AIM)
		DigitanksGame()->SetControlMode(MODE_NONE);
	else if (DigitanksGame()->GetCurrentPlayer() && DigitanksGame()->GetCurrentPlayer()->GetPrimarySelectionTank())
	{
		DigitanksGame()->GetCurrentPlayer()->GetPrimarySelectionTank()->SetCurrentWeapon(PROJECTILE_AIRSTRIKE);
		DigitanksGame()->SetControlMode(MODE_AIM);
		DigitanksGame()->SetAimTypeByWeapon(PROJECTILE_AIRSTRIKE);
	}

	SetupMenu();
}

void CHUD::BuildMiniBufferCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_MINIBUFFER);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	DigitanksWindow()->GetInstructor()->FinishedLesson("strategy-buildbuffer", true);

	SetupMenu();
}

void CHUD::BuildBufferCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_BUFFER);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
}

void CHUD::BuildBatteryCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_BATTERY);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	DigitanksWindow()->GetInstructor()->FinishedLesson("strategy-buildbuffer", true);

	SetupMenu();
}

void CHUD::BuildPSUCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_PSU);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
}

void CHUD::BuildLoaderCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	SetupMenu(MENUMODE_LOADERS);
}

void CHUD::BuildInfantryLoaderCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_INFANTRYLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
}

void CHUD::BuildTankLoaderCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_TANKLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
}

void CHUD::BuildArtilleryLoaderCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_ARTILLERYLOADER);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
}

void CHUD::CancelBuildCallback(const tstring& sArgs)
{
	DigitanksGame()->SetControlMode(MODE_NONE);
	SetupMenu();
}

void CHUD::BuildUnitCallback(const tstring& sArgs)
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

void CHUD::BuildScoutCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->BeginRogueProduction();
	SetupMenu();
	UpdateInfo();
	UpdateTeamInfo();
}

void CHUD::BuildTurretCallback(const tstring& sArgs)
{
	if (!m_bHUDActive)
		return;

	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU())
		return;

	CCPU* pCPU = DigitanksGame()->GetCurrentPlayer()->GetPrimaryCPU();
	if (!pCPU)
		return;

	pCPU->SetPreviewStructure(STRUCTURE_FIREWALL);

	DigitanksGame()->SetControlMode(MODE_BUILD);

	SetupMenu();
}

void CHUD::BeginUpgradeCallback(const tstring& sArgs)
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

void CHUD::CloakCallback(const tstring& sArgs)
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

void CHUD::ChooseWeaponCallback(const tstring& sArgs)
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
		DigitanksWindow()->GetInstructor()->FinishedLesson("artillery-aim", true);
	}
}

void CHUD::GoToMainCallback(const tstring& sArgs)
{
	SetupMenu(MENUMODE_MAIN);
}

void CHUD::ShowPowerInfoCallback(const tstring& sArgs)
{
	m_pTeamInfo->SetText("");

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
		return;

	CDigitanksPlayer* pCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();
	if (!pCurrentTeam)
		return;

	m_pTeamInfo->AppendText("POWER INFO\n \n");

	m_pTeamInfo->AppendText(tsprintf(tstring("Power stored: %.1f\n"), pCurrentTeam->GetPower()));
	m_pTeamInfo->AppendText(tsprintf(tstring("Power per turn: +%.1f\n \n"), pCurrentTeam->GetPowerPerTurn()));

	for (size_t i = 0; i < pCurrentTeam->GetNumUnits(); i++)
	{
		const CBaseEntity* pEntity = pCurrentTeam->GetUnit(i);
		if (!pEntity)
			continue;

		const CStructure* pStructure = dynamic_cast<const CStructure*>(pEntity);
		if (!pStructure)
			continue;

		if (pStructure->Power() > 0)
			m_pTeamInfo->AppendText(tsprintf(pStructure->GetEntityName() + _T(": +%.1f\n"), pStructure->Power()));

		const CCollector* pCollector = dynamic_cast<const CCollector*>(pEntity);
		if (!pCollector)
			continue;

		if (pCollector->IsConstructing())
			continue;

		float flEfficiency;
		if (!pCollector->GetSupplier() || !pCollector->GetSupplyLine())
			flEfficiency = 0;
		else
			flEfficiency = pCollector->GetSupplier()->GetChildEfficiency() * pCollector->GetSupplyLine()->GetIntegrity();

		m_pTeamInfo->AppendText(tsprintf(pCollector->GetEntityName() + _T(": +%.1f (%d%%)\n"), pCollector->GetPowerProduced(), (int)(flEfficiency * 100)));
	}

	LayoutTeamInfo();
}

void CHUD::ShowFleetInfoCallback(const tstring& sArgs)
{
	m_pTeamInfo->SetText("");

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
		return;

	CDigitanksPlayer* pCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();
	if (!pCurrentTeam)
		return;

	m_pTeamInfo->AppendText("FLEET INFO\n \n");

	m_pTeamInfo->AppendText(tsprintf(tstring("Fleet points available: %d\n"), pCurrentTeam->GetTotalFleetPoints()));
	m_pTeamInfo->AppendText(tsprintf(tstring("Fleet points in use: %d\n \n"), pCurrentTeam->GetUsedFleetPoints()));

	int iScouts = 0;
	int iTanks = 0;
	int iArtillery = 0;
	int iInfantry = 0;

	m_pTeamInfo->AppendText(_T("Suppliers:\n"));
	for (size_t i = 0; i < pCurrentTeam->GetNumUnits(); i++)
	{
		const CBaseEntity* pEntity = pCurrentTeam->GetUnit(i);
		if (!pEntity)
			continue;

		const CDigitanksEntity* pDTEnt = dynamic_cast<const CDigitanksEntity*>(pEntity);
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

		const CStructure* pStructure = dynamic_cast<const CStructure*>(pEntity);
		if (!pStructure)
			continue;

		if (pStructure->IsConstructing())
			continue;

		if (pStructure->FleetPoints() > 0)
			m_pTeamInfo->AppendText(tsprintf(pStructure->GetEntityName() + _T(": +%d\n"), pStructure->FleetPoints()));
	}

	m_pTeamInfo->AppendText(_T(" \nFleet:\n"));
	if (iScouts)
		m_pTeamInfo->AppendText(tsprintf(tstring("%d Rogues (%d): %d\n"), iScouts, CScout::ScoutFleetPoints(), iScouts*CScout::ScoutFleetPoints()));
	if (iInfantry)
		m_pTeamInfo->AppendText(tsprintf(tstring("%d Resistors (%d): %d\n"), iInfantry, CMechInfantry::InfantryFleetPoints(), iInfantry*CMechInfantry::InfantryFleetPoints()));
	if (iTanks)
		m_pTeamInfo->AppendText(tsprintf(tstring("%d Digitanks (%d): %d\n"), iTanks, CMainBattleTank::MainTankFleetPoints(), iTanks*CMainBattleTank::MainTankFleetPoints()));
	if (iArtillery)
		m_pTeamInfo->AppendText(tsprintf(tstring("%d Artillery (%d): %d\n"), iArtillery, CArtillery::ArtilleryFleetPoints(), iArtillery*CArtillery::ArtilleryFleetPoints()));

	LayoutTeamInfo();
}

void CHUD::ShowBandwidthInfoCallback(const tstring& sArgs)
{
	m_pTeamInfo->SetText("");

	if (DigitanksGame()->GetGameType() != GAMETYPE_STANDARD)
		return;

	CDigitanksPlayer* pCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();
	if (!pCurrentTeam)
		return;

	m_pTeamInfo->AppendText("BANDWIDTH INFO\n \n");

	if (pCurrentTeam->GetUpdateDownloading())
	{
		m_pTeamInfo->AppendText(tsprintf(tstring("Downloading: %s\n"), pCurrentTeam->GetUpdateDownloading()->GetName().c_str()));
		m_pTeamInfo->AppendText(tsprintf(tstring("Progress: %.1f/%dmb (%d turns)\n"), pCurrentTeam->GetUpdateDownloaded(), (int)pCurrentTeam->GetUpdateSize(), pCurrentTeam->GetTurnsToDownload()));
		m_pTeamInfo->AppendText(tsprintf(tstring("Download rate: %.1fmb/turn\n \n"), pCurrentTeam->GetBandwidth()));
	}

	for (size_t i = 0; i < pCurrentTeam->GetNumUnits(); i++)
	{
		const CBaseEntity* pEntity = pCurrentTeam->GetUnit(i);
		if (!pEntity)
			continue;

		const CStructure* pStructure = dynamic_cast<const CStructure*>(pEntity);
		if (!pStructure)
			continue;

		if (pStructure->IsConstructing())
			continue;

		if (pStructure->Bandwidth() > 0)
			m_pTeamInfo->AppendText(tsprintf(pStructure->GetEntityName() + _T(": +%.1fmb\n"), pStructure->Bandwidth()));
	}

	LayoutTeamInfo();
}

void CHUD::HideTeamInfoCallback(const tstring& sArgs)
{
	m_pTeamInfo->SetText("");
}

void CHUD::FireTurretCallback(const tstring& sArgs)
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
	: CLabel(0, 0, 100, 100, _T(""))
{
	m_hVictim = pVictim;
	m_flDamage = flDamage;
	m_bShield = bShield;
	m_flTime = GameServer()->GetGameTime();

	if (pVictim)
		m_vecLastOrigin = pVictim->GetGlobalOrigin();

	glgui::CRootPanel::Get()->AddControl(this, true);

	Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(m_vecLastOrigin);
	if (bShield)
		vecScreen += Vector(10, 10, 0);
	SetPos((int)vecScreen.x, (int)vecScreen.y);

	if (m_bShield)
		SetTextColor(Color(255, 255, 255));
	else if (flDamage < 0)
		SetTextColor(Color(0, 255, 0));
	else
		SetTextColor(Color(255, 0, 0));

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
		sprintf(szDamage, "+%d %s", -iDamage, bShield?"shield":"hull");
	else
		sprintf(szDamage, "-%d", iDamage);
	SetText(szDamage);

	SetFont(_T("header"), 18);
	SetAlign(CLabel::TA_TOPLEFT);
	SetWrap(false);
}

void CDamageIndicator::Think()
{
	double flFadeTime = 1.0f;

	if (GameServer()->GetGameTime() - m_flTime > flFadeTime)
	{
		GetParent().DowncastStatic<CPanel>()->RemoveControl(this);
		return;
	}

	if (m_hVictim != NULL)
		m_vecLastOrigin = m_hVictim->GetGlobalOrigin();

	float flOffset = RemapVal(GameServer()->GetGameTime() - m_flTime, 0, flFadeTime, 10.0, 20);

	Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(m_vecLastOrigin);
	if (m_bShield)
		vecScreen += Vector(10, 10, 0);

	SetPos((int)(vecScreen.x+flOffset), (int)(vecScreen.y-flOffset));

	SetAlpha((float)RemapVal(GameServer()->GetGameTime() - m_flTime, 0, flFadeTime, 255.0, 0));

	// Cull tanks behind the camera
	if (GameServer()->GetRenderer()->GetCameraVector().Dot((m_vecLastOrigin-GameServer()->GetCameraManager()->GetCameraPosition()).Normalized()) < 0)
		SetAlpha(0);

	BaseClass::Think();
}

void CDamageIndicator::Paint(float x, float y, float w, float h)
{
	if (m_bShield)
	{
		float iWidth = GetTextWidth();
		float iHeight = GetTextHeight();
		CRootPanel::PaintRect(x, y-iHeight/2, iWidth, iHeight, Color(0, 0, 0, GetAlpha()/2));
	}

	BaseClass::Paint(x, y, w, h);
}

CHitIndicator::CHitIndicator(CBaseEntity* pVictim, tstring sMessage)
	: CLabel(0, 0, 200, 100, _T(""))
{
	m_hVictim = pVictim;
	m_flTime = GameServer()->GetGameTime();
	m_vecLastOrigin = pVictim->GetGlobalOrigin();

	glgui::CRootPanel::Get()->AddControl(this, true);

	Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(pVictim->GetGlobalOrigin());
	vecScreen.x -= 20;
	vecScreen.y -= 20;
	SetPos((int)vecScreen.x, (int)vecScreen.y);

	SetTextColor(Color(255, 255, 255));

	SetText(sMessage.c_str());
	SetWrap(false);

	SetFont(_T("header"), 18);
	SetAlign(CLabel::TA_TOPLEFT);
}

void CHitIndicator::Think()
{
	double flFadeTime = 1.0f;

	if (GameServer()->GetGameTime() - m_flTime > flFadeTime)
	{
		GetParent().DowncastStatic<CPanel>()->RemoveControl(this);
		return;
	}

	if (m_hVictim != NULL)
		m_vecLastOrigin = m_hVictim->GetGlobalOrigin();

	float flOffset = RemapVal(GameServer()->GetGameTime() - m_flTime, 0, flFadeTime, 10.0, 20);

	Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(m_vecLastOrigin);
	vecScreen.x -= 20;
	vecScreen.y -= 20;

	SetPos((int)(vecScreen.x+flOffset), (int)(vecScreen.y-flOffset));

	SetAlpha((int)RemapVal(GameServer()->GetGameTime() - m_flTime, 0, flFadeTime, 255.0, 0));

	// Cull tanks behind the camera
	if (GameServer()->GetRenderer()->GetCameraVector().Dot((m_vecLastOrigin-GameServer()->GetCameraManager()->GetCameraPosition()).Normalized()) < 0)
		SetAlpha(0);

	BaseClass::Think();
}

void CHitIndicator::Paint(float x, float y, float w, float h)
{
	float iWidth = GetTextWidth();
	float iHeight = GetTextHeight();
	CRootPanel::PaintRect(x, y-iHeight/2, iWidth, iHeight, Color(0, 0, 0, GetAlpha()/2));

	BaseClass::Paint(x, y, w, h);
}

CSpeechBubble::CSpeechBubble(CBaseEntity* pSpeaker, tstring sSpeech)
	: CLabel(0, 0, 83*2/3, 47*2/3, _T(""))
{
	m_hSpeaker = pSpeaker;
	m_flTime = GameServer()->GetGameTime();
	m_vecLastOrigin = pSpeaker->GetGlobalOrigin();

	if (pSpeaker)
		m_flRadius = pSpeaker->GetBoundingRadius();
	else
		m_flRadius = 10;

	glgui::CRootPanel::Get()->AddControl(this, (DigitanksGame()->GetGameType() == GAMETYPE_MENU)?false:true);

	SetTextColor(Color(255, 255, 255));

	SetText(sSpeech.c_str());
	SetWrap(false);

	SetFont(_T("smileys"), 18);
	SetAlign(CLabel::TA_MIDDLECENTER);
}

void CSpeechBubble::Think()
{
	double flFadeTime = 3.0f;

	if (!GameServer())
		return;

	if (GameServer()->GetGameTime() - m_flTime > flFadeTime)
	{
		GetParent().DowncastStatic<CPanel>()->RemoveControl(this);
		return;
	}

	if (m_hSpeaker != NULL)
		m_vecLastOrigin = m_hSpeaker->GetGlobalOrigin();

	Vector vecUp;
	GameServer()->GetRenderer()->GetCameraVectors(NULL, NULL, &vecUp);

	Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(m_vecLastOrigin);
	Vector vecTop = GameServer()->GetRenderer()->ScreenPosition(m_vecLastOrigin + vecUp*m_flRadius);
	float flWidth = (vecTop - vecScreen).Length()*2 + 10;

	vecScreen.x -= (flWidth/2 + 50);
	vecScreen.y -= flWidth;

	SetPos((int)(vecScreen.x), (int)(vecScreen.y));

	SetAlpha((int)RemapValClamped(GameServer()->GetGameTime() - m_flTime, flFadeTime-1, flFadeTime, 255.0, 0));

	// Cull tanks behind the camera
	if (GameServer()->GetRenderer()->GetCameraVector().Dot((m_vecLastOrigin-GameServer()->GetCameraManager()->GetCameraPosition()).Normalized()) < 0)
		SetAlpha(0);

	BaseClass::Think();
}

void CSpeechBubble::Paint(float x, float y, float w, float h)
{
	if (!GameServer())
		return;

	if (DigitanksWindow()->GetHUD()->IsUpdatesPanelOpen())
		return;

	do {
		CRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBlend(BLEND_ALPHA);
		CHUD::PaintHUDSheet("SpeechBubble", x, y, w, h, Color(255, 255, 255, GetAlpha()));
	} while (false);

	BaseClass::Paint(x, y, w, h);
}

CHowToPlayPanel::CHowToPlayPanel()
	: CPanel(0, 0, 100, 100)
{
	m_pOpen = AddControl(new CLabel(0, 0, 100, 100, _T(""), _T("header"), 18));

	m_bOpen = false;

	m_flGoalLerp = 0;
	m_flCurLerp = 0;

	m_pControls = AddControl(new CLabel(0, 0, 100, 100, _T(""), _T("text"), 13));
}

void CHowToPlayPanel::Layout()
{
	BaseClass::Layout();

	if (IsOpen())
		m_pOpen->SetText(_T("Click to close"));
	else
		m_pOpen->SetText(_T("How to play"));

	m_pOpen->SetSize(150, 50);
	m_pOpen->SetPos(0, 300);
	m_pOpen->SetAlign(CLabel::TA_MIDDLECENTER);

	m_pControls->SetSize(330, 250);
	m_pControls->SetPos(10, 30);
	m_pControls->SetAlign(CLabel::TA_TOPLEFT);

	tstring sTips;

	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		sTips = _T("TIPS:\n* Structures can only be built on your Network.\n* To create more Network, build Buffers.\n* To harvest more resources, build a Buffer near an Electronode and then build a Capacitor on top of the Electronode.\n* Use Resistors and Firewalls to defend your base and destroy the enemy CPUs to win.\n \n");
	else
		sTips = _T("TIPS:\n* Tanks have a limited amount of movement energy per turn.\n* Each tank can fire once per turn.\n* When all tanks have moved or fired, press the 'End Turn' button to regain movement energy and attacks.\n \n");

	m_pControls->SetText(
		tstring(_T("OBJECTIVE: ")) + DigitanksGame()->GetObjective() + _T("\n \n") + sTips +
		_T("CONTROLS:\n")
		_T("Scroll view: Hold spacebar\n")
		_T("Rotate view: Hold right click\n")
		_T("Zoom view: Scrollwheel or pgup/pgdn\n")
		_T("Select similar units: Double click\n")
	);
}

void CHowToPlayPanel::Think()
{
	BaseClass::Think();

	SetVisible(!DigitanksGame()->IsFeatureDisabled(DISABLE_HOWTOPLAY));

	m_flCurLerp = Approach(m_flGoalLerp, m_flCurLerp, GameServer()->GetFrameTime()*2);

//	m_pUpdatesButton->SetPos(iWidth/2 - 617/2 - 35, 0);

	int iX = 150;

	// Keep it to the left of the downloads button. Don't want to cover that up.
	if (iX > (GetParent()->GetWidth()/2 - 617/2 - 35) - 150 - 10)
		iX = (GetParent()->GetWidth()/2 - 617/2 - 35) - 150 - 10;

	SetPos(iX, (int)(RemapVal(Bias(m_flCurLerp, 0.6f), 0, 1, -300, 0)));
	SetSize((int)(RemapVal(Bias(m_flCurLerp, 0.6f), 0, 1, 150, 350)), 350);

	m_pOpen->SetSize((int)(RemapVal(Bias(m_flCurLerp, 0.6f), 0, 1, 150, 300)), 50);
	m_pControls->SetAlpha(m_flCurLerp);
}

void CHowToPlayPanel::Paint(float x, float y, float w, float h)
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

	if (DigitanksGame()->GetCurrentLocalDigitanksPlayer()->IsMouseDragging())
		return false;

	return true;
}
