#include "campaigndata.h"

CCampaignData::CCampaignData(const CCampaignInfo* pCampaignInfo)
{
	m_pCampaignInfo = pCampaignInfo;
	m_iHighestLevelReached = 0;
	m_iCurrentLevel = 0;
}

eastl::string CCampaignData::GetFirstLevel()
{
	return m_pCampaignInfo->m_asLevels[0];
}

eastl::string CCampaignData::GetNextLevel()
{
	m_iCurrentLevel += 1;
	if (m_iHighestLevelReached < m_iCurrentLevel)
		m_iHighestLevelReached = m_iCurrentLevel;

	if (m_iCurrentLevel >= m_pCampaignInfo->m_asLevels.size())
		return "";

	return m_pCampaignInfo->m_asLevels[m_iCurrentLevel];
}
