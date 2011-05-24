#ifndef TINKER_PORTAL_H
#define TINKER_PORTAL_H

#include <EASTL/string.h>

#if !defined(TINKER_PORTAL_STEAM)
#ifndef TINKER_PORTAL_NONE
#define TINKER_PORTAL_NONE
#endif
#endif

bool TPortal_Startup();
void TPortal_Think();
void TPortal_Shutdown();

eastl::string16 TPortal_GetPortalIdentifier();
bool TPortal_IsAvailable();

eastl::string16 TPortal_GetPlayerNickname();

#endif
