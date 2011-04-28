#ifndef _DT_LOBBY_LISTENER_H
#define _DT_LOBBY_LISTENER_H

#include <tinker/lobby/lobby_server.h>

class CDigitanksLobbyListener : public ILobbyListener
{
public:
	virtual bool						ClientConnect(size_t iClient);
	virtual void						ClientDisconnect(size_t iClient);

	virtual bool						UpdateLobby(size_t iLobby, const eastl::string16& sKey, const eastl::string16& sValue);
	virtual bool						UpdatePlayer(size_t iID, const eastl::string16& sKey, const eastl::string16& sValue);
};

CDigitanksLobbyListener* DigitanksLobbyListener();

#endif
