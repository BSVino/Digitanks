#ifndef _CF_PLATFORM
#define _CF_PLATFORM

void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses);
size_t GetNumberOfProcessors();
void SleepMS(size_t iMS);

#endif