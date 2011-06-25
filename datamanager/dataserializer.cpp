#include "dataserializer.h"

#include <strutils.h>

#include "data.h"

void CDataSerializer::Read(std::basic_istream<tchar>& sStream, CData* pData)
{
	if (!sStream)
		return;

	if (!pData)
		return;

	tchar szLine[1024];
	tstring sLine;

	CData* pCurrentData = pData;
	CData* pLastData = NULL;

	while (sStream.getline(szLine, 1024))
	{
		sLine = tstring(szLine);

		size_t iComment = sLine.find(_T("//"));
		if (iComment != eastl::string::npos)
			sLine = sLine.substr(0, iComment);

		sLine = trim(sLine);

		if (sLine.length() == 0)
			continue;

		if (sLine[0] == '{')
		{
			pCurrentData = pLastData;
			continue;
		}

		if (sLine[0] == '}')
		{
			pCurrentData = pCurrentData->GetParent();
			continue;
		}

		eastl::vector<tstring> asTokens;
		tstrtok(sLine, asTokens, _T(":"));

		if (asTokens.size() == 1)
			pLastData = pCurrentData->AddChild(trim(sLine));
		else if (asTokens.size() >= 2)
			pLastData = pCurrentData->AddChild(trim(asTokens[0]), trim(sLine.substr(sLine.find(_T(':'))+1)));
	}
}

static void SaveData(std::basic_ostream<tchar>& sStream, CData* pData, size_t iLevel)
{
	tstring sTabs;
	for (size_t i = 0; i < iLevel; i++)
		sTabs += _T("\t");

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChild = pData->GetChild(i);

		if (pChild->GetValueTString().length())
			sStream << (sTabs + pChild->GetKey() + _T(": ") + pChild->GetValueTString() + _T("\n")).c_str();
		else
			sStream << (sTabs + pChild->GetKey() + _T("\n")).c_str();

		if (pChild->GetNumChildren())
		{
			sStream << (sTabs + _T("{\n")).c_str();
			SaveData(sStream, pChild, iLevel+1);
			sStream << (sTabs + _T("}\n")).c_str();
		}
	}
}

void CDataSerializer::Save(std::basic_ostream<tchar>& sStream, CData* pData)
{
	if (!sStream)
		return;

	if (!pData)
		return;

	SaveData(sStream, pData, 0);
}
