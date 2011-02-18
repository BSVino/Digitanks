#ifndef TINKER_PORTAL_H
#define TINKER_PORTAL_H

#include <EASTL/string.h>

#if !defined(TINKER_PORTAL_STEAM)
#define TINKER_PORTAL_NONE
#endif

bool TPortal_Startup();
void TPortal_Think();
void TPortal_Shutdown();

eastl::string16 TPortal_GetPlayerNickname();

#endif
