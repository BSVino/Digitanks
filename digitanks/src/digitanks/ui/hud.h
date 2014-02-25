#ifndef DT_HUD_H
#define DT_HUD_H

#include <common.h>

#include <glgui/picturebutton.h>
#include <textures/texturesheet.h>
#include <textures/texturelibrary.h>

#include "digitanksgame.h"
#include "updatespanel.h"
#include <powerup.h>
#include "weaponpanel.h"
#include "scenetree.h"

#define NUM_BUTTONS 10

inline void SetButtonSheetTexture(class glgui::CPictureButton* pButton, CTextureSheet* pSheet, const tstring& sArea)
{
	pButton->SetSheetTexture(pSheet->GetSheet(sArea), pSheet->GetArea(sArea), pSheet->GetSheetWidth(sArea), pSheet->GetSheetHeight(sArea));
}

typedef enum
{
	POWERBAR_SHIELD,
	POWERBAR_HEALTH,
	POWERBAR_ATTACK,
	POWERBAR_DEFENSE,
	POWERBAR_MOVEMENT,
} powerbar_type_t;

typedef struct
{
	powerup_type_t				ePowerupType;
	CEntityHandle<CDigitanksEntity>	hEntity;
	double						flTime;
	bool						bActive;
} powerup_notification_t;

class CPowerBar : public glgui::CLabel
{
	DECLARE_CLASS(CPowerBar, glgui::CLabel);

public:
								CPowerBar(powerbar_type_t ePowerbarType);

public:
	void						Think();
	void						Paint(float x, float y, float w, float h);

protected:
	powerbar_type_t				m_ePowerbarType;
};

class CDamageIndicator : public glgui::CLabel
{
	DECLARE_CLASS(CDamageIndicator, glgui::CLabel);

public:
								CDamageIndicator(CBaseEntity* pVictim, float flDamage, bool bShield);

public:
	void						Think();
	void						Paint(float x, float y, float w, float h);

protected:
	CEntityHandle<CBaseEntity>	m_hVictim;
	float						m_flDamage;
	bool						m_bShield;
	double						m_flTime;
	Vector						m_vecLastOrigin;
};

class CHitIndicator : public glgui::CLabel
{
	DECLARE_CLASS(CHitIndicator, glgui::CLabel);

public:
								CHitIndicator(CBaseEntity* pVictim, tstring sMessage);

public:
	void						Think();
	void						Paint(float x, float y, float w, float h);

protected:
	CEntityHandle<CBaseEntity>	m_hVictim;
	double						m_flTime;
	Vector						m_vecLastOrigin;
};

class CSpeechBubble : public glgui::CLabel
{
	DECLARE_CLASS(CSpeechBubble, glgui::CLabel);

public:
								CSpeechBubble(CBaseEntity* pSpeaker, tstring sSpeech);

public:
	void						Think();
	void						Paint(float x, float y, float w, float h);

protected:
	CEntityHandle<CBaseEntity>	m_hSpeaker;
	double						m_flTime;
	Vector						m_vecLastOrigin;
	float						m_flRadius;
};

class CMouseCapturePanel : public glgui::CPanel
{
	DECLARE_CLASS(CMouseCapturePanel, glgui::CPanel);

public:
	CMouseCapturePanel() : CPanel(0, 0, 0, 0) {};

	virtual bool			MousePressed(int code, int mx, int my);
	virtual bool			MouseReleased(int code, int mx, int my);
};

class CHowToPlayPanel : public glgui::CPanel
{
	DECLARE_CLASS(CHowToPlayPanel, glgui::CPanel);

public:
							CHowToPlayPanel();

public:
	virtual void			Layout();
	virtual void			Think();

	virtual void			Paint(float x, float y, float w, float h);

	virtual bool			IsOpen();
	virtual void			Open();
	virtual void			Close();

	virtual bool			MousePressed(int code, int mx, int my);
	virtual bool			MouseReleased(int code, int mx, int my);

protected:
	glgui::CControl<glgui::CLabel> m_pOpen;

	bool					m_bOpen;

	float					m_flGoalLerp;
	float					m_flCurLerp;

	glgui::CControl<glgui::CLabel> m_pControls;
};

class CHUD : public glgui::CPanel, public IDigitanksGameListener, public glgui::IEventListener
{
	DECLARE_CLASS(CHUD, glgui::CPanel);

public:
								CHUD();

public:
	virtual void				Layout();
	virtual void				Think();

	void						Paint(float x, float y, float w, float h);

	void						PaintCameraGuidedMissile(float x, float y, float w, float h);

	class CUpdatesPanel*		GetUpdatesPanel() { return m_pUpdatesPanel; }

