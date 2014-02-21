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

tstring TPortal_GetPortalIdentifier()
{
	return "Tinker";
}

tstring TPortal_GetPlayerNickname()
{
	return "";
}

#endif
