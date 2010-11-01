#ifndef _CF_PLATFORM
#define _CF_PLATFORM

#include <string>

void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses);
void GetScreenSize(int& iWidth, int& iHeight);
size_t GetNumberOfProcessors();
void SleepMS(size_t iMS);
void OpenBrowser(const wchar_t* pszAddress);
void CreateMinidump(void* pInfo);
wchar_t* OpenFileDialog(wchar_t* pszFileTypes);
wchar_t* SaveFileDialog(wchar_t* pszFileTypes);
std::string GetClipboard();
void SetClipboard(const std::string& sBuf);
void ShowMessage(const wchar_t* pszMessage);

#endif