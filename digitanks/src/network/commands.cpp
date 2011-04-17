#include "commands.h"

#include <enet/enet.h>
#include <assert.h>

#include <strutils.h>

#include <tinker/application.h>

void CNetworkCommand::RunCommand(const eastl::string16& sParameters)
{
	RunCommand(sParameters, m_iMessageTarget);
}

void CNetworkCommand::RunCommand(const eastl::string16& sParameters, int iTarget)
{
	if (iTarget == NETWORK_TOCLIENTS)
	{
		if (CNetwork::IsHost())
		{
			// If I'm the host then pass me the message too.
			wcstok(sParameters, m_asArguments);
			m_pfnCallback(this, -1, sParameters);
		}
		else
		{
			// If we're running client functions then we're going to get this message from the server anyway.
			if (CNetwork::IsRunningClientFunctions())
				return;

			// Some shared code. Run the callback but don't send it over the wire.
			wcstok(sParameters, m_asArguments);
			m_pfnCallback(this, -1, sParameters);
			return;
		}
	}

	if (iTarget == NETWORK_TOSERVER)
	{
		if (CNetwork::IsHost() || !CNetwork::IsConnected())
		{
			wcstok(sParameters, m_asArguments);
			m_pfnCallback(this, -1, sParameters);
			return;
		}

		// If we're running client functions then the server already knows about this call.
		if (!CNetwork::IsHost() && CNetwork::IsRunningClientFunctions())
		{
			wcstok(sParameters, m_asArguments);
			m_pfnCallback(this, -1, sParameters);
			return;
		}
	}

	if (!CNetwork::IsConnected())
		return;

	eastl::string16 sCommand = m_sName + L" " + sParameters;

	CNetworkParameters p;
	p.CreateExtraData(sizeof(eastl::string16::value_type) * (sCommand.length() + 1));
	char16_t* pszData = (char16_t*)p.m_pExtraData;

	assert(sizeof(eastl::string16::value_type) == sizeof(char16_t));
	wcscpy(pszData, sCommand.c_str());

	p.ui1 = CNetwork::GetClientID();

	CNetwork::CallFunctionParameters(iTarget, "NC", &p);
}

void CNetworkCommand::RunCallback(size_t iClient, const eastl::string16& sParameters)
{
	wcstok(sParameters, m_asArguments);

	m_pfnCallback(this, iClient, sParameters);
}

size_t CNetworkCommand::GetNumArguments()
{
	return m_asArguments.size();
}

eastl::string16 CNetworkCommand::Arg(size_t iArg)
{
	assert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return m_asArguments[iArg];
}

bool CNetworkCommand::ArgAsBool(size_t iArg)
{
	assert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return !!_wtoi(m_asArguments[iArg].c_str());
}

size_t CNetworkCommand::ArgAsUInt(size_t iArg)
{
	assert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return _wtoi(m_asArguments[iArg].c_str());
}

int CNetworkCommand::ArgAsInt(size_t iArg)
{
	assert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return _wtoi(m_asArguments[iArg].c_str());
}

float CNetworkCommand::ArgAsFloat(size_t iArg)
{
	assert(iArg < GetNumArguments());
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
