#ifndef DT_UI_H
#define DT_UI_H

#include <common.h>

#include <glgui/label.h>
#include <glgui/button.h>
#include <glgui/selector.h>

class CDigitanksMenu : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CDigitanksMenu, glgui::CPanel);

public:
									CDigitanksMenu();

public:
	virtual void					Layout();
	virtual void					Paint(float x, float y, float w, float h);

	virtual void					SetVisible(bool bVisible);

	EVENT_CALLBACK(CDigitanksMenu, Exit);
	EVENT_CALLBACK(CDigitanksMenu, Close);
	EVENT_CALLBACK(CDigitanksMenu, Save);
	EVENT_CALLBACK(CDigitanksMenu, SaveFile);
	EVENT_CALLBACK(CDigitanksMenu, Load);
	EVENT_CALLBACK(CDigitanksMenu, Open);
	EVENT_CALLBACK(CDigitanksMenu, Options);
	EVENT_CALLBACK(CDigitanksMenu, Quit);

protected:
	glgui::CControl<glgui::CLabel>  m_pDigitanks;

	glgui::CControl<glgui::CScrollSelector<int>> m_pDifficulty;
	glgui::CControl<glgui::CLabel>  m_pDifficultyLabel;

	glgui::CControl<glgui::CButton> m_pReturnToMenu;
	glgui::CControl<glgui::CButton> m_pReturnToGame;
	glgui::CControl<glgui::CButton> m_pSaveGame;
	glgui::CControl<glgui::CButton> m_pLoadGame;
	glgui::CControl<glgui::CButton> m_pOptions;
	glgui::CControl<glgui::CButton> m_pExit;

	glgui::CControl<class COptionsPanel> m_pOptionsPanel;
};

class CVictoryPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CVictoryPanel, glgui::CPanel);

public:
									CVictoryPanel();

public:
	virtual void					Layout();
	virtual void					Paint(float x, float y, float w, float h);

	virtual void					GameOver(bool bPlayerWon);

	EVENT_CALLBACK(CVictoryPanel, Restart);

protected:
	glgui::CControl<glgui::CLabel>  m_pVictory;

	glgui::CControl<glgui::CButton> m_pRestart;
};

class CStoryPanel : public glgui::CPanel
{
	DECLARE_CLASS(CStoryPanel, glgui::CPanel);

public:
									CStoryPanel();

public:
	virtual void					Layout();
	virtual void					Paint(float x, float y, float w, float h);

	virtual bool					IsCursorListener() {return true;};
	virtual bool					MousePressed(int code, int mx, int my);
	virtual bool					KeyPressed(int iKey, bool bCtrlDown = false);

protected:
	glgui::CControl<glgui::CLabel> m_pStory;
};

#endif