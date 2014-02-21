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
	tstring							BeginCampaign();
	tstring							ProceedToNextLevel();
	size_t							GetCurrentLevel() { return m_iCurrentLevel; };
	tstring							GetCurrentLevelFile();

	tstring							GetLevel(size_t i);

	void							NewCampaign();

	void							ReadData(const tstring& sFile);
	void							SaveData(const tstring& sFile);

protected:
	const CCampaignInfo*			m_pCampaignInfo;

	size_t							m_iHighestLevelReached;
	size_t							m_iCurrentLevel;
};

#endif
