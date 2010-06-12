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

	glgui::CScrollSelector<int>*	m_pDifficulty;
	glgui::CLabel*					m_pDifficultyLabel;

	glgui::CCheckBox*				m_pTutorialBox;
	glgui::CLabel*					m_pTutorialLabel;

	glgui::CButton*					m_pStartGame;
	glgui::CButton*					m_pExit;
};

class CVictoryPanel : public glgui::CPanel
{
	DECLARE_CLASS(CVictoryPanel, glgui::CPanel);

public:
									CVictoryPanel();

public:
	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	virtual bool					IsCursorListener() {return true;};
	virtual bool					MousePressed(int code, int mx, int my);
	virtual bool					KeyPressed(int iKey);

	virtual void					GameOver(bool bPlayerWon);

protected:
	glgui::CLabel*					m_pVictory;
};

class CDonatePanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CDonatePanel, glgui::CPanel);

public:
									CDonatePanel();

public:
	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	virtual void					ClosingApplication();

	EVENT_CALLBACK(CDonatePanel, Donate);
	EVENT_CALLBACK(CDonatePanel, Exit);

protected:
	glgui::CLabel*					m_pDonate;

	glgui::CButton*					m_pDonateButton;
	glgui::CButton*					m_pExitButton;
};

#endif