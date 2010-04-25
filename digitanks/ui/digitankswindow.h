#ifndef DT_DIGITANKSWINDOW_H
#define DT_DIGITANKSWINDOW_H

#include <string>

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

	static CDigitanksWindow*	Get() { return s_pDigitanksWindow; };

protected:
	int							m_iMouseStartX;
	int							m_iMouseStartY;

	size_t						m_iWindowWidth;
	size_t						m_iWindowHeight;

	static CDigitanksWindow*	s_pDigitanksWindow;
};

#endif