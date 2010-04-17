#include "modelwindow_ui.h"

#include <IL/il.h>
#include <maths.h>
#include <GL/freeglut.h>
#include <strutils.h>

#include "modelwindow.h"
#include "scenetree.h"
#include "../smak_version.h"
#include "picker.h"

// wingdi.h
#undef GetObject

void CModelWindow::InitUI()
{
	CMenu* pFile = CRootPanel::Get()->AddMenu("File");
	CMenu* pView = CRootPanel::Get()->AddMenu("View");
	CMenu* pTools = CRootPanel::Get()->AddMenu("Tools");
	CMenu* pHelp = CRootPanel::Get()->AddMenu("Help");

	pFile->AddSubmenu("Open...", this, Open);
	pFile->AddSubmenu("Open Into...", this, OpenInto);
	pFile->AddSubmenu("Reload", this, Reload);
//	pFile->AddSubmenu("Save...", this, Save);
	pFile->AddSubmenu("Close", this, Close);
	pFile->AddSubmenu("Exit", this, Exit);

	pView->AddSubmenu("3D view", this, Render3D);
	pView->AddSubmenu("UV view", this, RenderUV);
	pView->AddSubmenu("View wireframe", this, Wireframe);
	pView->AddSubmenu("Toggle light", this, LightToggle);
	pView->AddSubmenu("Toggle texture", this, TextureToggle);
	pView->AddSubmenu("Toggle normal map", this, NormalToggle);
	pView->AddSubmenu("Toggle AO map", this, AOToggle);
	pView->AddSubmenu("Toggle color AO map", this, ColorAOToggle);

	pTools->AddSubmenu("Generate AO map", this, GenerateAO);
	pTools->AddSubmenu("Generate color AO map", this, GenerateColorAO);
	pTools->AddSubmenu("Generate normal map", this, GenerateNormal);

	pHelp->AddSubmenu("Help", this, Help);
	pHelp->AddSubmenu("Register...", this, Register);
	pHelp->AddSubmenu("About SMAK", this, About);

	CButtonPanel* pTopButtons = new CButtonPanel(BA_TOP);

	m_pRender3D = new CButton(0, 0, 100, 100, "3D", true);
	m_pRenderUV = new CButton(0, 0, 100, 100, "UV", true);

	pTopButtons->AddButton(m_pRender3D, "Render 3D View", false, this, Render3D);
	pTopButtons->AddButton(m_pRenderUV, "Render UV View", false, this, RenderUV);

	CRootPanel::Get()->AddControl(pTopButtons);

	CButtonPanel* pBottomButtons = new CButtonPanel(BA_BOTTOM);

	m_pWireframe = new CPictureButton("Wire", m_iWireframeTexture, true);
	m_pUVWireframe = new CPictureButton("Wire", m_iUVTexture, true);
	m_pLight = new CPictureButton("Lght", m_iLightTexture, true);
	m_pTexture = new CPictureButton("Tex", m_iTextureTexture, true);
	m_pNormal = new CPictureButton("Nrml", m_iNormalTexture, true);
	m_pAO = new CPictureButton("AO", m_iAOTexture, true);
	m_pColorAO = new CPictureButton("C AO", m_iCAOTexture, true);

	pBottomButtons->AddButton(m_pWireframe, "Toggle Wireframe", true, this, Wireframe);
	pBottomButtons->AddButton(m_pUVWireframe, "Toggle UVs", true, this, UVWireframe);
	pBottomButtons->AddButton(m_pLight, "Toggle Light", false, this, Light);
	pBottomButtons->AddButton(m_pTexture, "Toggle Texture", false, this, Texture);
	pBottomButtons->AddButton(m_pNormal, "Toggle Normal Map", false, this, Normal);
	pBottomButtons->AddButton(m_pAO, "Toggle AO Map", false, this, AO);
	pBottomButtons->AddButton(m_pColorAO, "Toggle Color AO", false, this, ColorAO);

	CRootPanel::Get()->AddControl(pBottomButtons);

	CRootPanel::Get()->Layout();

	CSceneTreePanel::Open(&m_Scene);
}

void CModelWindow::Layout()
{
	CRootPanel::Get()->Layout();
}

void CModelWindow::OpenCallback()
{
	if (m_bLoadingFile)
		return;

	ReadFile(OpenFileDialog(L"All *.obj;*.sia;*.dae\0*.obj;*.sia;*.dae\0"));
}

void CModelWindow::OpenIntoCallback()
{
	if (m_bLoadingFile)
		return;

	ReadFileIntoScene(OpenFileDialog(L"All *.obj;*.sia;*.dae\0*.obj;*.sia;*.dae\0"));
}

void CModelWindow::ReloadCallback()
{
	if (m_bLoadingFile)
		return;

	ReloadFromFile();
}

void CModelWindow::SaveCallback()
{
	SaveFile(SaveFileDialog(L"Valve Source SMD\0*.smd\0"));
}

void CModelWindow::CloseCallback()
{
	if (m_bLoadingFile)
		return;

	DestroyAll();
}

void CModelWindow::ExitCallback()
{
	exit(0);
}

void CModelWindow::Render3DCallback()
{
	SetRenderMode(false);
}

void CModelWindow::RenderUVCallback()
{
	SetRenderMode(true);
}

void CModelWindow::SceneTreeCallback()
{
	CSceneTreePanel::Open(&m_Scene);
}

void CModelWindow::WireframeCallback()
{
	SetDisplayWireframe(m_pWireframe->GetState());
}

void CModelWindow::UVWireframeCallback()
{
	m_bDisplayUV = m_pUVWireframe->GetState();
}

void CModelWindow::LightCallback()
{
	SetDisplayLight(m_pLight->GetState());
}

void CModelWindow::TextureCallback()
{
	SetDisplayTexture(m_pTexture->GetState());
}

void CModelWindow::NormalCallback()
{
	if (CNormalPanel::Get() && CNormalPanel::Get()->IsGenerating() && !CNormalPanel::Get()->DoneGenerating())
	{
		m_pNormal->SetState(true, false);
		return;
	}

	if (!CNormalPanel::Get() || !CNormalPanel::Get()->DoneGenerating())
	{
		if (!CNormalPanel::Get() || !CNormalPanel::Get()->IsVisible())
			CNormalPanel::Open(&m_Scene, &m_aoMaterials);
	}

	SetDisplayNormal(m_pNormal->GetState());
}

void CModelWindow::AOCallback()
{
	if (CAOPanel::Get(false) && CAOPanel::Get(false)->IsGenerating() && !CAOPanel::Get(false)->DoneGenerating())
	{
		m_pAO->SetState(true, false);
		return;
	}

	if (!CAOPanel::Get(false) || !CAOPanel::Get(false)->DoneGenerating())
	{
		CAOPanel::Open(false, &m_Scene, &m_aoMaterials);
		m_pAO->SetState(false, false);
		return;
	}

	SetDisplayAO(m_pAO->GetState());
}

void CModelWindow::ColorAOCallback()
{
	if (CAOPanel::Get(true) && CAOPanel::Get(true)->IsGenerating() && !CAOPanel::Get(true)->DoneGenerating())
	{
		m_pColorAO->SetState(true, false);
		return;
	}

	if (!CAOPanel::Get(true) || !CAOPanel::Get(true)->DoneGenerating())
	{
		CAOPanel::Open(true, &m_Scene, &m_aoMaterials);
		m_pColorAO->SetState(false, false);
		return;
	}

	SetDisplayColorAO(m_pColorAO->GetState());
}

void CModelWindow::LightToggleCallback()
{
	SetDisplayLight(!m_bDisplayLight);
}

void CModelWindow::TextureToggleCallback()
{
	SetDisplayTexture(!m_bDisplayTexture);
}

void CModelWindow::NormalToggleCallback()
{
	SetDisplayNormal(!m_bDisplayNormal);
}

void CModelWindow::AOToggleCallback()
{
	SetDisplayAO(!m_bDisplayAO);
}

void CModelWindow::ColorAOToggleCallback()
{
	SetDisplayColorAO(!m_bDisplayColorAO);
}

void CModelWindow::GenerateAOCallback()
{
	CAOPanel::Open(false, &m_Scene, &m_aoMaterials);
}

void CModelWindow::GenerateColorAOCallback()
{
	CAOPanel::Open(true, &m_Scene, &m_aoMaterials);
}

void CModelWindow::GenerateNormalCallback()
{
	CNormalPanel::Open(&m_Scene, &m_aoMaterials);
}

void CModelWindow::HelpCallback()
{
	OpenHelpPanel();
}

void CModelWindow::RegisterCallback()
{
	OpenRegisterPanel();
}

void CModelWindow::AboutCallback()
{
	OpenAboutPanel();
}

void CModelWindow::OpenHelpPanel()
{
	CHelpPanel::Open();
}

