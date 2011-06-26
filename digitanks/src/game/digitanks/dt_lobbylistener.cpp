#include "dt_lobbylistener.h"

#include <common.h>

#include <digitanks/digitanksgame.h>

bool CDigitanksLobbyListener::ClientConnect(size_t iClient)
{
	return true;
}

void CDigitanksLobbyListener::ClientDisconnect(size_t iClient)
{
}

bool CDigitanksLobbyListener::UpdateLobby(size_t iLobby, const tstring& sKey, const tstring& sValue)
{
	return true;
}

bool CDigitanksLobbyListener::UpdatePlayer(size_t iID, const tstring& sKey, const tstring& sValue)
{
	if (sKey == _T("color") && sValue != _T("random"))
	{
		// Only one player can have any particular color.

		size_t iLobby = CGameLobbyServer::GetPlayerLobby(iID);
		CGameLobby* pLobby = CGameLobbyServer::GetLobby(iLobby);

		TAssert(pLobby);
		if (!pLobby)
			return false;

		for (size_t i = 0; i < pLobby->GetNumPlayers(); i++)
		{
			CLobbyPlayer* pPlayer = pLobby->GetPlayer(i);

			if (pPlayer->iID == iID)
				continue;

			if (pPlayer->GetInfoValue(_T("color")) == sValue)
				// Only one player can have any particular color. If another player already has this color deny the color change.
				return false;
		}
	}

	return true;
}

bool CDigitanksLobbyListener::BeginGame(size_t iLobby)
{
	if (CGameLobbyServer::GetLobby(iLobby)->GetInfoValue(_T("level_file")) == _T(""))
		return false;

	if (CDigitanksGame::GetLevel(CGameLobbyServer::GetLobby(iLobby)->GetInfoValue(_T("level_file"))) == NULL)
		return false;

	return true;
}

CDigitanksLobbyListener* DigitanksLobbyListener()
{
	static CDigitanksLobbyListener* pListener = new CDigitanksLobbyListener();

	return pListener;
}
