#ifndef _CF_PLATFORM
#define _CF_PLATFORM

void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses);
void GetScreenSize(int& iWidth, int& iHeight);
size_t GetNumberOfProcessors();
void SleepMS(size_t iMS);
void OpenBrowser(const wchar_t* pszAddress);

#endif