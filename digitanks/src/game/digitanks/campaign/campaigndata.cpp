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

eastl::string CCampaignData::BeginCampaign()
{
	m_iCurrentLevel = 0;
	return m_pCampaignInfo->m_asLevels[m_iCurrentLevel];
}

eastl::string CCampaignData::ProceedToNextLevel()
{
	m_iCurrentLevel += 1;
	if (m_iHighestLevelReached < m_iCurrentLevel)
		m_iHighestLevelReached = m_iCurrentLevel;

	return GetCurrentLevelFile();
}

eastl::string CCampaignData::GetCurrentLevelFile()
{
	if (m_iCurrentLevel >= m_pCampaignInfo->m_asLevels.size())
		return "";

	return m_pCampaignInfo->m_asLevels[m_iCurrentLevel];
}

eastl::string CCampaignData::GetLevel(size_t i)
{
	if (i >= m_pCampaignInfo->m_asLevels.size())
		return "";

	return m_pCampaignInfo->m_asLevels[i];
}

void CCampaignData::ReadData(const tstring& sFile)
{
	std::basic_ifstream<tchar> f(convertstring<tchar, char>(sFile).c_str());
	CData* pData = new CData();
	CDataSerializer::Read(f, pData);

	CData* pCampaign = pData->FindChild(_T("Campaign"));
	m_iHighestLevelReached = pCampaign->FindChild(_T("HighestLevel"))->GetValueInt();
	m_iCurrentLevel = pCampaign->FindChild(_T("CurrentLevel"))->GetValueInt();

	delete pData;
}

void CCampaignData::SaveData(const tstring& sFile)
{
	std::basic_ofstream<tchar> f(convertstring<tchar, char>(sFile).c_str());
	CData* pData = new CData();

	CData* pCampaign = pData->AddChild(_T("Campaign"));
	pCampaign->AddChild(_T("HighestLevel"))->SetValue(m_iHighestLevelReached);
	pCampaign->AddChild(_T("CurrentLevel"))->SetValue(m_iCurrentLevel);

	CDataSerializer::Save(f, pData);

	delete pData;
}
