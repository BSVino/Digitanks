#include "modelwindow_ui.h"

#include <IL/il.h>
#include <maths.h>
#include "modelwindow.h"

void CModelWindow::InitUI()
{
	CMenu* pFile = CRootPanel::Get()->AddMenu("File");
	CMenu* pView = CRootPanel::Get()->AddMenu("View");
	CMenu* pTools = CRootPanel::Get()->AddMenu("Tools");
	CMenu* pHelp = CRootPanel::Get()->AddMenu("Help");

	pFile->AddSubmenu("Open...", this, Open);
	pFile->AddSubmenu("Reload", this, Reload);
//	pFile->AddSubmenu("Save...", this, Save);
	pFile->AddSubmenu("Close", this, Close);
	pFile->AddSubmenu("Exit", this, Exit);

	pView->AddSubmenu("3D view", this, Render3D);
	pView->AddSubmenu("UV view", this, RenderUV);
	pView->AddSubmenu("View wireframe", this, Wireframe);
	pView->AddSubmenu("View flat shaded", this, Flat);
	pView->AddSubmenu("View smooth shaded", this, Smooth);
	pView->AddSubmenu("Toggle light", this, LightToggle);
	pView->AddSubmenu("Toggle texture", this, TextureToggle);
	pView->AddSubmenu("Toggle color AO map", this, ColorAOToggle);

	pTools->AddSubmenu("Generate color AO map", this, GenerateColorAO);

	pHelp->AddSubmenu("About SMAK", this, About);

	CButtonPanel* pTopButtons = new CButtonPanel(BA_TOP);

	m_pRender3D = new CButton(0, 0, 100, 100, "3D", true);
	m_pRenderUV = new CButton(0, 0, 100, 100, "UV", true);

	pTopButtons->AddButton(m_pRender3D, false, this, Render3D);
	pTopButtons->AddButton(m_pRenderUV, false, this, RenderUV);

	CRootPanel::Get()->AddControl(pTopButtons);

	CButtonPanel* pBottomButtons = new CButtonPanel(BA_BOTTOM);

	m_pWireframe = new CButton(0, 0, 100, 100, "Wire", true);
	m_pFlat = new CButton(0, 0, 100, 100, "Flat", true);
	m_pSmooth = new CButton(0, 0, 100, 100, "Smth", true);
	m_pLight = new CButton(0, 0, 100, 100, "Lght", true);
	m_pTexture = new CButton(0, 0, 100, 100, "Tex", true);
	m_pColorAO = new CButton(0, 0, 100, 100, "C AO", true);

	pBottomButtons->AddButton(m_pWireframe, false, this, Wireframe);
	pBottomButtons->AddButton(m_pFlat, false, this, Flat);
	pBottomButtons->AddButton(m_pSmooth, true, this, Smooth);
	pBottomButtons->AddButton(m_pLight, false, this, Light);
	pBottomButtons->AddButton(m_pTexture, false, this, Texture);
	pBottomButtons->AddButton(m_pColorAO, false, this, ColorAO);

	CRootPanel::Get()->AddControl(pBottomButtons);

	CRootPanel::Get()->Layout();
}

void CModelWindow::OpenCallback()
{
	ReadFile(OpenFileDialog());
}

void CModelWindow::ReloadCallback()
{
	ReloadFromFile();
}

void CModelWindow::SaveCallback()
{
	SaveFile(SaveFileDialog(L"Valve Source SMD\0*.smd\0"));
}

