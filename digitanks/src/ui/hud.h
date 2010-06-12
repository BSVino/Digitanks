#ifndef DT_HUD_H
#define DT_HUD_H

#include "glgui/glgui.h"
#include <common.h>
#include "game/digitanksgame.h"

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

class CHUD : public glgui::CPanel, public IDigitanksGameListener, public glgui::IEventListener
{
	DECLARE_CLASS(CHUD, glgui::CPanel);

	typedef enum
	{
		MENUMODE_MAIN,
		MENUMODE_PROMOTE,
	} menumode_t;

public:
								CHUD();

public:
	virtual void				Layout();
	virtual void				Think();

	void						Paint(int x, int y, int w, int h);

	void						UpdateAttackInfo();

	void						SetGame(class CDigitanksGame* pGame);

	void						SetupMenu(menumode_t eMenuMode);

	virtual void				GameStart();
	virtual void				GameOver(bool bPlayerWon);

	virtual void				NewCurrentTeam();
	virtual void				NewCurrentTank();

	virtual void				OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly);
	virtual void				OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled);

	virtual void				SetHUDActive(bool bActive);

	bool						ShouldAutoProceed() { return m_bAutoProceed; };
	void						SetAutoProceed(bool bAuto) { m_bAutoProceed = bAuto; };

	EVENT_CALLBACK(CHUD, Auto);
	EVENT_CALLBACK(CHUD, Move);
	EVENT_CALLBACK(CHUD, Turn);
	EVENT_CALLBACK(CHUD, Aim);
	EVENT_CALLBACK(CHUD, Fire);
	EVENT_CALLBACK(CHUD, Promote);
	EVENT_CALLBACK(CHUD, PromoteAttack);
	EVENT_CALLBACK(CHUD, PromoteDefense);
	EVENT_CALLBACK(CHUD, PromoteMovement);
	EVENT_CALLBACK(CHUD, GoToMain);

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

	size_t						m_iAvatarIcon;
	size_t						m_iShieldIcon;

	size_t						m_iCancelIcon;
	size_t						m_iMoveIcon;
	size_t						m_iTurnIcon;
	size_t						m_iAimIcon;
	size_t						m_iFireIcon;
	size_t						m_iPromoteIcon;

	size_t						m_iCompetitionWatermark;
};

#endif