	static void					PaintSheet(CMaterialHandle& hSheet, float x, float y, float w, float h, int sx, int sy, int sw, int sh, int tw, int th, const Color& c = Color(255,255,255));
	static void					PaintSheet(const CTextureSheet* pSheet, const tstring& sArea, float x, float y, float w, float h, const Color& c = Color(255,255,255));
	static void					PaintHUDSheet(const tstring& sArea, float x, float y, float w, float h, const Color& c = Color(255,255,255));
	static const CTextureSheet&	GetHUDSheet();
	static void					GetUnitSheet(unittype_t eUnit, CMaterialHandle& hSheet, int& sx, int& sy, int& sw, int& sh, int& tw, int& th);
	static void					PaintUnitSheet(unittype_t eUnit, float x, float y, float w, float h, const Color& c = Color(255,255,255));
	static void					GetWeaponSheet(weapon_t eWeapon, CMaterialHandle& hSheet, int& sx, int& sy, int& sw, int& sh, int& tw, int& th);
	static void					PaintWeaponSheet(weapon_t eWeapon, float x, float y, float w, float h, const Color& c = Color(255,255,255));
	static const CTextureSheet&	GetWeaponSheet();
	static const CTextureSheet&	GetButtonSheet();
	static const CTextureSheet&	GetDownloadSheet();
	static const CTextureSheet&	GetKeysSheet();
	static CMaterialHandle      GetActionTanksSheet();
	static CMaterialHandle      GetPurchasePanel();
	static CMaterialHandle      GetShieldTexture();

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
	void						SetButtonTexture(int iButton, const tstring& sArea);
	void						SetButtonColor(int iButton, Color clrButton);
	void						SetButtonInfo(int iButton, const tstring& pszInfo);
	void						SetButtonTooltip(int iButton, const tstring& sTooltip);
	void						ButtonCallback(int iButton);

	virtual void				GameStart();
	virtual void				GameOver(bool bPlayerWon);

	virtual void				NewCurrentTeam();
	virtual void				NewCurrentSelection();

	void						ShowFirstActionItem();
	void						ShowNextActionItem();
	void						ShowActionItem(CSelectable* pSelectable);
	void						ShowActionItem(actiontype_t eActionItem);
	void						ShowActionItem(size_t iActionItem);
	void						OnAddNewActionItem();

	void						ShowTankSelectionMedal();

	virtual void				OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly);
	virtual void				OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled);
	virtual void				OnDisabled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor);
	virtual void				OnMiss(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor);
	virtual void				OnCritical(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor);

	virtual void				OnAddUnitToTeam(class CDigitanksPlayer* pTeam, class CBaseEntity* pEntity);
	virtual void				OnRemoveUnitFromTeam(class CDigitanksPlayer* pTeam, class CBaseEntity* pEntity);

	virtual void				TankSpeak(class CBaseEntity* pTank, const tstring& sSpeech);

	virtual void				ClearTurnInfo();

	virtual void				SetHUDActive(bool bActive);

	bool						IsActive() { return m_bHUDActive; };

	virtual bool				IsVisible();

	void						ShowButtonInfo(int iButton);
	void						HideButtonInfo();
	bool						IsButtonInfoVisible();

	bool						IsUpdatesPanelOpen();
	void						SlideUpdateIcon(int x, int y);

	void						ShowFightSign();
	void						ShowShowdownSign();
	void						ShowNewTurnSign();

	void						AddPowerupNotification(const CDigitanksEntity* pEntity, powerup_type_t ePowerup);

	void						ShowFileRescue(const tstring& sTexture);

	void						ClearHintWeapon() { m_hHintWeapon = NULL; };

	void						CloseWeaponPanel();
	class CWeaponPanel*			GetWeaponPanel() { return m_pWeaponPanel; };

	class CSceneTree*			GetSceneTree() { return m_pSceneTree; };

	FRect						GetButtonDimensions(size_t i);

	EVENT_CALLBACK(CHUD, ChooseActionItem);
	EVENT_CALLBACK(CHUD, ShowSmallActionItem);
	EVENT_CALLBACK(CHUD, HideSmallActionItem);
	EVENT_CALLBACK(CHUD, CloseActionItems);

	EVENT_CALLBACK(CHUD, CursorInTurnButton);
	EVENT_CALLBACK(CHUD, CursorOutTurnButton);

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
	EVENT_CALLBACK(CHUD, Sentry);
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
	EVENT_CALLBACK(CHUD, BuildScout);
	EVENT_CALLBACK(CHUD, BuildTurret);
	EVENT_CALLBACK(CHUD, BeginUpgrade);
	EVENT_CALLBACK(CHUD, Cloak);
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
	EVENT_CALLBACK(CHUD, ShowPowerInfo);
	EVENT_CALLBACK(CHUD, ShowFleetInfo);
	EVENT_CALLBACK(CHUD, ShowBandwidthInfo);
	EVENT_CALLBACK(CHUD, HideTeamInfo);
	EVENT_CALLBACK(CHUD, FireTurret);

	void						LayoutTeamInfo();

	static void					SetNeedsUpdate();
	static void					SetTeamMembersUpdated();

