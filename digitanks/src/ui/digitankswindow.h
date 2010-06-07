#ifndef DT_DIGITANKSWINDOW_H
#define DT_DIGITANKSWINDOW_H

#include <string>
#include <vector.h>
#include <color.h>

typedef enum
{
	MODE_NONE = 0,
	MODE_MOVE,
	MODE_TURN,
	MODE_AIM,
	MODE_FIRE,
} controlmode_t;

class CDigitanksWindow
{
public:
								CDigitanksWindow();
								~CDigitanksWindow();

public:
	void						InitUI();

	void						CreateGame(int iPlayers, int iTanks);

	void						Run();	// Doesn't return

	void						Layout();

	static void					RenderCallback() { Get()->Render(); };
	void						Render();

	static void					WindowResizeCallback(int x, int y) { Get()->WindowResize(x, y); };
	void						WindowResize(int x, int y);

	static void					DisplayCallback() { Get()->Display(); };
	void						Display();

	static void					MouseMotionCallback(int x, int y) { Get()->MouseMotion(x, y); };
	void						MouseMotion(int x, int y);

	static void					MouseDraggedCallback(int x, int y) { Get()->MouseDragged(x, y); };
	void						MouseDragged(int x, int y);

	static void					MouseInputCallback(int iButton, int iState, int x, int y) { Get()->MouseInput(iButton, iState, x, y); };
	void						MouseInput(int iButton, int iState, int x, int y);

	static void					VisibleCallback(int vis) { Get()->Visible(vis); };
	void						Visible(int vis);

	static void					KeyPressCallback(unsigned char c, int x, int y) { Get()->KeyPress(c, x, y); };
	void						KeyPress(unsigned char c, int x, int y);

	static void					KeyReleaseCallback(unsigned char c, int x, int y) { Get()->KeyRelease(c, x, y); };
	void						KeyRelease(unsigned char c, int x, int y);

	static void					SpecialCallback(int k, int x, int y) { Get()->Special(k, x, y); };
	void						Special(int k, int x, int y);

	void						FakeCtrlAltShift();
	bool						IsCtrlDown() { return m_bCtrl; };
	bool						IsAltDown() { return m_bAlt; };
	bool						IsShiftDown() { return m_bShift; };

	int							GetWindowWidth() { return (int)m_iWindowWidth; };
	int							GetWindowHeight() { return (int)m_iWindowHeight; };

	bool						GetMouseGridPosition(Vector& vecPoint);

	void						GameOver(bool bPlayerWon);

	class CDigitanksMenu*		GetMenu() { return m_pMenu; };
	class CDigitanksGame*		GetGame() { return m_pDigitanksGame; };
	class CInstructor*			GetInstructor() { return m_pInstructor; };

	controlmode_t				GetControlMode();
	void						SetControlMode(controlmode_t eMode, bool bAutoProceed = false);

	static CDigitanksWindow*	Get() { return s_pDigitanksWindow; };

protected:
	int							m_iMouseStartX;
	int							m_iMouseStartY;

	size_t						m_iWindowWidth;
	size_t						m_iWindowHeight;

	class CDigitanksMenu*		m_pMenu;
	class CVictoryPanel*		m_pVictory;

	class CDigitanksGame*		m_pDigitanksGame;

	class CHUD*					m_pHUD;

	class CInstructor*			m_pInstructor;

	static CDigitanksWindow*	s_pDigitanksWindow;

	controlmode_t				m_eControlMode;

	bool						m_bCtrl;
	bool						m_bAlt;
	bool						m_bShift;
};

#endif