#ifndef CF_MODELWINDOW_H
#define CF_MODELWINDOW_H

#include <modelconverter/convmesh.h>
#include "modelgui.h"
#include <worklistener.h>

class CMaterial
{
public:
	CMaterial(size_t iBase)
	{
		m_iBase = iBase;
		m_iNormal = 0;
		m_iNormal2 = 0;
		m_iAO = 0;
		m_iColorAO = 0;
	}

	size_t		m_iBase;
	size_t		m_iNormal;
	size_t		m_iNormal2;
	size_t		m_iAO;
	size_t		m_iColorAO;
};

class CModelWindow : public modelgui::IEventListener, IWorkListener
{
public:
							CModelWindow();
							~CModelWindow();

	void					InitUI();

	void					CompileShaders();

	void					Run();	// Doesn't return

	void					DestroyAll();
	void					ReadFile(const wchar_t* pszFile);
	void					ReadFileIntoScene(const wchar_t* pszFile);
	void					ReloadFromFile();

	void					LoadIntoGL();
	static size_t			LoadTextureIntoGL(std::wstring sFilename);
	void					LoadTexturesIntoGL();

	// License management
	static unsigned long	PNG_CRC(unsigned char *buf, int len);
	static void				LoadSMAKTexture();
	static void				SetupSMAKTexture();
	static void				SaveSMAKTexture();
	static bool				GetSMAKTexture();
	static std::string		GetSMAKTextureCode();
	static void				GetSMAKTextureId(unsigned char* paiId);
	static void				SetSMAKTexture(const char* pszTex);

	void					SaveFile(const wchar_t* pszFile);

	void					Layout();

	static void				RenderCallback() { Get()->Render(); };
	void					Render();
	void					Render3D();
	void					RenderGround();
	void					RenderObjects();
	void					RenderSceneNode(CConversionSceneNode* pNode);
	void					RenderMeshInstance(CConversionMeshInstance* pMeshInstance);
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
	EVENT_CALLBACK(CModelWindow, OpenInto);
	EVENT_CALLBACK(CModelWindow, Reload);
	EVENT_CALLBACK(CModelWindow, Save);
	EVENT_CALLBACK(CModelWindow, Close);
	EVENT_CALLBACK(CModelWindow, Exit);
	EVENT_CALLBACK(CModelWindow, Render3D);
	EVENT_CALLBACK(CModelWindow, RenderUV);
	EVENT_CALLBACK(CModelWindow, SceneTree);
	EVENT_CALLBACK(CModelWindow, Wireframe);
	EVENT_CALLBACK(CModelWindow, Flat);
	EVENT_CALLBACK(CModelWindow, Smooth);
	EVENT_CALLBACK(CModelWindow, UVWireframe);
	EVENT_CALLBACK(CModelWindow, Light);
	EVENT_CALLBACK(CModelWindow, Texture);
	EVENT_CALLBACK(CModelWindow, Normal);
	EVENT_CALLBACK(CModelWindow, AO);
	EVENT_CALLBACK(CModelWindow, ColorAO);
	EVENT_CALLBACK(CModelWindow, LightToggle);
	EVENT_CALLBACK(CModelWindow, TextureToggle);
	EVENT_CALLBACK(CModelWindow, NormalToggle);
	EVENT_CALLBACK(CModelWindow, AOToggle);
	EVENT_CALLBACK(CModelWindow, ColorAOToggle);
	EVENT_CALLBACK(CModelWindow, GenerateAO);
	EVENT_CALLBACK(CModelWindow, GenerateColorAO);
	EVENT_CALLBACK(CModelWindow, GenerateNormal);
	EVENT_CALLBACK(CModelWindow, Help);
	EVENT_CALLBACK(CModelWindow, Register);
	EVENT_CALLBACK(CModelWindow, About);

	size_t					GetNextObjectId();

	static wchar_t*			OpenFileDialog(wchar_t* pszFileTypes);
	static wchar_t*			SaveFileDialog(wchar_t* pszFileTypes);
	static std::string		GetClipboard();
	static void				SetClipboard(const std::string& sBuf);

	void					OpenHelpPanel();
	void					OpenRegisterPanel();
	void					OpenAboutPanel();

