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
	eastl::string					GetFirstLevel();
	eastl::string					GetNextLevel();

protected:
	const CCampaignInfo*			m_pCampaignInfo;

	size_t							m_iHighestLevelReached;
	size_t							m_iCurrentLevel;
};

#endif
