#include "../modelwindow.h"

#include <windows.h>

wchar_t* CModelWindow::OpenFileDialog(wchar_t* pszFileTypes)
{
	static wchar_t szFile[256];
	szFile[0] = '\0';

	OPENFILENAME opf;
	opf.hwndOwner = 0;
	opf.lpstrFilter = pszFileTypes;
	opf.lpstrCustomFilter = 0;
	opf.nMaxCustFilter = 0L;
	opf.nFilterIndex = 1L;
	opf.lpstrFile = szFile;
	opf.lpstrFile[0] = '\0';
	opf.nMaxFile = 256;
	opf.lpstrFileTitle = 0;
	opf.nMaxFileTitle=50;
	opf.lpstrInitialDir = NULL;
	opf.lpstrTitle = L"Open Model";
	opf.nFileOffset = 0;
	opf.nFileExtension = 0;
	opf.lpstrDefExt = L"*.*";
	opf.lpfnHook = NULL;
	opf.lCustData = 0;
	opf.Flags = (OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR) & ~OFN_ALLOWMULTISELECT;
	opf.lStructSize = sizeof(OPENFILENAME);

	if(GetOpenFileName(&opf))
		return opf.lpstrFile;

	return NULL;
}

wchar_t* CModelWindow::SaveFileDialog(wchar_t* pszFileTypes)
{
	static wchar_t szFile[256];
	szFile[0] = '\0';

	OPENFILENAME opf;
	opf.hwndOwner = 0;
	opf.lpstrFilter = pszFileTypes;
	opf.lpstrCustomFilter = 0;
	opf.nMaxCustFilter = 0L;
	opf.nFilterIndex = 1L;
	opf.lpstrFile = szFile;
	opf.lpstrFile[0] = '\0';
	opf.nMaxFile = 256;
	opf.lpstrFileTitle = 0;
	opf.nMaxFileTitle=50;
	opf.lpstrInitialDir = NULL;
	opf.lpstrTitle = L"Save File";
	opf.nFileOffset = 0;
	opf.nFileExtension = 0;
	opf.lpstrDefExt = L"*.*";
	opf.lpfnHook = NULL;
	opf.lCustData = 0;
	opf.Flags = (OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR) & ~OFN_ALLOWMULTISELECT;
	opf.lStructSize = sizeof(OPENFILENAME);

	if(GetSaveFileName(&opf))
		return opf.lpstrFile;

	return NULL;
}

std::string CModelWindow::GetClipboard()
{
	if (!OpenClipboard(NULL))
		return "";

	HANDLE hData = GetClipboardData(CF_TEXT);
	char* szBuffer = (char*)GlobalLock(hData);
	GlobalUnlock(hData);
	CloseClipboard();

	std::string sClipboard(szBuffer);

	return sClipboard;
}

void CModelWindow::SetClipboard(const std::string& sBuf)
{
	if (!OpenClipboard(NULL))
		return;

	EmptyClipboard();

	HGLOBAL hClipboard;
	hClipboard = GlobalAlloc(GMEM_MOVEABLE, sBuf.length()+1);

	char* pszBuffer = (char*)GlobalLock(hClipboard);
	strcpy(pszBuffer, LPCSTR(sBuf.c_str()));

	GlobalUnlock(hClipboard);

	SetClipboardData(CF_TEXT, hClipboard);

	CloseClipboard();
}
