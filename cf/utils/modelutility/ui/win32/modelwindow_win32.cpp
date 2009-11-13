#include "../modelwindow.h"

#include <windows.h>

wchar_t* CModelWindow::OpenFileDialog()
{
	static wchar_t szFile[256];
	szFile[0] = '\0';

	OPENFILENAME opf;
	opf.hwndOwner = 0;
	opf.lpstrFilter = L"All *.obj;*.dae;*.sia\0*.obj;*.dae;*.sia\0";
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
	opf.Flags = (OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT) & ~OFN_ALLOWMULTISELECT;
	opf.lStructSize = sizeof(OPENFILENAME);

	if(GetOpenFileName(&opf))
		return opf.lpstrFile;

	return NULL;
}

wchar_t* CModelWindow::SaveFileDialog()
{
	static wchar_t szFile[256];
	szFile[0] = '\0';

	OPENFILENAME opf;
	opf.hwndOwner = 0;
	opf.lpstrFilter = L"Valve Source SMD\0*.smd\0";
	opf.lpstrCustomFilter = 0;
	opf.nMaxCustFilter = 0L;
	opf.nFilterIndex = 1L;
	opf.lpstrFile = szFile;
	opf.lpstrFile[0] = '\0';
	opf.nMaxFile = 256;
	opf.lpstrFileTitle = 0;
	opf.nMaxFileTitle=50;
	opf.lpstrInitialDir = NULL;
	opf.lpstrTitle = L"Save Model";
	opf.nFileOffset = 0;
	opf.nFileExtension = 0;
	opf.lpstrDefExt = L"*.*";
	opf.lpfnHook = NULL;
	opf.lCustData = 0;
	opf.Flags = (OFN_OVERWRITEPROMPT) & ~OFN_ALLOWMULTISELECT;
	opf.lStructSize = sizeof(OPENFILENAME);

	if(GetOpenFileName(&opf))
		return opf.lpstrFile;

	return NULL;
}