	// UI
	bool					GetRenderMode() { return m_bRenderUV; };
	void					SetRenderMode(bool bUV);
	void					SetDisplayWireframe(bool bWire);
	void					SetDisplayUVWireframe(bool bWire);
	void					SetDisplayLight(bool bLight);
	void					SetDisplayTexture(bool bTexture);
	void					SetDisplayNormal(bool bNormal);
	void					SetDisplayAO(bool bAO);
	void					SetDisplayColorAO(bool bAO);

	int						GetWindowWidth() { return (int)m_iWindowWidth; };
	int						GetWindowHeight() { return (int)m_iWindowHeight; };

	size_t					GetArrowTexture() { return m_iArrowTexture; };
	size_t					GetEditTexture() { return m_iEditTexture; };
	size_t					GetVisibilityTexture() { return m_iVisibilityTexture; };
	size_t					GetMaterialsNodeTexture() { return m_iTextureTexture; };
	size_t					GetMeshesNodeTexture() { return m_iWireframeTexture; };
	size_t					GetScenesNodeTexture() { return m_iAOTexture; };

	static CModelWindow*	Get() { return s_pModelWindow; };

	CConversionScene*		GetScene() { return &m_Scene; };
	std::vector<CMaterial>*	GetMaterials() { return &m_aoMaterials; };

	void					ClearDebugLines();
	void					AddDebugLine(Vector vecStart, Vector vecEnd);

	void					BeginProgress();
	void					SetAction(wchar_t* pszAction, size_t iTotalProgress);
	void					WorkProgress(size_t iProgress, bool bForceDraw = false);
	void					EndProgress();

protected:
	CConversionScene		m_Scene;
	bool					m_bLoadingFile;
	wchar_t					m_szFileLoaded[1024];

	std::vector<size_t>		m_aiObjects;
	size_t					m_iObjectsCreated;

	std::vector<CMaterial>	m_aoMaterials;

	CMaterial*				m_pLightHalo;
	CMaterial*				m_pLightBeam;

	size_t					m_iWireframeTexture;
	size_t					m_iFlatTexture;
	size_t					m_iSmoothTexture;
	size_t					m_iUVTexture;
	size_t					m_iLightTexture;
	size_t					m_iTextureTexture;
	size_t					m_iNormalTexture;
	size_t					m_iAOTexture;
	size_t					m_iCAOTexture;
	size_t					m_iArrowTexture;
	size_t					m_iVisibilityTexture;
	size_t					m_iEditTexture;

	float					m_flCameraDistance;

	bool					m_bCameraRotating;
	bool					m_bCameraDollying;
	bool					m_bCameraPanning;
	bool					m_bLightRotating;

	int						m_iMouseStartX;
	int						m_iMouseStartY;

	float					m_flCameraYaw;
	float					m_flCameraPitch;

	float					m_flLightYaw;
	float					m_flLightPitch;
	Vector					m_vecLightPosition;

	float					m_flCameraUVX;
	float					m_flCameraUVY;
	float					m_flCameraUVZoom;

	size_t					m_iWindowWidth;
	size_t					m_iWindowHeight;

	std::vector<Vector>		m_avecDebugLines;

	// Options
	bool					m_bRenderUV;
	bool					m_bDisplayWireframe;
	bool					m_bDisplayUV;
	bool					m_bDisplayLight;
	bool					m_bDisplayTexture;
	bool					m_bDisplayNormal;
	bool					m_bDisplayAO;
	bool					m_bDisplayColorAO;

	class GLUtesselator*	m_pTesselator;

	size_t					m_iShaderProgram;

	// Controls
	modelgui::CButton*		m_pRender3D;
	modelgui::CButton*		m_pRenderUV;

	modelgui::CButton*		m_pWireframe;
	modelgui::CButton*		m_pFlat;
	modelgui::CButton*		m_pSmooth;
	modelgui::CButton*		m_pUVWireframe;
	modelgui::CButton*		m_pLight;
	modelgui::CButton*		m_pTexture;
	modelgui::CButton*		m_pNormal;
	modelgui::CButton*		m_pAO;
	modelgui::CButton*		m_pColorAO;

	static CModelWindow*	s_pModelWindow;
};

#endif