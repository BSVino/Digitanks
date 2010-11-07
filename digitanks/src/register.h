#ifndef DT_REGISTER_H
#define DT_REGISTER_H

#include <EASTL/string.h>

bool IsRegistered();
void ReadProductCode();
eastl::string GetProductCode();
void SetLicenseKey(eastl::string sKey);
bool QueryRegistrationKey(eastl::string16 sKey, eastl::string16& sError);

#endif