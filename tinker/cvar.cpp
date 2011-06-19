#include "cvar.h"

#include <strutils.h>

#include <tinker/application.h>

CCommand::CCommand(tstring sName, CommandCallback pfnCallback)
{
	m_sName = sName;
	m_pfnCallback = pfnCallback;

	RegisterCommand(this);
}

CCommand::CCommand(eastl::string sName, CommandCallback pfnCallback)
{
	m_sName = convertstring<char, tchar>(sName);
	m_pfnCallback = pfnCallback;

	RegisterCommand(this);
}

void CCommand::Run(tstring sCommand)
{
	eastl::vector<tstring> asTokens;
	tstrtok(sCommand, asTokens);

	if (asTokens.size() == 0)
		return;

	eastl::map<tstring, CCommand*>::iterator it = GetCommands().find(asTokens[0]);
	if (it == GetCommands().end())
	{
		TMsg(_T("Unrecognized command.\n"));
		return;
	}

	CCommand* pCommand = it->second;
	pCommand->m_pfnCallback(pCommand, asTokens, sCommand);
}

eastl::vector<tstring> CCommand::GetCommandsBeginningWith(tstring sFragment)
{
	eastl::vector<tstring> sResults;

	size_t iFragLength = sFragment.length();

	eastl::map<tstring, CCommand*>& sCommands = GetCommands();
	for (eastl::map<tstring, CCommand*>::iterator it = sCommands.begin(); it != sCommands.end(); it++)
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

void SetCVar(CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand)
{
	CVar* pCVar = dynamic_cast<CVar*>(pCommand);
	TAssert(pCVar);
	if (!pCVar)
		return;

	if (asTokens.size() > 1)
		pCVar->SetValue(asTokens[1]);

	TMsg(sprintf(_T("%s = %s\n"), pCVar->GetName().c_str(), pCVar->GetValue().c_str()));
}

CVar::CVar(tstring sName, tstring sValue)
	: CCommand(sName, ::SetCVar)
{
	m_sValue = sValue;
}

CVar::CVar(eastl::string sName, eastl::string sValue)
	: CCommand(sName, ::SetCVar)
{
	m_sValue = convertstring<char, tchar>(sValue);
}

void CVar::SetValue(tstring sValue)
{
	m_sValue = sValue;
}

void CVar::SetValue(int iValue)
{
	m_sValue = sprintf(_T("%d"), iValue);
}

void CVar::SetValue(float flValue)
{
	m_sValue = sprintf(_T("%f"), flValue);
}

bool CVar::GetBool()
{
	if (m_sValue.comparei(_T("yes")) == 0)
		return true;

	if (m_sValue.comparei(_T("true")) == 0)
		return true;

	if (m_sValue.comparei(_T("on")) == 0)
		return true;

	// Don't want to use _wtoi I don't think it's as portable.
	if (atoi(convertstring<tchar, char>(m_sValue).c_str()) != 0)
		return true;

	return false;
}

int CVar::GetInt()
{
	// Don't want to use _wtoi I don't think it's as portable.
	return atoi(convertstring<tchar, char>(m_sValue).c_str());
}

float CVar::GetFloat()
{
	// Don't want to use _wtof I don't think it's as portable.
	return (float)atof(convertstring<tchar, char>(m_sValue).c_str());
}

CVar* CVar::FindCVar(tstring sName)
{
	eastl::map<tstring, CCommand*>::iterator it = GetCommands().find(sName);
	if (it == GetCommands().end())
		return NULL;

	CVar* pVar = dynamic_cast<CVar*>(it->second);
	return pVar;
}

void CVar::SetCVar(tstring sName, tstring sValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(sValue);
}

void CVar::SetCVar(tstring sName, int iValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(iValue);
}

void CVar::SetCVar(tstring sName, float flValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(flValue);
}

tstring CVar::GetCVarValue(tstring sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return _T("");

	return pVar->GetValue();
}

bool CVar::GetCVarBool(tstring sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return false;

	return pVar->GetBool();
}

int CVar::GetCVarInt(tstring sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return 0;

	return pVar->GetInt();
}

float CVar::GetCVarFloat(tstring sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return 0;

	return pVar->GetFloat();
}

void CVar::SetCVar(eastl::string sName, eastl::string sValue)
{
	SetCVar(convertstring<char, tchar>(sName), convertstring<char, tchar>(sValue));
}

void CVar::SetCVar(eastl::string sName, int iValue)
{
	SetCVar(convertstring<char, tchar>(sName), iValue);
}

void CVar::SetCVar(eastl::string sName, float flValue)
{
	SetCVar(convertstring<char, tchar>(sName), flValue);
}

eastl::string CVar::GetCVarValue(eastl::string sName)
{
	return convertstring<tchar, char>(GetCVarValue(convertstring<char, tchar>(sName)));
}

bool CVar::GetCVarBool(eastl::string sName)
{
	return GetCVarBool(convertstring<char, tchar>(sName));
}

int CVar::GetCVarInt(eastl::string sName)
{
	return GetCVarInt(convertstring<char, tchar>(sName));
}

float CVar::GetCVarFloat(eastl::string sName)
{
	return GetCVarFloat(convertstring<char, tchar>(sName));
}
