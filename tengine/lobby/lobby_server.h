#ifndef _TINKER_LOBBY_SERVER_H
#define _TINKER_LOBBY_SERVER_H

#include <tvector.h>

#include "lobby.h"

class CGameLobby
{
	friend class CGameLobbyServer;

public:
										CGameLobby();

public:
	void								Initialize();
	void								Shutdown();

	size_t								GetNumPlayers();
	size_t								GetPlayerIndexByID(size_t iID);
	size_t								GetPlayerIndexByClient(size_t iClient);
	CLobbyPlayer*						GetPlayer(size_t iIndex);
	CLobbyPlayer*						GetPlayerByID(size_t iID);
	CLobbyPlayer*						GetPlayerByClient(size_t iClient);

	void								UpdateInfo(const tstring& sKey, const tstring& sValue);
	tstring						GetInfoValue(const tstring& sKey);

	void								AddPlayer(size_t iID, size_t iClient);
	void								RemovePlayer(size_t iID);
	void								UpdatePlayer(size_t iID, const tstring& sKey, const tstring& sValue);

	void								SendFullUpdate(size_t iID);

protected:
	bool								m_bActive;
	size_t								m_iLobbyID;
	tvector<CLobbyPlayer>				m_aClients;
	tmap<tstring, tstring>				m_asInfo;
};

class ILobbyListener
{
public:
	virtual bool						ClientConnect(size_t iClient)=0;
	virtual void						ClientDisconnect(size_t iClient)=0;

	virtual bool						UpdateLobby(size_t iLobby, const tstring& sKey, const tstring& sValue)=0;
	virtual bool						UpdatePlayer(size_t iID, const tstring& sKey, const tstring& sValue)=0;

	virtual bool						BeginGame(size_t iLobby)=0;
};

class CGameLobbyServer
{
	friend class CGameLobby;

public:
	static size_t						CreateLobby();
	static void							DestroyLobby(size_t iLobby);
	static size_t						AddPlayer(size_t iLobby, size_t iClient);
	static void							RemovePlayer(size_t iID);
	static CGameLobby*					GetLobby(size_t iLobby);

	static size_t						GetActiveLobbies();
	static size_t						GetPlayerLobby(size_t iID);
	static size_t						GetClientPlayerID(size_t iClient);

	static void							UpdateLobby(size_t iLobby, const tstring& sKey, const tstring& sValue);
	static void							UpdatePlayer(size_t iID, const tstring& sKey, const tstring& sValue);

	static void							ClientEnterGame(int iConnection, class INetworkListener*, class CNetworkParameters*);
	static void							ClientDisconnect(int iConnection, class INetworkListener*, class CNetworkParameters*);

	static void							SetListener(ILobbyListener* pListener);

	static size_t						GetNextPlayerID();

protected:
	static tvector<CGameLobby>			s_aLobbies;
	static tmap<size_t, size_t>			s_aiPlayerLobbies;
	static tmap<size_t, size_t>			s_aiClientPlayerIDs;
	static ILobbyListener*				s_pListener;
};

#endif
