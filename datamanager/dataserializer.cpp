#include "dataserializer.h"

#include <strutils.h>

#include "data.h"

void CDataSerializer::Read(std::istream& sStream, CData* pData)
{
	if (!sStream)
		return;

	if (!pData)
		return;

	char szLine[1024];
	eastl::string sLine;

	CData* pCurrentData = pData;
	CData* pLastData = NULL;

	while (sStream.getline(szLine, 1024))
	{
		sLine = eastl::string(szLine);

		size_t iComment = sLine.find("//");
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

		eastl::vector<eastl::string> asTokens;
		strtok(sLine, asTokens, ":");

		if (asTokens.size() == 1)
			pLastData = pCurrentData->AddChild(trim(sLine));
		else if (asTokens.size() >= 2)
			pLastData = pCurrentData->AddChild(trim(asTokens[0]), trim(sLine.substr(sLine.find(L':')+1)));
	}
}

static void SaveData(std::ostream& sStream, CData* pData, size_t iLevel)
{
	eastl::string sTabs;
	for (size_t i = 0; i < iLevel; i++)
		sTabs += "\t";

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChild = pData->GetChild(i);

		if (pChild->GetValueString().length())
			sStream << (sTabs + pChild->GetKey() + ": " + pChild->GetValueString() + "\n").c_str();
		else
			sStream << (sTabs + pChild->GetKey() + "\n").c_str();

		if (pChild->GetNumChildren())
		{
			sStream << (sTabs + "{\n").c_str();
			SaveData(sStream, pChild, iLevel+1);
			sStream << (sTabs + "}\n").c_str();
		}
	}
}

void CDataSerializer::Save(std::ostream& sStream, CData* pData)
{
	if (!sStream)
		return;

	if (!pData)
		return;

	SaveData(sStream, pData, 0);
}
