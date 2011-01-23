#include "cvar.h"

#include <assert.h>

#include <strutils.h>

#include <tinker/application.h>

CCommand::CCommand(eastl::string16 sName, CommandCallback pfnCallback)
{
	m_sName = sName;
	m_pfnCallback = pfnCallback;

	RegisterCommand(this);
}

CCommand::CCommand(eastl::string sName, CommandCallback pfnCallback)
{
	m_sName = convertstring<char, char16_t>(sName);
	m_pfnCallback = pfnCallback;

	RegisterCommand(this);
}

void CCommand::Run(eastl::string16 sCommand)
{
	eastl::vector<eastl::string16> asTokens;
	wcstok(sCommand, asTokens);

	if (asTokens.size() == 0)
		return;

	eastl::map<eastl::string16, CCommand*>::iterator it = GetCommands().find(asTokens[0]);
	if (it == GetCommands().end())
	{
		TMsg(L"Unrecognized command.\n");
		return;
	}

	CCommand* pCommand = it->second;
	pCommand->m_pfnCallback(pCommand, asTokens);
}

void CCommand::RegisterCommand(CCommand* pCommand)
{
	GetCommands()[pCommand->m_sName] = pCommand;
}

void SetCVar(CCommand* pCommand, eastl::vector<eastl::string16>& asTokens)
{
	CVar* pCVar = dynamic_cast<CVar*>(pCommand);
	assert(pCVar);
	if (!pCVar)
		return;

	if (asTokens.size() > 1)
		pCVar->SetValue(asTokens[1]);

	TMsg(sprintf(L"%s = %s\n", pCVar->GetName().c_str(), pCVar->GetValue().c_str()));
}

CVar::CVar(eastl::string16 sName, eastl::string16 sValue)
	: CCommand(sName, SetCVar)
{
	m_sValue = sValue;
}

CVar::CVar(eastl::string sName, eastl::string sValue)
	: CCommand(sName, SetCVar)
{
	m_sValue = convertstring<char, char16_t>(sValue);
}

void CVar::SetValue(eastl::string16 sValue)
{
	m_sValue = sValue;
}

bool CVar::GetBool()
{
	if (m_sValue.comparei(L"yes") == 0)
		return true;

	if (m_sValue.comparei(L"true") == 0)
		return true;

	if (m_sValue.comparei(L"on") == 0)
		return true;

	// Don't want to use _wtoi I don't think it's as portable.
	if (atoi(convertstring<char16_t, char>(m_sValue).c_str()) != 0)
		return true;

	return false;
}

int CVar::GetInt()
{
	// Don't want to use _wtoi I don't think it's as portable.
	return atoi(convertstring<char16_t, char>(m_sValue).c_str());
}

float CVar::GetFloat()
{
	// Don't want to use _wtof I don't think it's as portable.
	return (float)atof(convertstring<char16_t, char>(m_sValue).c_str());
}
