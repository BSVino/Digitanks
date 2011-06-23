#include "campaigninfo.h"

#include <iostream>
#include <fstream>

#include <strutils.h>

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
	CData* pName = pData->FindChild(_T("Name"));
	if (pName)
		m_sName = pName->GetValueString();

	CData* pLevels = pData->FindChild(_T("Levels"));

	if (pLevels)
	{
		for (size_t i = 0; i < pLevels->GetNumChildren(); i++)
		{
			CData* pChild = pLevels->FindChild(sprintf(_T("%d"), i+1));

			TAssert(pChild);
			if (!pChild)
				continue;

			m_asLevels.push_back(pChild->GetValueString());
		}
	}
}

const CCampaignInfo* CCampaignInfo::GetCampaignInfo()
{
	static CCampaignInfo gInfo(_T("scripts/campaign.txt"));

	return &gInfo;
}
