#include "campaignentity.h"

#include <tinker/cvar.h>

#include <ui/digitankswindow.h>
#include "campaigndata.h"

REGISTER_ENTITY(CCampaignEntity);

NETVAR_TABLE_BEGIN(CCampaignEntity);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCampaignEntity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iLevel);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CCampaignEntity);
INPUTS_TABLE_END();

void CCampaignEntity::Spawn()
{
	BaseClass::Spawn();

	m_iLevel = DigitanksWindow()->GetCampaignData()->GetCurrentLevel();
}

bool CCampaignEntity::OnUnserialize(std::istream& i)
{
	DigitanksGame()->SetCurrentLevel(DigitanksWindow()->GetCampaignData()->GetLevel(m_iLevel));

	return BaseClass::OnUnserialize(i);
}
