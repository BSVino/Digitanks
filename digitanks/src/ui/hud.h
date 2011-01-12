#ifndef DT_HUD_H
#define DT_HUD_H

#include "glgui/glgui.h"
#include <common.h>
#include "digitanks/digitanksgame.h"
#include "updatespanel.h"

#define NUM_BUTTONS 10

typedef enum
{
	POWERBAR_HEALTH,
	POWERBAR_ATTACK,
	POWERBAR_DEFENSE,
	POWERBAR_MOVEMENT,
} powerbar_type_t;

class CPowerBar : public glgui::CLabel
{
	DECLARE_CLASS(CPowerBar, glgui::CLabel);

public:
								CPowerBar(powerbar_type_t ePowerbarType);

public:
	void						Think();
	void						Paint(int x, int y, int w, int h);

protected:
	powerbar_type_t				m_ePowerbarType;
};

class CDamageIndicator : public glgui::CLabel
{
	DECLARE_CLASS(CDamageIndicator, glgui::CLabel);

public:
								CDamageIndicator(CBaseEntity* pVictim, float flDamage, bool bShield);

public:
	virtual void				Destructor();
	virtual void				Delete() { delete this; };

public:
	void						Think();
	void						Paint(int x, int y, int w, int h);

protected:
	CEntityHandle<CBaseEntity>	m_hVictim;
	float						m_flDamage;
	bool						m_bShield;
	float						m_flTime;
	Vector						m_vecLastOrigin;
};

class CHitIndicator : public glgui::CLabel
{
	DECLARE_CLASS(CHitIndicator, glgui::CLabel);

public:
								CHitIndicator(CBaseEntity* pVictim, eastl::string16 sMessage);

public:
	virtual void				Destructor();
	virtual void				Delete() { delete this; };

public:
	void						Think();
	void						Paint(int x, int y, int w, int h);

protected:
	CEntityHandle<CBaseEntity>	m_hVictim;
	float						m_flTime;
	Vector						m_vecLastOrigin;
};

class CSpeechBubble : public glgui::CLabel
{
	DECLARE_CLASS(CSpeechBubble, glgui::CLabel);

public:
								CSpeechBubble(CBaseEntity* pSpeaker, eastl::string sSpeech);

public:
	virtual void				Destructor();
	virtual void				Delete() { delete this; };

public:
	void						Think();
	void						Paint(int x, int y, int w, int h);

protected:
	CEntityHandle<CBaseEntity>	m_hSpeaker;
	float						m_flTime;
	Vector						m_vecLastOrigin;
	float						m_flRadius;
};

class CMouseCapturePanel : public glgui::CPanel
{
	DECLARE_CLASS(CMouseCapturePanel, glgui::CPanel);

public:
	CMouseCapturePanel() : CPanel(0, 0, 0, 0) {};

	virtual bool			MousePressed(int code, int mx, int my) { BaseClass::MousePressed(code, mx, my); return true; }
	virtual bool			MouseReleased(int code, int mx, int my) { BaseClass::MouseReleased(code, mx, my); return true; }
};

class CHUD : public glgui::CPanel, public IDigitanksGameListener, public glgui::IEventListener
{
	DECLARE_CLASS(CHUD, glgui::CPanel);

public:
								CHUD();

public:
	virtual void				Layout();
	virtual void				Think();

	void						Paint(int x, int y, int w, int h);

	static void					PaintSheet(size_t iTexture, int x, int y, int w, int h, int sx, int sy, int sw, int sh, int tw, int th, const Color& c = Color(255,255,255));
	static void					PaintHUDSheet(int x, int y, int w, int h, int sx, int sy, int sw, int sh, const Color& c = Color(255,255,255));

	void						ClientEnterGame();

	void						UpdateInfo();
	void						UpdateTankInfo(CDigitank* pTank);
	void						UpdateStructureInfo(CStructure* pStructure);
	void						UpdateTeamInfo();
	void						UpdateScoreboard();
	void						UpdateTurnButton();

	void						SetGame(class CDigitanksGame* pGame);

	void						SetupMenu();
	void						SetupMenu(menumode_t eMenuMode);

