#include "portal.h"

#ifdef TINKER_PORTAL_STEAM

#include <winsock2.h>

#include <steam/steam_api.h>
#include <strutils.h>

bool TPortal_Startup()
{
	bool bOkay = SteamAPI_Init();

	if (!bOkay)
		return false;

	// 127.0.0.1 = 2130706433
	SteamUser()->InitiateGameConnection( NULL, 0, k_steamIDNonSteamGS, 2130706433, 30203, false );
	SteamUtils()->SetOverlayNotificationPosition( k_EPositionBottomLeft );

	return true;
}

void TPortal_Think()
{
	SteamAPI_RunCallbacks();
}

void TPortal_Shutdown()
{
	SteamUser()->TerminateGameConnection( 2130706433, 30203 );

	SteamAPI_Shutdown();
}

bool TPortal_IsAvailable()
{
	return !!SteamUser();
}

tstring TPortal_GetPortalIdentifier()
{
	return "Steam";
}

tstring TPortal_GetPlayerNickname()
{
	if (!SteamFriends())
		return "";

	const char* pszNickname = SteamFriends()->GetPersonaName();

	return tstring(pszNickname);
}

#endif
