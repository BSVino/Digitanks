#include "cvar.h"

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
	pCommand->m_pfnCallback(pCommand, asTokens, sCommand);
}

eastl::vector<eastl::string16> CCommand::GetCommandsBeginningWith(eastl::string16 sFragment)
{
	eastl::vector<eastl::string16> sResults;

	size_t iFragLength = sFragment.length();

	eastl::map<eastl::string16, CCommand*>& sCommands = GetCommands();
	for (eastl::map<eastl::string16, CCommand*>::iterator it = sCommands.begin(); it != sCommands.end(); it++)
	{
		if (it->first.substr(0, iFragLength) == sFragment)
			sResults.push_back(it->first);
	}

	return sResults;
}

void CCommand::RegisterCommand(CCommand* pCommand)
{
	GetCommands()[pCommand->m_sName] = pCommand;
}

void SetCVar(CCommand* pCommand, eastl::vector<eastl::string16>& asTokens, const eastl::string16& sCommand)
{
	CVar* pCVar = dynamic_cast<CVar*>(pCommand);
	TAssert(pCVar);
	if (!pCVar)
		return;

	if (asTokens.size() > 1)
		pCVar->SetValue(asTokens[1]);

	TMsg(sprintf(L"%s = %s\n", pCVar->GetName().c_str(), pCVar->GetValue().c_str()));
}

CVar::CVar(eastl::string16 sName, eastl::string16 sValue)
	: CCommand(sName, ::SetCVar)
{
	m_sValue = sValue;
}

CVar::CVar(eastl::string sName, eastl::string sValue)
	: CCommand(sName, ::SetCVar)
{
	m_sValue = convertstring<char, char16_t>(sValue);
}

void CVar::SetValue(eastl::string16 sValue)
{
	m_sValue = sValue;
}

void CVar::SetValue(int iValue)
{
	m_sValue = sprintf(L"%d", iValue);
}

void CVar::SetValue(float flValue)
{
	m_sValue = sprintf(L"%f", flValue);
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

CVar* CVar::FindCVar(eastl::string16 sName)
{
	eastl::map<eastl::string16, CCommand*>::iterator it = GetCommands().find(sName);
	if (it == GetCommands().end())
		return NULL;

	CVar* pVar = dynamic_cast<CVar*>(it->second);
	return pVar;
}

void CVar::SetCVar(eastl::string16 sName, eastl::string16 sValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(sValue);
}

void CVar::SetCVar(eastl::string16 sName, int iValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(iValue);
}

void CVar::SetCVar(eastl::string16 sName, float flValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(flValue);
}

eastl::string16 CVar::GetCVarValue(eastl::string16 sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return L"";

	return pVar->GetValue();
}

bool CVar::GetCVarBool(eastl::string16 sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return false;

	return pVar->GetBool();
}

int CVar::GetCVarInt(eastl::string16 sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return 0;

	return pVar->GetInt();
}

float CVar::GetCVarFloat(eastl::string16 sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return 0;

	return pVar->GetFloat();
}

void CVar::SetCVar(eastl::string sName, eastl::string sValue)
{
	SetCVar(convertstring<char, char16_t>(sName), convertstring<char, char16_t>(sValue));
}

void CVar::SetCVar(eastl::string sName, int iValue)
{
	SetCVar(convertstring<char, char16_t>(sName), iValue);
}

void CVar::SetCVar(eastl::string sName, float flValue)
{
	SetCVar(convertstring<char, char16_t>(sName), flValue);
}

eastl::string CVar::GetCVarValue(eastl::string sName)
{
	return convertstring<char16_t, char>(GetCVarValue(convertstring<char, char16_t>(sName)));
}

bool CVar::GetCVarBool(eastl::string sName)
{
	return GetCVarBool(convertstring<char, char16_t>(sName));
}

int CVar::GetCVarInt(eastl::string sName)
{
	return GetCVarInt(convertstring<char, char16_t>(sName));
}

float CVar::GetCVarFloat(eastl::string sName)
{
	return GetCVarFloat(convertstring<char, char16_t>(sName));
}