void CModelWindow::OpenRegisterPanel()
{
	CRegisterPanel::Open();
}

void CModelWindow::OpenAboutPanel()
{
	CAboutPanel::Open();
}

void CModelWindow::BeginProgress()
{
	CProgressBar::Get()->SetVisible(true);
}

void CModelWindow::SetAction(wchar_t* pszAction, size_t iTotalProgress)
{
	CProgressBar::Get()->SetTotalProgress(iTotalProgress);
	CProgressBar::Get()->SetAction(pszAction);
	WorkProgress(0, true);
}

void CModelWindow::WorkProgress(size_t iProgress, bool bForceDraw)
{
	static int iLastTime = 0;
	static int iLastGenerate = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && glutGet(GLUT_ELAPSED_TIME) - iLastTime < 100)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	glutMainLoopEvent();

	CModelWindow::Get()->Render();
	CRootPanel::Get()->Think();
	CRootPanel::Get()->Paint(0, 0, (int)m_iWindowWidth, (int)m_iWindowHeight);
	glutSwapBuffers();

	iLastTime = glutGet(GLUT_ELAPSED_TIME);
}

void CModelWindow::EndProgress()
{
	CProgressBar::Get()->SetVisible(false);
}

#define BTN_HEIGHT 32
#define BTN_SPACE 8
#define BTN_SECTION 18

CButtonPanel::CButtonPanel(buttonalignment_t eAlign)
: CPanel(0, 0, BTN_HEIGHT, BTN_HEIGHT)
{
	m_eAlign = eAlign;
}

void CButtonPanel::Layout()
{
	int iX = 0;

	for (size_t i = 0; i < m_apButtons.size(); i++)
	{
		IControl* pButton = m_apButtons[i];

		if (!pButton->IsVisible())
			continue;

		pButton->SetSize(BTN_HEIGHT, BTN_HEIGHT);
		pButton->SetPos(iX, 0);

		IControl* pHint = m_apHints[i];
		pHint->SetPos(iX + BTN_HEIGHT/2 - pHint->GetWidth()/2, -18);

		iX += BTN_HEIGHT + m_aiSpaces[i];
	}

	SetSize(iX - m_aiSpaces[m_aiSpaces.size()-1], BTN_HEIGHT);
	if (m_eAlign == BA_TOP)
		SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, BTN_HEIGHT + BTN_SPACE);
	else
		SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, CRootPanel::Get()->GetHeight() - BTN_HEIGHT*2 - BTN_SPACE);

	CPanel::Layout();
}

void CButtonPanel::AddButton(CButton* pButton, char* pszHints, bool bNewSection, IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	AddControl(pButton);
	m_apButtons.push_back(pButton);

	m_aiSpaces.push_back(bNewSection?BTN_SECTION:BTN_SPACE);

	CLabel* pHint = new CLabel(0, 0, 0, 0, pszHints);
	pHint->SetAlpha(0);
	pHint->EnsureTextFits();
	AddControl(pHint);
	m_apHints.push_back(pHint);

	if (pListener)
	{
		pButton->SetClickedListener(pListener, pfnCallback);
		pButton->SetUnclickedListener(pListener, pfnCallback);
	}
}

void CButtonPanel::Think()
{
	int mx, my;
	CRootPanel::GetFullscreenMousePos(mx, my);

	for (size_t i = 0; i < m_apButtons.size(); i++)
	{
		if (!m_apButtons[i]->IsVisible())
		{
			m_apHints[i]->SetAlpha((int)Approach(0.0f, (float)m_apHints[i]->GetAlpha(), 30.0f));
			continue;
		}

		int x = 0, y = 0, w = 0, h = 0;
		m_apButtons[i]->GetAbsDimensions(x, y, w, h);

		float flAlpha = (float)m_apHints[i]->GetAlpha();

		if (mx >= x && my >= y && mx < x + w && my < y + h)
			m_apHints[i]->SetAlpha((int)Approach(255.0f, flAlpha, 30.0f));
		else
			m_apHints[i]->SetAlpha((int)Approach(0.0f, flAlpha, 30.0f));
	}

	CPanel::Think();
}

void CButtonPanel::Paint(int x, int y, int w, int h)
{
	CPanel::Paint(x, y, w, h);
}

CProgressBar* CProgressBar::s_pProgressBar = NULL;

CProgressBar::CProgressBar()
	: CPanel(0, 0, 100, 100)
{
	CRootPanel::Get()->AddControl(this);
	SetVisible(false);

	m_pAction = new CLabel(0, 0, 200, BTN_HEIGHT, "");
	AddControl(m_pAction);
	m_pAction->SetWrap(false);

	Layout();
}

void CProgressBar::Layout()
{
	SetSize(GetParent()->GetWidth()/2, BTN_HEIGHT);
	SetPos(GetParent()->GetWidth()/4, GetParent()->GetHeight()/5);

	m_pAction->SetSize(GetWidth(), BTN_HEIGHT);
}

void CProgressBar::Paint(int x, int y, int w, int h)
{
	float flTotalProgress = (float)m_iCurrentProgress/(float)m_iTotalProgress;

	CPanel::PaintRect(x, y, w, h);
	CPanel::PaintRect(x+10, y+10, (int)((w-20)*flTotalProgress), h-20, g_clrBoxHi);

	m_pAction->Paint();
}

void CProgressBar::SetTotalProgress(size_t iProgress)
{
	m_iTotalProgress = iProgress;
	SetProgress(0);
}

void CProgressBar::SetProgress(size_t iProgress, wchar_t* pszAction)
{
	m_iCurrentProgress = iProgress;

	SetAction(pszAction);
}

void CProgressBar::SetAction(wchar_t* pszAction)
{
	if (pszAction)
		m_sAction = std::wstring(pszAction);

	m_pAction->SetText(m_sAction.c_str());

	if (m_iTotalProgress)
	{
		wchar_t szProgress[100];
		wsprintf(szProgress, L" %d/%d", m_iCurrentProgress, m_iTotalProgress);
		m_pAction->AppendText(szProgress);
	}
}

CProgressBar* CProgressBar::Get()
{
	if (!s_pProgressBar)
		s_pProgressBar = new CProgressBar();

	return s_pProgressBar;
}

void CCloseButton::Paint(int x, int y, int w, int h)
{
	Color c;
	
	c.SetRed((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.r() * 2, 255.0f));
	c.SetGreen((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.g(), (float)g_clrBoxHi.g()));
	c.SetBlue((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.b(), (float)g_clrBoxHi.b()));
	c.SetAlpha(255);

	CRootPanel::PaintRect(x, y, w, h, c);
}

void CMinimizeButton::Paint(int x, int y, int w, int h)
{
	Color c;
	
	c.SetRed((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.r(), (float)g_clrBox.r()));
	c.SetGreen((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.g(), (float)g_clrBoxHi.g()));
	c.SetBlue((int)RemapVal(m_flHighlight, 0.0f, 1.0f, (float)g_clrBox.b() * 2, 255.0f));
	c.SetAlpha(255);

	CRootPanel::PaintRect(x, y+h/3+1, w, h/3, c);
}

CMovablePanel::CMovablePanel(char* pszName)
	: CPanel(0, 0, 200, 350)
{
	m_bMoving = false;

	m_pName = new CLabel(0, GetHeight()-HEADER_HEIGHT, GetWidth(), HEADER_HEIGHT, pszName);
	AddControl(m_pName);

	m_pCloseButton = new CCloseButton();
	AddControl(m_pCloseButton);

	m_pCloseButton->SetClickedListener(this, CloseWindow);

	m_pMinimizeButton = new CMinimizeButton();
	AddControl(m_pMinimizeButton);

	m_pMinimizeButton->SetClickedListener(this, MinimizeWindow);

	CRootPanel::Get()->AddControl(this, true);

	m_bHasCloseButton = true;
	m_bMinimized = false;

	m_bClearBackground = false;
}

CMovablePanel::~CMovablePanel()
{
	CRootPanel::Get()->RemoveControl(this);
	Destructor();
}

void CMovablePanel::Layout()
{
	m_pName->SetDimensions(0, 0, GetWidth(), HEADER_HEIGHT);

	m_pCloseButton->SetVisible(m_bHasCloseButton);

	int iButtonSize = HEADER_HEIGHT*2/3;
	if (m_bHasCloseButton)
	{
		m_pCloseButton->SetDimensions(GetWidth() - HEADER_HEIGHT/2 - iButtonSize/2, HEADER_HEIGHT/2 - iButtonSize/2, iButtonSize, iButtonSize);
		m_pMinimizeButton->SetDimensions(GetWidth() - HEADER_HEIGHT*3/2 - iButtonSize/2, HEADER_HEIGHT/2 - iButtonSize/2, iButtonSize, iButtonSize);
	}
	else
		m_pMinimizeButton->SetDimensions(GetWidth() - HEADER_HEIGHT/2 - iButtonSize/2, HEADER_HEIGHT/2 - iButtonSize/2, iButtonSize, iButtonSize);

	CPanel::Layout();
}

