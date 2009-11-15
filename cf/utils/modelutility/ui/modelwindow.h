#ifndef CF_MODELWINDOW_H
#define CF_MODELWINDOW_H

#include <modelconverter/convmesh.h>
#include "modelgui.h"

class CMaterial
{
public:
	CMaterial(size_t iBase)
	{
		m_iBase = iBase;
		m_iColorAO = 0;
	}

	size_t		m_iBase;
	size_t		m_iColorAO;
};

typedef enum
{
	DT_WIREFRAME,
	DT_FLAT,
	DT_SMOOTH,
} displaytype_t;

class CModelWindow : public modelgui::IEventListener
{
public:
							CModelWindow();
							~CModelWindow();

	void					InitUI();

	void					Run();	// Doesn't return

	void					DestroyAll();
	void					ReadFile(const wchar_t* pszFile);
	void					ReloadFromFile();

	void					LoadIntoGL();
	size_t					LoadTextureIntoGL(const wchar_t* pszFilename);
	void					LoadTexturesIntoGL();
	void					CreateGLLists();

	void					SaveFile(const wchar_t* pszFile);

	static void				RenderCallback() { Get()->Render(); };
	void					Render();
	void					Render3D();
	void					RenderGround();
	void					RenderObjects();
	void					RenderLightSource();
	void					RenderUV();

	static void				WindowResizeCallback(int x, int y) { Get()->WindowResize(x, y); };
	void					WindowResize(int x, int y);

	static void				DisplayCallback() { Get()->Display(); };
	void					Display();

	static void				MouseMotionCallback(int x, int y) { Get()->MouseMotion(x, y); };
	void					MouseMotion(int x, int y);

	static void				MouseDraggedCallback(int x, int y) { Get()->MouseDragged(x, y); };
	void					MouseDragged(int x, int y);

	static void				MouseInputCallback(int iButton, int iState, int x, int y) { Get()->MouseInput(iButton, iState, x, y); };
	void					MouseInput(int iButton, int iState, int x, int y);

	static void				VisibleCallback(int vis) { Get()->Visible(vis); };
	void					Visible(int vis);

	static void				KeyPressCallback(unsigned char c, int x, int y) { Get()->KeyPress(c, x, y); };
	void					KeyPress(unsigned char c, int x, int y);

	static void				SpecialCallback(int k, int x, int y) { Get()->Special(k, x, y); };
	void					Special(int k, int x, int y);

	EVENT_CALLBACK(CModelWindow, Open);
	EVENT_CALLBACK(CModelWindow, Reload);
	EVENT_CALLBACK(CModelWindow, Save);
	EVENT_CALLBACK(CModelWindow, Close);
	EVENT_CALLBACK(CModelWindow, Exit);
	EVENT_CALLBACK(CModelWindow, Render3D);
	EVENT_CALLBACK(CModelWindow, RenderUV);
	EVENT_CALLBACK(CModelWindow, Wireframe);
	EVENT_CALLBACK(CModelWindow, Flat);
	EVENT_CALLBACK(CModelWindow, Smooth);
	EVENT_CALLBACK(CModelWindow, Light);
	EVENT_CALLBACK(CModelWindow, Texture);
	EVENT_CALLBACK(CModelWindow, ColorAO);
	EVENT_CALLBACK(CModelWindow, LightToggle);
	EVENT_CALLBACK(CModelWindow, TextureToggle);
	EVENT_CALLBACK(CModelWindow, ColorAOToggle);
	EVENT_CALLBACK(CModelWindow, GenerateColorAO);
	EVENT_CALLBACK(CModelWindow, About);

	size_t					GetNextObjectId();

	wchar_t*				OpenFileDialog();
	wchar_t*				SaveFileDialog();
	void					OpenAboutPanel();

	// UI
	void					SetRenderMode(bool bUV);
	void					SetDisplayType(displaytype_t eType);
	void					SetDisplayLight(bool bLight);
	void					SetDisplayTexture(bool bTexture);
	void					SetDisplayColorAO(bool bAO);

	static CModelWindow*	Get() { return s_pModelWindow; };

	void					ClearDebugLines();
	void					AddDebugLine(Vector vecStart, Vector vecEnd);

protected:
	CConversionScene		m_Scene;
	wchar_t					m_szFileLoaded[1024];

	std::vector<size_t>		m_aiObjects;
	size_t					m_iObjectsCreated;

	std::vector<CMaterial>	m_aoMaterials;

	CMaterial*				m_pLightHalo;
	CMaterial*				m_pLightBeam;

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

	float					m_flCameraUVZoom;

	size_t					m_iWindowWidth;
	size_t					m_iWindowHeight;

	std::vector<Vector>		m_avecDebugLines;

	// Options
	bool					m_bRenderUV;
	displaytype_t			m_eDisplayType;
	bool					m_bDisplayLight;
	bool					m_bDisplayTexture;
	bool					m_bDisplayColorAO;

	// Controls
	modelgui::CButton*		m_pRender3D;
	modelgui::CButton*		m_pRenderUV;

	modelgui::CButton*		m_pWireframe;
	modelgui::CButton*		m_pFlat;
	modelgui::CButton*		m_pSmooth;
	modelgui::CButton*		m_pLight;
	modelgui::CButton*		m_pTexture;
	modelgui::CButton*		m_pColorAO;

	static CModelWindow*	s_pModelWindow;
};

#endif