#include "campaigninfo.h"

#include <iostream>
#include <fstream>

#include <strutils.h>

#include <shell.h>

#include <datamanager/dataserializer.h>

CCampaignInfo::CCampaignInfo(tstring sScript)
{
	std::basic_ifstream<tchar> f(convertstring<tchar, char>(sScript).c_str());

	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	ReadFromData(pData);

	delete pData;
}

void CCampaignInfo::ReadFromData(CData* pData)
{
	CData* pName = pData->FindChild("Name");
	if (pName)
		m_sName = pName->GetValueString();

	CData* pLevels = pData->FindChild("Levels");

	if (pLevels)
	{
		for (size_t i = 0; i < pLevels->GetNumChildren(); i++)
		{
			CData* pChild = pLevels->FindChild(sprintf(tstring("%d"), i+1));

			TAssert(pChild);
			if (!pChild)
				continue;

			m_asLevels.push_back(pChild->GetValueString());
		}
	}
}

const CCampaignInfo* CCampaignInfo::GetCampaignInfo()
{
	static CCampaignInfo gInfo("scripts/campaign.txt");

	return &gInfo;
}
