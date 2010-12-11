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
		else if (asTokens.size() == 2)
			pLastData = pCurrentData->AddChild(trim(asTokens[0]), trim(asTokens[1]));
	}
}
