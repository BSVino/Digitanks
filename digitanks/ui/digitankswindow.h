#ifndef DT_DIGITANKSWINDOW_H
#define DT_DIGITANKSWINDOW_H

#include <string>
#include <vector.h>
#include <color.h>

class CDigitanksWindow
{
public:
								CDigitanksWindow();
								~CDigitanksWindow();

	void						InitUI();

	void						CompileShaders();

	void						Run();	// Doesn't return

	static size_t				LoadTextureIntoGL(std::wstring sFilename);

	void						Layout();

	static void					RenderCallback() { Get()->Render(); };
	void						Render();
	void						RenderGround();
	void						RenderObjects();
	void						RenderLightSource();
	void						RenderGame(class CDigitanksGame* pGame);
	void						RenderTank(class CDigitank* pTank, Vector vecOrigin, EAngle angDirection, Color clrTank);
	void						RenderMovementSelection();

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

	static void					SpecialCallback(int k, int x, int y) { Get()->Special(k, x, y); };
	void						Special(int k, int x, int y);

	int							GetWindowWidth() { return (int)m_iWindowWidth; };
	int							GetWindowHeight() { return (int)m_iWindowHeight; };

	Vector						ScreenPosition(Vector vecWorld);
	Vector						WorldPosition(Vector vecScreen);
	bool						GetMouseGridPosition(Vector& vecPoint);

	CDigitanksGame*				GetGame() { return m_pDigitanksGame; };

	static CDigitanksWindow*	Get() { return s_pDigitanksWindow; };

protected:
	int							m_iMouseStartX;
	int							m_iMouseStartY;

	size_t						m_iWindowWidth;
	size_t						m_iWindowHeight;

	double						m_aiModelView[16];
	double						m_aiProjection[16];
	int							m_aiViewport[4];

	class CDigitanksGame*		m_pDigitanksGame;

	class CHUD*					m_pHUD;

	static CDigitanksWindow*	s_pDigitanksWindow;
};

inline class CDigitanksGame* DigitanksGame()
{
	if (!CDigitanksWindow::Get())
		return NULL;

	return CDigitanksWindow::Get()->GetGame();
}

#endif