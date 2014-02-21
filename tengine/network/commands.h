#ifndef _TINKER_COMMANDS_H
#define _TINKER_COMMANDS_H

#include <tmap.h>
#include <tvector.h>
#include <color.h>
#include <vector.h>
#include <strutils.h>

#include "network.h"

typedef void (*CommandServerCallback)(int iConnection, class CNetworkCommand* pCmd, size_t iClient, const tstring& sParameters);

class CNetworkCommand
{
public:
	CNetworkCommand(int iConnection, tstring sName, CommandServerCallback pfnCallback, network_id_t iTarget)
	{
		m_iConnection = iConnection;
		m_sName = sName.replace(" ", "-");
		m_pfnCallback = pfnCallback;
		m_iMessageTarget = iTarget;
	};

	CNetworkCommand(tstring sName, CommandServerCallback pfnCallback, network_id_t iTarget)
	{
		m_iConnection = CONNECTION_UNDEFINED;
		m_sName = sName.replace(" ", "-");
		m_pfnCallback = pfnCallback;
		m_iMessageTarget = iTarget;
	};

public:
	void					RunCommand(const tstring& sParameters);
	void					RunCommand(const tstring& sParameters, int iTarget);
	void					RunCommand(int iConnection, const tstring& sParameters);
	void					RunCommand(int iConnection, const tstring& sParameters, int iTarget);

	void					RunCallback(int iConnection, size_t iClient, const tstring& sParameters);

	// Flips the message around, it becomes a message to all clients
	network_id_t			GetMessageTarget() { return m_iMessageTarget; };

	size_t					GetNumArguments();
	tstring			Arg(size_t i);
	bool					ArgAsBool(size_t i);
	size_t					ArgAsUInt(size_t i);
	int						ArgAsInt(size_t i);
	float					ArgAsFloat(size_t i);

	static tmap<tstring, CNetworkCommand*>& GetCommands();
	static CNetworkCommand*	GetCommand(const tstring& sName);
	static void				RegisterCommand(CNetworkCommand* pCommand);

protected:
	tstring			m_sName;
	CommandServerCallback	m_pfnCallback;

	tvector<tstring>		m_asArguments;

	int						m_iConnection;
	network_id_t			m_iMessageTarget;
};

#define CLIENT_COMMAND(cxn, name) \
void ClientCommand_##name(int iConnection, CNetworkCommand* pCmd, size_t iClient, const tstring& sParameters); \
CNetworkCommand name(cxn, #name, ClientCommand_##name, NETWORK_TOSERVER); \
class CRegisterClientCommand##name \
{ \
public: \
	CRegisterClientCommand##name() \
	{ \
		CNetworkCommand::RegisterCommand(&name); \
	} \
} g_RegisterClientCommand##name = CRegisterClientCommand##name(); \
void ClientCommand_##name(int iConnection, CNetworkCommand* pCmd, size_t iClient, const tstring& sParameters) \

#define SERVER_COMMAND_TARGET(cxn, name, target) \
void ServerCommand_##name(int iConnection, CNetworkCommand* pCmd, size_t iClient, const tstring& sParameters); \
CNetworkCommand name(cxn, #name, ServerCommand_##name, target); \
class CRegisterServerCommand##name \
{ \
public: \
	CRegisterServerCommand##name() \
	{ \
		CNetworkCommand::RegisterCommand(&name); \
	} \
} g_RegisterServerCommand##name = CRegisterServerCommand##name(); \
void ServerCommand_##name(int iConnection, CNetworkCommand* pCmd, size_t iClient, const tstring& sParameters) \

#define SERVER_COMMAND(cxn, name) \
	SERVER_COMMAND_TARGET(cxn, name, NETWORK_TOCLIENTS)

#define CLIENT_GAME_COMMAND(name) \
	CLIENT_COMMAND(CONNECTION_GAME, name) \

#define SERVER_GAME_COMMAND(name) \
	SERVER_COMMAND(CONNECTION_GAME, name) \

#endif
