#ifndef DT_UI_H
#define DT_UI_H

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

	EVENT_CALLBACK(CDigitanksMenu, Exit);
	EVENT_CALLBACK(CDigitanksMenu, Close);
	EVENT_CALLBACK(CDigitanksMenu, Save);
	EVENT_CALLBACK(CDigitanksMenu, Load);
	EVENT_CALLBACK(CDigitanksMenu, Options);
	EVENT_CALLBACK(CDigitanksMenu, Quit);

protected:
	glgui::CLabel*					m_pDigitanks;

	glgui::CScrollSelector<int>*	m_pDifficulty;
	glgui::CLabel*					m_pDifficultyLabel;

	glgui::CButton*					m_pReturnToMenu;
	glgui::CButton*					m_pReturnToGame;
	glgui::CButton*					m_pSaveGame;
	glgui::CButton*					m_pLoadGame;
	glgui::CButton*					m_pOptions;
	glgui::CButton*					m_pExit;

	class COptionsPanel*			m_pOptionsPanel;
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
	virtual bool					KeyPressed(int iKey, bool bCtrlDown = false);

	virtual void					GameOver(bool bPlayerWon);

protected:
	glgui::CLabel*					m_pVictory;
};

class CPurchasePanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CPurchasePanel, glgui::CPanel);

public:
									CPurchasePanel();

public:
	virtual void					Layout();
	virtual void					Think();
	virtual void					Paint(int x, int y, int w, int h);

	virtual void					ClosingApplication();
	virtual void					OpeningApplication();

	EVENT_CALLBACK(CPurchasePanel, Purchase);
	EVENT_CALLBACK(CPurchasePanel, Exit);
	EVENT_CALLBACK(CPurchasePanel, MainMenu);
	EVENT_CALLBACK(CPurchasePanel, Email);

protected:
	glgui::CLabel*					m_pPurchase;
	glgui::CButton*					m_pPurchaseButton;

	glgui::CLabel*					m_pEnterEmail;
	glgui::CTextField*				m_pEmail;
	glgui::CButton*					m_pContinueButton;

	bool							m_bClosing;

	float							m_flTimeOpened;
};

class CStoryPanel : public glgui::CPanel
{
	DECLARE_CLASS(CStoryPanel, glgui::CPanel);

public:
									CStoryPanel();

public:
	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	virtual bool					IsCursorListener() {return true;};
	virtual bool					MousePressed(int code, int mx, int my);
	virtual bool					KeyPressed(int iKey, bool bCtrlDown = false);

protected:
	glgui::CLabel*					m_pStory;
};

#endif