void CMovablePanel::Think()
{
	if (m_bMoving)
	{
		int mx, my;
		CRootPanel::GetFullscreenMousePos(mx, my);

		SetPos(m_iStartX + mx - m_iMouseStartX, m_iStartY + my - m_iMouseStartY);
	}

	CPanel::Think();
}

void CMovablePanel::Paint(int x, int y, int w, int h)
{
	if (!m_bMinimized && !m_bClearBackground)
		CRootPanel::PaintRect(x, y, w, h, g_clrPanel);

	Color clrBox = g_clrBoxHi;
	if (m_bClearBackground)
		clrBox.SetAlpha(clrBox.a()*3/2);

	CRootPanel::PaintRect(x, y, w, HEADER_HEIGHT, clrBox);

	if (m_bMinimized)
	{
		m_pName->Paint();
		if (m_bHasCloseButton)
			m_pCloseButton->Paint();
		m_pMinimizeButton->Paint();
	}
	else
		CPanel::Paint(x, y, w, h);
}

bool CMovablePanel::MousePressed(int iButton, int mx, int my)
{
	int x, y;
	GetAbsPos(x, y);

	if (iButton == 0 && mx > x && mx < x + GetWidth() - HEADER_HEIGHT*2 && my > y && my < y + HEADER_HEIGHT )
	{
		m_iMouseStartX = mx;
		m_iMouseStartY = my;
		m_bMoving = true;

		GetPos(x, y);
		m_iStartX = x;
		m_iStartY = y;

		return true;
	}

	return CPanel::MousePressed(iButton, mx, my);
}

bool CMovablePanel::MouseReleased(int iButton, int mx, int my)
{
	if (m_bMoving)
	{
		m_bMoving = false;
		return true;
	}

	CPanel::MouseReleased(iButton, mx, my);

	return false;
}

void CMovablePanel::Minimize()
{
	m_bMinimized = !m_bMinimized;

	if (m_bMinimized)
	{
		m_iNonMinimizedHeight = GetHeight();
		SetSize(GetWidth(), HEADER_HEIGHT);
	}
	else
	{
		SetSize(GetWidth(), m_iNonMinimizedHeight);
	}

	Layout();
}

void CMovablePanel::CloseWindowCallback()
{
	SetVisible(false);
}

void CMovablePanel::MinimizeWindowCallback()
{
	Minimize();
}

CAOPanel* CAOPanel::s_pAOPanel = NULL;
CAOPanel* CAOPanel::s_pColorAOPanel = NULL;

