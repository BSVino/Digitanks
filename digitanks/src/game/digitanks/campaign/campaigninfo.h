#ifndef DT_CAMPAIGNINFO_H
#define DT_CAMPAIGNINFO_H

#include <EASTL/string.h>
#include <EASTL/vector.h>

#include <common.h>

// Data loaded from the disk about the campaign
class CCampaignInfo
{
	DECLARE_CLASS(CCampaignInfo, CCampaignInfo);

public:
									CCampaignInfo(eastl::string16 sScript);

public:
	static const CCampaignInfo*		GetCampaignInfo();

protected:
	void							ReadFromData(class CData* pData);

public:
	eastl::string					m_sName;
	eastl::vector<eastl::string>	m_asLevels;
};

#endif
