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
	EVENT_CALLBACK(CDigitanksMenu, Quit);

protected:
	glgui::CLabel*					m_pDigitanks;

	glgui::CScrollSelector<int>*	m_pDifficulty;
	glgui::CLabel*					m_pDifficultyLabel;

	glgui::CButton*					m_pReturnToMenu;
	glgui::CButton*					m_pReturnToGame;
	glgui::CButton*					m_pSaveGame;
	glgui::CButton*					m_pLoadGame;
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
	virtual void					Paint(int x, int y, int w, int h);

	virtual void					ClosingApplication();
	virtual void					OpeningApplication();

	EVENT_CALLBACK(CPurchasePanel, Purchase);
	EVENT_CALLBACK(CPurchasePanel, Exit);
	EVENT_CALLBACK(CPurchasePanel, MainMenu);
	EVENT_CALLBACK(CPurchasePanel, Register);
	EVENT_CALLBACK(CPurchasePanel, RegisterOffline);
	EVENT_CALLBACK(CPurchasePanel, CopyProductCode);
	EVENT_CALLBACK(CPurchasePanel, SetKey);

protected:
	glgui::CLabel*					m_pPurchase;

	glgui::CTextField*				m_pRegistrationKey;
	glgui::CButton*					m_pRegister;
	glgui::CLabel*					m_pRegisterResult;

	glgui::CButton*					m_pRegisterOffline;
	glgui::CLabel*					m_pProductCode;

	glgui::CButton*					m_pPurchaseButton;
	glgui::CButton*					m_pExitButton;
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