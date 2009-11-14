#include "modelwindow_ui.h"

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

	pHelp->AddSubmenu("About SMAK", this, About);

	CButtonPanel* pButtons = new CButtonPanel();

	m_pWireframe = new CButton(0, 0, 100, 100, "Wire", true);
	m_pFlat = new CButton(0, 0, 100, 100, "Flat", true);
	m_pSmooth = new CButton(0, 0, 100, 100, "Smth", true);
	m_pAO = new CButton(0, 0, 100, 100, "AO", true);

	pButtons->AddButton(m_pWireframe, false, this, Wireframe);
	pButtons->AddButton(m_pFlat, false, this, Flat);
	pButtons->AddButton(m_pSmooth, true, this, Smooth);
	pButtons->AddButton(m_pAO, true, this, AO);

	CRootPanel::Get()->AddControl(pButtons);

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
	SaveFile(SaveFileDialog());
}

void CModelWindow::CloseCallback()
{
	DestroyAll();
}

void CModelWindow::ExitCallback()
{
	exit(0);
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

void CModelWindow::AOCallback()
{
	m_bDisplayAO = m_pAO->GetState();

	CreateGLLists();
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

CButtonPanel::CButtonPanel()
: CPanel(0, 0, BTN_HEIGHT, BTN_HEIGHT)
{
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

	SetSize(iX + BTN_HEIGHT - m_aiSpaces[m_aiSpaces.size()-1], BTN_HEIGHT);
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

CAboutPanel* CAboutPanel::s_pAboutPanel = NULL;

CAboutPanel::CAboutPanel()
	: CPanel(0, 0, 100, 100)
{
	m_pInfo = new CLabel(0, 0, 100, 100, "");
	AddControl(m_pInfo);
	CRootPanel::Get()->AddControl(this);
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

	CPanel::Layout();
}

void CAboutPanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h);

	CPanel::Paint(x, y, w, h);
}

bool CAboutPanel::MousePressed(int iButton, int mx, int my)
{
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
