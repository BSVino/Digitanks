#ifndef LW_GLGUI_FILEDIALOG_H
#define LW_GLGUI_FILEDIALOG_H

#include "glgui.h"

namespace glgui
{
	class CFileDialog : public CPanel, public IEventListener
	{
		DECLARE_CLASS(CFileDialog, CPanel);

	public:
									CFileDialog(const tstring& sDirectory, const tstring& sExtension, bool bSave);

	public:
		virtual void				Destructor();

		virtual void				Layout();
		virtual void				Paint(int x, int y, int w, int h);

		EVENT_CALLBACK(CFileDialog, NewDirectory);
		EVENT_CALLBACK(CFileDialog, Explore);
		EVENT_CALLBACK(CFileDialog, FileSelected);
		EVENT_CALLBACK(CFileDialog, NewFileChanged);
		EVENT_CALLBACK(CFileDialog, Select);
		EVENT_CALLBACK(CFileDialog, Close);

		static void					ShowOpenDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback);
		static void					ShowSaveDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback);
		static tstring				GetFile();

	protected:
		CLabel*						m_pDirectoryLabel;
		CTextField*					m_pDirectory;
		CButton*					m_pOpenInExplorer;

		CLabel*						m_pFilesLabel;
		CTree*						m_pFileList;

		CTextField*					m_pNewFile;
		CButton*					m_pSelect;
		CButton*					m_pCancel;

		tstring						m_sDirectory;
		tstring						m_sExtension;
		bool						m_bSave;

		IEventListener*				m_pSelectListener;
		IEventListener::Callback	m_pfnSelectCallback;

		static CFileDialog*			s_pDialog;
	};
};

#endif
