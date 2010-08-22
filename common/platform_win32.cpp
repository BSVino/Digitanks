#include <windows.h>
#include <iphlpapi.h>
#include <tchar.h>
#include <dbghelp.h>

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
	Sleep(1);
}

void OpenBrowser(const wchar_t* pszAddress)
{
	ShellExecute(NULL, L"open", pszAddress, NULL, NULL, SW_SHOWNORMAL);
}

static int g_iMinidumpsWritten = 0;

void CreateMinidump(void* pInfo)
{
#ifndef _DEBUG
	time_t currTime = ::time( NULL );
	struct tm * pTime = ::localtime( &currTime );

	wchar_t szModule[MAX_PATH];
	::GetModuleFileName( NULL, szModule, sizeof(szModule) / sizeof(wchar_t) );
	wchar_t *pModule = wcsrchr( szModule, '.' );

	if ( pModule )
		*pModule = 0;

	pModule = wcsrchr( szModule, '\\' );
	if ( pModule )
		pModule++;
	else
		pModule = _T("unknown");

	wchar_t szFileName[MAX_PATH];
	_snwprintf( szFileName, sizeof(szFileName) / sizeof(wchar_t),
			_T("%s_%d.%.2d.%2d.%.2d.%.2d.%.2d_%d.mdmp"),
			pModule,
			pTime->tm_year + 1900,
			pTime->tm_mon + 1,
			pTime->tm_mday,
			pTime->tm_hour,
			pTime->tm_min,
			pTime->tm_sec,
			g_iMinidumpsWritten++
			);

	HANDLE hFile = CreateFile( szFileName, GENERIC_READ | GENERIC_WRITE,
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