	void						SetButtonListener(int iButton, IEventListener::Callback pfnCallback);
	void						SetButtonTexture(int iButton, size_t iTexture);
	void						SetButtonColor(int iButton, Color clrButton);
	void						SetButtonInfo(int iButton, const eastl::string16& pszInfo);
	void						ButtonCallback(int iButton);

	virtual void				GameStart();
	virtual void				GameOver(bool bPlayerWon);

	virtual void				NewCurrentTeam();
	virtual void				NewCurrentSelection();

	void						ShowFirstActionItem();
	void						ShowNextActionItem();
	void						ShowActionItem(CSelectable* pSelectable);
	void						ShowActionItem(size_t iActionItem);

	virtual void				OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly);
	virtual void				OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled);

	virtual void				TankSpeak(class CBaseEntity* pTank, const eastl::string& sSpeech);

	virtual void				ClearTurnInfo();
	virtual void				AppendTurnInfo(const eastl::string16& pszInfo);

	virtual void				SetHUDActive(bool bActive);

	bool						IsActive() { return m_bHUDActive; };

	virtual bool				IsVisible();

	void						ShowButtonInfo(int iButton);
	void						HideButtonInfo();

	bool						IsUpdatesPanelOpen();

	EVENT_CALLBACK(CHUD, NextActionItem);

	EVENT_CALLBACK(CHUD, ButtonCursorIn0);
	EVENT_CALLBACK(CHUD, ButtonCursorIn1);
	EVENT_CALLBACK(CHUD, ButtonCursorIn2);
	EVENT_CALLBACK(CHUD, ButtonCursorIn3);
	EVENT_CALLBACK(CHUD, ButtonCursorIn4);
	EVENT_CALLBACK(CHUD, ButtonCursorIn5);
	EVENT_CALLBACK(CHUD, ButtonCursorIn6);
	EVENT_CALLBACK(CHUD, ButtonCursorIn7);
	EVENT_CALLBACK(CHUD, ButtonCursorIn8);
	EVENT_CALLBACK(CHUD, ButtonCursorIn9);
	EVENT_CALLBACK(CHUD, ButtonCursorOut);

	EVENT_CALLBACK(CHUD, EndTurn);
	EVENT_CALLBACK(CHUD, OpenUpdates);
	EVENT_CALLBACK(CHUD, Auto);
	EVENT_CALLBACK(CHUD, Move);
	EVENT_CALLBACK(CHUD, CancelAutoMove);
	EVENT_CALLBACK(CHUD, Turn);
	EVENT_CALLBACK(CHUD, Aim);
	EVENT_CALLBACK(CHUD, Fortify);
	EVENT_CALLBACK(CHUD, Charge);
	EVENT_CALLBACK(CHUD, Promote);
	EVENT_CALLBACK(CHUD, PromoteAttack);
	EVENT_CALLBACK(CHUD, PromoteDefense);
	EVENT_CALLBACK(CHUD, PromoteMovement);
	EVENT_CALLBACK(CHUD, FireSpecial);
	EVENT_CALLBACK(CHUD, BuildMiniBuffer);
	EVENT_CALLBACK(CHUD, BuildBuffer);
	EVENT_CALLBACK(CHUD, BuildBattery);
	EVENT_CALLBACK(CHUD, BuildPSU);
	EVENT_CALLBACK(CHUD, BuildLoader);
	EVENT_CALLBACK(CHUD, BuildTankLoader);
	EVENT_CALLBACK(CHUD, BuildInfantryLoader);
	EVENT_CALLBACK(CHUD, BuildArtilleryLoader);
	EVENT_CALLBACK(CHUD, CancelBuild);
	EVENT_CALLBACK(CHUD, BuildUnit);
	EVENT_CALLBACK(CHUD, CancelBuildUnit);
	EVENT_CALLBACK(CHUD, BuildScout);
	EVENT_CALLBACK(CHUD, CancelBuildScout);
	EVENT_CALLBACK(CHUD, InstallMenu);
	EVENT_CALLBACK(CHUD, InstallProduction);
	EVENT_CALLBACK(CHUD, InstallBandwidth);
	EVENT_CALLBACK(CHUD, InstallFleetSupply);
	EVENT_CALLBACK(CHUD, InstallEnergyBonus);
	EVENT_CALLBACK(CHUD, InstallRechargeBonus);
	EVENT_CALLBACK(CHUD, InstallTankAttack);
	EVENT_CALLBACK(CHUD, InstallTankDefense);
	EVENT_CALLBACK(CHUD, InstallTankMovement);
	EVENT_CALLBACK(CHUD, InstallTankHealth);
	EVENT_CALLBACK(CHUD, InstallTankRange);
	EVENT_CALLBACK(CHUD, CancelInstall);
	EVENT_CALLBACK(CHUD, BeginUpgrade);
	EVENT_CALLBACK(CHUD, CancelUpgrade);
	EVENT_CALLBACK(CHUD, ChooseWeapon);
	EVENT_CALLBACK(CHUD, ChooseWeapon0);
	EVENT_CALLBACK(CHUD, ChooseWeapon1);
	EVENT_CALLBACK(CHUD, ChooseWeapon2);
	EVENT_CALLBACK(CHUD, ChooseWeapon3);
	EVENT_CALLBACK(CHUD, ChooseWeapon4);
	EVENT_CALLBACK(CHUD, ChooseWeapon5);
	EVENT_CALLBACK(CHUD, ChooseWeapon6);
	EVENT_CALLBACK(CHUD, ChooseWeapon7);
	EVENT_CALLBACK(CHUD, ChooseWeapon8);
	EVENT_CALLBACK(CHUD, GoToMain);

	static void					SetNeedsUpdate();

