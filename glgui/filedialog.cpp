#include "filedialog.h"

#include <tinker_platform.h>

namespace glgui
{
	CFileDialog* CFileDialog::s_pDialog = NULL;

	CFileDialog::CFileDialog(const tstring& sDirectory, const tstring& sExtension, bool bSave)
		: CPanel(0, 0, 100, 100)
	{
		m_sDirectory = sDirectory;
		m_sExtension = sExtension;
		m_bSave = bSave;

		m_pDirectoryLabel = new CLabel(5, 5, 1, 1, "Folder:");
		AddControl(m_pDirectoryLabel);
		m_pDirectory = new CTextField();
		m_pDirectory->SetContentsChangedListener(this, NewDirectory);
		m_pDirectory->SetText(m_sDirectory);
		AddControl(m_pDirectory);
		m_pOpenInExplorer = new CButton(0, 0, 20, 20, "Open in explorer", false, "sans-serif", 8);
		m_pOpenInExplorer->SetClickedListener(this, Explore);
		AddControl(m_pOpenInExplorer);

		m_pFilesLabel = new CLabel(5, 5, 1, 1, "Files:");
		AddControl(m_pFilesLabel);
		m_pFileList = new CTree(~0, ~0, ~0);
		m_pFileList->SetSelectedListener(this, FileSelected);
		AddControl(m_pFileList);

		m_pNewFile = new CTextField();
		m_pNewFile->SetContentsChangedListener(this, NewFileChanged);
		m_pNewFile->SetVisible(bSave);
		AddControl(m_pNewFile);
		m_pSelect = new CButton(0, 0, 20, 20, bSave?"Save":"Open");
		m_pSelect->SetClickedListener(this, Select);
		m_pSelect->SetEnabled(false);
		AddControl(m_pSelect);
		m_pCancel = new CButton(0, 0, 20, 20, "Cancel");
		m_pCancel->SetClickedListener(this, Close);
		AddControl(m_pCancel);

		CRootPanel::Get()->AddControl(this, true);
	}

	void CFileDialog::Destructor()
	{
		TAssert(s_pDialog == this);
		s_pDialog = NULL;

		BaseClass::Destructor();
	}

	void CFileDialog::Layout()
	{
		SetSize(400, 300);
		SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

		m_pDirectoryLabel->EnsureTextFits();
		m_pFilesLabel->EnsureTextFits();

		m_pDirectory->SetPos(5, m_pDirectoryLabel->GetBottom()+5);
		m_pDirectory->SetSize(GetWidth()-55, m_pDirectory->GetHeight());

		m_pOpenInExplorer->SetSize(40, m_pDirectory->GetHeight());
		m_pOpenInExplorer->SetPos(GetWidth()-45, m_pDirectoryLabel->GetBottom()+5);

		m_pFilesLabel->SetPos(5, m_pDirectory->GetBottom()+5);
		m_pFileList->SetPos(5, m_pFilesLabel->GetBottom()+5);
		m_pFileList->SetSize(200, 170);

		m_pFileList->ClearTree();

		eastl::vector<tstring> asFiles = ListDirectory(m_sDirectory, false);

		for (size_t i = 0; i < asFiles.size(); i++)
		{
			tstring sFile = asFiles[i];

			if (sFile.length() < m_sExtension.length())
				continue;

			if (sFile.substr(sFile.length()-m_sExtension.length()) != m_sExtension)
				continue;

			m_pFileList->AddNode(sFile);
		}

		m_pSelect->SetSize(60, m_pNewFile->GetHeight());
		m_pSelect->SetPos(GetWidth() - 130, GetHeight() - m_pNewFile->GetHeight() - 5);

		m_pCancel->SetSize(60, m_pNewFile->GetHeight());
		m_pCancel->SetPos(GetWidth() - 65, GetHeight() - m_pNewFile->GetHeight() - 5);

		m_pNewFile->SetPos(5, GetHeight() - m_pNewFile->GetHeight() - 5);
		m_pNewFile->SetSize(GetWidth() - 140, m_pNewFile->GetHeight());

		BaseClass::Layout();
	}

	void CFileDialog::Paint(int x, int y, int w, int h)
	{
		CRootPanel::PaintRect(x, y, w, h, g_clrBox/2);

		CRect r = m_pFileList->GetAbsDimensions();
		CRootPanel::PaintRect((int)r.x, (int)r.y, (int)r.w, (int)r.h, g_clrBox);

		BaseClass::Paint(x, y, w, h);
	}

	void CFileDialog::NewDirectoryCallback()
	{
		m_sDirectory = m_pDirectory->GetText();
		Layout();
	}

	void CFileDialog::ExploreCallback()
	{
		OpenExplorer(m_sDirectory);
	}

	void CFileDialog::FileSelectedCallback()
	{
		m_pNewFile->SetText("");

		m_pSelect->SetEnabled(!!m_pFileList->GetSelectedNode());
	}

	void CFileDialog::NewFileChangedCallback()
	{
		m_pFileList->Unselect();

		m_pSelect->SetEnabled(m_pNewFile->GetText().length() > 0);
	}

	void CFileDialog::SelectCallback()
	{
		if (m_pSelectListener && m_pfnSelectCallback)
			m_pfnSelectCallback(m_pSelectListener);

		Destructor();
		Delete();
	}

	void CFileDialog::CloseCallback()
	{
		Destructor();
		Delete();
	}

	void CFileDialog::ShowOpenDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback)
	{
		if (s_pDialog)
		{
			s_pDialog->Destructor();
			s_pDialog->Delete();
		}

		s_pDialog = new CFileDialog(sDirectory, sExtension, false);
		s_pDialog->Layout();
		s_pDialog->m_pSelectListener = pListener;
		s_pDialog->m_pfnSelectCallback = pfnCallback;
	}

	void CFileDialog::ShowSaveDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback)
	{
		if (s_pDialog)
		{
			s_pDialog->Destructor();
			s_pDialog->Delete();
		}

		s_pDialog = new CFileDialog(sDirectory, sExtension, true);
		s_pDialog->Layout();
		s_pDialog->m_pSelectListener = pListener;
		s_pDialog->m_pfnSelectCallback = pfnCallback;
	}

	tstring CFileDialog::GetFile()
	{
		if (!s_pDialog)
			return "";

		if (s_pDialog->m_bSave && s_pDialog->m_pNewFile->GetText().length())
		{
			tstring sName = s_pDialog->m_pNewFile->GetText();
			if (sName.length() <= s_pDialog->m_sExtension.length())
				return s_pDialog->m_sDirectory + DIR_SEP + s_pDialog->m_pNewFile->GetText() + s_pDialog->m_sExtension;

			tstring sNameExtension = sName.substr(sName.length() - s_pDialog->m_sExtension.length());
			if (sNameExtension == s_pDialog->m_sExtension)
				return s_pDialog->m_sDirectory + DIR_SEP + s_pDialog->m_pNewFile->GetText();

			return s_pDialog->m_sDirectory + DIR_SEP + s_pDialog->m_pNewFile->GetText() + s_pDialog->m_sExtension;
		}

		if (!s_pDialog->m_pFileList->GetSelectedNode())
			return "";

		return s_pDialog->m_sDirectory + DIR_SEP + s_pDialog->m_pFileList->GetSelectedNode()->m_pLabel->GetText();
	}
}
