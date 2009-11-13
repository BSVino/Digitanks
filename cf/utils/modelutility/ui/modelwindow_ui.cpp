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
	pFile->AddSubmenu("Exit", this, Exit);

	pHelp->AddSubmenu("About SMAK", this, About);

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

void CModelWindow::ExitCallback()
{
	exit(0);
}

void CModelWindow::AboutCallback()
{
	OpenAboutPanel();
}

void CModelWindow::OpenAboutPanel()
{
	CAboutPanel::Open();
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
	m_pInfo->AppendText("FCollada copyright © 2006, Feeling Software\n");
	m_pInfo->AppendText("DevIL copyright © 2001-2009, Denton Woods\n");
	m_pInfo->AppendText("FTGL copyright © 2001-2003 Henry Maddocks\n");
	m_pInfo->AppendText("Freeglut copyright © 1999-2000, Pawel W. Olszta\n");
	m_pInfo->AppendText("GLEW copyright © 2002-2007, Milan Ikits, Marcelo E. Magallon, Lev Povalahev\n");
	m_pInfo->AppendText("Freetype copyright © 1996-2002, 2006 by David Turner, Robert Wilhelm, and Werner Lemberg\n");

	CPanel::Layout();
}

void CAboutPanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::Get()->PaintRect(x, y, w, h, 255);

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
