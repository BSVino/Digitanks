#ifndef DT_MENU_H
#define DT_MENU_H

#include <common.h>
#include "glgui/glgui.h"

class CDigitanksMenu : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CDigitanksMenu, glgui::CPanel);

public:
									CDigitanksMenu();

public:
	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	virtual void					SetVisible(bool bVisible);

	EVENT_CALLBACK(CDigitanksMenu, Tutorial);
	EVENT_CALLBACK(CDigitanksMenu, StartGame);
	EVENT_CALLBACK(CDigitanksMenu, Exit);

protected:
	glgui::CLabel*					m_pDigitanks;

	glgui::CScrollSelector<int>*	m_pNumberOfPlayers;
	glgui::CLabel*					m_pPlayersLabel;

	glgui::CScrollSelector<int>*	m_pNumberOfTanks;
	glgui::CLabel*					m_pTanksLabel;

	glgui::CCheckBox*				m_pTutorialBox;
	glgui::CLabel*					m_pTutorialLabel;

	glgui::CButton*					m_pStartGame;
	glgui::CButton*					m_pExit;
};

#endif