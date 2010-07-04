#ifndef DT_HUD_H
#define DT_HUD_H

#include "glgui/glgui.h"
#include <common.h>
#include "digitanks/digitanksgame.h"

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
								CHitIndicator(CBaseEntity* pVictim, std::wstring sMessage);

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
								CSpeechBubble(CDigitank* pSpeaker, std::string sSpeech, size_t iBubble);

public:
	virtual void				Destructor();
	virtual void				Delete() { delete this; };

public:
	void						Think();
	void						Paint(int x, int y, int w, int h);

protected:
	CEntityHandle<CDigitank>	m_hSpeaker;
	float						m_flTime;
	Vector						m_vecLastOrigin;
	size_t						m_iBubble;
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

	void						UpdateInfo();
	void						UpdateTankInfo(CDigitank* pTank);

	void						SetGame(class CDigitanksGame* pGame);

	void						SetupMenu();
	void						SetupMenu(menumode_t eMenuMode);

	void						SetButton1Listener(IEventListener::Callback pfnCallback);
	void						SetButton2Listener(IEventListener::Callback pfnCallback);
	void						SetButton3Listener(IEventListener::Callback pfnCallback);
	void						SetButton4Listener(IEventListener::Callback pfnCallback);
	void						SetButton5Listener(IEventListener::Callback pfnCallback);

	void						SetButton1Help(const char* pszHelp);
	void						SetButton2Help(const char* pszHelp);
	void						SetButton3Help(const char* pszHelp);
	void						SetButton4Help(const char* pszHelp);
	void						SetButton5Help(const char* pszHelp);

	void						SetButton1Texture(size_t iTexture);
	void						SetButton2Texture(size_t iTexture);
	void						SetButton3Texture(size_t iTexture);
	void						SetButton4Texture(size_t iTexture);
	void						SetButton5Texture(size_t iTexture);

	void						SetButton1Color(Color clrButton);
	void						SetButton2Color(Color clrButton);
	void						SetButton3Color(Color clrButton);
	void						SetButton4Color(Color clrButton);
	void						SetButton5Color(Color clrButton);

	virtual void				GameStart();
	virtual void				GameOver(bool bPlayerWon);

	virtual void				NewCurrentTeam();
	virtual void				NewCurrentSelection();

	virtual void				OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly);
	virtual void				OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled);

	virtual void				TankSpeak(class CDigitank* pTank, const std::string& sSpeech);

	virtual void				SetHUDActive(bool bActive);

	bool						ShouldAutoProceed() { return m_bAutoProceed; };
	void						SetAutoProceed(bool bAuto);

	bool						IsActive() { return m_bHUDActive; };

	EVENT_CALLBACK(CHUD, Auto);
	EVENT_CALLBACK(CHUD, Move);
	EVENT_CALLBACK(CHUD, Turn);
	EVENT_CALLBACK(CHUD, Aim);
	EVENT_CALLBACK(CHUD, Fire);
	EVENT_CALLBACK(CHUD, Fortify);
	EVENT_CALLBACK(CHUD, Promote);
	EVENT_CALLBACK(CHUD, PromoteAttack);
	EVENT_CALLBACK(CHUD, PromoteDefense);
	EVENT_CALLBACK(CHUD, PromoteMovement);
	EVENT_CALLBACK(CHUD, BuildBuffer);
	EVENT_CALLBACK(CHUD, BuildPSU);
	EVENT_CALLBACK(CHUD, BuildTankLoader);
	EVENT_CALLBACK(CHUD, BuildInfantryLoader);
	EVENT_CALLBACK(CHUD, CancelBuild);
	EVENT_CALLBACK(CHUD, BuildUnit);
	EVENT_CALLBACK(CHUD, CancelBuildUnit);
	EVENT_CALLBACK(CHUD, GoToMain);

	size_t						GetSpeechBubble() { return m_iSpeechBubble; };

protected:
	class CDigitanksGame*		m_pGame;

	CPowerBar*					m_pHealthBar;
	CPowerBar*					m_pAttackPower;
	CPowerBar*					m_pDefensePower;
	CPowerBar*					m_pMovementPower;

	menumode_t					m_eMenuMode;

	glgui::CButton*				m_pAutoButton;

	glgui::CPictureButton*		m_pButton1;
	glgui::CPictureButton*		m_pButton2;
	glgui::CPictureButton*		m_pButton3;
	glgui::CPictureButton*		m_pButton4;
	glgui::CPictureButton*		m_pButton5;

	glgui::CLabel*				m_pButtonHelp1;
	glgui::CLabel*				m_pButtonHelp2;
	glgui::CLabel*				m_pButtonHelp3;
	glgui::CLabel*				m_pButtonHelp4;
	glgui::CLabel*				m_pButtonHelp5;

	glgui::CLabel*				m_pFireAttack;
	glgui::CLabel*				m_pFireDefend;

	glgui::CLabel*				m_pAttackInfo;

	glgui::CLabel*				m_pLowShieldsWarning;

	glgui::CLabel*				m_pFrontShieldInfo;
	glgui::CLabel*				m_pRearShieldInfo;
	glgui::CLabel*				m_pLeftShieldInfo;
	glgui::CLabel*				m_pRightShieldInfo;

	glgui::CLabel*				m_pTankInfo;

	glgui::CLabel*				m_pPressEnter;

	bool						m_bAutoProceed;
	bool						m_bHUDActive;

	glgui::CLabel*				m_pOpenTutorial;

	glgui::CLabel*				m_pFPS;

	glgui::CLabel*				m_pTeamInfo;

	size_t						m_iAvatarIcon;
	size_t						m_iShieldIcon;

	size_t						m_iSpeechBubble;

//	size_t						m_iCompetitionWatermark;
};

#endif