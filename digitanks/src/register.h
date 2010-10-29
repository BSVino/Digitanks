#ifndef DT_REGISTER_H
#define DT_REGISTER_H

#include <string>

bool IsRegistered();
void ReadProductCode();
std::string GetProductCode();
void SetLicenseKey(std::string sKey);
bool QueryRegistrationKey(std::wstring sKey, std::wstring& sError);

#endif