void CModelWindow::CloseCallback()
{
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

void CModelWindow::WireframeCallback()
{
	SetDisplayType(DT_WIREFRAME);
}

void CModelWindow::FlatCallback()
{
	SetDisplayType(DT_FLAT);
}

void CModelWindow::SmoothCallback()
{
	SetDisplayType(DT_SMOOTH);
}

void CModelWindow::LightCallback()
{
	SetDisplayLight(m_pLight->GetState());
}

void CModelWindow::TextureCallback()
{
	SetDisplayTexture(m_pTexture->GetState());
}

void CModelWindow::ColorAOCallback()
{
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

void CModelWindow::ColorAOToggleCallback()
{
	SetDisplayColorAO(!m_bDisplayColorAO);
}

void CModelWindow::GenerateColorAOCallback()
{
	CColorAOPanel::Open(&m_Scene, &m_aoMaterials);
}

void CModelWindow::AboutCallback()
{
	OpenAboutPanel();
}

void CModelWindow::OpenAboutPanel()
{
	CAboutPanel::Open();
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

	for (size_t i = 0; i < m_apControls.size(); i++)
	{
		IControl* pControl = m_apControls[i];

		pControl->SetSize(BTN_HEIGHT, BTN_HEIGHT);
		pControl->SetPos(iX, 0);

		iX += BTN_HEIGHT + m_aiSpaces[i];
	}

	SetSize(iX - m_aiSpaces[m_aiSpaces.size()-1], BTN_HEIGHT);
	if (m_eAlign == BA_TOP)
		SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, CRootPanel::Get()->GetHeight() - BTN_HEIGHT*2 - BTN_SPACE);
	else
		SetPos(CRootPanel::Get()->GetWidth()/2 - GetWidth()/2, BTN_HEIGHT + BTN_SPACE);

	CPanel::Layout();
}

void CButtonPanel::AddButton(CButton* pButton, bool bNewSection, IEventListener* pListener, IEventListener::Callback pfnCallback)
{
	AddControl(pButton);

	m_aiSpaces.push_back(bNewSection?BTN_SECTION:BTN_SPACE);

	if (pListener)
	{
		pButton->SetClickedListener(pListener, pfnCallback);
		pButton->SetUnclickedListener(pListener, pfnCallback);
	}
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

CMovablePanel::CMovablePanel(char* pszName)
	: CPanel(0, 0, 200, 300)
{
	m_bMoving = false;

	m_pName = new CLabel(0, GetHeight()-HEADER_HEIGHT, GetWidth(), HEADER_HEIGHT, pszName);
	AddControl(m_pName);

	m_pCloseButton = new CCloseButton();
	AddControl(m_pCloseButton);

	m_pCloseButton->SetClickedListener(this, CloseWindow);

	CRootPanel::Get()->AddControl(this);
}

CMovablePanel::~CMovablePanel()
{
	CRootPanel::Get()->RemoveControl(this);
}

void CMovablePanel::Layout()
{
	m_pName->SetDimensions(0, GetHeight()-HEADER_HEIGHT, GetWidth(), HEADER_HEIGHT);

	int iButtonSize = HEADER_HEIGHT*2/3;
	m_pCloseButton->SetDimensions(GetWidth() - HEADER_HEIGHT/2 - iButtonSize/2, GetHeight() - HEADER_HEIGHT/2 - iButtonSize/2, iButtonSize, iButtonSize);

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
	CRootPanel::PaintRect(x, y, w, h, g_clrPanel);

	CRootPanel::PaintRect(x, y+h-HEADER_HEIGHT, w, HEADER_HEIGHT, g_clrBoxHi);

	CPanel::Paint(x, y, w, h);
}

bool CMovablePanel::MousePressed(int iButton, int mx, int my)
{
	int x, y;
	GetAbsPos(x, y);

	if (iButton == 0 && mx > x && mx < x + GetWidth() - HEADER_HEIGHT*3/2 && my > y + GetHeight() - HEADER_HEIGHT && my < y + GetHeight())
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

	return CPanel::MouseReleased(iButton, mx, my);
}

void CMovablePanel::CloseWindowCallback()
{
	SetVisible(false);
}

CColorAOPanel* CColorAOPanel::s_pColorAOPanel = NULL;

CColorAOPanel::CColorAOPanel(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials)
	: CMovablePanel("Color AO generator"), m_oGenerator(pScene, paoMaterials)
{
	m_pScene = pScene;
	m_paoMaterials = paoMaterials;

	SetPos(GetParent()->GetWidth() - GetWidth() - 100, GetParent()->GetHeight() - GetHeight() - 100);

	m_pGenerate = new CButton(0, 0, 100, 100, "Generate");
	AddControl(m_pGenerate);

	m_pGenerate->SetClickedListener(this, Generate);

	m_pSave = new CButton(0, 0, 100, 100, "Save Map");
	AddControl(m_pSave);

	m_pSave->SetClickedListener(this, SaveMap);

	Layout();
}

void CColorAOPanel::Layout()
{
	m_pGenerate->SetSize(GetWidth()/2, GetWidth()/6);
	m_pGenerate->SetPos(GetWidth()/4, GetWidth()/8+GetWidth()/6+10);

	m_pSave->SetSize(GetWidth()/2, GetWidth()/6);
	m_pSave->SetPos(GetWidth()/4, GetWidth()/8);

	CMovablePanel::Layout();
}

void CColorAOPanel::GenerateCallback()
{
	bool bRenderUV = CModelWindow::Get()->GetRenderMode();

	// Switch over to UV mode so we can see our progress.
	CModelWindow::Get()->SetRenderMode(true);

	// If the 3d model was there get rid of it.
	CModelWindow::Get()->Render();
	CRootPanel::Get()->Paint();
	glutSwapBuffers();
	CModelWindow::Get()->Render();
	CRootPanel::Get()->Paint();
	glutSwapBuffers();

	CModelWindow::Get()->SetDisplayColorAO(true);

	m_oGenerator.SetSize(32, 32);
	m_oGenerator.SetUseTexture(true);
	m_oGenerator.SetWorkListener(this);
	m_oGenerator.Generate();

	size_t iColorAO = m_oGenerator.GenerateTexture();
	for (size_t i = 0; i < m_paoMaterials->size(); i++)
	{
		if ((*m_paoMaterials)[i].m_iColorAO)
			glDeleteTextures(1, &(*m_paoMaterials)[i].m_iColorAO);
		(*m_paoMaterials)[i].m_iColorAO = iColorAO;
	}

	CModelWindow::Get()->CreateGLLists();

	CModelWindow::Get()->SetRenderMode(bRenderUV);
}

void CColorAOPanel::SaveMapCallback()
{
	if (!m_oGenerator.HasGenerated())
		return;

	m_oGenerator.SaveToFile(CModelWindow::Get()->SaveFileDialog(L"Bitmap (.bmp)\0*.bmp\0JPEG (.jpg)\0*.jpg\0Truevision Targa (.tga)\0*.tga\0Adobe PhotoShop (.psd)\0*.psd\0"));
}

void CColorAOPanel::WorkProgress()
{
	static int iLastTime = 0;

	int iCurrentTime = glutGet(GLUT_ELAPSED_TIME);

	// Don't update too often or it'll slow us down just because of the updates.
	if (iCurrentTime - iLastTime < 100)
		return;

	iLastTime = iCurrentTime;

	size_t iColorAO = m_oGenerator.GenerateTexture();
	for (size_t i = 0; i < m_paoMaterials->size(); i++)
	{
		if ((*m_paoMaterials)[i].m_iColorAO)
			glDeleteTextures(1, &(*m_paoMaterials)[i].m_iColorAO);
		(*m_paoMaterials)[i].m_iColorAO = iColorAO;
	}

	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);

	CModelWindow::Get()->RenderUV();
	glutSwapBuffers();
}

void CColorAOPanel::Open(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials)
{
	// Get rid of the last one, in case we've changed the scene.
	if (s_pColorAOPanel)
		delete s_pColorAOPanel;

	s_pColorAOPanel = new CColorAOPanel(pScene, paoMaterials);

	s_pColorAOPanel->SetVisible(true);
	s_pColorAOPanel->Layout();
}

void CColorAOPanel::Close()
{
	if (!s_pColorAOPanel)
		return;

	s_pColorAOPanel->SetVisible(false);
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

		SetSize(600, 200);
		SetPos(pw/2 - GetWidth()/2, ph/2 - GetHeight()/2);
	}

	m_pInfo->SetSize(GetWidth(), GetHeight());
	m_pInfo->SetPos(0, 0);

	m_pInfo->SetText("SMAK - The Super Model Army Knife\n");
	m_pInfo->AppendText("Copyright © 2009, Jorge Rodriguez <jrodriguez@matreyastudios.com>\n");
	m_pInfo->AppendText(" \n");
	//m_pInfo->AppendText("FCollada copyright © 2006, Feeling Software\n");	// Put back in when Collada support is back.
	m_pInfo->AppendText("DevIL copyright © 2001-2009, Denton Woods\n");
	m_pInfo->AppendText("FTGL copyright © 2001-2003 Henry Maddocks\n");
	m_pInfo->AppendText("Freeglut copyright © 1999-2000, Pawel W. Olszta\n");
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

	return true;
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
