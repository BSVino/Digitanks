#include "dt_lobbylistener.h"

#include <assert.h>

bool CDigitanksLobbyListener::ClientConnect(size_t iClient)
{
	return true;
}

void CDigitanksLobbyListener::ClientDisconnect(size_t iClient)
{
}

bool CDigitanksLobbyListener::UpdateLobby(size_t iLobby, const eastl::string16& sKey, const eastl::string16& sValue)
{
	return true;
}

bool CDigitanksLobbyListener::UpdatePlayer(size_t iID, const eastl::string16& sKey, const eastl::string16& sValue)
{
	if (sKey == L"color" && sValue != L"random")
	{
		// Only one player can have any particular color.

		size_t iLobby = CGameLobbyServer::GetPlayerLobby(iID);
		CGameLobby* pLobby = CGameLobbyServer::GetLobby(iLobby);

		assert(pLobby);
		if (!pLobby)
			return false;

		for (size_t i = 0; i < pLobby->GetNumPlayers(); i++)
		{
			CLobbyPlayer* pPlayer = pLobby->GetPlayer(i);

			if (pPlayer->iID == iID)
				continue;

			if (pPlayer->GetInfoValue(L"color") == sValue)
				// Only one player can have any particular color. If another player already has this color deny the color change.
				return false;
		}
	}

	return true;
}

CDigitanksLobbyListener* DigitanksLobbyListener()
{
	static CDigitanksLobbyListener* pListener = new CDigitanksLobbyListener();

	return pListener;
}
