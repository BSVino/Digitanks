#include "modelwindow_ui.h"

#include "modelwindow.h"

void CModelWindow::InitUI()
{
	CRootPanel::Get()->AddMenu("File");
	CRootPanel::Get()->AddMenu("View");
	CRootPanel::Get()->AddMenu("Tools");
	CRootPanel::Get()->AddMenu("About");

	CRootPanel::Get()->Layout();
}
