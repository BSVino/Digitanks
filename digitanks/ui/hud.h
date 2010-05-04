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
	virtual void				GameOver();

	virtual void				NewCurrentTeam();
	virtual void				NewCurrentTank();

	EVENT_CALLBACK(CHUD, Move);
	EVENT_CALLBACK(CHUD, Turn);
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

	glgui::CButton*				m_pButton1;
	glgui::CButton*				m_pButton2;
	glgui::CButton*				m_pButton3;
	glgui::CButton*				m_pButton4;

	glgui::CLabel*				m_pAttackInfo;

	glgui::CLabel*				m_pFrontShieldInfo;
	glgui::CLabel*				m_pRearShieldInfo;
	glgui::CLabel*				m_pLeftShieldInfo;
	glgui::CLabel*				m_pRightShieldInfo;

	glgui::CLabel*				m_pTankInfo;
};

#endif