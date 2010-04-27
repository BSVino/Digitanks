#ifndef DT_HUD_H
#define DT_HUD_H

#include "glgui/glgui.h"

typedef enum
{
	POWERBAR_ATTACK,
	POWERBAR_DEFENSE,
	POWERBAR_MOVEMENT,
} powerbar_type_t;

class CPowerBar : public glgui::CBaseControl
{
public:
								CPowerBar(powerbar_type_t ePowerbarType);

public:
	void						Paint(int x, int y, int w, int h);

protected:
	powerbar_type_t				m_ePowerbarType;
};

class CHUD : public glgui::CPanel
{
public:
								CHUD();

public:
	virtual void				Layout();
	virtual void				Think();

	void						Paint(int x, int y, int w, int h);

	void						SetGame(class CDigitanksGame* pGame) { m_pGame = pGame; };

protected:
	class CDigitanksGame*		m_pGame;

	CPowerBar*					m_pAttackPower;
	CPowerBar*					m_pDefensePower;
	CPowerBar*					m_pMovementPower;
};

#endif