#include "campaigndata.h"

#include <iostream>
#include <fstream>

#include <datamanager/dataserializer.h>

CCampaignData::CCampaignData(const CCampaignInfo* pCampaignInfo)
{
	m_pCampaignInfo = pCampaignInfo;
	m_iHighestLevelReached = 0;
	m_iCurrentLevel = 0;
}

tstring CCampaignData::BeginCampaign()
{
	m_iCurrentLevel = 0;
	return m_pCampaignInfo->m_asLevels[m_iCurrentLevel];
}

tstring CCampaignData::ProceedToNextLevel()
{
	m_iCurrentLevel += 1;
	if (m_iHighestLevelReached < m_iCurrentLevel)
		m_iHighestLevelReached = m_iCurrentLevel;

	return GetCurrentLevelFile();
}

tstring CCampaignData::GetCurrentLevelFile()
{
	if (m_iCurrentLevel >= m_pCampaignInfo->m_asLevels.size())
		return "";

	return m_pCampaignInfo->m_asLevels[m_iCurrentLevel];
}

tstring CCampaignData::GetLevel(size_t i)
{
	if (i >= m_pCampaignInfo->m_asLevels.size())
		return "";

	return m_pCampaignInfo->m_asLevels[i];
}

void CCampaignData::ReadData(const tstring& sFile)
{
	FILE* fp = tfopen_asset(sFile, "r");
	CData* pData = new CData();
	CDataSerializer::Read(fp, pData);

	CData* pCampaign = pData->FindChild("Campaign");
	m_iHighestLevelReached = pCampaign->FindChild("HighestLevel")->GetValueInt();
	m_iCurrentLevel = pCampaign->FindChild("CurrentLevel")->GetValueInt();

	delete pData;
}

void CCampaignData::SaveData(const tstring& sFile)
{
	FILE* fp = tfopen_asset(sFile, "w");
	CData* pData = new CData();

	CData* pCampaign = pData->AddChild("Campaign");
	pCampaign->AddChild("HighestLevel")->SetValue(m_iHighestLevelReached);
	pCampaign->AddChild("CurrentLevel")->SetValue(m_iCurrentLevel);

	CDataSerializer::Save(fp, pData);

	delete pData;
}
