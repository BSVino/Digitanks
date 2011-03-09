#include "portal.h"

#ifdef TINKER_PORTAL_NONE

bool TPortal_Startup()
{
	return true;
}

void TPortal_Think()
{
}

void TPortal_Shutdown()
{
}

bool TPortal_IsAvailable()
{
	return false;
}

eastl::string16 TPortal_GetPortalIdentifier()
{
	return L"Tinker";
}

eastl::string16 TPortal_GetPlayerNickname()
{
	return L"";
}

#endif
