#ifndef _TINKER_COMMANDS_H
#define _TINKER_COMMANDS_H

#include <assert.h>

#include <EASTL/map.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>

#include <color.h>
#include <vector.h>
#include <strutils.h>

#include "network.h"

typedef void (*CommandServerCallback)(int iConnection, class CNetworkCommand* pCmd, size_t iClient, const eastl::string16& sParameters);

class CNetworkCommand
{
public:
	CNetworkCommand(int iConnection, eastl::string16 sName, CommandServerCallback pfnCallback, int iTarget)
	{
		m_iConnection = iConnection;
		m_sName = str_replace(sName, L" ", L"-");
		m_pfnCallback = pfnCallback;
		m_iMessageTarget = iTarget;
	};

	CNetworkCommand(eastl::string16 sName, CommandServerCallback pfnCallback, int iTarget)
	{
		m_iConnection = CONNECTION_UNDEFINED;
		m_sName = str_replace(sName, L" ", L"-");
		m_pfnCallback = pfnCallback;
		m_iMessageTarget = iTarget;
	};

public:
	void					RunCommand(const eastl::string16& sParameters);
	void					RunCommand(const eastl::string16& sParameters, int iTarget);
	void					RunCommand(int iConnection, const eastl::string16& sParameters);
	void					RunCommand(int iConnection, const eastl::string16& sParameters, int iTarget);

	void					RunCallback(int iConnection, size_t iClient, const eastl::string16& sParameters);

	// Flips the message around, it becomes a message to all clients
	int						GetMessageTarget() { return m_iMessageTarget; };

	size_t					GetNumArguments();
	eastl::string16			Arg(size_t i);
	bool					ArgAsBool(size_t i);
	size_t					ArgAsUInt(size_t i);
	int						ArgAsInt(size_t i);
	float					ArgAsFloat(size_t i);

	static eastl::map<eastl::string16, CNetworkCommand*>& GetCommands();
	static CNetworkCommand*	GetCommand(const eastl::string16& sName);
	static void				RegisterCommand(CNetworkCommand* pCommand);

protected:
	eastl::string16			m_sName;
	CommandServerCallback	m_pfnCallback;

	eastl::vector<eastl::string16> m_asArguments;

	int						m_iConnection;
	int						m_iMessageTarget;
};

#define CLIENT_COMMAND(cxn, name) \
void ClientCommand_##name(int iConnection, CNetworkCommand* pCmd, size_t iClient, const eastl::string16& sParameters); \
CNetworkCommand name(cxn, convertstring<char, char16_t>(#name), ClientCommand_##name, NETWORK_TOSERVER); \
class CRegisterClientCommand##name \
{ \
public: \
	CRegisterClientCommand##name() \
	{ \
		CNetworkCommand::RegisterCommand(&name); \
	} \
} g_RegisterClientCommand##name = CRegisterClientCommand##name(); \
void ClientCommand_##name(int iConnection, CNetworkCommand* pCmd, size_t iClient, const eastl::string16& sParameters) \

#define SERVER_COMMAND(cxn, name) \
void ServerCommand_##name(int iConnection, CNetworkCommand* pCmd, size_t iClient, const eastl::string16& sParameters); \
CNetworkCommand name(cxn, convertstring<char, char16_t>(#name), ServerCommand_##name, NETWORK_TOCLIENTS); \
class CRegisterServerCommand##name \
{ \
public: \
	CRegisterServerCommand##name() \
	{ \
		CNetworkCommand::RegisterCommand(&name); \
	} \
} g_RegisterServerCommand##name = CRegisterServerCommand##name(); \
void ServerCommand_##name(int iConnection, CNetworkCommand* pCmd, size_t iClient, const eastl::string16& sParameters) \

#define CLIENT_GAME_COMMAND(name) \
	CLIENT_COMMAND(CONNECTION_GAME, name) \

#define SERVER_GAME_COMMAND(name) \
	SERVER_COMMAND(CONNECTION_GAME, name) \

#endif
