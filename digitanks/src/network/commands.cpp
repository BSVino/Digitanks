#include "commands.h"

#include <common.h>
#include <strutils.h>

#include <tinker/application.h>
#include <tinker/cvar.h>

CVar net_debug("net_debug", "off");

void CNetworkCommand::RunCommand(const eastl::string16& sParameters)
{
	RunCommand(m_iConnection, sParameters, m_iMessageTarget);
}

void CNetworkCommand::RunCommand(int iConnection, const eastl::string16& sParameters)
{
	RunCommand(iConnection, sParameters, m_iMessageTarget);
}

void CNetworkCommand::RunCommand(const eastl::string16& sParameters, int iTarget)
{
	RunCommand(m_iConnection, sParameters, iTarget);
}

void CNetworkCommand::RunCommand(int iConnection, const eastl::string16& sParameters, int iTarget)
{
	TAssert(iConnection != CONNECTION_UNDEFINED);

	bool bNoNetwork = false;
	bool bPredict = false;
	if (iTarget == NETWORK_TOCLIENTS)
	{
		if (Network(iConnection)->IsHost())
			bPredict = true;
		else
		{
			// If we're running client functions then we're going to get this message from the server anyway.
			if (Network(iConnection)->IsRunningClientFunctions())
				return;

			// Some shared code. Run the callback but don't send it over the wire.
			bNoNetwork = true;
			bPredict = true;
		}
	}

	if (iTarget == NETWORK_TOSERVER)
	{
		if (Network(iConnection)->IsHost() || !Network(iConnection)->IsConnected())
		{
			bNoNetwork = true;
			bPredict = true;
		}

		// If we're running client functions then the server already knows about this call.
		else if (!Network(iConnection)->IsHost() && Network(iConnection)->IsRunningClientFunctions())
		{
			bNoNetwork = true;
			bPredict = true;
		}
	}

	if (!bNoNetwork && Network(iConnection)->IsConnected())
	{
		eastl::string16 sCommand = m_sName + L" " + sParameters;

		CNetworkParameters p;
		p.CreateExtraData(sizeof(eastl::string16::value_type) * (sCommand.length() + 1));
		char16_t* pszData = (char16_t*)p.m_pExtraData;

		TAssert(sizeof(eastl::string16::value_type) == sizeof(char16_t));
		wcscpy(pszData, sCommand.c_str());

		p.ui1 = Network(iConnection)->GetClientID();

		Network(iConnection)->CallFunctionParameters(iTarget, "NC", &p);

		if (net_debug.GetBool())
		{
			if (iTarget == NETWORK_TOSERVER)
				TMsg(L"Net cmd to server: ");
			else if (iTarget == NETWORK_TOCLIENTS)
				TMsg(L"Net cmd to clients: ");
			else if (iTarget == NETWORK_TOEVERYONE)
				TMsg(L"Net cmd to all: ");
			else
				TMsg(sprintf(L"Net cmd to client %d: ", iTarget));

			TMsg(sCommand + L"\n");
		}
	}

	if (bPredict)
	{
		wcstok(sParameters, m_asArguments);
		m_pfnCallback(iConnection, this, -1, sParameters);
	}
}

void CNetworkCommand::RunCallback(int iConnection, size_t iClient, const eastl::string16& sParameters)
{
	if (net_debug.GetBool())
	{
		if (Network(iConnection)->IsHost())
			TMsg(sprintf(L"Cxn %d cmd from client %d: ", iConnection, iClient) + m_sName + L" " + sParameters + L"\n");
		else
			TMsg(sprintf(L"Cxn %d cmd from server: ", iConnection) + m_sName + L" " + sParameters + L"\n");
	}

	wcstok(sParameters, m_asArguments);

	m_pfnCallback(iConnection, this, iClient, sParameters);
}

size_t CNetworkCommand::GetNumArguments()
{
	return m_asArguments.size();
}

eastl::string16 CNetworkCommand::Arg(size_t iArg)
{
	TAssert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return m_asArguments[iArg];
}

bool CNetworkCommand::ArgAsBool(size_t iArg)
{
	TAssert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return !!_wtoi(m_asArguments[iArg].c_str());
}

size_t CNetworkCommand::ArgAsUInt(size_t iArg)
{
	TAssert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return _wtoi(m_asArguments[iArg].c_str());
}

int CNetworkCommand::ArgAsInt(size_t iArg)
{
	TAssert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return _wtoi(m_asArguments[iArg].c_str());
}

float CNetworkCommand::ArgAsFloat(size_t iArg)
{
	TAssert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return (float)_wtof(m_asArguments[iArg].c_str());
}

eastl::map<eastl::string16, CNetworkCommand*>& CNetworkCommand::GetCommands()
{
	static eastl::map<eastl::string16, CNetworkCommand*> aCommands;
	return aCommands;
}

CNetworkCommand* CNetworkCommand::GetCommand(const eastl::string16& sName)
{
	eastl::map<eastl::string16, CNetworkCommand*>::iterator it = GetCommands().find(sName);
	if (it == GetCommands().end())
		return NULL;

	return it->second;
}

void CNetworkCommand::RegisterCommand(CNetworkCommand* pCommand)
{
	GetCommands()[pCommand->m_sName] = pCommand;
}
