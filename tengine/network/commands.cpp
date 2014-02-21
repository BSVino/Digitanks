#include "commands.h"

#include <common.h>
#include <strutils.h>

#include <tinker/application.h>
#include <tinker/cvar.h>

CVar net_debug("net_debug", "off");

void CNetworkCommand::RunCommand(const tstring& sParameters)
{
	RunCommand(m_iConnection, sParameters, m_iMessageTarget);
}

void CNetworkCommand::RunCommand(int iConnection, const tstring& sParameters)
{
	RunCommand(iConnection, sParameters, m_iMessageTarget);
}

void CNetworkCommand::RunCommand(const tstring& sParameters, int iTarget)
{
	RunCommand(m_iConnection, sParameters, iTarget);
}

void CNetworkCommand::RunCommand(int iConnection, const tstring& sParameters, int iTarget)
{
	TAssert(iConnection != CONNECTION_UNDEFINED);

	bool bNoNetwork = false;
	bool bPredict = false;
	if (iTarget == NETWORK_TOCLIENTS || iTarget == NETWORK_TOREMOTECLIENTS)
	{
		if (Network(iConnection)->IsHost())
		{
			if (iTarget == NETWORK_TOCLIENTS)
				bPredict = true;
		}
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

	else if (iTarget == NETWORK_TOSERVER)
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

	else if (iTarget == NETWORK_BOT)
		return;

	else if (iTarget == NETWORK_LOCAL)
	{
		if (Network(iConnection)->IsHost())
		{
			bNoNetwork = true;
			bPredict = true;
		}
		else
		{
			TAssert(false);
			return;
		}
	}

	if (!bNoNetwork && Network(iConnection)->IsConnected())
	{
		tstring sCommand = m_sName + " " + sParameters;

		CNetworkParameters p;
		p.CreateExtraData(sizeof(tstring::value_type) * (sCommand.length() + 1));
		tchar* pszData = (tchar*)p.m_pExtraData;

		tstrncpy(pszData, sCommand.length()+1, sCommand.c_str(), sCommand.length()+1);

		p.ui1 = Network(iConnection)->GetClientID();

		Network(iConnection)->CallFunctionParameters(iTarget, "NC", &p);

		if (net_debug.GetBool())
		{
			if (iTarget == NETWORK_TOSERVER)
				TMsg(sprintf(tstring("Cxn %d to server: "), iConnection));
			else if (iTarget == NETWORK_TOCLIENTS)
				TMsg(sprintf(tstring("Cxn %d to clients: "), iConnection));
			else if (iTarget == NETWORK_TOEVERYONE)
				TMsg(sprintf(tstring("Cxn %d to all: "), iConnection));
			else
				TMsg(sprintf(tstring("Cxn %d to client %d: "), iConnection, iTarget));

			TMsg(sCommand + "\n");
		}
	}

	if (bPredict)
	{
		tstrtok(sParameters, m_asArguments);
		m_pfnCallback(iConnection, this, -1, sParameters);
	}
}

void CNetworkCommand::RunCallback(int iConnection, size_t iClient, const tstring& sParameters)
{
	if (net_debug.GetBool())
	{
		if (Network(iConnection)->IsHost())
			TMsg(sprintf(tstring("Cxn %d cmd from client %d: "), iConnection, iClient) + m_sName + " " + sParameters + "\n");
		else
			TMsg(sprintf(tstring("Cxn %d cmd from server: "), iConnection) + m_sName + " " + sParameters + "\n");
	}

	tstrtok(sParameters, m_asArguments);

	m_pfnCallback(iConnection, this, iClient, sParameters);
}

size_t CNetworkCommand::GetNumArguments()
{
	return m_asArguments.size();
}

tstring CNetworkCommand::Arg(size_t iArg)
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

	return !!stoi(m_asArguments[iArg].c_str());
}

size_t CNetworkCommand::ArgAsUInt(size_t iArg)
{
	TAssert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return stoi(m_asArguments[iArg].c_str());
}

int CNetworkCommand::ArgAsInt(size_t iArg)
{
	TAssert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return stoi(m_asArguments[iArg].c_str());
}

float CNetworkCommand::ArgAsFloat(size_t iArg)
{
	TAssert(iArg < GetNumArguments());
	if (iArg >= GetNumArguments())
		return 0;

	return (float)stof(m_asArguments[iArg].c_str());
}

tmap<tstring, CNetworkCommand*>& CNetworkCommand::GetCommands()
{
	static tmap<tstring, CNetworkCommand*> aCommands;
	return aCommands;
}

CNetworkCommand* CNetworkCommand::GetCommand(const tstring& sName)
{
	tmap<tstring, CNetworkCommand*>::iterator it = GetCommands().find(sName);
	if (it == GetCommands().end())
		return NULL;

	return it->second;
}

void CNetworkCommand::RegisterCommand(CNetworkCommand* pCommand)
{
	GetCommands()[pCommand->m_sName] = pCommand;
}