protected:
	CPowerBar*					m_pHealthBar;
	CPowerBar*					m_pAttackPower;
	CPowerBar*					m_pDefensePower;
	CPowerBar*					m_pMovementPower;

	menumode_t					m_eMenuMode;

	glgui::CLabel*				m_pActionItem;
	glgui::CButton*				m_pNextActionItem;
	size_t						m_iCurrentActionItem;
	bool						m_bAllActionItemsHandled;

	CMouseCapturePanel*			m_pButtonPanel;

	glgui::CPictureButton*		m_apButtons[10];

	glgui::CLabel*				m_pFireAttack;
	glgui::CLabel*				m_pFireDefend;

	glgui::CLabel*				m_pAttackInfo;
	float						m_flAttackInfoAlpha;
	float						m_flAttackInfoAlphaGoal;

	glgui::CLabel*				m_pFrontShieldInfo;
	glgui::CLabel*				m_pRearShieldInfo;
	glgui::CLabel*				m_pLeftShieldInfo;
	glgui::CLabel*				m_pRightShieldInfo;

	glgui::CLabel*				m_pTankInfo;
	size_t						m_iTankInfoPanel;

	glgui::CLabel*				m_pTurnInfo;
	float						m_flTurnInfoHeight;
	float						m_flTurnInfoHeightGoal;
	float						m_flTurnInfoLerp;
	float						m_flTurnInfoLerpGoal;

	glgui::CLabel*				m_pResearchInfo;

	glgui::CLabel*				m_pButtonInfo;
	eastl::string16				m_aszButtonInfos[NUM_BUTTONS];

	glgui::CLabel*				m_pPressEnter;

	bool						m_bHUDActive;

	bool						m_bNeedsUpdate;

	glgui::CLabel*				m_pDemoNotice;

	glgui::CLabel*				m_pPowerInfo;
	glgui::CLabel*				m_pFleetInfo;
	glgui::CLabel*				m_pBandwidthInfo;

	glgui::CPictureButton*		m_pUpdatesButton;
	CUpdatesPanel*				m_pUpdatesPanel;
	bool						m_bUpdatesBlinking;

	glgui::CPictureButton*		m_pTurnButton;

	glgui::CLabel*				m_pScoreboard;

	class CWeaponPanel*			m_pWeaponPanel;

	size_t						m_iHUDSheet;

	size_t						m_iTurnSound;

	size_t						m_iKeysSheet;

	CEntityHandle<CBaseWeapon>	m_hHintWeapon;
	glgui::CLabel*				m_pSpacebarHint;

	size_t						m_iObstruction;

	size_t						m_iCompetitionWatermark;
};

#endif