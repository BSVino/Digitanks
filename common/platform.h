#ifndef _LW_PLATFORM
#define _LW_PLATFORM

#include <EASTL/string.h>
#include <EASTL/vector.h>

void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses);
void GetScreenSize(int& iWidth, int& iHeight);
size_t GetNumberOfProcessors();
void SleepMS(size_t iMS);
void OpenBrowser(const wchar_t* pszAddress);
void CreateMinidump(void* pInfo, wchar_t* pszDirectory);
wchar_t* OpenFileDialog(wchar_t* pszFileTypes);
wchar_t* SaveFileDialog(wchar_t* pszFileTypes);
eastl::string GetClipboard();
void SetClipboard(const eastl::string& sBuf);
void ShowMessage(const wchar_t* pszMessage);
eastl::string16 GetAppDataDirectory(const eastl::string16& sDirectory, const eastl::string16& sFile);
eastl::vector<eastl::string16> ListDirectory(eastl::string16 sDirectory, bool bDirectories = true);
bool IsFile(eastl::string16 sPath);
bool IsDirectory(eastl::string16 sPath);

#endif