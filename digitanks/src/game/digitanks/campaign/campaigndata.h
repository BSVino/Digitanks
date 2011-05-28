#ifndef DT_CAMPAIGNDATA_H
#define DT_CAMPAIGNDATA_H

#include <common.h>

#include "campaigninfo.h"

// Data about the user's campaign profile
class CCampaignData
{
	DECLARE_CLASS(CCampaignData, CCampaignData);

public:
									CCampaignData(const CCampaignInfo* pCampaignInfo);

public:
	eastl::string					BeginCampaign();
	eastl::string					ProceedToNextLevel();
	size_t							GetCurrentLevel() { return m_iCurrentLevel; };
	eastl::string					GetCurrentLevelFile();

	void							NewCampaign();

	void							ReadData(const eastl::string16& sFile);
	void							SaveData(const eastl::string16& sFile);

protected:
	const CCampaignInfo*			m_pCampaignInfo;

	size_t							m_iHighestLevelReached;
	size_t							m_iCurrentLevel;
};

#endif
