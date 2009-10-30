#ifndef CF_MODELWINDOW_H
#define CF_MODELWINDOW_H

#include <modelconverter/convmesh.h>

class CModelWindow
{
public:
							CModelWindow();

	void					Run();	// Doesn't return

	void					ReadFile(const char* pszFile);

	void					LoadIntoGL();

	static void				RenderCallback() { Get()->Render(); };
	void					Render();
	void					RenderGround();
	void					RenderObjects();
	void					RenderLightSource();

	static void				WindowResizeCallback(int x, int y) { Get()->WindowResize(x, y); };
	void					WindowResize(int x, int y);

	static void				IdleCallback() { Get()->Idle(); };
	void					Idle();

	static void				MouseMotionCallback(int x, int y) { Get()->MouseMotion(x, y); };
	void					MouseMotion(int x, int y);

	static void				MouseInputCallback(int iButton, int iState, int x, int y) { Get()->MouseInput(iButton, iState, x, y); };
	void					MouseInput(int iButton, int iState, int x, int y);

	static void				VisibleCallback(int vis) { Get()->Visible(vis); };
	void					Visible(int vis);

	static void				KeyPressCallback(unsigned char c, int x, int y) { Get()->KeyPress(c, x, y); };
	void					KeyPress(unsigned char c, int x, int y);

	static void				SpecialCallback(int k, int x, int y) { Get()->Special(k, x, y); };
	void					Special(int k, int x, int y);

	size_t					GetNextObjectId();

	static CModelWindow*	Get() { return s_pModelWindow; };

protected:
	CConversionScene		m_Scene;

	std::vector<size_t>		m_aiObjects;
	size_t					m_iObjectsCreated;

	std::vector<size_t>		m_aiMaterials;

	float					m_flCameraDistance;

	bool					m_bCameraRotating;
	bool					m_bCameraDollying;
	bool					m_bLightRotating;

	int						m_iMouseStartX;
	int						m_iMouseStartY;

	float					m_flCameraYaw;
	float					m_flCameraPitch;

	float					m_flLightYaw;
	float					m_flLightPitch;

	static CModelWindow*	s_pModelWindow;
};

#endif