protected:
	glgui::CControl<CPowerBar> m_pShieldBar;
	glgui::CControl<CPowerBar> m_pHealthBar;
	glgui::CControl<CPowerBar> m_pAttackPower;
	glgui::CControl<CPowerBar> m_pDefensePower;
	glgui::CControl<CPowerBar> m_pMovementPower;

	menumode_t					m_eMenuMode;

	glgui::CControl<glgui::CLabel> m_pActionItem;
	tvector<glgui::CPictureButton*> m_apActionItemButtons;
	glgui::CControl<glgui::CButton> m_pCloseActionItems;
	size_t						m_iCurrentActionItem;
	bool						m_bAllActionItemsHandled;
	float						m_flActionItemsLerp;
	float						m_flActionItemsLerpGoal;
	float						m_flActionItemsWidth;
	float						m_flSmallActionItemLerp;
	float						m_flSmallActionItemLerpGoal;
	size_t						m_iCurrentSmallActionItem;
	tstring				m_sSmallActionItem;

	double						m_flSelectorMedalStart;
	CMaterialHandle             m_hSelectorMedalTexture;

	tvector<tmap<size_t, CEntityHandle<CDigitank> > > m_ahScoreboardTanks;

	glgui::CControl<CMouseCapturePanel> m_pButtonPanel;

	glgui::CControl<CHowToPlayPanel> m_pHowToPlayPanel;

	glgui::CControl<glgui::CPictureButton> m_apButtons[10];

	glgui::CControl<glgui::CLabel> m_pAttackInfo;
	float						m_flAttackInfoAlpha;
	float						m_flAttackInfoAlphaGoal;

	glgui::CControl<glgui::CLabel> m_pTankInfo;
	size_t						m_iTankInfoPanel;

	glgui::CControl<glgui::CLabel> m_pTurnInfo;
	float						m_flTurnInfoHeight;
	float						m_flTurnInfoHeightGoal;
	float						m_flTurnInfoLerp;
	float						m_flTurnInfoLerpGoal;

	glgui::CControl<glgui::CLabel> m_pResearchInfo;

	glgui::CControl<glgui::CLabel> m_pButtonInfo;
	tstring				m_aszButtonInfos[NUM_BUTTONS];

	glgui::CControl<glgui::CLabel> m_pPressEnter;

	bool						m_bHUDActive;

	bool						m_bNeedsUpdate;

	glgui::CControl<glgui::CLabel> m_pDemoNotice;

	glgui::CControl<glgui::CLabel> m_pPowerInfo;
	glgui::CControl<glgui::CLabel> m_pFleetInfo;
	glgui::CControl<glgui::CLabel> m_pBandwidthInfo;
	glgui::CControl<glgui::CLabel> m_pTeamInfo;

	glgui::CControl<glgui::CPictureButton> m_pUpdatesButton;
	glgui::CControl<CUpdatesPanel> m_pUpdatesPanel;
	bool						m_bUpdatesBlinking;
	float						m_flUpdateIconSlide;
	int							m_iUpdateIconSlideStartX;
	int							m_iUpdateIconSlideStartY;

	glgui::CControl<glgui::CPictureButton> m_pTurnButton;
	bool						m_bBlinkTurnButton;

	glgui::CControl<glgui::CLabel> m_pTurnWarning;
	float						m_flTurnWarningGoal;
	float						m_flTurnWarningLerp;

	glgui::CControl<glgui::CLabel> m_pScoreboard;

	glgui::CControl<CWeaponPanel> m_pWeaponPanel;
	glgui::CControl<CSceneTree> m_pSceneTree;

	CTextureSheet				m_HUDSheet;
	CTextureSheet				m_UnitsSheet;
	CTextureSheet				m_WeaponsSheet;
	CTextureSheet				m_ButtonSheet;
	CTextureSheet				m_DownloadSheet;
	CMaterialHandle             m_hShieldTexture;

	size_t						m_iTurnSound;

	CTextureSheet				m_KeysSheet;

	CEntityHandle<CBaseWeapon>	m_hHintWeapon;
	glgui::CControl<glgui::CLabel> m_pSpacebarHint;

	CMaterialHandle             m_hActionTanksSheet;
	CTextureSheet				m_ActionSignsSheet;
	double						m_flActionSignStart;
	typedef enum
	{
		ACTIONSIGN_NONE = 0,
		ACTIONSIGN_FIGHT,
		ACTIONSIGN_SHOWDOWN,
		ACTIONSIGN_NEWTURN,
	} actionsign_t;
	actionsign_t				m_eActionSign;

	tstring				m_sFileRescueTexture;
	double						m_flFileRescueStart;

	CTextureSheet				m_PowerupsSheet;
	tvector<powerup_notification_t>	m_aPowerupNotifications;

	CMaterialHandle             m_hPurchasePanel;

	size_t						m_iCompetitionWatermark;
};

#endif