CAOPanel::CAOPanel(bool bColor, CConversionScene* pScene, std::vector<CMaterial>* paoMaterials)
	: CMovablePanel(bColor?"Color AO generator":"AO generator"), m_oGenerator(pScene, paoMaterials)
{
	m_bColor = bColor;

	m_pScene = pScene;
	m_paoMaterials = paoMaterials;

	if (m_bColor)
		SetPos(GetParent()->GetWidth() - GetWidth() - 50, GetParent()->GetHeight() - GetHeight() - 150);
	else
		SetPos(GetParent()->GetWidth() - GetWidth() - 200, GetParent()->GetHeight() - GetHeight() - 100);

	m_pSizeLabel = new CLabel(0, 0, 32, 32, "Size");
	AddControl(m_pSizeLabel);

	m_pSizeSelector = new CScrollSelector<int>();
#ifdef _DEBUG
	m_pSizeSelector->AddSelection(CScrollSelection<int>(16, L"16x16"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(32, L"32x32"));
#endif
	m_pSizeSelector->AddSelection(CScrollSelection<int>(64, L"64x64"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(128, L"128x128"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(256, L"256x256"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(512, L"512x512"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(1024, L"1024x1024"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(2048, L"2048x2048"));
	//m_pSizeSelector->AddSelection(CScrollSelection<int>(4096, L"4096x4096"));
	m_pSizeSelector->SetSelection(2);
	AddControl(m_pSizeSelector);

	m_pEdgeBleedLabel = new CLabel(0, 0, 32, 32, "Edge Bleed");
	AddControl(m_pEdgeBleedLabel);

	m_pEdgeBleedSelector = new CScrollSelector<int>();
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(0, L"0"));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(1, L"1"));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(2, L"2"));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(3, L"3"));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(4, L"4"));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(5, L"5"));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(6, L"6"));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(7, L"7"));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(8, L"8"));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(9, L"9"));
	m_pEdgeBleedSelector->AddSelection(CScrollSelection<int>(10, L"10"));
	m_pEdgeBleedSelector->SetSelection(1);
	AddControl(m_pEdgeBleedSelector);

	if (!m_bColor)
	{
		m_pAOMethodLabel = new CLabel(0, 0, 32, 32, "Method");
		AddControl(m_pAOMethodLabel);

		m_pAOMethodSelector = new CScrollSelector<int>();
		m_pAOMethodSelector->AddSelection(CScrollSelection<int>(AOMETHOD_SHADOWMAP, L"Shadow map (fast!)"));
		m_pAOMethodSelector->AddSelection(CScrollSelection<int>(AOMETHOD_RAYTRACE, L"Raytraced (slow!)"));
		m_pAOMethodSelector->AddSelection(CScrollSelection<int>(AOMETHOD_TRIDISTANCE, L"Tri distance"));
		m_pAOMethodSelector->SetSelectedListener(this, AOMethod);
		AddControl(m_pAOMethodSelector);

		m_pRayDensityLabel = new CLabel(0, 0, 32, 32, "Ray Density");
		AddControl(m_pRayDensityLabel);

		m_pRayDensitySelector = new CScrollSelector<int>();
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(5, L"5"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(6, L"6"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(7, L"7"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(8, L"8"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(9, L"9"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(10, L"10"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(11, L"11"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(12, L"12"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(13, L"13"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(14, L"14"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(15, L"15"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(16, L"16"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(17, L"17"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(18, L"18"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(19, L"19"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(20, L"20"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(21, L"21"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(22, L"22"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(23, L"23"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(24, L"24"));
		m_pRayDensitySelector->AddSelection(CScrollSelection<int>(25, L"25"));
		m_pRayDensitySelector->SetSelection(15);
		AddControl(m_pRayDensitySelector);

		m_pFalloffLabel = new CLabel(0, 0, 32, 32, "Ray Falloff");
		AddControl(m_pFalloffLabel);

		m_pFalloffSelector = new CScrollSelector<float>();
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.01f, L"0.01"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.05f, L"0.05"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.1f, L"0.1"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.25f, L"0.25"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.5f, L"0.5"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(0.75f, L"0.75"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.0f, L"1.0"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.25f, L"1.25"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.5f, L"1.5"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(1.75f, L"1.75"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(2.0f, L"2.0"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(2.5f, L"2.5"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(3.0f, L"3.0"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(4.0f, L"4.0"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(5.0f, L"5.0"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(6.0f, L"6.0"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(7.5f, L"7.5"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(10.0f, L"10"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(25.0f, L"25"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(50.0f, L"50"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(100.0f, L"100"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(250.0f, L"250"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(500.0f, L"500"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(1000.0f, L"1000"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(2500.0f, L"2500"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(5000.0f, L"5000"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(10000.0f, L"10000"));
		m_pFalloffSelector->AddSelection(CScrollSelection<float>(-1.0f, L"No falloff"));
		m_pFalloffSelector->SetSelection(6);
		AddControl(m_pFalloffSelector);

		m_pLightsLabel = new CLabel(0, 0, 32, 32, "Lights");
		AddControl(m_pLightsLabel);

		m_pLightsSelector = new CScrollSelector<int>();
		m_pLightsSelector->AddSelection(CScrollSelection<int>(500, L"500"));
		m_pLightsSelector->AddSelection(CScrollSelection<int>(1000, L"1000"));
		m_pLightsSelector->AddSelection(CScrollSelection<int>(1500, L"1500"));
		m_pLightsSelector->AddSelection(CScrollSelection<int>(2000, L"2000"));
		m_pLightsSelector->AddSelection(CScrollSelection<int>(2500, L"2500"));
		m_pLightsSelector->AddSelection(CScrollSelection<int>(3000, L"3000"));
		m_pLightsSelector->SetSelection(3);
		AddControl(m_pLightsSelector);

		m_pRandomLabel = new CLabel(0, 0, 32, 32, "Randomize rays");
		AddControl(m_pRandomLabel);

		m_pRandomCheckBox = new CCheckBox();
		AddControl(m_pRandomCheckBox);

		m_pCreaseLabel = new CLabel(0, 0, 32, 32, "Crease edges");
		AddControl(m_pCreaseLabel);

		m_pCreaseCheckBox = new CCheckBox();
		AddControl(m_pCreaseCheckBox);

		m_pGroundOcclusionLabel = new CLabel(0, 0, 32, 32, "Ground occlusion");
		AddControl(m_pGroundOcclusionLabel);

		m_pGroundOcclusionCheckBox = new CCheckBox();
		AddControl(m_pGroundOcclusionCheckBox);
	}

	m_pGenerate = new CButton(0, 0, 100, 100, "Generate");
	AddControl(m_pGenerate);

	m_pGenerate->SetClickedListener(this, Generate);

	m_pSave = new CButton(0, 0, 100, 100, "Save Map");
	AddControl(m_pSave);

	m_pSave->SetClickedListener(this, SaveMap);
	m_pSave->SetVisible(false);

	Layout();
}

void CAOPanel::Layout()
{
	int iSpace = 20;

	m_pSizeLabel->EnsureTextFits();

	int iSelectorSize = m_pSizeLabel->GetHeight() - 4;

	m_pSizeSelector->SetSize(GetWidth() - m_pSizeLabel->GetWidth() - iSpace, iSelectorSize);

	int iControlY = HEADER_HEIGHT;

	m_pSizeSelector->SetPos(GetWidth() - m_pSizeSelector->GetWidth() - iSpace/2, iControlY);
	m_pSizeLabel->SetPos(5, iControlY);

	iControlY += 30;

	m_pEdgeBleedLabel->EnsureTextFits();
	m_pEdgeBleedLabel->SetPos(5, iControlY);

	m_pEdgeBleedSelector->SetSize(GetWidth() - m_pEdgeBleedLabel->GetWidth() - iSpace, iSelectorSize);
	m_pEdgeBleedSelector->SetPos(GetWidth() - m_pEdgeBleedSelector->GetWidth() - iSpace/2, iControlY);

	if (!m_bColor)
	{
		iControlY += 30;

		m_pAOMethodLabel->EnsureTextFits();
		m_pAOMethodLabel->SetPos(5, iControlY);

		m_pAOMethodSelector->SetSize(GetWidth() - m_pAOMethodLabel->GetWidth() - iSpace, iSelectorSize);
		m_pAOMethodSelector->SetPos(GetWidth() - m_pAOMethodSelector->GetWidth() - iSpace/2, iControlY);

		bool bRaytracing = (m_pAOMethodSelector->GetSelectionValue() == AOMETHOD_RAYTRACE);
		m_pRayDensityLabel->SetVisible(bRaytracing);
		m_pRayDensitySelector->SetVisible(bRaytracing);

		m_pFalloffSelector->SetVisible(bRaytracing);
		m_pFalloffLabel->SetVisible(bRaytracing);

		m_pRandomCheckBox->SetVisible(bRaytracing);
		m_pRandomLabel->SetVisible(bRaytracing);

		if (bRaytracing)
		{
			iControlY += 30;

			m_pRayDensityLabel->EnsureTextFits();
			m_pRayDensityLabel->SetPos(5, iControlY);

			m_pRayDensitySelector->SetSize(GetWidth() - m_pRayDensityLabel->GetWidth() - iSpace, iSelectorSize);
			m_pRayDensitySelector->SetPos(GetWidth() - m_pRayDensitySelector->GetWidth() - iSpace/2, iControlY);

			iControlY += 30;

			m_pFalloffLabel->EnsureTextFits();
			m_pFalloffLabel->SetPos(5, iControlY);

			m_pFalloffSelector->SetSize(GetWidth() - m_pFalloffLabel->GetWidth() - iSpace, iSelectorSize);
			m_pFalloffSelector->SetPos(GetWidth() - m_pFalloffSelector->GetWidth() - iSpace/2, iControlY);

			iControlY += 30;

			m_pRandomLabel->EnsureTextFits();
			m_pRandomLabel->SetPos(25, iControlY);

			m_pRandomCheckBox->SetPos(10, iControlY + m_pRandomLabel->GetHeight()/2 - m_pRandomCheckBox->GetHeight()/2);
		}

		bool bShadowmapping = (m_pAOMethodSelector->GetSelectionValue() == AOMETHOD_SHADOWMAP);
		m_pLightsLabel->SetVisible(bShadowmapping);
		m_pLightsSelector->SetVisible(bShadowmapping);

		if (bShadowmapping)
		{
			iControlY += 30;

			m_pLightsLabel->EnsureTextFits();
			m_pLightsLabel->SetPos(5, iControlY);

			m_pLightsSelector->SetSize(GetWidth() - m_pLightsLabel->GetWidth() - iSpace, iSelectorSize);
			m_pLightsSelector->SetPos(GetWidth() - m_pLightsSelector->GetWidth() - iSpace/2, iControlY);
		}

		m_pGroundOcclusionLabel->SetVisible(bShadowmapping || bRaytracing);
		m_pGroundOcclusionCheckBox->SetVisible(bShadowmapping || bRaytracing);

		if (bShadowmapping || bRaytracing)
		{
			// If we're on the raytracing screen there was already the Randomize rays checkbox
			// so keep the spacing smaller.
			if (bRaytracing)
				iControlY += 20;
			else
				iControlY += 30;

			m_pGroundOcclusionLabel->EnsureTextFits();
			m_pGroundOcclusionLabel->SetPos(25, iControlY);

			m_pGroundOcclusionCheckBox->SetPos(10, iControlY + m_pGroundOcclusionLabel->GetHeight()/2 - m_pGroundOcclusionCheckBox->GetHeight()/2);
		}

		if (bShadowmapping || bRaytracing)
			iControlY += 20;
		else
			iControlY += 30;

		m_pCreaseLabel->EnsureTextFits();
		m_pCreaseLabel->SetPos(25, iControlY);

		m_pCreaseCheckBox->SetPos(10, iControlY + m_pCreaseLabel->GetHeight()/2 - m_pCreaseCheckBox->GetHeight()/2);
	}

	m_pSave->SetSize(GetWidth()/2, GetWidth()/6);
	m_pSave->SetPos(GetWidth()/4, GetHeight() - (int)(m_pSave->GetHeight()*1.5f));
	m_pSave->SetVisible(m_oGenerator.DoneGenerating());

	m_pGenerate->SetSize(GetWidth()/2, GetWidth()/6);
	m_pGenerate->SetPos(GetWidth()/4, GetHeight() - (int)(m_pSave->GetHeight()*1.5f) - (int)(m_pGenerate->GetHeight()*1.5f));

	CMovablePanel::Layout();
}

bool CAOPanel::KeyPressed(int iKey)
{
	if (iKey == 27 && m_oGenerator.IsGenerating())
	{
		m_pSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return true;
	}
	else
		return CMovablePanel::KeyPressed(iKey);
}

void CAOPanel::GenerateCallback()
{
	if (m_oGenerator.IsGenerating())
	{
		m_pSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return;
	}

	m_pSave->SetVisible(false);

	// Switch over to UV mode so we can see our progress.
	CModelWindow::Get()->SetRenderMode(true);

	// If the 3d model was there get rid of it.
	CModelWindow::Get()->Render();
	CRootPanel::Get()->Paint(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());
	glutSwapBuffers();
	CModelWindow::Get()->Render();
	CRootPanel::Get()->Paint(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());
	glutSwapBuffers();

	if (m_bColor)
		CModelWindow::Get()->SetDisplayColorAO(true);
	else
		CModelWindow::Get()->SetDisplayAO(true);

	m_pGenerate->SetText("Cancel");

	int iSize = m_pSizeSelector->GetSelectionValue();
	m_oGenerator.SetMethod(m_bColor?AOMETHOD_RENDER:(aomethod_t)m_pAOMethodSelector->GetSelectionValue());
	m_oGenerator.SetSize(iSize, iSize);
	m_oGenerator.SetBleed(m_pEdgeBleedSelector->GetSelectionValue());
	m_oGenerator.SetUseTexture(true);
	m_oGenerator.SetWorkListener(this);
	if (!m_bColor)
	{
		if (m_pAOMethodSelector->GetSelectionValue() == AOMETHOD_SHADOWMAP)
			m_oGenerator.SetSamples(m_pLightsSelector->GetSelectionValue());
		else
			m_oGenerator.SetSamples(m_pRayDensitySelector->GetSelectionValue());
		m_oGenerator.SetRandomize(m_pRandomCheckBox->GetToggleState());
		m_oGenerator.SetCreaseEdges(m_pCreaseCheckBox->GetToggleState());
		m_oGenerator.SetGroundOcclusion(m_pGroundOcclusionCheckBox->GetToggleState());
		m_oGenerator.SetRayFalloff(m_pFalloffSelector->GetSelectionValue());
	}
	m_oGenerator.Generate();

	size_t iAO;
	if (m_oGenerator.DoneGenerating())
		iAO = m_oGenerator.GenerateTexture();

	for (size_t i = 0; i < m_paoMaterials->size(); i++)
	{
		size_t& iAOTexture = m_bColor?(*m_paoMaterials)[i].m_iColorAO:(*m_paoMaterials)[i].m_iAO;

		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		if (iAOTexture)
			glDeleteTextures(1, &iAOTexture);

		if (m_oGenerator.DoneGenerating())
			iAOTexture = iAO;
		else
			iAOTexture = 0;
	}

	m_pSave->SetVisible(m_oGenerator.DoneGenerating());

	m_pGenerate->SetText("Generate");
}

void CAOPanel::SaveMapCallback()
{
	if (!m_oGenerator.DoneGenerating())
		return;

	m_oGenerator.SaveToFile(CModelWindow::Get()->SaveFileDialog(L"Portable Network Graphics (.png)\0*.png\0Bitmap (.bmp)\0*.bmp\0JPEG (.jpg)\0*.jpg\0Truevision Targa (.tga)\0*.tga\0Adobe PhotoShop (.psd)\0*.psd\0"));
}

void CAOPanel::BeginProgress()
{
	CProgressBar::Get()->SetVisible(true);
}

void CAOPanel::SetAction(wchar_t* pszAction, size_t iTotalProgress)
{
	CProgressBar::Get()->SetTotalProgress(iTotalProgress);
	CProgressBar::Get()->SetAction(pszAction);
	WorkProgress(0, true);
}

void CAOPanel::WorkProgress(size_t iProgress, bool bForceDraw)
{
	static int iLastTime = 0;
	static int iLastGenerate = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && glutGet(GLUT_ELAPSED_TIME) - iLastTime < 10)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	// We need to correct the viewport size before we push any events, or else any Layout() commands during
	// button presses and the line will use the wrong viewport size.
	glViewport(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());

	glutMainLoopEvent();

	if (m_oGenerator.IsGenerating() && glutGet(GLUT_ELAPSED_TIME) - iLastGenerate > 500)
	{
		size_t iAO = m_oGenerator.GenerateTexture(true);

		for (size_t i = 0; i < m_paoMaterials->size(); i++)
		{
			size_t& iAOTexture = m_bColor?(*m_paoMaterials)[i].m_iColorAO:(*m_paoMaterials)[i].m_iAO;

			if (iAOTexture)
				glDeleteTextures(1, &iAOTexture);

			iAOTexture = iAO;
		}

		iLastGenerate = glutGet(GLUT_ELAPSED_TIME);
	}

	CModelWindow::Get()->Render();
	CRootPanel::Get()->Think();
	CRootPanel::Get()->Paint(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());
	glutSwapBuffers();

	iLastTime = glutGet(GLUT_ELAPSED_TIME);
}

void CAOPanel::EndProgress()
{
	CProgressBar::Get()->SetVisible(false);
}

void CAOPanel::FindBestRayFalloff()
{
	if (!m_pScene->GetNumMeshes())
		return;

	if (!m_bColor)
		m_pFalloffSelector->SetSelection(m_pFalloffSelector->FindClosestSelectionValue(m_pScene->m_oExtends.Size().Length()/2));
}

void CAOPanel::Open(bool bColor, CConversionScene* pScene, std::vector<CMaterial>* paoMaterials)
{
	CAOPanel* pPanel = Get(bColor);

	// Get rid of the last one, in case we've changed the scene.
	if (pPanel)
		delete pPanel;

	if (bColor)
		s_pColorAOPanel = new CAOPanel(true, pScene, paoMaterials);
	else
		s_pAOPanel = new CAOPanel(false, pScene, paoMaterials);

	pPanel = Get(bColor);

	if (!pPanel)
		return;

	pPanel->SetVisible(true);
	pPanel->Layout();

	pPanel->FindBestRayFalloff();
}

CAOPanel* CAOPanel::Get(bool bColor)
{
	return bColor?s_pColorAOPanel:s_pAOPanel;
}

void CAOPanel::SetVisible(bool bVisible)
{
	m_oGenerator.StopGenerating();

	CMovablePanel::SetVisible(bVisible);
}

void CAOPanel::AOMethodCallback()
{
	// So we can appear/disappear the ray density bar if the AO method has changed.
	Layout();
}

CNormalPanel* CNormalPanel::s_pNormalPanel = NULL;

CNormalPanel::CNormalPanel(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials)
	: CMovablePanel("Normal map generator"), m_oGenerator(pScene, paoMaterials)
{
	m_pScene = pScene;
	m_paoMaterials = paoMaterials;

	m_pMeshInstancePicker = NULL;

	SetSize(400, 450);
	SetPos(GetParent()->GetWidth() - GetWidth() - 50, GetParent()->GetHeight() - GetHeight() - 100);

	m_pSizeLabel = new CLabel(0, 0, 32, 32, "Size");
	AddControl(m_pSizeLabel);

	m_pSizeSelector = new CScrollSelector<int>();
#ifdef _DEBUG
	m_pSizeSelector->AddSelection(CScrollSelection<int>(16, L"16x16"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(32, L"32x32"));
#endif
	m_pSizeSelector->AddSelection(CScrollSelection<int>(64, L"64x64"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(128, L"128x128"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(256, L"256x256"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(512, L"512x512"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(1024, L"1024x1024"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(2048, L"2048x2048"));
	m_pSizeSelector->AddSelection(CScrollSelection<int>(4096, L"4096x4096"));
	m_pSizeSelector->SetSelection(4);
	AddControl(m_pSizeSelector);

	m_pLoResLabel = new CLabel(0, 0, 32, 32, "Low Resolution Meshes");
	AddControl(m_pLoResLabel);

	m_pLoRes = new CTree(CModelWindow::Get()->GetArrowTexture(), CModelWindow::Get()->GetEditTexture(), CModelWindow::Get()->GetVisibilityTexture());
	m_pLoRes->SetBackgroundColor(g_clrBox);
	AddControl(m_pLoRes);

	m_pHiResLabel = new CLabel(0, 0, 32, 32, "High Resolution Meshes");
	AddControl(m_pHiResLabel);

	m_pHiRes = new CTree(CModelWindow::Get()->GetArrowTexture(), CModelWindow::Get()->GetEditTexture(), CModelWindow::Get()->GetVisibilityTexture());
	m_pHiRes->SetBackgroundColor(g_clrBox);
	AddControl(m_pHiRes);

	m_pAddLoRes = new CButton(0, 0, 100, 100, "Add");
	m_pAddLoRes->SetClickedListener(this, AddLoRes);
	AddControl(m_pAddLoRes);

	m_pAddHiRes = new CButton(0, 0, 100, 100, "Add");
	m_pAddHiRes->SetClickedListener(this, AddHiRes);
	AddControl(m_pAddHiRes);

	m_pRemoveLoRes = new CButton(0, 0, 100, 100, "Remove");
	m_pRemoveLoRes->SetClickedListener(this, RemoveLoRes);
	AddControl(m_pRemoveLoRes);

	m_pRemoveHiRes = new CButton(0, 0, 100, 100, "Remove");
	m_pRemoveHiRes->SetClickedListener(this, RemoveHiRes);
	AddControl(m_pRemoveHiRes);

	m_pTextureCheckBox = new CCheckBox();
	m_pTextureCheckBox->SetClickedListener(this, UpdateNormal2);
	m_pTextureCheckBox->SetUnclickedListener(this, UpdateNormal2);
	AddControl(m_pTextureCheckBox);

	m_pTextureLabel = new CLabel(0, 0, 100, 100, "Use texture");
	AddControl(m_pTextureLabel);

	m_pHiDepthLabel = new CLabel(0, 0, 32, 32, "Texture Hi-Res Depth");
	AddControl(m_pHiDepthLabel);

	m_pHiDepthSelector = new CScrollSelector<float>();
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.0f, L"0%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.1f, L"10%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.2f, L"20%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.3f, L"30%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.4f, L"40%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.5f, L"50%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.6f, L"60%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.7f, L"70%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.8f, L"80%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(0.9f, L"90%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.0f, L"100%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.1f, L"110%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.2f, L"120%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.3f, L"130%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.4f, L"140%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.5f, L"150%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.6f, L"160%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.7f, L"170%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.8f, L"180%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(1.9f, L"190%"));
	m_pHiDepthSelector->AddSelection(CScrollSelection<float>(2.0f, L"200%"));
	m_pHiDepthSelector->SetSelection(10);
	m_pHiDepthSelector->SetSelectedListener(this, UpdateNormal2);
	AddControl(m_pHiDepthSelector);

	m_pLoDepthLabel = new CLabel(0, 0, 32, 32, "Texture Lo-Res Depth");
	AddControl(m_pLoDepthLabel);

	m_pLoDepthSelector = new CScrollSelector<float>();
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.0f, L"0%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.1f, L"10%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.2f, L"20%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.3f, L"30%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.4f, L"40%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.5f, L"50%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.6f, L"60%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.7f, L"70%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.8f, L"80%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(0.9f, L"90%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.0f, L"100%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.1f, L"110%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.2f, L"120%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.3f, L"130%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.4f, L"140%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.5f, L"150%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.6f, L"160%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.7f, L"170%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.8f, L"180%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(1.9f, L"190%"));
	m_pLoDepthSelector->AddSelection(CScrollSelection<float>(2.0f, L"200%"));
	m_pLoDepthSelector->SetSelection(10);
	m_pLoDepthSelector->SetSelectedListener(this, UpdateNormal2);
	AddControl(m_pLoDepthSelector);

	m_pGenerate = new CButton(0, 0, 100, 100, "Generate");
	m_pGenerate->SetClickedListener(this, Generate);
	AddControl(m_pGenerate);

	m_pSave = new CButton(0, 0, 100, 100, "Save Map");
	AddControl(m_pSave);

	m_pSave->SetClickedListener(this, SaveMap);
	m_pSave->SetVisible(false);

	Layout();
}

void CNormalPanel::Layout()
{
	int iSpace = 20;

	m_pSizeLabel->EnsureTextFits();

	int iSelectorSize = m_pSizeLabel->GetHeight() - 4;

	m_pSizeSelector->SetSize(GetWidth() - m_pSizeLabel->GetWidth() - iSpace, iSelectorSize);

	int iControlY = HEADER_HEIGHT;

	m_pSizeSelector->SetPos(GetWidth() - m_pSizeSelector->GetWidth() - iSpace/2, iControlY);
	m_pSizeLabel->SetPos(5, iControlY);

	int iTreeWidth = GetWidth()/2-15;

	m_pLoResLabel->EnsureTextFits();
	m_pLoResLabel->SetPos(10, 40);

	m_pLoRes->SetSize(iTreeWidth, 150);
	m_pLoRes->SetPos(10, 70);

	m_pAddLoRes->SetSize(40, 20);
	m_pAddLoRes->SetPos(10, 225);

	m_pRemoveLoRes->SetSize(60, 20);
	m_pRemoveLoRes->SetPos(60, 225);

	m_pHiResLabel->EnsureTextFits();
	m_pHiResLabel->SetPos(iTreeWidth+20, 40);

	m_pHiRes->SetSize(iTreeWidth, 150);
	m_pHiRes->SetPos(iTreeWidth+20, 70);

	m_pAddHiRes->SetSize(40, 20);
	m_pAddHiRes->SetPos(iTreeWidth+20, 225);

	m_pRemoveHiRes->SetSize(60, 20);
	m_pRemoveHiRes->SetPos(iTreeWidth+70, 225);

	m_pTextureLabel->SetPos(35, 220);
	m_pTextureLabel->EnsureTextFits();
	m_pTextureLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pTextureLabel->SetWrap(false);
	m_pTextureCheckBox->SetPos(20, 220 + m_pTextureLabel->GetHeight()/2 - m_pTextureCheckBox->GetHeight()/2);

	m_pHiDepthLabel->SetPos(10, 280);
	m_pHiDepthLabel->EnsureTextFits();
	m_pHiDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pHiDepthLabel->SetWrap(false);
	m_pHiDepthSelector->SetPos(m_pHiDepthLabel->GetRight() + 10, 280 + m_pHiDepthLabel->GetHeight()/2 - m_pHiDepthSelector->GetHeight()/2);
	m_pHiDepthSelector->SetRight(GetWidth() - 10);

	m_pLoDepthLabel->SetPos(10, 300);
	m_pLoDepthLabel->EnsureTextFits();
	m_pLoDepthLabel->SetAlign(CLabel::TA_LEFTCENTER);
	m_pLoDepthLabel->SetWrap(false);
	m_pLoDepthSelector->SetPos(m_pLoDepthLabel->GetRight() + 10, 300 + m_pLoDepthLabel->GetHeight()/2 - m_pLoDepthSelector->GetHeight()/2);
	m_pLoDepthSelector->SetRight(GetWidth() - 10);

	m_pSave->SetSize(100, 33);
	m_pSave->SetPos(GetWidth() - m_pSave->GetWidth() - (int)(m_pSave->GetHeight()*0.5f), GetHeight() - (int)(m_pSave->GetHeight()*1.5f));
	m_pSave->SetVisible(m_oGenerator.DoneGenerating());

	m_pGenerate->SetSize(100, 33);
	m_pGenerate->SetPos(GetWidth() - m_pGenerate->GetWidth() - (int)(m_pGenerate->GetHeight()*0.5f), GetHeight() - (int)(m_pSave->GetHeight()*1.5f) - (int)(m_pGenerate->GetHeight()*1.5f));

	size_t i;
	m_pLoRes->ClearTree();
	if (!m_apLoResMeshes.size())
		m_pLoRes->AddNode(L"No meshes.");
	else
	{
		for (i = 0; i < m_apLoResMeshes.size(); i++)
		{
			m_pLoRes->AddNode<CConversionMeshInstance>(m_apLoResMeshes[i]->GetMesh()->GetName(), m_apLoResMeshes[i]);
			m_pLoRes->GetNode(i)->SetIcon(CModelWindow::Get()->GetMeshesNodeTexture());
		}
	}

	m_pHiRes->ClearTree();
	if (!m_apHiResMeshes.size())
		m_pHiRes->AddNode(L"No meshes.");
	else
	{
		for (i = 0; i < m_apHiResMeshes.size(); i++)
		{
			m_pHiRes->AddNode<CConversionMeshInstance>(m_apHiResMeshes[i]->GetMesh()->GetName(), m_apHiResMeshes[i]);
			m_pHiRes->GetNode(i)->SetIcon(CModelWindow::Get()->GetMeshesNodeTexture());
		}
	}

	CMovablePanel::Layout();
}

void CNormalPanel::UpdateScene()
{
	m_apLoResMeshes.clear();
	m_apHiResMeshes.clear();
	m_pTextureCheckBox->SetState(false, false);
}

void CNormalPanel::Think()
{
	if (m_oGenerator.IsNewNormal2Available())
	{
		size_t iNormal2 = m_oGenerator.GetNormalMap2();

		for (size_t i = 0; i < m_paoMaterials->size(); i++)
		{
			size_t& iNormalTexture = (*m_paoMaterials)[i].m_iNormal2;

			if (!m_pScene->GetMaterial(i)->IsVisible())
				continue;

			if (iNormalTexture)
				glDeleteTextures(1, &iNormalTexture);

			iNormalTexture = iNormal2;
			break;
		}

		m_pSave->SetVisible(!!iNormal2);
	}

	m_pTextureLabel->SetText("Use texture");
	if (m_oGenerator.IsGeneratingNewNormal2())
	{
		m_pTextureLabel->AppendText(" (Generating... ");
		char szPercentage[10];
		sprintf(szPercentage, "%d", (int)(m_oGenerator.GetNormal2GenerationProgress()*100));
		m_pTextureLabel->AppendText(szPercentage);
		m_pTextureLabel->AppendText("%)");
		m_pSave->SetVisible(false);
	}

	CMovablePanel::Think();
}

void CNormalPanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);
}

bool CNormalPanel::KeyPressed(int iKey)
{
	if (iKey == 27 && m_oGenerator.IsGenerating())
	{
		m_pSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return true;
	}
	else
		return CMovablePanel::KeyPressed(iKey);
}

void CNormalPanel::GenerateCallback()
{
	if (m_oGenerator.IsGenerating())
	{
		m_pSave->SetVisible(false);
		m_oGenerator.StopGenerating();
		return;
	}

	m_pSave->SetVisible(false);

	m_pGenerate->SetText("Cancel");

	CModelWindow::Get()->SetDisplayNormal(true);

	// Disappear all of the hi-res meshes so we can see the lo res better.
	for (size_t m = 0; m < m_apHiResMeshes.size(); m++)
		m_apHiResMeshes[m]->SetVisible(false);

	int iSize = m_pSizeSelector->GetSelectionValue();
	m_oGenerator.SetSize(iSize, iSize);
	m_oGenerator.SetModels(m_apHiResMeshes, m_apLoResMeshes);
	m_oGenerator.SetWorkListener(this);
	m_oGenerator.Generate();

	size_t iNormal = 0;
	if (m_oGenerator.DoneGenerating())
		iNormal = m_oGenerator.GenerateTexture();

	for (size_t i = 0; i < m_paoMaterials->size(); i++)
	{
		size_t& iNormalTexture =(*m_paoMaterials)[i].m_iNormal;

		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		if (iNormalTexture)
			glDeleteTextures(1, &iNormalTexture);

		if (m_oGenerator.DoneGenerating())
			iNormalTexture = iNormal;
		else
			iNormalTexture = 0;
	}

	m_pSave->SetVisible(m_oGenerator.DoneGenerating());

	m_pGenerate->SetText("Generate");
}

void CNormalPanel::SaveMapCallback()
{
	if (!m_oGenerator.DoneGenerating())
		return;

	const wchar_t* pszFilename = CModelWindow::Get()->SaveFileDialog(L"Portable Network Graphics (.png)\0*.png\0Bitmap (.bmp)\0*.bmp\0JPEG (.jpg)\0*.jpg\0Truevision Targa (.tga)\0*.tga\0Adobe PhotoShop (.psd)\0*.psd\0");

	if (!pszFilename)
		return;

	m_oGenerator.SaveToFile(pszFilename);

	for (size_t i = 0; i < m_paoMaterials->size(); i++)
	{
		if (!m_pScene->GetMaterial(i)->IsVisible())
			continue;

		m_pScene->GetMaterial(i)->m_sNormalTexture = pszFilename;
	}

	CRootPanel::Get()->Layout();
}

void CNormalPanel::BeginProgress()
{
	CProgressBar::Get()->SetVisible(true);
}

void CNormalPanel::SetAction(wchar_t* pszAction, size_t iTotalProgress)
{
	CProgressBar::Get()->SetTotalProgress(iTotalProgress);
	CProgressBar::Get()->SetAction(pszAction);
	WorkProgress(0, true);
}

void CNormalPanel::WorkProgress(size_t iProgress, bool bForceDraw)
{
	static int iLastTime = 0;
	static int iLastGenerate = 0;

	// Don't update too often or it'll slow us down just because of the updates.
	if (!bForceDraw && glutGet(GLUT_ELAPSED_TIME) - iLastTime < 10)
		return;

	CProgressBar::Get()->SetProgress(iProgress);

	// We need to correct the viewport size before we push any events, or else any Layout() commands during
	// button presses and the line will use the wrong viewport size.
	glViewport(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());

	glutMainLoopEvent();

	if (m_oGenerator.IsGenerating() && glutGet(GLUT_ELAPSED_TIME) - iLastGenerate > 500)
	{
		size_t iNormal = m_oGenerator.GenerateTexture(true);

		for (size_t i = 0; i < m_paoMaterials->size(); i++)
		{
			size_t& iNormalTexture = (*m_paoMaterials)[i].m_iNormal;

			if (iNormalTexture)
				glDeleteTextures(1, &iNormalTexture);

			iNormalTexture = iNormal;
		}

		iLastGenerate = glutGet(GLUT_ELAPSED_TIME);
	}

	CModelWindow::Get()->Render();
	CRootPanel::Get()->Think();
	CRootPanel::Get()->Paint(0, 0, CModelWindow::Get()->GetWindowWidth(), CModelWindow::Get()->GetWindowHeight());
	glutSwapBuffers();

	iLastTime = glutGet(GLUT_ELAPSED_TIME);
}

void CNormalPanel::EndProgress()
{
	CProgressBar::Get()->SetVisible(false);
}

void CNormalPanel::AddLoResCallback()
{
	if (m_pMeshInstancePicker)
		delete m_pMeshInstancePicker;

	m_pMeshInstancePicker = new CMeshInstancePicker(this, AddLoResMesh);

	int x, y, w, h, pw, ph;
	GetAbsDimensions(x, y, w, h);
	m_pMeshInstancePicker->GetSize(pw, ph);
	m_pMeshInstancePicker->SetPos(x + w/2 - pw/2, y + h/2 - ph/2);
}

void CNormalPanel::AddHiResCallback()
{
	if (m_pMeshInstancePicker)
		delete m_pMeshInstancePicker;

	m_pMeshInstancePicker = new CMeshInstancePicker(this, AddHiResMesh);

	int x, y, w, h, pw, ph;
	GetAbsDimensions(x, y, w, h);
	m_pMeshInstancePicker->GetSize(pw, ph);
	m_pMeshInstancePicker->SetPos(x + w/2 - pw/2, y + h/2 - ph/2);
}

void CNormalPanel::AddLoResMeshCallback()
{
	CConversionMeshInstance* pMeshInstance = m_pMeshInstancePicker->GetPickedMeshInstance();
	if (!pMeshInstance)
		return;

	bool bFound = false;
	size_t i;
	for (i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshInstance)
			bFound = true;

	for (i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshInstance)
			bFound = true;

	if (!bFound)
		m_apLoResMeshes.push_back(pMeshInstance);

	delete m_pMeshInstancePicker;
	m_pMeshInstancePicker = NULL;

	Layout();

	UpdateNormal2Callback();
}

void CNormalPanel::AddHiResMeshCallback()
{
	CConversionMeshInstance* pMeshInstance = m_pMeshInstancePicker->GetPickedMeshInstance();
	if (!pMeshInstance)
		return;

	bool bFound = false;
	size_t i;
	for (i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshInstance)
			bFound = true;

	for (i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshInstance)
			bFound = true;

	if (!bFound)
		m_apHiResMeshes.push_back(pMeshInstance);

	delete m_pMeshInstancePicker;
	m_pMeshInstancePicker = NULL;

	Layout();
}

void CNormalPanel::RemoveLoResCallback()
{
	CTreeNode* pNode = m_pLoRes->GetSelectedNode();
	if (!pNode)
		return;

	CTreeNodeObject<CConversionMeshInstance>* pMeshNode = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pNode);
	if (!pMeshNode)
		return;

	for (size_t i = 0; i < m_apLoResMeshes.size(); i++)
		if (m_apLoResMeshes[i] == pMeshNode->GetObject())
			m_apLoResMeshes.erase(m_apLoResMeshes.begin()+i);

	Layout();

	UpdateNormal2Callback();
}

void CNormalPanel::RemoveHiResCallback()
{
	CTreeNode* pNode = m_pHiRes->GetSelectedNode();
	if (!pNode)
		return;

	CTreeNodeObject<CConversionMeshInstance>* pMeshNode = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pNode);
	if (!pMeshNode)
		return;

	for (size_t i = 0; i < m_apHiResMeshes.size(); i++)
		if (m_apHiResMeshes[i] == pMeshNode->GetObject())
			m_apHiResMeshes.erase(m_apHiResMeshes.begin()+i);

	Layout();
}

void CNormalPanel::UpdateNormal2Callback()
{
	m_oGenerator.SetModels(m_apHiResMeshes, m_apLoResMeshes);

	m_oGenerator.SetNormalTextureHiDepth(m_pHiDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTextureLoDepth(m_pLoDepthSelector->GetSelectionValue());
	m_oGenerator.SetNormalTexture(m_pTextureCheckBox->GetState());

	if (!m_pTextureCheckBox->GetState())
		m_pSave->SetVisible(false);
}

void CNormalPanel::Open(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials)
{
	CNormalPanel* pPanel = s_pNormalPanel;

	if (pPanel)
		delete pPanel;

	pPanel = s_pNormalPanel = new CNormalPanel(pScene, paoMaterials);

	if (!pPanel)
		return;

	pPanel->SetVisible(true);
	pPanel->Layout();
}

void CNormalPanel::SetVisible(bool bVisible)
{
	m_oGenerator.StopGenerating();

	CMovablePanel::SetVisible(bVisible);
}

CHelpPanel* CHelpPanel::s_pHelpPanel = NULL;

CHelpPanel::CHelpPanel()
	: CMovablePanel("Help")
{
	m_pInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pInfo);
	Layout();
}

void CHelpPanel::Layout()
{
	if (GetParent())
	{
		int px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(600, 200);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_pInfo->SetSize(GetWidth(), GetHeight());
	m_pInfo->SetPos(0, 0);

	m_pInfo->SetText("CONTROLS:\n");
	m_pInfo->AppendText("Left Mouse Button - Move the camera\n");
	m_pInfo->AppendText("Right Mouse Button - Zoom in and out\n");
	m_pInfo->AppendText("Ctrl-LMB - Rotate the light\n");
	m_pInfo->AppendText(" \n");
	m_pInfo->AppendText("For in-depth help information please visit our website, http://www.matreyastudios.com/smak\n");

	CMovablePanel::Layout();
}

void CHelpPanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);
}

bool CHelpPanel::MousePressed(int iButton, int mx, int my)
{
	if (CMovablePanel::MousePressed(iButton, mx, my))
		return true;

	Close();

	return false;
}

void CHelpPanel::Open()
{
	if (!s_pHelpPanel)
		s_pHelpPanel = new CHelpPanel();

	s_pHelpPanel->SetVisible(true);
	s_pHelpPanel->Layout();
}

void CHelpPanel::Close()
{
	if (!s_pHelpPanel)
		return;

	s_pHelpPanel->SetVisible(false);
}

CAboutPanel* CAboutPanel::s_pAboutPanel = NULL;

CAboutPanel::CAboutPanel()
	: CMovablePanel("About SMAK")
{
	m_pInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pInfo);
	Layout();
}

void CAboutPanel::Layout()
{
	if (GetParent())
	{
		int px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(600, 250);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_pInfo->SetSize(GetWidth(), GetHeight());
	m_pInfo->SetPos(0, 0);

	m_pInfo->SetText("SMAK - The Super Model Army Knife\n");
	m_pInfo->AppendText("Version " SMAK_VERSION "\n");
	m_pInfo->AppendText("Copyright © 2010, Jorge Rodriguez <jrodriguez@matreyastudios.com>\n");
	m_pInfo->AppendText(" \n");
	m_pInfo->AppendText("FCollada copyright © 2006, Feeling Software\n");
	m_pInfo->AppendText("DevIL copyright © 2001-2009, Denton Woods\n");
	m_pInfo->AppendText("FTGL copyright © 2001-2003 Henry Maddocks\n");
	m_pInfo->AppendText("Freeglut copyright © 1999-2000, Pawel W. Olszta\n");
	m_pInfo->AppendText("pthreads-win32 copyright © 2001, 2006 Ross P. Johnson\n");
	m_pInfo->AppendText("GLEW copyright © 2002-2007, Milan Ikits, Marcelo E. Magallon, Lev Povalahev\n");
	m_pInfo->AppendText("Freetype copyright © 1996-2002, 2006 by David Turner, Robert Wilhelm, and Werner Lemberg\n");

	CMovablePanel::Layout();
}

void CAboutPanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);
}

bool CAboutPanel::MousePressed(int iButton, int mx, int my)
{
	if (CMovablePanel::MousePressed(iButton, mx, my))
		return true;

	Close();

	return false;
}

void CAboutPanel::Open()
{
	if (!s_pAboutPanel)
		s_pAboutPanel = new CAboutPanel();

	s_pAboutPanel->SetVisible(true);
	s_pAboutPanel->Layout();
}

void CAboutPanel::Close()
{
	if (!s_pAboutPanel)
		return;

	s_pAboutPanel->SetVisible(false);
}

CRegisterPanel* CRegisterPanel::s_pRegisterPanel = NULL;

CRegisterPanel::CRegisterPanel()
	: CMovablePanel("Register SMAK")
{
	m_pInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pInfo);

	m_pPirates = new CButton(0, 0, 1, 1, "A message to pirates");
	m_pPirates->SetClickedListener(this, Pirates);
	AddControl(m_pPirates);

	m_bBadTry = false;

	Layout();
}

void CRegisterPanel::Layout()
{
	if (GetParent())
	{
		int px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(600, 300);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_pInfo->SetAlign(CLabel::TA_TOPCENTER);
	m_pInfo->SetSize(GetWidth(), GetHeight()-40);
	m_pInfo->SetPos(0, 40);

	if (!CModelWindow::Get()->GetSMAKTexture())
	{
		m_pInfo->SetText("Steps to Register SMAK\n");
		m_pInfo->AppendText(" \n");
		m_pInfo->AppendText("1. Visit http://www.matreyastudios.com/smak and purchase SMAK using the on-site store.\n");
		m_pInfo->AppendText("2. You will be sent an unlock key in your email. Select it and use the COPY command (Ctrl-c)\n");
		m_pInfo->AppendText("3. While this window is open, use the PASTE command (Ctrl-v)\n");
		m_pInfo->AppendText("4. ...?\n");
		m_pInfo->AppendText("5. Profit!\n");

		m_pInfo->AppendText(" \n");
		m_pInfo->AppendText(" \n");

		m_pInfo->AppendText("Your product code is: ");
		m_pInfo->AppendText(CModelWindow::Get()->GetSMAKTextureCode().c_str());
		m_pInfo->AppendText("\n");
		m_pInfo->AppendText("Use the COPY command (Ctrl-c) to move it to the clipboard.\n");

		if (m_bBadTry)
		{
			m_pInfo->AppendText(" \n");
			m_pInfo->AppendText("Sorry, it seems that didn't work. Please try again.\n");
			m_pInfo->AppendText("Make sure you copy only the key itself, without any quotes or other text.\n");
		}
	}
	else
	{
		m_pInfo->SetText("Your installation of SMAK is now fully registered. Thanks!\n");
	}

	m_pPirates->EnsureTextFits();
	m_pPirates->SetDimensions(GetWidth() - m_pPirates->GetWidth() - 5, GetHeight() - m_pPirates->GetHeight() - 5, m_pPirates->GetWidth(), m_pPirates->GetHeight());

	CMovablePanel::Layout();
}

void CRegisterPanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);
}

bool CRegisterPanel::MousePressed(int iButton, int mx, int my)
{
	if (CMovablePanel::MousePressed(iButton, mx, my))
		return true;

	return false;
}

bool CRegisterPanel::KeyPressed(int iKey)
{
	if (!CModelWindow::Get()->GetSMAKTexture() && (iKey == 'v' || ((glutGetModifiers()&GLUT_ACTIVE_CTRL) && iKey == 'v'-'a'+1)))
	{
		std::string sClipboard = trim(CModelWindow::GetClipboard());
		CModelWindow::Get()->SetSMAKTexture(sClipboard.c_str());
		m_bBadTry = true;
		Layout();
		return true;
	}
	else if (!CModelWindow::Get()->GetSMAKTexture() && (iKey == 'c' || ((glutGetModifiers()&GLUT_ACTIVE_CTRL) && iKey == 'c'-'a'+1)))
	{
		CModelWindow::SetClipboard(CModelWindow::Get()->GetSMAKTextureCode());
		return true;
	}
	else
		return CMovablePanel::KeyPressed(iKey);
}

void CRegisterPanel::PiratesCallback()
{
	CPiratesPanel::Open();
	Close();
}

void CRegisterPanel::Open()
{
	if (!s_pRegisterPanel)
		s_pRegisterPanel = new CRegisterPanel();

	s_pRegisterPanel->m_bBadTry = false;
	s_pRegisterPanel->SetVisible(true);
	s_pRegisterPanel->Layout();
}

void CRegisterPanel::Close()
{
	if (!s_pRegisterPanel)
		return;

	s_pRegisterPanel->SetVisible(false);
}

CPiratesPanel* CPiratesPanel::s_pPiratesPanel = NULL;

CPiratesPanel::CPiratesPanel()
	: CMovablePanel("A Message to Software Pirates")
{
	m_pInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pInfo, true);
	Layout();
}

void CPiratesPanel::Layout()
{
	if (GetParent())
	{
		int px, py, pw, ph;
		GetParent()->GetAbsDimensions(px, py, pw, ph);

		SetSize(800, 350);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_pInfo->SetSize(GetWidth()-10, GetHeight()-10);
	m_pInfo->SetPos(5, 5);
	m_pInfo->SetAlign(CLabel::TA_LEFTCENTER);

	m_pInfo->SetText("Dear pirates,\n");
	m_pInfo->AppendText(" \n");
	m_pInfo->AppendText("I know who you are. I know where you live. I'm coming for you.\n");
	m_pInfo->AppendText(" \n");
	m_pInfo->AppendText("Just kidding.\n");
	m_pInfo->AppendText(" \n");
	m_pInfo->AppendText("Obviously there's nothing I can do to keep a determined person from pirating my software. I know how these things work, I've pirated plenty of things myself. All I can do is ask you sincerely, if you pirate this software and you are able to use it and enjoy it, please pay for it. It's not a very expensive tool. I worked very hard on this thing (Almost as hard as you did to pirate it!) and all I'm trying to do is exchange a hard day's work for a couple of well-earned bones.\n");
	m_pInfo->AppendText(" \n");
	m_pInfo->AppendText("I'm not some big corporation trying to screw its customers. I'm not an industry assocation who will come after you if you download one thing. I'm not the man trying to hold you down, bro. I'm just a guy who's trying to make a living doing something that he loves. I don't have a BMW or a Rolex. I'm just a dude trying to feed his dog.\n");
	m_pInfo->AppendText(" \n");
	m_pInfo->AppendText("So please, when you pirate this software, think of the dog.\n");
	m_pInfo->AppendText(" \n");
	m_pInfo->AppendText("Jorge Rodriguez <jrodriguez@matreyastudios.com>\n");

	CMovablePanel::Layout();
}

void CPiratesPanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);
}

bool CPiratesPanel::MousePressed(int iButton, int mx, int my)
{
	if (CMovablePanel::MousePressed(iButton, mx, my))
		return true;

	Close();

	return false;
}

void CPiratesPanel::Open()
{
	if (!s_pPiratesPanel)
		s_pPiratesPanel = new CPiratesPanel();

	s_pPiratesPanel->SetVisible(true);
	s_pPiratesPanel->Layout();
}

void CPiratesPanel::Close()
{
	if (!s_pPiratesPanel)
		return;

	s_pPiratesPanel->SetVisible(false);
}
