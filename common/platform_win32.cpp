#include <platform.h>

// tchar.h defines it
#ifdef _T
#undef _T
#endif

#include <windows.h>
#include <iphlpapi.h>
#include <tchar.h>
#include <dbghelp.h>

// tchar.h defines it so reset it
#ifdef _T
#undef _T
#define _T EA_CHAR16
#endif

#include <tstring.h>

void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses)
{
	static unsigned char aiAddresses[16][8];

	IP_ADAPTER_INFO AdapterInfo[16];

	DWORD dwBufLen = sizeof(AdapterInfo);

	DWORD dwStatus = GetAdaptersInfo(
		AdapterInfo,
		&dwBufLen);

	iAddresses = 0;

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	do {
		// Use only ethernet, not wireless controllers. They are more reliable and wireless controllers get disabled sometimes.
		if (pAdapterInfo->Type != MIB_IF_TYPE_ETHERNET)
			continue;

		// Skip virtual controllers, they can be changed or disabled.
		if (strstr(pAdapterInfo->Description, "VMware"))
			continue;

		if (strstr(pAdapterInfo->Description, "Hamachi"))
			continue;

		// Skip USB controllers, they can be unplugged.
		if (strstr(pAdapterInfo->Description, "USB"))
			continue;

		memcpy(aiAddresses[iAddresses++], pAdapterInfo->Address, sizeof(char)*8);
	}
	while(pAdapterInfo = pAdapterInfo->Next);

	paiAddresses = &aiAddresses[0][0];
}

void GetScreenSize(int& iWidth, int& iHeight)
{
	iWidth = GetSystemMetrics(SM_CXSCREEN);
	iHeight = GetSystemMetrics(SM_CYSCREEN);
}

size_t GetNumberOfProcessors()
{
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	return SystemInfo.dwNumberOfProcessors;
}

void SleepMS(size_t iMS)
{
	Sleep(iMS);
}

void OpenBrowser(const tstring& sURL)
{
	ShellExecute(NULL, L"open", sURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

static int g_iMinidumpsWritten = 0;

void CreateMinidump(void* pInfo, tchar* pszDirectory)
{
#ifndef _DEBUG
	time_t currTime = ::time( NULL );
	struct tm * pTime = ::localtime( &currTime );

	tchar szModule[MAX_PATH];
	::GetModuleFileName( NULL, szModule, sizeof(szModule) / sizeof(tchar) );
	tchar *pModule = wcsrchr( szModule, '.' );

	if ( pModule )
		*pModule = 0;

	pModule = wcsrchr( szModule, '\\' );
	if ( pModule )
		pModule++;
	else
		pModule = _T("unknown");

	tchar szFileName[MAX_PATH];
	_snwprintf( szFileName, sizeof(szFileName) / sizeof(tchar),
			L"%s_%d.%.2d.%2d.%.2d.%.2d.%.2d_%d.mdmp",
			pModule,
			pTime->tm_year + 1900,
			pTime->tm_mon + 1,
			pTime->tm_mday,
			pTime->tm_hour,
			pTime->tm_min,
			pTime->tm_sec,
			g_iMinidumpsWritten++
			);

	HANDLE hFile = CreateFile( GetAppDataDirectory(pszDirectory, szFileName).c_str(), GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) )
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId           = GetCurrentThreadId();
		mdei.ExceptionPointers  = (EXCEPTION_POINTERS*)pInfo;
		mdei.ClientPointers     = FALSE;

		MINIDUMP_CALLBACK_INFORMATION mci;

		mci.CallbackRoutine     = NULL;
		mci.CallbackParam       = 0;

		MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

		BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(),
			hFile, mdt, (pInfo != 0) ? &mdei : 0, 0, &mci );

		if( rv )
		{
			// Success... message to user?
		}

		CloseHandle( hFile );
	}
#endif
}

tchar* OpenFileDialog(const tchar* pszFileTypes, const tchar* pszDirectory)
{
	static tchar szFile[256];
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
	opf.lpstrInitialDir = pszDirectory;
	opf.lpstrTitle = L"Open File";
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

tchar* SaveFileDialog(const tchar* pszFileTypes, const tchar* pszDirectory)
{
	static tchar szFile[256];
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
	opf.lpstrInitialDir = pszDirectory;
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

eastl::string GetClipboard()
{
	if (!OpenClipboard(NULL))
		return "";

	HANDLE hData = GetClipboardData(CF_TEXT);
	char* szBuffer = (char*)GlobalLock(hData);
	GlobalUnlock(hData);
	CloseClipboard();

	eastl::string sClipboard(szBuffer);

	return sClipboard;
}

void SetClipboard(const eastl::string& sBuf)
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

tstring GetAppDataDirectory(const tstring& sDirectory, const tstring& sFile)
{
	size_t iSize;
	_wgetenv_s(&iSize, NULL, 0, L"APPDATA");

	tstring sSuffix;
	sSuffix.append(sDirectory).append(_T("\\")).append(sFile);

	if (!iSize)
		return sSuffix;

	tchar* pszVar = (tchar*)malloc(iSize * sizeof(tchar));
	if (!pszVar)
		return sSuffix;

	_wgetenv_s(&iSize, pszVar, iSize, L"APPDATA");

	tstring sReturn(pszVar);

	free(pszVar);

	CreateDirectory(tstring(sReturn).append(_T("\\")).append(sDirectory).c_str(), NULL);

	sReturn.append(_T("\\")).append(sSuffix);
	return sReturn;
}

eastl::vector<tstring> ListDirectory(tstring sDirectory, bool bDirectories)
{
	eastl::vector<tstring> asResult;

	tchar szPath[MAX_PATH];
	_swprintf(szPath, L"%s\\*", sDirectory.c_str());

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(szPath, &fd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		int count = 0;
		do
		{
			if (!bDirectories && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			// Duh.
			if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
				continue;

			asResult.push_back(fd.cFileName);
		} while(FindNextFile(hFind, &fd));

		FindClose(hFind);
	}

	return asResult;
}

bool IsFile(tstring sPath)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(sPath.c_str(), &fd);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return false;

	return true;
}

bool IsDirectory(tstring sPath)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(sPath.c_str(), &fd);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	return false;
}

void DebugPrint(tstring sText)
{
	OutputDebugString(sText.c_str());
}

void Exec(eastl::string sLine)
{
	system(sLine.c_str());
}
