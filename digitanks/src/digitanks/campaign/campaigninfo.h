#ifndef DT_CAMPAIGNINFO_H
#define DT_CAMPAIGNINFO_H

#include <tvector.h>

#include <tstring.h>
#include <common.h>

// Data loaded from the disk about the campaign
class CCampaignInfo
{
	DECLARE_CLASS(CCampaignInfo, CCampaignInfo);

public:
									CCampaignInfo(tstring sScript);

public:
	static const CCampaignInfo*		GetCampaignInfo();

protected:
	void							ReadFromData(class CData* pData);

public:
	tstring				m_sName;
	tvector<tstring>	m_asLevels;
};

#endif
